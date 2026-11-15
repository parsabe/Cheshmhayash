// Cheshm Hayash - his room. Desk under the window, cold glass of tea, the bed with the German
// grammar book, the wall calendar, a setar in the corner.

#include "World/CHLocations.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Game/CHInteractable.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Math/RandomStream.h"
#include "UI/CHStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	// Inner half extents of the room and wall thickness (cm).
	const float HX = 230.f;
	const float HY = 180.f;
	const float H = 265.f;
	const float T = 20.f;

	// after: the calendar he took off the wall lies on the floor; the letter falls from the book
	const FVector CalendarFloor(-30.f, 104.f, 0.f);
	const float CalendarYaw = 14.f;
	const FVector BookSpot(-70.f, -HY + 62.f, 48.2f);
	const FVector LetterSpot(-46.f, -58.f, 2.3f);

	TSharedRef<SWidget> CalendarText(const FString& Text, float Size, const FLinearColor& Color, bool bBold = false)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Text))
			.Font(CH::FarsiFont(Size, bBold ? CH::EWeight::Bold : CH::EWeight::Regular))
			.ColorAndOpacity(FSlateColor(Color))
			.Justification(ETextJustify::Center)
			.TextShapingMethod(ETextShapingMethod::FullShaping)
			.TextFlowDirection(ETextFlowDirection::RightToLeft);
	}

	/** The wall calendar page for Mehr 1401, with the 26th (the visa appointment) circled in red. */
	TSharedRef<SWidget> MakeCalendarWidget()
	{
		const FLinearColor Ink(0.035f, 0.035f, 0.04f, 1.f);
		const FLinearColor Red(0.50f, 0.03f, 0.025f, 1.f);

		TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel)
			.MinDesiredSlotWidth(60.f)
			.MinDesiredSlotHeight(62.f);

		// the Persian week starts on Saturday and reads right to left
		const TCHAR* Weekdays[] = { TEXT("ش"), TEXT("ی"), TEXT("د"), TEXT("س"), TEXT("چ"), TEXT("پ"), TEXT("ج") };
		for (int32 Weekday = 0; Weekday < 7; ++Weekday)
		{
			Grid->AddSlot(6 - Weekday, 0)
				[
					SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						CalendarText(Weekdays[Weekday], 23.f, Weekday == 6 ? Red : FLinearColor(0.25f, 0.25f, 0.27f, 1.f))
					]
				];
		}

		// 1 Mehr 1401 fell on a Friday (the 7th column)
		for (int32 Day = 1; Day <= 30; ++Day)
		{
			const int32 Cell = Day - 1 + 6;
			const int32 Weekday = Cell % 7;
			const int32 Row = Cell / 7 + 1;

			TSharedRef<SWidget> Number = CalendarText(CH::PersianDigits(FString::FromInt(Day)), 30.f, Weekday == 6 ? Red : Ink);
			TSharedPtr<SWidget> CellContent = Number;
			if (Day == 26)
			{
				CellContent = SNew(SOverlay)
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(64.f).HeightOverride(58.f)
						[
							SNew(SImage).Image(CH::RingBrush(29.f, 4.5f, FLinearColor(0.62f, 0.03f, 0.02f, 1.f)))
						]
					]
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						Number
					];
			}
			Grid->AddSlot(6 - Weekday, Row)
				[
					SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						CellContent.ToSharedRef()
					]
				];
		}

		return SNew(SBox)
			.WidthOverride(460.f)
			.HeightOverride(620.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f)
				[
					CalendarText(TEXT("مهر ۱۴۰۱"), 56.f, Red, true)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(30.f, 2.f, 30.f, 8.f)
				[
					SNew(SBox).HeightOverride(3.f)
					[
						SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(FSlateColor(Ink))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					Grid
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
				[
					CalendarText(TEXT("۲۶ مهر · وقت سفارت"), 22.f, Red)
				]
			];
	}
}

void ACHLoc_Room::Build(const FString& InVariant)
{
	Super::Build(InVariant);

	const bool bNight = Variant == TEXT("night");
	const bool bAfter = Variant == TEXT("after");

	// a small room: seen through his own eyes (and, after, through hers)
	PlayerBoomLength = 0.f;
	// glows below are set relative to this room's exposure
	CHLight::SetExposure(bNight ? -0.5f : (bAfter ? 0.3f : 0.1f));

	BuildShell();
	BuildFurniture();

	FCHEnv Env;
	if (bNight)
	{
		Env.SunRotation = FRotator(9.f, 200.f, 0.f);
		Env.SunIntensity = 2.f;
		Env.bSunLightsWorld = false;
		Env.SkyLightIntensity = 0.6f;
		Env.ExposureEV = -0.5f;
		Env.Temperature = 5600.f;
	}
	else if (bAfter)
	{
		// a pale, cold morning; nothing in the room has moved
		Env.SunRotation = FRotator(-33.f, 190.f, 0.f);
		Env.SunIntensity = 4.5f;
		Env.SunColor = FLinearColor(0.92f, 0.95f, 1.f);
		Env.SunVolumetric = 1.2f;
		Env.SkyLightIntensity = 1.9f;
		Env.ExposureEV = 0.3f;
		Env.Temperature = 5900.f;
		PointLight(FVector(HX - 48.f, 0.f, 150.f), FLinearColor(0.80f, 0.87f, 1.f), 3.4f, 700.f, true, 0.3f);
		PointLight(FVector(-60.f, 0.f, 200.f), FLinearColor(0.85f, 0.88f, 0.95f), 1.2f, 600.f, false, 0.f);
	}
	else
	{
		// low warm morning sun coming in through the window
		Env.SunRotation = FRotator(-21.f, 197.f, 0.f);
		Env.SunIntensity = 12.f;
		Env.SunColor = FLinearColor(1.f, 0.80f, 0.58f);
		Env.SunVolumetric = 2.2f;
		Env.SkyLightIntensity = 1.6f;
		Env.ExposureEV = 0.1f;
		Env.Temperature = 6100.f;

		// daylight spilling in from the window, and the warm bounce off the sunlit bed and floor
		PointLight(FVector(HX - 48.f, 0.f, 150.f), FLinearColor(0.72f, 0.84f, 1.f), 3.0f, 650.f, true, 0.3f);
		PointLight(FVector(-40.f, -60.f, 70.f), FLinearColor(1.f, 0.74f, 0.48f), 2.6f, 520.f, false, 0.2f);
	}
	Env.bFog = true;
	Env.FogDensity = 0.012f;
	Env.FogHeightFalloff = 0.02f;
	Env.FogColor = FLinearColor(0.05f, 0.04f, 0.03f);
	Env.bVolumetricFog = true;
	Env.VolumetricExtinction = 0.8f;
	Env.Grain = 0.28f;
	Env.Vignette = 0.5f;
	Env.Fringe = 0.25f;
	Env.Bloom = 0.6f;
	Env.Saturation = 0.92f;
	Env.Contrast = 1.04f;
	if (bAfter)
	{
		Env.FogColor = FLinearColor(0.05f, 0.055f, 0.06f);
		Env.Grain = 0.22f;
		Env.Fringe = 0.15f;
		Env.Saturation = 0.66f;
		Env.Contrast = 0.97f;
	}
	ApplyEnv(Env);
	if (bAfter && Sun)
	{
		Sun->SetLightSourceAngle(5.f);
	}

	// ---- the calendar page: Mehr 1401, the 26th circled in red
	if (bAfter)
	{
		// face up on the floor, where it fell when he took it down
		const FVector PageUp = FRotator(0.f, 90.f + CalendarYaw, 0.f).Vector();
		WorldWidget(MakeCalendarWidget(), CalendarFloor + FVector(0.f, 0.f, 2.5f), FRotationMatrix::MakeFromXZ(FVector::UpVector, PageUp).Rotator(), 0.1f, false, FVector2D(460.0, 620.0));
		SpawnInteractable(TEXT("book"), TEXT("کتابش را باز کن"), TEXT("Open his book"), BookSpot + FVector(0.f, 0.f, 22.f), 165.f);
	}
	else
	{
		CalendarSheet = WorldWidget(MakeCalendarWidget(), FVector(-15.f, HY - 1.75f, 150.f), FRotator(0.f, -90.f, 0.f), 0.1f, false, FVector2D(460.0, 620.0));
	}

	if (bAfter)
	{
		// she came in; the door stays open behind her
		DoorAngle = DoorTarget = 82.f;
		DoorPivot->SetRelativeRotation(FRotator(0.f, DoorAngle, 0.f));
	}

	// ---- marks (people stand on the floor)
	Mark(TEXT("PlayerStart"), bAfter ? FVector(-HX + 55.f, 95.f, 0.f) : FVector(-70.f, 45.f, 0.f), FRotator(0.f, bAfter ? -15.f : 0.f, 0.f));
	MarkLookAt(TEXT("FloorCalendar"), CalendarFloor + FVector(-6.f, -64.f, 112.f), CalendarFloor + FVector(0.f, 6.f, 2.f), 46.f);
	MarkLookAt(TEXT("Letter"), LetterSpot + FVector(-22.f, 44.f, 92.f), LetterSpot, 40.f);
	// the night outside (1.7); his shadow on the wall where the calendar hung (1.9)
	MarkLookAt(TEXT("Window"), FVector(70.f, 10.f, 150.f), FVector(HX + 600.f, -40.f, 185.f), 52.f);
	MarkLookAt(TEXT("Shadow"), FVector(-40.f, 100.f, 150.f), FVector(HX - 40.f, 25.f, 118.f), 48.f);
	MarkLookAt(TEXT("BedBook"), BookSpot + FVector(10.f, 95.f, 75.f), BookSpot, 40.f);
	Mark(TEXT("He"), FVector(HX - 96.f, 58.f, 0.f));
	Mark(TEXT("ChairSeat"), FVector(HX - 118.f, -4.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("SisterHall"), FVector(-HX - T - 120.f, 105.f, 0.f));
	Mark(TEXT("SisterIn"), FVector(-HX + 100.f, 98.f, 0.f));

	MarkLookAt(TEXT("Wide"), FVector(-205.f, -140.f, 178.f), FVector(150.f, 40.f, 95.f), 78.f);
	MarkLookAt(TEXT("Desk"), FVector(40.f, -70.f, 150.f), FVector(HX - 40.f, 10.f, 88.f), 55.f);
	MarkLookAt(TEXT("Tea"), FVector(HX - 112.f, 86.f, 104.f), FVector(HX - 46.f, 40.f, 81.f), 34.f);
	MarkLookAt(TEXT("TeaClose"), FVector(HX - 96.f, 75.f, 98.f), FVector(HX - 46.f, 40.f, 81.f), 27.f);
	MarkLookAt(TEXT("Calendar"), FVector(-15.f, 22.f, 152.f), FVector(-15.f, HY, 150.f), 40.f);
	MarkLookAt(TEXT("CalendarClose"), FVector(-22.f, 108.f, 138.f), FVector(-22.f, HY, 136.f), 34.f);
	MarkLookAt(TEXT("Door"), FVector(130.f, -50.f, 142.f), FVector(-HX, 105.f, 112.f), 58.f);
	MarkLookAt(TEXT("Book"), FVector(-58.f, -38.f, 112.f), FVector(-70.f, -HY + 62.f, 48.f), 40.f);
}

void ACHLoc_Room::BuildShell()
{
	UMaterialInterface* Wall = Lit(FLinearColor(0.34f, 0.27f, 0.185f), 0.95f, 0.22f);
	UMaterialInterface* Ceiling = Lit(FLinearColor(0.42f, 0.39f, 0.33f), 0.95f, 0.15f);
	UMaterialInterface* Floor = Lit(FLinearColor(0.17f, 0.10f, 0.055f), 0.55f, 0.4f);
	UMaterialInterface* Trim = Lit(FLinearColor(0.30f, 0.27f, 0.22f), 0.7f, 0.2f);
	UMaterialInterface* DarkWood = Lit(FLinearColor(0.06f, 0.035f, 0.02f), 0.5f, 0.3f);
	UMaterialInterface* Curtain = Lit(FLinearColor(0.55f, 0.48f, 0.36f), 1.f, 0.1f);

	const float FullX = 2.f * (HX + T);
	const float FullY = 2.f * (HY + T);

	// floor and ceiling
	BoxAt(-HX - T, -HY - T, -20.f, FullX, FullY, 20.f, Floor);
	BoxAt(-HX - T, -HY - T, H, FullX, FullY, 20.f, Ceiling);

	// side walls
	BoxAt(-HX - T, -HY - T, 0.f, FullX, T, H, Wall);
	BoxAt(-HX - T, HY, 0.f, FullX, T, H, Wall);

	// window wall (+X): opening Y[-70,70] Z[95,215]
	BoxAt(HX, -HY, 0.f, T, HY - 70.f, H, Wall);
	BoxAt(HX, 70.f, 0.f, T, HY - 70.f, H, Wall);
	BoxAt(HX, -70.f, 0.f, T, 140.f, 95.f, Wall);
	BoxAt(HX, -70.f, 215.f, T, 140.f, H - 215.f, Wall);
	BoxAt(HX - 10.f, -76.f, 91.f, 34.f, 152.f, 4.f, Trim);            // sill
	BoxAt(HX + 8.f, -2.f, 95.f, 5.f, 4.f, 120.f, DarkWood, false);    // mullion
	BoxAt(HX + 8.f, -70.f, 153.f, 5.f, 140.f, 4.f, DarkWood, false);  // transom
	BoxAt(HX - 5.f, -108.f, 55.f, 3.f, 36.f, 172.f, Curtain, false);  // curtains
	BoxAt(HX - 5.f, 72.f, 55.f, 3.f, 36.f, 172.f, Curtain, false);
	BoxAt(HX - 8.f, -112.f, 226.f, 6.f, 224.f, 4.f, DarkWood, false); // curtain rail

	// door wall (-X): opening Y[60,150] Z[0,205]
	BoxAt(-HX - T, -HY, 0.f, T, HY + 60.f, H, Wall);
	BoxAt(-HX - T, 150.f, 0.f, T, HY - 150.f, H, Wall);
	BoxAt(-HX - T, 60.f, 205.f, T, 90.f, H - 205.f, Wall);

	// door leaf on a hinge
	DoorPivot = Pivot(FVector(-HX - T * 0.5f, 150.f, 0.f));
	BoxOn(DoorPivot, FVector(0.f, -45.f, 102.5f), FVector(4.f, 90.f, 205.f), DarkWood, true);
	BoxOn(DoorPivot, FVector(4.f, -80.f, 100.f), FVector(7.f, 12.f, 2.5f), Trim, false);

	// a dim hallway behind the door so an open door never shows the void
	UMaterialInterface* Hall = Lit(FLinearColor(0.20f, 0.17f, 0.13f), 0.95f, 0.3f);
	const float HallX = -HX - T - 220.f;
	BoxAt(HallX, 10.f, -20.f, 220.f, 190.f, 20.f, Floor);
	BoxAt(HallX, 10.f, H, 220.f, 190.f, 20.f, Hall);
	BoxAt(HallX - T, 10.f, 0.f, T, 190.f, H, Hall);
	BoxAt(HallX, 10.f - T, 0.f, 220.f, T, H, Hall);
	BoxAt(HallX, 200.f, 0.f, 220.f, T, H, Hall);
	PointLight(FVector(HallX + 110.f, 105.f, 215.f), FLinearColor(1.f, 0.72f, 0.42f), 5.f, 600.f, false, 0.4f);

	// neighbouring blocks seen through the window
	UMaterialInterface* Concrete = Lit(FLinearColor(0.22f, 0.21f, 0.20f), 0.95f, 0.5f);
	UMaterialInterface* Concrete2 = Lit(FLinearColor(0.28f, 0.25f, 0.21f), 0.95f, 0.5f);
	UMaterialInterface* Tank = Lit(FLinearColor(0.35f, 0.36f, 0.38f), 0.5f, 0.3f);
	BoxAt(1500.f, -1700.f, -1400.f, 900.f, 1100.f, 1310.f, Concrete, false);
	BoxAt(1700.f, -300.f, -1400.f, 800.f, 900.f, 1420.f, Concrete2, false);
	BoxAt(2000.f, 900.f, -1400.f, 1000.f, 1200.f, 1280.f, Concrete, false);
	BoxAt(3600.f, -3200.f, -1400.f, 1400.f, 2600.f, 1560.f, Concrete2, false);
	BoxAt(3900.f, -200.f, -1400.f, 1300.f, 1500.f, 1470.f, Concrete, false);
	BoxAt(3700.f, 1700.f, -1400.f, 1500.f, 2400.f, 1600.f, Concrete2, false);
	BoxAt(6500.f, -6000.f, -1400.f, 2000.f, 12000.f, 1500.f, Concrete, false);
	// water tanks, a stair head and a dish on the nearest roofs
	Cyl(FVector(1950.f, -1050.f, -35.f), 55.f, 110.f, Tank);
	Cyl(FVector(2050.f, 150.f, 75.f), 55.f, 110.f, Tank);
	BoxAt(2150.f, 300.f, 20.f, 220.f, 260.f, 190.f, Concrete, false);
	Cyl(FVector(2500.f, 1400.f, -65.f), 55.f, 110.f, Tank);
	Ball(FVector(1850.f, -150.f, 60.f), FVector(90.f, 30.f, 90.f), Tank, false, FRotator(0.f, 30.f, 25.f));

	// far across the city: the Alborz, Damavand in the middle
	BuildAlborz(4.f, FLinearColor(0.20f, 0.24f, 0.32f), FLinearColor(0.80f, 0.82f, 0.86f));
}

void ACHLoc_Room::BuildFurniture()
{
	const bool bNight = Variant == TEXT("night");
	const bool bAfter = Variant == TEXT("after");

	UMaterialInterface* Wood = Lit(FLinearColor(0.20f, 0.11f, 0.055f), 0.5f, 0.3f);
	UMaterialInterface* DarkWood = Lit(FLinearColor(0.06f, 0.035f, 0.02f), 0.5f, 0.3f);
	UMaterialInterface* Metal = Lit(FLinearColor(0.10f, 0.10f, 0.11f), 0.35f, 0.1f, 0.8f);
	UMaterialInterface* Ceramic = Lit(FLinearColor(0.75f, 0.73f, 0.68f), 0.25f, 0.05f);
	UMaterialInterface* Paper = Lit(FLinearColor(0.78f, 0.75f, 0.66f), 0.95f, 0.1f);
	UMaterialInterface* Cream = Lit(FLinearColor(0.62f, 0.58f, 0.50f), 1.f, 0.15f);
	UMaterialInterface* Blanket = Lit(FLinearColor(0.10f, 0.14f, 0.20f), 1.f, 0.25f);
	UMaterialInterface* Tea = Lit(FLinearColor(0.42f, 0.09f, 0.012f), 0.08f, 0.f);
	UMaterialInterface* Plastic = Lit(FLinearColor(0.03f, 0.03f, 0.035f), 0.4f, 0.1f);

	// ---- rug (a Persian carpet in a few flat layers)
	Box(FVector(-25.f, 35.f, 0.4f), FVector(210.f, 150.f, 0.8f), Lit(FLinearColor(0.20f, 0.025f, 0.02f), 1.f, 0.3f), false);
	Box(FVector(-25.f, 35.f, 0.7f), FVector(190.f, 130.f, 0.8f), Lit(FLinearColor(0.025f, 0.035f, 0.09f), 1.f, 0.3f), false);
	Box(FVector(-25.f, 35.f, 1.0f), FVector(170.f, 110.f, 0.8f), Lit(FLinearColor(0.27f, 0.035f, 0.025f), 1.f, 0.3f), false);
	Box(FVector(-25.f, 35.f, 1.3f), FVector(52.f, 52.f, 0.8f), Lit(FLinearColor(0.52f, 0.42f, 0.26f), 1.f, 0.3f), false, FRotator(0.f, 45.f, 0.f));
	Box(FVector(-25.f, 35.f, 1.6f), FVector(30.f, 30.f, 0.8f), Lit(FLinearColor(0.025f, 0.035f, 0.09f), 1.f, 0.3f), false, FRotator(0.f, 45.f, 0.f));

	// ---- desk under the window
	const float DeskX = HX - 40.f;
	Box(FVector(DeskX, 0.f, 74.f), FVector(64.f, 150.f, 4.f), Wood);
	Box(FVector(DeskX, -73.f, 36.f), FVector(60.f, 4.f, 72.f), Wood);
	Box(FVector(DeskX, 73.f, 36.f), FVector(60.f, 4.f, 72.f), Wood);
	Box(FVector(HX - 12.f, 0.f, 50.f), FVector(3.f, 142.f, 40.f), Wood, false);

	// the glass of tea on its saucer
	// a narrow-waisted Persian tea glass: the tea itself, and a faint shell of glass around and above it
	const FVector TeaPos(HX - 46.f, 40.f, 76.f);
	UMaterialInterface* GlassShell = Glow(FLinearColor(0.85f, 0.92f, 1.f), 0.22f, 0.16f);
	Cyl(TeaPos + FVector(0.f, 0.f, 0.4f), 6.0f, 0.8f, Ceramic);
	Cyl(TeaPos + FVector(0.f, 0.f, 2.3f), 2.7f, 3.0f, Tea);
	Cyl(TeaPos + FVector(0.f, 0.f, 4.9f), 2.25f, 2.4f, Tea);
	Cyl(TeaPos + FVector(0.f, 0.f, 7.5f), 2.95f, 3.0f, Tea);
	Cyl(TeaPos + FVector(0.f, 0.f, 5.4f), 3.15f, 9.2f, GlassShell)->SetCastShadow(false);
	Box(TeaPos + FVector(3.f, -9.f, 1.3f), FVector(1.f, 9.f, 0.5f), Metal, false, FRotator(0.f, 25.f, 0.f)); // spoon
	Box(TeaPos + FVector(-6.f, 7.f, 1.6f), FVector(1.6f, 1.6f, 1.2f), Ceramic, false);                        // sugar cubes
	Box(TeaPos + FVector(-4.f, 8.5f, 1.6f), FVector(1.6f, 1.6f, 1.2f), Ceramic, false, FRotator(0.f, 30.f, 0.f));

	// laptop
	Box(FVector(DeskX - 6.f, -18.f, 77.f), FVector(24.f, 34.f, 1.6f), Plastic, false);
	Box(FVector(DeskX + 9.f, -18.f, 88.5f), FVector(1.4f, 34.f, 23.f), Plastic, false, FRotator(-14.f, 0.f, 0.f));
	Box(FVector(DeskX + 8.1f, -18.f, 88.6f), FVector(0.4f, 31.f, 20.f), bAfter ? Plastic : Emissive(FLinearColor(0.20f, 0.36f, 0.62f), bNight ? 1.4f : 0.45f), false, FRotator(-14.f, 0.f, 0.f));

	// books and a pen cup
	Box(FVector(DeskX + 2.f, -56.f, 78.f), FVector(22.f, 30.f, 4.f), Lit(FLinearColor(0.20f, 0.05f, 0.04f), 0.8f, 0.2f), false, FRotator(0.f, 6.f, 0.f));
	Box(FVector(DeskX + 2.f, -56.f, 81.5f), FVector(20.f, 28.f, 3.f), Lit(FLinearColor(0.07f, 0.12f, 0.10f), 0.8f, 0.2f), false, FRotator(0.f, -8.f, 0.f));
	Box(FVector(DeskX + 2.f, -56.f, 84.2f), FVector(19.f, 26.f, 2.4f), Lit(FLinearColor(0.50f, 0.42f, 0.22f), 0.8f, 0.2f), false, FRotator(0.f, 3.f, 0.f));
	Cyl(FVector(DeskX + 14.f, 62.f, 81.f), 3.6f, 10.f, Plastic);
	Box(FVector(DeskX + 14.f, 62.f, 89.f), FVector(0.7f, 0.7f, 12.f), Lit(FLinearColor(0.6f, 0.45f, 0.1f), 0.6f, 0.f), false, FRotator(8.f, 0.f, 6.f));
	Box(FVector(DeskX + 15.f, 63.f, 89.f), FVector(0.7f, 0.7f, 12.f), Lit(FLinearColor(0.1f, 0.15f, 0.5f), 0.6f, 0.f), false, FRotator(-6.f, 0.f, -9.f));

	// desk lamp
	Cyl(FVector(DeskX + 12.f, 20.f, 77.f), 6.f, 2.f, Metal);
	Box(FVector(DeskX + 12.f, 20.f, 95.f), FVector(1.6f, 1.6f, 36.f), Metal, false, FRotator(0.f, 0.f, 0.f));
	Cone(FVector(DeskX + 6.f, 20.f, 112.f), 9.f, 12.f, bNight ? Emissive(FLinearColor(1.f, 0.62f, 0.28f), CHLight::Emissive(2.5f), FLinearColor(0.08f, 0.05f, 0.02f)) : Metal, false, FRotator(-35.f, 0.f, 0.f));
	if (bNight)
	{
		PointLight(FVector(DeskX + 2.f, 20.f, 100.f), FLinearColor(1.f, 0.58f, 0.26f), 5.f, 480.f, true, 0.8f);
	}

	// chair, pushed back from the desk
	const FVector Chair(HX - 118.f, -4.f, 0.f);
	Box(Chair + FVector(0.f, 0.f, 45.f), FVector(42.f, 42.f, 4.f), DarkWood);
	Box(Chair + FVector(-20.f, 0.f, 72.f), FVector(4.f, 42.f, 50.f), DarkWood, false);
	for (int32 i = 0; i < 4; ++i)
	{
		const float SX = (i & 1) ? 18.f : -18.f;
		const float SY = (i & 2) ? 18.f : -18.f;
		Box(Chair + FVector(SX, SY, 22.f), FVector(4.f, 4.f, 44.f), DarkWood, false);
	}

	// ---- bed along the -Y wall, head toward the window
	const float BedY = -HY + 52.f;
	Box(FVector(-60.f, BedY, 16.f), FVector(205.f, 100.f, 32.f), DarkWood);
	Box(FVector(-60.f, BedY, 38.f), FVector(198.f, 94.f, 14.f), Cream);
	Box(FVector(-88.f, BedY, 46.f), FVector(140.f, 97.f, 3.f), Blanket, false);
	Box(FVector(22.f, BedY, 49.f), FVector(36.f, 62.f, 9.f), Paper, false, FRotator(0.f, 4.f, 0.f));
	Box(FVector(44.f, BedY, 58.f), FVector(5.f, 102.f, 84.f), DarkWood);

	// the German grammar book on the bed
	const FVector BookPos = BookSpot;
	const FRotator BookRot(0.f, 22.f, 0.f);
	UMaterialInterface* Cover = Lit(FLinearColor(0.30f, 0.03f, 0.03f), 0.7f, 0.1f);
	if (bAfter)
	{
		// closed, with a letter folded inside; a corner of the paper shows
		Box(BookPos + FVector(0.f, 0.f, 1.2f), FVector(22.f, 16.f, 3.2f), Cover, false, BookRot);
		Box(BookPos + BookRot.RotateVector(FVector(3.f, 8.5f, 1.4f)), FVector(10.f, 3.f, 0.3f), Paper, false, BookRot + FRotator(0.f, 7.f, 0.f));
		LetterPaper = Box(LetterSpot, FVector(15.f, 21.f, 0.3f), Paper, false, FRotator(0.f, -24.f, 0.f), true);
		LetterPaper->SetVisibility(false);
	}
	else
	{
		// open, where he was studying
		Box(BookPos, FVector(22.f, 32.f, 0.8f), Cover, false, BookRot);
		Box(BookPos + BookRot.RotateVector(FVector(0.f, -7.6f, 1.0f)), FVector(20.f, 14.6f, 1.4f), Paper, false, FRotator(0.f, 22.f, 5.f));
		Box(BookPos + BookRot.RotateVector(FVector(0.f, 7.6f, 1.0f)), FVector(20.f, 14.6f, 1.4f), Paper, false, FRotator(0.f, 22.f, -5.f));
	}

	// picture above the bed
	Box(FVector(-80.f, -HY + 1.5f, 165.f), FVector(70.f, 3.f, 50.f), DarkWood, false);
	Box(FVector(-80.f, -HY + 3.2f, 165.f), FVector(62.f, 1.f, 42.f), Lit(FLinearColor(0.16f, 0.20f, 0.22f), 0.9f, 0.5f), false);

	// ---- wardrobe by the door
	Box(FVector(-150.f, HY - 29.f, 100.f), FVector(100.f, 56.f, 200.f), DarkWood);
	Box(FVector(-150.f, HY - 58.f, 100.f), FVector(1.f, 1.f, 190.f), Lit(FLinearColor(0.02f, 0.012f, 0.008f), 0.5f, 0.f), false);
	Box(FVector(-156.f, HY - 58.5f, 105.f), FVector(1.5f, 2.f, 14.f), Metal, false);
	Box(FVector(-144.f, HY - 58.5f, 105.f), FVector(1.5f, 2.f, 14.f), Metal, false);

	// ---- bookshelf by the window
	const float ShelfX = 110.f;
	Box(FVector(ShelfX, HY - 2.f, 100.f), FVector(104.f, 3.f, 200.f), Wood, false);
	Box(FVector(ShelfX - 52.f, HY - 14.f, 100.f), FVector(3.f, 28.f, 200.f), Wood);
	Box(FVector(ShelfX + 52.f, HY - 14.f, 100.f), FVector(3.f, 28.f, 200.f), Wood);
	FRandomStream Rng(1401);
	const FLinearColor BookColors[] = {
		FLinearColor(0.25f, 0.05f, 0.04f), FLinearColor(0.05f, 0.10f, 0.16f), FLinearColor(0.35f, 0.28f, 0.14f),
		FLinearColor(0.07f, 0.13f, 0.08f), FLinearColor(0.45f, 0.40f, 0.33f), FLinearColor(0.10f, 0.08f, 0.07f),
		FLinearColor(0.30f, 0.14f, 0.05f) };
	for (int32 Level = 0; Level < 6; ++Level)
	{
		const float Z = 4.f + Level * 39.f;
		Box(FVector(ShelfX, HY - 14.f, Z), FVector(102.f, 28.f, 3.f), Wood, false);
		if (Level == 5)
		{
			break;
		}
		float X = ShelfX - 48.f;
		while (X < ShelfX + 44.f)
		{
			const float Width = Rng.FRandRange(2.4f, 5.2f);
			const float Height = Rng.FRandRange(19.f, 29.f);
			const float Depth = Rng.FRandRange(15.f, 20.f);
			if (Rng.FRand() < 0.12f)
			{
				X += Rng.FRandRange(4.f, 12.f);
				continue;
			}
			const FLinearColor Color = BookColors[Rng.RandRange(0, UE_ARRAY_COUNT(BookColors) - 1)] * Rng.FRandRange(0.7f, 1.15f);
			BoxInstance(FVector(X + Width * 0.5f, HY - 4.f - Depth * 0.5f, Z + 1.5f + Height * 0.5f), FVector(Width, Depth, Height),
				Lit(FLinearColor(Color.R, Color.G, Color.B), 0.8f, 0.2f), FRotator(0.f, 0.f, Rng.FRand() < 0.1f ? 8.f : 0.f));
			X += Width + 0.3f;
		}
	}

	// ---- wall calendar (the page; the month grid and the red circle are drawn on top of it)
	if (bAfter)
	{
		Box(CalendarFloor + FVector(0.f, 0.f, 2.1f), FVector(46.f, 62.f, 0.4f), Paper, false, FRotator(0.f, CalendarYaw, 0.f));
	}
	else
	{
		CalendarPage = Box(FVector(-15.f, HY - 0.8f, 150.f), FVector(46.f, 1.6f, 62.f), Paper, false, FRotator::ZeroRotator, true);
	}
	Box(FVector(-15.f, HY - 1.2f, 183.f), FVector(4.f, 2.4f, 4.f), Metal, false);

	// ---- the setar, leaning in the corner by the bed
	const FVector Setar(HX - 26.f, -HY + 22.f, 0.f);
	UMaterialInterface* SetarWood = Lit(FLinearColor(0.33f, 0.17f, 0.06f), 0.35f, 0.15f);
	Ball(Setar + FVector(0.f, 0.f, 20.f), FVector(17.f, 20.f, 30.f), SetarWood, false, FRotator(0.f, 0.f, -8.f));
	Box(Setar + FVector(0.f, -6.5f, 70.f), FVector(2.6f, 2.2f, 78.f), DarkWood, false, FRotator(0.f, 0.f, -8.f));
	Box(Setar + FVector(0.f, -11.5f, 106.f), FVector(8.f, 1.4f, 1.4f), DarkWood, false);
	Box(Setar + FVector(0.f, -10.9f, 101.f), FVector(8.f, 1.4f, 1.4f), DarkWood, false);

	// ---- ceiling lamp
	Cyl(FVector(0.f, 0.f, H - 9.f), 1.2f, 18.f, Metal);
	Ball(FVector(0.f, 0.f, H - 26.f), FVector(26.f, 26.f, 20.f), Lit(FLinearColor(0.55f, 0.50f, 0.40f), 0.6f, 0.1f));
}

void ACHLoc_Room::OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat)
{
	if (Event == TEXT("door_open"))
	{
		DoorTarget = 82.f;
	}
	else if (Event == TEXT("door_close"))
	{
		DoorTarget = 0.f;
	}
	else if (Event == TEXT("calendar_down"))
	{
		// he takes the calendar off the wall (1.9); the nail stays
		if (CalendarPage)
		{
			CalendarPage->SetVisibility(false);
		}
		if (CalendarSheet)
		{
			CalendarSheet->SetVisibility(false);
		}
	}
}

void ACHLoc_Room::OnInteract(const FString& Id, ACHInteractable* What)
{
	if (What)
	{
		What->SetEnabled(false);
	}
	if (Id == TEXT("book"))
	{
		// she opens it; the letter slips out onto the floor
		if (LetterPaper)
		{
			LetterPaper->SetVisibility(true);
		}
		FinishSegment();
	}
}

void ACHLoc_Room::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (DoorPivot && !FMath::IsNearlyEqual(DoorAngle, DoorTarget, 0.01f))
	{
		DoorAngle = FMath::FInterpTo(DoorAngle, DoorTarget, DeltaSeconds, 3.2f);
		DoorPivot->SetRelativeRotation(FRotator(0.f, DoorAngle, 0.f));
	}
}
