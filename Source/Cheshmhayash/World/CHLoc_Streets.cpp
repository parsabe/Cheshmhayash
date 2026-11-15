// Cheshm Hayash - the city at night.

#include "World/CHLocations.h"
#include "Game/CHGuard.h"
#include "Game/CHSegment.h"
#include "Math/RandomStream.h"

void ACHLoc_Streets::Build(const FString& InVariant)
{
	Super::Build(InVariant);
	const bool bDay = Variant == TEXT("day") || Variant == TEXT("campus") || Variant == TEXT("massacre");
	InitCity(!bDay, Variant == TEXT("massacre") ? -0.4f : (bDay ? -0.7f : -1.4f));

	PlayerBoomLength = 330.f;
	PlayerBoomOffset = FVector(0.f, 55.f, 75.f);

	if (Variant == TEXT("nights"))
	{
		BuildNights();
	}
	else if (Variant == TEXT("lastnight"))
	{
		BuildLastNight();
	}
	else if (Variant == TEXT("day"))
	{
		BuildDay();
	}
	else if (Variant == TEXT("campus"))
	{
		BuildCampus();
	}
	else if (Variant == TEXT("garage"))
	{
		BuildGarage();
	}
	else if (Variant == TEXT("massacre"))
	{
		BuildMassacre();
	}
	else
	{
		BuildTest();
	}
}

void ACHLoc_Streets::BuildTest()
{
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	FRandomStream Rng(1401);

	NightEnv(-1.4f, 0.035f);
	BuildAlborz(12.f, FLinearColor(0.010f, 0.012f, 0.020f), FLinearColor(0.05f, 0.055f, 0.07f));

	// one long street (along +X), a dead end behind him, a cross street at the far end
	const float Road = 800.f;
	const float Walk = 450.f;
	const float Edge = Road * 0.5f + Walk; // building line
	Ground(-3000.f, -4000.f, 11000.f, 4000.f);
	StreetX(-900.f, 7700.f, 0.f, Road, Walk, 7);
	StreetY(-3800.f, 3800.f, 8100.f, Road, Walk, 8);

	// north side blocks (facing -Y), with an alley
	struct FBlock
	{
		float X0;
		float X1;
		int32 Floors;
		uint8 Shops;
	};
	const FBlock North[] = { { -900.f, 700.f, 5, 4 }, { 700.f, 2000.f, 4, 4 }, { 2000.f, 2900.f, 6, 0 }, { 3350.f, 5000.f, 5, 4 }, { 5000.f, 6400.f, 6, 4 }, { 6400.f, 7700.f - Walk, 4, 4 } };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(North); ++Index)
	{
		FCHBuildingStyle Style;
		Style.Floors = North[Index].Floors;
		// the third block shows the street a blank side wall: the one with the painted-over mural
		Style.Facades = Index == 2 ? (1 | 2) : (4 | 1 | 2);
		Style.Shops = North[Index].Shops;
		Style.Seed = 100 + Index;
		Building(North[Index].X0, Edge, North[Index].X1 - North[Index].X0, 1500.f, Style);
	}
	// the alley's back wall
	BoxAt(2900.f, Edge + 1450.f, -20.f, 450.f, 50.f, 420.f, MFacades[2]);

	// south side blocks (facing +Y), with a side street where the van waits
	const FBlock South[] = { { -900.f, 900.f, 5, 8 }, { 900.f, 2600.f, 5, 8 }, { 2600.f, 4300.f, 6, 8 }, { 4300.f, 5200.f, 4, 8 }, { 5900.f, 7700.f - Walk, 5, 8 } };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(South); ++Index)
	{
		FCHBuildingStyle Style;
		Style.Floors = South[Index].Floors;
		Style.Facades = 8 | 1 | 2;
		Style.Shops = South[Index].Shops;
		Style.Seed = 200 + Index;
		Building(South[Index].X0, -Edge - 1500.f, South[Index].X1 - South[Index].X0, 1500.f, Style);
	}
	StreetY(-3800.f, -Edge, 5550.f, 450.f, 125.f, 9, false);

	// behind him: a block closes the street
	{
		FCHBuildingStyle Style;
		Style.Floors = 6;
		Style.Facades = 2;
		Style.Seed = 300;
		Building(-2400.f, -Edge - 1500.f, 1500.f, 2.f * (Edge + 1500.f), Style);
	}
	// beyond the cross street
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FCHBuildingStyle Style;
		Style.Floors = 5 + Index % 2;
		Style.Facades = 1;
		Style.Shops = 1;
		Style.Seed = 400 + Index;
		Building(8100.f + Edge, -3800.f + Index * 2533.f, 1400.f, 2533.f, Style);
	}

	// lamps on alternating sides; some are broken, one flickers
	int32 LampIndex = 0;
	for (float X = 300.f; X < 7400.f; X += 1250.f, ++LampIndex)
	{
		const bool bNorth = (LampIndex % 2) == 0;
		const bool bWorking = LampIndex != 2 && LampIndex != 5;
		Lamp(FVector(X, bNorth ? Edge - 70.f : -Edge + 70.f, 14.f), bNorth ? -90.f : 90.f, bWorking, LampIndex == 3);
	}

	// power cables over the street
	for (float X = 900.f; X < 7400.f; X += 2100.f)
	{
		Wires(FVector(X, -Edge, 620.f), FVector(X + 400.f, Edge, 640.f), 140.f);
		Wires(FVector(X, -Edge, 660.f), FVector(X + 420.f, Edge, 690.f), 160.f);
	}

	// parked cars along the south kerb, a burnt one in the road
	const FLinearColor CarColors[] = { FLinearColor(0.5f, 0.5f, 0.48f), FLinearColor(0.08f, 0.08f, 0.09f), FLinearColor(0.25f, 0.03f, 0.02f), FLinearColor(0.3f, 0.3f, 0.32f) };
	for (float X = 600.f; X < 7200.f; X += Rng.FRandRange(650.f, 1300.f))
	{
		if (X > 5200.f && X < 5950.f)
		{
			continue;
		}
		Car(FVector(X, -Road * 0.5f + 110.f, 0.f), (Rng.FRand() < 0.5f) ? 0.f : 180.f, CarColors[Rng.RandRange(0, 3)]);
	}
	Car(FVector(4700.f, 140.f, 0.f), 23.f, FLinearColor::Black, true);

	// bins to hide behind
	Dumpster(FVector(1900.f, -Edge + 160.f, 14.f), 0.f);
	Dumpster(FVector(4150.f, Edge - 160.f, 14.f), 180.f);
	Dumpster(FVector(3120.f, Edge + 900.f, 0.f), 90.f);

	// a fire in the road, and a wall: what is worth filming
	Fire(FVector(3800.f, 120.f, 0.f), 1.3f, true, 3);
	SpawnFilmable(TEXT("test_fire"), TEXT("آتش وسط خیابان"), TEXT("A fire in the middle of the street"), FVector(3800.f, 120.f, 120.f), 3.f);

	const FVector WallAt(2450.f, Edge - 1.f, 250.f);
	PaintedOver(WallAt + FVector(0.f, -1.f, 0.f), -90.f, FVector2D(760.f, 300.f));
	Graffiti(TEXT("زن، زندگی، آزادی"), WallAt + FVector(0.f, -6.f, 10.f), -90.f, 72.f, FLinearColor(0.55f, 0.02f, 0.015f));
	SpawnFilmable(TEXT("test_wall"), TEXT("دیوار: زن، زندگی، آزادی"), TEXT("A wall: Woman, Life, Freedom"), WallAt + FVector(0.f, -40.f, 0.f), 3.f);

	// the morality police van in the side street, a watcher by it
	Van(FVector(5550.f, -Edge - 600.f, 0.f), 90.f);
	if (ACHGuard* Watcher = SpawnGuard(ECHGuardKind::Watcher, TEXT("morality_f"), { FVector(5550.f, -Edge + 80.f, 14.f) }, 90.f))
	{
		Watcher->SetScan(35.f, 145.f, 8.f);
	}

	// riot police on foot
	SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(1200.f, Edge - 220.f, 14.f), FVector(2650.f, Edge - 220.f, 14.f) }, 0.f, 1);
	SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(4300.f, -280.f, 0.f), FVector(4300.f, 280.f, 0.f), FVector(4900.f, 280.f, 0.f), FVector(4900.f, -280.f, 0.f) }, 90.f, 2);
	// a man who looks like anyone
	SpawnGuard(ECHGuardKind::Plainclothes, TEXT("man"), { FVector(3200.f, Edge - 260.f, 14.f), FVector(3650.f, Edge - 300.f, 14.f) }, 0.f, 3);
	// Basij on a motorcycle, up and down the street
	SpawnGuard(ECHGuardKind::Motorcycle, TEXT("basij"), { FVector(700.f, -180.f, 0.f), FVector(7200.f, -180.f, 0.f), FVector(7200.f, 180.f, 0.f), FVector(700.f, 180.f, 0.f) }, 0.f, 4);

	// where he starts, a checkpoint halfway, the way out
	Mark(TEXT("PlayerStart"), FVector(-400.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f));
	if (Segment)
	{
		Segment->AddCheckpoint(FBox(FVector(3500.f, -Edge, -100.f), FVector(3700.f, Edge, 500.f)), FTransform(FRotator::ZeroRotator, FVector(3600.f, -200.f, 108.f)));
		Segment->AddExit(FBox(FVector(7750.f, -Road * 0.5f, -100.f), FVector(8500.f, Road * 0.5f, 500.f)), TEXT("end"));
		Segment->RequestGrid(FBox2D(FVector2D(-950.f, -3800.f), FVector2D(8500.f, 2500.f)), 50.f, 0.f);
	}
	MarkLookAt(TEXT("Establish"), FVector(-250.f, -350.f, 260.f), FVector(1800.f, 300.f, 300.f), 70.f);
}
