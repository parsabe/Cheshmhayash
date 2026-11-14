// Cheshm Hayash - the women's prison outside the city (3.1). A pale Aban morning.

#include "World/CHLocations.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Math/RandomStream.h"

void ACHLoc_PrisonGate::Build(const FString& InVariant)
{
	Super::Build(InVariant);
	InitCity(false, -0.3f);

	FCHEnv Env;
	// a low sun from the south-west: on the wall, on her face as she comes out
	Env.SunRotation = FRotator(-22.f, 60.f, 0.f);
	Env.SunIntensity = 5.f;
	Env.SunColor = FLinearColor(1.f, 0.95f, 0.88f);
	Env.SkyLightIntensity = 2.3f;
	Env.FogDensity = 0.022f;
	Env.FogHeightFalloff = 0.08f;
	Env.FogColor = FLinearColor(0.32f, 0.35f, 0.40f);
	Env.ExposureEV = -0.3f;
	Env.Grain = 0.2f;
	Env.Vignette = 0.35f;
	Env.Fringe = 0.15f;
	Env.Bloom = 0.5f;
	Env.Saturation = 0.72f;
	Env.Contrast = 0.95f;
	Env.Temperature = 6000.f;
	ApplyEnv(Env);
	if (Sun)
	{
		Sun->SetLightSourceAngle(6.f);
	}
	BuildAlborz(-60.f, FLinearColor(0.32f, 0.34f, 0.38f), FLinearColor(0.75f, 0.77f, 0.8f), 900000.f, true);

	FRandomStream Rng(3101);
	UMaterialInterface* Dust = Lit(FLinearColor(0.30f, 0.27f, 0.22f), 0.95f, 0.6f);
	UMaterialInterface* Concrete = Lit(FLinearColor(0.42f, 0.41f, 0.39f), 0.95f, 0.65f);
	UMaterialInterface* Steel = Lit(FLinearColor(0.13f, 0.17f, 0.14f), 0.6f, 0.5f, 0.3f);
	UMaterialInterface* Wire = Lit(FLinearColor(0.05f, 0.05f, 0.05f), 0.5f, 0.3f, 0.6f);

	BoxAt(-9000.f, -9000.f, -45.f, 18000.f, 18000.f, 40.f, Dust);
	StreetX(-6000.f, 6000.f, -250.f, 900.f, 320.f, 41, true);

	// the wall, the wire on it, the gate
	const float WallY = 900.f;
	const float WallH = 650.f;
	BoxAt(-6000.f, WallY, -20.f, 5680.f, 100.f, WallH + 20.f, Concrete);
	BoxAt(320.f, WallY, -20.f, 5680.f, 100.f, WallH + 20.f, Concrete);
	BoxAt(-420.f, WallY - 20.f, 520.f, 840.f, 140.f, 160.f, Concrete);
	for (float X = -6000.f; X < 6000.f; X += 55.f)
	{
		BoxInstance(FVector(X, WallY + 50.f, WallH + 45.f), FVector(80.f, 2.f, 2.f), Wire, FRotator(0.f, 0.f, 0.f) + FRotator(38.f, 0.f, 0.f));
		BoxInstance(FVector(X + 27.f, WallY + 50.f, WallH + 45.f), FVector(80.f, 2.f, 2.f), Wire, FRotator(-38.f, 0.f, 0.f));
	}
	BoxInstance(FVector(0.f, WallY + 50.f, WallH + 70.f), FVector(12000.f, 2.f, 2.f), Wire);
	BoxInstance(FVector(0.f, WallY + 50.f, WallH + 20.f), FVector(12000.f, 2.f, 2.f), Wire);

	// left leaf, right leaf with a small door in it
	BoxAt(-320.f, WallY + 40.f, 0.f, 320.f, 8.f, 520.f, Steel);
	BoxAt(0.f, WallY + 40.f, 0.f, 120.f, 8.f, 520.f, Steel);
	BoxAt(220.f, WallY + 40.f, 0.f, 100.f, 8.f, 520.f, Steel);
	BoxAt(120.f, WallY + 40.f, 210.f, 100.f, 8.f, 310.f, Steel);
	for (float Z = 60.f; Z < 520.f; Z += 90.f)
	{
		BoxInstance(FVector(0.f, WallY + 36.f, Z), FVector(640.f, 3.f, 8.f), Wire);
	}
	DoorPivot = Pivot(FVector(220.f, WallY + 44.f, 0.f));
	BoxOn(DoorPivot, FVector(-50.f, 0.f, 105.f), FVector(100.f, 7.f, 210.f), Steel, true);
	BoxOn(DoorPivot, FVector(-88.f, -6.f, 105.f), FVector(4.f, 5.f, 14.f), Wire, false);

	// a watchtower, the blocks behind
	BoxAt(820.f, WallY - 40.f, -20.f, 240.f, 240.f, 980.f, Concrete);
	BoxAt(780.f, WallY - 80.f, 960.f, 320.f, 320.f, 220.f, Concrete);
	Box(FVector(940.f, WallY - 81.f, 1080.f), FVector(260.f, 2.f, 70.f), MCarGlass, false);
	for (int32 Index = 0; Index < 5; ++Index)
	{
		FCHBuildingStyle Style;
		Style.Floors = 3;
		Style.Facades = 4;
		Style.bBalconies = false;
		Style.bRoofProps = false;
		Style.Seed = 700 + Index;
		Building(-4200.f + Index * 1800.f, WallY + 900.f, 1500.f, 1200.f, Style);
	}

	// across the road: open, dusty land; low walls
	for (int32 Index = 0; Index < 6; ++Index)
	{
		BoxAt(-5000.f + Index * 1900.f + Rng.FRandRange(0.f, 400.f), -2600.f - Rng.FRandRange(0.f, 900.f), -20.f, Rng.FRandRange(500.f, 1200.f), 30.f, Rng.FRandRange(120.f, 220.f), Concrete);
	}
	Lamp(FVector(-1500.f, 400.f, 14.f), 180.f, false);
	Lamp(FVector(1800.f, 400.f, 14.f), 180.f, false);
	Car(FVector(-900.f, -560.f, 0.f), 0.f, FLinearColor(0.52f, 0.52f, 0.5f));

	// where they stand, the cameras (the faces are shot three-quarter, never through a back)
	const FVector Brother(-60.f, 560.f, 14.f);
	const FVector Meet(10.f, 680.f, 14.f);
	Mark(TEXT("PlayerStart"), FVector(-900.f, -1100.f, 0.f));
	Mark(TEXT("BrotherSpot"), Brother, FRotator(0.f, 90.f, 0.f));
	Mark(TEXT("Inside"), FVector(170.f, 1250.f, 0.f), FRotator(0.f, -90.f, 0.f));
	Mark(TEXT("Door"), FVector(170.f, 880.f, 0.f), FRotator(0.f, -90.f, 0.f));
	Mark(TEXT("Meet"), Meet, FRotator(0.f, -120.f, 0.f));
	MarkLookAt(TEXT("Wide"), FVector(-700.f, -1500.f, 260.f), FVector(60.f, 900.f, 300.f), 55.f);
	MarkLookAt(TEXT("Gate"), FVector(-430.f, 360.f, 170.f), FVector(170.f, 940.f, 120.f), 44.f);
	MarkLookAt(TEXT("TwoShot"), FVector(260.f, 450.f, 150.f), FVector(-25.f, 620.f, 145.f), 40.f);
	// medium shots over the other one's shoulder: the sun on her; behind him
	MarkLookAt(TEXT("SisterFace"), FVector(-148.f, 518.f, 162.f), Meet + FVector(0.f, 0.f, 128.f), 40.f);
	MarkLookAt(TEXT("BrotherFace"), FVector(98.f, 722.f, 164.f), Brother + FVector(0.f, 0.f, 132.f), 40.f);
}

void ACHLoc_PrisonGate::OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat)
{
	if (Event == TEXT("gate_open"))
	{
		DoorTarget = -95.f;
	}
}

void ACHLoc_PrisonGate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (DoorPivot && !FMath::IsNearlyEqual(DoorAngle, DoorTarget, 0.01f))
	{
		DoorAngle = FMath::FInterpTo(DoorAngle, DoorTarget, DeltaSeconds, 1.6f);
		DoorPivot->SetRelativeRotation(FRotator(0.f, DoorAngle, 0.f));
	}
}
