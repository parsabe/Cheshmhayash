// Cheshm Hayash - player controller.

#include "Core/CHPlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/CHPlayerCharacter.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Game/CHSegment.h"
#include "InputCoreTypes.h"
#include "Story/CHSave.h"
#include "Story/CHStoryDirector.h"

ACHPlayerController::ACHPlayerController()
{
	bShowMouseCursor = false;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;
}

void ACHPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CursorState = -1;
	bHasLastCursor = false;
	UpdateCursorMode(false);

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -80.f;
		PlayerCameraManager->ViewPitchMax = 80.f;
	}
}

void ACHPlayerController::SetSceneCamera(AActor* Camera, float BlendTime)
{
	if (ActiveSceneCamera.IsValid() && ActiveSceneCamera.Get() != GetPawn())
	{
		ActiveSceneCamera->SetActorRotation(SceneCameraBaseRot);
	}
	ActiveSceneCamera = Camera;
	SceneLookOffset = FRotator::ZeroRotator;
	if (Camera && Camera != GetPawn())
	{
		SceneCameraBaseRot = Camera->GetActorRotation();
	}

	AActor* Target = Camera ? Camera : static_cast<AActor*>(GetPawn());
	if (!Target)
	{
		return;
	}
	if (BlendTime > 0.f)
	{
		SetViewTargetWithBlend(Target, BlendTime, VTBlend_EaseInOut, 2.f);
	}
	else
	{
		SetViewTarget(Target);
	}
}

void ACHPlayerController::Shake(float Seconds, float Strength)
{
	ShakeTime = 0.f;
	ShakeDuration = FMath::Max(0.05f, Seconds);
	ShakeStrength = Strength;
}

void ACHPlayerController::UpdateShake(float DeltaTime)
{
	if (ShakeDuration <= 0.f && ShakeOffset.IsNearlyZero())
	{
		return;
	}
	FRotator NewOffset = FRotator::ZeroRotator;
	if (ShakeDuration > 0.f)
	{
		ShakeTime += DeltaTime;
		const float Fade = FMath::Clamp(1.f - ShakeTime / ShakeDuration, 0.f, 1.f);
		const float Amount = ShakeStrength * Fade * Fade;
		NewOffset.Pitch = FMath::PerlinNoise1D(ShakeTime * 23.f) * Amount;
		NewOffset.Yaw = FMath::PerlinNoise1D(ShakeTime * 19.f + 7.3f) * Amount;
		if (ShakeTime >= ShakeDuration)
		{
			ShakeDuration = 0.f;
			NewOffset = FRotator::ZeroRotator;
		}
	}
	// apply only the change, so the offset never accumulates
	SetControlRotation(GetControlRotation() + (NewOffset - ShakeOffset));
	ShakeOffset = NewOffset;
	if (ActiveSceneCamera.IsValid() && ActiveSceneCamera.Get() != GetPawn())
	{
		ActiveSceneCamera->SetActorRotation(SceneCameraBaseRot + SceneLookOffset + ShakeOffset);
	}
}

void ACHPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateShake(DeltaTime);

	UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	if (!Director)
	{
		return;
	}

	if (WasInputKeyJustPressed(EKeys::Escape) || WasInputKeyJustPressed(EKeys::P))
	{
		Director->InputPause();
	}

	const bool bControl = Director->IsPlayerControlEnabled() && !IsPaused();
	UpdateCursorMode(Director->WantsCursor());

	// any key (the main menu's "press any key")
	{
		TArray<FKey> AllKeys;
		EKeys::GetAllKeys(AllKeys);
		for (const FKey& Key : AllKeys)
		{
			if (Key.IsDigital() && !Key.IsGamepadKey() && !Key.IsTouch() && !Key.IsMouseButton() && WasInputKeyJustPressed(Key))
			{
				Director->InputAnyKey();
				break;
			}
		}
		if (WasInputKeyJustPressed(EKeys::LeftMouseButton) || WasInputKeyJustPressed(EKeys::RightMouseButton))
		{
			Director->InputAnyKey();
		}
	}

	// ---- menus, choices, advancing text
	if (WasInputKeyJustPressed(EKeys::Up) || (!bControl && WasInputKeyJustPressed(EKeys::W)))
	{
		Director->InputNav(-1);
	}
	if (WasInputKeyJustPressed(EKeys::Down) || (!bControl && WasInputKeyJustPressed(EKeys::S)))
	{
		Director->InputNav(1);
	}
	if (WasInputKeyJustPressed(EKeys::Enter) ||
		(!bControl && (WasInputKeyJustPressed(EKeys::SpaceBar) || WasInputKeyJustPressed(EKeys::E))))
	{
		Director->InputAdvance();
	}
	if (!bControl && WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		Director->InputClick();
	}
	if (WasInputKeyJustPressed(EKeys::PageDown))
	{
		Director->DevSkipScene();
	}
	const FKey NumberKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(NumberKeys); ++Index)
	{
		if (WasInputKeyJustPressed(NumberKeys[Index]))
		{
			Director->InputChoice(Index);
		}
	}

	// ---- mouse camera look: active in all 3D views (both first-person pawn and scene cameras) whenever no menu cursor is up
	const bool bWantCursor = Director->WantsCursor();
	if (!bWantCursor && !IsPaused())
	{
		float MouseX = 0.f;
		float MouseY = 0.f;
		GetInputMouseDelta(MouseX, MouseY);

		if (FSlateApplication::IsInitialized())
		{
			const FVector2D CurPos = FSlateApplication::Get().GetCursorPos();
			if (bHasLastCursor && FMath::IsNearlyZero(MouseX, 1e-4f) && FMath::IsNearlyZero(MouseY, 1e-4f))
			{
				const FVector2D Delta = CurPos - LastCursorPos;
				MouseX = Delta.X * 0.14f;
				MouseY = -Delta.Y * 0.14f;
			}
			LastCursorPos = CurPos;
			bHasLastCursor = true;
		}

		if (!FMath::IsNearlyZero(MouseX, 1e-5f) || !FMath::IsNearlyZero(MouseY, 1e-5f))
		{
			const float Sens = 1.65f * MouseSensitivity * CHSettings::GetSensitivity();
			const float YawDelta = MouseX * Sens;
			const float PitchDelta = MouseY * Sens * (CHSettings::GetInvertY() ? -1.f : 1.f);

			if (ActiveSceneCamera.IsValid() && ActiveSceneCamera.Get() != GetPawn() && GetViewTarget() == ActiveSceneCamera.Get())
			{
				SceneLookOffset.Yaw = FMath::Clamp(SceneLookOffset.Yaw + YawDelta, -85.f, 85.f);
				SceneLookOffset.Pitch = FMath::Clamp(SceneLookOffset.Pitch + PitchDelta, -55.f, 55.f);
				ActiveSceneCamera->SetActorRotation(SceneCameraBaseRot + SceneLookOffset + ShakeOffset);
			}
			else
			{
				FRotator ControlRot = GetControlRotation();
				ControlRot.Yaw = FRotator::NormalizeAxis(ControlRot.Yaw + YawDelta);
				ControlRot.Pitch = FMath::Clamp(FRotator::NormalizeAxis(ControlRot.Pitch) + PitchDelta, -80.f, 80.f);
				ControlRot.Roll = 0.f;
				SetControlRotation(ControlRot);
			}
		}
	}
	else
	{
		bHasLastCursor = false;
	}

	// ---- moving, looking, filming, using things
	ACHPlayerCharacter* Hero = Cast<ACHPlayerCharacter>(GetPawn());
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	if (Segment)
	{
		Segment->SetFilmHeld(bControl && IsInputKeyDown(EKeys::RightMouseButton));
	}
	if (!bControl || !Hero || (Segment && Segment->IsInputBlocked()))
	{
		return;
	}
	if (Segment && (WasInputKeyJustPressed(EKeys::E) || WasInputKeyJustPressed(EKeys::LeftMouseButton)))
	{
		Segment->Interact();
	}
	if (!Director->IsMoveAllowed())
	{
		return;
	}

	FVector2D Move(0.f, 0.f);
	if (IsInputKeyDown(EKeys::W) || IsInputKeyDown(EKeys::Up))
	{
		Move.Y += 1.f;
	}
	if (IsInputKeyDown(EKeys::S) || IsInputKeyDown(EKeys::Down))
	{
		Move.Y -= 1.f;
	}
	if (IsInputKeyDown(EKeys::D) || IsInputKeyDown(EKeys::Right))
	{
		Move.X += 1.f;
	}
	if (IsInputKeyDown(EKeys::A) || IsInputKeyDown(EKeys::Left))
	{
		Move.X -= 1.f;
	}

	const bool bSprint = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
	const bool bSneak = IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::C);
	Hero->SetPace(bSprint, bSneak);

	if (!Move.IsNearlyZero())
	{
		Move.Normalize();
		const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
		const FRotationMatrix YawMatrix(YawRotation);
		Hero->AddMovementInput(YawMatrix.GetUnitAxis(EAxis::X), Move.Y);
		Hero->AddMovementInput(YawMatrix.GetUnitAxis(EAxis::Y), Move.X);
	}

	if (WasInputKeyJustPressed(EKeys::SpaceBar))
	{
		Hero->Jump();
	}
}

void ACHPlayerController::UpdateCursorMode(bool bWantCursor)
{
	const int8 Desired = bWantCursor ? 1 : 0;
	UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	const bool bNeedsCaptureFix = (!bWantCursor && Viewport && Viewport->GetMouseCaptureMode() != EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
	if (Desired == CursorState && !bNeedsCaptureFix)
	{
		return;
	}
	CursorState = Desired;
	bShowMouseCursor = bWantCursor;
	if (bWantCursor)
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(Mode);
	}
	else
	{
		FInputModeGameOnly Mode;
		Mode.SetConsumeCaptureMouseDown(false);
		SetInputMode(Mode);
		if (Viewport)
		{
			Viewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
			Viewport->SetMouseLockMode(EMouseLockMode::LockAlways);
			Viewport->SetHideCursorDuringCapture(true);
		}
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().SetAllUserFocusToGameViewport();
		}
	}
}
