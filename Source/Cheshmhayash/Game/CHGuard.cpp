// Cheshm Hayash - guards: patrol, notice, chase, search, return.

#include "Game/CHGuard.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Game/CHSegment.h"
#include "Game/CHVisionCone.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/CHLevelBuilder.h"
#include "World/CHLooks.h"

namespace
{
	const float EyeHeight = 66.f; // above the capsule centre
	const FLinearColor CalmCone(0.80f, 0.84f, 0.80f);
	const FLinearColor WaryCone(1.f, 0.46f, 0.08f);
	const FLinearColor ChaseCone(1.f, 0.06f, 0.03f);
}

ACHGuard::ACHGuard()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 92.f);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 320.f, 0.f);
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->MaxAcceleration = 1000.f;
	Movement->BrakingDecelerationWalking = 1400.f;
	Movement->GravityScale = 1.4f;
	Movement->MaxStepHeight = 40.f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	USkeletalMeshComponent* Body = GetMesh();
	Body->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -92.f), FRotator(0.f, -90.f, 0.f));
	if (MannyFinder.Object)
	{
		Body->SetSkeletalMeshAsset(MannyFinder.Object);
	}
	if (AnimFinder.Class)
	{
		Body->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Body->SetAnimInstanceClass(AnimFinder.Class);
	}

	Cone = CreateDefaultSubobject<UCHVisionCone>(TEXT("Cone"));
	Cone->SetupAttachment(RootComponent);
	Cone->SetRelativeLocation(FVector(0.f, 0.f, -74.f));

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(RootComponent);
	Flashlight->SetRelativeLocationAndRotation(FVector(24.f, 16.f, 36.f), FRotator(-13.f, 0.f, 0.f));
	Flashlight->SetIntensityUnits(ELightUnits::Candelas);
	Flashlight->Intensity = 260.f;
	Flashlight->LightColor = FColor(214, 226, 255);
	Flashlight->AttenuationRadius = 1700.f;
	Flashlight->OuterConeAngle = 21.f;
	Flashlight->InnerConeAngle = 9.f;
	Flashlight->SourceRadius = 2.f;
	Flashlight->CastShadows = true;
	Flashlight->VolumetricScatteringIntensity = 3.f;

	AlertMark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AlertMark"));
	AlertMark->SetupAttachment(RootComponent);
	AlertMark->SetStaticMesh(CubeFinder.Object);
	AlertMark->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 132.f), FRotator(45.f, 0.f, 45.f));
	AlertMark->SetRelativeScale3D(FVector(0.11f));
	AlertMark->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	AlertMark->SetCastShadow(false);
	AlertMark->SetVisibility(false);
}

void ACHGuard::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Unlit.M_CH_Unlit"), nullptr, LOAD_NoWarn))
	{
		AlertMaterial = UMaterialInstanceDynamic::Create(Base, this);
		AlertMaterial->SetVectorParameterValue(TEXT("Color"), WaryCone);
		AlertMaterial->SetScalarParameterValue(TEXT("Strength"), 8.f);
		AlertMark->SetMaterial(0, AlertMaterial);
	}
	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
	{
		Segment->AddGuard(this);
	}
}

void ACHGuard::Setup(ECHGuardKind InKind, const FString& InLook, const TArray<FVector>& InRoute, int32 InSeed)
{
	Kind = InKind;
	Look = InLook;
	Seed = InSeed;
	Route = InRoute;
	StartTransform = GetActorTransform();
	if (!GetController())
	{
		SpawnDefaultController();
	}
	bRevealed = true;
	bHasLight = true;

	switch (Kind)
	{
	case ECHGuardKind::Watcher:
		WalkSpeed = 120.f;
		ChaseSpeed = 400.f;
		SightRange = 1150.f;
		SightHalfAngle = 30.f;
		WaitAtPoint = 0.f;
		bHasLight = !Look.Contains(TEXT("_f")) && Look != TEXT("chador");
		break;
	case ECHGuardKind::Plainclothes:
		WalkSpeed = 125.f;
		ChaseSpeed = 470.f;
		SightRange = 950.f;
		SightHalfAngle = 46.f;
		NoticeTime = 0.9f;
		bHasLight = false;
		bRevealed = false;
		break;
	case ECHGuardKind::Motorcycle:
		WalkSpeed = 640.f;
		ChaseSpeed = 880.f;
		SightRange = 2000.f;
		SightHalfAngle = 17.f;
		NearRange = 330.f;
		CatchRadius = 190.f;
		NoticeTime = 0.85f;
		WaitAtPoint = 0.f;
		break;
	default:
		break;
	}

	if (Kind == ECHGuardKind::Motorcycle)
	{
		GetCapsuleComponent()->SetCapsuleSize(52.f, 92.f);
		GetMesh()->SetVisibility(false);
		GetMesh()->SetComponentTickEnabled(false);
		UCharacterMovementComponent* Movement = GetCharacterMovement();
		Movement->RotationRate = FRotator(0.f, 210.f, 0.f);
		Movement->MaxAcceleration = 1500.f;
		Movement->BrakingDecelerationWalking = 1800.f;
		BuildMotorcycle();
	}
	else
	{
		CHLooks::Apply(this, GetMesh(), Look, Seed, Shapes, Materials);
	}

	Cone->Configure(SightRange, SightHalfAngle);
	Flashlight->SetVisibility(bHasLight && bRevealed);
}

void ACHGuard::SetScan(float YawA, float YawB, float Period)
{
	ScanYawA = YawA;
	ScanYawB = YawB;
	ScanPeriod = FMath::Max(1.f, Period);
	bScans = true;
}

UStaticMeshComponent* ACHGuard::AddPart(UStaticMesh* PartMesh, const FVector& Center, const FVector& Size, UMaterialInterface* Material, const FRotator& Rotation)
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetStaticMesh(PartMesh);
	if (Material)
	{
		Part->SetMaterial(0, Material);
	}
	Part->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Part->SetCanEverAffectNavigation(false);
	Part->SetupAttachment(RootComponent);
	Part->SetRelativeTransform(FTransform(Rotation, Center, Size / 100.0));
	Part->RegisterComponent();
	Parts.Add(Part);
	return Part;
}

void ACHGuard::BuildMotorcycle()
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	auto Mat = [this](const FLinearColor& Color, float Roughness = 0.6f)
	{
		UMaterialInstanceDynamic* Material = CHLooks::MakeLit(this, Color, Roughness);
		Materials.Add(Material);
		return Material;
	};
	UMaterialInterface* Frame = Mat(FLinearColor(0.012f, 0.012f, 0.014f), 0.4f);
	UMaterialInterface* Tyre = Mat(FLinearColor(0.006f, 0.006f, 0.006f), 0.9f);
	UMaterialInterface* Chrome = Mat(FLinearColor(0.25f, 0.25f, 0.26f), 0.25f);
	UMaterialInterface* Uniform = Mat(FLinearColor(0.045f, 0.040f, 0.026f), 0.9f);
	UMaterialInterface* Helmet = Mat(FLinearColor(0.010f, 0.010f, 0.011f), 0.35f);
	UMaterialInterface* Lamp = nullptr;
	UMaterialInterface* Tail = nullptr;
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Unlit.M_CH_Unlit"), nullptr, LOAD_NoWarn))
	{
		UMaterialInstanceDynamic* LampMat = UMaterialInstanceDynamic::Create(Base, this);
		LampMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.92f, 0.75f));
		LampMat->SetScalarParameterValue(TEXT("Strength"), 30.f);
		Materials.Add(LampMat);
		Lamp = LampMat;
		UMaterialInstanceDynamic* TailMat = UMaterialInstanceDynamic::Create(Base, this);
		TailMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.02f, 0.01f));
		TailMat->SetScalarParameterValue(TEXT("Strength"), 12.f);
		Materials.Add(TailMat);
		Tail = TailMat;
	}

	// the floor is 92 cm below the capsule centre
	const float F = -92.f;
	AddPart(Cylinder, FVector(74.f, 0.f, F + 33.f), FVector(66.f, 66.f, 12.f), Tyre, FRotator(0.f, 0.f, 90.f));
	AddPart(Cylinder, FVector(-68.f, 0.f, F + 33.f), FVector(66.f, 66.f, 14.f), Tyre, FRotator(0.f, 0.f, 90.f));
	AddPart(Cube, FVector(0.f, 0.f, F + 62.f), FVector(120.f, 26.f, 34.f), Frame);
	AddPart(Cube, FVector(18.f, 0.f, F + 86.f), FVector(52.f, 30.f, 20.f), Frame);
	AddPart(Cube, FVector(-32.f, 0.f, F + 88.f), FVector(70.f, 26.f, 8.f), Tyre);
	AddPart(Cube, FVector(64.f, 0.f, F + 64.f), FVector(7.f, 7.f, 62.f), Chrome, FRotator(-24.f, 0.f, 0.f));
	AddPart(Cube, FVector(52.f, 0.f, F + 104.f), FVector(6.f, 72.f, 5.f), Chrome);
	AddPart(Cylinder, FVector(76.f, 0.f, F + 92.f), FVector(18.f, 18.f, 8.f), Lamp, FRotator(90.f, 0.f, 0.f));
	AddPart(Cube, FVector(-92.f, 0.f, F + 78.f), FVector(4.f, 12.f, 5.f), Tail);

	// the rider, leaning forward
	AddPart(Cube, FVector(-18.f, 0.f, F + 126.f), FVector(34.f, 44.f, 62.f), Uniform, FRotator(-18.f, 0.f, 0.f));
	AddPart(Sphere, FVector(-6.f, 0.f, F + 170.f), FVector(29.f, 27.f, 27.f), Helmet);
	AddPart(Cube, FVector(18.f, 18.f, F + 126.f), FVector(54.f, 9.f, 9.f), Uniform, FRotator(-20.f, 0.f, 0.f));
	AddPart(Cube, FVector(18.f, -18.f, F + 126.f), FVector(54.f, 9.f, 9.f), Uniform, FRotator(-20.f, 0.f, 0.f));
	AddPart(Cube, FVector(-4.f, 15.f, F + 84.f), FVector(48.f, 12.f, 12.f), Uniform, FRotator(-30.f, 0.f, 0.f));
	AddPart(Cube, FVector(-4.f, -15.f, F + 84.f), FVector(48.f, 12.f, 12.f), Uniform, FRotator(-30.f, 0.f, 0.f));

	// the passenger with a baton
	AddPart(Cube, FVector(-56.f, 0.f, F + 130.f), FVector(32.f, 42.f, 60.f), Uniform, FRotator(-6.f, 0.f, 0.f));
	AddPart(Sphere, FVector(-52.f, 0.f, F + 174.f), FVector(28.f, 26.f, 26.f), Helmet);
	AddPart(Cube, FVector(-44.f, 24.f, F + 156.f), FVector(8.f, 8.f, 72.f), Frame, FRotator(-35.f, 0.f, 0.f));

	TailLight = NewObject<UPointLightComponent>(this);
	TailLight->SetupAttachment(RootComponent);
	TailLight->SetRelativeLocation(FVector(-100.f, 0.f, F + 78.f));
	TailLight->SetIntensityUnits(ELightUnits::Candelas);
	TailLight->Intensity = 6.f;
	TailLight->LightColor = FColor(255, 10, 6);
	TailLight->AttenuationRadius = 260.f;
	TailLight->CastShadows = false;
	TailLight->RegisterComponent();

	// the headlight is the "flashlight"
	Flashlight->SetRelativeLocationAndRotation(FVector(84.f, 0.f, F + 92.f), FRotator(-5.f, 0.f, 0.f));
	Flashlight->SetIntensity(1300.f);
	Flashlight->SetLightColor(FLinearColor(1.f, 0.93f, 0.78f));
	Flashlight->SetAttenuationRadius(2800.f);
	Flashlight->SetOuterConeAngle(26.f);
	Flashlight->SetInnerConeAngle(12.f);
	Cone->SetRelativeLocation(FVector(40.f, 0.f, -74.f));
}

void ACHGuard::ResetGuard()
{
	SetActorLocationAndRotation(StartTransform.GetLocation(), StartTransform.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
	GetCharacterMovement()->StopMovementImmediately();
	Suspicion = 0.f;
	LostTime = 0.f;
	bSeesPlayer = false;
	RouteIndex = 0;
	WaitTime = 0.f;
	Path.Reset();
	State = ECHGuardState::Patrol;
	StateTime = 0.f;
	if (Kind == ECHGuardKind::Plainclothes && bRevealed)
	{
		Reveal(false);
	}
}

void ACHGuard::Stun(float Seconds)
{
	SetState(ECHGuardState::Stunned);
	StunTime = Seconds;
	GetCharacterMovement()->StopMovementImmediately();
}

void ACHGuard::HearNoise(const FVector& Where, float Amount)
{
	if (State == ECHGuardState::Chase || State == ECHGuardState::Stunned)
	{
		return;
	}
	LastKnown = Where;
	LostTime = 0.f;
	Suspicion = FMath::Min(0.85f, Suspicion + Amount);
	if (State == ECHGuardState::Patrol || State == ECHGuardState::Return)
	{
		SetState(ECHGuardState::Suspicious);
	}
}

void ACHGuard::Reveal(bool bInRevealed)
{
	bRevealed = bInRevealed;
	if (Kind != ECHGuardKind::Plainclothes)
	{
		return;
	}
	if (bRevealed)
	{
		CHLooks::Tint(this, GetMesh(), FLinearColor(0.012f, 0.012f, 0.014f), FLinearColor(0.02f, 0.02f, 0.022f), Materials);
	}
	else
	{
		CHLooks::Apply(this, GetMesh(), Look, Seed, Shapes, Materials);
	}
}

void ACHGuard::SetState(ECHGuardState NewState)
{
	if (State == NewState)
	{
		return;
	}
	State = NewState;
	StateTime = 0.f;
	WaitTime = 0.f;
	Path.Reset();
	RepathTime = 0.f;

	switch (NewState)
	{
	case ECHGuardState::Chase:
		Suspicion = 1.f;
		if (Kind == ECHGuardKind::Plainclothes && !bRevealed)
		{
			Reveal(true);
		}
		break;
	case ECHGuardState::Search:
		Suspicion = FMath::Min(Suspicion, 0.6f);
		break;
	case ECHGuardState::Return:
	{
		// back to the nearest point of the route
		float Best = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index < Route.Num(); ++Index)
		{
			const float Distance = FVector::Dist2D(Route[Index], GetActorLocation());
			if (Distance < Best)
			{
				Best = Distance;
				RouteIndex = Index;
			}
		}
		break;
	}
	default:
		break;
	}
}

bool ACHGuard::CanSee(const APawn* Player) const
{
	const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, EyeHeight);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CHGuardSight), false, this);
	Params.AddIgnoredActor(Player);
	for (const float Z : { 58.f, 5.f, -50.f })
	{
		const FVector Target = Player->GetActorLocation() + FVector(0.f, 0.f, Z);
		if (!GetWorld()->LineTraceTestByChannel(Eye, Target, ECC_Visibility, Params))
		{
			return true;
		}
	}
	return false;
}

void ACHGuard::Perceive(float DeltaSeconds, UCHSegmentSubsystem* Segment)
{
	bSeesPlayer = false;
	APawn* Player = Segment->GetPlayerPawn();
	if (!Player || Segment->IsPlayerUntouchable())
	{
		Suspicion = FMath::Max(0.f, Suspicion - 0.5f * DeltaSeconds);
		LostTime += DeltaSeconds;
		return;
	}

	const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, EyeHeight);
	const FVector PlayerLocation = Player->GetActorLocation();
	const FVector To = PlayerLocation - Eye;
	const float Distance = To.Size2D();
	const float Visibility = Segment->GetPlayerVisibility();
	const float Range = SightRange * FMath::Clamp(0.55f + 0.45f * Visibility, 0.35f, 1.5f);

	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();
	FVector Direction = To;
	Direction.Z = 0.f;
	Direction.Normalize();
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Forward, Direction), -1.f, 1.f)));
	const float Near = NearRange * FMath::Clamp(Visibility, 0.3f, 1.3f);

	const bool bInView = (Distance < Range && Angle < SightHalfAngle) || Distance < Near;
	const bool bHeightOk = FMath::Abs(To.Z) < FMath::Max(450.f, Distance * 0.9f);

	if (bInView && bHeightOk && CanSee(Player))
	{
		bSeesPlayer = true;
		LastKnown = PlayerLocation;
		LostTime = 0.f;
		const float Closeness = 1.f - FMath::Clamp(Distance / FMath::Max(Range, 1.f), 0.f, 1.f);
		const float Rate = (0.3f + 1.7f * Closeness * Closeness) * FMath::Max(Visibility, 0.05f) / NoticeTime;
		Suspicion = FMath::Min(1.f, Suspicion + Rate * DeltaSeconds);
		if (Kind == ECHGuardKind::Plainclothes && !bRevealed && (Distance < 650.f || Suspicion > 0.3f))
		{
			Reveal(true);
		}
	}
	else
	{
		LostTime += DeltaSeconds;
		if (State != ECHGuardState::Chase)
		{
			Suspicion = FMath::Max(0.f, Suspicion - 0.2f * DeltaSeconds);
		}
	}
}

bool ACHGuard::MoveTo(const FVector& Target, float Speed, float DeltaSeconds, float RepathInterval)
{
	const FVector Location = GetActorLocation();
	const bool bBike = Kind == ECHGuardKind::Motorcycle;
	const float Arrive = bBike ? 170.f : 55.f;
	if (FVector::Dist2D(Location, Target) < Arrive)
	{
		Path.Reset();
		return true;
	}

	RepathTime -= DeltaSeconds;
	if (Path.Num() == 0 || RepathTime <= 0.f || FVector::Dist2D(PathGoal, Target) > 150.f)
	{
		PathGoal = Target;
		RepathTime = RepathInterval;
		PathIndex = 0;
		Path.Reset();
		const UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
		const bool bGrid = Segment && Segment->GetGrid().IsValid();
		if (!bGrid || Segment->GetGrid().LineWalkable(Location, Target) || !Segment->GetGrid().FindPath(Location, Target, Path))
		{
			Path.Reset();
			Path.Add(FVector(Target.X, Target.Y, 0.f));
		}
	}

	const float Reach = bBike ? 160.f : 45.f;
	while (Path.IsValidIndex(PathIndex) && FVector::Dist2D(Location, Path[PathIndex]) < Reach && PathIndex < Path.Num() - 1)
	{
		++PathIndex;
	}
	if (!Path.IsValidIndex(PathIndex))
	{
		Path.Reset();
		return false;
	}

	FVector Direction = Path[PathIndex] - Location;
	Direction.Z = 0.f;
	if (Direction.IsNearlyZero())
	{
		return false;
	}
	Direction.Normalize();
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	AddMovementInput(Direction, 1.f);
	return false;
}

void ACHGuard::FaceTowards(const FVector& Target, float DeltaSeconds, float DegreesPerSecond)
{
	FVector To = Target - GetActorLocation();
	To.Z = 0.f;
	if (To.IsNearlyZero())
	{
		return;
	}
	const FRotator Desired(0.f, To.Rotation().Yaw, 0.f);
	SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), Desired, DeltaSeconds, DegreesPerSecond));
}

void ACHGuard::Think(float DeltaSeconds, UCHSegmentSubsystem* Segment, bool bLive)
{
	StateTime += DeltaSeconds;

	switch (State)
	{
	case ECHGuardState::Patrol:
	case ECHGuardState::Return:
	{
		if (bLive && Suspicion >= 1.f)
		{
			SetState(ECHGuardState::Chase);
			return;
		}
		if (bLive && bSeesPlayer && Suspicion > 0.12f)
		{
			SetState(ECHGuardState::Suspicious);
			return;
		}
		if (Route.Num() == 0)
		{
			return;
		}
		if (Kind == ECHGuardKind::Watcher || Route.Num() == 1)
		{
			if (MoveTo(Route[0], WalkSpeed, DeltaSeconds))
			{
				State = ECHGuardState::Patrol;
				if (bScans)
				{
					const float Alpha = 0.5f + 0.5f * FMath::Sin(Time * UE_TWO_PI / ScanPeriod);
					const float Yaw = FMath::Lerp(ScanYawA, ScanYawB, Alpha);
					SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), FRotator(0.f, Yaw, 0.f), DeltaSeconds, 90.f));
				}
			}
			return;
		}
		if (WaitTime > 0.f)
		{
			// a slow look around while standing
			WaitTime -= DeltaSeconds;
			AddActorWorldRotation(FRotator(0.f, FMath::Sin(Time * 1.1f) * 35.f * DeltaSeconds, 0.f));
			return;
		}
		if (MoveTo(Route[RouteIndex], WalkSpeed, DeltaSeconds))
		{
			RouteIndex = (RouteIndex + 1) % Route.Num();
			WaitTime = WaitAtPoint;
			State = ECHGuardState::Patrol;
		}
		return;
	}

	case ECHGuardState::Suspicious:
		FaceTowards(LastKnown, DeltaSeconds, 200.f);
		if (Suspicion >= 1.f && bLive)
		{
			SetState(ECHGuardState::Chase);
		}
		else if (!bSeesPlayer && LostTime > 1.4f)
		{
			SetState(Suspicion > 0.3f ? ECHGuardState::Search : ECHGuardState::Return);
		}
		else if (Suspicion <= 0.f)
		{
			SetState(ECHGuardState::Return);
		}
		return;

	case ECHGuardState::Chase:
	{
		APawn* Player = Segment ? Segment->GetPlayerPawn() : nullptr;
		if (!bLive || !Player)
		{
			SetState(ECHGuardState::Return);
			return;
		}
		const FVector Target = bSeesPlayer ? Player->GetActorLocation() : LastKnown;
		MoveTo(Target, ChaseSpeed, DeltaSeconds, 0.3f);

		const FVector PlayerLocation = Player->GetActorLocation();
		if (!Segment->IsPlayerUntouchable()
			&& FVector::Dist2D(PlayerLocation, GetActorLocation()) < CatchRadius
			&& FMath::Abs(PlayerLocation.Z - GetActorLocation().Z) < 140.f
			&& CanSee(Player))
		{
			Segment->OnCaught(this);
			return;
		}
		if (!bSeesPlayer && LostTime > 3.5f)
		{
			SetState(ECHGuardState::Search);
		}
		return;
	}

	case ECHGuardState::Search:
		if (bLive && bSeesPlayer && Suspicion >= 1.f)
		{
			SetState(ECHGuardState::Chase);
			return;
		}
		if (WaitTime > 0.f || MoveTo(LastKnown, WalkSpeed * 1.7f, DeltaSeconds) || StateTime > 8.f)
		{
			// arrived: look around for a while, then go back
			WaitTime += DeltaSeconds;
			AddActorWorldRotation(FRotator(0.f, FMath::Sin(Time * 1.4f) * 95.f * DeltaSeconds, 0.f));
			if (WaitTime > 4.f)
			{
				SetState(ECHGuardState::Return);
			}
		}
		return;

	default:
		return;
	}
}

void ACHGuard::UpdateVisuals(float DeltaSeconds)
{
	const bool bShowCone = bRevealed && State != ECHGuardState::Stunned;

	FLinearColor Color = CalmCone;
	float Opacity = 0.2f;
	if (State == ECHGuardState::Chase)
	{
		Color = ChaseCone;
		Opacity = 0.42f;
	}
	else if (Suspicion > 0.02f)
	{
		const float Alpha = FMath::Clamp(Suspicion * 1.6f, 0.f, 1.f);
		Color = FMath::Lerp(CalmCone, WaryCone, Alpha);
		Opacity = FMath::Lerp(0.2f, 0.36f, Suspicion);
	}
	if (Kind == ECHGuardKind::Motorcycle)
	{
		Opacity *= 0.7f;
	}
	Cone->SetVisibility(bShowCone);
	if (bShowCone)
	{
		Cone->Refresh(Color, Opacity);
	}

	Flashlight->SetVisibility(bHasLight && bRevealed);

	const bool bAlert = State == ECHGuardState::Chase || Suspicion > 0.05f || State == ECHGuardState::Search;
	AlertMark->SetVisibility(bAlert && bRevealed);
	if (bAlert && AlertMaterial)
	{
		const bool bRed = State == ECHGuardState::Chase;
		AlertMaterial->SetVectorParameterValue(TEXT("Color"), bRed ? ChaseCone : WaryCone);
		AlertMaterial->SetScalarParameterValue(TEXT("Strength"), CHLight::Emissive(3.f));
		const float Pulse = 0.10f + 0.03f * FMath::Sin(Time * (bRed ? 14.f : 6.f));
		AlertMark->SetRelativeScale3D(FVector(Pulse));
		AlertMark->AddRelativeRotation(FRotator(0.f, 120.f * DeltaSeconds, 0.f));
	}
}

void ACHGuard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Time += DeltaSeconds;

	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	const bool bLive = Segment && Segment->IsActive();

	if (State == ECHGuardState::Stunned)
	{
		StunTime -= DeltaSeconds;
		if (StunTime <= 0.f)
		{
			SetState(ECHGuardState::Search);
		}
		UpdateVisuals(DeltaSeconds);
		return;
	}

	if (bLive)
	{
		Perceive(DeltaSeconds, Segment);
	}
	else
	{
		bSeesPlayer = false;
		Suspicion = FMath::Max(0.f, Suspicion - DeltaSeconds);
		if (State != ECHGuardState::Patrol && State != ECHGuardState::Return)
		{
			SetState(ECHGuardState::Return);
		}
	}

	Think(DeltaSeconds, Segment, bLive);
	UpdateVisuals(DeltaSeconds);
}
