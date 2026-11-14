// Cheshm Hayash - the Tehran kit: dense 4-6 storey concrete blocks with balconies, shop shutters,
// water tanks, evaporative coolers and satellite dishes on the roofs; streets with jubes (the open
// water channels) and plane trees, sodium lamps, sagging power cables, parked cars, the
// green-white van. Every outdoor location derives from this.

#pragma once

#include "CoreMinimal.h"
#include "World/CHLevelBuilder.h"
#include "CHCity.generated.h"

class ACHFire;
class ULocalLightComponent;

struct FCHBuildingStyle
{
	int32 Floors = 5;
	/** Sides with windows: 1 = -X, 2 = +X, 4 = -Y, 8 = +Y. */
	uint8 Facades = 0xF;
	/** Sides with shops on the ground floor (same bits). */
	uint8 Shops = 0;
	bool bBalconies = true;
	/** Fraction of windows lit at night. */
	float LitWindows = 0.22f;
	bool bRoofProps = true;
	int32 Seed = 0;
};

UCLASS()
class ACHCityBuilder : public ACHLevelBuilder
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** Creates the palette for the location's exposure. Call at the start of Build, after Super::Build. */
	void InitCity(bool bInNight, float ExposureEV);

	// ---- ground and streets
	void Ground(float MinX, float MinY, float MaxX, float MaxY, float Z = 0.f);
	/** A street running along X: asphalt, curbs, jubes, raised sidewalks and plane trees. */
	void StreetX(float X0, float X1, float CenterY, float RoadWidth, float WalkWidth, int32 Seed, bool bTrees = true);
	void StreetY(float Y0, float Y1, float CenterX, float RoadWidth, float WalkWidth, int32 Seed, bool bTrees = true);

	// ---- buildings
	/** Returns the roof height. */
	float Building(float MinX, float MinY, float SizeX, float SizeY, const FCHBuildingStyle& Style);

	// ---- props
	void Lamp(const FVector& Base, float Yaw, bool bWorking, bool bFlicker = false);
	void Wires(const FVector& A, const FVector& B, float Sag = 110.f);
	void PlaneTree(const FVector& Base, float Height, int32 Seed);
	void Car(const FVector& Center, float Yaw, const FLinearColor& Color, bool bBurnt = false);
	/** A municipal bin. Its local -Y side (relative to Yaw) is a hiding spot. */
	void Dumpster(const FVector& Center, float Yaw, bool bHidingSpot = true);
	/** Gasht-e Ershad: the morality police's green and white van. */
	void Van(const FVector& Center, float Yaw);
	ACHFire* Fire(const FVector& Base, float Scale = 1.f, bool bShadows = false, int32 Seed = 0);
	/** Spray paint on a wall whose outward normal points along Yaw. */
	UWidgetComponent* Graffiti(const FString& Text, const FVector& Location, float Yaw, float FontSize, const FLinearColor& Color, bool bFarsi = true);
	/** Old painted-over official mural: a block of faded paint. */
	void PaintedOver(const FVector& Center, float Yaw, const FVector2D& Size);

	/** Pieces of an object placed relative to its origin and yaw. */
	UStaticMeshComponent* BoxRel(const FVector& Origin, float Yaw, const FVector& Local, const FVector& Size, UMaterialInterface* Material, bool bCollision = false, const FRotator& Extra = FRotator::ZeroRotator);
	UStaticMeshComponent* CylRel(const FVector& Origin, float Yaw, const FVector& Local, float Radius, float Length, UMaterialInterface* Material, const FRotator& Extra = FRotator::ZeroRotator, bool bCollision = false);

	/** Night city: dark sky with a cold edge, heavy fog, grain. */
	void NightEnv(float ExposureEV = -1.4f, float FogDensity = 0.035f);

	bool bNight = true;

	// palette (kept alive by the base class material cache)
	UMaterialInterface* MAsphalt = nullptr;
	UMaterialInterface* MLine = nullptr;
	UMaterialInterface* MCurb = nullptr;
	UMaterialInterface* MSidewalk = nullptr;
	UMaterialInterface* MJube = nullptr;
	UMaterialInterface* MGlass = nullptr;
	UMaterialInterface* MGlassLitWarm = nullptr;
	UMaterialInterface* MGlassLitCool = nullptr;
	UMaterialInterface* MGlassLitDim = nullptr;
	UMaterialInterface* MFrame = nullptr;
	UMaterialInterface* MBand = nullptr;
	UMaterialInterface* MMetal = nullptr;
	UMaterialInterface* MShutter = nullptr;
	UMaterialInterface* MRust = nullptr;
	UMaterialInterface* MPole = nullptr;
	UMaterialInterface* MLampOn = nullptr;
	UMaterialInterface* MLampOff = nullptr;
	UMaterialInterface* MTank = nullptr;
	UMaterialInterface* MTankBlue = nullptr;
	UMaterialInterface* MCooler = nullptr;
	UMaterialInterface* MDish = nullptr;
	UMaterialInterface* MBark = nullptr;
	UMaterialInterface* MLeaves = nullptr;
	UMaterialInterface* MTyre = nullptr;
	UMaterialInterface* MCarGlass = nullptr;
	UMaterialInterface* MWire = nullptr;
	UMaterialInterface* MRoof = nullptr;
	UMaterialInterface* MShopLit = nullptr;
	TArray<UMaterialInterface*> MFacades;
	TArray<UMaterialInterface*> MSigns;
	TArray<UMaterialInterface*> MLaundry;

private:
	void Facade(const FVector& Origin, float Yaw, float Length, const FCHBuildingStyle& Style, bool bShops, FRandomStream& Rng);
	void RoofProps(float MinX, float MinY, float SizeX, float SizeY, float RoofZ, FRandomStream& Rng);

	UPROPERTY()
	TArray<TObjectPtr<ULocalLightComponent>> FlickerLights;

	TArray<float> FlickerBase;
	float CityTime = 0.f;
};
