// Cheshm Hayash - game mode.

#include "Core/CHGameMode.h"
#include "Cheshmhayash.h"
#include "Core/CHPlayerCharacter.h"
#include "Core/CHPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Story/CHStoryDirector.h"
#include "World/CHLevelBuilder.h"
#include "World/CHLocations.h"

ACHGameMode::ACHGameMode()
{
	DefaultPawnClass = ACHPlayerCharacter::StaticClass();
	PlayerControllerClass = ACHPlayerController::StaticClass();
	HUDClass = nullptr;
}

TSubclassOf<ACHLevelBuilder> ACHGameMode::BuilderClassFor(const FString& InLocation)
{
	if (InLocation == TEXT("Room"))
	{
		return ACHLoc_Room::StaticClass();
	}
	if (InLocation == TEXT("Streets"))
	{
		return ACHLoc_Streets::StaticClass();
	}
	if (InLocation == TEXT("FinalRoof"))
	{
		return ACHLoc_FinalRoof::StaticClass();
	}
	if (InLocation == TEXT("PrisonGate"))
	{
		return ACHLoc_PrisonGate::StaticClass();
	}
	if (InLocation == TEXT("Home"))
	{
		return ACHLoc_Home::StaticClass();
	}
	if (InLocation == TEXT("Cemetery"))
	{
		return ACHLoc_Cemetery::StaticClass();
	}
	return ACHLoc_Boot::StaticClass();
}

void ACHGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UWorld* World = GetWorld();
	LocationName = UGameplayStatics::GetCurrentLevelName(World, true);

	FString Variant;
	if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
	{
		Variant = Director->GetPendingVariant();
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Builder = World->SpawnActor<ACHLevelBuilder>(BuilderClassFor(LocationName), FTransform::Identity, Params);
	if (Builder)
	{
		Builder->Build(Variant);

		// The framework spawns the pawn at the first player start it finds.
		World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), Builder->GetPlayerStart(), Params);
	}

	UE_LOG(LogCheshmhayash, Log, TEXT("Location '%s' (variant '%s') built by %s"), *LocationName, *Variant, *GetNameSafe(Builder));
}

void ACHGameMode::StartPlay()
{
	Super::StartPlay();

	if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
	{
		Director->OnLocationReady(Builder, LocationName);
	}
}
