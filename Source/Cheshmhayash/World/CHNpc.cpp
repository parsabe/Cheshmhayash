// Cheshm Hayash - a person in the world who is not the player.

#include "World/CHNpc.h"
#include "Animation/AnimSequence.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "World/CHLooks.h"

namespace
{
	void RotateBoneCS(UPoseableMeshComponent* PoseMesh, FName Bone, const FQuat& DeltaCS)
	{
		if (!PoseMesh)
		{
			return;
		}
		const FRotator CurRot = PoseMesh->GetBoneRotationByName(Bone, EBoneSpaces::ComponentSpace);
		const FQuat NextQuat = DeltaCS * CurRot.Quaternion();
		PoseMesh->SetBoneRotationByName(Bone, NextQuat.Rotator(), EBoneSpaces::ComponentSpace);
	}
}

ACHNpc::ACHNpc()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	// Hidden skeletal mesh drives walk/jog/idle/fall animations
	AnimBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AnimBody"));
	AnimBody->SetupAttachment(SceneRoot);
	AnimBody->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	AnimBody->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	AnimBody->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	AnimBody->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	AnimBody->SetHiddenInGame(true);
	AnimBody->SetCastShadow(false);

	// Visible poseable mesh holds CHLooks clothing/hair and procedural bone poses
	Body = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(SceneRoot);
	Body->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	Body->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

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
		AnimBody->SetSkeletalMeshAsset(MannyFinder.Object);
		Body->SetSkinnedAssetAndUpdate(MannyFinder.Object);
	}
}

USkinnedMeshComponent* ACHNpc::GetBody() const
{
	return Body;
}

void ACHNpc::SetLook(const FString& InLook, int32 Seed)
{
	Look = InLook;
	PoseTime = FMath::Abs(Seed * 1.37f);
	CHLooks::Apply(this, Body, InLook, Seed, Shapes, Materials);
	if (USkeletalMesh* SkelAsset = Cast<USkeletalMesh>(Body->GetSkinnedAsset()))
	{
		if (AnimBody->GetSkeletalMeshAsset() != SkelAsset)
		{
			AnimBody->SetSkeletalMeshAsset(SkelAsset);
		}
	}
	SetActorScale3D(CHLooks::Scale3D(InLook, Seed));

	// a new mesh asset resets the animation
	CurrentAnim = nullptr;
	PlayLoop(bMoving ? WalkAnim : IdleAnim);
	Body->CopyPoseFromSkeletalComponent(AnimBody);
	UpdateProps();
	ApplyProceduralPose(0.f);
}

void ACHNpc::SetPose(const FString& InPose)
{
	Pose = InPose;
	if (Pose == TEXT("sit_chair") || Pose == TEXT("sit_desk"))
	{
		PoseElevation = -36.f;
	}
	else if (Pose == TEXT("sit_sofa"))
	{
		PoseElevation = -40.f;
	}
	else if (Pose == TEXT("sit_stairs") || Pose == TEXT("sit_floor"))
	{
		PoseElevation = -54.f;
	}
	else
	{
		PoseElevation = -Sink;
	}
	if (Pose == TEXT("cry"))
	{
		bShaking = true;
	}
	Body->SetRelativeLocation(FVector(0.f, 0.f, PoseElevation));
	UpdateProps();
	ApplyProceduralPose(0.f);
}

void ACHNpc::UpdateProps()
{
	for (UStaticMeshComponent* Prop : PropShapes)
	{
		if (Prop)
		{
			Prop->DestroyComponent();
		}
	}
	PropShapes.Reset();

	if (!Body)
	{
		return;
	}

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"), nullptr, LOAD_NoWarn);
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), nullptr, LOAD_NoWarn);

	auto AddProp = [&](UStaticMesh* Mesh, FName Bone, const FVector& RelLoc, const FRotator& RelRot, const FVector& SizeCm, const FLinearColor& Color)
	{
		if (!Mesh)
		{
			return;
		}
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetStaticMesh(Mesh);
		if (UMaterialInstanceDynamic* Mat = CHLooks::MakeLit(this, Color, 0.5f))
		{
			Comp->SetMaterial(0, Mat);
			Materials.Add(Mat);
		}
		Comp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Comp->SetCanEverAffectNavigation(false);
		Comp->SetupAttachment(Body, Bone);
		Comp->SetRelativeLocationAndRotation(RelLoc, RelRot);
		Comp->SetRelativeScale3D(SizeCm / 100.0);
		Comp->RegisterComponent();
		PropShapes.Add(Comp);
	};

	if (Pose == TEXT("smoke"))
	{
		// Small white cigarette + glowing orange tip in right hand
		AddProp(Cylinder, TEXT("hand_r"), FVector(4.f, 8.f, 2.f), FRotator(90.f, 0.f, 0.f), FVector(1.0f, 1.0f, 7.5f), FLinearColor(0.85f, 0.83f, 0.78f));
		AddProp(Cylinder, TEXT("hand_r"), FVector(4.f, 12.2f, 2.f), FRotator(90.f, 0.f, 0.f), FVector(1.1f, 1.1f, 1.4f), FLinearColor(0.95f, 0.28f, 0.04f));
	}
	else if (Pose == TEXT("phone") || Pose == TEXT("sit_desk"))
	{
		// Held smartphone with softly lit screen in right hand
		AddProp(Cube, TEXT("hand_r"), FVector(2.f, 9.f, 3.f), FRotator(15.f, 20.f, 0.f), FVector(8.f, 15.f, 1.2f), FLinearColor(0.03f, 0.03f, 0.04f));
		AddProp(Cube, TEXT("hand_r"), FVector(2.f, 9.f, 3.8f), FRotator(15.f, 20.f, 0.f), FVector(7.f, 13.5f, 0.4f), FLinearColor(0.35f, 0.48f, 0.65f));
	}
	else if (Pose == TEXT("pour_tea"))
	{
		// Small porcelain/metal teapot held in right hand
		AddProp(Cylinder, TEXT("hand_r"), FVector(0.f, 12.f, -2.f), FRotator::ZeroRotator, FVector(13.f, 13.f, 15.f), FLinearColor(0.55f, 0.52f, 0.48f));
	}
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
		AnimBody->PlayAnimation(Sequence, true);
	}
	AnimBody->SetPlayRate(Rate);
}

void ACHNpc::WalkTo(const FVector& Target, float Speed)
{
	MoveTarget = Target;
	MoveSpeed = FMath::Max(10.f, Speed);
	bMoving = true;
	Body->SetRelativeLocation(FVector::ZeroVector);
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
	Body->SetRelativeLocation(FVector(0.f, 0.f, PoseElevation));
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
	if (Centimetres <= 1.f)
	{
		if (Pose.StartsWith(TEXT("sit_")))
		{
			SetPose(TEXT(""));
		}
		else
		{
			PoseElevation = 0.f;
			Body->SetRelativeLocation(FVector::ZeroVector);
		}
	}
	else if (Pose.IsEmpty())
	{
		// Automatically map legacy sink values to real seated poses!
		if (Centimetres >= 50.f)
		{
			SetPose(TEXT("sit_stairs"));
		}
		else if (Centimetres >= 40.f)
		{
			SetPose(TEXT("sit_sofa"));
		}
		else
		{
			SetPose(TEXT("sit_chair"));
		}
	}
}

void ACHNpc::PlayFall(bool bRagdoll)
{
	bMoving = false;
	Pose.Empty();
	PoseElevation = 0.f;
	Body->SetRelativeLocation(FVector::ZeroVector);
	if (FallAnim)
	{
		CurrentAnim = FallAnim;
		AnimBody->PlayAnimation(FallAnim, false);
		AnimBody->SetPlayRate(1.f);
	}
	RagdollTimer = bRagdoll ? (FallAnim ? FallAnim->GetPlayLength() * 0.85f : 0.01f) : -1.f;
}

void ACHNpc::StartRagdoll()
{
	RagdollTimer = -1.f;
	bRagdolling = true;
	AnimBody->SetCollisionProfileName(TEXT("Ragdoll"));
	AnimBody->SetAllBodiesSimulatePhysics(true);
	AnimBody->SetSimulatePhysics(true);
	AnimBody->WakeAllRigidBodies();
	AnimBody->bBlendPhysics = true;
}

void ACHNpc::Freeze()
{
	bFrozen = true;
	bMoving = false;
	AnimBody->bPauseAnims = true;
}

void ACHNpc::ApplyProceduralPose(float DeltaSeconds)
{
	if (!Body || bMoving || bRagdolling || CurrentAnim == FallAnim)
	{
		return;
	}

	PoseTime += DeltaSeconds;

	// In Body's component space: +Y is character Forward, -X is character Right, +Z is character Up
	const FVector Fwd(0.f, 1.f, 0.f);
	const FVector Up(0.f, 0.f, 1.f);
	const FVector Down(0.f, 0.f, -1.f);

	// Subtle breathing and head micro-motion on all non-frozen characters
	const float Breath = FMath::Sin(PoseTime * 1.9f) * 0.025f;
	const float HeadSwayX = FMath::Sin(PoseTime * 0.75f) * 0.05f;
	const float HeadSwayY = FMath::Cos(PoseTime * 0.55f) * 0.03f;
	RotateBoneCS(Body, TEXT("spine_02"), FQuat::FindBetweenNormals(Up, FVector(0.f, Breath, 1.f).GetSafeNormal()));
	RotateBoneCS(Body, TEXT("head"), FQuat::FindBetweenNormals(Up, FVector(HeadSwayX, HeadSwayY, 1.f).GetSafeNormal()));

	if (Pose == TEXT("sit_chair") || Pose == TEXT("sit_desk"))
	{
		// Thighs horizontal forward on chair seat, calves vertical down to floor
		RotateBoneCS(Body, TEXT("thigh_l"), FQuat::FindBetweenNormals(Down, FVector(0.08f, 0.98f, -0.06f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("thigh_r"), FQuat::FindBetweenNormals(Down, FVector(-0.08f, 0.98f, -0.06f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("calf_l"), FQuat::FindBetweenNormals(Fwd, FVector(0.f, -0.08f, -1.f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("calf_r"), FQuat::FindBetweenNormals(Fwd, FVector(0.f, -0.08f, -1.f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("foot_l"), FQuat::FindBetweenNormals(Down, Fwd));
		RotateBoneCS(Body, TEXT("foot_r"), FQuat::FindBetweenNormals(Down, Fwd));

		// Slight forward lean at desk + forearms resting forward on lap/desk
		RotateBoneCS(Body, TEXT("spine_03"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.14f, 0.99f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_l"), FQuat::FindBetweenNormals(Down, FVector(0.08f, 0.42f, -0.90f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.08f, 0.42f, -0.90f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_l"), FQuat::FindBetweenNormals(Down, FVector(-0.22f, 0.92f, -0.10f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.22f, 0.92f, -0.10f).GetSafeNormal()));
	}
	else if (Pose == TEXT("sit_sofa"))
	{
		// Relaxed seated posture on sofa/couch
		RotateBoneCS(Body, TEXT("spine_01"), FQuat::FindBetweenNormals(Up, FVector(0.f, -0.12f, 0.99f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("thigh_l"), FQuat::FindBetweenNormals(Down, FVector(0.12f, 0.98f, -0.04f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("thigh_r"), FQuat::FindBetweenNormals(Down, FVector(-0.12f, 0.98f, -0.04f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("calf_l"), FQuat::FindBetweenNormals(Fwd, FVector(0.f, -0.05f, -1.f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("calf_r"), FQuat::FindBetweenNormals(Fwd, FVector(0.f, -0.05f, -1.f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("foot_l"), FQuat::FindBetweenNormals(Down, Fwd));
		RotateBoneCS(Body, TEXT("foot_r"), FQuat::FindBetweenNormals(Down, Fwd));

		RotateBoneCS(Body, TEXT("upperarm_l"), FQuat::FindBetweenNormals(Down, FVector(0.10f, 0.30f, -0.95f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.10f, 0.30f, -0.95f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_l"), FQuat::FindBetweenNormals(Down, FVector(-0.28f, 0.88f, -0.25f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.28f, 0.88f, -0.25f).GetSafeNormal()));
	}
	else if (Pose == TEXT("sit_stairs") || Pose == TEXT("sit_floor"))
	{
		// Hunched forward sitting on steps/ground with knees bent up and elbows on knees
		RotateBoneCS(Body, TEXT("spine_01"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.32f, 0.95f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("spine_03"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.25f, 0.97f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("head"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.35f, 0.94f).GetSafeNormal()));

		RotateBoneCS(Body, TEXT("thigh_l"), FQuat::FindBetweenNormals(Down, FVector(0.14f, 0.90f, 0.38f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("thigh_r"), FQuat::FindBetweenNormals(Down, FVector(-0.14f, 0.90f, 0.38f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("calf_l"), FQuat::FindBetweenNormals(FVector(0.f, 0.90f, 0.38f).GetSafeNormal(), FVector(0.f, -0.20f, -0.98f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("calf_r"), FQuat::FindBetweenNormals(FVector(0.f, 0.90f, 0.38f).GetSafeNormal(), FVector(0.f, -0.20f, -0.98f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("foot_l"), FQuat::FindBetweenNormals(Down, Fwd));
		RotateBoneCS(Body, TEXT("foot_r"), FQuat::FindBetweenNormals(Down, Fwd));

		RotateBoneCS(Body, TEXT("upperarm_l"), FQuat::FindBetweenNormals(Down, FVector(0.05f, 0.55f, -0.83f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.05f, 0.55f, -0.83f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_l"), FQuat::FindBetweenNormals(Down, FVector(-0.30f, 0.85f, 0.35f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.30f, 0.85f, 0.35f).GetSafeNormal()));
	}
	else if (Pose == TEXT("smoke"))
	{
		// Periodic smoking gesture: raises right hand to mouth every ~7 seconds
		const float Cycle = FMath::Fmod(PoseTime, 7.2f);
		float Puff = 0.f;
		if (Cycle < 2.2f)
		{
			Puff = FMath::SmoothStep(0.f, 0.6f, Cycle) * (1.f - FMath::SmoothStep(1.6f, 2.2f, Cycle));
		}
		const FVector ArmDir = FMath::Lerp(FVector(-0.08f, 0.18f, -0.98f), FVector(-0.05f, 0.62f, -0.78f), Puff).GetSafeNormal();
		const FVector ForearmDir = FMath::Lerp(FVector(0.15f, 0.55f, -0.82f), FVector(0.28f, 0.45f, 0.85f), Puff).GetSafeNormal();
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, ArmDir));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, ForearmDir));
		RotateBoneCS(Body, TEXT("head"), FQuat::FindBetweenNormals(Up, FVector(-0.06f * Puff, 0.14f * Puff, 0.99f).GetSafeNormal()));
	}
	else if (Pose == TEXT("phone"))
	{
		// Holding phone in right hand and looking down at screen
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.05f, 0.42f, -0.90f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.32f, 0.82f, 0.45f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("neck_01"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.25f, 0.97f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("head"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.32f, 0.95f).GetSafeNormal()));
	}
	else if (Pose == TEXT("cry"))
	{
		// Both hands raised covering face, head bowed
		RotateBoneCS(Body, TEXT("spine_03"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.22f, 0.97f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("head"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.38f, 0.92f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_l"), FQuat::FindBetweenNormals(Down, FVector(0.05f, 0.65f, -0.75f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.05f, 0.65f, -0.75f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_l"), FQuat::FindBetweenNormals(Down, FVector(-0.28f, 0.40f, 0.87f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.28f, 0.40f, 0.87f).GetSafeNormal()));
	}
	else if (Pose == TEXT("hug"))
	{
		// Arms wrapped forward in an embrace
		RotateBoneCS(Body, TEXT("spine_03"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.15f, 0.99f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("head"), FQuat::FindBetweenNormals(Up, FVector(0.18f, 0.16f, 0.97f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_l"), FQuat::FindBetweenNormals(Down, FVector(0.12f, 0.85f, -0.50f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.12f, 0.85f, -0.50f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_l"), FQuat::FindBetweenNormals(Down, FVector(-0.55f, 0.82f, 0.10f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.55f, 0.82f, 0.10f).GetSafeNormal()));
	}
	else if (Pose == TEXT("pour_tea"))
	{
		// Extending right arm forward and tilting teapot periodically
		const float PourTilt = 0.5f + 0.5f * FMath::Sin(PoseTime * 1.4f);
		RotateBoneCS(Body, TEXT("spine_03"), FQuat::FindBetweenNormals(Up, FVector(0.f, 0.14f, 0.99f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.05f, 0.55f, -0.83f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.10f, 0.95f, -0.15f * PourTilt).GetSafeNormal()));
	}
	else if (Pose == TEXT("talk"))
	{
		// Subtle hand gestures while talking
		const float G1 = 0.25f + 0.2f * FMath::Sin(PoseTime * 2.6f);
		const float G2 = 0.20f + 0.18f * FMath::Cos(PoseTime * 2.1f);
		RotateBoneCS(Body, TEXT("upperarm_r"), FQuat::FindBetweenNormals(Down, FVector(-0.08f, G1, -0.95f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_r"), FQuat::FindBetweenNormals(Down, FVector(0.18f, 0.65f + G1, -0.45f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("upperarm_l"), FQuat::FindBetweenNormals(Down, FVector(0.08f, G2, -0.96f).GetSafeNormal()));
		RotateBoneCS(Body, TEXT("lowerarm_l"), FQuat::FindBetweenNormals(Down, FVector(-0.18f, 0.55f + G2, -0.55f).GetSafeNormal()));
	}
}

void ACHNpc::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFrozen)
	{
		return;
	}

	if (RagdollTimer >= 0.f)
	{
		RagdollTimer -= DeltaSeconds;
		if (RagdollTimer < 0.f)
		{
			StartRagdoll();
		}
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
			Body->SetRelativeLocation(FVector(0.f, 0.f, PoseElevation));
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

	if (Body && AnimBody)
	{
		Body->CopyPoseFromSkeletalComponent(AnimBody);
		ApplyProceduralPose(DeltaSeconds);
	}

	if (bShaking)
	{
		ShakeTime += DeltaSeconds;
		const float Tremble = FMath::Sin(ShakeTime * 19.f) * 0.5f + FMath::Sin(ShakeTime * 7.3f) * 0.35f;
		const float Sob = FMath::Max(0.f, FMath::Sin(ShakeTime * 2.1f));
		Body->SetRelativeLocation(FVector(0.f, 0.f, PoseElevation + Tremble * Sob));
		Body->SetRelativeRotation(FRotator(0.f, -90.f, Tremble * Sob * 0.8f + Sob * 2.5f));
	}
}
