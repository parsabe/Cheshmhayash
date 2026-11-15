// Cheshm Hayash - interiors of the family's life. Variants:
//   "dinner"  the living room the night she was taken (1.4)
//   "years"   the same room years later, the news on the television (3.3)
//   "office"  the lawyer's office (1.6)
//   "stairs"  the stairwell of their building (1.8)

#include "World/CHLocations.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Game/CHInteractable.h"
#include "Game/CHSegment.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/RandomStream.h"
#include "Story/CHSave.h"
#include "Story/CHStoryDirector.h"
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
	const float HX = 360.f;
	const float HY = 260.f;
	const float H = 280.f;
	const float T = 20.f;
}

void ACHLoc_Home::Build(const FString& InVariant)
{
	Super::Build(InVariant);
	PlayerBoomLength = 0.f;

	if (Variant == TEXT("office"))
	{
		BuildOffice();
	}
	else if (Variant == TEXT("stairs"))
	{
		BuildStairs();
	}
	else
	{
		BuildLiving();
	}
}

void ACHLoc_Home::BuildShell(const FLinearColor& WallColor, const FLinearColor& Outside, float OutsideStrength)
{
	UMaterialInterface* Wall = Lit(WallColor, 0.95f, 0.2f);
	UMaterialInterface* Ceiling = Lit(FLinearColor(0.45f, 0.43f, 0.39f), 0.95f, 0.1f);
	UMaterialInterface* Floor = Lit(FLinearColor(0.16f, 0.10f, 0.06f), 0.55f, 0.35f);

	BoxAt(-HX - T, -HY - T, -20.f, 2.f * (HX + T), 2.f * (HY + T), 20.f, Floor);
	BoxAt(-HX - T, -HY - T, H, 2.f * (HX + T), 2.f * (HY + T), 20.f, Ceiling);
	BoxAt(-HX - T, -HY - T, 0.f, T, 2.f * (HY + T), H, Wall);
	BoxAt(HX, -HY - T, 0.f, T, 2.f * (HY + T), H, Wall);
	BoxAt(-HX, -HY - T, 0.f, 2.f * HX, T, H, Wall);
	// the window wall (+Y)
	BoxAt(-HX, HY, 0.f, HX - 90.f, T, H, Wall);
	BoxAt(90.f, HY, 0.f, HX - 90.f, T, H, Wall);
	BoxAt(-90.f, HY, 0.f, 180.f, T, 90.f, Wall);
	BoxAt(-90.f, HY, 230.f, 180.f, T, H - 230.f, Wall);
	Box(FVector(0.f, HY + 300.f, 160.f), FVector(900.f, 4.f, 500.f), Unlit(Outside, OutsideStrength), false);
}

// ---------------------------------------------------------------------------------------------
// The living room
// ---------------------------------------------------------------------------------------------

void ACHLoc_Home::BuildLiving()
{
	const bool bYears = Variant == TEXT("years");
	const bool bMemorial = Variant == TEXT("memorial");
	const bool bAfter = bYears || bMemorial;
	const float EV = bAfter ? -0.6f : -0.4f;
	CHLight::SetExposure(EV);

	UMaterialInterface* Wood = Lit(FLinearColor(0.15f, 0.08f, 0.04f), 0.5f, 0.3f);
	UMaterialInterface* Fabric = Lit(bAfter ? FLinearColor(0.16f, 0.12f, 0.10f) : FLinearColor(0.20f, 0.06f, 0.05f), 1.f, 0.2f);
	UMaterialInterface* Dark = Lit(FLinearColor(0.012f, 0.012f, 0.014f), 0.3f, 0.f);
	UMaterialInterface* Ceramic = Lit(FLinearColor(0.75f, 0.73f, 0.68f), 0.25f, 0.05f);
	UMaterialInterface* Tea = Lit(FLinearColor(0.42f, 0.09f, 0.012f), 0.08f, 0.f);
	UMaterialInterface* Glass = Glow(FLinearColor(0.85f, 0.92f, 1.f), 0.2f, 0.16f);
	UMaterialInterface* Metal = Lit(FLinearColor(0.35f, 0.33f, 0.28f), 0.3f, 0.1f, 0.8f);
	UMaterialInterface* Paper = Lit(FLinearColor(0.78f, 0.75f, 0.66f), 0.95f, 0.1f);

	BuildShell(bAfter ? FLinearColor(0.40f, 0.37f, 0.32f) : FLinearColor(0.36f, 0.31f, 0.24f),
		bAfter ? FLinearColor(0.02f, 0.035f, 0.08f) : FLinearColor(0.008f, 0.01f, 0.02f), 1.f);
	BoxAt(-130.f, HY - 6.f, 40.f, 40.f, 4.f, 210.f, Fabric, false);
	BoxAt(90.f, HY - 6.f, 40.f, 40.f, 4.f, 210.f, Fabric, false);

	// carpet
	Box(FVector(0.f, 0.f, 0.5f), FVector(420.f, 300.f, 1.f), Lit(FLinearColor(0.22f, 0.03f, 0.025f), 1.f, 0.3f), false);
	Box(FVector(0.f, 0.f, 0.8f), FVector(390.f, 270.f, 1.f), Lit(FLinearColor(0.03f, 0.04f, 0.10f), 1.f, 0.3f), false);
	Box(FVector(0.f, 0.f, 1.1f), FVector(360.f, 240.f, 1.f), Lit(FLinearColor(0.30f, 0.04f, 0.03f), 1.f, 0.3f), false);
	Box(FVector(0.f, 0.f, 1.4f), FVector(80.f, 80.f, 1.f), Lit(FLinearColor(0.55f, 0.45f, 0.28f), 1.f, 0.3f), false, FRotator(0.f, 45.f, 0.f));

	// the sofa against -X, facing the television
	Box(FVector(-HX + 50.f, 0.f, 21.f), FVector(90.f, 250.f, 42.f), Fabric);
	Box(FVector(-HX + 10.f, 0.f, 62.f), FVector(20.f, 250.f, 80.f), Fabric);
	Box(FVector(-HX + 50.f, -132.f, 32.f), FVector(90.f, 16.f, 64.f), Fabric);
	Box(FVector(-HX + 50.f, 132.f, 32.f), FVector(90.f, 16.f, 64.f), Fabric);

	// a low table, two glasses of tea
	Box(FVector(-120.f, 0.f, 38.f), FVector(70.f, 120.f, 4.f), Wood);
	for (int32 Leg = 0; Leg < 4; ++Leg)
	{
		Box(FVector(-120.f + ((Leg & 1) ? 30.f : -30.f), (Leg & 2) ? 55.f : -55.f, 18.f), FVector(4.f, 4.f, 36.f), Wood, false);
	}
	Box(FVector(-120.f, 0.f, 41.f), FVector(40.f, 60.f, 2.f), Metal, false);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FVector At(-122.f, Index == 0 ? -14.f : 14.f, 42.f);
		Cyl(At + FVector(0.f, 0.f, 0.4f), 5.f, 0.8f, Ceramic);
		Cyl(At + FVector(0.f, 0.f, 4.5f), 2.7f, 7.5f, Tea);
		Cyl(At + FVector(0.f, 0.f, 4.8f), 3.f, 8.6f, Glass)->SetCastShadow(false);
		if (bAfter)
		{
			// hot, this time
			for (int32 Wisp = 0; Wisp < 3; ++Wisp)
			{
				UStaticMeshComponent* Sprite = Quad(At + FVector(0.f, 0.f, 12.f), FVector2D(8.f, 8.f), FRotator::ZeroRotator, Glow(FLinearColor(0.9f, 0.9f, 0.88f), CHLight::Emissive(0.12f), 0.1f, 1.f), true);
				Steam.Add(Sprite);
				SteamBase.Add(At + FVector(0.f, 0.f, 10.f));
				SteamAge.Add(Wisp * 0.9f);
			}
		}
	}

	// the television on its cabinet
	Box(FVector(HX - 25.f, 0.f, 28.f), FVector(45.f, 170.f, 56.f), Wood);
	Box(FVector(HX - 30.f, 0.f, 98.f), FVector(8.f, 132.f, 78.f), Dark);
	if (bYears)
	{
		TvScreen = WorldWidget(MakeNewsScreen(), FVector(HX - 35.f, 0.f, 98.f), FRotator(0.f, 180.f, 0.f), 0.2f, true, FVector2D(620.0, 360.0));
		if (UMaterialInstanceDynamic* Screen = TvScreen->GetMaterialInstance())
		{
			Screen->SetScalarParameterValue(TEXT("EmissiveStrength"), CHLight::Emissive(0.9f));
		}
		PointLight(FVector(HX - 80.f, 0.f, 100.f), FLinearColor(0.75f, 0.82f, 1.f), 7.f, 800.f, false, 0.3f);
	}
	if (bAfter)
	{
		// his photograph with a black ribbon across its corner, a flower
		Box(FVector(HX - 25.f, 70.f, 74.f), FVector(4.f, 26.f, 34.f), Wood, false);
		Box(FVector(HX - 27.5f, 70.f, 74.f), FVector(1.f, 22.f, 30.f), Lit(FLinearColor(0.05f, 0.05f, 0.055f), 0.6f, 0.f), false);
		Box(FVector(HX - 28.5f, 78.f, 84.f), FVector(1.f, 4.f, 18.f), Dark, false, FRotator(0.f, 0.f, 45.f));
		Cyl(FVector(HX - 25.f, 40.f, 64.f), 3.f, 14.f, Glass);
		Ball(FVector(HX - 25.f, 40.f, 79.f), FVector(7.f, 7.f, 6.f), Lit(FLinearColor(0.5f, 0.02f, 0.03f), 0.8f, 0.f));
	}

	// a floor lamp
	Cyl(FVector(-HX + 40.f, HY - 40.f, 80.f), 2.f, 160.f, Metal);
	Cone(FVector(-HX + 40.f, HY - 40.f, 168.f), 22.f, 30.f, Emissive(FLinearColor(1.f, 0.62f, 0.3f), CHLight::Emissive(1.5f), FLinearColor(0.2f, 0.12f, 0.06f)), false, FRotator(180.f, 0.f, 0.f));
	PointLight(FVector(-HX + 40.f, HY - 40.f, 150.f), FLinearColor(1.f, 0.62f, 0.32f), 6.f, 700.f, true, 0.5f);

	if (!bAfter)
	{
		// ---- the table: dinner laid, nobody eating
		const FVector Table(150.f, -120.f, 0.f);
		Box(Table + FVector(0.f, 0.f, 73.f), FVector(160.f, 95.f, 4.f), Wood);
		for (int32 Leg = 0; Leg < 4; ++Leg)
		{
			Box(Table + FVector((Leg & 1) ? 72.f : -72.f, (Leg & 2) ? 40.f : -40.f, 36.f), FVector(5.f, 5.f, 71.f), Wood, false);
		}
		Box(Table + FVector(0.f, 0.f, 75.3f), FVector(150.f, 85.f, 0.6f), Lit(FLinearColor(0.55f, 0.50f, 0.42f), 1.f, 0.1f), false);
		for (int32 Plate = 0; Plate < 4; ++Plate)
		{
			const FVector At = Table + FVector((Plate & 1) ? 48.f : -48.f, (Plate & 2) ? 26.f : -26.f, 76.f);
			Cyl(At, 11.f, 1.2f, Ceramic);
			Box(At + FVector(15.f, 0.f, 0.2f), FVector(1.2f, 12.f, 0.4f), Metal, false);
		}
		Ball(Table + FVector(0.f, 0.f, 80.f), FVector(30.f, 22.f, 9.f), Lit(FLinearColor(0.80f, 0.76f, 0.60f), 0.9f, 0.f));
		Cyl(Table + FVector(0.f, 0.f, 76.6f), 17.f, 1.6f, Ceramic);
		Cyl(Table + FVector(-28.f, -3.f, 82.f), 9.f, 12.f, Metal);
		Box(Table + FVector(30.f, 4.f, 76.6f), FVector(22.f, 30.f, 1.2f), Lit(FLinearColor(0.62f, 0.48f, 0.28f), 1.f, 0.1f), false, FRotator(0.f, 20.f, 0.f));
		for (int32 Chair = 0; Chair < 2; ++Chair)
		{
			const FVector At = Table + FVector(Chair == 0 ? -40.f : 40.f, 78.f, 0.f);
			Box(At + FVector(0.f, 0.f, 44.f), FVector(40.f, 40.f, 4.f), Wood);
			Box(At + FVector(0.f, 19.f, 70.f), FVector(40.f, 4.f, 50.f), Wood, false);
		}
		// the brother's page of phone numbers
		Box(Table + FVector(-44.f, 30.f, 77.5f), FVector(15.f, 21.f, 0.3f), Paper, false, FRotator(0.f, 8.f, 0.f));

		// a ceiling light over the room
		Ball(FVector(0.f, 0.f, H - 22.f), FVector(30.f, 30.f, 18.f), Emissive(FLinearColor(1.f, 0.78f, 0.5f), CHLight::Emissive(1.2f), FLinearColor(0.3f, 0.25f, 0.18f)));
		PointLight(FVector(0.f, 0.f, H - 50.f), FLinearColor(1.f, 0.76f, 0.5f), 7.f, 900.f, true, 0.4f);

		// the balcony beyond the window, where his father smokes
		BoxAt(-140.f, HY + T, -20.f, 280.f, 130.f, 20.f, Lit(FLinearColor(0.2f, 0.2f, 0.2f), 0.9f, 0.4f));
		BoxAt(-140.f, HY + T + 124.f, 0.f, 280.f, 6.f, 95.f, Metal, false);
		Box(FVector(26.f, HY + 118.f, 138.f), FVector(1.2f, 1.2f, 1.2f), Emissive(FLinearColor(1.f, 0.35f, 0.08f), CHLight::Emissive(6.f)), false);

		// ---- what they will take to the court tomorrow
		auto Item = [this](const TCHAR* Id, UStaticMeshComponent* Prop, const TCHAR* Fa, const TCHAR* En)
		{
			Items.Add(Id, Prop);
			SpawnInteractable(Id, Fa, En, Prop->GetComponentLocation() + FVector(0.f, 0.f, 12.f), 150.f);
			++ItemsLeft;
		};
		Item(TEXT("take.id"), Box(FVector(HX - 30.f, -62.f, 57.f), FVector(8.6f, 5.4f, 0.4f), Lit(FLinearColor(0.60f, 0.66f, 0.70f), 0.5f, 0.f), false, FRotator(0.f, 15.f, 0.f), true),
			TEXT("کارت ملی‌اش را بردار"), TEXT("Take her ID card"));
		Item(TEXT("take.manteau"), Box(FVector(-HX + 50.f, 132.f, 66.f), FVector(70.f, 20.f, 5.f), Lit(FLinearColor(0.03f, 0.03f, 0.035f), 1.f, 0.1f), false, FRotator(0.f, 3.f, 0.f), true),
			TEXT("مانتوی بلند را بردار"), TEXT("Take the long manteau"));
		Item(TEXT("take.deed"), Box(FVector(-118.f, -40.f, 40.5f), FVector(21.f, 29.f, 0.4f), Paper, false, FRotator(0.f, -12.f, 0.f), true),
			TEXT("سند خانه را بردار"), TEXT("Take the house deed"));
	}

	FCHEnv Env;
	Env.bAtmosphere = false;
	Env.SunIntensity = 0.f;
	Env.bSunLightsWorld = false;
	Env.SkyLightIntensity = 0.2f;
	Env.bFog = false;
	Env.ExposureEV = EV;
	Env.Grain = 0.25f;
	Env.Vignette = 0.5f;
	Env.Fringe = 0.2f;
	Env.Bloom = 0.6f;
	Env.Saturation = 0.9f;
	Env.Temperature = 6800.f;
	ApplyEnv(Env);

	// people
	Mark(TEXT("PlayerStart"), FVector(-60.f, -HY + 70.f, 0.f), FRotator(0.f, 60.f, 0.f));
	Mark(TEXT("SofaLeft"), FVector(-HX + 60.f, -45.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("SofaRight"), FVector(-HX + 60.f, 50.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("SofaClose"), FVector(-HX + 60.f, 20.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("TableSeat"), FVector(110.f, -42.f, 0.f), FRotator(0.f, -90.f, 0.f));
	Mark(TEXT("TableRight"), FVector(190.f, -42.f, 0.f), FRotator(0.f, -90.f, 0.f));
	Mark(TEXT("CenterLeft"), FVector(-20.f, -35.f, 0.f), FRotator(0.f, 90.f, 0.f));
	Mark(TEXT("CenterRight"), FVector(-20.f, 45.f, 0.f), FRotator(0.f, -90.f, 0.f));
	Mark(TEXT("DoorIn"), FVector(220.f, -HY + 55.f, 0.f), FRotator(0.f, 135.f, 0.f));
	Mark(TEXT("Balcony"), FVector(0.f, HY + 95.f, 0.f), FRotator(0.f, 90.f, 0.f));
	MarkLookAt(TEXT("Wide"), FVector(HX - 60.f, -HY + 50.f, 190.f), FVector(-HX + 60.f, 20.f, 70.f), 62.f);
	MarkLookAt(TEXT("TV"), FVector(-HX + 15.f, -10.f, 150.f), FVector(HX - 30.f, 0.f, 98.f), 42.f);
	MarkLookAt(TEXT("Faces"), FVector(HX - 90.f, 30.f, 115.f), FVector(-HX + 60.f, 0.f, 95.f), 38.f);
	MarkLookAt(TEXT("CenterCam"), FVector(160.f, -15.f, 145.f), FVector(-20.f, 5.f, 105.f), 45.f);
	MarkLookAt(TEXT("PhotoCam"), FVector(HX - 110.f, 55.f, 92.f), FVector(HX - 26.f, 62.f, 75.f), 34.f);
	MarkLookAt(TEXT("Tea"), FVector(-60.f, -60.f, 75.f), FVector(-122.f, 0.f, 46.f), 30.f);
	MarkLookAt(TEXT("Table"), FVector(-140.f, 60.f, 165.f), FVector(150.f, -115.f, 80.f), 50.f);
	MarkLookAt(TEXT("Dinner"), FVector(60.f, -HY + 30.f, 150.f), FVector(150.f, -100.f, 78.f), 44.f);
	MarkLookAt(TEXT("Window"), FVector(-60.f, -40.f, 150.f), FVector(10.f, HY + 100.f, 140.f), 40.f);
}

TSharedRef<SWidget> ACHLoc_Home::MakeNewsScreen() const
{
	// a black-bordered news screen; no name, no face, no portrait
	const bool bFa = CHSettings::IsPersian();
	const FString NewsLine = bFa ? TEXT("رهبر درگذشت.") : CHSettings::Latin(TEXT("The Leader has died."), TEXT("Der Führer ist gestorben."));
	return SNew(SBox)
		.WidthOverride(620.f)
		.HeightOverride(360.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 1.f)))
			]
			+ SOverlay::Slot()
			.Padding(30.f)
			[
				SNew(SBorder)
				.BorderImage(CH::WhiteBrush())
				.BorderBackgroundColor(FSlateColor(FLinearColor(0.10f, 0.11f, 0.13f, 1.f)))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(NewsLine))
					.Font(bFa ? CH::FarsiFont(58.f, CH::EWeight::Bold) : CH::LatinFont(34.f, CH::EWeight::Bold))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.93f, 1.f)))
					.TextShapingMethod(bFa ? ETextShapingMethod::FullShaping : ETextShapingMethod::Auto)
					.TextFlowDirection(bFa ? ETextFlowDirection::RightToLeft : ETextFlowDirection::LeftToRight)
				]
			]
		];
}

// ---------------------------------------------------------------------------------------------
// The lawyer's office
// ---------------------------------------------------------------------------------------------

void ACHLoc_Home::BuildOffice()
{
	const float EV = 0.f;
	CHLight::SetExposure(EV);
	FRandomStream Rng(1601);

	UMaterialInterface* Wood = Lit(FLinearColor(0.13f, 0.07f, 0.035f), 0.45f, 0.3f);
	UMaterialInterface* Leather = Lit(FLinearColor(0.05f, 0.035f, 0.03f), 0.5f, 0.2f);
	UMaterialInterface* Metal = Lit(FLinearColor(0.30f, 0.30f, 0.31f), 0.4f, 0.2f, 0.7f);
	UMaterialInterface* Paper = Lit(FLinearColor(0.78f, 0.75f, 0.66f), 0.95f, 0.1f);

	BuildShell(FLinearColor(0.42f, 0.41f, 0.36f), FLinearColor(0.55f, 0.6f, 0.66f), CHLight::Emissive(1.1f));
	Box(FVector(0.f, 0.f, 0.5f), FVector(2.f * HX, 2.f * HY, 1.f), Lit(FLinearColor(0.17f, 0.18f, 0.17f), 0.9f, 0.4f), false);
	// blinds: bars of light across the window
	for (float Z = 96.f; Z < 230.f; Z += 11.f)
	{
		BoxInstance(FVector(0.f, HY - 2.f, Z), FVector(180.f, 3.f, 5.f), Metal, FRotator(0.f, 0.f, 28.f));
	}

	// the desk between them
	const FVector Desk(60.f, 0.f, 0.f);
	Box(Desk + FVector(0.f, 0.f, 74.f), FVector(85.f, 190.f, 5.f), Wood);
	Box(Desk + FVector(30.f, 0.f, 36.f), FVector(4.f, 180.f, 72.f), Wood);
	Box(Desk + FVector(-5.f, -90.f, 36.f), FVector(70.f, 4.f, 72.f), Wood);
	Box(Desk + FVector(-5.f, 90.f, 36.f), FVector(70.f, 4.f, 72.f), Wood);
	// files, a lamp, a telephone, a glass of tea gone cold
	for (int32 Index = 0; Index < 7; ++Index)
	{
		Box(Desk + FVector(14.f, 58.f, 78.f + Index * 2.6f), FVector(24.f, 32.f, 2.4f), Index % 2 ? Paper : Lit(FLinearColor(0.35f, 0.30f, 0.18f), 0.9f, 0.2f), false, FRotator(0.f, Rng.FRandRange(-9.f, 9.f), 0.f));
	}
	Box(Desk + FVector(16.f, -62.f, 79.f), FVector(18.f, 22.f, 6.f), Leather, false, FRotator(0.f, -12.f, 0.f));
	Cyl(Desk + FVector(24.f, -78.f, 78.f), 6.f, 2.f, Metal);
	Cyl(Desk + FVector(24.f, -78.f, 95.f), 1.f, 34.f, Metal);
	Cone(Desk + FVector(18.f, -78.f, 114.f), 9.f, 12.f, Lit(FLinearColor(0.05f, 0.12f, 0.07f), 0.4f, 0.1f), false, FRotator(-30.f, 0.f, 0.f));
	PointLight(Desk + FVector(8.f, -78.f, 100.f), FLinearColor(1.f, 0.72f, 0.45f), 1.5f, 380.f, true, 0.4f);

	// the house deed: it will cross the desk
	DeedFrom = Desk + FVector(-34.f, -22.f, 77.f);
	DeedTo = Desk + FVector(6.f, -8.f, 77.f);
	Deed = Box(DeedFrom, FVector(21.f, 29.f, 0.4f), Paper, false, FRotator(0.f, -6.f, 0.f), true);
	Deed->SetVisibility(false);

	// chairs: his behind the desk, two in front
	Box(Desk + FVector(88.f, 0.f, 46.f), FVector(50.f, 54.f, 8.f), Leather);
	Box(Desk + FVector(112.f, 0.f, 90.f), FVector(8.f, 54.f, 90.f), Leather, false);
	for (int32 Chair = 0; Chair < 2; ++Chair)
	{
		const FVector At = Desk + FVector(-95.f, Chair == 0 ? -42.f : 42.f, 0.f);
		Box(At + FVector(0.f, 0.f, 44.f), FVector(44.f, 44.f, 5.f), Leather);
		Box(At + FVector(-21.f, 0.f, 72.f), FVector(5.f, 44.f, 52.f), Leather, false);
	}

	// shelves of files behind him, floor to ceiling
	for (int32 Level = 0; Level < 6; ++Level)
	{
		const float Z = 6.f + Level * 42.f;
		Box(FVector(HX - 16.f, 0.f, Z), FVector(30.f, 420.f, 3.f), Wood, false);
		float Y = -205.f;
		while (Y < 200.f)
		{
			const float Width = Rng.FRandRange(4.f, 8.f);
			if (Rng.FRand() < 0.9f)
			{
				const float Tone = Rng.FRandRange(0.5f, 1.f);
				BoxInstance(FVector(HX - 16.f, Y + Width * 0.5f, Z + 2.f + 16.f), FVector(24.f, Width, Rng.FRandRange(28.f, 33.f)),
					Lit(Rng.FRand() < 0.5f ? FLinearColor(0.10f * Tone, 0.14f * Tone, 0.24f * Tone) : FLinearColor(0.30f * Tone, 0.28f * Tone, 0.22f * Tone), 0.8f, 0.2f));
			}
			Y += Width + 0.4f;
		}
	}
	Box(FVector(HX - 16.f, -212.f, 130.f), FVector(30.f, 4.f, 260.f), Wood, false);
	Box(FVector(HX - 16.f, 212.f, 130.f), FVector(30.f, 4.f, 260.f), Wood, false);

	// daylight through the blinds, a tube on the ceiling
	PointLight(FVector(0.f, HY - 50.f, 170.f), FLinearColor(0.80f, 0.88f, 1.f), 16.f, 900.f, true, 0.6f);
	Box(FVector(-40.f, -40.f, H - 4.f), FVector(120.f, 14.f, 6.f), Emissive(FLinearColor(0.9f, 0.95f, 1.f), CHLight::Emissive(1.2f), FLinearColor(0.3f, 0.3f, 0.3f)), false);
	PointLight(FVector(-40.f, -40.f, H - 40.f), FLinearColor(0.88f, 0.93f, 1.f), 9.f, 900.f, false, 0.2f);

	FCHEnv Env;
	Env.bAtmosphere = false;
	Env.SunIntensity = 0.f;
	Env.bSunLightsWorld = false;
	Env.SkyLightIntensity = 0.2f;
	Env.bFog = false;
	Env.ExposureEV = EV;
	Env.Grain = 0.25f;
	Env.Vignette = 0.45f;
	Env.Fringe = 0.2f;
	Env.Bloom = 0.5f;
	Env.Saturation = 0.78f;
	Env.Temperature = 6200.f;
	ApplyEnv(Env);

	Mark(TEXT("PlayerStart"), FVector(-250.f, -150.f, 0.f), FRotator(0.f, 20.f, 0.f));
	Mark(TEXT("LawyerSeat"), Desk + FVector(88.f, 0.f, 0.f), FRotator(0.f, 180.f, 0.f));
	Mark(TEXT("FatherSeat"), Desk + FVector(-95.f, -42.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("HeSeat"), Desk + FVector(-95.f, 42.f, 0.f), FRotator(0.f, 0.f, 0.f));
	MarkLookAt(TEXT("Office"), FVector(-300.f, -210.f, 165.f), Desk + FVector(20.f, 0.f, 95.f), 58.f);
	MarkLookAt(TEXT("Lawyer"), Desk + FVector(-150.f, 110.f, 135.f), Desk + FVector(88.f, 0.f, 105.f), 40.f);
	MarkLookAt(TEXT("Father"), Desk + FVector(70.f, 95.f, 135.f), Desk + FVector(-95.f, -42.f, 100.f), 40.f);
	MarkLookAt(TEXT("Deed"), Desk + FVector(-60.f, 50.f, 150.f), Desk + FVector(-10.f, -14.f, 77.f), 34.f);
}

// ---------------------------------------------------------------------------------------------
// The stairwell
// ---------------------------------------------------------------------------------------------

void ACHLoc_Home::BuildStairs()
{
	const float EV = -1.f;
	CHLight::SetExposure(EV);

	UMaterialInterface* Wall = Lit(FLinearColor(0.30f, 0.29f, 0.25f), 0.95f, 0.5f);
	UMaterialInterface* Stone = Lit(FLinearColor(0.22f, 0.21f, 0.20f), 0.7f, 0.5f);
	UMaterialInterface* Rail = Lit(FLinearColor(0.04f, 0.04f, 0.045f), 0.5f, 0.3f, 0.6f);

	const float HalfWidth = 130.f;
	const int32 Steps = 11;
	const float Depth = 28.f;
	const float Rise = 17.f;
	const float TopX = Steps * Depth;
	const float TopZ = Steps * Rise;

	// the hall below, the flight, the landing above
	BoxAt(-420.f, -HalfWidth, -20.f, 420.f, 2.f * HalfWidth, 20.f, Stone);
	for (int32 Index = 0; Index < Steps; ++Index)
	{
		BoxAt(Index * Depth, -HalfWidth, 0.f, Depth, 2.f * HalfWidth, Rise * (Index + 1), Stone);
	}
	BoxAt(TopX, -HalfWidth, 0.f, 260.f, 2.f * HalfWidth, TopZ, Stone);
	// walls and the ceiling far above
	BoxAt(-440.f, -HalfWidth - 20.f, -20.f, TopX + 720.f, 20.f, 620.f, Wall);
	BoxAt(-440.f, HalfWidth, -20.f, TopX + 720.f, 20.f, 620.f, Wall);
	BoxAt(TopX + 260.f, -HalfWidth, 0.f, 20.f, 2.f * HalfWidth, 620.f, Wall);
	BoxAt(-440.f, -HalfWidth, 0.f, 20.f, 2.f * HalfWidth, 620.f, Wall);
	BoxAt(-440.f, -HalfWidth, 600.f, TopX + 720.f, 2.f * HalfWidth, 20.f, Wall);
	// a handrail down one side
	{
		const FVector From(0.f, HalfWidth - 8.f, 95.f);
		const FVector To(TopX, HalfWidth - 8.f, TopZ + 95.f);
		Box((From + To) * 0.5f, FVector((To - From).Size(), 4.f, 4.f), Rail, false, (To - From).Rotation());
	}
	// the door of their flat on the landing, a line of light under it
	Box(FVector(TopX + 259.f, 0.f, TopZ + 105.f), FVector(3.f, 95.f, 210.f), Lit(FLinearColor(0.10f, 0.06f, 0.035f), 0.5f, 0.3f), false);
	Box(FVector(TopX + 257.f, 0.f, TopZ + 1.5f), FVector(2.f, 90.f, 2.f), Emissive(FLinearColor(1.f, 0.7f, 0.4f), CHLight::Emissive(2.f)), false);

	// one bulb, high up
	Ball(FVector(TopX * 0.6f, 0.f, 520.f), FVector(9.f, 9.f, 11.f), Emissive(FLinearColor(1.f, 0.75f, 0.45f), CHLight::Emissive(4.f)));
	PointLight(FVector(TopX * 0.6f, 0.f, 500.f), FLinearColor(1.f, 0.72f, 0.42f), 9.f, 1300.f, true, 0.5f);
	PointLight(FVector(TopX + 200.f, 0.f, TopZ + 30.f), FLinearColor(1.f, 0.7f, 0.4f), 0.8f, 300.f, false, 0.f);

	FCHEnv Env;
	Env.bAtmosphere = false;
	Env.SunIntensity = 0.f;
	Env.bSunLightsWorld = false;
	Env.SkyLightIntensity = 0.15f;
	Env.bFog = false;
	Env.ExposureEV = EV;
	Env.Grain = 0.3f;
	Env.Vignette = 0.55f;
	Env.Fringe = 0.25f;
	Env.Bloom = 0.6f;
	Env.Saturation = 0.8f;
	Env.Temperature = 6500.f;
	ApplyEnv(Env);

	// he sits at the top of the flight, his legs down the steps, facing away
	const FVector SeatAt(TopX + 22.f, 0.f, TopZ);
	Mark(TEXT("PlayerStart"), FVector(-300.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("NeighbourStep"), FVector(4.f * Depth, -45.f, 4.f * Rise), FRotator(0.f, 180.f, 0.f));
	Mark(TEXT("HeMidStep"), FVector(2.f * Depth, 35.f, 2.f * Rise), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("BrotherStep"), SeatAt + FVector(0.f, -42.f, 0.f), FRotator(0.f, 180.f, 0.f));
	Mark(TEXT("HeStep"), SeatAt + FVector(0.f, 44.f, 0.f), FRotator(0.f, 180.f, 0.f));
	MarkLookAt(TEXT("StairTalk"), FVector(7.f * Depth, 85.f, 7.f * Rise + 125.f), FVector(3.f * Depth, -5.f, 3.f * Rise + 85.f), 46.f);
	MarkLookAt(TEXT("Above"), FVector(TopX + 235.f, 95.f, TopZ + 175.f), SeatAt + FVector(0.f, -20.f, 40.f), 50.f);
	MarkLookAt(TEXT("Behind"), FVector(TopX + 225.f, -60.f, TopZ + 105.f), SeatAt + FVector(0.f, 5.f, 42.f), 50.f);
	MarkLookAt(TEXT("Below"), FVector(-330.f, -70.f, 120.f), SeatAt + FVector(0.f, 0.f, 70.f), 46.f);
}

// ---------------------------------------------------------------------------------------------

void ACHLoc_Home::OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat)
{
	if (Event == TEXT("she_cries"))
	{
		if (ACHNpc* Sister = FindNpc(TEXT("sister")))
		{
			Sister->SetShaking(true);
		}
	}
	else if (Event == TEXT("arm_around"))
	{
		// he moves closer to her on the sofa
		if (ACHNpc* Brother = FindNpc(TEXT("brother")))
		{
			Brother->SetActorLocation(Brother->GetActorLocation() + FVector(0.f, 32.f, 0.f));
			Brother->FaceYaw(18.f);
		}
	}
	else if (Event == TEXT("deed"))
	{
		// his father slides the deed across the desk
		if (Deed)
		{
			Deed->SetVisibility(true);
			DeedTime = 0.f;
		}
	}
}

void ACHLoc_Home::OnInteract(const FString& Id, ACHInteractable* What)
{
	if (What)
	{
		What->SetEnabled(false);
	}
	if (!Id.StartsWith(TEXT("take.")))
	{
		return;
	}
	if (TObjectPtr<UStaticMeshComponent>* Prop = Items.Find(Id))
	{
		(*Prop)->SetVisibility(false);
	}
	if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
	{
		if (Id == TEXT("take.id"))
		{
			Director->Toast(TEXT("از این عکس متنفر بود."), TEXT("She hated this photo."), CH::Col::Paper, 3.5f);
		}
		else if (Id == TEXT("take.manteau"))
		{
			Director->Toast(TEXT("هیچ‌وقت نپوشیدش."), TEXT("She never wore it."), CH::Col::Paper, 3.5f);
		}
		else if (Id == TEXT("take.deed"))
		{
			Director->Toast(TEXT("همه‌ی دارایی‌شان. یک برگ کاغذ."), TEXT("Everything they own. One page."), CH::Col::Paper, 3.5f);
		}
	}
	if (--ItemsLeft <= 0)
	{
		FinishDelay = 3.2f;
	}
}

void ACHLoc_Home::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	HomeTime += DeltaSeconds;

	// the last line stays up a moment before the scene moves on
	if (FinishDelay > 0.f)
	{
		FinishDelay -= DeltaSeconds;
		if (FinishDelay <= 0.f)
		{
			FinishSegment();
		}
	}

	if (Deed && DeedTime >= 0.f && DeedTime < 2.6f)
	{
		DeedTime = FMath::Min(2.6f, DeedTime + DeltaSeconds);
		Deed->SetWorldLocation(FMath::Lerp(DeedFrom, DeedTo, FMath::SmoothStep(0.f, 1.f, DeedTime / 2.6f)));
	}

	APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	for (int32 Index = 0; Index < Steam.Num(); ++Index)
	{
		float& Age = SteamAge[Index];
		Age += DeltaSeconds;
		const float Life = 2.7f;
		if (Age > Life)
		{
			Age = 0.f;
		}
		const float Phase = Age / Life;
		const FVector At = SteamBase[Index] + FVector(FMath::Sin(HomeTime * 1.3f + Index) * 2.f * Phase, 0.f, 22.f * Phase);
		Steam[Index]->SetWorldLocation(At);
		const float Size = (0.06f + 0.12f * Phase);
		Steam[Index]->SetWorldScale3D(FVector(Size, Size, 1.f));
		if (Camera)
		{
			Steam[Index]->SetWorldRotation(FRotationMatrix::MakeFromZ((Camera->GetCameraLocation() - At).GetSafeNormal()).Rotator());
		}
		Steam[Index]->SetVisibility(Phase < 0.95f);
	}
}
