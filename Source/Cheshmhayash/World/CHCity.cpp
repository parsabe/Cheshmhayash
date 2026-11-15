// Cheshm Hayash - the Tehran kit.

#include "World/CHCity.h"
#include "Components/LocalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "Game/CHSegment.h"
#include "Math/RandomStream.h"
#include "UI/CHStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "World/CHFire.h"

namespace
{
	const float GroundFloorH = 380.f;
	const float FloorH = 310.f;
	const float ParapetH = 100.f;
	const float ParapetT = 20.f;
}

void ACHCityBuilder::InitCity(bool bInNight, float ExposureEV)
{
	bNight = bInNight;
	CHLight::SetExposure(ExposureEV);
	auto Glow = [](float Screen) { return CHLight::Emissive(Screen); };

	MAsphalt = Lit(FLinearColor(0.032f, 0.032f, 0.035f), 0.72f, 0.55f);
	MLine = Lit(FLinearColor(0.40f, 0.38f, 0.28f), 0.8f, 0.7f);
	MCurb = Lit(FLinearColor(0.28f, 0.27f, 0.25f), 0.85f, 0.5f);
	MSidewalk = Lit(FLinearColor(0.17f, 0.13f, 0.11f), 0.9f, 0.55f);
	MJube = Lit(FLinearColor(0.010f, 0.010f, 0.010f), 0.25f, 0.2f);
	MGlass = Lit(FLinearColor(0.010f, 0.012f, 0.016f), 0.1f, 0.f);
	MGlassLitWarm = Emissive(FLinearColor(1.f, 0.56f, 0.24f), bNight ? Glow(2.2f) : 0.f, FLinearColor(0.05f, 0.035f, 0.02f));
	MGlassLitDim = Emissive(FLinearColor(1.f, 0.42f, 0.16f), bNight ? Glow(0.8f) : 0.f, FLinearColor(0.04f, 0.03f, 0.02f));
	MGlassLitCool = Emissive(FLinearColor(0.42f, 0.56f, 1.f), bNight ? Glow(1.2f) : 0.f, FLinearColor(0.02f, 0.025f, 0.04f));
	MFrame = Lit(FLinearColor(0.025f, 0.023f, 0.022f), 0.6f, 0.3f);
	MBand = Lit(FLinearColor(0.30f, 0.28f, 0.25f), 0.85f, 0.45f);
	MMetal = Lit(FLinearColor(0.12f, 0.12f, 0.13f), 0.45f, 0.4f, 0.6f);
	MShutter = Lit(FLinearColor(0.16f, 0.16f, 0.17f), 0.55f, 0.6f, 0.5f);
	MRust = Lit(FLinearColor(0.10f, 0.07f, 0.05f), 0.7f, 0.6f, 0.3f);
	MPole = Lit(FLinearColor(0.05f, 0.05f, 0.055f), 0.6f, 0.5f, 0.4f);
	MLampOn = Emissive(FLinearColor(1.f, 0.56f, 0.16f), Glow(40.f), FLinearColor(0.1f, 0.06f, 0.02f));
	MLampOff = Lit(FLinearColor(0.05f, 0.05f, 0.05f), 0.4f, 0.4f);
	MTank = Lit(FLinearColor(0.32f, 0.33f, 0.35f), 0.4f, 0.4f, 0.5f);
	MTankBlue = Lit(FLinearColor(0.05f, 0.10f, 0.20f), 0.5f, 0.4f);
	MCooler = Lit(FLinearColor(0.30f, 0.28f, 0.22f), 0.7f, 0.6f, 0.2f);
	MDish = Lit(FLinearColor(0.45f, 0.45f, 0.44f), 0.5f, 0.4f);
	MBark = Lit(FLinearColor(0.22f, 0.20f, 0.16f), 0.9f, 0.6f);
	MLeaves = Lit(bNight ? FLinearColor(0.025f, 0.04f, 0.018f) : FLinearColor(0.06f, 0.10f, 0.035f), 0.9f, 0.4f);
	MTyre = Lit(FLinearColor(0.012f, 0.012f, 0.012f), 0.9f, 0.3f);
	MCarGlass = Lit(FLinearColor(0.008f, 0.010f, 0.013f), 0.08f, 0.f);
	MWire = Lit(FLinearColor(0.01f, 0.01f, 0.01f), 0.6f, 0.f);
	MRoof = Lit(FLinearColor(0.10f, 0.095f, 0.09f), 0.9f, 0.8f);
	MShopLit = Emissive(FLinearColor(1.f, 0.72f, 0.42f), Glow(1.4f), FLinearColor(0.1f, 0.08f, 0.05f));

	MFacades.Reset();
	MFacades.Add(Lit(FLinearColor(0.36f, 0.33f, 0.28f), 0.9f, 0.45f)); // travertine cream
	MFacades.Add(Lit(FLinearColor(0.30f, 0.26f, 0.21f), 0.92f, 0.5f)); // beige concrete
	MFacades.Add(Lit(FLinearColor(0.22f, 0.21f, 0.20f), 0.95f, 0.55f)); // grey concrete
	MFacades.Add(Lit(FLinearColor(0.31f, 0.19f, 0.13f), 0.92f, 0.5f)); // brick
	MFacades.Add(Lit(FLinearColor(0.40f, 0.37f, 0.32f), 0.88f, 0.4f)); // light stone
	MFacades.Add(Lit(FLinearColor(0.17f, 0.15f, 0.13f), 0.95f, 0.6f)); // dark stained

	MSigns.Reset();
	MSigns.Add(Lit(FLinearColor(0.20f, 0.03f, 0.02f), 0.5f, 0.4f));
	MSigns.Add(Lit(FLinearColor(0.03f, 0.10f, 0.05f), 0.5f, 0.4f));
	MSigns.Add(Lit(FLinearColor(0.04f, 0.06f, 0.16f), 0.5f, 0.4f));
	MSigns.Add(Lit(FLinearColor(0.30f, 0.24f, 0.05f), 0.5f, 0.4f));
	MSigns.Add(Lit(FLinearColor(0.12f, 0.12f, 0.12f), 0.5f, 0.4f));

	MLaundry.Reset();
	MLaundry.Add(Lit(FLinearColor(0.30f, 0.05f, 0.04f), 1.f, 0.1f));
	MLaundry.Add(Lit(FLinearColor(0.40f, 0.38f, 0.33f), 1.f, 0.1f));
	MLaundry.Add(Lit(FLinearColor(0.05f, 0.08f, 0.18f), 1.f, 0.1f));
	MLaundry.Add(Lit(FLinearColor(0.20f, 0.17f, 0.05f), 1.f, 0.1f));
}

void ACHCityBuilder::NightEnv(float ExposureEV, float FogDensity)
{
	FCHEnv Env;
	// A real night: the people shout in the dark. The sun is far below the horizon and only colours
	// the sky edge a cold blue; a faint moon and sky keep the shapes of the street and the mountains;
	// the light itself comes from lamps, windows and fires.
	Env.SunRotation = FRotator(9.f, 60.f, 0.f);
	Env.SunIntensity = 2.f;
	Env.bSunLightsWorld = false;
	Env.bSunShadows = false;
	Env.bMoon = true;
	Env.MoonRotation = FRotator(-38.f, -130.f, 0.f);
	Env.MoonIntensity = 0.12f;
	Env.MoonColor = FLinearColor(0.5f, 0.62f, 1.f);
	Env.SkyLightIntensity = 0.5f;
	Env.SkyLightColor = FLinearColor(0.55f, 0.65f, 1.f);
	Env.bFog = true;
	Env.FogDensity = FogDensity;
	Env.FogHeightFalloff = 0.09f;
	Env.FogColor = FLinearColor(0.010f, 0.010f, 0.016f);
	Env.bVolumetricFog = true;
	Env.VolumetricExtinction = 1.3f;
	Env.ExposureEV = ExposureEV;
	Env.Grain = 0.4f;
	Env.Vignette = 0.6f;
	Env.Fringe = 0.5f;
	Env.Bloom = 1.0f;
	Env.Saturation = 0.95f;
	Env.Contrast = 1.08f;
	Env.Temperature = 5900.f;
	ApplyEnv(Env);
}

// ---------------------------------------------------------------------------------------------
// Pieces
// ---------------------------------------------------------------------------------------------

UStaticMeshComponent* ACHCityBuilder::BoxRel(const FVector& Origin, float Yaw, const FVector& Local, const FVector& Size, UMaterialInterface* Material, bool bCollision, const FRotator& Extra)
{
	const FRotator Rot(0.f, Yaw, 0.f);
	return Box(Origin + Rot.RotateVector(Local), Size, Material, bCollision, (Rot.Quaternion() * Extra.Quaternion()).Rotator());
}

UStaticMeshComponent* ACHCityBuilder::CylRel(const FVector& Origin, float Yaw, const FVector& Local, float Radius, float Length, UMaterialInterface* Material, const FRotator& Extra, bool bCollision)
{
	const FRotator Rot(0.f, Yaw, 0.f);
	return Cyl(Origin + Rot.RotateVector(Local), Radius, Length, Material, bCollision, (Rot.Quaternion() * Extra.Quaternion()).Rotator());
}

void ACHCityBuilder::Ground(float MinX, float MinY, float MaxX, float MaxY, float Z)
{
	BoxAt(MinX, MinY, Z - 40.f, MaxX - MinX, MaxY - MinY, 40.f, MAsphalt);
}

void ACHCityBuilder::StreetX(float X0, float X1, float CenterY, float RoadWidth, float WalkWidth, int32 Seed, bool bTrees)
{
	FRandomStream Rng(Seed);
	const float Length = X1 - X0;

	BoxAt(X0, CenterY - RoadWidth * 0.5f, -20.f, Length, RoadWidth, 20.f, MAsphalt);
	for (float X = X0 + 200.f; X < X1 - 300.f; X += 650.f)
	{
		BoxInstance(FVector(X + 150.f, CenterY, 0.4f), FVector(300.f, 14.f, 0.8f), MLine);
	}

	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const float Edge = CenterY + Side * RoadWidth * 0.5f;
		BoxAt(X0, Side > 0 ? Edge : Edge - 20.f, -20.f, Length, 20.f, 34.f, MCurb);
		BoxAt(X0, Side > 0 ? Edge + 20.f : Edge - 75.f, -60.f, Length, 55.f, 30.f, MJube);
		BoxAt(X0, Side > 0 ? Edge + 75.f : Edge - WalkWidth, -20.f, Length, WalkWidth - 75.f, 34.f, MSidewalk);
		if (bTrees)
		{
			for (float X = X0 + Rng.FRandRange(250.f, 600.f); X < X1 - 250.f; X += Rng.FRandRange(750.f, 1150.f))
			{
				PlaneTree(FVector(X, Side > 0 ? Edge + 47.f : Edge - 47.f, -30.f), Rng.FRandRange(950.f, 1350.f), Rng.RandRange(0, 99999));
			}
		}
	}
}

void ACHCityBuilder::StreetY(float Y0, float Y1, float CenterX, float RoadWidth, float WalkWidth, int32 Seed, bool bTrees)
{
	FRandomStream Rng(Seed);
	const float Length = Y1 - Y0;

	BoxAt(CenterX - RoadWidth * 0.5f, Y0, -20.f, RoadWidth, Length, 20.f, MAsphalt);
	for (float Y = Y0 + 200.f; Y < Y1 - 300.f; Y += 650.f)
	{
		BoxInstance(FVector(CenterX, Y + 150.f, 0.4f), FVector(14.f, 300.f, 0.8f), MLine);
	}

	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const float Edge = CenterX + Side * RoadWidth * 0.5f;
		BoxAt(Side > 0 ? Edge : Edge - 20.f, Y0, -20.f, 20.f, Length, 34.f, MCurb);
		BoxAt(Side > 0 ? Edge + 20.f : Edge - 75.f, Y0, -60.f, 55.f, Length, 30.f, MJube);
		BoxAt(Side > 0 ? Edge + 75.f : Edge - WalkWidth, Y0, -20.f, WalkWidth - 75.f, Length, 34.f, MSidewalk);
		if (bTrees)
		{
			for (float Y = Y0 + Rng.FRandRange(250.f, 600.f); Y < Y1 - 250.f; Y += Rng.FRandRange(750.f, 1150.f))
			{
				PlaneTree(FVector(Side > 0 ? Edge + 47.f : Edge - 47.f, Y, -30.f), Rng.FRandRange(950.f, 1350.f), Rng.RandRange(0, 99999));
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Buildings
// ---------------------------------------------------------------------------------------------

float ACHCityBuilder::Building(float MinX, float MinY, float SizeX, float SizeY, const FCHBuildingStyle& Style)
{
	FRandomStream Rng(Style.Seed * 977 + 3);
	const int32 Floors = FMath::Max(1, Style.Floors);
	const float Height = Style.HeightOverride > 0.f ? Style.HeightOverride : GroundFloorH + (Floors - 1) * FloorH;
	UMaterialInterface* Wall = MFacades.Num() > 0 ? MFacades[Rng.RandRange(0, MFacades.Num() - 1)] : MBand;

	// the mass, the roof and its parapet (walkable)
	BoxAt(MinX, MinY, -20.f, SizeX, SizeY, Height + 20.f, Wall);
	Box(FVector(MinX + SizeX * 0.5f, MinY + SizeY * 0.5f, Height + 1.5f), FVector(SizeX - 2.f * ParapetT, SizeY - 2.f * ParapetT, 3.f), MRoof, false);
	auto ParapetFor = [&Style](uint8 Bit) { return (Style.LowParapets & Bit) ? 45.f : ParapetH; };
	BoxAt(MinX, MinY, Height, SizeX, ParapetT, ParapetFor(4), Wall);
	BoxAt(MinX, MinY + SizeY - ParapetT, Height, SizeX, ParapetT, ParapetFor(8), Wall);
	BoxAt(MinX, MinY + ParapetT, Height, ParapetT, SizeY - 2.f * ParapetT, ParapetFor(1), Wall);
	BoxAt(MinX + SizeX - ParapetT, MinY + ParapetT, Height, ParapetT, SizeY - 2.f * ParapetT, ParapetFor(2), Wall);

	struct FSide
	{
		uint8 Bit;
		FVector Origin;
		float Yaw;
		float Length;
	};
	const FSide Sides[] = {
		{ 8, FVector(MinX, MinY + SizeY, 0.f), 0.f, SizeX },
		{ 4, FVector(MinX + SizeX, MinY, 0.f), 180.f, SizeX },
		{ 2, FVector(MinX + SizeX, MinY + SizeY, 0.f), -90.f, SizeY },
		{ 1, FVector(MinX, MinY, 0.f), 90.f, SizeY },
	};

	FCHBuildingStyle Effective = Style;
	Effective.Floors = Floors;
	for (const FSide& Side : Sides)
	{
		if (Style.Facades & Side.Bit)
		{
			Facade(Side.Origin, Side.Yaw, Side.Length, Effective, (Style.Shops & Side.Bit) != 0, Rng);
		}
	}
	if (Style.bRoofProps)
	{
		RoofProps(MinX + ParapetT, MinY + ParapetT, SizeX - 2.f * ParapetT, SizeY - 2.f * ParapetT, Height, Rng);
	}
	return Height;
}

void ACHCityBuilder::Facade(const FVector& Origin, float Yaw, float Length, const FCHBuildingStyle& Style, bool bShops, FRandomStream& Rng)
{
	const FRotator Rot(0.f, Yaw, 0.f);
	const FVector U = Rot.RotateVector(FVector::ForwardVector);
	const FVector N = Rot.RotateVector(FVector::RightVector);
	auto At = [&](float Along, float Out, float Z) { return Origin + U * Along + N * Out + FVector(0.f, 0.f, Z); };

	const int32 Bays = FMath::Max(1, FMath::FloorToInt32(Length / 290.f));
	const float BayW = Length / Bays;
	const float WinW = FMath::Clamp(BayW - 100.f, 60.f, 150.f);
	const int32 Parity = Rng.RandRange(0, 1);
	UMaterialInterface* BalconyWall = MFacades.Num() > 0 ? MFacades[Rng.RandRange(0, MFacades.Num() - 1)] : MBand;
	UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);

	for (int32 Floor = 0; Floor < Style.Floors; ++Floor)
	{
		const float Z0 = Floor == 0 ? 0.f : GroundFloorH + (Floor - 1) * FloorH;
		const float FH = Floor == 0 ? GroundFloorH : FloorH;
		if (Floor > 0)
		{
			BoxInstance(At(Length * 0.5f, 4.f, Z0), FVector(Length, 8.f, 14.f), MBand, Rot);
		}

		for (int32 Bay = 0; Bay < Bays; ++Bay)
		{
			const float Center = (Bay + 0.5f) * BayW;

			if (Floor == 0)
			{
				if (bShops)
				{
					const float ShopW = BayW - 36.f;
					const bool bOpen = bNight && Rng.FRand() < 0.06f;
					if (bOpen)
					{
						Box(At(Center, 2.f, 125.f), FVector(ShopW, 4.f, 250.f), MShopLit, false, Rot);
						BoxInstance(At(Center, 10.f, 300.f), FVector(ShopW, 20.f, 60.f), MShutter, Rot);
						PointLight(At(Center, 170.f, 210.f), FLinearColor(1.f, 0.72f, 0.42f), 12.f, 600.f, false, 0.5f);
						if (Segment)
						{
							Segment->AddLightPool(At(Center, 220.f, 0.f), 260.f);
						}
					}
					else
					{
						BoxInstance(At(Center, 4.f, 150.f), FVector(ShopW, 8.f, 300.f), MShutter, Rot);
						for (float Z = 14.f; Z < 300.f; Z += 16.f)
						{
							BoxInstance(At(Center, 8.5f, Z), FVector(ShopW, 2.f, 3.f), MRust, Rot);
						}
					}
					BoxInstance(At(Center, 8.f, 342.f), FVector(ShopW + 12.f, 14.f, 62.f), MSigns[Rng.RandRange(0, MSigns.Num() - 1)], Rot);
				}
				else if (Bay == Bays / 2)
				{
					// the building entrance
					BoxInstance(At(Center, 3.f, 118.f), FVector(FMath::Min(130.f, BayW - 40.f), 6.f, 236.f), MFrame, Rot);
					BoxInstance(At(Center, 30.f, 246.f), FVector(FMath::Min(180.f, BayW - 10.f), 60.f, 10.f), MBand, Rot);
				}
				else
				{
					// barred ground floor windows
					BoxInstance(At(Center, 1.5f, 200.f), FVector(WinW, 3.f, 120.f), MGlass, Rot);
					for (float Bar = -WinW * 0.5f + 12.f; Bar < WinW * 0.5f; Bar += 14.f)
					{
						BoxInstance(At(Center + Bar, 6.f, 200.f), FVector(2.f, 2.f, 124.f), MFrame, Rot);
					}
				}
				continue;
			}

			// windows
			const float WinZ = Z0 + FH * 0.5f + 12.f;
			UMaterialInterface* Glass = MGlass;
			if (bNight && Rng.FRand() < Style.LitWindows)
			{
				const float Pick = Rng.FRand();
				Glass = Pick < 0.6f ? MGlassLitWarm : (Pick < 0.85f ? MGlassLitDim : MGlassLitCool);
			}
			BoxInstance(At(Center, 1.5f, WinZ), FVector(WinW, 3.f, 155.f), Glass, Rot);
			BoxInstance(At(Center, 3.5f, WinZ), FVector(5.f, 4.f, 155.f), MFrame, Rot);
			BoxInstance(At(Center, 6.f, WinZ - 82.f), FVector(WinW + 20.f, 12.f, 6.f), MBand, Rot);

			// balconies, with the things people keep on them
			if (Style.bBalconies && Bays > 1 && ((Bay + Floor + Parity) % 2 == 0))
			{
				const float BW = FMath::Min(BayW - 24.f, 260.f);
				BoxInstance(At(Center, 52.f, Z0 + 7.f), FVector(BW, 104.f, 14.f), MBand, Rot);
				BoxInstance(At(Center, 101.f, Z0 + 56.f), FVector(BW, 6.f, 84.f), BalconyWall, Rot);
				BoxInstance(At(Center - BW * 0.5f + 3.f, 52.f, Z0 + 56.f), FVector(6.f, 104.f, 84.f), BalconyWall, Rot);
				BoxInstance(At(Center + BW * 0.5f - 3.f, 52.f, Z0 + 56.f), FVector(6.f, 104.f, 84.f), BalconyWall, Rot);

				const float Pick = Rng.FRand();
				if (Pick < 0.28f)
				{
					// a satellite dish on the rail
					const FVector DishAt = At(Center + BW * 0.3f, 112.f, Z0 + 128.f);
					Ball(DishAt, FVector(58.f, 58.f, 15.f), MDish, false, FRotator(62.f, Yaw + 90.f + Rng.FRandRange(-25.f, 25.f), 0.f));
				}
				else if (Pick < 0.5f)
				{
					BoxInstance(At(Center - BW * 0.22f, 70.f, Z0 + 46.f), FVector(72.f, 50.f, 64.f), MCooler, Rot);
				}
				else if (Pick < 0.66f)
				{
					// laundry on a line
					BoxInstance(At(Center, 80.f, Z0 + 175.f), FVector(BW - 20.f, 1.f, 1.f), MWire, Rot);
					for (int32 Cloth = 0; Cloth < 3; ++Cloth)
					{
						const float Along = Center - BW * 0.3f + Cloth * BW * 0.28f;
						BoxInstance(At(Along, 80.f, Z0 + 150.f), FVector(Rng.FRandRange(30.f, 55.f), 2.f, Rng.FRandRange(40.f, 60.f)), MLaundry[Rng.RandRange(0, MLaundry.Num() - 1)], Rot);
					}
				}
			}
		}
	}
}

void ACHCityBuilder::RoofProps(float MinX, float MinY, float SizeX, float SizeY, float RoofZ, FRandomStream& Rng)
{
	auto Spot = [&](float Margin)
	{
		const float MX = FMath::Min(Margin, SizeX * 0.45f);
		const float MY = FMath::Min(Margin, SizeY * 0.45f);
		return FVector(Rng.FRandRange(MinX + MX, MinX + SizeX - MX), Rng.FRandRange(MinY + MY, MinY + SizeY - MY), RoofZ);
	};
	UMaterialInterface* Concrete = MFacades.Num() > 2 ? MFacades[2] : MBand;

	// the stair head
	if (SizeX > 500.f && SizeY > 500.f)
	{
		const FVector P = Spot(240.f);
		BoxAt(P.X - 140.f, P.Y - 125.f, RoofZ, 280.f, 250.f, 260.f, Concrete);
		Box(P + FVector(141.f, 0.f, 105.f), FVector(4.f, 100.f, 210.f), MRust, false);
	}

	// water tanks
	const int32 Tanks = Rng.RandRange(1, 3);
	for (int32 Index = 0; Index < Tanks; ++Index)
	{
		const FVector P = Spot(130.f);
		const bool bStand = Rng.FRand() < 0.4f;
		float Base = RoofZ;
		if (bStand)
		{
			for (int32 Leg = 0; Leg < 4; ++Leg)
			{
				const FVector LegAt = P + FVector((Leg & 1) ? 48.f : -48.f, (Leg & 2) ? 48.f : -48.f, 45.f);
				Box(LegAt, FVector(6.f, 6.f, 90.f), MRust, false);
			}
			Box(P + FVector(0.f, 0.f, 92.f), FVector(120.f, 120.f, 6.f), MRust, false);
			Base += 95.f;
		}
		const float TankH = Rng.FRandRange(120.f, 160.f);
		Cyl(FVector(P.X, P.Y, Base + TankH * 0.5f), Rng.FRandRange(50.f, 64.f), TankH, Rng.FRand() < 0.6f ? MTank : MTankBlue, true);
	}

	// evaporative coolers with their ducts
	const int32 Coolers = Rng.RandRange(1, 3);
	for (int32 Index = 0; Index < Coolers; ++Index)
	{
		const FVector P = Spot(110.f);
		Box(P + FVector(0.f, 0.f, 50.f), FVector(110.f, 110.f, 100.f), MCooler, true);
		Box(P + FVector(0.f, 0.f, 102.f), FVector(116.f, 116.f, 6.f), MCooler, false);
		Box(P + FVector(0.f, 0.f, 25.f) + FVector((Rng.FRand() < 0.5f) ? 90.f : -90.f, 0.f, 0.f), FVector(80.f, 40.f, 40.f), MMetal, false);
	}

	// satellite dishes
	const int32 Dishes = Rng.RandRange(1, 4);
	for (int32 Index = 0; Index < Dishes; ++Index)
	{
		const FVector P = Spot(70.f);
		Cyl(P + FVector(0.f, 0.f, 45.f), 3.f, 90.f, MPole);
		Ball(P + FVector(0.f, 0.f, 100.f), FVector(88.f, 88.f, 20.f), MDish, false, FRotator(58.f, Rng.FRandRange(160.f, 220.f), 0.f));
	}

	// an old antenna
	if (Rng.FRand() < 0.6f)
	{
		const FVector P = Spot(60.f);
		Cyl(P + FVector(0.f, 0.f, 160.f), 2.5f, 320.f, MPole);
		Box(P + FVector(0.f, 0.f, 280.f), FVector(4.f, 130.f, 4.f), MPole, false);
		Box(P + FVector(0.f, 0.f, 245.f), FVector(4.f, 95.f, 4.f), MPole, false);
	}
}

// ---------------------------------------------------------------------------------------------
// Props
// ---------------------------------------------------------------------------------------------

void ACHCityBuilder::Lamp(const FVector& Base, float Yaw, bool bWorking, bool bFlicker)
{
	Cyl(Base + FVector(0.f, 0.f, 380.f), 8.f, 760.f, MPole, true);
	const FVector Arm = FRotator(0.f, Yaw, 0.f).Vector();
	Box(Base + Arm * 95.f + FVector(0.f, 0.f, 752.f), FVector(190.f, 7.f, 7.f), MPole, false, FRotator(0.f, Yaw, 0.f));
	const FVector Head = Base + Arm * 180.f + FVector(0.f, 0.f, 742.f);
	Box(Head, FVector(66.f, 30.f, 14.f), bWorking ? MLampOn : MLampOff, false, FRotator(0.f, Yaw, 0.f));
	if (!bWorking)
	{
		return;
	}
	USpotLightComponent* Light = SpotLight(Head - FVector(0.f, 0.f, 10.f), FRotator(-90.f, 0.f, 0.f), FLinearColor(1.f, 0.52f, 0.16f), 200.f, 1600.f, 62.f, true, 1.2f);
	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
	{
		Segment->AddLightPool(FVector(Head.X, Head.Y, 0.f), 460.f);
	}
	if (bFlicker && Light)
	{
		FlickerLights.Add(Light);
		FlickerBase.Add(Light->Intensity);
	}
}

void ACHCityBuilder::Wires(const FVector& A, const FVector& B, float Sag)
{
	const int32 Segments = 10;
	FVector Previous = A;
	for (int32 Index = 1; Index <= Segments; ++Index)
	{
		const float T = static_cast<float>(Index) / Segments;
		FVector Point = FMath::Lerp(A, B, T);
		Point.Z -= Sag * 4.f * T * (1.f - T);
		const FVector Delta = Point - Previous;
		BoxInstance((Previous + Point) * 0.5f, FVector(Delta.Size() + 1.f, 2.5f, 2.5f), MWire, Delta.Rotation());
		Previous = Point;
	}
}

void ACHCityBuilder::PlaneTree(const FVector& Base, float Height, int32 Seed)
{
	FRandomStream Rng(Seed);
	const float TrunkH = Height * 0.5f;
	Cyl(Base + FVector(0.f, 0.f, TrunkH * 0.5f), 16.f, TrunkH, MBark, true);
	Cyl(Base + FVector(22.f, 0.f, TrunkH + 110.f), 9.f, 260.f, MBark, false, FRotator(0.f, 0.f, 16.f));
	Cyl(Base + FVector(-14.f, 10.f, TrunkH + 110.f), 9.f, 250.f, MBark, false, FRotator(-14.f, 0.f, -10.f));
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector Offset(Rng.FRandRange(-170.f, 170.f), Rng.FRandRange(-170.f, 170.f), TrunkH + Rng.FRandRange(200.f, Height * 0.48f));
		const float Size = Rng.FRandRange(330.f, 500.f);
		UStaticMeshComponent* Canopy = Ball(Base + Offset, FVector(Size, Size, Size * 0.72f), MLeaves, false);
		Canopy->SetCastShadow(true);
	}
}

void ACHCityBuilder::Car(const FVector& Center, float Yaw, const FLinearColor& Color, bool bBurnt)
{
	UMaterialInterface* Paint = bBurnt ? Lit(FLinearColor(0.02f, 0.018f, 0.016f), 0.95f, 0.8f) : Lit(Color, 0.32f, 0.25f);
	BoxRel(Center, Yaw, FVector(0.f, 0.f, 58.f), FVector(420.f, 172.f, 62.f), Paint, true);
	BoxRel(Center, Yaw, FVector(-25.f, 0.f, 112.f), FVector(220.f, 156.f, 50.f), Paint, true);
	BoxRel(Center, Yaw, FVector(-25.f, 0.f, 112.f), FVector(226.f, 160.f, 36.f), bBurnt ? Paint : MCarGlass, false);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector Wheel((Index & 1) ? 135.f : -135.f, (Index & 2) ? 80.f : -80.f, 31.f);
		CylRel(Center, Yaw, Wheel, 31.f, 22.f, MTyre, FRotator(0.f, 0.f, 90.f));
	}
}

void ACHCityBuilder::Dumpster(const FVector& Center, float Yaw, bool bHidingSpot)
{
	UMaterialInterface* Green = Lit(FLinearColor(0.018f, 0.06f, 0.03f), 0.6f, 0.65f);
	BoxRel(Center, Yaw, FVector(0.f, 0.f, 64.f), FVector(170.f, 105.f, 112.f), Green, true);
	BoxRel(Center, Yaw, FVector(0.f, 6.f, 124.f), FVector(176.f, 114.f, 8.f), Green, false, FRotator(0.f, 0.f, -5.f));
	for (int32 Index = 0; Index < 4; ++Index)
	{
		CylRel(Center, Yaw, FVector((Index & 1) ? 70.f : -70.f, (Index & 2) ? 40.f : -40.f, 7.f), 7.f, 6.f, MTyre, FRotator(0.f, 0.f, 90.f));
	}
	if (bHidingSpot)
	{
		if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
		{
			Segment->AddHidingSpot(Center + FRotator(0.f, Yaw, 0.f).RotateVector(FVector(0.f, -100.f, 0.f)), 75.f);
		}
	}
}

void ACHCityBuilder::Van(const FVector& Center, float Yaw)
{
	UMaterialInterface* White = Lit(FLinearColor(0.60f, 0.60f, 0.58f), 0.35f, 0.25f);
	UMaterialInterface* Green = Lit(FLinearColor(0.02f, 0.15f, 0.06f), 0.4f, 0.25f);
	UMaterialInterface* Bar = Emissive(FLinearColor(0.9f, 0.08f, 0.05f), CHLight::Emissive(1.5f), FLinearColor(0.05f, 0.01f, 0.01f));

	BoxRel(Center, Yaw, FVector(0.f, 0.f, 128.f), FVector(520.f, 200.f, 180.f), White, true);
	BoxRel(Center, Yaw, FVector(0.f, 0.f, 82.f), FVector(524.f, 204.f, 46.f), Green, false);
	BoxRel(Center, Yaw, FVector(0.f, 0.f, 150.f), FVector(524.f, 204.f, 9.f), Green, false);
	BoxRel(Center, Yaw, FVector(-20.f, 0.f, 182.f), FVector(420.f, 205.f, 44.f), MCarGlass, false);
	BoxRel(Center, Yaw, FVector(255.f, 0.f, 178.f), FVector(16.f, 184.f, 58.f), MCarGlass, false, FRotator(-12.f, 0.f, 0.f));
	BoxRel(Center, Yaw, FVector(60.f, 0.f, 225.f), FVector(110.f, 46.f, 13.f), Bar, false);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector Wheel((Index & 1) ? 170.f : -170.f, (Index & 2) ? 92.f : -92.f, 36.f);
		CylRel(Center, Yaw, Wheel, 36.f, 26.f, MTyre, FRotator(0.f, 0.f, 90.f));
	}

	// lettering on both sides
	const FRotator Rot(0.f, Yaw, 0.f);
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const FVector At = Center + Rot.RotateVector(FVector(-10.f, Side * 103.f, 118.f));
		WorldText(TEXT("گشت ارشاد"), true, 40.f, FLinearColor(0.02f, 0.12f, 0.05f), At, FRotator(0.f, Yaw + Side * 90.f, 0.f), 0.9f, false, true);
	}

	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
	{
		Segment->AddVanZone(Center, 950.f);
	}
}

ACHFire* ACHCityBuilder::Fire(const FVector& Base, float Scale, bool bShadows, int32 Seed)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACHFire* FireActor = GetWorld()->SpawnActor<ACHFire>(ACHFire::StaticClass(), FTransform(Base), Params);
	if (FireActor)
	{
		FireActor->Setup(Scale, true, bShadows, Seed);
		if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
		{
			Segment->AddLightPool(Base, 380.f * Scale);
		}
	}
	return FireActor;
}

UWidgetComponent* ACHCityBuilder::Graffiti(const FString& Text, const FVector& Location, float Yaw, float FontSize, const FLinearColor& Color, bool bFarsi)
{
	return WorldText(Text, bFarsi, FontSize, Color, Location, FRotator(0.f, Yaw, 0.f), 1.f, false, true);
}

void ACHCityBuilder::PaintedOver(const FVector& Center, float Yaw, const FVector2D& Size)
{
	// an official mural, faded and half covered with white paint
	const FRotator Rot(0.f, Yaw, 0.f);
	const FVector N = Rot.Vector();
	const FVector Along = FRotator(0.f, Yaw + 90.f, 0.f).Vector();
	UMaterialInterface* Green = Lit(FLinearColor(0.03f, 0.10f, 0.05f), 0.85f, 0.6f);
	UMaterialInterface* White = Lit(FLinearColor(0.42f, 0.41f, 0.38f), 0.9f, 0.5f);
	UMaterialInterface* Faded = Lit(FLinearColor(0.20f, 0.17f, 0.13f), 0.9f, 0.6f);
	Box(Center + N * 1.f, FVector(1.f, Size.X, Size.Y), Green, false, Rot);
	Box(Center + N * 1.5f + FVector(0.f, 0.f, Size.Y * 0.32f), FVector(1.f, Size.X * 0.9f, Size.Y * 0.12f), Faded, false, Rot);
	for (int32 Stroke = 0; Stroke < 5; ++Stroke)
	{
		const float Offset = (Stroke - 2) * Size.X * 0.17f;
		Box(Center + N * 2.f + Along * Offset + FVector(0.f, 0.f, -Size.Y * 0.05f), FVector(1.f, Size.X * 0.12f, Size.Y * 0.22f), Faded, false, Rot);
	}
	// rough white roller strokes over the old words
	Box(Center + N * 2.5f + Along * Size.X * 0.12f + FVector(0.f, 0.f, -Size.Y * 0.05f), FVector(1.f, Size.X * 0.7f, Size.Y * 0.45f), White, false, FRotator(0.f, Yaw, -3.f));
}

void ACHCityBuilder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CityTime += DeltaSeconds;

	for (int32 Index = 0; Index < FlickerLights.Num(); ++Index)
	{
		if (ULocalLightComponent* Light = FlickerLights[Index])
		{
			const float Noise = FMath::PerlinNoise1D(CityTime * 2.7f + Index * 13.1f);
			const float Level = Noise > 0.3f ? 0.05f : (Noise > 0.22f ? 0.5f : 1.f);
			Light->SetIntensity(FlickerBase[Index] * Level);
		}
	}
}
