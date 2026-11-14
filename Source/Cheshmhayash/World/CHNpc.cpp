// Cheshm Hayash - a person in the world who is not the player.

#include "World/CHNpc.h"
#include "Animation/AnimSequence.h"
#include "AnimationRuntime.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/RandomStream.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	enum class EHead : uint8 { None, Scarf, LooseScarf, Chador, Helmet };

	struct FLookDef
	{
		bool bFemale = false;
		FLinearColor Upper = FLinearColor(0.08f, 0.08f, 0.08f);
		FLinearColor Lower = FLinearColor(0.035f, 0.035f, 0.04f);
		EHead Head = EHead::None;
		FLinearColor HeadColor = FLinearColor(0.02f, 0.02f, 0.02f);
		float Scale = 1.f;
	};

	FLookDef MakeLook(const FString& Look, int32 Seed)
	{
		FRandomStream Rng(Seed * 7919 + 13);
		FLookDef Def;

		const FLinearColor Manteaus[] = {
			FLinearColor(0.07f, 0.10f, 0.13f), FLinearColor(0.13f, 0.08f, 0.07f), FLinearColor(0.09f, 0.11f, 0.08f),
			FLinearColor(0.12f, 0.11f, 0.10f), FLinearColor(0.05f, 0.05f, 0.07f), FLinearColor(0.15f, 0.12f, 0.08f) };
		const FLinearColor Scarves[] = {
			FLinearColor(0.30f, 0.16f, 0.14f), FLinearColor(0.10f, 0.13f, 0.20f), FLinearColor(0.34f, 0.30f, 0.24f),
			FLinearColor(0.05f, 0.05f, 0.06f), FLinearColor(0.16f, 0.22f, 0.17f) };
		const FLinearColor Shirts[] = {
			FLinearColor(0.10f, 0.11f, 0.13f), FLinearColor(0.14f, 0.12f, 0.09f), FLinearColor(0.06f, 0.08f, 0.07f),
			FLinearColor(0.16f, 0.15f, 0.14f), FLinearColor(0.09f, 0.06f, 0.05f), FLinearColor(0.05f, 0.06f, 0.09f) };

		if (Look == TEXT("sister"))
		{
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.07f, 0.13f, 0.15f);
			Def.Lower = FLinearColor(0.03f, 0.035f, 0.05f);
			Def.Head = EHead::LooseScarf;
			Def.HeadColor = FLinearColor(0.26f, 0.15f, 0.14f);
		}
		else if (Look == TEXT("sister_after"))
		{
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.09f, 0.09f, 0.10f);
			Def.Lower = FLinearColor(0.03f, 0.03f, 0.035f);
			Def.Head = EHead::Scarf;
			Def.HeadColor = FLinearColor(0.045f, 0.045f, 0.055f);
		}
		else if (Look == TEXT("sister_older"))
		{
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.13f, 0.11f, 0.10f);
			Def.Lower = FLinearColor(0.035f, 0.035f, 0.04f);
		}
		else if (Look == TEXT("brother"))
		{
			Def.Upper = FLinearColor(0.11f, 0.12f, 0.14f);
		}
		else if (Look == TEXT("he"))
		{
			Def.Upper = FLinearColor(0.055f, 0.065f, 0.085f);
		}
		else if (Look == TEXT("kian"))
		{
			Def.Upper = FLinearColor(0.20f, 0.09f, 0.05f);
		}
		else if (Look == TEXT("mother"))
		{
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.16f, 0.12f, 0.12f);
			Def.Head = EHead::LooseScarf;
			Def.HeadColor = FLinearColor(0.50f, 0.45f, 0.38f);
		}
		else if (Look == TEXT("father"))
		{
			Def.Upper = FLinearColor(0.14f, 0.12f, 0.09f);
			Def.Lower = FLinearColor(0.05f, 0.045f, 0.04f);
		}
		else if (Look == TEXT("chador") || Look == TEXT("morality_f"))
		{
			Def.bFemale = true;
			Def.Head = EHead::Chador;
			Def.HeadColor = FLinearColor(0.012f, 0.012f, 0.014f);
		}
		else if (Look == TEXT("oldwoman"))
		{
			Def.bFemale = true;
			Def.Head = EHead::Chador;
			Def.HeadColor = FLinearColor(0.018f, 0.018f, 0.02f);
			Def.Scale = 0.9f;
		}
		else if (Look == TEXT("woman") || Look == TEXT("student_f"))
		{
			Def.bFemale = true;
			Def.Upper = Manteaus[Rng.RandRange(0, UE_ARRAY_COUNT(Manteaus) - 1)];
			Def.HeadColor = Scarves[Rng.RandRange(0, UE_ARRAY_COUNT(Scarves) - 1)];
			// by Mehr many students wear no scarf at all
			Def.Head = Look == TEXT("student_f") ? (Rng.FRand() < 0.5f ? EHead::None : EHead::LooseScarf) : (Rng.FRand() < 0.3f ? EHead::LooseScarf : EHead::Scarf);
			Def.Scale = Rng.FRandRange(0.94f, 1.02f);
		}
		else if (Look == TEXT("girl"))
		{
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.15f, 0.09f, 0.06f);
		}
		else if (Look == TEXT("guard"))
		{
			Def.Upper = FLinearColor(0.012f, 0.013f, 0.015f);
			Def.Lower = FLinearColor(0.012f, 0.013f, 0.015f);
			Def.Head = EHead::Helmet;
			Def.HeadColor = FLinearColor(0.015f, 0.016f, 0.018f);
			Def.Scale = 1.04f;
		}
		else if (Look == TEXT("basij"))
		{
			Def.Upper = FLinearColor(0.055f, 0.05f, 0.03f);
			Def.Lower = FLinearColor(0.02f, 0.02f, 0.018f);
			Def.Head = EHead::Helmet;
			Def.HeadColor = FLinearColor(0.02f, 0.02f, 0.02f);
		}
		else if (Look == TEXT("morality_m"))
		{
			Def.Upper = FLinearColor(0.025f, 0.05f, 0.035f);
			Def.Lower = FLinearColor(0.02f, 0.035f, 0.025f);
		}
		else if (Look == TEXT("lawyer"))
		{
			Def.Upper = FLinearColor(0.03f, 0.035f, 0.05f);
			Def.Lower = FLinearColor(0.025f, 0.028f, 0.04f);
		}
		else
		{
			// man, student_m, plain, driver, professor and anything unknown: an ordinary man
			Def.Upper = Shirts[Rng.RandRange(0, UE_ARRAY_COUNT(Shirts) - 1)];
			Def.Lower = FLinearColor(0.03f, 0.03f, 0.035f) * Rng.FRandRange(0.8f, 1.6f);
			Def.Scale = Rng.FRandRange(0.96f, 1.04f);
		}
		return Def;
	}
}

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
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> QuinnFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> JogFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> FallFinder(TEXT("/Game/Characters/Mannequins/Anims/Death/MM_Death_Back_01.MM_Death_Back_01"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	MannyMesh = MannyFinder.Object;
	QuinnMesh = QuinnFinder.Object;
	IdleAnim = IdleFinder.Object;
	WalkAnim = WalkFinder.Object;
	JogAnim = JogFinder.Object;
	FallAnim = FallFinder.Object;
	SphereMesh = SphereFinder.Object;
	ConeMesh = ConeFinder.Object;
	CubeMesh = CubeFinder.Object;

	if (MannyMesh)
	{
		Body->SetSkeletalMeshAsset(MannyMesh);
	}
}

UMaterialInstanceDynamic* ACHNpc::MakeMaterial(const FLinearColor& Color, float Roughness)
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Lit.M_CH_Lit"), nullptr, LOAD_NoWarn);
	if (!Base)
	{
		return nullptr;
	}
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, this);
	Material->SetVectorParameterValue(TEXT("Color"), Color);
	Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
	Material->SetScalarParameterValue(TEXT("Grime"), 0.f);
	Materials.Add(Material);
	return Material;
}

void ACHNpc::SetColors(const FLinearColor& Upper, const FLinearColor& Lower)
{
	// mannequin slot 0 = head, arms and legs; slot 1 = torso
	const int32 NumSlots = Body->GetNumMaterials();
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		if (UMaterialInstanceDynamic* Material = MakeMaterial(Slot == 0 ? Lower : Upper))
		{
			Body->SetMaterial(Slot, Material);
		}
	}
}

UStaticMeshComponent* ACHNpc::AddShape(UStaticMesh* Mesh, const FVector& Center, const FVector& Size, const FLinearColor& Color, FName Bone, const FRotator& Rotation)
{
	UStaticMeshComponent* Shape = NewObject<UStaticMeshComponent>(this);
	Shape->SetMobility(EComponentMobility::Movable);
	Shape->SetStaticMesh(Mesh);
	if (UMaterialInstanceDynamic* Material = MakeMaterial(Color, 0.95f))
	{
		Shape->SetMaterial(0, Material);
	}
	Shape->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Shape->SetupAttachment(GetRootComponent());
	Shape->SetRelativeTransform(FTransform(Rotation, Center, Size / 100.0));
	Shape->RegisterComponent();
	if (Bone != NAME_None && Body->GetBoneIndex(Bone) != INDEX_NONE)
	{
		Shape->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform, Bone);
	}
	Shapes.Add(Shape);
	return Shape;
}

void ACHNpc::ClearShapes()
{
	for (UStaticMeshComponent* Shape : Shapes)
	{
		if (Shape)
		{
			Shape->DestroyComponent();
		}
	}
	Shapes.Reset();
}

void ACHNpc::SetLook(const FString& InLook, int32 Seed)
{
	Look = InLook;
	const FLookDef Def = MakeLook(InLook, Seed);

	ClearShapes();
	Materials.Reset();

	USkeletalMesh* Mesh = Def.bFemale ? QuinnMesh.Get() : MannyMesh.Get();
	if (Mesh)
	{
		Body->SetSkeletalMeshAsset(Mesh);
	}
	SetActorScale3D(FVector(Def.Scale));
	SetColors(Def.Upper, Def.Lower);

	// where the head is, in actor space (feet at the origin, facing +X), from the reference pose
	FVector HeadJoint(0.f, 0.f, 160.f);
	if (Mesh)
	{
		const FReferenceSkeleton& RefSkeleton = Mesh->GetRefSkeleton();
		const int32 HeadIndex = RefSkeleton.FindBoneIndex(TEXT("head"));
		if (HeadIndex != INDEX_NONE)
		{
			const FTransform HeadTransform = FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, HeadIndex);
			HeadJoint = FRotator(0.f, -90.f, 0.f).RotateVector(HeadTransform.GetLocation());
		}
	}
	const FVector HeadCenter = HeadJoint + FVector(2.f, 0.f, 9.f);

	switch (Def.Head)
	{
	case EHead::Scarf:
		// wraps the head, the face shows at the front; the cloth falls over the shoulders
		AddShape(SphereMesh, HeadCenter + FVector(-2.5f, 0.f, 1.5f), FVector(24.f, 23.5f, 26.5f), Def.HeadColor, TEXT("head"));
		AddShape(ConeMesh, HeadCenter + FVector(-2.f, 0.f, -19.f), FVector(34.f, 40.f, 26.f), Def.HeadColor, TEXT("neck_01"));
		break;
	case EHead::LooseScarf:
		// slipping back: the hair at the front shows
		AddShape(SphereMesh, HeadCenter + FVector(-5.5f, 0.f, 1.f), FVector(22.5f, 23.f, 25.f), Def.HeadColor, TEXT("head"));
		AddShape(ConeMesh, HeadCenter + FVector(-4.f, 0.f, -19.f), FVector(30.f, 38.f, 24.f), Def.HeadColor, TEXT("neck_01"));
		break;
	case EHead::Chador:
	{
		AddShape(SphereMesh, HeadCenter + FVector(-3.f, 0.f, 1.f), FVector(26.f, 25.f, 28.f), Def.HeadColor, TEXT("head"));
		const float Top = HeadCenter.Z + 12.f;
		AddShape(ConeMesh, FVector(-2.f, 0.f, Top * 0.5f), FVector(84.f, 84.f, Top), Def.HeadColor);
		break;
	}
	case EHead::Helmet:
		AddShape(SphereMesh, HeadCenter + FVector(0.f, 0.f, 3.f), FVector(27.f, 26.f, 24.f), Def.HeadColor, TEXT("head"));
		break;
	default:
		break;
	}

	PlayLoop(IdleAnim);
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
