// Cheshm Hayash - player controller.

#include "Core/CHPlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/CHPlayerCharacter.h"
#include "InputCoreTypes.h"
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

	SetInputMode(FInputModeGameOnly());

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -70.f;
		PlayerCameraManager->ViewPitchMax = 55.f;
	}
}

void ACHPlayerController::SetSceneCamera(AActor* Camera, float BlendTime)
{
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

void ACHPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

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
		(!bControl && (WasInputKeyJustPressed(EKeys::SpaceBar) || WasInputKeyJustPressed(EKeys::LeftMouseButton) || WasInputKeyJustPressed(EKeys::E))))
	{
		Director->InputAdvance();
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

	// ---- moving and looking
	ACHPlayerCharacter* Hero = Cast<ACHPlayerCharacter>(GetPawn());
	if (!bControl || !Hero)
	{
		return;
	}

	float MouseX = 0.f;
	float MouseY = 0.f;
	GetInputMouseDelta(MouseX, MouseY);
	AddYawInput(MouseX * MouseSensitivity);
	AddPitchInput(-MouseY * MouseSensitivity);

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
