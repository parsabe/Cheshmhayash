// Cheshm Hayash - a fire in the street: flickering flames, warm light, a column of smoke.
// A small CPU "particle" system of camera-facing soft sprites (no Niagara assets needed).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/RandomStream.h"
#include "CHFire.generated.h"

class UMaterialInstanceDynamic;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS()
class ACHFire : public AActor
{
	GENERATED_BODY()

public:
	ACHFire();

	virtual void Tick(float DeltaSeconds) override;

	/** Scale 1 = a burning bin, 2.5 = a barricade of tyres. */
	void Setup(float InScale, bool bInSmoke = true, bool bShadows = false, int32 Seed = 0);
	void SetBurning(bool bInBurning);

	UPointLightComponent* GetLight() const { return Light; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Fire")
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Flames;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Puffs;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> PuffMaterials;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Halo;

	TArray<float> PuffAge;
	TArray<float> PuffLife;
	TArray<FVector> PuffDrift;

	FRandomStream Rng;
	float Scale = 1.f;
	float Time = 0.f;
	float BaseIntensity = 60.f;
	bool bBurning = true;
	bool bSmoke = true;
};
