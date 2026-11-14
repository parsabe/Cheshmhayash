// Cheshm Hayash - game mode. Every map is empty: this spawns the level builder that matches the
// map name, lets it build the location in code, then hands the location to the story director.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CHGameMode.generated.h"

class ACHLevelBuilder;

UCLASS()
class ACHGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACHGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;

	ACHLevelBuilder* GetBuilder() const { return Builder; }

	/** Location name -> builder class. */
	static TSubclassOf<ACHLevelBuilder> BuilderClassFor(const FString& LocationName);

private:
	UPROPERTY()
	TObjectPtr<ACHLevelBuilder> Builder;

	FString LocationName;
};
