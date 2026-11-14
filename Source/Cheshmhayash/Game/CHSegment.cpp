// Cheshm Hayash - one gameplay segment in a location.

#include "Game/CHSegment.h"
#include "Camera/PlayerCameraManager.h"
#include "CollisionQueryParams.h"
#include "Core/CHPlayerCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/CHFilmable.h"
#include "Game/CHGuard.h"
#include "Game/CHInteractable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Story/CHStoryDirector.h"
#include "UI/CHStyle.h"
#include "UI/SCHHud.h"
#include "UI/SCHRootOverlay.h"
#include "World/CHLevelBuilder.h"

UCHSegmentSubsystem* UCHSegmentSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = (WorldContextObject && GEngine) ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UCHSegmentSubsystem>() : nullptr;
}

bool UCHSegmentSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UCHSegmentSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCHSegmentSubsystem, STATGROUP_Tickables);
}

// ---------------------------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------------------------

void UCHSegmentSubsystem::RequestGrid(const FBox2D& Bounds, float CellSize, float FloorZ)
{
	GridBounds = Bounds;
	GridCell = CellSize;
	GridFloor = FloorZ;
	GridDelayFrames = 2;
	bGridRequested = true;
}

void UCHSegmentSubsystem::AddGuard(ACHGuard* Guard)
{
	Guards.AddUnique(Guard);
}

void UCHSegmentSubsystem::AddFilmable(ACHFilmable* Filmable)
{
	Filmables.AddUnique(Filmable);
}

void UCHSegmentSubsystem::AddInteractable(ACHInteractable* Interactable)
{
	Interactables.AddUnique(Interactable);
}

void UCHSegmentSubsystem::AddLightPool(const FVector& Center, float Radius)
{
	LightPools.Add({ Center, Radius });
}

void UCHSegmentSubsystem::AddHidingSpot(const FVector& Center, float Radius)
{
	HidingSpots.Add({ Center, Radius });
}

void UCHSegmentSubsystem::AddCrowdZone(const FVector& Center, float Radius)
{
	CrowdZones.Add({ Center, Radius });
}

void UCHSegmentSubsystem::AddVanZone(const FVector& Center, float Radius)
{
	VanZones.Add({ Center, Radius });
}

void UCHSegmentSubsystem::AddExit(const FBox& Box, const FString& Id)
{
	Exits.Add({ Box, Id });
}

void UCHSegmentSubsystem::AddCheckpoint(const FBox& Box, const FTransform& InRespawn)
{
	FCHCheckpoint Checkpoint;
	Checkpoint.Box = Box;
	Checkpoint.Respawn = InRespawn;
	Checkpoints.Add(Checkpoint);
}

// ---------------------------------------------------------------------------------------------
// Segment
// ---------------------------------------------------------------------------------------------

void UCHSegmentSubsystem::Begin(ACHLevelBuilder* InBuilder, const FTransform& InRespawn, int32 InSecondChances)
{
	Builder = InBuilder;
	Respawn = InRespawn;
	SecondChances = InSecondChances;
	bActive = true;
	GraceTime = 1.2f;
	RestartTime = 0.f;
	FilmProgress = 0.f;
	FilmTarget = nullptr;
	for (FCHCheckpoint& Checkpoint : Checkpoints)
	{
		Checkpoint.bReached = false;
	}
	const UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	bDevBot = Director && Director->IsDevBot();
	bDevGhost = Director && Director->IsDevGhost();
	BotPath.Reset();
}

void UCHSegmentSubsystem::End()
{
	bActive = false;
	bFilming = false;
	bFilmHeld = false;
	FilmTarget = nullptr;
	PromptTarget = nullptr;
	if (ACHPlayerCharacter* Hero = Cast<ACHPlayerCharacter>(GetPlayerPawn()))
	{
		Hero->SetFilming(false);
	}
	PushHud();
}

void UCHSegmentSubsystem::SetHint(const FString& Fa, const FString& En)
{
	HintFa = Fa;
	HintEn = En;
}

APawn* UCHSegmentSubsystem::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

// ---------------------------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------------------------

void UCHSegmentSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGridRequested && --GridDelayFrames <= 0)
	{
		bGridRequested = false;
		const double Start = FPlatformTime::Seconds();
		Grid.Build(GetWorld(), GridBounds, GridCell, GridFloor);
		UE_LOG(LogTemp, Log, TEXT("CH nav grid built in %.1f ms"), (FPlatformTime::Seconds() - Start) * 1000.0);
	}

	UpdateRestart(DeltaTime);

	APawn* Pawn = GetPlayerPawn();
	if (bActive && Pawn)
	{
		UpdatePlayer(DeltaTime, Pawn);
		UpdateDevBot(DeltaTime, Pawn);
		UpdateFilming(DeltaTime, Pawn);
		UpdateInteraction(Pawn);
		UpdateZones(Pawn);
	}
	else
	{
		bFilming = false;
		PromptTarget = nullptr;
	}
	if (GraceTime > 0.f)
	{
		GraceTime -= DeltaTime;
	}

	MaxSuspicion = 0.f;
	bAnyChasing = false;
	for (const TWeakObjectPtr<ACHGuard>& Guard : Guards)
	{
		if (Guard.IsValid())
		{
			MaxSuspicion = FMath::Max(MaxSuspicion, Guard->GetSuspicion());
			bAnyChasing |= Guard->IsChasing();
		}
	}

	PushHud();
}

void UCHSegmentSubsystem::UpdatePlayer(float DeltaTime, APawn* Pawn)
{
	ACHPlayerCharacter* Hero = Cast<ACHPlayerCharacter>(Pawn);
	const FVector Location = Pawn->GetActorLocation();
	const float Speed = Pawn->GetVelocity().Size2D();

	float Visibility = 1.f;
	if (Hero && Hero->IsSneaking())
	{
		Visibility *= 0.55f;
	}
	else if (Speed > 450.f)
	{
		Visibility *= 1.25f;
	}
	if (bFilming)
	{
		Visibility *= 1.75f;
	}

	bInLight = false;
	for (const FCHZone& Pool : LightPools)
	{
		if (FVector::Dist2D(Pool.Center, Location) < Pool.Radius)
		{
			bInLight = true;
			break;
		}
	}
	Visibility *= bInLight ? 1.35f : 0.8f;

	bHidden = false;
	if (Speed < 60.f && !bFilming)
	{
		for (const FCHZone& Spot : HidingSpots)
		{
			if (FVector::Dist2D(Spot.Center, Location) < Spot.Radius)
			{
				bHidden = true;
				break;
			}
		}
	}
	if (bHidden)
	{
		Visibility *= 0.12f;
	}
	for (const FCHZone& Crowd : CrowdZones)
	{
		if (FVector::Dist2D(Crowd.Center, Location) < Crowd.Radius)
		{
			Visibility *= 0.5f;
			break;
		}
	}
	PlayerVisibility = Visibility;

	// running feet are heard
	NoiseTimer -= DeltaTime;
	if (Speed > 450.f && NoiseTimer <= 0.f && !(Hero && Hero->IsSneaking()))
	{
		NoiseTimer = 0.5f;
		for (const TWeakObjectPtr<ACHGuard>& Guard : Guards)
		{
			if (Guard.IsValid())
			{
				const float Distance = FVector::Dist(Guard->GetActorLocation(), Location);
				if (Distance < 800.f)
				{
					Guard->HearNoise(Location, 0.28f * (1.f - Distance / 800.f));
				}
			}
		}
	}
}

void UCHSegmentSubsystem::UpdateDevBot(float DeltaTime, APawn* Pawn)
{
	bBotFilm = false;
	if (!bDevBot || RestartTime > 0.f || Exits.Num() == 0)
	{
		return;
	}
	APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());
	if (!Controller)
	{
		return;
	}
	const FVector Location = Pawn->GetActorLocation();

	// film anything worth filming that is close and in sight
	if (bFilmAllowed)
	{
		for (const TWeakObjectPtr<ACHFilmable>& Filmable : Filmables)
		{
			if (!Filmable.IsValid() || Filmable->IsFilmed())
			{
				continue;
			}
			const FVector Target = Filmable->GetActorLocation();
			if (FVector::Dist2D(Target, Location) > 1500.f)
			{
				continue;
			}
			FCollisionQueryParams Params(SCENE_QUERY_STAT(CHBot), false, Pawn);
			if (GetWorld()->LineTraceTestByChannel(Location + FVector(0.f, 0.f, 60.f), Target, ECC_Visibility, Params))
			{
				continue;
			}
			const FVector Eye = Controller->PlayerCameraManager ? Controller->PlayerCameraManager->GetCameraLocation() : Location;
			const FRotator Look = (Target - Eye).Rotation();
			Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), Look, DeltaTime, 6.f));
			bBotFilm = true;
			return;
		}
	}

	// walk to the exit
	const FVector Goal = Exits[0].Box.GetCenter();
	BotRepath -= DeltaTime;
	if (BotPath.Num() == 0 || BotRepath <= 0.f)
	{
		BotRepath = 1.f;
		BotIndex = 0;
		if (!Grid.IsValid() || !Grid.FindPath(Location, Goal, BotPath))
		{
			BotPath.Reset();
			BotPath.Add(Goal);
		}
	}
	while (BotPath.IsValidIndex(BotIndex) && FVector::Dist2D(Location, BotPath[BotIndex]) < 60.f && BotIndex < BotPath.Num() - 1)
	{
		++BotIndex;
	}
	if (BotPath.IsValidIndex(BotIndex))
	{
		FVector Direction = BotPath[BotIndex] - Location;
		Direction.Z = 0.f;
		if (!Direction.IsNearlyZero())
		{
			Direction.Normalize();
			Pawn->AddMovementInput(Direction, 1.f);
			const FRotator Want(-12.f, Direction.Rotation().Yaw, 0.f);
			Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), Want, DeltaTime, 2.5f));
		}
	}
}

void UCHSegmentSubsystem::UpdateFilming(float DeltaTime, APawn* Pawn)
{
	ACHPlayerCharacter* Hero = Cast<ACHPlayerCharacter>(Pawn);
	bFilming = bFilmAllowed && (bFilmHeld || bBotFilm) && RestartTime <= 0.f;
	if (Hero)
	{
		Hero->SetFilming(bFilming);
	}

	if (!bFilming)
	{
		FilmProgress = FMath::Max(0.f, FilmProgress - DeltaTime * 1.5f);
		if (FilmTarget.IsValid())
		{
			FilmTarget->SetProgress(FilmProgress);
		}
		if (FilmProgress <= 0.f)
		{
			FilmTarget = nullptr;
		}
		return;
	}

	APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!Camera)
	{
		return;
	}
	const FVector CameraLocation = Camera->GetCameraLocation();
	const FVector CameraForward = Camera->GetCameraRotation().Vector();

	ACHFilmable* Best = nullptr;
	float BestAngle = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<ACHFilmable>& Filmable : Filmables)
	{
		if (!Filmable.IsValid() || Filmable->IsFilmed())
		{
			continue;
		}
		const FVector To = Filmable->GetActorLocation() - CameraLocation;
		const float Distance = To.Size();
		if (Distance > Filmable->Range || Distance < 1.f)
		{
			continue;
		}
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(CameraForward, To / Distance), -1.f, 1.f)));
		if (Angle > Filmable->AimAngle)
		{
			continue;
		}
		// stop a little short: the moment may sit on a wall
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CHFilmSight), false, Pawn);
		const FVector End = CameraLocation + To * (1.f - FMath::Min(80.f / Distance, 0.5f));
		if (GetWorld()->LineTraceTestByChannel(CameraLocation, End, ECC_Visibility, Params))
		{
			continue;
		}
		if (Angle < BestAngle)
		{
			BestAngle = Angle;
			Best = Filmable.Get();
		}
	}

	if (Best != FilmTarget.Get())
	{
		if (FilmTarget.IsValid())
		{
			FilmTarget->SetProgress(0.f);
		}
		FilmTarget = Best;
		FilmProgress = 0.f;
	}
	if (!Best)
	{
		return;
	}

	FilmProgress += DeltaTime / FMath::Max(0.5f, Best->FilmSeconds);
	Best->SetProgress(FilmProgress);
	if (FilmProgress < 1.f)
	{
		return;
	}

	// the clip is saved
	Best->MarkFilmed();
	FilmTarget = nullptr;
	FilmProgress = 0.f;
	if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
	{
		UTexture* Image = Director->CaptureClip(Best->ClipId, Best->CaptionFa, Best->CaptionEn);
		Director->Toast(TEXT("ثبت شد"), FString::Printf(TEXT("SAVED  ·  %s"), *Best->CaptionEn.ToUpper()), CH::Col::Paper, 3.2f, Image);
	}
	if (Builder.IsValid())
	{
		Builder->OnFilmed(Best->ClipId);
	}
}

void UCHSegmentSubsystem::UpdateInteraction(APawn* Pawn)
{
	PromptTarget = nullptr;
	if (RestartTime > 0.f)
	{
		return;
	}
	const FVector Location = Pawn->GetActorLocation();
	float Best = TNumericLimits<float>::Max();
	for (const TWeakObjectPtr<ACHInteractable>& Interactable : Interactables)
	{
		if (!Interactable.IsValid() || !Interactable->IsEnabled())
		{
			continue;
		}
		const FVector To = Interactable->GetActorLocation() - Location;
		const float Distance = To.Size2D();
		if (Distance > Interactable->Radius || FMath::Abs(To.Z) > 220.f)
		{
			continue;
		}
		if (Distance < Best)
		{
			Best = Distance;
			PromptTarget = Interactable;
		}
	}
}

void UCHSegmentSubsystem::Interact()
{
	if (bActive && PromptTarget.IsValid() && Builder.IsValid() && RestartTime <= 0.f)
	{
		ACHInteractable* Target = PromptTarget.Get();
		Builder->OnInteract(Target->Id, Target);
	}
}

void UCHSegmentSubsystem::UpdateZones(APawn* Pawn)
{
	const FVector Location = Pawn->GetActorLocation();

	for (FCHCheckpoint& Checkpoint : Checkpoints)
	{
		if (!Checkpoint.bReached && Checkpoint.Box.IsInsideOrOn(Location))
		{
			Checkpoint.bReached = true;
			Respawn = Checkpoint.Respawn;
		}
	}

	bInVanZone = false;
	for (const FCHZone& Van : VanZones)
	{
		if (FVector::Dist2D(Van.Center, Location) < Van.Radius)
		{
			bInVanZone = true;
			break;
		}
	}

	if (RestartTime > 0.f)
	{
		return;
	}
	for (const FCHExit& Exit : Exits)
	{
		if (Exit.Box.IsInsideOrOn(Location))
		{
			const FString Id = Exit.Id;
			if (Builder.IsValid())
			{
				Builder->OnSegmentExit(Id);
			}
			break;
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Caught
// ---------------------------------------------------------------------------------------------

void UCHSegmentSubsystem::OnCaught(ACHGuard* Guard)
{
	if (!bActive || IsPlayerUntouchable())
	{
		return;
	}
	UCHStoryDirector* Director = UCHStoryDirector::Get(this);

	if (SecondChances > 0)
	{
		// a second chance: he twists free and runs
		--SecondChances;
		GraceTime = 2.6f;
		const FVector Location = Guard ? Guard->GetActorLocation() : FVector::ZeroVector;
		for (const TWeakObjectPtr<ACHGuard>& Other : Guards)
		{
			if (Other.IsValid() && (Other.Get() == Guard || (Other->IsChasing() && FVector::Dist(Other->GetActorLocation(), Location) < 1000.f)))
			{
				Other->Stun(Other.Get() == Guard ? 3.4f : 1.8f);
			}
		}
		if (Director)
		{
			Director->Flash(FLinearColor(0.6f, 0.02f, 0.01f), 0.5f);
			Director->Toast(TEXT("فرار کن!"), TEXT("RUN"), CH::Col::Ember, 1.8f);
		}
		return;
	}

	// back to the last checkpoint
	RestartTime = 2.4f;
	bRestartDone = false;
	if (Director)
	{
		Director->Fade(1.f, 0.3f, FLinearColor::Black);
		Director->Toast(TEXT("دیدنت."), TEXT("THEY SAW YOU"), CH::Col::Paper, 2.0f);
	}
}

void UCHSegmentSubsystem::UpdateRestart(float DeltaTime)
{
	if (RestartTime <= 0.f)
	{
		return;
	}
	RestartTime -= DeltaTime;
	if (!bRestartDone && RestartTime < 1.4f)
	{
		bRestartDone = true;
		ResetToCheckpoint();
	}
	if (RestartTime <= 0.f)
	{
		RestartTime = 0.f;
		GraceTime = 1.5f;
		if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
		{
			Director->Fade(0.f, 0.9f, FLinearColor::Black);
		}
	}
}

void UCHSegmentSubsystem::ResetToCheckpoint()
{
	for (const TWeakObjectPtr<ACHGuard>& Guard : Guards)
	{
		if (Guard.IsValid())
		{
			Guard->ResetGuard();
		}
	}
	if (APawn* Pawn = GetPlayerPawn())
	{
		Pawn->TeleportTo(Respawn.GetLocation(), Respawn.Rotator(), false, true);
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			Character->GetCharacterMovement()->StopMovementImmediately();
		}
		if (AController* Controller = Pawn->GetController())
		{
			Controller->SetControlRotation(Respawn.Rotator());
		}
	}
	if (FilmTarget.IsValid())
	{
		FilmTarget->SetProgress(0.f);
	}
	FilmTarget = nullptr;
	FilmProgress = 0.f;
	BotPath.Reset();
	if (Builder.IsValid())
	{
		Builder->OnSegmentReset();
	}
}

// ---------------------------------------------------------------------------------------------
// HUD
// ---------------------------------------------------------------------------------------------

void UCHSegmentSubsystem::PushHud()
{
	UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	TSharedPtr<SCHRootOverlay> UI = Director ? Director->GetUI() : nullptr;
	if (!UI.IsValid() || !UI->Hud.IsValid())
	{
		return;
	}

	FCHHudState State;
	State.bActive = bActive;
	State.Suspicion = bActive ? MaxSuspicion : 0.f;
	State.bChased = bActive && bAnyChasing;
	State.bFilming = bActive && bFilming;
	State.bCanFilm = bActive && bFilmAllowed;
	State.FilmProgress = FilmProgress;
	if (FilmTarget.IsValid())
	{
		State.bHasTarget = true;
		State.TargetFa = FilmTarget->CaptionFa;
		State.TargetEn = FilmTarget->CaptionEn;
	}
	if (bActive && PromptTarget.IsValid())
	{
		State.PromptFa = PromptTarget->PromptFa;
		State.PromptEn = PromptTarget->PromptEn;
	}
	if (bActive)
	{
		State.HintFa = HintFa;
		State.HintEn = HintEn;
	}
	State.Chances = SecondChances;
	State.bHidden = bActive && bHidden;
	UI->Hud->SetState(State);
}
