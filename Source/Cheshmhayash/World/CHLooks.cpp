// Cheshm Hayash - how people look.

#include "World/CHLooks.h"
#include "AnimationRuntime.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/RandomStream.h"

namespace
{
	enum class EHead : uint8 { None, Scarf, LooseScarf, Chador, Helmet, Cap };

	struct FLookDef
	{
		bool bFemale = false;
		FLinearColor Upper = FLinearColor(0.08f, 0.08f, 0.08f);
		FLinearColor Lower = FLinearColor(0.035f, 0.035f, 0.04f);
		EHead Head = EHead::None;
		FLinearColor HeadColor = FLinearColor(0.02f, 0.02f, 0.02f);
		float Scale = 1.f;
	};

	template <typename T>
	T* LoadAsset(const TCHAR* Path)
	{
		return LoadObject<T>(nullptr, Path, nullptr, LOAD_NoWarn);
	}

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
			// thin, hair short, no scarf
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.09f, 0.09f, 0.10f);
			Def.Lower = FLinearColor(0.03f, 0.03f, 0.035f);
			Def.Scale = 0.97f;
		}
		else if (Look == TEXT("sister_older"))
		{
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.13f, 0.11f, 0.10f);
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
			Def.HeadColor = FLinearColor(0.010f, 0.010f, 0.012f);
			Def.Upper = Def.Lower = FLinearColor(0.012f, 0.012f, 0.014f);
		}
		else if (Look == TEXT("oldwoman"))
		{
			Def.bFemale = true;
			Def.Head = EHead::Chador;
			Def.HeadColor = FLinearColor(0.018f, 0.018f, 0.02f);
			Def.Upper = Def.Lower = FLinearColor(0.02f, 0.02f, 0.022f);
			Def.Scale = 0.92f;
		}
		else if (Look == TEXT("woman") || Look == TEXT("student_f"))
		{
			Def.bFemale = true;
			Def.Upper = Manteaus[Rng.RandRange(0, UE_ARRAY_COUNT(Manteaus) - 1)];
			Def.HeadColor = Scarves[Rng.RandRange(0, UE_ARRAY_COUNT(Scarves) - 1)];
			// by Mehr many young women walk without a scarf
			Def.Head = Look == TEXT("student_f")
				? (Rng.FRand() < 0.55f ? EHead::None : EHead::LooseScarf)
				: (Rng.FRand() < 0.35f ? EHead::LooseScarf : EHead::Scarf);
			Def.Scale = Rng.FRandRange(0.95f, 1.02f);
		}
		else if (Look == TEXT("girl"))
		{
			Def.bFemale = true;
			Def.Upper = FLinearColor(0.15f, 0.09f, 0.06f);
		}
		else if (Look == TEXT("guard"))
		{
			Def.Upper = FLinearColor(0.012f, 0.013f, 0.016f);
			Def.Lower = FLinearColor(0.010f, 0.011f, 0.013f);
			Def.Head = EHead::Helmet;
			Def.HeadColor = FLinearColor(0.015f, 0.016f, 0.018f);
		}
		else if (Look == TEXT("basij"))
		{
			Def.Upper = FLinearColor(0.050f, 0.045f, 0.028f);
			Def.Lower = FLinearColor(0.020f, 0.020f, 0.018f);
			Def.Head = EHead::Helmet;
			Def.HeadColor = FLinearColor(0.02f, 0.02f, 0.02f);
		}
		else if (Look == TEXT("morality_m"))
		{
			Def.Upper = FLinearColor(0.020f, 0.050f, 0.032f);
			Def.Lower = FLinearColor(0.018f, 0.032f, 0.024f);
			Def.Head = EHead::Cap;
			Def.HeadColor = FLinearColor(0.018f, 0.04f, 0.026f);
		}
		else if (Look == TEXT("plain_revealed"))
		{
			Def.Upper = FLinearColor(0.012f, 0.012f, 0.014f);
			Def.Lower = FLinearColor(0.02f, 0.02f, 0.022f);
		}
		else if (Look == TEXT("lawyer"))
		{
			Def.Upper = FLinearColor(0.03f, 0.035f, 0.05f);
			Def.Lower = FLinearColor(0.025f, 0.028f, 0.04f);
		}
		else
		{
			// man, student_m, plain (plainclothes look like anyone), driver, professor...
			Def.Upper = Shirts[Rng.RandRange(0, UE_ARRAY_COUNT(Shirts) - 1)];
			Def.Lower = FLinearColor(0.03f, 0.03f, 0.035f) * Rng.FRandRange(0.8f, 1.6f);
			Def.Scale = Rng.FRandRange(0.96f, 1.04f);
		}
		return Def;
	}

	/** Reference pose transform of a bone in mesh component space. */
	bool RefBone(const USkeletalMesh* Mesh, FName Bone, FTransform& Out)
	{
		if (!Mesh)
		{
			return false;
		}
		const FReferenceSkeleton& RefSkeleton = Mesh->GetRefSkeleton();
		const int32 Index = RefSkeleton.FindBoneIndex(Bone);
		if (Index == INDEX_NONE)
		{
			return false;
		}
		Out = FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkeleton, Index);
		return true;
	}

	/** Character space (forward, right, up) to mannequin component space (which faces +Y). */
	FVector ToBody(const FVector& C)
	{
		return FVector(-C.Y, C.X, C.Z);
	}

	UStaticMeshComponent* AddShape(AActor* Owner, USkeletalMeshComponent* Body, UStaticMesh* Mesh, const FVector& CenterChar,
		const FVector& SizeCm, UMaterialInterface* Material, FName Bone)
	{
		UStaticMeshComponent* Shape = NewObject<UStaticMeshComponent>(Owner);
		Shape->SetMobility(EComponentMobility::Movable);
		Shape->SetStaticMesh(Mesh);
		if (Material)
		{
			Shape->SetMaterial(0, Material);
		}
		Shape->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Shape->SetCanEverAffectNavigation(false);

		// the shape's +X points where the person looks (mannequin +Y)
		const FTransform InBody(FRotator(0.f, 90.f, 0.f), ToBody(CenterChar), SizeCm / 100.0);
		FTransform BoneRef;
		if (Bone != NAME_None && RefBone(Body->GetSkeletalMeshAsset(), Bone, BoneRef))
		{
			Shape->SetupAttachment(Body, Bone);
			Shape->SetRelativeTransform(InBody.GetRelativeTransform(BoneRef));
		}
		else
		{
			Shape->SetupAttachment(Body);
			Shape->SetRelativeTransform(InBody);
		}
		Shape->RegisterComponent();
		return Shape;
	}
}

bool CHLooks::IsFemale(const FString& Look)
{
	return MakeLook(Look, 0).bFemale;
}

UMaterialInstanceDynamic* CHLooks::MakeLit(UObject* Outer, const FLinearColor& Color, float Roughness)
{
	UMaterialInterface* Base = LoadAsset<UMaterialInterface>(TEXT("/Game/Materials/M_CH_Lit.M_CH_Lit"));
	if (!Base)
	{
		return nullptr;
	}
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Outer);
	Material->SetVectorParameterValue(TEXT("Color"), Color);
	Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
	Material->SetScalarParameterValue(TEXT("Grime"), 0.f);
	return Material;
}

void CHLooks::Tint(UObject* Outer, USkeletalMeshComponent* Body, const FLinearColor& Upper, const FLinearColor& Lower,
	TArray<TObjectPtr<UMaterialInstanceDynamic>>& InOutMaterials)
{
	// mannequin slot 0 = head, arms and legs; slot 1 = torso
	const int32 NumSlots = Body->GetNumMaterials();
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		if (UMaterialInstanceDynamic* Material = MakeLit(Outer, Slot == 0 ? Lower : Upper))
		{
			Body->SetMaterial(Slot, Material);
			InOutMaterials.Add(Material);
		}
	}
}

float CHLooks::Apply(AActor* Owner, USkeletalMeshComponent* Body, const FString& Look, int32 Seed,
	TArray<TObjectPtr<UStaticMeshComponent>>& InOutShapes, TArray<TObjectPtr<UMaterialInstanceDynamic>>& InOutMaterials)
{
	for (UStaticMeshComponent* Shape : InOutShapes)
	{
		if (Shape)
		{
			Shape->DestroyComponent();
		}
	}
	InOutShapes.Reset();
	InOutMaterials.Reset();

	if (!Owner || !Body)
	{
		return 1.f;
	}

	const FLookDef Def = MakeLook(Look, Seed);
	USkeletalMesh* Mesh = LoadAsset<USkeletalMesh>(Def.bFemale
		? TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple")
		: TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (Mesh && Body->GetSkeletalMeshAsset() != Mesh)
	{
		Body->SetSkeletalMeshAsset(Mesh);
	}
	Tint(Owner, Body, Def.Upper, Def.Lower, InOutMaterials);

	if (Def.Head == EHead::None)
	{
		return Def.Scale;
	}

	UStaticMesh* Sphere = LoadAsset<UStaticMesh>(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cone = LoadAsset<UStaticMesh>(TEXT("/Engine/BasicShapes/Cone.Cone"));
	UStaticMesh* Cylinder = LoadAsset<UStaticMesh>(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	FVector HeadJoint(0.f, 0.f, 160.f);
	FTransform HeadRef;
	if (RefBone(Body->GetSkeletalMeshAsset(), TEXT("head"), HeadRef))
	{
		const FVector B = HeadRef.GetLocation();
		HeadJoint = FVector(B.Y, -B.X, B.Z);
	}
	const FVector Head = HeadJoint + FVector(2.f, 0.f, 9.f);

	UMaterialInstanceDynamic* Cloth = MakeLit(Owner, Def.HeadColor, 0.95f);
	if (Cloth)
	{
		InOutMaterials.Add(Cloth);
	}

	auto Add = [&](UStaticMesh* ShapeMesh, const FVector& Center, const FVector& Size, FName Bone)
	{
		if (ShapeMesh)
		{
			InOutShapes.Add(AddShape(Owner, Body, ShapeMesh, Center, Size, Cloth, Bone));
		}
	};

	switch (Def.Head)
	{
	case EHead::Scarf:
		// wraps the head, the face shows at the front; the cloth falls over the shoulders
		Add(Sphere, Head + FVector(-2.5f, 0.f, 1.5f), FVector(24.f, 23.5f, 26.5f), TEXT("head"));
		Add(Cone, Head + FVector(-2.f, 0.f, -19.f), FVector(34.f, 40.f, 26.f), TEXT("neck_01"));
		break;
	case EHead::LooseScarf:
		// slipping back: the hair at the front shows
		Add(Sphere, Head + FVector(-5.5f, 0.f, 1.f), FVector(22.5f, 23.f, 25.f), TEXT("head"));
		Add(Cone, Head + FVector(-4.f, 0.f, -19.f), FVector(30.f, 38.f, 24.f), TEXT("neck_01"));
		break;
	case EHead::Chador:
	{
		// a black chador from the crown to the ground
		Add(Sphere, Head + FVector(-2.5f, 0.f, 1.5f), FVector(26.f, 25.f, 28.f), TEXT("head"));
		const float Top = Head.Z + 8.f;
		Add(Cone, FVector(-3.f, 0.f, Top * 0.5f), FVector(80.f, 86.f, Top), NAME_None);
		break;
	}
	case EHead::Helmet:
		// riot helmet with a dark visor
		Add(Sphere, Head + FVector(0.5f, 0.f, 3.f), FVector(28.f, 27.f, 25.f), TEXT("head"));
		Add(Cylinder, Head + FVector(9.f, 0.f, -1.f), FVector(6.f, 22.f, 10.f), TEXT("head"));
		break;
	case EHead::Cap:
		Add(Sphere, Head + FVector(-0.5f, 0.f, 6.f), FVector(23.f, 22.f, 13.f), TEXT("head"));
		Add(Cylinder, Head + FVector(10.f, 0.f, 3.f), FVector(12.f, 18.f, 1.5f), TEXT("head"));
		break;
	default:
		break;
	}
	return Def.Scale;
}
