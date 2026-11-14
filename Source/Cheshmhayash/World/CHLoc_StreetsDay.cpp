// Cheshm Hayash - the city by day (1.2 the run, 1.5 the taxi), the university yard (2.1)
// and the parking garage under it (2.2).

#include "World/CHLocations.h"
#include "Camera/CameraActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Game/CHGuard.h"
#include "Game/CHInteractable.h"
#include "Game/CHSegment.h"
#include "Math/RandomStream.h"
#include "Story/CHStoryDirector.h"
#include "UI/CHStyle.h"
#include "World/CHNpc.h"

namespace
{
	const float DayRoad = 900.f;
	const float DayWalk = 450.f;
	const float DayEdge = DayRoad * 0.5f + DayWalk;
	const float TaxiEnd = 9300.f;

	const FVector GarageMin(-3000.f, -1800.f, 0.f);
	const FVector GarageMax(3000.f, 1800.f, 330.f);

	FCHEnv DayEnv(bool bHarsh)
	{
		FCHEnv Env;
		// 14:20 in Shahrivar: a hard white sun. The university morning is softer.
		Env.SunRotation = bHarsh ? FRotator(-56.f, 10.f, 0.f) : FRotator(-34.f, 120.f, 0.f);
		Env.SunIntensity = bHarsh ? 8.f : 6.f;
		Env.SunColor = bHarsh ? FLinearColor(1.f, 0.96f, 0.88f) : FLinearColor(1.f, 0.93f, 0.84f);
		Env.SkyLightIntensity = 2.8f;
		Env.FogDensity = 0.012f;
		Env.FogHeightFalloff = 0.08f;
		Env.FogColor = FLinearColor(0.40f, 0.42f, 0.44f);
		Env.ExposureEV = -0.7f;
		Env.Grain = 0.22f;
		Env.Vignette = 0.38f;
		Env.Fringe = 0.2f;
		Env.Bloom = 0.5f;
		Env.Saturation = bHarsh ? 0.9f : 0.82f;
		Env.Contrast = bHarsh ? 1.06f : 1.f;
		Env.Temperature = bHarsh ? 6700.f : 6300.f;
		return Env;
	}
}

// ---------------------------------------------------------------------------------------------
// 1.2 / 1.5: one long avenue in daylight
// ---------------------------------------------------------------------------------------------

void ACHLoc_Streets::BuildDay()
{
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	FRandomStream Rng(2201);

	ApplyEnv(DayEnv(true));
	BuildAlborz(55.f, FLinearColor(0.34f, 0.37f, 0.44f), FLinearColor(0.85f, 0.86f, 0.88f), 900000.f, true);

	Ground(-3500.f, -4500.f, 13500.f, 4500.f);
	StreetX(-900.f, 11400.f, 0.f, DayRoad, DayWalk, 17);

	// blocks on both sides, shops at street level
	float X = -900.f;
	int32 Index = 0;
	while (X < 11400.f)
	{
		const float Width = Rng.FRandRange(1200.f, 1900.f);
		for (int32 Side = 0; Side < 2; ++Side)
		{
			FCHBuildingStyle Style;
			Style.Floors = Rng.RandRange(4, 6);
			Style.Facades = Side == 0 ? 4 : 8;
			Style.Shops = Style.Facades;
			Style.Seed = 600 + Index * 2 + Side;
			Building(X, Side == 0 ? DayEdge : -DayEdge - 1500.f, Width - 20.f, 1500.f, Style);
		}
		X += Width;
		++Index;
	}
	// the avenue is closed behind him and far ahead
	for (int32 End = 0; End < 2; ++End)
	{
		FCHBuildingStyle Style;
		Style.Floors = 6;
		Style.Facades = End == 0 ? 2 : 1;
		Style.Seed = 690 + End;
		Building(End == 0 ? -2400.f : 11400.f, -DayEdge - 1500.f, 1500.f, 2.f * (DayEdge + 1500.f), Style);
	}

	for (float LampX = 300.f; LampX < 11000.f; LampX += 1400.f)
	{
		const bool bNorth = (FMath::RoundToInt32(LampX / 1400.f) % 2) == 0;
		Lamp(FVector(LampX, bNorth ? DayEdge - 70.f : -DayEdge + 70.f, 14.f), bNorth ? -90.f : 90.f, false);
	}
	for (float WireX = 900.f; WireX < 10800.f; WireX += 2300.f)
	{
		Wires(FVector(WireX, -DayEdge, 620.f), FVector(WireX + 400.f, DayEdge, 640.f), 140.f);
	}

	// parked cars at both kerbs; the afternoon traffic standing in the road
	const FLinearColor CarColors[] = { FLinearColor(0.62f, 0.62f, 0.60f), FLinearColor(0.08f, 0.08f, 0.09f), FLinearColor(0.30f, 0.04f, 0.03f), FLinearColor(0.34f, 0.35f, 0.38f), FLinearColor(0.55f, 0.50f, 0.36f) };
	for (float CarX = 500.f; CarX < 10900.f; CarX += Rng.FRandRange(600.f, 1100.f))
	{
		Car(FVector(CarX, -DayRoad * 0.5f + 110.f, 0.f), 0.f, CarColors[Rng.RandRange(0, 4)]);
		if (Rng.FRand() < 0.7f)
		{
			Car(FVector(CarX + Rng.FRandRange(-200.f, 200.f), DayRoad * 0.5f - 110.f, 0.f), 180.f, CarColors[Rng.RandRange(0, 4)]);
		}
	}
	for (float CarX = 1500.f; CarX < 10000.f; CarX += Rng.FRandRange(1300.f, 2200.f))
	{
		Car(FVector(CarX, Rng.FRand() < 0.5f ? 150.f : -130.f, 0.f), Rng.FRandRange(-4.f, 4.f), CarColors[Rng.RandRange(0, 4)]);
	}
	// the van, gone on ahead of him
	Van(FVector(10300.f, 120.f, 0.f), 0.f);

	// people on the pavements, going about an ordinary afternoon
	const TCHAR* Looks[] = { TEXT("woman"), TEXT("man"), TEXT("chador"), TEXT("student_f"), TEXT("oldwoman"), TEXT("student_m"), TEXT("man"), TEXT("woman") };
	for (int32 Person = 0; Person < 26; ++Person)
	{
		const bool bNorth = Rng.FRand() < 0.5f;
		const FVector At(Rng.FRandRange(200.f, 10800.f), (bNorth ? 1.f : -1.f) * Rng.FRandRange(DayRoad * 0.5f + 90.f, DayEdge - 60.f), 14.f);
		SpawnNpc(FString::Printf(TEXT("passer%d"), Person), Looks[Person % UE_ARRAY_COUNT(Looks)], At, Rng.FRand() < 0.5f ? 0.f : 180.f, 700 + Person);
	}

	Mark(TEXT("PlayerStart"), FVector(-300.f, DayRoad * 0.5f + 200.f, 14.f), FRotator(0.f, 0.f, 0.f));
	MarkLookAt(TEXT("Establish"), FVector(-500.f, DayRoad * 0.5f + 60.f, 210.f), FVector(2500.f, 100.f, 260.f), 66.f);
	// the back seat of a taxi, looking out past the driver's side
	Mark(TEXT("Taxi"), FVector(-200.f, -150.f, 128.f), FRotator(-1.f, 24.f, 0.f), 62.f);
	if (Segment)
	{
		Segment->AddExit(FBox(FVector(9700.f, -DayEdge, -100.f), FVector(10100.f, DayEdge, 600.f)), TEXT("end"));
	}
}

// ---------------------------------------------------------------------------------------------
// 2.1: the university yard
// ---------------------------------------------------------------------------------------------

void ACHLoc_Streets::BuildCampus()
{
	FRandomStream Rng(2101);

	ApplyEnv(DayEnv(false));
	BuildAlborz(80.f, FLinearColor(0.34f, 0.37f, 0.44f), FLinearColor(0.85f, 0.86f, 0.88f), 900000.f, true);

	UMaterialInterface* Paving = Lit(FLinearColor(0.42f, 0.40f, 0.36f), 0.9f, 0.5f);
	UMaterialInterface* Step = Lit(FLinearColor(0.50f, 0.48f, 0.44f), 0.85f, 0.4f);
	UMaterialInterface* Brick = Lit(FLinearColor(0.38f, 0.22f, 0.15f), 0.9f, 0.4f);
	UMaterialInterface* Wood = Lit(FLinearColor(0.20f, 0.12f, 0.06f), 0.7f, 0.3f);
	UMaterialInterface* Card = Lit(FLinearColor(0.45f, 0.33f, 0.20f), 0.95f, 0.2f);
	const FLinearColor Red(0.55f, 0.02f, 0.015f);

	Ground(-5000.f, -5000.f, 5000.f, 5000.f);
	BoxAt(-2600.f, -2200.f, 0.f, 5200.f, 3700.f, 3.f, Paving);

	// the faculty building across the top of the yard, wide steps up to its doors
	{
		FCHBuildingStyle Style;
		Style.Floors = 4;
		Style.Facades = 4;
		Style.bBalconies = false;
		Style.bRoofProps = false;
		Style.Seed = 810;
		Building(-2600.f, 1500.f, 5200.f, 1500.f, Style);
	}
	const int32 Steps = 6;
	for (int32 Index = 0; Index < Steps; ++Index)
	{
		BoxAt(-900.f, 1500.f - 70.f * (Index + 1), 0.f, 1800.f, 70.f, 17.f * (Steps - Index), Step);
	}
	BoxAt(-160.f, 1494.f, 17.f * Steps, 320.f, 8.f, 290.f, Lit(FLinearColor(0.07f, 0.05f, 0.04f), 0.6f, 0.2f), false);

	// red paint, on the steps and on the wall
	WorldText(TEXT("زن · زندگی · آزادی"), true, 150.f, Red, FVector(0.f, 1500.f - 70.f * Steps - 190.f, 4.f),
		FRotationMatrix::MakeFromXZ(FVector::UpVector, FVector(0.f, 1.f, 0.f)).Rotator(), 1.f, false, true);
	Graffiti(TEXT("زن · زندگی · آزادی"), FVector(1500.f, 1498.f, 240.f), -90.f, 84.f, Red);
	Graffiti(TEXT("ZAN · ZENDEGI · AZADI"), FVector(-1500.f, 1498.f, 230.f), -90.f, 60.f, Red, false);

	// the wings closing the yard, a wall with the gate behind him
	for (int32 Side = 0; Side < 2; ++Side)
	{
		FCHBuildingStyle Style;
		Style.Floors = 3;
		Style.Facades = Side == 0 ? 2 : 1;
		Style.bBalconies = false;
		Style.Seed = 820 + Side;
		Building(Side == 0 ? -4000.f : 2600.f, -2200.f, 1400.f, 3700.f, Style);
	}
	BoxAt(-2600.f, -2260.f, 0.f, 2300.f, 60.f, 260.f, Brick);
	BoxAt(300.f, -2260.f, 0.f, 2300.f, 60.f, 260.f, Brick);
	// the gate is shut behind him: nobody is going to class, nobody is going home
	BoxAt(-300.f, -2250.f, 0.f, 600.f, 20.f, 240.f, MShutter);

	for (int32 Tree = 0; Tree < 6; ++Tree)
	{
		PlaneTree(FVector(-2000.f + Tree * 800.f, Tree % 2 ? -1300.f : -300.f, 0.f), Rng.FRandRange(700.f, 900.f), 830 + Tree);
	}
	for (int32 Bench = 0; Bench < 4; ++Bench)
	{
		const FVector At(-1800.f + Bench * 1200.f, -800.f, 0.f);
		Box(At + FVector(0.f, 0.f, 42.f), FVector(180.f, 45.f, 6.f), Wood);
		Box(At + FVector(-75.f, 0.f, 20.f), FVector(8.f, 40.f, 40.f), MPole, false);
		Box(At + FVector(75.f, 0.f, 20.f), FVector(8.f, 40.f, 40.f), MPole, false);
	}

	// students in the yard and on the steps, not going in
	const TCHAR* Looks[] = { TEXT("student_f"), TEXT("student_m"), TEXT("girl"), TEXT("student_m"), TEXT("student_f"), TEXT("woman") };
	for (int32 Index = 0; Index < 30; ++Index)
	{
		const bool bOnSteps = Index < 9;
		FVector At;
		float Sink = 0.f;
		float Yaw;
		if (bOnSteps)
		{
			const int32 StepIndex = 1 + Index % 4;
			At = FVector(-760.f + Index * 180.f, 1500.f - 70.f * StepIndex - 35.f, 17.f * (Steps - StepIndex));
			Sink = 42.f;
			Yaw = -90.f + Rng.FRandRange(-25.f, 25.f);
		}
		else
		{
			// loose circles
			const int32 Group = (Index - 9) / 4;
			const FVector Center(-1900.f + Group * 780.f, Group % 2 ? 250.f : -500.f, 3.f);
			const float Angle = (Index % 4) * 90.f + Group * 31.f;
			At = Center + FRotator(0.f, Angle, 0.f).Vector() * 95.f;
			Yaw = Angle + 180.f;
		}
		if (ACHNpc* Student = SpawnNpc(FString::Printf(TEXT("yard%d"), Index), Looks[Index % UE_ARRAY_COUNT(Looks)], At, Yaw, 840 + Index))
		{
			Student->SetSink(Sink);
		}
	}

	// three people with something to say
	const FVector Professor(-1150.f, 700.f, 3.f);
	SpawnNpc(TEXT("professor"), TEXT("father"), Professor, -70.f, 871);
	Box(Professor + FVector(45.f, -30.f, 20.f), FVector(48.f, 36.f, 34.f), Card, false, FRotator(0.f, 20.f, 0.f));
	Box(Professor + FVector(45.f, -30.f, 39.f), FVector(30.f, 22.f, 5.f), Lit(FLinearColor(0.2f, 0.05f, 0.04f), 0.8f, 0.2f), false, FRotator(0.f, 35.f, 0.f));
	SpawnTalk(TEXT("2.1.professor"), TEXT("با استاد حرف بزن"), TEXT("Talk to the professor"), Professor + FVector(0.f, 0.f, 90.f));

	const FVector Girl(420.f, 880.f, 3.f);
	SpawnNpc(TEXT("shorthair"), TEXT("girl"), Girl, -110.f, 872);
	SpawnTalk(TEXT("2.1.girl"), TEXT("با دختر حرف بزن"), TEXT("Talk to the girl"), Girl + FVector(0.f, 0.f, 90.f));

	const FVector Boy(1500.f, 150.f, 3.f);
	SpawnNpc(TEXT("phoneboy"), TEXT("student_m"), Boy, 200.f, 873);
	Box(Boy + FVector(-22.f, -8.f, 118.f), FVector(1.f, 7.f, 14.f), Emissive(FLinearColor(0.5f, 0.6f, 0.8f), CHLight::Emissive(0.8f)), false, FRotator(-20.f, 20.f, 0.f));
	SpawnTalk(TEXT("2.1.boy"), TEXT("با پسر حرف بزن"), TEXT("Talk to the boy"), Boy + FVector(0.f, 0.f, 90.f));

	Mark(TEXT("PlayerStart"), FVector(0.f, -1900.f, 3.f), FRotator(0.f, 90.f, 0.f));
	MarkLookAt(TEXT("Establish"), FVector(-300.f, -2000.f, 420.f), FVector(0.f, 1300.f, 150.f), 62.f);
	MarkLookAt(TEXT("Steps"), FVector(250.f, 300.f, 190.f), FVector(0.f, 1400.f, 90.f), 50.f);
	MarkLookAt(TEXT("ProfessorCam"), Professor + FVector(190.f, -230.f, 160.f), Professor + FVector(0.f, 0.f, 130.f), 42.f);
	MarkLookAt(TEXT("GirlCam"), Girl + FVector(-200.f, -220.f, 160.f), Girl + FVector(0.f, 0.f, 132.f), 42.f);
	MarkLookAt(TEXT("BoyCam"), Boy + FVector(-250.f, 150.f, 160.f), Boy + FVector(0.f, 0.f, 130.f), 42.f);
}

// ---------------------------------------------------------------------------------------------
// 2.2: the parking garage
// ---------------------------------------------------------------------------------------------

void ACHLoc_Streets::BuildGarage()
{
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	FRandomStream Rng(2202);

	NightEnv(-2.1f, 0.012f);

	UMaterialInterface* Concrete = Lit(FLinearColor(0.20f, 0.20f, 0.19f), 0.9f, 0.6f);
	UMaterialInterface* Floor = Lit(FLinearColor(0.09f, 0.09f, 0.09f), 0.7f, 0.6f);
	UMaterialInterface* Paint = Lit(FLinearColor(0.55f, 0.48f, 0.12f), 0.7f, 0.5f);
	UMaterialInterface* Tube = Emissive(FLinearColor(0.75f, 0.85f, 1.f), CHLight::Emissive(2.2f), FLinearColor(0.2f, 0.2f, 0.2f));
	UMaterialInterface* TubeDead = Lit(FLinearColor(0.25f, 0.25f, 0.25f), 0.5f, 0.3f);

	const FVector Size = GarageMax - GarageMin;
	BoxAt(GarageMin.X - 400.f, GarageMin.Y - 400.f, -40.f, Size.X + 1600.f, Size.Y + 800.f, 40.f, Floor);
	BoxAt(GarageMin.X - 60.f, GarageMin.Y - 60.f, GarageMax.Z, Size.X + 1300.f, Size.Y + 120.f, 60.f, Concrete);
	BoxAt(GarageMin.X - 60.f, GarageMin.Y - 60.f, 0.f, 60.f, Size.Y + 120.f, GarageMax.Z, Concrete);
	BoxAt(GarageMin.X, GarageMin.Y - 60.f, 0.f, Size.X + 1200.f, 60.f, GarageMax.Z, Concrete);
	BoxAt(GarageMin.X, GarageMax.Y, 0.f, Size.X + 1200.f, 60.f, GarageMax.Z, Concrete);
	// the far wall, open in the middle where the ramp climbs to the street
	BoxAt(GarageMax.X, GarageMin.Y, 0.f, 60.f, Size.Y * 0.5f - 260.f, GarageMax.Z, Concrete);
	BoxAt(GarageMax.X, 260.f, 0.f, 60.f, Size.Y * 0.5f - 260.f, GarageMax.Z, Concrete);
	BoxAt(GarageMax.X, -260.f, 0.f, 1200.f, 20.f, GarageMax.Z, Concrete);
	BoxAt(GarageMax.X, 240.f, 0.f, 1200.f, 20.f, GarageMax.Z, Concrete);
	// the night outside, at the top of the ramp
	Box(FVector(GarageMax.X + 1190.f, 0.f, 165.f), FVector(6.f, 480.f, 330.f), Emissive(FLinearColor(0.35f, 0.45f, 0.7f), CHLight::Emissive(0.5f)), false);
	PointLight(FVector(GarageMax.X + 900.f, 0.f, 220.f), FLinearColor(0.5f, 0.62f, 1.f), 6.f, 1500.f, false, 1.5f);

	// pillars, painted bays, tubes of light: most of them dead
	int32 TubeIndex = 0;
	for (float X = GarageMin.X + 600.f; X < GarageMax.X; X += 800.f)
	{
		for (float Y = GarageMin.Y + 600.f; Y < GarageMax.Y; Y += 1200.f)
		{
			BoxAt(X - 25.f, Y - 25.f, 0.f, 50.f, 50.f, GarageMax.Z, Concrete);
			BoxInstance(FVector(X, Y, 60.f), FVector(52.f, 52.f, 30.f), Paint);
		}
		const bool bLit = (TubeIndex++ % 4) != 2;
		Box(FVector(X - 400.f, 0.f, GarageMax.Z - 6.f), FVector(12.f, 240.f, 8.f), bLit ? Tube : TubeDead, false);
		if (bLit)
		{
			PointLight(FVector(X - 400.f, 0.f, GarageMax.Z - 50.f), FLinearColor(0.72f, 0.82f, 1.f), 7.f, 1500.f, true, 0.6f);
			if (Segment)
			{
				Segment->AddLightPool(FVector(X - 400.f, 0.f, 0.f), 330.f);
			}
		}
	}
	for (float X = GarageMin.X + 200.f; X < GarageMax.X; X += 270.f)
	{
		BoxInstance(FVector(X, GarageMin.Y + 260.f, 0.6f), FVector(8.f, 520.f, 1.f), Paint);
		BoxInstance(FVector(X, GarageMax.Y - 260.f, 0.6f), FVector(8.f, 520.f, 1.f), Paint);
	}

	// cars left where they were parked this morning
	const FLinearColor CarColors[] = { FLinearColor(0.5f, 0.5f, 0.48f), FLinearColor(0.06f, 0.06f, 0.07f), FLinearColor(0.25f, 0.03f, 0.02f), FLinearColor(0.3f, 0.3f, 0.32f) };
	for (float X = GarageMin.X + 335.f; X < GarageMax.X - 200.f; X += 270.f)
	{
		if (Rng.FRand() < 0.62f)
		{
			Car(FVector(X, GarageMin.Y + 270.f, 0.f), 90.f, CarColors[Rng.RandRange(0, 3)]);
		}
		if (Rng.FRand() < 0.62f)
		{
			Car(FVector(X, GarageMax.Y - 270.f, 0.f), -90.f, CarColors[Rng.RandRange(0, 3)]);
		}
	}
	// a middle island of cars: cover between the two aisles
	for (float X = GarageMin.X + 700.f; X < GarageMax.X - 900.f; X += 560.f)
	{
		if (Rng.FRand() < 0.75f)
		{
			Car(FVector(X, Rng.FRand() < 0.5f ? -140.f : 140.f, 0.f), Rng.FRand() < 0.5f ? 0.f : 180.f, CarColors[Rng.RandRange(0, 3)]);
			if (Segment)
			{
				Segment->AddHidingSpot(FVector(X, 0.f, 0.f), 150.f);
			}
		}
	}

	// three students who did not get out
	GarageOut = FVector(GarageMax.X + 1000.f, 0.f, 0.f);
	const FVector Hiding[] = { FVector(-1500.f, 1150.f, 0.f), FVector(350.f, -1180.f, 0.f), FVector(1900.f, 1180.f, 0.f) };
	const TCHAR* Looks[] = { TEXT("student_f"), TEXT("student_m"), TEXT("girl") };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Hiding); ++Index)
	{
		if (ACHNpc* Student = SpawnNpc(FString::Printf(TEXT("trapped%d"), Index), Looks[Index], Hiding[Index], Index == 1 ? 90.f : -90.f, 880 + Index))
		{
			Student->SetSink(38.f);
		}
		SpawnInteractable(FString::Printf(TEXT("trapped%d"), Index), TEXT("راه را نشانش بده"), TEXT("Show the way out"), Hiding[Index] + FVector(0.f, 0.f, 70.f), 200.f);
		if (Segment)
		{
			Segment->AddHidingSpot(Hiding[Index], 170.f);
		}
	}
	GarageLeft = UE_ARRAY_COUNT(Hiding);

	// they walk the aisles with flashlights; a motorcycle circles
	SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(-2200.f, 620.f, 0.f), FVector(900.f, 620.f, 0.f) }, 0.f, 21);
	SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(2400.f, -620.f, 0.f), FVector(-900.f, -620.f, 0.f) }, 180.f, 22);
	SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(2300.f, 700.f, 0.f), FVector(2300.f, -700.f, 0.f) }, -90.f, 23);
	SpawnGuard(ECHGuardKind::Motorcycle, TEXT("basij"), { FVector(-2500.f, -640.f, 0.f), FVector(2500.f, -640.f, 0.f), FVector(2500.f, 640.f, 0.f), FVector(-2500.f, 640.f, 0.f) }, 0.f, 24);

	Mark(TEXT("PlayerStart"), FVector(-2700.f, -1250.f, 0.f), FRotator(0.f, 20.f, 0.f));
	MarkLookAt(TEXT("Establish"), FVector(-2850.f, -1500.f, 250.f), FVector(0.f, 0.f, 80.f), 72.f);
	if (Segment)
	{
		Segment->AddCheckpoint(FBox(FVector(-200.f, GarageMin.Y, -100.f), FVector(200.f, GarageMax.Y, 400.f)), FTransform(FRotator::ZeroRotator, FVector(0.f, -1350.f, 108.f)));
		Segment->RequestGrid(FBox2D(FVector2D(GarageMin.X, GarageMin.Y), FVector2D(GarageMax.X + 1200.f, GarageMax.Y)), 50.f, 0.f);
	}
}

void ACHLoc_Streets::SendStudentOut(const FString& Id, ACHInteractable* What)
{
	UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	What->SetEnabled(false);
	if (ACHNpc* Student = FindNpc(Id))
	{
		Student->SetSink(0.f);
		Student->WalkTo(GarageOut, 400.f);
		GarageRunners.Add(Student);
	}
	--GarageLeft;
	if (Director)
	{
		if (GarageLeft > 0)
		{
			Director->Toast(TEXT("برو. از کنار دیوار. نایست."), TEXT("Go. Along the wall. Don't stop."), CH::Col::Paper, 3.f);
		}
		else if (Director->GetFlag(TEXT("walk_offer")) == TEXT("A"))
		{
			// he offered to walk her, that morning
			Director->Toast(TEXT("اون روز باهاش نرفتم. این یکی رو تنها نمی‌ذارم."), TEXT("I didn't walk her. I'm not leaving this one."), CH::Col::Paper, 4.5f);
		}
		else
		{
			Director->Toast(TEXT("آخری. برو."), TEXT("The last one. Go."), CH::Col::Paper, 3.f);
		}
	}
	if (GarageLeft <= 0 && Segment)
	{
		// all of them are out: now him
		Segment->AddExit(FBox(FVector(GarageMax.X + 500.f, -260.f, -100.f), FVector(GarageMax.X + 1200.f, 260.f, 400.f)), TEXT("garage_out"));
		Segment->SetHint(TEXT("همه رفتند. حالا تو. از رمپ برو بالا."), TEXT("They're all out. Now you. Up the ramp."));
	}
}

void ACHLoc_Streets::TickDay(float DeltaSeconds)
{
	// the taxi crawls up the avenue
	if (bTaxi)
	{
		if (ACameraActor* Camera = GetCamera(TEXT("Taxi")))
		{
			FVector Location = Camera->GetActorLocation();
			Location.X = FMath::Min(TaxiEnd, Location.X + 190.f * DeltaSeconds);
			Location.Z = 128.f + FMath::Sin(StreetsTime * 7.f) * 0.5f;
			Camera->SetActorLocation(Location);
		}
	}
	// the students he sent out are gone once they reach the ramp
	for (int32 Index = GarageRunners.Num() - 1; Index >= 0; --Index)
	{
		if (!GarageRunners[Index] || !GarageRunners[Index]->IsMoving())
		{
			if (GarageRunners[Index])
			{
				GarageRunners[Index]->SetActorHiddenInGame(true);
			}
			GarageRunners.RemoveAt(Index);
		}
	}
}
