// Cheshm Hayash - a person in the world who is not the player.

#include "World/CHNpc.h"
#include "Animation/AnimSequence.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "World/CHLooks.h"

ACHNpc::ACHNpc()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	Body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
	Body->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	Body->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Body->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> JogFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> FallFinder(TEXT("/Game/Characters/Mannequins/Anims/Death/MM_Death_Back_01.MM_Death_Back_01"));
	IdleAnim = IdleFinder.Object;
	WalkAnim = WalkFinder.Object;
	JogAnim = JogFinder.Object;
	FallAnim = FallFinder.Object;
	if (MannyFinder.Object)
	{
		Body->SetSkeletalMeshAsset(MannyFinder.Object);
	}
}

void ACHNpc::SetLook(const FString& InLook, int32 Seed)
{
	Look = InLook;
	const float Scale = CHLooks::Apply(this, Body, InLook, Seed, Shapes, Materials);
	SetActorScale3D(FVector(Scale));

	// a new mesh asset resets the animation
	CurrentAnim = nullptr;
	PlayLoop(bMoving ? WalkAnim : IdleAnim);
}

void ACHNpc::PlayLoop(UAnimSequence* Sequence, float Rate)
{
	if (!Sequence || bFrozen)
	{
		return;
	}
	if (CurrentAnim != Sequence)
	{
		CurrentAnim = Sequence;
		Body->PlayAnimation(Sequence, true);
	}
	Body->SetPlayRate(Rate);
}

void ACHNpc::WalkTo(const FVector& Target, float Speed)
{
	MoveTarget = Target;
	MoveSpeed = FMath::Max(10.f, Speed);
	bMoving = true;
	if (MoveSpeed > 260.f)
	{
		PlayLoop(JogAnim, FMath::Clamp(MoveSpeed / 420.f, 0.7f, 1.5f));
	}
	else
	{
		PlayLoop(WalkAnim, FMath::Clamp(MoveSpeed / 150.f, 0.55f, 1.6f));
	}
}

void ACHNpc::Stop()
{
	bMoving = false;
	PlayLoop(IdleAnim);
}

void ACHNpc::FaceLocation(const FVector& Target, bool bInstant)
{
	FVector Direction = Target - GetActorLocation();
	Direction.Z = 0.f;
	if (!Direction.IsNearlyZero())
	{
		FaceYaw(Direction.Rotation().Yaw, bInstant);
	}
}

void ACHNpc::FaceYaw(float Yaw, bool bInstant)
{
	TargetYaw = Yaw;
	if (bInstant)
	{
		SetActorRotation(FRotator(0.f, Yaw, 0.f));
		bTurning = false;
	}
	else
	{
		bTurning = true;
	}
}

void ACHNpc::SetSink(float Centimetres)
{
	Sink = Centimetres;
	Body->SetRelativeLocation(FVector(0.f, 0.f, -Sink));
}

void ACHNpc::PlayFall()
{
	bMoving = false;
	if (FallAnim)
	{
		CurrentAnim = FallAnim;
		Body->PlayAnimation(FallAnim, false);
	}
}

void ACHNpc::Freeze()
{
	bFrozen = true;
	bMoving = false;
	Body->bPauseAnims = true;
}

void ACHNpc::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFrozen)
	{
		return;
	}

	if (bMoving)
	{
		const FVector Location = GetActorLocation();
		FVector ToTarget = MoveTarget - Location;
		ToTarget.Z = 0.f;
		const float Distance = ToTarget.Size();
		const float Step = MoveSpeed * DeltaSeconds;
		if (Distance <= Step)
		{
			SetActorLocation(FVector(MoveTarget.X, MoveTarget.Y, Location.Z));
			bMoving = false;
			PlayLoop(IdleAnim);
		}
		else
		{
			const FVector Direction = ToTarget / Distance;
			SetActorLocation(Location + Direction * Step);
			TargetYaw = Direction.Rotation().Yaw;
			bTurning = true;
		}
	}

	if (bTurning)
	{
		const FRotator Current = GetActorRotation();
		const FRotator Target(0.f, TargetYaw, 0.f);
		const FRotator Next = FMath::RInterpConstantTo(Current, Target, DeltaSeconds, 320.f);
		SetActorRotation(Next);
		if (Next.Equals(Target, 0.5f))
		{
			bTurning = false;
		}
	}

	if (bShaking)
	{
		ShakeTime += DeltaSeconds;
		const float Tremble = FMath::Sin(ShakeTime * 19.f) * 0.5f + FMath::Sin(ShakeTime * 7.3f) * 0.35f;
		const float Sob = FMath::Max(0.f, FMath::Sin(ShakeTime * 2.1f));
		Body->SetRelativeLocation(FVector(0.f, 0.f, -Sink + Tremble * Sob));
		Body->SetRelativeRotation(FRotator(0.f, -90.f, Tremble * Sob * 0.8f + Sob * 2.5f));
	}
}
