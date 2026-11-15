// Cheshm Hayash - the next protest, the night after the rooftop (2.7). The whole street is out,
// he with them. A truck comes with a heavy machine gun (a DShK) on its bed and they fire into the
// people. No blood is ever shown: people fall, people run. A bullet takes his left eye; he falls.
// Then, far from the street, only his family, in a warm light: the last thing he sees.

#include "World/CHLocations.h"
#include "Camera/CameraActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/CHPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"
#include "Story/CHJson.h"
#include "Story/CHStoryDirector.h"
#include "World/CHFire.h"
#include "World/CHNpc.h"

namespace
{
	const float MsRoad = 1400.f;
	const float MsWalk = 500.f;
	const float MsEdge = MsRoad * 0.5f + MsWalk;
	const FVector HeSpot(2300.f, -150.f, 0.f);
	const float TruckStartX = 8200.f;
	const float TruckStopX = 4700.f;
	const FVector VisionAt(-30000.f, 0.f, 0.f);
	const TCHAR* MorningChants[] = {
		TEXT("زن، زندگی، آزادی"),
		TEXT("نمی‌ترسیم، نمی‌ترسیم"),
		TEXT("مرد، میهن، آبادی"),
		TEXT("این آخرین پیامه"),
	};
}

void ACHLoc_Streets::BuildMassacre()
{
	FRandomStream Rng(2507);

	// the next night: the whole avenue out again in the dark, lit by lamps and fires
	NightEnv(-1.3f, 0.04f);
	BuildAlborz(15.f, FLinearColor(0.010f, 0.012f, 0.020f), FLinearColor(0.05f, 0.055f, 0.07f));
	Ground(-3000.f, -5000.f, 11000.f, 5000.f);

	StreetX(-900.f, 9000.f, 0.f, MsRoad, MsWalk, 71);
	BoxAt(-900.f, -40.f, -20.f, 9900.f, 80.f, 40.f, MCurb);
	auto Block = [this](float X0, float Y0, float X1, float Y1, int32 Floors, uint8 Facades, uint8 Shops, int32 Seed)
	{
		FCHBuildingStyle Style;
		Style.Floors = Floors;
		Style.Facades = Facades;
		Style.Shops = Shops;
		Style.LitWindows = 0.04f;
		Style.Seed = Seed;
		Building(X0, Y0, X1 - X0, Y1 - Y0, Style);
	};
	float X = -900.f;
	for (int32 Index = 0; X < 9000.f; ++Index)
	{
		const float W = Rng.FRandRange(1300.f, 2000.f);
		Block(X, MsEdge, X + W - 20.f, MsEdge + 1500.f, Rng.RandRange(4, 6), 4, 4, 300 + Index);
		Block(X, -MsEdge - 1500.f, X + W - 20.f, -MsEdge, Rng.RandRange(4, 6), 8, 8, 320 + Index);
		X += W;
	}
	Block(-2400.f, -MsEdge - 1500.f, -900.f, MsEdge + 1500.f, 6, 2, 0, 340);
	for (float LampX = 0.f; LampX < 8800.f; LampX += 1400.f)
	{
		Lamp(FVector(LampX, MsEdge - 80.f, 14.f), -90.f, true, false);
		Lamp(FVector(LampX + 700.f, -MsEdge + 80.f, 14.f), 90.f, true, false);
	}
	for (float WireX = 500.f; WireX < 8800.f; WireX += 2000.f)
	{
		Wires(FVector(WireX, -MsEdge, 700.f), FVector(WireX + 300.f, MsEdge, 720.f), 180.f);
	}
	// the night's barricades, still smouldering
	Fire(FVector(3300.f, 520.f, 0.f), 1.3f, true, 11);
	Fire(FVector(3150.f, -560.f, 0.f), 1.1f, true, 12);
	Fire(FVector(1200.f, 560.f, 0.f), 1.0f, true, 13);
	Fire(FVector(600.f, -480.f, 0.f), 0.9f, false, 14);
	Car(FVector(3400.f, -250.f, 0.f), 70.f, FLinearColor::Black, true);

	// ---- the people, and him among them
	SpawnCrowd(FBox2D(FVector2D(200.f, -650.f), FVector2D(2900.f, 650.f)), 54, 25);
	for (int32 Index = 0; Index < Crowd.Num(); ++Index)
	{
		if (Crowd[Index] && Rng.FRand() < 0.5f)
		{
			Crowd[Index]->SetPose(TEXT("fist"));
		}
	}
	for (int32 Index = 0; Index < 6; ++Index)
	{
		const FVector At(Rng.FRandRange(500.f, 2800.f), Rng.FRandRange(-500.f, 500.f), Rng.FRandRange(380.f, 620.f));
		UWidgetComponent* Chant = WorldText(MorningChants[Index % UE_ARRAY_COUNT(MorningChants)], true, 46.f, FLinearColor(1.f, 0.93f, 0.82f), At, FRotator(0.f, 180.f, 0.f), 1.1f, true, true);
		Chants.Add(Chant);
		ChantAge.Add(Rng.FRandRange(0.f, 6.f));
		ChantBase.Add(At);
	}
	if (ACHNpc* He = SpawnNpc(TEXT("he"), TEXT("he"), HeSpot, 0.f, 1))
	{
		He->SetPose(TEXT("fist"));
	}

	// a line of riot police further down, between the people and where the truck will stop
	for (int32 Index = 0; Index < 9; ++Index)
	{
		SpawnNpc(FString::Printf(TEXT("riot%d"), Index), TEXT("guard"), FVector(4050.f + (Index % 2) * 90.f, -560.f + Index * 140.f, 0.f), 180.f, 500 + Index);
	}

	// ---- the truck: an old pickup, painted olive, a DShK on a mount in its bed
	UMaterialInterface* Olive = Lit(FLinearColor(0.11f, 0.12f, 0.08f), 0.7f, 0.5f);
	UMaterialInterface* Dark = Lit(FLinearColor(0.02f, 0.02f, 0.022f), 0.4f, 0.3f, 0.6f);
	UMaterialInterface* Glass = Lit(FLinearColor(0.03f, 0.04f, 0.05f), 0.1f, 0.f);
	TruckPivot = Pivot(FVector(TruckStartX, 0.f, 0.f), FRotator(0.f, 180.f, 0.f));
	BoxOn(TruckPivot, FVector(100.f, 0.f, 95.f), FVector(170.f, 200.f, 120.f), Olive, true);           // cab
	BoxOn(TruckPivot, FVector(150.f, 0.f, 128.f), FVector(4.f, 180.f, 50.f), Glass, false, FRotator(-25.f, 0.f, 0.f));
	BoxOn(TruckPivot, FVector(240.f, 0.f, 62.f), FVector(110.f, 200.f, 55.f), Olive, true);           // bonnet
	BoxOn(TruckPivot, FVector(-130.f, 0.f, 62.f), FVector(290.f, 210.f, 55.f), Olive, true);          // bed
	BoxOn(TruckPivot, FVector(-130.f, 103.f, 110.f), FVector(290.f, 6.f, 45.f), Olive, false);
	BoxOn(TruckPivot, FVector(-130.f, -103.f, 110.f), FVector(290.f, 6.f, 45.f), Olive, false);
	for (int32 Wheel = 0; Wheel < 4; ++Wheel)
	{
		BoxOn(TruckPivot, FVector(Wheel < 2 ? 220.f : -150.f, Wheel % 2 ? 100.f : -100.f, 38.f), FVector(76.f, 30.f, 76.f), MTyre, false);
	}
	// the gun: a pedestal, the receiver, a long barrel over the cab towards the people
	BoxOn(TruckPivot, FVector(-110.f, 0.f, 150.f), FVector(16.f, 16.f, 120.f), Dark, false);
	BoxOn(TruckPivot, FVector(-95.f, 0.f, 220.f), FVector(70.f, 22.f, 26.f), Dark, false);
	BoxOn(TruckPivot, FVector(30.f, 0.f, 224.f), FVector(190.f, 7.f, 7.f), Dark, false);
	BoxOn(TruckPivot, FVector(120.f, 0.f, 224.f), FVector(14.f, 12.f, 12.f), Dark, false);
	BoxOn(TruckPivot, FVector(-80.f, 0.f, 190.f), FVector(8.f, 60.f, 50.f), Dark, false);          // shield
	Muzzle = BoxOn(TruckPivot, FVector(135.f, 0.f, 224.f), FVector(22.f, 22.f, 22.f), Glow(FLinearColor(1.f, 0.75f, 0.35f), CHLight::Emissive(30.f), 0.9f, 1.f), false);
	Muzzle->SetVisibility(false);
	MuzzleLight = PointLight(FVector(TruckStartX - 135.f, 0.f, 224.f), FLinearColor(1.f, 0.7f, 0.35f), 0.f, 2600.f, true, 2.f);
	if (ACHNpc* Gunner = SpawnNpc(TEXT("gunner"), TEXT("guard"), FVector(TruckStartX + 150.f, 0.f, 70.f), 180.f, 777))
	{
		Gunner->SetPose(TEXT("phone"));
	}
	UMaterialInterface* Tracer = Glow(FLinearColor(1.f, 0.82f, 0.4f), CHLight::Emissive(12.f), 0.9f);
	for (int32 Index = 0; Index < 12; ++Index)
	{
		UStaticMeshComponent* Round = Box(FVector::ZeroVector, FVector(70.f, 2.f, 2.f), Tracer, false, FRotator::ZeroRotator, true);
		Round->SetVisibility(false);
		Round->SetCastShadow(false);
		Tracers.Add(Round);
		TracerFrom.Add(FVector::ZeroVector);
		TracerTo.Add(FVector::ZeroVector);
		TracerAge.Add(-1.f);
	}

	// ---- the last thing he sees: his family, in a warm light, far from the street
	UMaterialInterface* Light = Unlit(FLinearColor(1.f, 0.86f, 0.66f), CHLight::Emissive(1.4f));
	Box(VisionAt + FVector(900.f, 0.f, 400.f), FVector(20.f, 4000.f, 2000.f), Light, false);
	Box(VisionAt + FVector(0.f, 0.f, -10.f), FVector(4000.f, 4000.f, 20.f), Lit(FLinearColor(0.75f, 0.66f, 0.55f), 1.f, 0.f), true);
	PointLight(VisionAt + FVector(-200.f, 0.f, 300.f), FLinearColor(1.f, 0.82f, 0.62f), 40.f, 2200.f, false, 0.f);
	PointLight(VisionAt + FVector(500.f, 0.f, 200.f), FLinearColor(1.f, 0.78f, 0.55f), 25.f, 1500.f, false, 0.f);
	const TCHAR* Family[][2] = { { TEXT("vmom"), TEXT("mother") }, { TEXT("vdad"), TEXT("father") }, { TEXT("vbrother"), TEXT("brother") }, { TEXT("vsister"), TEXT("sister") } };
	for (int32 Index = 0; Index < 4; ++Index)
	{
		if (ACHNpc* Person = SpawnNpc(Family[Index][0], Family[Index][1], VisionAt + FVector(250.f, 80.f + Index * 72.f, 0.f), 195.f, 900 + Index))
		{
			Person->SetPose(Index == 0 ? TEXT("cry") : TEXT(""));
		}
	}

	// ---- cameras
	Mark(TEXT("PlayerStart"), FVector(-500.f, -300.f, 0.f), FRotator::ZeroRotator);
	MarkLookAt(TEXT("Wide"), FVector(-300.f, -1050.f, 480.f), FVector(2600.f, 100.f, 150.f), 64.f);
	MarkLookAt(TEXT("Crowd"), FVector(1100.f, 760.f, 230.f), FVector(2700.f, -200.f, 170.f), 58.f);
	MarkLookAt(TEXT("HeClose"), HeSpot + FVector(280.f, 230.f, 165.f), HeSpot + FVector(0.f, 0.f, 150.f), 42.f);
	MarkLookAt(TEXT("TruckCam"), FVector(2850.f, -760.f, 210.f), FVector(TruckStopX, 0.f, 230.f), 52.f);
	MarkLookAt(TEXT("Gun"), FVector(TruckStopX - 420.f, 260.f, 210.f), FVector(TruckStopX - 60.f, 0.f, 220.f), 46.f);
	MarkLookAt(TEXT("HeEyes"), HeSpot + FVector(10.f, 0.f, 166.f), FVector(TruckStopX, 0.f, 230.f), 70.f);
	// he lies in the right half of the picture: the left is his lost eye
	MarkLookAt(TEXT("Fallen"), HeSpot + FVector(300.f, -430.f, 340.f), HeSpot + FVector(270.f, 0.f, 0.f), 46.f);
	MarkLookAt(TEXT("Vision"), VisionAt + FVector(-420.f, 0.f, 150.f), VisionAt + FVector(260.f, 0.f, 135.f), 50.f);
	TruckX = TruckStartX;
	TruckTargetX = TruckStartX;
}

void ACHLoc_Streets::OnMassacreEvent(const FString& Event)
{
	ACHPlayerController* Controller = Cast<ACHPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	if (Event == TEXT("truck"))
	{
		TruckTargetX = TruckStopX;
		if (Director)
		{
			Director->PlaySfx(TEXT("loop_engine"), 0.9f);
		}
	}
	else if (Event == TEXT("open_fire"))
	{
		bFiring = true;
		FireTime = 0.f;
		// the chants break off
		for (int32 Index = 0; Index < ChantAge.Num(); ++Index)
		{
			ChantAge[Index] = FMath::Max(ChantAge[Index], 5.2f);
		}
		if (Controller)
		{
			Controller->Shake(6.f, 1.4f);
		}
	}
	else if (Event == TEXT("pov"))
	{
		// his own eyes
		if (ACHNpc* He = FindNpc(TEXT("he")))
		{
			He->SetActorHiddenInGame(true);
		}
	}
	else if (Event == TEXT("hit"))
	{
		if (Controller)
		{
			Controller->Shake(0.6f, 6.f);
		}
		if (Director)
		{
			Director->Flash(FLinearColor(1.f, 0.95f, 0.9f), 0.25f);
		}
		if (ACameraActor* Camera = GetCamera(TEXT("HeEyes")))
		{
			FallFrom = Camera->GetActorTransform();
			FallTime = 0.f;
		}
	}
	else if (Event == TEXT("lay_he"))
	{
		bFiring = false;
		// the street has emptied: whoever was still running is gone
		for (int32 Index = 0; Index < 80; ++Index)
		{
			const FString Name = FString::Printf(TEXT("crowd%d"), Index);
			if (const ACHNpc* Person = FindNpc(Name))
			{
				if (Person->IsMoving() || FVector::Dist2D(Person->GetActorLocation(), HeSpot) < 650.f || FVector::Dist2D(Person->GetBody()->GetBoneLocation(TEXT("pelvis")), HeSpot) < 650.f)
				{
					RemoveNpc(Name);
				}
			}
		}
		// a fresh body where he stood: it falls and lies still
		RemoveNpc(TEXT("he"));
		if (ACHNpc* Body = SpawnNpc(TEXT("he_body"), TEXT("he"), HeSpot, 160.f, 1))
		{
			Body->LieDown();
		}
	}
}

void ACHLoc_Streets::TickMassacre(float DeltaSeconds)
{
	if (!TruckPivot)
	{
		return;
	}
	// the truck rolls in and stops
	if (!FMath::IsNearlyEqual(TruckX, TruckTargetX, 1.f))
	{
		TruckX = FMath::FInterpConstantTo(TruckX, TruckTargetX, DeltaSeconds, FMath::Max(120.f, FMath::Abs(TruckX - TruckTargetX) * 0.9f));
		TruckPivot->SetRelativeLocation(FVector(TruckX, 0.f, FMath::Sin(StreetsTime * 9.f) * 0.8f));
		if (ACHNpc* Gunner = FindNpc(TEXT("gunner")))
		{
			Gunner->SetActorLocation(FVector(TruckX + 150.f, 0.f, 70.f + 92.f));
		}
	}
	const FVector MuzzleAt(TruckX - 135.f, 0.f, 224.f);
	if (MuzzleLight)
	{
		MuzzleLight->SetWorldLocation(MuzzleAt);
	}

	if (bFiring)
	{
		FireTime += DeltaSeconds;
		// bursts: a second of fire, a breath, again
		const bool bBurst = FMath::Fmod(FireTime, 1.6f) < 1.1f;
		const bool bFlash = bBurst && FMath::Fmod(FireTime * 11.f, 1.f) < 0.45f;
		if (Muzzle)
		{
			Muzzle->SetVisibility(bFlash);
		}
		if (MuzzleLight)
		{
			MuzzleLight->SetIntensity(bFlash ? 60.f : 0.f);
		}
		BurstTimer -= DeltaSeconds;
		if (bBurst && BurstTimer <= 0.f)
		{
			BurstTimer = 1.6f;
			if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
			{
				Director->PlaySfx(TEXT("sfx_dshk"), 1.f);
			}
		}
		// tracers into the street
		TracerTimer -= DeltaSeconds;
		if (bBurst && TracerTimer <= 0.f)
		{
			TracerTimer = 0.09f;
			for (int32 Index = 0; Index < Tracers.Num(); ++Index)
			{
				if (TracerAge[Index] < 0.f)
				{
					TracerAge[Index] = 0.f;
					TracerFrom[Index] = MuzzleAt;
					TracerTo[Index] = FVector(FMath::FRandRange(-600.f, 2600.f), FMath::FRandRange(-900.f, 900.f), FMath::FRandRange(0.f, 260.f));
					Tracers[Index]->SetVisibility(true);
					Tracers[Index]->SetWorldRotation((TracerTo[Index] - TracerFrom[Index]).Rotation());
					break;
				}
			}
		}
		// the people fall, the people run; never anything more
		VictimTimer -= DeltaSeconds;
		if (VictimTimer <= 0.f && Crowd.Num() > 0)
		{
			VictimTimer = FMath::FRandRange(0.12f, 0.3f);
			const int32 Pick = FMath::RandRange(0, Crowd.Num() - 1);
			if (ACHNpc* Person = Crowd[Pick])
			{
				if (FVector::Dist2D(Person->GetActorLocation(), HeSpot) > 160.f && FMath::FRand() < 0.4f)
				{
					Person->PlayFall(true);
				}
				else
				{
					Person->SetPose(TEXT(""));
					Person->WalkTo(FVector(FMath::FRandRange(-900.f, -300.f), Person->GetActorLocation().Y + FMath::FRandRange(-300.f, 300.f), 0.f), FMath::FRandRange(380.f, 520.f));
				}
				Crowd.RemoveAt(Pick);
			}
		}
	}
	for (int32 Index = 0; Index < Tracers.Num(); ++Index)
	{
		if (TracerAge[Index] < 0.f)
		{
			continue;
		}
		TracerAge[Index] += DeltaSeconds;
		const float Length = FVector::Dist(TracerFrom[Index], TracerTo[Index]);
		const float T = TracerAge[Index] * 9000.f / FMath::Max(1.f, Length);
		if (T >= 1.f)
		{
			TracerAge[Index] = -1.f;
			Tracers[Index]->SetVisibility(false);
			continue;
		}
		Tracers[Index]->SetWorldLocation(FMath::Lerp(TracerFrom[Index], TracerTo[Index], T));
	}

	// his eyes falling to the ground: the view drops, tips over, settles on the asphalt
	if (FallTime >= 0.f)
	{
		FallTime += DeltaSeconds;
		if (ACameraActor* Camera = GetCamera(TEXT("HeEyes")))
		{
			const float T = FMath::Clamp(FallTime / 1.3f, 0.f, 1.f);
			const float Drop = T * T;
			const FRotator Start = FallFrom.Rotator();
			const FVector Location = FallFrom.GetLocation() + FVector(-30.f * T, 18.f * T, -150.f * Drop);
			Camera->SetActorLocationAndRotation(Location, FRotator(Start.Pitch + 14.f * T, Start.Yaw - 25.f * T, -78.f * FMath::InterpEaseOut(0.f, 1.f, T, 2.5f)));
		}
		if (FallTime > 2.f)
		{
			FallTime = -1.f;
		}
	}
}
