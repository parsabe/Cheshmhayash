// Cheshm Hayash - the rooftop at the city's edge, facing east toward the mountains.
// East is +X. Damavand stands to the north-east, the sun comes up a little south of east.

#include "World/CHLocations.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Game/CHFilmable.h"
#include "Game/CHInteractable.h"
#include "Game/CHSegment.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/RandomStream.h"
#include "Story/CHJson.h"
#include "UI/CHStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "World/CHNpc.h"

namespace
{
	const float RoofC = 1220.f;
	const float SunAzimuth = -8.f;
	const FVector SitSpot(1850.f, 100.f, 1220.f);
	const FVector BodySpot(1660.f, 70.f, 1220.f);
	const FVector PhoneSpot(1720.f, -10.f, 1221.2f);
}

void ACHLoc_FinalRoof::Build(const FString& InVariant)
{
	Super::Build(InVariant);
	InitCity(true, -0.4f);

	PlayerBoomLength = 300.f;
	PlayerBoomOffset = FVector(0.f, 50.f, 70.f);

	// ---- the sky before dawn (bright and clear so the rooftops and Alborz mountains are fully visible)
	FCHEnv Env;
	Env.bSunLightsWorld = true;
	Env.bSunShadows = true;
	Env.SunVolumetric = 1.6f;
	Env.SkyLightIntensity = 2.8f;
	Env.SkyLightColor = FLinearColor(0.75f, 0.80f, 0.95f);
	Env.bFog = true;
	Env.FogDensity = 0.005f;
	Env.FogHeightFalloff = 0.04f;
	Env.FogColor = FLinearColor(0.12f, 0.14f, 0.22f);
	Env.bVolumetricFog = true;
	Env.VolumetricExtinction = 0.25f;
	Env.ExposureEV = -0.5f;
	Env.Grain = 0.18f;
	Env.Vignette = 0.28f;
	Env.Fringe = 0.18f;
	Env.Bloom = 0.9f;
	Env.Saturation = 1.05f;
	Env.Contrast = 1.04f;
	Env.Temperature = 6400.f;
	ApplyEnv(Env);
	// "night": the first night he climbs up here (1.7); warm dawn light in 2.6
	const bool bFirstNight = Variant == TEXT("night");
	SetSun(bFirstNight ? 12.f : 16.f, 0.f);

	// Continuous solid floor slab at RoofC across the entire rooftop and beyond so nobody can ever fall or be blocked
	if (UStaticMeshComponent* RoofFloor = BoxAt(-6000.f, -6000.f, RoofC - 28.f, 16000.f, 12000.f, 28.f, MRoof, true))
	{
		RoofFloor->SetVisibility(false);
		RoofFloor->SetCastShadow(false);
	}

	// ---- three connected roofs at the same level RoofC
	BuildRoof(-1800.f, -600.f, 1350.f, 1200.f, RoofC, 1 | 2 | 4 | 8, 11);
	BuildRoof(-450.f, -700.f, 1330.f, 1200.f, RoofC, 1 | 2 | 4 | 8, 12);
	BuildRoof(880.f, -550.f, 1120.f, 1200.f, RoofC, 1 | 2 | 4 | 8, 13);

	// Warm rooftop lights along his path so everything is clearly lit
	for (float LX = -1300.f; LX <= 1800.f; LX += 620.f)
	{
		PointLight(FVector(LX, 60.f, RoofC + 260.f), FLinearColor(1.f, 0.78f, 0.52f), 14.f, 1400.f, false, 0.4f);
	}

	// the stair head he came up through, its door open
	{
		UMaterialInterface* Wall = MFacades.Num() > 2 ? MFacades[2] : MBand;
		BoxAt(-1780.f, -170.f, RoofC, 260.f, 340.f, 250.f, Wall, false);
		Box(FVector(-1519.f, 0.f, RoofC + 105.f), FVector(3.f, 100.f, 210.f), Emissive(FLinearColor(1.f, 0.6f, 0.3f), CHLight::Emissive(0.6f), FLinearColor(0.02f, 0.015f, 0.01f)), false);
	}

	// things on the roofs, kept off his path (no blocking collision)
	UMaterialInterface* Cloth = Lit(FLinearColor(0.42f, 0.40f, 0.36f), 1.f, 0.1f);
	Cyl(FVector(-1000.f, -420.f, RoofC + 70.f), 60.f, 140.f, MTank, false);
	Box(FVector(-1300.f, 420.f, RoofC + 50.f), FVector(110.f, 110.f, 100.f), MCooler, false);
	Ball(FVector(-800.f, 460.f, RoofC + 100.f), FVector(88.f, 88.f, 20.f), MDish, false, FRotator(58.f, 190.f, 0.f));
	Cyl(FVector(-800.f, 460.f, RoofC + 45.f), 3.f, 90.f, MPole, false);
	Cyl(FVector(300.f, -500.f, RoofC + 165.f), 58.f, 140.f, MTankBlue, false);
	Box(FVector(300.f, -500.f, RoofC + 47.f), FVector(120.f, 120.f, 94.f), MRust, false);
	BoxInstance(FVector(100.f, 360.f, RoofC + 170.f), FVector(800.f, 1.5f, 1.5f), MWire);
	for (int32 Index = 0; Index < 5; ++Index)
	{
		BoxInstance(FVector(-220.f + Index * 150.f, 360.f, RoofC + 140.f), FVector(70.f, 2.f, 55.f), Index % 2 ? Cloth : MLaundry[Index % MLaundry.Num()]);
	}
	Box(FVector(1100.f, -400.f, RoofC + 50.f), FVector(110.f, 110.f, 100.f), MCooler, false);
	Cyl(FVector(1260.f, 480.f, RoofC + 75.f), 62.f, 150.f, MTank, false);
	Cyl(FVector(1500.f, -420.f, RoofC + 160.f), 2.5f, 320.f, MPole, false);
	Box(FVector(1500.f, -420.f, RoofC + 280.f), FVector(4.f, 130.f, 4.f), MPole, false);
	Ball(FVector(1900.f, -440.f, RoofC + 95.f), FVector(80.f, 80.f, 18.f), MDish, false, FRotator(60.f, 200.f, 0.f));

	// ---- the city around and below, and the mountains close and clear
	BuildDistantCity();
	BuildAlborz(-14.f, FLinearColor(0.32f, 0.35f, 0.45f), FLinearColor(0.92f, 0.94f, 0.98f), 140000.f, true);

	// ---- marks and cinematic cameras for his rooftop walk toward the mountains
	Mark(TEXT("PlayerStart"), FVector(-1380.f, 0.f, RoofC), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("RoofWalkStart"), FVector(250.f, 100.f, RoofC), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("RoofWalkMid"), FVector(1150.f, 100.f, RoofC), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("RoofEdge"), SitSpot, FRotator(0.f, -8.f, 0.f));
	MarkLookAt(TEXT("WalkCam1"), FVector(-120.f, -260.f, RoofC + 185.f), FVector(1200.f, 100.f, RoofC + 125.f), 58.f);
	MarkLookAt(TEXT("WalkCam2"), FVector(1320.f, -240.f, RoofC + 155.f), SitSpot + FVector(400.f, 80.f, 120.f), 52.f);
	MarkLookAt(TEXT("MountainWide"), FVector(1520.f, -120.f, RoofC + 165.f), FVector(28000.f, -3500.f, 5200.f), 56.f);

	if (bFirstNight)
	{
		if (ACHFilmable* City = SpawnFilmable(TEXT("city"), TEXT("شهرِ تاریک، پر از صدا"), TEXT("The dark city, full of voices"), FVector(40000.f, 2000.f, 1000.f), 3.f, 90000.f))
		{
			City->bIgnoreSight = true;
			City->AimAngle = 60.f;
			City->SetGlowVisible(false);
		}
		MarkLookAt(TEXT("Climb"), FVector(-1100.f, -420.f, RoofC + 210.f), FVector(600.f, 300.f, RoofC + 60.f), 62.f);
		return;
	}
	if (ACHInteractable* Sit = SpawnInteractable(TEXT("sit"), TEXT("بنشین"), TEXT("Sit down"), SitSpot + FVector(0.f, 0.f, 60.f), 320.f))
	{
		Sit->SetActorHiddenInGame(false);
	}

	// the sunrise he will film (far away, in the orange)
	const FVector SunPoint = FRotator(3.f, -8.f, 0.f).Vector() * 40000.f + SitSpot;
	if (ACHFilmable* Sunrise = SpawnFilmable(TEXT("sunrise"), TEXT("نگاه کن… داره میاد بالا"), TEXT("Look... it's coming up"), SunPoint, 4.f, 80000.f))
	{
		Sunrise->bIgnoreSight = true;
		Sunrise->AimAngle = 45.f;
		Sunrise->SetGlowVisible(false);
	}

	// cameras: his phone, the last shot, the phone where it fell
	Mark(TEXT("PhoneView"), SitSpot + FVector(15.f, 0.f, 108.f), FRotator(3.f, -9.f, 0.f), 68.f);
	MarkLookAt(TEXT("Approach"), FVector(1200.f, -380.f, RoofC + 220.f), SitSpot + FVector(600.f, 120.f, 150.f), 60.f);
	Mark(TEXT("LastShot"), FVector(1330.f, 90.f, RoofC + 48.f), FRotator(3.f, -3.f, 0.f), 62.f);
	MarkLookAt(TEXT("PhoneClose"), PhoneSpot + FVector(-22.f, -18.f, 34.f), PhoneSpot, 34.f);
	MarkLookAt(TEXT("Wide"), FVector(-700.f, 900.f, 1900.f), FVector(2400.f, -100.f, 1400.f), 65.f);
	Mark(TEXT("Body"), BodySpot, FRotator(0.f, 90.f, 0.f));

	// the phone, where it will fall
	PhoneBody = Box(PhoneSpot, FVector(15.f, 7.4f, 0.9f), Lit(FLinearColor(0.01f, 0.01f, 0.012f), 0.25f, 0.f), false, FRotator(0.f, 32.f, 0.f), true);
	PhoneBody->SetVisibility(false);
	PhoneScreen = WorldWidget(MakePhoneScreen(), PhoneSpot + FVector(0.f, 0.f, 0.55f), FRotator(90.f, 32.f, 0.f), 0.02f, true, FVector2D(350.0, 720.0));
	PhoneScreen->SetVisibility(false);
}

void ACHLoc_FinalRoof::BuildRoof(float MinX, float MinY, float SizeX, float SizeY, float Height, uint8 LowSides, int32 Seed)
{
	FCHBuildingStyle Style;
	Style.Floors = FMath::Max(1, FMath::FloorToInt32((Height - 380.f) / 310.f) + 1);
	Style.HeightOverride = Height;
	Style.Facades = 0xF;
	Style.LowParapets = LowSides;
	Style.bRoofProps = false;
	Style.LitWindows = 0.1f;
	Style.Seed = Seed;
	Building(MinX, MinY, SizeX, SizeY, Style);
}

void ACHLoc_FinalRoof::BuildDistantCity()
{
	FRandomStream Rng(1401);

	// the street below the edge, and the blocks around the three roofs
	StreetY(-9000.f, 9000.f, 2850.f, 800.f, 450.f, 21, true);
	const struct { float X; float Y; float SX; float SY; int32 Floors; } Around[] = {
		{ -2600.f, 800.f, 4600.f, 1500.f, 4 },
		{ -2600.f, -2350.f, 4600.f, 1500.f, 3 },
		{ -4200.f, -1300.f, 2300.f, 2600.f, 5 },
		{ 3700.f, -3600.f, 1500.f, 2300.f, 2 },
		{ 3700.f, -1100.f, 1500.f, 2000.f, 3 },
		{ 3700.f, 1100.f, 1500.f, 2400.f, 2 },
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Around); ++Index)
	{
		FCHBuildingStyle Style;
		Style.Floors = Around[Index].Floors;
		Style.Seed = 500 + Index;
		Style.LitWindows = 0.08f;
		Building(Around[Index].X, Around[Index].Y, Around[Index].SX, Around[Index].SY, Style);
	}

	// ground to the horizon
	BoxAt(-12000.f, -90000.f, -40.f, 110000.f, 180000.f, 40.f, MAsphalt);

	// the low city stretching east toward the hills
	UMaterialInterface* Tones[] = {
		Lit(FLinearColor(0.20f, 0.18f, 0.16f), 0.95f, 0.4f),
		Lit(FLinearColor(0.26f, 0.23f, 0.19f), 0.95f, 0.4f),
		Lit(FLinearColor(0.15f, 0.145f, 0.14f), 0.95f, 0.4f) };
	for (int32 Index = 0; Index < 1100; ++Index)
	{
		const float X = Rng.FRandRange(5600.f, 70000.f);
		const float Spread = 4000.f + X * 0.95f;
		const float Y = Rng.FRandRange(-Spread, Spread);
		const float W = Rng.FRandRange(700.f, 2000.f);
		const float D = Rng.FRandRange(700.f, 2000.f);
		const float H = (Rng.FRand() < 0.85f ? Rng.FRandRange(300.f, 800.f) : Rng.FRandRange(800.f, 1400.f)) * (X < 12000.f ? 0.75f : 1.f);
		BoxInstance(FVector(X, Y, H * 0.5f), FVector(W, D, H), Tones[Index % 3], FRotator(0.f, Rng.FRandRange(-8.f, 8.f), 0.f));
	}

	// street lights still on in the waking city
	UMaterialInterface* LampDot = Emissive(FLinearColor(1.f, 0.55f, 0.2f), CHLight::Emissive(18.f), FLinearColor(0.1f, 0.05f, 0.02f));
	for (int32 Index = 0; Index < 420; ++Index)
	{
		const float X = Rng.FRandRange(4000.f, 60000.f);
		const float Spread = 3000.f + X * 0.9f;
		BoxInstance(FVector(X, Rng.FRandRange(-Spread, Spread), Rng.FRandRange(450.f, 650.f)), FVector(30.f, 30.f, 30.f), LampDot);
	}

	// low hills between the city and the mountains
	UMaterialInterface* Hill = Lit(FLinearColor(0.18f, 0.17f, 0.20f), 0.95f, 0.f);
	for (int32 Index = 0; Index < 9; ++Index)
	{
		const float Yaw = -50.f + Index * 12.f + Rng.FRandRange(-4.f, 4.f);
		const FVector At = FRotator(0.f, Yaw, 0.f).Vector() * Rng.FRandRange(75000.f, 115000.f);
		const float Radius = Rng.FRandRange(22000.f, 42000.f);
		const float Height = Rng.FRandRange(4000.f, 8500.f);
		Cone(FVector(At.X, At.Y, Height * 0.5f - 200.f), Radius, Height, Hill, false)->SetCastShadow(false);
	}
}

void ACHLoc_FinalRoof::SetSun(float Elevation, float Warmth)
{
	SunElevation = Elevation;
	if (!Sun)
	{
		return;
	}
	const float SafeElev = FMath::Max(Elevation, 12.f);
	const float Up = FMath::Clamp((Elevation + 4.f) / 20.f, 0.f, 1.f);
	const float Smooth = FMath::SmoothStep(0.f, 1.f, Up);

	Sun->SetWorldRotation(FRotator(-SafeElev, SunAzimuth + 180.f, 0.f));
	Sun->SetIntensity(FMath::Lerp(10.f, 16.f, Smooth));
	Sun->SetLightColor(FMath::Lerp(FLinearColor(1.f, 0.65f, 0.42f), FLinearColor(1.f, 0.85f, 0.64f), Smooth));
	if (SkyLight)
	{
		SkyLight->SetIntensity(FMath::Lerp(2.8f, 2.4f, Smooth));
	}

	const float EV = FMath::Lerp(-0.5f, 0.2f, Smooth);
	CHLight::SetExposure(EV);
	if (PostProcess)
	{
		FPostProcessSettings& S = PostProcess->Settings;
		S.AutoExposureMinBrightness = EV;
		S.AutoExposureMaxBrightness = EV;
		S.AutoExposureBias = 1.2f;
		S.BloomIntensity = FMath::Lerp(0.8f, 1.4f, Smooth);
		S.WhiteTemp = FMath::Lerp(6300.f, 7400.f, Smooth);
		const float Saturation = FMath::Lerp(1.02f, 1.18f, Smooth);
		S.ColorSaturation = FVector4(Saturation, Saturation, Saturation, 1.0);
		const float Contrast = FMath::Lerp(1.04f, 1.10f, Smooth);
		S.ColorContrast = FVector4(Contrast, Contrast, Contrast, 1.0);
		S.FilmGrainIntensity = FMath::Lerp(0.18f, 0.10f, Smooth);
		S.VignetteIntensity = FMath::Lerp(0.28f, 0.22f, Smooth);
	}
	if (Fog)
	{
		Fog->SetFogDensity(FMath::Lerp(0.005f, 0.003f, Smooth));
		Fog->SetFogInscatteringColor(FMath::Lerp(FLinearColor(0.12f, 0.14f, 0.22f), FLinearColor(0.24f, 0.16f, 0.10f), Smooth));
		Fog->SetDirectionalInscatteringColor(FMath::Lerp(FLinearColor(0.35f, 0.20f, 0.12f), FLinearColor(0.75f, 0.42f, 0.18f), Smooth));
	}
}

void ACHLoc_FinalRoof::StartSegment(const FString& InSegment, const TSharedPtr<FJsonObject>& Beat)
{
	Super::StartSegment(InSegment, Beat);
	Segment = InSegment;
	if (Segment == TEXT("film"))
	{
		// the first light while he films: the sun crests as the clip is saved
		SunFrom = SunElevation;
		SunTo = 2.2f;
		SunTime = 0.f;
		SunDuration = 11.f;
		// nothing else to use up here now
		for (TActorIterator<ACHInteractable> It(GetWorld()); It; ++It)
		{
			It->SetEnabled(false);
		}
	}
}

void ACHLoc_FinalRoof::OnInteract(const FString& Id, ACHInteractable* What)
{
	if (Id == TEXT("sit") && What)
	{
		What->SetEnabled(false);
		FinishSegment();
	}
}

void ACHLoc_FinalRoof::OnFilmed(const FString& Id)
{
	if (Id == TEXT("sunrise") || Id == TEXT("city"))
	{
		FinishSegment();
	}
}

void ACHLoc_FinalRoof::OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat)
{
	if (Event == TEXT("sunrise"))
	{
		SunFrom = SunElevation;
		SunTo = Beat.IsValid() ? CHJson::Float(*Beat, TEXT("to"), 16.f) : 16.f;
		SunTime = 0.f;
		SunDuration = Beat.IsValid() ? CHJson::Float(*Beat, TEXT("time"), 50.f) : 50.f;
	}
	else if (Event == TEXT("phone_fall"))
	{
		if (ACameraActor* Camera = GetCamera(TEXT("PhoneView")))
		{
			FallFrom = Camera->GetActorTransform();
			FallTime = 0.f;
		}
	}
	else if (Event == TEXT("lay_body"))
	{
		if (ACHNpc* Body = SpawnNpc(TEXT("he"), TEXT("he"), BodySpot, -8.f))
		{
			Body->PlayFall(true);
		}
		PhoneBody->SetVisibility(true);
	}
	else if (Event == TEXT("frame_last_shot"))
	{
		// frame him wherever the fall left him: low on the roof, looking east past him at the sun
		FVector Center = BodySpot;
		if (const ACHNpc* Body = FindNpc(TEXT("he")))
		{
			const FVector Pelvis = Body->GetBody()->GetBoneLocation(TEXT("pelvis"));
			// trust the ragdoll only if it is still on this roof
			if (FVector::Dist2D(Pelvis, BodySpot) < 400.f && FMath::Abs(Pelvis.Z - RoofC) < 150.f)
			{
				Center = FVector(Pelvis.X, Pelvis.Y, RoofC);
			}
		}
		if (ACameraActor* Camera = GetCamera(TEXT("LastShot")))
		{
			Camera->SetActorLocationAndRotation(FVector(Center.X - 330.f, Center.Y - 70.f, RoofC + 40.f), FRotator(2.f, -6.f, 0.f));
			UE_LOG(LogTemp, Log, TEXT("CH last shot: body %s camera %s"), *Center.ToString(), *Camera->GetActorLocation().ToString());
		}
		// the phone lies a little beyond his hand, on the side away from the camera's body
		const FVector Phone(Center.X + 35.f, Center.Y - 135.f, PhoneSpot.Z);
		PhoneBody->SetWorldLocation(Phone);
		PhoneScreen->SetWorldLocation(Phone + FVector(0.f, 0.f, 0.55f));
		if (ACameraActor* Close = GetCamera(TEXT("PhoneClose")))
		{
			const FVector Eye = Phone + FVector(-50.f, -110.f, 120.f);
			Close->SetActorLocationAndRotation(Eye, (Phone - Eye).Rotation());
			if (UCameraComponent* Lens = Close->GetCameraComponent())
			{
				Lens->SetFieldOfView(50.f);
			}
			UE_LOG(LogTemp, Log, TEXT("CH phone close: phone %s eye %s rot %s"), *Phone.ToString(), *Eye.ToString(), *Close->GetActorRotation().ToString());
		}
	}
	else if (Event == TEXT("phone_message"))
	{
		PhoneScreen->SetVisibility(true);
		PhoneScreen->RequestRedraw();
		if (UMaterialInstanceDynamic* Screen = PhoneScreen->GetMaterialInstance())
		{
			Screen->SetScalarParameterValue(TEXT("EmissiveStrength"), CHLight::Emissive(1.1f));
		}
	}
}

void ACHLoc_FinalRoof::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SunTime < SunDuration && !FMath::IsNearlyEqual(SunFrom, SunTo))
	{
		SunTime = FMath::Min(SunTime + DeltaSeconds, SunDuration);
		const float T = SunTime / FMath::Max(SunDuration, 0.01f);
		SetSun(FMath::Lerp(SunFrom, SunTo, FMath::SmoothStep(0.f, 1.f, T)), 0.f);
	}

	if (FallTime >= 0.f)
	{
		FallTime += DeltaSeconds;
		if (ACameraActor* Camera = GetCamera(TEXT("PhoneView")))
		{
			const float T = FMath::Clamp(FallTime / 1.1f, 0.f, 1.f);
			const float Drop = T * T;
			// the phone tips over and lands on its side; a small bounce at the end
			const float Bounce = FallTime > 1.1f ? FMath::Sin(FMath::Min(FallTime - 1.1f, 0.35f) / 0.35f * UE_PI) * 4.f : 0.f;
			const FVector Location = FallFrom.GetLocation() + FVector(22.f * T, -6.f * T, -100.f * Drop + Bounce);
			const FRotator Start = FallFrom.Rotator();
			const float Roll = 84.f * FMath::InterpEaseOut(0.f, 1.f, T, 2.5f) - Bounce * 1.5f;
			Camera->SetActorLocationAndRotation(Location, FRotator(Start.Pitch - 2.f * T, Start.Yaw + 7.f * T, Roll));
		}
		if (FallTime > 2.f)
		{
			FallTime = -1.f;
		}
	}
}

TSharedRef<SWidget> ACHLoc_FinalRoof::MakePhoneScreen() const
{
	// the lock screen of his phone: the time, and a message he will not read
	return SNew(SBox)
		.WidthOverride(350.f)
		.HeightOverride(720.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(FSlateColor(FLinearColor(0.012f, 0.014f, 0.02f, 1.f)))
			]
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			.Padding(18.f, 70.f, 18.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("06:04")))
					.Font(CH::LatinFont(64.f, CH::EWeight::Light))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.88f, 0.84f, 1.f)))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 70.f, 0.f, 0.f)
				[
					SNew(SBorder)
					.BorderImage(CH::RoundedBrush(18.f))
					.BorderBackgroundColor(FSlateColor(FLinearColor(0.16f, 0.17f, 0.19f, 1.f)))
					.Padding(FMargin(16.f, 14.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("KIAN  ·  now")))
							.Font(CH::LatinFont(16.f, CH::EWeight::Bold, 100))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.74f, 0.7f, 1.f)))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("صندلی کنار پنجره رزرو شد. دفعه‌ی بعد، باشه؟")))
							.Font(CH::FarsiFont(26.f))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.93f, 0.9f, 1.f)))
							.WrapTextAt(300.f)
							.TextShapingMethod(ETextShapingMethod::FullShaping)
							.TextFlowDirection(ETextFlowDirection::RightToLeft)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Window seat booked. Next time, right?")))
							.Font(CH::LatinFont(18.f, CH::EWeight::Regular))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.69f, 0.66f, 1.f)))
							.WrapTextAt(300.f)
						]
					]
				]
			]
		];
}
