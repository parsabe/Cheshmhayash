// Cheshm Hayash - the locations of the story. One class per map; each builds itself in code.

#pragma once

#include "CoreMinimal.h"
#include "World/CHLevelBuilder.h"
#include "CHLocations.generated.h"

/** Black void behind the opening cards, the title and the ending cards. */
UCLASS()
class ACHLoc_Boot : public ACHLevelBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
};

/** His room (1.1, 1.9, 3.2). Variants: "morning", "night", "after". */
UCLASS()
class ACHLoc_Room : public ACHLevelBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) override;

private:
	void BuildShell();
	void BuildFurniture();

	UPROPERTY()
	TObjectPtr<USceneComponent> DoorPivot;

	float DoorAngle = 0.f;
	float DoorTarget = 0.f;
};
