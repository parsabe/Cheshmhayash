// Cheshm Hayash - the cemetery at sunrise (3.4). Rows of graves, many from 1401. His stone.
// East is +X, like the rooftop: the sun comes up beside Damavand exactly as it did that morning.
// The paths run east-west; his grave lies in the first row beside the middle path.

#include "World/CHLocations.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Game/CHInteractable.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/RandomStream.h"
#include "Story/CHJson.h"
#include "UI/CHStyle.h"
#include "UI/SCHShapes.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "World/CHLooks.h"
#include "World/CHNpc.h"

namespace
{
	const float SunAzimuth = -8.f;
	/** His grave: a polished dark stone, grey with dust until she washes it. */
	const FVector Grave(580.f, 180.f, 0.f);
	const FVector StoneSize(80.f, 155.f, 14.f);
	const FLinearColor DustyStone(0.27f, 0.26f, 0.25f);
	const FLinearColor CleanStone(0.035f, 0.035f, 0.04f);
	/** Where the name is cut into the stone (under the petals). */
	const FVector NameSpot(Grave.X, Grave.Y + 34.f, StoneSize.Z);
	const float StartElevation = 0.5f;
}

void ACHLoc_Cemetery::Build(const FString& InVariant)
{
	Super::Build(InVariant);
	InitCity(false, -2.f);

	PlayerBoomLength = 260.f;
	PlayerBoomOffset = FVector(0.f, 40.f, 60.f);

	FCHEnv Env;
	Env.bSunLightsWorld = true;
	Env.bSunShadows = true;
	Env.SunVolumetric = 1.6f;
	Env.SkyLightIntensity = 1.3f;
	Env.FogDensity = 0.014f;
	Env.FogHeightFalloff = 0.05f;
	Env.FogColor = FLinearColor(0.05f, 0.05f, 0.07f);
	Env.bVolumetricFog = true;
	Env.VolumetricExtinction = 0.45f;
	Env.ExposureEV = -2.f;
	Env.Grain = 0.2f;
	Env.Vignette = 0.42f;
	Env.Fringe = 0.18f;
	Env.Bloom = 0.9f;
	Env.Saturation = 1.f;
	Env.Contrast = 1.05f;
	Env.Temperature = 6500.f;
	ApplyEnv(Env);
	SetSun(StartElevation);
	BuildAlborz(-20.f, FLinearColor(0.06f, 0.06f, 0.075f), FLinearColor(0.58f, 0.58f, 0.62f), 1500000.f, true);

	FRandomStream Rng(1401);
	UMaterialInterface* Earth = Lit(FLinearColor(0.24f, 0.21f, 0.17f), 0.95f, 0.6f);
	UMaterialInterface* PathStone = Lit(FLinearColor(0.36f, 0.34f, 0.31f), 0.9f, 0.5f);
	UMaterialInterface* Stones[] = {
		Lit(FLinearColor(0.34f, 0.33f, 0.32f), 0.6f, 0.3f),
		Lit(FLinearColor(0.05f, 0.05f, 0.055f), 0.3f, 0.2f),
		Lit(FLinearColor(0.44f, 0.42f, 0.39f), 0.6f, 0.3f),
		Lit(FLinearColor(0.14f, 0.14f, 0.15f), 0.45f, 0.3f) };
	UMaterialInterface* Engraving = Lit(FLinearColor(0.62f, 0.58f, 0.48f), 0.7f, 0.f);
	UMaterialInterface* Cypress = Lit(FLinearColor(0.03f, 0.055f, 0.035f), 0.9f, 0.3f);
	UMaterialInterface* Trunk = Lit(FLinearColor(0.06f, 0.045f, 0.03f), 0.9f, 0.3f);
	UMaterialInterface* FlowersRed = Lit(FLinearColor(0.45f, 0.02f, 0.03f), 0.8f, 0.f);
	UMaterialInterface* FlowersWhite = Lit(FLinearColor(0.75f, 0.74f, 0.70f), 0.8f, 0.f);
	UMaterialInterface* Glass = Glow(FLinearColor(0.85f, 0.92f, 1.f), 0.2f, 0.18f);

	BoxAt(-20000.f, -20000.f, -40.f, 40000.f, 40000.f, 40.f, Earth);

	// paths between the blocks of graves
	for (float P = -2700.f; P <= 2700.f; P += 900.f)
	{
		BoxAt(-6000.f, P - 90.f, -38.f, 12000.f, 180.f, 40.f, PathStone);
	}

	// rows and rows of flat stones; a great many from that autumn
	for (float P = -2700.f; P < 2700.f; P += 900.f)
	{
		for (int32 Row = 0; Row < 4; ++Row)
		{
			const float RowY = P + 180.f + Row * 175.f;
			for (float X = -5500.f; X < 5500.f; X += 95.f)
			{
				if (FMath::IsNearlyEqual(RowY, Grave.Y, 1.f) && FMath::IsNearlyEqual(X, Grave.X, 1.f))
				{
					continue;
				}
				// the near side of each stone is engraved; many end in the same year
				UMaterialInterface* Material = Stones[Rng.RandRange(0, 3)];
				BoxInstance(FVector(X, RowY, 6.f), FVector(70.f, 140.f, 12.f), Material);
				BoxInstance(FVector(X, RowY + 28.f, 12.3f), FVector(36.f, 4.f, 0.8f), Engraving);
				BoxInstance(FVector(X, RowY + 4.f, 12.3f), FVector(28.f, 3.f, 0.8f), Engraving);
				BoxInstance(FVector(X, RowY - 30.f, 12.3f), FVector(18.f, 3.f, 0.8f), Engraving);
				const float Flower = Rng.FRand();
				if (Flower < 0.14f)
				{
					BoxInstance(FVector(X + Rng.FRandRange(-15.f, 15.f), RowY - 45.f, 15.f), FVector(10.f, 24.f, 6.f), Flower < 0.08f ? FlowersRed : FlowersWhite, FRotator(0.f, Rng.FRandRange(-30.f, 30.f), 0.f));
				}
			}
		}
	}

	// cypresses between the rows, away from his grave so nothing stands between it and the sun
	for (float P = -2700.f; P < 2700.f; P += 900.f)
	{
		for (float X = -5200.f; X < 5200.f; X += Rng.FRandRange(600.f, 1100.f))
		{
			const float Y = P + 442.f;
			if (FMath::Abs(X - Grave.X) < 1300.f && FMath::Abs(Y - Grave.Y) < 1000.f)
			{
				continue;
			}
			const float Height = Rng.FRandRange(650.f, 1050.f);
			Cyl(FVector(X, Y, 60.f), 9.f, 120.f, Trunk);
			Cone(FVector(X, Y, 100.f + Height * 0.5f), Rng.FRandRange(55.f, 75.f), Height, Cypress);
		}
	}

	// other mourners far off, in black
	const FVector Mourners[] = { FVector(-1900.f, 1080.f, 0.f), FVector(-1820.f, 1130.f, 0.f), FVector(2700.f, -620.f, 0.f), FVector(-3100.f, -760.f, 0.f) };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Mourners); ++Index)
	{
		SpawnNpc(FString::Printf(TEXT("mourner%d"), Index), Index == 3 ? TEXT("oldwoman") : TEXT("chador"), Mourners[Index], Rng.FRandRange(0.f, 360.f), 900 + Index);
	}

	// his stone: the epitaph cut into it; the name under rose petals
	Stone = Box(Grave + FVector(0.f, 0.f, StoneSize.Z * 0.5f), StoneSize, nullptr, true, FRotator::ZeroRotator, true);
	StoneMaterial = CHLooks::MakeLit(this, DustyStone, 0.75f);
	Stone->SetMaterial(0, StoneMaterial);
	// face up, the top of the page toward the head of the grave (+Y), read from the path
	const FRotator FaceUp = FRotationMatrix::MakeFromXZ(FVector::UpVector, FVector(0.f, 1.f, 0.f)).Rotator();
	WorldWidget(MakeTombstone(), Grave + FVector(0.f, 0.f, StoneSize.Z + 0.3f), FaceUp, 0.15f, false, FVector2D(470.0, 920.0));
	ScatterPetals(90, 0.f, 3);

	// the rosewater bottle she brought, the phone she will leave
	Cyl(Grave + FVector(-62.f, -95.f, 13.f), 5.5f, 24.f, Glow(FLinearColor(1.f, 0.82f, 0.86f), CHLight::Emissive(0.2f), 0.5f));
	Box(Grave + FVector(-62.f, -95.f, 27.f), FVector(4.f, 4.f, 5.f), Glass, false);
	Phone = Box(Grave + FVector(18.f, -46.f, StoneSize.Z + 0.6f), FVector(7.4f, 15.f, 0.9f), Lit(FLinearColor(0.01f, 0.01f, 0.012f), 0.25f, 0.f), false, FRotator(0.f, -8.f, 0.f), true);
	Phone->SetVisibility(false);

	// three things to do, one after another
	const FVector At = Grave + FVector(0.f, -150.f, 40.f);
	Steps.Add(SpawnInteractable(TEXT("rosewater"), TEXT("گلاب بریز و سنگ را بشوی"), TEXT("Pour the rosewater, wash the stone"), At, 190.f));
	Steps.Add(SpawnInteractable(TEXT("petals"), TEXT("گلبرگ‌ها را بپاش"), TEXT("Scatter the petals"), At, 190.f));
	Steps.Add(SpawnInteractable(TEXT("phone"), TEXT("گوشی‌اش را روی سنگ بگذار"), TEXT("Place his phone on the stone"), At, 190.f));
	for (int32 Index = 1; Index < Steps.Num(); ++Index)
	{
		if (Steps[Index])
		{
			Steps[Index]->SetEnabled(false);
		}
	}

	// marks: where she comes from, where she kneels; the cameras
	Mark(TEXT("PlayerStart"), FVector(-1150.f, -20.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("Kneel"), Grave + FVector(0.f, -125.f, 0.f), FRotator(0.f, 90.f, 0.f));
	MarkLookAt(TEXT("Dawn"), FVector(-1420.f, 30.f, 175.f), FVector(4000.f, -650.f, 420.f), 58.f);
	MarkLookAt(TEXT("Side"), Grave + FVector(-330.f, -260.f, 120.f), Grave + FVector(30.f, -60.f, 50.f), 44.f);
	MarkLookAt(TEXT("Stone"), Grave + FVector(0.f, -92.f, 132.f), Grave + FVector(0.f, 6.f, 10.f), 46.f);
	MarkLookAt(TEXT("Name"), NameSpot + FVector(0.f, -38.f, 52.f), NameSpot, 40.f);
	MarkLookAt(TEXT("Low"), Grave + FVector(-135.f, 62.f, 22.f), Grave + FVector(4000.f, -520.f, 330.f), 52.f);
}

TSharedRef<SWidget> ACHLoc_Cemetery::MakeTombstone() const
{
	// cut into polished stone and filled with pale paint
	const FLinearColor Ink(0.80f, 0.76f, 0.66f, 1.f);
	auto Line = [&Ink](const TCHAR* Text, float Size, bool bBold)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Text))
			.Font(CH::FarsiFont(Size, bBold ? CH::EWeight::Bold : CH::EWeight::Regular))
			.ColorAndOpacity(FSlateColor(Ink))
			.Justification(ETextJustify::Center)
			.TextShapingMethod(ETextShapingMethod::FullShaping)
			.TextFlowDirection(ETextFlowDirection::RightToLeft);
	};

	return SNew(SBox)
		.WidthOverride(470.f)
		.HeightOverride(920.f)
		.Padding(FMargin(26.f, 40.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(SBox).WidthOverride(320.f).HeightOverride(150.f)
				[
					SNew(SCHSunEngraving).Color(Ink)
				]
			]
			// the name is cut here, under the petals; the game never writes it
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 120.f, 0.f, 0.f)
			[
				Line(TEXT("دانشجو · برادر · رفیق"), 36.f, false)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 74.f, 0.f, 0.f)
			[
				Line(TEXT("او چشم‌هایش را داد"), 40.f, true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f)
			[
				Line(TEXT("تا ما خورشید را ببینیم"), 40.f, true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 70.f, 0.f, 0.f)
			[
				Line(TEXT("۲۴ مهر ۱۴۰۱"), 36.f, false)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 56.f, 0.f, 0.f)
			[
				Line(TEXT("زن · زندگی · آزادی"), 36.f, true)
			]
		];
}

void ACHLoc_Cemetery::ScatterPetals(int32 Count, float Spread, int32 Seed)
{
	FRandomStream Rng(Seed);
	UMaterialInterface* Petals[] = {
		Lit(FLinearColor(0.55f, 0.03f, 0.05f), 0.7f, 0.f),
		Lit(FLinearColor(0.72f, 0.10f, 0.15f), 0.7f, 0.f),
		Lit(FLinearColor(0.40f, 0.02f, 0.04f), 0.7f, 0.f),
		Lit(FLinearColor(0.80f, 0.55f, 0.58f), 0.7f, 0.f) };
	// no spread: a thick drift over the place where the name is cut
	const bool bDrift = Spread <= 0.f;
	const FVector Center = bDrift ? NameSpot : Grave + FVector(0.f, 0.f, StoneSize.Z);
	const FVector2D Extent = bDrift ? FVector2D(28.f, 10.f) : FVector2D(StoneSize.X * 0.5f + Spread, StoneSize.Y * 0.5f + Spread);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		// most of the scattered petals land on the stone, some around it
		float DX = Rng.FRandRange(-Extent.X, Extent.X);
		float DY = Rng.FRandRange(-Extent.Y, Extent.Y);
		if (!bDrift && Rng.FRand() < 0.6f)
		{
			DX *= 0.45f;
			DY *= 0.5f;
		}
		const bool bOnStone = FMath::Abs(Center.X + DX - Grave.X) < StoneSize.X * 0.5f - 2.f && FMath::Abs(Center.Y + DY - Grave.Y) < StoneSize.Y * 0.5f - 2.f;
		const float Z = bOnStone ? StoneSize.Z + 0.5f + Rng.FRandRange(0.f, bDrift ? 1.6f : 0.6f) : 0.4f;
		BoxInstance(FVector(Center.X + DX, Center.Y + DY, Z), FVector(Rng.FRandRange(2.6f, 4.2f), Rng.FRandRange(1.8f, 3.f), 0.3f),
			Petals[Rng.RandRange(0, 3)], FRotator(Rng.FRandRange(-10.f, 10.f), Rng.FRandRange(0.f, 360.f), Rng.FRandRange(-10.f, 10.f)));
	}
}

void ACHLoc_Cemetery::OnInteract(const FString& Id, ACHInteractable* What)
{
	if (What)
	{
		What->SetEnabled(false);
	}
	auto Enable = [this](int32 Index)
	{
		if (Steps.IsValidIndex(Index) && Steps[Index])
		{
			Steps[Index]->SetEnabled(true);
		}
	};
	if (Id == TEXT("rosewater"))
	{
		// the rosewater darkens the stone and the dust comes away; it shines
		bWetting = true;
		Enable(1);
	}
	else if (Id == TEXT("petals"))
	{
		ScatterPetals(170, 34.f, 7);
		Enable(2);
	}
	else if (Id == TEXT("phone"))
	{
		Phone->SetVisibility(true);
		FinishSegment();
	}
}

void ACHLoc_Cemetery::OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat)
{
	if (Event == TEXT("sunrise"))
	{
		SunFrom = SunElevation;
		SunTo = Beat.IsValid() ? CHJson::Float(*Beat, TEXT("to"), 9.f) : 9.f;
		SunTime = 0.f;
		SunDuration = Beat.IsValid() ? CHJson::Float(*Beat, TEXT("time"), 60.f) : 60.f;
	}
}

void ACHLoc_Cemetery::ApplyWet()
{
	if (StoneMaterial)
	{
		StoneMaterial->SetVectorParameterValue(TEXT("Color"), FMath::Lerp(DustyStone, CleanStone, WetAmount));
		StoneMaterial->SetScalarParameterValue(TEXT("Roughness"), FMath::Lerp(0.75f, 0.08f, WetAmount));
	}
}

void ACHLoc_Cemetery::SetSun(float Elevation)
{
	SunElevation = Elevation;
	if (!Sun)
	{
		return;
	}
	const float Up = FMath::Clamp((Elevation + 1.f) / 11.f, 0.f, 1.f);
	const float Smooth = FMath::SmoothStep(0.f, 1.f, Up);
	Sun->SetWorldRotation(FRotator(-Elevation, SunAzimuth + 180.f, 0.f));
	Sun->SetIntensity(FMath::Lerp(8.f, 12.f, Smooth));
	Sun->SetLightColor(FMath::Lerp(FLinearColor(1.f, 0.42f, 0.16f), FLinearColor(1.f, 0.80f, 0.58f), Smooth));
	if (SkyLight)
	{
		SkyLight->SetIntensity(FMath::Lerp(3.f, 1.2f, Smooth));
	}
	const float EV = FMath::Lerp(-2.4f, 1.0f, Smooth);
	CHLight::SetExposure(EV);
	if (PostProcess)
	{
		FPostProcessSettings& S = PostProcess->Settings;
		S.AutoExposureMinBrightness = EV;
		S.AutoExposureMaxBrightness = EV;
		S.AutoExposureBias = 0.f;
		S.WhiteTemp = FMath::Lerp(6400.f, 7200.f, Smooth);
		S.BloomIntensity = FMath::Lerp(0.8f, 1.2f, Smooth);
	}
	if (Fog)
	{
		Fog->SetFogDensity(FMath::Lerp(0.014f, 0.006f, Smooth));
		Fog->SetFogInscatteringColor(FMath::Lerp(FLinearColor(0.05f, 0.05f, 0.07f), FLinearColor(0.09f, 0.06f, 0.045f), Smooth));
		Fog->SetDirectionalInscatteringColor(FMath::Lerp(FLinearColor(0.35f, 0.20f, 0.12f), FLinearColor(0.65f, 0.35f, 0.15f), Smooth));
	}
}

void ACHLoc_Cemetery::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SunTime < SunDuration && !FMath::IsNearlyEqual(SunFrom, SunTo))
	{
		SunTime = FMath::Min(SunTime + DeltaSeconds, SunDuration);
		SetSun(FMath::Lerp(SunFrom, SunTo, FMath::SmoothStep(0.f, 1.f, SunTime / FMath::Max(SunDuration, 0.01f))));
	}

	if (bWetting && WetAmount < 1.f)
	{
		WetAmount = FMath::Min(1.f, WetAmount + DeltaSeconds / 2.5f);
		ApplyWet();
	}
}
