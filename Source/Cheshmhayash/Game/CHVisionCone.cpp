// Cheshm Hayash - a guard's vision fan.

#include "Game/CHVisionCone.h"
#include "CollisionQueryParams.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "World/CHLevelBuilder.h"

UCHVisionCone::UCHVisionCone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SetCastShadow(false);
	SetCanEverAffectNavigation(false);
	bUseAsyncCooking = false;
}

void UCHVisionCone::Configure(float InRange, float InHalfAngle)
{
	Range = FMath::Max(100.f, InRange);
	HalfAngle = FMath::Clamp(InHalfAngle, 5.f, 89.f);
	Segments = FMath::Clamp(FMath::RoundToInt32(HalfAngle * 2.f / 2.5f), 8, 48);
	bCreated = false;
	ClearAllMeshSections();
}

void UCHVisionCone::Refresh(const FLinearColor& Color, float Opacity)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!Material)
	{
		if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Glow.M_CH_Glow"), nullptr, LOAD_NoWarn))
		{
			Material = UMaterialInstanceDynamic::Create(Base, this);
			Material->SetScalarParameterValue(TEXT("Soft"), 0.55f);
			Material->SetScalarParameterValue(TEXT("Strength"), 1.f);
			SetMaterial(0, Material);
		}
	}
	if (Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		Material->SetScalarParameterValue(TEXT("Strength"), CHLight::Emissive(0.55f));
	}

	const FTransform& World2 = GetComponentTransform();
	const FVector Origin = World2.GetLocation();
	const float Yaw = World2.Rotator().Yaw;
	const FVector Eye = Origin + FVector(0.f, 0.f, 100.f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CHVisionCone), false, GetOwner());
	const FCollisionObjectQueryParams Objects(ECC_WorldStatic);

	const int32 NumVerts = Segments + 2;
	Vertices.SetNum(NumVerts);
	Normals.Init(FVector::UpVector, NumVerts);
	UVs.SetNum(NumVerts);
	Colors.Init(FLinearColor::White, NumVerts);
	Tangents.Init(FProcMeshTangent(1.f, 0.f, 0.f), NumVerts);

	Vertices[0] = FVector::ZeroVector;
	UVs[0] = FVector2D(0.5, 0.5);
	for (int32 i = 0; i <= Segments; ++i)
	{
		const float Local = FMath::Lerp(-HalfAngle, HalfAngle, static_cast<float>(i) / Segments);
		const FVector Direction = FRotator(0.f, Yaw + Local, 0.f).Vector();
		float Distance = Range;
		FHitResult Hit;
		if (World->LineTraceSingleByObjectType(Hit, Eye, Eye + Direction * Range, Objects, Params))
		{
			Distance = FMath::Max(10.f, Hit.Distance);
		}
		const float Rad = FMath::DegreesToRadians(Local);
		Vertices[i + 1] = FVector(FMath::Cos(Rad) * Distance, FMath::Sin(Rad) * Distance, 0.f);
		const float Fraction = Distance / Range;
		UVs[i + 1] = FVector2D(0.5 + 0.5 * Fraction * FMath::Cos(Rad), 0.5 + 0.5 * Fraction * FMath::Sin(Rad));
	}

	if (!bCreated)
	{
		Triangles.Reset();
		for (int32 i = 0; i < Segments; ++i)
		{
			Triangles.Add(0);
			Triangles.Add(i + 2);
			Triangles.Add(i + 1);
		}
		CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
		if (Material)
		{
			SetMaterial(0, Material);
		}
		bCreated = true;
	}
	else
	{
		UpdateMeshSection_LinearColor(0, Vertices, Normals, UVs, Colors, Tangents);
	}
}
