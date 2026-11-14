// Cheshm Hayash - the family's living room.

#include "World/CHLocations.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Game/CHInteractable.h"
#include "Game/CHSegment.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/RandomStream.h"
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
	const bool bYears = Variant == TEXT("years");
	const float EV = bYears ? -0.6f : -0.4f;
	CHLight::SetExposure(EV);
	PlayerBoomLength = 0.f;

	UMaterialInterface* Wall = Lit(bYears ? FLinearColor(0.40f, 0.37f, 0.32f) : FLinearColor(0.36f, 0.31f, 0.24f), 0.95f, 0.2f);
	UMaterialInterface* Ceiling = Lit(FLinearColor(0.45f, 0.43f, 0.39f), 0.95f, 0.1f);
	UMaterialInterface* Floor = Lit(FLinearColor(0.16f, 0.10f, 0.06f), 0.55f, 0.35f);
	UMaterialInterface* Wood = Lit(FLinearColor(0.15f, 0.08f, 0.04f), 0.5f, 0.3f);
	UMaterialInterface* Fabric = Lit(bYears ? FLinearColor(0.16f, 0.12f, 0.10f) : FLinearColor(0.20f, 0.06f, 0.05f), 1.f, 0.2f);
	UMaterialInterface* Dark = Lit(FLinearColor(0.012f, 0.012f, 0.014f), 0.3f, 0.f);
	UMaterialInterface* Ceramic = Lit(FLinearColor(0.75f, 0.73f, 0.68f), 0.25f, 0.05f);
	UMaterialInterface* Tea = Lit(FLinearColor(0.42f, 0.09f, 0.012f), 0.08f, 0.f);
	UMaterialInterface* Glass = Glow(FLinearColor(0.85f, 0.92f, 1.f), 0.2f, 0.16f);
	UMaterialInterface* Metal = Lit(FLinearColor(0.35f, 0.33f, 0.28f), 0.3f, 0.1f, 0.8f);

	// the room
	BoxAt(-HX - T, -HY - T, -20.f, 2.f * (HX + T), 2.f * (HY + T), 20.f, Floor);
	BoxAt(-HX - T, -HY - T, H, 2.f * (HX + T), 2.f * (HY + T), 20.f, Ceiling);
	BoxAt(-HX - T, -HY - T, 0.f, T, 2.f * (HY + T), H, Wall);
	BoxAt(HX, -HY - T, 0.f, T, 2.f * (HY + T), H, Wall);
	BoxAt(-HX, -HY - T, 0.f, 2.f * HX, T, H, Wall);
	// the window wall (+Y) with an evening outside
	BoxAt(-HX, HY, 0.f, HX - 90.f, T, H, Wall);
	BoxAt(90.f, HY, 0.f, HX - 90.f, T, H, Wall);
	BoxAt(-90.f, HY, 0.f, 180.f, T, 90.f, Wall);
	BoxAt(-90.f, HY, 230.f, 180.f, T, H - 230.f, Wall);
	Box(FVector(0.f, HY + 300.f, 160.f), FVector(900.f, 4.f, 500.f), Unlit(bYears ? FLinearColor(0.02f, 0.035f, 0.08f) : FLinearColor(0.008f, 0.01f, 0.02f), 1.f), false);
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
		if (bYears)
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
	TvScreen = WorldWidget(MakeNewsScreen(), FVector(HX - 35.f, 0.f, 98.f), FRotator(0.f, 180.f, 0.f), 0.2f, true, FVector2D(620.0, 360.0));
	if (UMaterialInstanceDynamic* Screen = TvScreen->GetMaterialInstance())
	{
		Screen->SetScalarParameterValue(TEXT("EmissiveStrength"), CHLight::Emissive(0.9f));
	}
	PointLight(FVector(HX - 80.f, 0.f, 100.f), FLinearColor(0.75f, 0.82f, 1.f), bYears ? 7.f : 0.f, 800.f, false, 0.3f);

	// his photograph with a black ribbon across its corner, a flower
	Box(FVector(HX - 25.f, 70.f, 74.f), FVector(4.f, 26.f, 34.f), Wood, false, FRotator(0.f, 0.f, 0.f));
	Box(FVector(HX - 27.5f, 70.f, 74.f), FVector(1.f, 22.f, 30.f), Lit(FLinearColor(0.05f, 0.05f, 0.055f), 0.6f, 0.f), false);
	Box(FVector(HX - 28.5f, 78.f, 84.f), FVector(1.f, 4.f, 18.f), Dark, false, FRotator(0.f, 0.f, 45.f));
	Cyl(FVector(HX - 25.f, 40.f, 64.f), 3.f, 14.f, Glass);
	Ball(FVector(HX - 25.f, 40.f, 79.f), FVector(7.f, 7.f, 6.f), Lit(FLinearColor(0.5f, 0.02f, 0.03f), 0.8f, 0.f));

	// a floor lamp
	Cyl(FVector(-HX + 40.f, HY - 40.f, 80.f), 2.f, 160.f, Metal);
	Cone(FVector(-HX + 40.f, HY - 40.f, 168.f), 22.f, 30.f, Emissive(FLinearColor(1.f, 0.62f, 0.3f), CHLight::Emissive(1.5f), FLinearColor(0.2f, 0.12f, 0.06f)), false, FRotator(180.f, 0.f, 0.f));
	PointLight(FVector(-HX + 40.f, HY - 40.f, 150.f), FLinearColor(1.f, 0.62f, 0.32f), 6.f, 700.f, true, 0.5f);

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
	Mark(TEXT("PlayerStart"), FVector(0.f, -HY + 80.f, 0.f), FRotator(0.f, 90.f, 0.f));
	Mark(TEXT("SofaLeft"), FVector(-HX + 60.f, -45.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("SofaRight"), FVector(-HX + 60.f, 50.f, 0.f), FRotator(0.f, 0.f, 0.f));
	Mark(TEXT("SofaClose"), FVector(-HX + 60.f, 20.f, 0.f), FRotator(0.f, 0.f, 0.f));
	MarkLookAt(TEXT("Wide"), FVector(HX - 60.f, -HY + 50.f, 190.f), FVector(-HX + 60.f, 20.f, 70.f), 62.f);
	MarkLookAt(TEXT("TV"), FVector(-HX + 15.f, -10.f, 150.f), FVector(HX - 30.f, 0.f, 98.f), 42.f);
	MarkLookAt(TEXT("Faces"), FVector(HX - 90.f, 30.f, 115.f), FVector(-HX + 60.f, 0.f, 95.f), 38.f);
	MarkLookAt(TEXT("Tea"), FVector(-60.f, -60.f, 75.f), FVector(-122.f, 0.f, 46.f), 30.f);
}

TSharedRef<SWidget> ACHLoc_Home::MakeNewsScreen() const
{
	// a black-bordered news screen; no name, no face, no portrait
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
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("رهبر درگذشت.")))
						.Font(CH::FarsiFont(58.f, CH::EWeight::Bold))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.93f, 1.f)))
						.TextShapingMethod(ETextShapingMethod::FullShaping)
						.TextFlowDirection(ETextFlowDirection::RightToLeft)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 10.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("The Leader has died.")))
						.Font(CH::LatinFont(24.f, CH::EWeight::Regular))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.75f, 0.74f, 1.f)))
					]
				]
			]
		];
}

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
}

void ACHLoc_Home::OnInteract(const FString& Id, ACHInteractable* What)
{
	if (What)
	{
		What->SetEnabled(false);
	}
}

void ACHLoc_Home::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	HomeTime += DeltaSeconds;

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
