// Cheshm Hayash - a fire in the street.

#include "World/CHFire.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/CHLevelBuilder.h"

namespace
{
	UStaticMeshComponent* MakeSprite(AActor* Owner, UStaticMesh* Mesh, UMaterialInterface* Material)
	{
		UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner);
		Component->SetMobility(EComponentMobility::Movable);
		Component->SetStaticMesh(Mesh);
		Component->SetMaterial(0, Material);
		Component->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Component->SetCastShadow(false);
		Component->SetCanEverAffectNavigation(false);
		Component->SetupAttachment(Owner->GetRootComponent());
		Component->RegisterComponent();
		return Component;
	}
}

ACHFire::ACHFire()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(SceneRoot);
	Light->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	Light->SetIntensityUnits(ELightUnits::Candelas);
	Light->Intensity = 60.f;
	Light->LightColor = FColor(255, 120, 40);
	Light->AttenuationRadius = 1600.f;
	Light->SourceRadius = 25.f;
	Light->CastShadows = false;
	Light->VolumetricScatteringIntensity = 3.f;
}

void ACHFire::Setup(float InScale, bool bInSmoke, bool bShadows, int32 Seed)
{
	Scale = InScale;
	bSmoke = bInSmoke;
	Rng.Initialize(Seed * 31 + 7);

	BaseIntensity = 70.f * Scale * Scale;
	Light->SetIntensity(BaseIntensity);
	Light->SetAttenuationRadius(1300.f + 700.f * Scale);
	Light->SetRelativeLocation(FVector(0.f, 0.f, 70.f * Scale));
	Light->SetCastShadows(bShadows);

	UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Glow.M_CH_Glow"), nullptr, LOAD_NoWarn);
	if (!Cone || !Plane || !Glow)
	{
		return;
	}

	// flames: translucent glowing cones, re-shaped every frame
	const FLinearColor FlameColors[] = { FLinearColor(1.f, 0.35f, 0.04f), FLinearColor(1.f, 0.55f, 0.12f), FLinearColor(1.f, 0.78f, 0.35f) };
	for (int32 Index = 0; Index < 6; ++Index)
	{
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Glow, this);
		Material->SetVectorParameterValue(TEXT("Color"), FlameColors[Index % 3]);
		Material->SetScalarParameterValue(TEXT("Strength"), CHLight::Emissive(Index % 3 == 2 ? 9.f : 6.f));
		Material->SetScalarParameterValue(TEXT("Opacity"), 0.55f);
		Material->SetScalarParameterValue(TEXT("Soft"), 0.f);
		Flames.Add(MakeSprite(this, Cone, Material));
	}

	// a soft halo around the flames
	{
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Glow, this);
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.42f, 0.10f));
		Material->SetScalarParameterValue(TEXT("Strength"), CHLight::Emissive(1.6f));
		Material->SetScalarParameterValue(TEXT("Opacity"), 0.45f);
		Material->SetScalarParameterValue(TEXT("Soft"), 1.f);
		Halo = MakeSprite(this, Plane, Material);
	}

	// smoke
	if (bSmoke)
	{
		const int32 NumPuffs = FMath::RoundToInt32(10 + 4 * Scale);
		for (int32 Index = 0; Index < NumPuffs; ++Index)
		{
			UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Glow, this);
			Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.045f, 0.04f));
			Material->SetScalarParameterValue(TEXT("Strength"), CHLight::Emissive(0.5f));
			Material->SetScalarParameterValue(TEXT("Opacity"), 0.f);
			Material->SetScalarParameterValue(TEXT("Soft"), 1.f);
			Puffs.Add(MakeSprite(this, Plane, Material));
			PuffMaterials.Add(Material);
			const float Life = Rng.FRandRange(5.f, 8.f);
			PuffLife.Add(Life);
			PuffAge.Add(Life * Index / NumPuffs);
			PuffDrift.Add(FVector(Rng.FRandRange(-40.f, 60.f), Rng.FRandRange(-40.f, 40.f), 0.f));
		}
	}
}

void ACHFire::SetBurning(bool bInBurning)
{
	bBurning = bInBurning;
	Light->SetVisibility(bBurning);
	for (UStaticMeshComponent* Flame : Flames)
	{
		Flame->SetVisibility(bBurning);
	}
	if (Halo)
	{
		Halo->SetVisibility(bBurning);
	}
}

void ACHFire::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Time += DeltaSeconds;

	APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	const FVector CameraLocation = Camera ? Camera->GetCameraLocation() : GetActorLocation() + FVector(-1000.f, 0.f, 200.f);
	const FVector Origin = GetActorLocation();

	if (bBurning)
	{
		// light flicker: two noises
		const float Flicker = 0.72f + 0.18f * FMath::PerlinNoise1D(Time * 7.3f) + 0.10f * FMath::Sin(Time * 23.f + 1.3f);
		Light->SetIntensity(BaseIntensity * Flicker);

		for (int32 Index = 0; Index < Flames.Num(); ++Index)
		{
			const float Phase = Index * 1.7f;
			const float Height = (60.f + 45.f * (Index % 3 == 2 ? 0.6f : 1.f)) * Scale * (0.75f + 0.35f * FMath::Abs(FMath::PerlinNoise1D(Time * 3.1f + Phase)));
			const float Radius = (Index % 3 == 2 ? 18.f : 26.f) * Scale;
			const FVector Offset((FMath::Sin(Phase * 2.3f) * 22.f + FMath::Sin(Time * 2.f + Phase) * 5.f) * Scale,
				(FMath::Cos(Phase * 1.9f) * 22.f) * Scale, Height * 0.5f + 8.f * Scale);
			Flames[Index]->SetRelativeLocation(Offset);
			Flames[Index]->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, Height / 100.f));
			Flames[Index]->SetRelativeRotation(FRotator(FMath::Sin(Time * 4.f + Phase) * 6.f, 0.f, FMath::Cos(Time * 3.3f + Phase) * 6.f));
		}

		if (Halo)
		{
			const FVector HaloLocation = Origin + FVector(0.f, 0.f, 60.f * Scale);
			Halo->SetWorldLocation(HaloLocation);
			Halo->SetWorldRotation(FRotationMatrix::MakeFromZ((CameraLocation - HaloLocation).GetSafeNormal()).Rotator());
			const float Size = 3.2f * Scale * (0.9f + 0.1f * Flicker);
			Halo->SetWorldScale3D(FVector(Size, Size, 1.f));
		}
	}

	for (int32 Index = 0; Index < Puffs.Num(); ++Index)
	{
		float& Age = PuffAge[Index];
		Age += DeltaSeconds;
		if (Age > PuffLife[Index])
		{
			Age = 0.f;
			PuffDrift[Index] = FVector(Rng.FRandRange(-40.f, 70.f), Rng.FRandRange(-40.f, 40.f), 0.f);
		}
		const float T = Age / PuffLife[Index];
		const FVector Location = Origin + FVector(0.f, 0.f, (90.f + 520.f * T) * Scale) + PuffDrift[Index] * T * 4.f * Scale;
		Puffs[Index]->SetWorldLocation(Location);
		Puffs[Index]->SetWorldRotation(FRotationMatrix::MakeFromZ((CameraLocation - Location).GetSafeNormal()).Rotator());
		const float Size = (0.8f + 3.4f * T) * Scale;
		Puffs[Index]->SetWorldScale3D(FVector(Size, Size, 1.f));
		const float Alpha = FMath::Sin(T * UE_PI) * (bBurning ? 0.30f : 0.18f);
		PuffMaterials[Index]->SetScalarParameterValue(TEXT("Opacity"), Alpha);
		// lit from below by the fire while low
		const float Warm = bBurning ? FMath::Clamp(1.f - T * 2.5f, 0.f, 1.f) : 0.f;
		PuffMaterials[Index]->SetVectorParameterValue(TEXT("Color"), FMath::Lerp(FLinearColor(0.04f, 0.038f, 0.036f), FLinearColor(0.5f, 0.18f, 0.05f), Warm));
	}
}
