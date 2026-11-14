// Cheshm Hayash - Boot: a black void behind the cards.

#include "World/CHLocations.h"

void ACHLoc_Boot::Build(const FString& InVariant)
{
	Super::Build(InVariant);

	FCHEnv Env;
	Env.bAtmosphere = false;
	Env.bFog = false;
	Env.SunIntensity = 0.f;
	Env.bSunLightsWorld = false;
	Env.SkyLightIntensity = 0.f;
	Env.ExposureEV = 4.f;
	Env.Grain = 0.f;
	Env.Bloom = 0.f;
	ApplyEnv(Env);

	Mark(TEXT("PlayerStart"), FVector(0.f, 0.f, 100.f));
}
