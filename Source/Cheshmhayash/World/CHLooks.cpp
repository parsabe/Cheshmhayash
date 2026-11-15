// Cheshm Hayash - how people look. Styled characters built on the mannequin skeleton with
// multi-part clothing silhouettes (jackets, coats, manteaus, collars, sleeves, trousers, shoes,
// hair, headscarves, chadors, backpacks) attached to bones so they move naturally with animations
// and poses. Faces stay in shadow without facial features (rule 3.3).

#include "World/CHLooks.h"
#include "AnimationRuntime.h"
#include "Components/SkinnedMeshComponent.h"
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
	enum class EHair : uint8 { ShortMale, WavyMale, BaldingMale, LongFemale, ShortFemale, TiedFemale, None };
	enum class ETop : uint8 { JacketShirt, Hoodie, Cardigan, Suit, Manteau, Sweater, Uniform, ChadorDrape };

	struct FLookDef
	{
		bool bFemale = false;
		EHead Head = EHead::None;
		EHair Hair = EHair::ShortMale;
		ETop Top = ETop::JacketShirt;

		FLinearColor Skin = FLinearColor(0.24f, 0.17f, 0.13f);
		FLinearColor HairColor = FLinearColor(0.018f, 0.014f, 0.012f);
		FLinearColor OuterCoat = FLinearColor(0.09f, 0.12f, 0.16f);
		FLinearColor InnerShirt = FLinearColor(0.38f, 0.36f, 0.33f);
		FLinearColor Trousers = FLinearColor(0.045f, 0.055f, 0.08f);
		FLinearColor Shoes = FLinearColor(0.03f, 0.03f, 0.035f);
		FLinearColor ShoeSole = FLinearColor(0.45f, 0.43f, 0.40f);
		FLinearColor HeadColor = FLinearColor(0.02f, 0.02f, 0.02f);
		FLinearColor BagColor = FLinearColor(0.06f, 0.05f, 0.04f);

		bool bBackpack = false;
		bool bShoulderBag = false;
		bool bBeard = false;
		bool bWatch = false;
		float Scale = 1.f;
		FVector BuildScale = FVector(1.f, 1.f, 1.f);
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
			FLinearColor(0.08f, 0.16f, 0.19f), // deep teal
			FLinearColor(0.22f, 0.11f, 0.10f), // burgundy
			FLinearColor(0.11f, 0.15f, 0.10f), // olive green
			FLinearColor(0.24f, 0.20f, 0.15f), // warm camel/beige
			FLinearColor(0.06f, 0.07f, 0.11f), // navy
			FLinearColor(0.28f, 0.18f, 0.11f), // autumn rust
			FLinearColor(0.15f, 0.14f, 0.16f)  // charcoal plum
		};
		const FLinearColor Scarves[] = {
			FLinearColor(0.36f, 0.16f, 0.15f), // crimson rose
			FLinearColor(0.12f, 0.18f, 0.28f), // slate blue
			FLinearColor(0.42f, 0.36f, 0.28f), // warm sand
			FLinearColor(0.06f, 0.06f, 0.07f), // black
			FLinearColor(0.18f, 0.26f, 0.20f), // sage
			FLinearColor(0.38f, 0.24f, 0.14f)  // ochre
		};
		const FLinearColor Jackets[] = {
			FLinearColor(0.09f, 0.13f, 0.18f), // slate denim
			FLinearColor(0.18f, 0.13f, 0.09f), // brown suede
			FLinearColor(0.08f, 0.12f, 0.09f), // olive field jacket
			FLinearColor(0.22f, 0.20f, 0.18f), // stone grey
			FLinearColor(0.16f, 0.07f, 0.06f), // maroon
			FLinearColor(0.05f, 0.06f, 0.09f), // dark navy
			FLinearColor(0.25f, 0.14f, 0.07f)  // rust/terracotta
		};
		const FLinearColor InnerTops[] = {
			FLinearColor(0.46f, 0.44f, 0.40f), // off-white
			FLinearColor(0.18f, 0.20f, 0.24f), // heather blue
			FLinearColor(0.28f, 0.12f, 0.11f), // faded red
			FLinearColor(0.12f, 0.12f, 0.13f), // charcoal
			FLinearColor(0.35f, 0.30f, 0.22f)  // warm tan
		};
		const FLinearColor Pants[] = {
			FLinearColor(0.05f, 0.07f, 0.11f), // dark indigo jeans
			FLinearColor(0.08f, 0.11f, 0.16f), // mid blue jeans
			FLinearColor(0.04f, 0.04f, 0.045f),// black jeans
			FLinearColor(0.14f, 0.12f, 0.09f), // khaki chinos
			FLinearColor(0.08f, 0.08f, 0.085f) // charcoal trousers
		};

		if (Look == TEXT("he"))
		{
			// University student: layered slate-blue overshirt/jacket over a warm cream tee,
			// dark indigo jeans, worn sneakers with light soles, wavy dark hair, wristwatch.
			Def.bFemale = false;
			Def.Hair = EHair::WavyMale;
			Def.Top = ETop::JacketShirt;
			Def.OuterCoat = FLinearColor(0.08f, 0.13f, 0.19f);
			Def.InnerShirt = FLinearColor(0.44f, 0.41f, 0.36f);
			Def.Trousers = FLinearColor(0.045f, 0.06f, 0.095f);
			Def.Shoes = FLinearColor(0.05f, 0.05f, 0.06f);
			Def.ShoeSole = FLinearColor(0.50f, 0.48f, 0.44f);
			Def.bWatch = true;
			Def.bBeard = true;
			Def.Scale = 1.0f;
		}
		else if (Look == TEXT("sister"))
		{
			// 19, loud, funny, fearless: long, flowing dark hair past her shoulders,
			// stylish teal manteau open over a soft cream top, rose shawl draped loosely back,
			// dark fitted trousers, cream sneakers, shoulder tote bag.
			Def.bFemale = true;
			Def.Hair = EHair::LongFemale;
			Def.Head = EHead::LooseScarf;
			Def.Top = ETop::Manteau;
			Def.HairColor = FLinearColor(0.015f, 0.011f, 0.009f);
			Def.OuterCoat = FLinearColor(0.08f, 0.19f, 0.22f);
			Def.InnerShirt = FLinearColor(0.48f, 0.44f, 0.40f);
			Def.HeadColor = FLinearColor(0.36f, 0.15f, 0.16f);
			Def.Trousers = FLinearColor(0.04f, 0.045f, 0.06f);
			Def.Shoes = FLinearColor(0.46f, 0.44f, 0.40f);
			Def.ShoeSole = FLinearColor(0.55f, 0.53f, 0.50f);
			Def.bShoulderBag = true;
			Def.BagColor = FLinearColor(0.18f, 0.10f, 0.06f);
			Def.Scale = 0.96f;
		}
		else if (Look == TEXT("sister_after"))
		{
			// Released from prison in Aban: thinner, short uneven dark hair, no headscarf,
			// oversized muted sweater/coat.
			Def.bFemale = true;
			Def.Hair = EHair::ShortFemale;
			Def.Head = EHead::None;
			Def.Top = ETop::Sweater;
			Def.HairColor = FLinearColor(0.015f, 0.011f, 0.009f);
			Def.OuterCoat = FLinearColor(0.14f, 0.14f, 0.16f);
			Def.InnerShirt = FLinearColor(0.24f, 0.23f, 0.22f);
			Def.Trousers = FLinearColor(0.04f, 0.04f, 0.05f);
			Def.Shoes = FLinearColor(0.12f, 0.12f, 0.13f);
			Def.Scale = 0.95f;
			Def.BuildScale = FVector(0.92f, 0.92f, 0.96f);
		}
		else if (Look == TEXT("sister_older"))
		{
			// Years later: long flowing dark hair again, no headscarf, warm autumn coat.
			Def.bFemale = true;
			Def.Hair = EHair::LongFemale;
			Def.Head = EHead::None;
			Def.Top = ETop::Manteau;
			Def.HairColor = FLinearColor(0.015f, 0.011f, 0.009f);
			Def.OuterCoat = FLinearColor(0.22f, 0.15f, 0.11f);
			Def.InnerShirt = FLinearColor(0.12f, 0.11f, 0.11f);
			Def.Trousers = FLinearColor(0.04f, 0.04f, 0.045f);
			Def.Shoes = FLinearColor(0.06f, 0.05f, 0.04f);
			Def.ShoeSole = FLinearColor(0.08f, 0.07f, 0.06f);
			Def.Scale = 0.97f;
		}
		else if (Look == TEXT("brother"))
		{
			// Older brother: slightly broader shoulders, collared button-down jacket over dark shirt,
			// short hair and beard.
			Def.bFemale = false;
			Def.Hair = EHair::ShortMale;
			Def.Top = ETop::JacketShirt;
			Def.OuterCoat = FLinearColor(0.13f, 0.16f, 0.20f);
			Def.InnerShirt = FLinearColor(0.22f, 0.24f, 0.27f);
			Def.Trousers = FLinearColor(0.05f, 0.055f, 0.065f);
			Def.Shoes = FLinearColor(0.06f, 0.04f, 0.03f);
			Def.ShoeSole = FLinearColor(0.08f, 0.06f, 0.04f);
			Def.bBeard = true;
			Def.Scale = 1.02f;
			Def.BuildScale = FVector(1.05f, 1.05f, 1.02f);
		}
		else if (Look == TEXT("kian"))
		{
			// Best friend: warm terracotta/rust hoodie with hood on back, jeans, backpack.
			Def.bFemale = false;
			Def.Hair = EHair::WavyMale;
			Def.Top = ETop::Hoodie;
			Def.OuterCoat = FLinearColor(0.26f, 0.10f, 0.05f);
			Def.InnerShirt = FLinearColor(0.35f, 0.34f, 0.32f);
			Def.Trousers = FLinearColor(0.06f, 0.08f, 0.12f);
			Def.Shoes = FLinearColor(0.08f, 0.08f, 0.09f);
			Def.Scale = 0.99f;
		}
		else if (Look == TEXT("mother"))
		{
			// Mother: warm rose-brown long cardigan/dress, soft cream/sand shawl, dark hair.
			Def.bFemale = true;
			Def.Hair = EHair::TiedFemale;
			Def.Head = EHead::LooseScarf;
			Def.Top = ETop::Manteau;
			Def.HairColor = FLinearColor(0.04f, 0.03f, 0.025f);
			Def.OuterCoat = FLinearColor(0.20f, 0.13f, 0.13f);
			Def.InnerShirt = FLinearColor(0.32f, 0.28f, 0.26f);
			Def.HeadColor = FLinearColor(0.48f, 0.43f, 0.36f);
			Def.Trousers = FLinearColor(0.05f, 0.05f, 0.055f);
			Def.Shoes = FLinearColor(0.08f, 0.06f, 0.05f);
			Def.ShoeSole = FLinearColor(0.08f, 0.06f, 0.05f);
			Def.Scale = 0.94f;
			Def.BuildScale = FVector(1.04f, 1.03f, 0.94f);
		}
		else if (Look == TEXT("father"))
		{
			// Father: greying hair, warm knit cardigan over collared shirt, slightly heavier build.
			Def.bFemale = false;
			Def.Hair = EHair::BaldingMale;
			Def.Top = ETop::Cardigan;
			Def.HairColor = FLinearColor(0.22f, 0.22f, 0.23f);
			Def.OuterCoat = FLinearColor(0.18f, 0.14f, 0.10f);
			Def.InnerShirt = FLinearColor(0.38f, 0.37f, 0.34f);
			Def.Trousers = FLinearColor(0.07f, 0.065f, 0.06f);
			Def.Shoes = FLinearColor(0.06f, 0.045f, 0.03f);
			Def.ShoeSole = FLinearColor(0.05f, 0.04f, 0.03f);
			Def.bBeard = true;
			Def.Scale = 0.98f;
			Def.BuildScale = FVector(1.08f, 1.06f, 0.98f);
		}
		else if (Look == TEXT("lawyer"))
		{
			// Lawyer: tailored dark charcoal suit jacket with lapels, crisp white shirt, dark tie.
			Def.bFemale = false;
			Def.Hair = EHair::BaldingMale;
			Def.Top = ETop::Suit;
			Def.HairColor = FLinearColor(0.12f, 0.12f, 0.13f);
			Def.OuterCoat = FLinearColor(0.04f, 0.045f, 0.065f);
			Def.InnerShirt = FLinearColor(0.55f, 0.55f, 0.54f);
			Def.Trousers = FLinearColor(0.035f, 0.04f, 0.055f);
			Def.Shoes = FLinearColor(0.03f, 0.025f, 0.02f);
			Def.ShoeSole = FLinearColor(0.03f, 0.025f, 0.02f);
			Def.Scale = 1.0f;
		}
		else if (Look == TEXT("chador") || Look == TEXT("morality_f"))
		{
			Def.bFemale = true;
			Def.Hair = EHair::None;
			Def.Head = EHead::Chador;
			Def.Top = ETop::ChadorDrape;
			Def.HeadColor = FLinearColor(0.012f, 0.012f, 0.015f);
			Def.OuterCoat = FLinearColor(0.014f, 0.014f, 0.017f);
			Def.Trousers = FLinearColor(0.012f, 0.012f, 0.015f);
			Def.Scale = Rng.FRandRange(0.95f, 1.01f);
		}
		else if (Look == TEXT("oldwoman"))
		{
			Def.bFemale = true;
			Def.Hair = EHair::None;
			Def.Head = EHead::Chador;
			Def.Top = ETop::ChadorDrape;
			Def.HeadColor = FLinearColor(0.022f, 0.022f, 0.026f);
			Def.OuterCoat = FLinearColor(0.025f, 0.025f, 0.028f);
			Def.Trousers = FLinearColor(0.02f, 0.02f, 0.024f);
			Def.Scale = 0.91f;
			Def.BuildScale = FVector(1.04f, 1.02f, 0.91f);
		}
		else if (Look == TEXT("woman") || Look == TEXT("student_f"))
		{
			Def.bFemale = true;
			Def.Top = ETop::Manteau;
			Def.OuterCoat = Manteaus[Rng.RandRange(0, UE_ARRAY_COUNT(Manteaus) - 1)];
			Def.InnerShirt = InnerTops[Rng.RandRange(0, UE_ARRAY_COUNT(InnerTops) - 1)];
			Def.Trousers = Pants[Rng.RandRange(0, UE_ARRAY_COUNT(Pants) - 1)];
			Def.HeadColor = Scarves[Rng.RandRange(0, UE_ARRAY_COUNT(Scarves) - 1)];
			Def.Hair = (Rng.FRand() < 0.7f) ? EHair::LongFemale : EHair::TiedFemale;
			const FLinearColor Hairs[] = {
				FLinearColor(0.015f, 0.012f, 0.010f),
				FLinearColor(0.035f, 0.022f, 0.014f),
				FLinearColor(0.06f, 0.035f, 0.018f)
			};
			Def.HairColor = Hairs[Rng.RandRange(0, 2)];
			Def.Head = Look == TEXT("student_f")
				? (Rng.FRand() < 0.65f ? EHead::None : EHead::LooseScarf)
				: (Rng.FRand() < 0.45f ? EHead::LooseScarf : (Rng.FRand() < 0.5f ? EHead::None : EHead::Scarf));
			Def.bBackpack = (Look == TEXT("student_f") && Rng.FRand() < 0.55f);
			Def.bShoulderBag = (!Def.bBackpack && Rng.FRand() < 0.55f);
			Def.BagColor = FLinearColor(0.08f, 0.06f, 0.05f) * Rng.FRandRange(0.7f, 1.5f);
			Def.Scale = Rng.FRandRange(0.93f, 1.02f);
			Def.BuildScale = FVector(Rng.FRandRange(0.93f, 1.05f), Rng.FRandRange(0.93f, 1.05f), Def.Scale);
		}
		else if (Look == TEXT("girl"))
		{
			// University girl who cut her hair short in the yard
			Def.bFemale = true;
			Def.Hair = EHair::ShortFemale;
			Def.Head = EHead::None;
			Def.Top = ETop::Manteau;
			Def.OuterCoat = FLinearColor(0.22f, 0.12f, 0.08f);
			Def.InnerShirt = FLinearColor(0.40f, 0.38f, 0.35f);
			Def.Trousers = FLinearColor(0.05f, 0.07f, 0.11f);
			Def.bBackpack = true;
			Def.Scale = 0.96f;
		}
		else if (Look == TEXT("guard"))
		{
			Def.bFemale = false;
			Def.Hair = EHair::None;
			Def.Head = EHead::Helmet;
			Def.Top = ETop::Uniform;
			Def.OuterCoat = FLinearColor(0.018f, 0.022f, 0.020f);
			Def.InnerShirt = FLinearColor(0.012f, 0.014f, 0.013f);
			Def.Trousers = FLinearColor(0.014f, 0.016f, 0.015f);
			Def.HeadColor = FLinearColor(0.015f, 0.016f, 0.018f);
			Def.ShoeSole = FLinearColor(0.01f, 0.01f, 0.01f);
			Def.Scale = 1.04f;
			Def.BuildScale = FVector(1.08f, 1.08f, 1.04f);
		}
		else if (Look == TEXT("basij"))
		{
			Def.bFemale = false;
			Def.Hair = EHair::None;
			Def.Head = EHead::Helmet;
			Def.Top = ETop::Uniform;
			Def.OuterCoat = FLinearColor(0.055f, 0.050f, 0.032f);
			Def.InnerShirt = FLinearColor(0.035f, 0.032f, 0.022f);
			Def.Trousers = FLinearColor(0.025f, 0.025f, 0.022f);
			Def.HeadColor = FLinearColor(0.02f, 0.02f, 0.02f);
			Def.ShoeSole = FLinearColor(0.015f, 0.015f, 0.015f);
			Def.Scale = 1.02f;
		}
		else if (Look == TEXT("morality_m"))
		{
			Def.bFemale = false;
			Def.Hair = EHair::ShortMale;
			Def.Head = EHead::Cap;
			Def.Top = ETop::Uniform;
			Def.OuterCoat = FLinearColor(0.025f, 0.058f, 0.036f);
			Def.InnerShirt = FLinearColor(0.020f, 0.045f, 0.028f);
			Def.Trousers = FLinearColor(0.018f, 0.032f, 0.024f);
			Def.HeadColor = FLinearColor(0.018f, 0.045f, 0.028f);
			Def.ShoeSole = FLinearColor(0.01f, 0.01f, 0.01f);
			Def.bBeard = true;
			Def.Scale = 1.02f;
		}
		else if (Look == TEXT("plain_revealed"))
		{
			Def.bFemale = false;
			Def.Hair = EHair::ShortMale;
			Def.Top = ETop::JacketShirt;
			Def.OuterCoat = FLinearColor(0.018f, 0.018f, 0.020f);
			Def.InnerShirt = FLinearColor(0.03f, 0.03f, 0.035f);
			Def.Trousers = FLinearColor(0.02f, 0.02f, 0.025f);
			Def.bBeard = true;
		}
		else
		{
			// man, student_m, plain, driver, neighbour...
			Def.bFemale = false;
			Def.Hair = (Rng.FRand() < 0.45f) ? EHair::WavyMale : (Rng.FRand() < 0.75f ? EHair::ShortMale : EHair::BaldingMale);
			Def.Top = (Look == TEXT("student_m") && Rng.FRand() < 0.45f) ? ETop::Hoodie
				: (Rng.FRand() < 0.7f ? ETop::JacketShirt : ETop::Sweater);
			Def.OuterCoat = Jackets[Rng.RandRange(0, UE_ARRAY_COUNT(Jackets) - 1)];
			Def.InnerShirt = InnerTops[Rng.RandRange(0, UE_ARRAY_COUNT(InnerTops) - 1)];
			Def.Trousers = Pants[Rng.RandRange(0, UE_ARRAY_COUNT(Pants) - 1)];
			Def.bBeard = Rng.FRand() < 0.55f;
			Def.bBackpack = (Look == TEXT("student_m") && Rng.FRand() < 0.6f);
			Def.BagColor = FLinearColor(0.05f, 0.05f, 0.06f) * Rng.FRandRange(0.8f, 1.5f);
			Def.Scale = Rng.FRandRange(0.95f, 1.05f);
			Def.BuildScale = FVector(Rng.FRandRange(0.94f, 1.08f), Rng.FRandRange(0.94f, 1.08f), Def.Scale);
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

	/** Character space (forward +X, right +Y, up +Z) to mannequin component space (which faces +Y). */
	FVector ToBody(const FVector& C)
	{
		return FVector(-C.Y, C.X, C.Z);
	}

	/** Bone location in character space (+X forward, +Y right, +Z up). */
	FVector BoneChar(const USkeletalMesh* Mesh, FName Bone, const FVector& Fallback)
	{
		FTransform Ref;
		if (RefBone(Mesh, Bone, Ref))
		{
			const FVector B = Ref.GetLocation();
			return FVector(B.Y, -B.X, B.Z);
		}
		return Fallback;
	}

	UStaticMeshComponent* AddShape(AActor* Owner, USkinnedMeshComponent* Body, UStaticMesh* Mesh, const FVector& CenterChar,
		const FVector& SizeCm, UMaterialInterface* Material, FName Bone, const FRotator& RotChar = FRotator::ZeroRotator)
	{
		if (!Mesh)
		{
			return nullptr;
		}
		UStaticMeshComponent* Shape = NewObject<UStaticMeshComponent>(Owner);
		Shape->SetMobility(EComponentMobility::Movable);
		Shape->SetStaticMesh(Mesh);
		if (Material)
		{
			Shape->SetMaterial(0, Material);
		}
		Shape->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Shape->SetCanEverAffectNavigation(false);

		// Character +X points along mannequin +Y (Yaw +90 in component space)
		const FQuat BaseRot(FRotator(0.f, 90.f, 0.f));
		const FQuat ExtraRot(RotChar);
		const FTransform InBody(BaseRot * ExtraRot, ToBody(CenterChar), SizeCm / 100.0);
		FTransform BoneRef;
		const USkeletalMesh* SkelAsset = Cast<USkeletalMesh>(Body->GetSkinnedAsset());
		if (Bone != NAME_None && RefBone(SkelAsset, Bone, BoneRef))
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

FVector CHLooks::Scale3D(const FString& Look, int32 Seed)
{
	return MakeLook(Look, Seed).BuildScale;
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
	Material->SetScalarParameterValue(TEXT("Grime"), 0.15f);
	return Material;
}

void CHLooks::Tint(UObject* Outer, USkinnedMeshComponent* Body, const FLinearColor& Upper, const FLinearColor& Lower,
	TArray<TObjectPtr<UMaterialInstanceDynamic>>& InOutMaterials)
{
	// mannequin slot 0 = head, arms and legs; slot 1 = torso
	const int32 NumSlots = Body->GetNumMaterials();
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		if (UMaterialInstanceDynamic* Material = MakeLit(Outer, Slot == 0 ? Lower : Upper, 0.88f))
		{
			Body->SetMaterial(Slot, Material);
			InOutMaterials.Add(Material);
		}
	}
}

float CHLooks::Apply(AActor* Owner, USkinnedMeshComponent* Body, const FString& Look, int32 Seed,
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
	USkeletalMesh* SkelMesh = LoadAsset<USkeletalMesh>(Def.bFemale
		? TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple")
		: TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (SkelMesh && Body->GetSkinnedAsset() != SkelMesh)
	{
		Body->SetSkinnedAssetAndUpdate(SkelMesh);
	}

	// Base mannequin tint: slot 1 (torso) gets inner shirt, slot 0 (limbs/head) gets dark shadowed skin/trouser base
	Tint(Owner, Body, Def.InnerShirt, Def.Trousers * 0.85f, InOutMaterials);

	UStaticMesh* Cube = LoadAsset<UStaticMesh>(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sphere = LoadAsset<UStaticMesh>(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cone = LoadAsset<UStaticMesh>(TEXT("/Engine/BasicShapes/Cone.Cone"));
	UStaticMesh* Cylinder = LoadAsset<UStaticMesh>(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	auto Mat = [&](const FLinearColor& Col, float Rough = 0.88f) -> UMaterialInstanceDynamic*
	{
		UMaterialInstanceDynamic* M = MakeLit(Owner, Col, Rough);
		if (M)
		{
			InOutMaterials.Add(M);
		}
		return M;
	};

	UMaterialInstanceDynamic* MOuter = Mat(Def.OuterCoat, 0.88f);
	UMaterialInstanceDynamic* MInner = Mat(Def.InnerShirt, 0.90f);
	UMaterialInstanceDynamic* MPants = Mat(Def.Trousers, 0.86f);
	UMaterialInstanceDynamic* MShoes = Mat(Def.Shoes, 0.70f);
	UMaterialInstanceDynamic* MSole  = Mat(Def.ShoeSole, 0.85f);
	UMaterialInstanceDynamic* MSkin  = Mat(Def.Skin, 0.82f);
	UMaterialInstanceDynamic* MFaceShadow = Mat(Def.Skin * 0.35f, 0.95f); // shadowed featureless face (rule 3.3)
	UMaterialInstanceDynamic* MHair  = Mat(Def.HairColor, 0.85f);

	auto Add = [&](UStaticMesh* ShapeMesh, const FVector& Center, const FVector& Size, UMaterialInterface* M, FName Bone, const FRotator& Rot = FRotator::ZeroRotator)
	{
		if (UStaticMeshComponent* Comp = AddShape(Owner, Body, ShapeMesh, Center, Size, M, Bone, Rot))
		{
			InOutShapes.Add(Comp);
		}
	};

	const USkeletalMesh* Asset = Cast<USkeletalMesh>(Body->GetSkinnedAsset());
	const FVector HeadJoint = BoneChar(Asset, TEXT("head"), FVector(0.f, 0.f, 160.f));
	const FVector NeckJoint = BoneChar(Asset, TEXT("neck_01"), FVector(0.f, 0.f, 151.f));
	const FVector Spine3    = BoneChar(Asset, TEXT("spine_03"), FVector(0.f, 0.f, 128.f));
	const FVector Spine2    = BoneChar(Asset, TEXT("spine_02"), FVector(0.f, 0.f, 114.f));
	const FVector Pelvis    = BoneChar(Asset, TEXT("pelvis"), FVector(0.f, 0.f, 96.f));
	const FVector UArmL     = BoneChar(Asset, TEXT("upperarm_l"), FVector(-2.f, -18.f, 142.f));
	const FVector UArmR     = BoneChar(Asset, TEXT("upperarm_r"), FVector(-2.f, 18.f, 142.f));
	const FVector LArmL     = BoneChar(Asset, TEXT("lowerarm_l"), FVector(-2.f, -24.f, 118.f));
	const FVector LArmR     = BoneChar(Asset, TEXT("lowerarm_r"), FVector(-2.f, 24.f, 118.f));
	const FVector HandL     = BoneChar(Asset, TEXT("hand_l"), FVector(2.f, -28.f, 95.f));
	const FVector HandR     = BoneChar(Asset, TEXT("hand_r"), FVector(2.f, 28.f, 95.f));
	const FVector ThighL    = BoneChar(Asset, TEXT("thigh_l"), FVector(0.f, -10.f, 92.f));
	const FVector ThighR    = BoneChar(Asset, TEXT("thigh_r"), FVector(0.f, 10.f, 92.f));
	const FVector CalfL     = BoneChar(Asset, TEXT("calf_l"), FVector(-1.f, -10.f, 50.f));
	const FVector CalfR     = BoneChar(Asset, TEXT("calf_r"), FVector(-1.f, 10.f, 50.f));
	const FVector FootL     = BoneChar(Asset, TEXT("foot_l"), FVector(2.f, -10.f, 9.f));
	const FVector FootR     = BoneChar(Asset, TEXT("foot_r"), FVector(2.f, 10.f, 9.f));

	const FVector Head = HeadJoint + FVector(2.f, 0.f, 9.f);
	const float W = Def.bFemale ? 0.88f : 1.0f;

	// ---- 1. Head, Neck & Hands (skin tone on neck/hands, shadowed featureless face per 3.3) ----
	Add(Cylinder, NeckJoint + FVector(0.f, 0.f, 3.f), FVector(11.f * W, 11.f * W, 10.f), MSkin, TEXT("neck_01"));
	Add(Sphere, Head + FVector(1.f, 0.f, -1.f), FVector(19.5f, 17.5f * W, 22.f), MFaceShadow, TEXT("head"));
	if (Def.bBeard && !Def.bFemale)
	{
		Add(Sphere, Head + FVector(3.2f, 0.f, -5.5f), FVector(16.f, 15.5f, 11.f), MHair, TEXT("head"));
	}
	Add(Sphere, HandL + FVector(0.f, -1.f, -3.f), FVector(9.f * W, 5.5f * W, 11.f * W), MSkin, TEXT("hand_l"));
	Add(Sphere, HandR + FVector(0.f, 1.f, -3.f), FVector(9.f * W, 5.5f * W, 11.f * W), MSkin, TEXT("hand_r"));

	if (Def.bWatch)
	{
		UMaterialInstanceDynamic* MWatch = Mat(FLinearColor(0.02f, 0.02f, 0.025f), 0.4f);
		Add(Cylinder, HandL + FVector(0.f, 0.f, 3.5f), FVector(6.8f, 6.8f, 2.2f), MWatch, TEXT("lowerarm_l"));
	}

	// ---- 2. Hair (sculpted multi-part volumes; sister has long, flowing dark hair) ----
	switch (Def.Hair)
	{
	case EHair::WavyMale:
		// Full wavy dark hair: crown, textured front wave, back & side volumes
		Add(Sphere, Head + FVector(-1.0f, 0.f, 5.2f), FVector(22.5f, 20.5f, 16.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(5.5f, 1.5f, 6.5f), FVector(13.f, 16.5f, 9.5f), MHair, TEXT("head"), FRotator(-12.f, 8.f, 0.f));
		Add(Sphere, Head + FVector(-4.2f, 0.f, 0.5f), FVector(16.f, 20.f, 18.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(-1.5f, -7.8f, 1.5f), FVector(14.f, 6.5f, 14.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(-1.5f, 7.8f, 1.5f), FVector(14.f, 6.5f, 14.f), MHair, TEXT("head"));
		break;

	case EHair::ShortMale:
		Add(Sphere, Head + FVector(-1.2f, 0.f, 4.5f), FVector(21.2f, 19.2f, 15.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(-3.8f, 0.f, 0.f), FVector(15.f, 18.8f, 16.f), MHair, TEXT("head"));
		break;

	case EHair::BaldingMale:
		Add(Sphere, Head + FVector(-3.5f, 0.f, 1.f), FVector(17.f, 19.8f, 14.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(-1.f, -8.2f, 1.f), FVector(13.f, 5.f, 11.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(-1.f, 8.2f, 1.f), FVector(13.f, 5.f, 11.f), MHair, TEXT("head"));
		break;

	case EHair::LongFemale:
		// Rich, long, flowing dark hair framing her face and cascading down past her shoulders
		Add(Sphere, Head + FVector(-1.0f, 0.f, 5.2f), FVector(22.f, 20.5f, 16.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(4.2f, -4.5f, 4.8f), FVector(12.f, 11.f, 10.f), MHair, TEXT("head"), FRotator(0.f, -15.f, -12.f));
		Add(Sphere, Head + FVector(4.2f, 4.5f, 4.8f), FVector(12.f, 11.f, 10.f), MHair, TEXT("head"), FRotator(0.f, 15.f, 12.f));
		// Side waves framing the face
		Add(Cylinder, Head + FVector(1.5f, -9.2f, -6.f), FVector(6.5f, 5.5f, 22.f), MHair, TEXT("head"), FRotator(6.f, 0.f, -8.f));
		Add(Cylinder, Head + FVector(1.5f, 9.2f, -6.f), FVector(6.5f, 5.5f, 22.f), MHair, TEXT("head"), FRotator(6.f, 0.f, 8.f));
		// Back cascade from crown to mid-back
		Add(Sphere, Head + FVector(-5.5f, 0.f, -2.f), FVector(15.f, 21.f, 22.f), MHair, TEXT("head"));
		Add(Cone, Spine3 + FVector(-11.f, 0.f, 12.f), FVector(14.f, 26.f, 38.f), MHair, TEXT("spine_03"), FRotator(8.f, 0.f, 0.f));
		Add(Sphere, Spine3 + FVector(-11.5f, -6.f, 4.f), FVector(10.f, 11.f, 28.f), MHair, TEXT("spine_03"), FRotator(6.f, 0.f, -6.f));
		Add(Sphere, Spine3 + FVector(-11.5f, 6.f, 4.f), FVector(10.f, 11.f, 28.f), MHair, TEXT("spine_03"), FRotator(6.f, 0.f, 6.f));
		// Front shoulder locks draping over the collarbone
		Add(Cylinder, Spine3 + FVector(9.5f, -9.5f, 14.f), FVector(5.5f, 5.0f, 20.f), MHair, TEXT("spine_03"), FRotator(-10.f, 0.f, 8.f));
		Add(Cylinder, Spine3 + FVector(9.5f, 9.5f, 14.f), FVector(5.5f, 5.0f, 20.f), MHair, TEXT("spine_03"), FRotator(-10.f, 0.f, -8.f));
		break;

	case EHair::ShortFemale:
		// Fresh uneven short bob (2.1 yard girl / 3.1 sister after prison)
		Add(Sphere, Head + FVector(-1.0f, 0.f, 4.8f), FVector(22.f, 20.5f, 16.f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(-3.5f, 0.f, -1.f), FVector(17.f, 21.f, 17.f), MHair, TEXT("head"));
		Add(Cylinder, Head + FVector(1.5f, -8.8f, -2.5f), FVector(6.f, 5.f, 13.f), MHair, TEXT("head"), FRotator(5.f, 0.f, -6.f));
		Add(Cylinder, Head + FVector(1.5f, 8.8f, -3.5f), FVector(6.f, 5.f, 15.f), MHair, TEXT("head"), FRotator(5.f, 0.f, 6.f));
		break;

	case EHair::TiedFemale:
		Add(Sphere, Head + FVector(-1.2f, 0.f, 4.5f), FVector(21.5f, 19.8f, 15.5f), MHair, TEXT("head"));
		Add(Sphere, Head + FVector(-9.5f, 0.f, -1.5f), FVector(11.f, 11.f, 11.f), MHair, TEXT("head"));
		break;

	default:
		break;
	}

	// ---- 3. Layered Tops (Jackets, Manteaus, Hoodies, Cardigans, Suits) ----
	if (Def.Top != ETop::ChadorDrape)
	{
		// Inner shirt / chest layer
		Add(Cube, Spine3 + FVector(1.f, 0.f, 4.f), FVector(21.f * W, 31.f * W, 24.f), MInner, TEXT("spine_03"));
		Add(Cube, Spine2 + FVector(0.5f, 0.f, 0.f), FVector(19.5f * W, 28.f * W, 20.f), MInner, TEXT("spine_02"));

		if (Def.Top == ETop::JacketShirt || Def.Top == ETop::Suit || Def.Top == ETop::Cardigan)
		{
			// Open front jacket/cardigan panels (left and right) + back panel + collar + sleeves
			const float CoatLen = (Def.Top == ETop::Suit) ? 30.f : 26.f;
			Add(Cube, Spine3 + FVector(-4.5f, 0.f, 2.f), FVector(14.f * W, 34.f * W, CoatLen), MOuter, TEXT("spine_03"));
			Add(Cube, Spine3 + FVector(5.5f, -9.5f * W, 2.f), FVector(12.f * W, 13.f * W, CoatLen), MOuter, TEXT("spine_03"));
			Add(Cube, Spine3 + FVector(5.5f, 9.5f * W, 2.f), FVector(12.f * W, 13.f * W, CoatLen), MOuter, TEXT("spine_03"));
			// Lower jacket waist
			Add(Cube, Spine2 + FVector(-4.f, 0.f, -2.f), FVector(14.f * W, 32.f * W, 24.f), MOuter, TEXT("spine_02"));
			Add(Cube, Spine2 + FVector(5.0f, -9.0f * W, -2.f), FVector(11.f * W, 12.f * W, 24.f), MOuter, TEXT("spine_02"));
			Add(Cube, Spine2 + FVector(5.0f, 9.0f * W, -2.f), FVector(11.f * W, 12.f * W, 24.f), MOuter, TEXT("spine_02"));
			// Jacket collar around neck
			Add(Cylinder, NeckJoint + FVector(-1.5f, 0.f, -2.f), FVector(16.f * W, 18.f * W, 7.f), MOuter, TEXT("neck_01"));
		}
		else if (Def.Top == ETop::Hoodie || Def.Top == ETop::Sweater || Def.Top == ETop::Uniform)
		{
			// Full pullover hoodie / sweater / tactical vest
			Add(Cube, Spine3 + FVector(0.5f, 0.f, 3.f), FVector(24.f * W, 35.f * W, 26.f), MOuter, TEXT("spine_03"));
			Add(Cube, Spine2 + FVector(0.f, 0.f, -1.f), FVector(22.5f * W, 32.f * W, 24.f), MOuter, TEXT("spine_02"));
			if (Def.Top == ETop::Hoodie)
			{
				// Folded hood resting behind the neck
				Add(Sphere, NeckJoint + FVector(-8.f, 0.f, -2.f), FVector(16.f, 22.f, 12.f), MOuter, TEXT("spine_03"));
			}
			else if (Def.Top == ETop::Uniform)
			{
				// Shoulder pads + utility belt for guards
				UMaterialInstanceDynamic* MPad = Mat(Def.OuterCoat * 0.7f, 0.5f);
				Add(Sphere, UArmL + FVector(0.f, -3.f, 2.f), FVector(15.f, 15.f, 14.f), MPad, TEXT("upperarm_l"));
				Add(Sphere, UArmR + FVector(0.f, 3.f, 2.f), FVector(15.f, 15.f, 14.f), MPad, TEXT("upperarm_r"));
				Add(Cylinder, Pelvis + FVector(0.f, 0.f, 4.f), FVector(24.f, 33.f, 6.f), MPad, TEXT("pelvis"));
			}
		}
		else if (Def.Top == ETop::Manteau)
		{
			// Iranian manteau (long open-front coat reaching mid-thigh, moving with pelvis & thighs)
			Add(Cube, Spine3 + FVector(-3.5f, 0.f, 2.f), FVector(15.f * W, 33.f * W, 26.f), MOuter, TEXT("spine_03"));
			Add(Cube, Spine3 + FVector(5.0f, -8.5f * W, 2.f), FVector(11.f * W, 11.5f * W, 26.f), MOuter, TEXT("spine_03"));
			Add(Cube, Spine3 + FVector(5.0f, 8.5f * W, 2.f), FVector(11.f * W, 11.5f * W, 26.f), MOuter, TEXT("spine_03"));
			Add(Cube, Spine2 + FVector(0.f, 0.f, -2.f), FVector(21.5f * W, 30.5f * W, 24.f), MOuter, TEXT("spine_02"));
			// Manteau skirt over hips and upper thighs
			Add(Cube, Pelvis + FVector(-1.f, 0.f, -6.f), FVector(23.f * W, 33.f * W, 24.f), MOuter, TEXT("pelvis"));
			Add(Cylinder, ThighL + FVector(-0.5f, -1.f, -12.f), FVector(15.5f * W, 16.5f * W, 26.f), MOuter, TEXT("thigh_l"));
			Add(Cylinder, ThighR + FVector(-0.5f, 1.f, -12.f), FVector(15.5f * W, 16.5f * W, 26.f), MOuter, TEXT("thigh_r"));
		}

		// Sleeves on upper and lower arms
		Add(Cylinder, (UArmL + LArmL) * 0.5f, FVector(11.5f * W, 11.5f * W, 26.f), MOuter, TEXT("upperarm_l"));
		Add(Cylinder, (UArmR + LArmR) * 0.5f, FVector(11.5f * W, 11.5f * W, 26.f), MOuter, TEXT("upperarm_r"));
		Add(Cylinder, (LArmL + HandL) * 0.5f + FVector(0.f, 0.f, 2.f), FVector(9.8f * W, 9.8f * W, 22.f), MOuter, TEXT("lowerarm_l"));
		Add(Cylinder, (LArmR + HandR) * 0.5f + FVector(0.f, 0.f, 2.f), FVector(9.8f * W, 9.8f * W, 22.f), MOuter, TEXT("lowerarm_r"));

		// ---- 4. Trousers & Shoes ----
		Add(Cube, Pelvis + FVector(0.f, 0.f, -2.f), FVector(21.f * W, 30.f * W, 18.f), MPants, TEXT("pelvis"));
		Add(Cylinder, (ThighL + CalfL) * 0.5f, FVector(14.5f * W, 14.5f * W, 40.f), MPants, TEXT("thigh_l"));
		Add(Cylinder, (ThighR + CalfR) * 0.5f, FVector(14.5f * W, 14.5f * W, 40.f), MPants, TEXT("thigh_r"));
		Add(Cylinder, (CalfL + FootL) * 0.5f + FVector(0.f, 0.f, 2.f), FVector(11.2f * W, 11.2f * W, 38.f), MPants, TEXT("calf_l"));
		Add(Cylinder, (CalfR + FootR) * 0.5f + FVector(0.f, 0.f, 2.f), FVector(11.2f * W, 11.2f * W, 38.f), MPants, TEXT("calf_r"));

		// Sneakers / shoes with distinct soles
		Add(Cube, FootL + FVector(4.5f, 0.f, -3.5f), FVector(23.f * W, 10.2f * W, 7.5f), MShoes, TEXT("foot_l"));
		Add(Cube, FootR + FVector(4.5f, 0.f, -3.5f), FVector(23.f * W, 10.2f * W, 7.5f), MShoes, TEXT("foot_r"));
		Add(Cube, FootL + FVector(4.5f, 0.f, -7.0f), FVector(23.8f * W, 10.6f * W, 2.2f), MSole, TEXT("foot_l"));
		Add(Cube, FootR + FVector(4.5f, 0.f, -7.0f), FVector(23.8f * W, 10.6f * W, 2.2f), MSole, TEXT("foot_r"));
	}

	// ---- 5. Bags / Backpacks ----
	if (Def.bBackpack)
	{
		UMaterialInstanceDynamic* MBag = Mat(Def.BagColor, 0.9f);
		Add(Cube, Spine3 + FVector(-14.f, 0.f, -1.f), FVector(14.f, 26.f, 34.f), MBag, TEXT("spine_03"));
		Add(Cube, Spine3 + FVector(4.f, -10.f * W, 6.f), FVector(18.f, 3.5f, 16.f), MBag, TEXT("spine_03"));
		Add(Cube, Spine3 + FVector(4.f, 10.f * W, 6.f), FVector(18.f, 3.5f, 16.f), MBag, TEXT("spine_03"));
	}
	else if (Def.bShoulderBag)
	{
		UMaterialInstanceDynamic* MBag = Mat(Def.BagColor, 0.85f);
		Add(Cube, Pelvis + FVector(-2.f, 16.f * W, 4.f), FVector(24.f, 8.f, 26.f), MBag, TEXT("spine_02"), FRotator(0.f, 0.f, -6.f));
		Add(Cube, Spine3 + FVector(1.f, 12.f * W, 4.f), FVector(16.f, 2.5f, 28.f), MBag, TEXT("spine_03"));
	}

	// ---- 6. Headgear (Scarf, LooseScarf, Chador, Helmet, Cap) ----
	if (Def.Head != EHead::None)
	{
		UMaterialInstanceDynamic* MHead = Mat(Def.HeadColor, 0.92f);
		switch (Def.Head)
		{
		case EHead::Scarf:
			Add(Sphere, Head + FVector(-2.5f, 0.f, 1.5f), FVector(24.f, 23.5f, 26.5f), MHead, TEXT("head"));
			Add(Cone, Head + FVector(-2.f, 0.f, -19.f), FVector(34.f, 40.f, 26.f), MHead, TEXT("neck_01"));
			break;
		case EHead::LooseScarf:
			// Draped loosely back on the shoulders so the hair clearly shows in front and behind
			Add(Cone, NeckJoint + FVector(-2.f, 0.f, -8.f), FVector(30.f, 38.f, 20.f), MHead, TEXT("neck_01"));
			break;
		case EHead::Chador:
		{
			Add(Sphere, Head + FVector(-2.5f, 0.f, 1.5f), FVector(26.f, 25.f, 28.f), MHead, TEXT("head"));
			const float Top = Head.Z + 8.f;
			Add(Cone, FVector(-3.f, 0.f, Top * 0.5f), FVector(80.f, 86.f, Top), MHead, NAME_None);
			break;
		}
		case EHead::Helmet:
			Add(Sphere, Head + FVector(0.5f, 0.f, 3.f), FVector(28.f, 27.f, 25.f), MHead, TEXT("head"));
			Add(Cylinder, Head + FVector(9.f, 0.f, -1.f), FVector(6.f, 22.f, 10.f), MHead, TEXT("head"));
			break;
		case EHead::Cap:
			Add(Sphere, Head + FVector(-0.5f, 0.f, 6.f), FVector(23.f, 22.f, 13.f), MHead, TEXT("head"));
			Add(Cylinder, Head + FVector(10.f, 0.f, 3.f), FVector(12.f, 18.f, 1.5f), MHead, TEXT("head"));
			break;
		default:
			break;
		}
	}

	return Def.Scale;
}
