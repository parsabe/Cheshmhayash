// Cheshm Hayash - the nights of Mehr (2.4, 2.5) and the last night (2.6).

#include "World/CHLocations.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/CHPlayerController.h"
#include "Game/CHFilmable.h"
#include "Game/CHGuard.h"
#include "Game/CHInteractable.h"
#include "Game/CHSegment.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "Story/CHStoryDirector.h"
#include "UI/CHStyle.h"
#include "World/CHFire.h"
#include "World/CHNpc.h"

namespace
{
	const float Road = 800.f;
	const float Walk = 450.f;
	const float Edge = Road * 0.5f + Walk;

	const TCHAR* ChantTexts[] = {
		TEXT("زن، زندگی، آزادی"),
		TEXT("مرد، میهن، آبادی"),
		TEXT("نترسید، نترسید، ما همه با هم هستیم"),
		TEXT("آزادی، آزادی، آزادی"),
		TEXT("مرگ بر دیکتاتور"),
		TEXT("زن، زندگی، آزادی"),
	};
}

// ---------------------------------------------------------------------------------------------
// 2.4 - the nights of Mehr
// ---------------------------------------------------------------------------------------------

void ACHLoc_Streets::BuildNights()
{
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	FRandomStream Rng(2401);

	NightEnv(-1.3f, 0.04f);
	BuildAlborz(80.f, FLinearColor(0.010f, 0.012f, 0.020f), FLinearColor(0.05f, 0.055f, 0.07f));
	Ground(-3000.f, -5000.f, 10000.f, 10000.f);

	// street A along +X, the crossing, street B north
	StreetX(-600.f, 5150.f, 0.f, Road, Walk, 31);
	BoxAt(5150.f, -Edge, -20.f, 1700.f, 2.f * Edge, 20.f, MAsphalt);
	StreetY(Edge, 7400.f, 6000.f, Road, Walk, 32);
	StreetY(-2600.f, -Edge, 6000.f, Road, Walk, 33);

	auto Block = [this](float X0, float Y0, float X1, float Y1, int32 Floors, uint8 Facades, uint8 Shops, int32 Seed)
	{
		FCHBuildingStyle Style;
		Style.Floors = Floors;
		Style.Facades = Facades;
		Style.Shops = Shops;
		Style.Seed = Seed;
		Building(X0, Y0, X1 - X0, Y1 - Y0, Style);
	};

	// south of A
	Block(-600.f, -Edge - 1500.f, 900.f, -Edge, 5, 8, 8, 1);
	Block(900.f, -Edge - 1500.f, 2300.f, -Edge, 4, 8, 8, 2);
	Block(2300.f, -Edge - 1500.f, 3700.f, -Edge, 6, 8, 8, 3);
	Block(3700.f, -Edge - 1500.f, 5150.f, -Edge, 5, 8 | 2, 8, 4);
	// north of A
	Block(-600.f, Edge, 1000.f, Edge + 1500.f, 5, 4, 4, 5);
	Block(1000.f, Edge, 2400.f, Edge + 1500.f, 4, 4, 4, 6);
	Block(2400.f, Edge, 3800.f, Edge + 1500.f, 6, 4, 4, 7);
	Block(3800.f, Edge, 5150.f, Edge + 1500.f, 5, 4 | 2, 4 | 2, 8);
	// west of B, north part (the wall block shows B a blank side)
	Block(3650.f, Edge + 1500.f, 5150.f, 3800.f, 5, 2, 2, 9);
	Block(3650.f, 3800.f, 5150.f, 5400.f, 6, 0, 0, 10);
	Block(3650.f, 5400.f, 5150.f, 6150.f, 4, 2 | 8, 2, 11);
	Block(3650.f, 6650.f, 5150.f, 7400.f, 5, 2 | 4, 2, 12);
	// the backs of the blocks
	Block(-600.f, Edge + 1500.f, 3650.f, 7400.f, 5, 0, 0, 13);
	BoxAt(3550.f, 6150.f, -20.f, 100.f, 500.f, 600.f, MFacades[2]);
	// east of B
	const float EastX = 6000.f + Edge;
	Block(EastX, -2600.f, EastX + 1500.f, -Edge, 5, 1, 1, 14);
	Block(EastX, -Edge, EastX + 1500.f, 1200.f, 4, 1, 1, 15);
	Block(EastX, 1200.f, EastX + 1500.f, 3000.f, 6, 1, 1, 16);
	Block(EastX, 3000.f, EastX + 1500.f, 4800.f, 5, 1, 1, 17);
	Block(EastX, 4800.f, EastX + 1500.f, 6400.f, 4, 1, 1, 18);
	Block(EastX, 6400.f, EastX + 1500.f, 7400.f, 5, 1, 1, 19);
	// closing the ends
	Block(5150.f, -4100.f, 6850.f, -2600.f, 6, 8, 8, 20);
	Block(5150.f, 7400.f, 6850.f, 8900.f, 6, 4, 4, 21);
	Block(-2100.f, -Edge - 1500.f, -600.f, Edge + 1500.f, 6, 2, 0, 22);

	// lamps lit along the streets
	int32 LampIndex = 0;
	for (float X = 200.f; X < 5000.f; X += 1300.f, ++LampIndex)
	{
		const bool bNorth = (LampIndex % 2) == 0;
		Lamp(FVector(X, bNorth ? Edge - 70.f : -Edge + 70.f, 14.f), bNorth ? -90.f : 90.f, true, false);
	}
	LampIndex = 0;
	for (float Y = 1300.f; Y < 7200.f; Y += 1300.f, ++LampIndex)
	{
		const bool bWest = (LampIndex % 2) == 0;
		Lamp(FVector(bWest ? 6000.f - Edge + 70.f : 6000.f + Edge - 70.f, Y, 14.f), bWest ? 0.f : 180.f, true, false);
	}
	for (float X = 700.f; X < 5000.f; X += 1900.f)
	{
		Wires(FVector(X, -Edge, 640.f), FVector(X + 300.f, Edge, 660.f), 150.f);
	}
	for (float Y = 1600.f; Y < 7000.f; Y += 1900.f)
	{
		Wires(FVector(6000.f - Edge, Y, 650.f), FVector(6000.f + Edge, Y + 250.f, 640.f), 150.f);
	}

	// parked cars, a burnt one
	const FLinearColor CarColors[] = { FLinearColor(0.5f, 0.5f, 0.48f), FLinearColor(0.08f, 0.08f, 0.09f), FLinearColor(0.25f, 0.03f, 0.02f), FLinearColor(0.3f, 0.3f, 0.32f) };
	for (float X = 500.f; X < 4900.f; X += Rng.FRandRange(700.f, 1300.f))
	{
		if (FMath::Abs(X - 4200.f) < 500.f)
		{
			continue;
		}
		Car(FVector(X, Road * 0.5f - 110.f, 0.f), (Rng.FRand() < 0.5f) ? 0.f : 180.f, CarColors[Rng.RandRange(0, 3)]);
	}
	for (float Y = 1500.f; Y < 6800.f; Y += Rng.FRandRange(900.f, 1500.f))
	{
		Car(FVector(6000.f + Road * 0.5f - 110.f, Y, 0.f), 90.f, CarColors[Rng.RandRange(0, 3)]);
	}
	Car(FVector(3150.f, 120.f, 0.f), 28.f, FLinearColor::Black, true);

	// bins to hide behind
	Dumpster(FVector(1900.f, -Edge + 160.f, 14.f), 0.f);
	Dumpster(FVector(3500.f, Edge - 160.f, 14.f), 180.f);
	Dumpster(FVector(6000.f - Edge + 160.f, 4300.f, 14.f), 270.f);
	Dumpster(FVector(3950.f, 6560.f, 0.f), 180.f);

	// fires for light
	Fire(FVector(5900.f, -480.f, 0.f), 1.6f, true, 7);
	Fire(FVector(6250.f, 5300.f, 0.f), 1.1f, false, 8);

	// ---- a headscarf burning on a fire, a young woman beside it
	Fire(FVector(1300.f, -80.f, 0.f), 1.0f, true, 9);
	Box(FVector(1300.f, -80.f, 70.f), FVector(70.f, 50.f, 3.f), Lit(FLinearColor(0.35f, 0.05f, 0.04f), 1.f, 0.f), false, FRotator(8.f, 20.f, -6.f));
	if (ACHNpc* Woman = SpawnNpc(TEXT("scarf_woman"), TEXT("student_f"), FVector(1310.f, -300.f, 0.f), 95.f, 77))
	{
		Woman->FaceLocation(FVector(1300.f, -80.f, 0.f), true);
	}
	SpawnFilmable(TEXT("scarf"), TEXT("روسری در آتش"), TEXT("A headscarf in the fire"), FVector(1300.f, -80.f, 140.f), 3.f);

	// ---- a wounded stranger against a shutter
	if (ACHNpc* Wounded = SpawnNpc(TEXT("wounded"), TEXT("man"), FVector(2750.f, -Edge + 45.f, 14.f), 90.f, 7))
	{
		Wounded->SetSink(62.f);
		Wounded->SetShaking(true);
	}
	SpawnInteractable(TEXT("wounded"), TEXT("یک نفر زخمی است"), TEXT("Someone is hurt"), FVector(2750.f, -Edge + 140.f, 70.f), 220.f);
	PointLight(FVector(2900.f, -Edge + 160.f, 260.f), FLinearColor(1.f, 0.7f, 0.42f), 9.f, 520.f, true, 0.5f);
	MarkLookAt(TEXT("WoundedCam"), FVector(2580.f, -Edge + 330.f, 150.f), FVector(2750.f, -Edge + 40.f, 70.f), 50.f);

	// ---- students hiding behind a car, two of them coming
	Car(FVector(4200.f, -250.f, 0.f), 0.f, FLinearColor(0.45f, 0.45f, 0.43f));
	for (int32 Index = 0; Index < 3; ++Index)
	{
		if (ACHNpc* Student = SpawnNpc(FString::Printf(TEXT("student%d"), Index), Index == 1 ? TEXT("student_m") : TEXT("student_f"), FVector(4080.f + Index * 110.f, -395.f, 0.f), 90.f, 30 + Index))
		{
			Student->SetSink(48.f);
			Students.Add(Student);
		}
	}
	SpawnInteractable(TEXT("students"), TEXT("بچه‌ها پشت ماشین قایم شده‌اند"), TEXT("Students hiding behind a car"), FVector(4200.f, -460.f, 60.f), 260.f);
	MarkLookAt(TEXT("StudentsCam"), FVector(3900.f, -560.f, 120.f), FVector(4900.f, 300.f, 140.f), 60.f);
	StudentGuards.Add(SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(5000.f, 620.f, 14.f), FVector(3700.f, 620.f, 14.f) }, 180.f, 41));
	StudentGuards.Add(SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(5050.f, 450.f, 0.f), FVector(3750.f, 450.f, 0.f) }, 180.f, 42));

	// ---- an old man raises his shutter for the students
	{
		const float ShopX = 6000.f - Edge;
		ShopDoor = FVector(ShopX - 20.f, 3000.f, 0.f);
		ShopCurb = FVector(6000.f - Road * 0.5f - 60.f, 3150.f, 0.f);
		Box(FVector(ShopX + 2.f, 3000.f, 130.f), FVector(4.f, 280.f, 260.f), MShopLit, false);
		PointLight(FVector(ShopX + 150.f, 3000.f, 220.f), FLinearColor(1.f, 0.72f, 0.42f), 14.f, 650.f, false, 0.6f);
		ShutterPivot = Pivot(FVector(ShopX + 6.f, 3000.f, 0.f));
		BoxOn(ShutterPivot, FVector(0.f, 0.f, 150.f), FVector(8.f, 290.f, 300.f), MShutter, true);
		for (float Z = 14.f; Z < 300.f; Z += 16.f)
		{
			BoxOn(ShutterPivot, FVector(4.5f, 0.f, Z), FVector(2.f, 290.f, 3.f), MRust, false);
		}
		if (ACHNpc* OldMan = SpawnNpc(TEXT("oldman"), TEXT("man"), FVector(ShopX + 70.f, 3200.f, 14.f), 0.f, 21))
		{
			OldMan->SetActorScale3D(FVector(0.95f));
		}
		if (Segment)
		{
			Segment->AddLightPool(FVector(ShopX + 200.f, 3000.f, 0.f), 300.f);
		}
		SpawnFilmable(TEXT("shutter"), TEXT("پیرمرد کرکره را برای دانشجوها بالا داد"), TEXT("An old man lifts his shutter for the students"), FVector(ShopX + 120.f, 3000.f, 170.f), 3.5f);
	}

	// ---- the wall: Woman, Life, Freedom over the old slogans
	{
		const FVector WallAt(6000.f - Edge + 1.f, 4600.f, 260.f);
		PaintedOver(WallAt, 0.f, FVector2D(820.f, 320.f));
		Graffiti(TEXT("زن، زندگی، آزادی"), WallAt + FVector(5.f, 0.f, 10.f), 0.f, 76.f, FLinearColor(0.55f, 0.02f, 0.015f));
		SpawnFilmable(TEXT("wall"), TEXT("زن، زندگی، آزادی — روی شعارهای قدیمی"), TEXT("Woman, Life, Freedom - over the old slogans"), WallAt + FVector(40.f, 0.f, 0.f), 3.f);
	}

	// ---- the people who hunt him
	SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(1700.f, Edge - 230.f, 14.f), FVector(3300.f, Edge - 230.f, 14.f) }, 0.f, 51);
	SpawnGuard(ECHGuardKind::Plainclothes, TEXT("man"), { FVector(3300.f, -Edge + 230.f, 14.f), FVector(3700.f, -Edge + 260.f, 14.f) }, 0.f, 52);
	SpawnGuard(ECHGuardKind::Motorcycle, TEXT("basij"), { FVector(300.f, -180.f, 0.f), FVector(5300.f, -180.f, 0.f), FVector(5300.f, 180.f, 0.f), FVector(300.f, 180.f, 0.f) }, 0.f, 53);
	SpawnGuard(ECHGuardKind::Motorcycle, TEXT("basij"), { FVector(6180.f, 1200.f, 0.f), FVector(6180.f, 7000.f, 0.f), FVector(5820.f, 7000.f, 0.f), FVector(5820.f, 1200.f, 0.f) }, 90.f, 54);
	SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { FVector(6000.f + Edge - 230.f, 4300.f, 14.f), FVector(6000.f + Edge - 230.f, 5900.f, 14.f) }, 90.f, 55);
	Van(FVector(6260.f, 2000.f, 0.f), 90.f);
	if (ACHGuard* Watcher = SpawnGuard(ECHGuardKind::Watcher, TEXT("morality_f"), { FVector(6000.f + Edge - 150.f, 2250.f, 14.f) }, 180.f, 56))
	{
		Watcher->SetScan(140.f, 230.f, 8.f);
	}
	if (ACHGuard* Watcher = SpawnGuard(ECHGuardKind::Watcher, TEXT("morality_m"), { FVector(6420.f, 1650.f, 0.f) }, 270.f, 57))
	{
		Watcher->SetScan(200.f, 290.f, 9.f);
	}

	// ---- the alley where he stops, and Mom's message
	Lamp(FVector(4200.f, 6600.f, 0.f), -90.f, true, true);
	MarkLookAt(TEXT("AlleyCam"), FVector(4720.f, 6560.f, 160.f), FVector(4250.f, 6230.f, 70.f), 55.f);
	Mark(TEXT("AlleyHe"), FVector(4250.f, 6205.f, 0.f), FRotator(0.f, 90.f, 0.f));

	// start, checkpoints, the way out
	Mark(TEXT("PlayerStart"), FVector(-300.f, 0.f, 0.f), FRotator::ZeroRotator);
	MarkLookAt(TEXT("Establish"), FVector(-200.f, -500.f, 300.f), FVector(1800.f, 200.f, 250.f), 70.f);
	if (Segment)
	{
		Segment->AddCheckpoint(FBox(FVector(5150.f, -Edge, -100.f), FVector(6850.f, Edge, 500.f)), FTransform(FRotator(0.f, 90.f, 0.f), FVector(5700.f, 0.f, 100.f)));
		Segment->AddCheckpoint(FBox(FVector(5150.f, 3600.f, -100.f), FVector(6850.f, 3900.f, 500.f)), FTransform(FRotator(0.f, 90.f, 0.f), FVector(5650.f, 3750.f, 110.f)));
		Segment->AddExit(FBox(FVector(3700.f, 6150.f, -100.f), FVector(4650.f, 6650.f, 500.f)), TEXT("alley"));
		Segment->RequestGrid(FBox2D(FVector2D(-700.f, -2700.f), FVector2D(7000.f, 7500.f)), 50.f, 0.f);
	}
}

// ---------------------------------------------------------------------------------------------
// 2.6 - the last night
// ---------------------------------------------------------------------------------------------

void ACHLoc_Streets::BuildLastNight()
{
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
	FRandomStream Rng(2406);

	NightEnv(-1.6f, 0.045f);
	BuildAlborz(15.f, FLinearColor(0.010f, 0.012f, 0.020f), FLinearColor(0.05f, 0.055f, 0.07f));
	Ground(-3000.f, -5000.f, 10000.f, 8000.f);

	// a wide avenue with a median
	const float AvRoad = 1400.f;
	const float AvWalk = 500.f;
	const float AvEdge = AvRoad * 0.5f + AvWalk;
	StreetX(-900.f, 7200.f, 0.f, AvRoad, AvWalk, 61);
	BoxAt(-900.f, -40.f, -20.f, 8100.f, 80.f, 40.f, MCurb);

	auto Block = [this](float X0, float Y0, float X1, float Y1, int32 Floors, uint8 Facades, uint8 Shops, int32 Seed)
	{
		FCHBuildingStyle Style;
		Style.Floors = Floors;
		Style.Facades = Facades;
		Style.Shops = Shops;
		Style.Seed = Seed;
		Building(X0, Y0, X1 - X0, Y1 - Y0, Style);
	};

	// north of the avenue, with the escape alley at X 2600..3000
	Block(-900.f, AvEdge, 1000.f, AvEdge + 1500.f, 6, 4, 4, 1);
	Block(1000.f, AvEdge, 2600.f, AvEdge + 1500.f, 5, 4 | 2, 4, 2);
	Block(3000.f, AvEdge, 4800.f, AvEdge + 1500.f, 6, 4 | 1, 4, 3);
	Block(4800.f, AvEdge, 7200.f, AvEdge + 1500.f, 5, 4, 4, 4);
	// south of the avenue
	Block(-900.f, -AvEdge - 1500.f, 1500.f, -AvEdge, 5, 8, 8, 5);
	Block(1500.f, -AvEdge - 1500.f, 3600.f, -AvEdge, 6, 8, 8, 6);
	Block(3600.f, -AvEdge - 1500.f, 5400.f, -AvEdge, 5, 8, 8, 7);
	Block(5400.f, -AvEdge - 1500.f, 7200.f, -AvEdge, 6, 8, 8, 8);
	// the ends
	Block(-2400.f, -AvEdge - 1500.f, -900.f, AvEdge + 1500.f, 6, 2, 0, 9);
	Block(7200.f, -AvEdge - 1500.f, 8700.f, AvEdge + 1500.f, 6, 1, 0, 10);

	// the alley north: walls, a wall to climb, a yard, the stairwell door
	const float AlleyX0 = 2600.f;
	const float AlleyX1 = 3000.f;
	const float AlleyY0 = AvEdge + 1500.f;
	Block(1000.f, AlleyY0, AlleyX0, 5400.f, 5, 2, 0, 11);
	Block(AlleyX1, AlleyY0, 4800.f, 5400.f, 5, 1, 0, 12);
	BoxAt(AlleyX0, 5400.f, -20.f, AlleyX1 - AlleyX0, 300.f, 900.f, MFacades[1]);
	// crates along the side of the alley (open passage, no blocking wall)
	UMaterialInterface* Crate = Lit(FLinearColor(0.16f, 0.11f, 0.06f), 0.9f, 0.6f);
	Box(FVector(2660.f, 3530.f, 45.f), FVector(90.f, 90.f, 90.f), Crate, false);
	Box(FVector(2940.f, 3480.f, 40.f), FVector(90.f, 90.f, 80.f), Crate, false, FRotator(0.f, 12.f, 0.f));
	// the stairwell door at the end
	Box(FVector(2800.f, 5398.f, 115.f), FVector(150.f, 6.f, 230.f), Emissive(FLinearColor(1.f, 0.6f, 0.3f), CHLight::Emissive(0.8f), FLinearColor(0.02f, 0.015f, 0.01f)), false);
	PointLight(FVector(2800.f, 5300.f, 250.f), FLinearColor(1.f, 0.65f, 0.35f), 14.f, 900.f, false, 0.8f);
	Lamp(FVector(2620.f, 3200.f, 0.f), 0.f, true, false);
	Lamp(FVector(2620.f, 4300.f, 0.f), 0.f, true, false);
	Dumpster(FVector(2900.f, 2900.f, 0.f), 90.f);

	// lamps lit along the avenue
	int32 LampIndex = 0;
	for (float X = 0.f; X < 7000.f; X += 1400.f, ++LampIndex)
	{
		Lamp(FVector(X, AvEdge - 80.f, 14.f), -90.f, true, false);
		Lamp(FVector(X + 700.f, -AvEdge + 80.f, 14.f), 90.f, true, false);
	}
	for (float X = 500.f; X < 7000.f; X += 2000.f)
	{
		Wires(FVector(X, -AvEdge, 700.f), FVector(X + 300.f, AvEdge, 720.f), 180.f);
	}

	// fires among the people, phones held up
	Fire(FVector(1100.f, 450.f, 0.f), 1.2f, true, 1);
	Fire(FVector(2500.f, -420.f, 0.f), 1.4f, true, 2);
	Fire(FVector(3900.f, 250.f, 0.f), 2.2f, true, 3);

	// the crowd, and the words rising from it
	SpawnCrowd(FBox2D(FVector2D(300.f, -640.f), FVector2D(3200.f, 640.f)), 46, 7);
	if (Segment)
	{
		Segment->AddCrowdZone(FVector(1750.f, 0.f, 0.f), 1600.f);
	}
	for (int32 Index = 0; Index < 7; ++Index)
	{
		const FVector At(Rng.FRandRange(400.f, 3100.f), Rng.FRandRange(-500.f, 500.f), Rng.FRandRange(380.f, 650.f));
		UWidgetComponent* Chant = WorldText(ChantTexts[Index % UE_ARRAY_COUNT(ChantTexts)], true, 46.f, FLinearColor(1.f, 0.93f, 0.82f), At, FRotator(0.f, 180.f, 0.f), 1.1f, true, true);
		Chants.Add(Chant);
		ChantAge.Add(Rng.FRandRange(0.f, 6.f));
		ChantBase.Add(At);
	}

	// they wait at the far end, in the dark
	const FVector ChaserSpots[] = { FVector(4700.f, -500.f, 0.f), FVector(4800.f, 0.f, 0.f), FVector(4700.f, 500.f, 0.f), FVector(5100.f, -250.f, 0.f), FVector(5100.f, 300.f, 0.f) };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ChaserSpots); ++Index)
	{
		if (ACHGuard* Guard = SpawnGuard(ECHGuardKind::Foot, TEXT("guard"), { ChaserSpots[Index] }, 180.f, 70 + Index))
		{
			Guard->ChaseSpeed = 470.f;
			Guard->SetDormant(true);
			Chasers.Add(Guard);
		}
	}
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const float Y = Index == 0 ? -300.f : 300.f;
		if (ACHGuard* Bike = SpawnGuard(ECHGuardKind::Motorcycle, TEXT("basij"), { FVector(6200.f, Y, 0.f), FVector(-400.f, Y, 0.f) }, 180.f, 80 + Index))
		{
			Bike->SetDormant(true);
			Chasers.Add(Bike);
		}
	}
	if (ACHGuard* Plain = SpawnGuard(ECHGuardKind::Plainclothes, TEXT("man"), { FVector(2800.f, 4500.f, 0.f), FVector(2750.f, 5100.f, 0.f) }, 90.f, 90))
	{
		Plain->SetDormant(true);
		Chasers.Add(Plain);
	}

	Mark(TEXT("PlayerStart"), FVector(450.f, -150.f, 0.f), FRotator::ZeroRotator);
	MarkLookAt(TEXT("Establish"), FVector(200.f, -900.f, 520.f), FVector(2600.f, 200.f, 200.f), 68.f);
	if (Segment)
	{
		Segment->AddExit(FBox(FVector(3300.f, -AvEdge, -100.f), FVector(3500.f, AvEdge, 600.f)), TEXT("front"));
		Segment->AddExit(FBox(FVector(AlleyX0, 5050.f, -100.f), FVector(AlleyX1, 5400.f, 600.f)), TEXT("door"));
		Segment->RequestGrid(FBox2D(FVector2D(-1000.f, -AvEdge - 100.f), FVector2D(7300.f, 5500.f)), 50.f, 0.f);
	}
}

void ACHLoc_Streets::SpawnCrowd(const FBox2D& Area, int32 Count, int32 Seed)
{
	FRandomStream Rng(Seed);
	const TCHAR* Looks[] = { TEXT("student_f"), TEXT("woman"), TEXT("man"), TEXT("student_f"), TEXT("man"), TEXT("student_f") };
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FVector At(Rng.FRandRange(Area.Min.X, Area.Max.X), Rng.FRandRange(Area.Min.Y, Area.Max.Y), 0.f);
		if (ACHNpc* Person = SpawnNpc(FString::Printf(TEXT("crowd%d"), Index), Looks[Index % UE_ARRAY_COUNT(Looks)], At, Rng.FRandRange(-25.f, 25.f), 400 + Index))
		{
			Crowd.Add(Person);
			// some march slowly towards the front, the rest stand and sing
			const FVector Goal = Rng.FRand() < 0.55f ? At + FVector(Rng.FRandRange(600.f, 1400.f), Rng.FRandRange(-120.f, 120.f), 0.f) : At;
			CrowdGoals.Add(Goal);
			if (!Goal.Equals(At))
			{
				Person->WalkTo(Goal, Rng.FRandRange(55.f, 95.f));
			}
		}
	}
}

void ACHLoc_Streets::StartShots()
{
	if (bShotsFired)
	{
		return;
	}
	bShotsFired = true;
	if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
	{
		Director->PlaySfx(TEXT("sfx_shots"), 1.f);
	}
	FRandomStream Rng(99);

	UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector Where = Pawn ? Pawn->GetActorLocation() : FVector(3300.f, 0.f, 0.f);

	if (Director)
	{
		Director->Flash(FLinearColor(1.f, 0.95f, 0.85f), 0.35f);
	}
	if (ACHPlayerController* Controller = Cast<ACHPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		Controller->Shake(1.6f, 3.f);
	}

	// the people run
	for (ACHNpc* Person : Crowd)
	{
		if (Person)
		{
			Person->WalkTo(FVector(Rng.FRandRange(-800.f, -300.f), Person->GetActorLocation().Y + Rng.FRandRange(-200.f, 200.f), 0.f), Rng.FRandRange(360.f, 480.f));
		}
	}
	// gas and smoke between him and them
	for (int32 Index = 0; Index < 4; ++Index)
	{
		if (ACHFire* Smoke = Fire(FVector(3700.f + Index * 220.f, -500.f + Index * 330.f, 0.f), 2.3f, false, 30 + Index))
		{
			Smoke->SetBurning(false);
		}
	}
	// they come
	for (ACHGuard* Guard : Chasers)
	{
		if (Guard)
		{
			Guard->Alert(Where);
		}
	}
	// the chants break off
	for (int32 Index = 0; Index < ChantAge.Num(); ++Index)
	{
		ChantAge[Index] = FMath::Max(ChantAge[Index], 5.2f);
	}

	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
	{
		Segment->SetHint(TEXT("فرار کن! از کوچه‌ی سمت چپ."), TEXT("Run. The alley on your left."));
		Segment->SetRespawn(FTransform(FRotator(0.f, 90.f, 0.f), FVector(2800.f, 1000.f, 110.f)));
	}
}

// ---------------------------------------------------------------------------------------------
// Shared
// ---------------------------------------------------------------------------------------------

void ACHLoc_Streets::StartSegment(const FString& Segment, const TSharedPtr<FJsonObject>& Beat)
{
	Super::StartSegment(Segment, Beat);
}

void ACHLoc_Streets::OnInteract(const FString& Id, ACHInteractable* What)
{
	UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	if (!Director || !What)
	{
		return;
	}
	if (Id.StartsWith(TEXT("talk.")))
	{
		Super::OnInteract(Id, What);
	}
	else if (Id.StartsWith(TEXT("trapped")))
	{
		SendStudentOut(Id, What);
	}
	else if (Id == TEXT("wounded"))
	{
		What->SetEnabled(false);
		Director->PlayInterlude(TEXT("2.4.wounded"));
	}
	else if (Id == TEXT("students"))
	{
		What->SetEnabled(false);
		Director->PlayInterlude(TEXT("2.4.students"));
	}
}

void ACHLoc_Streets::OnInterludeEnded()
{
	Super::OnInterludeEnded();
	UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	if (!Director)
	{
		return;
	}
	// he draws them away: the students run, the two come after him
	if (Director->GetFlag(TEXT("drew")) == TEXT("yes") && Students.Num() > 0)
	{
		for (ACHNpc* Student : Students)
		{
			if (Student)
			{
				Student->SetSink(0.f);
				Student->WalkTo(Student->GetActorLocation() + FVector(-2500.f, -60.f, 0.f), 420.f);
			}
		}
		Students.Reset();
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			for (ACHGuard* Guard : StudentGuards)
			{
				if (Guard)
				{
					Guard->Alert(Pawn->GetActorLocation());
				}
			}
		}
	}
}

void ACHLoc_Streets::OnSegmentExit(const FString& Id)
{
	if (Id == TEXT("front"))
	{
		StartShots();
		return;
	}
	FinishSegment();
}

void ACHLoc_Streets::OnSegmentReset()
{
	if (!bShotsFired)
	{
		return;
	}
	// back at the mouth of the alley; they come again from where they stood
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	for (ACHGuard* Guard : Chasers)
	{
		if (Guard && Pawn)
		{
			Guard->Alert(Pawn->GetActorLocation());
		}
	}
}

void ACHLoc_Streets::OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat)
{
	OnMassacreEvent(Event);
	if (Event == TEXT("wounded_helped"))
	{
		RemoveNpc(TEXT("wounded"));
	}
	else if (Event == TEXT("shots"))
	{
		StartShots();
	}
	else if (Event == TEXT("taxi"))
	{
		bTaxi = true;
	}
}

void ACHLoc_Streets::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StreetsTime += DeltaSeconds;
	TickDay(DeltaSeconds);
	TickMassacre(DeltaSeconds);
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);

	// the old man's shutter goes up as he comes near; students slip inside
	if (ShutterPivot)
	{
		const bool bNear = Pawn && FVector::Dist2D(Pawn->GetActorLocation(), ShopDoor) < 2600.f;
		if (bNear && ShutterOpen < 1.f)
		{
			ShutterOpen = FMath::Min(1.f, ShutterOpen + DeltaSeconds / 3.5f);
			ShutterPivot->SetRelativeLocation(FVector(6000.f - Edge + 6.f, 3000.f, 255.f * FMath::SmoothStep(0.f, 1.f, ShutterOpen)));
		}
		if (ShutterOpen > 0.6f)
		{
			ShopLoop -= DeltaSeconds;
			if (ShopLoop <= 0.f && ShopStudents.Num() < 6)
			{
				ShopLoop = 2.6f;
				const int32 Index = ShopStudents.Num();
				if (ACHNpc* Student = SpawnNpc(FString::Printf(TEXT("shop%d"), Index), Index % 2 ? TEXT("student_m") : TEXT("student_f"), ShopCurb + FVector(0.f, Index % 2 ? 120.f : -60.f, 0.f), 180.f, 60 + Index))
				{
					Student->WalkTo(ShopDoor + FVector(0.f, (Index % 3 - 1) * 60.f, 0.f), 330.f);
					ShopStudents.Add(Student);
				}
			}
			for (int32 Index = ShopStudents.Num() - 1; Index >= 0; --Index)
			{
				if (ShopStudents[Index] && !ShopStudents[Index]->IsMoving())
				{
					ShopStudents[Index]->SetActorHiddenInGame(true);
				}
			}
		}
	}

	// the words rising from the crowd
	for (int32 Index = 0; Index < Chants.Num(); ++Index)
	{
		UWidgetComponent* Chant = Chants[Index];
		if (!Chant)
		{
			continue;
		}
		float& Age = ChantAge[Index];
		Age += DeltaSeconds;
		const float Life = 6.f;
		if (Age > Life)
		{
			if (bShotsFired)
			{
				Chant->SetVisibility(false);
				continue;
			}
			Age = 0.f;
			FRandomStream Rng(Index * 131 + FMath::FloorToInt32(StreetsTime * 10.f));
			ChantBase[Index] = FVector(Rng.FRandRange(400.f, 3100.f), Rng.FRandRange(-500.f, 500.f), Rng.FRandRange(380.f, 600.f));
		}
		const float T = Age / Life;
		Chant->SetWorldLocation(ChantBase[Index] + FVector(0.f, 0.f, 60.f * Age));
		Chant->SetTintColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FMath::Sin(T * UE_PI) * 0.9f));
		// face the camera, roughly
		if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			FVector ToCamera = Camera->GetCameraLocation() - Chant->GetComponentLocation();
			ToCamera.Z = 0.f;
			if (!ToCamera.IsNearlyZero())
			{
				Chant->SetWorldRotation(FRotator(0.f, ToCamera.Rotation().Yaw, 0.f));
			}
		}
	}

	// people who ran far enough are gone
	if (bShotsFired)
	{
		for (int32 Index = Crowd.Num() - 1; Index >= 0; --Index)
		{
			if (Crowd[Index] && !Crowd[Index]->IsMoving())
			{
				Crowd[Index]->SetActorHiddenInGame(true);
			}
		}
	}
}
