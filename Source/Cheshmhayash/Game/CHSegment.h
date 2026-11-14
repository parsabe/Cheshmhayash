// Cheshm Hayash - one gameplay segment in a location: stealth, filming, using things.
// Knows the guards, the moments worth filming, light pools, hiding spots, exits and checkpoints;
// works out how visible he is, handles being caught (a second chance, or back to the checkpoint)
// and feeds the HUD.

#pragma once

#include "CoreMinimal.h"
#include "Game/CHNavGrid.h"
#include "Subsystems/WorldSubsystem.h"
#include "CHSegment.generated.h"

class ACHFilmable;
class ACHGuard;
class ACHInteractable;
class ACHLevelBuilder;
class APawn;

struct FCHZone
{
	FVector Center = FVector::ZeroVector;
	float Radius = 0.f;
};

struct FCHExit
{
	FBox Box;
	FString Id;
};

struct FCHCheckpoint
{
	FBox Box;
	FTransform Respawn;
	bool bReached = false;
};

UCLASS()
class UCHSegmentSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UCHSegmentSubsystem* Get(const UObject* WorldContextObject);

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// ---- set up by the location while it builds
	/** The walkability grid is rasterised a couple of frames later, once the new geometry is in the physics scene. */
	void RequestGrid(const FBox2D& Bounds, float CellSize = 50.f, float FloorZ = 0.f);
	const FCHNavGrid& GetGrid() const { return Grid; }
	void AddGuard(ACHGuard* Guard);
	void AddFilmable(ACHFilmable* Filmable);
	void AddInteractable(ACHInteractable* Interactable);
	void AddLightPool(const FVector& Center, float Radius);
	void AddHidingSpot(const FVector& Center, float Radius);
	/** Inside a crowd he is much harder to pick out. */
	void AddCrowdZone(const FVector& Center, float Radius);
	/** Near a morality police van: the music stops, only a heartbeat. */
	void AddVanZone(const FVector& Center, float Radius);
	void AddExit(const FBox& Box, const FString& Id);
	void AddCheckpoint(const FBox& Box, const FTransform& Respawn);
	const TArray<TWeakObjectPtr<ACHGuard>>& GetGuards() const { return Guards; }

	// ---- the segment
	void Begin(ACHLevelBuilder* InBuilder, const FTransform& InRespawn, int32 InSecondChances);
	void End();
	bool IsActive() const { return bActive; }
	void SetHint(const FString& Fa, const FString& En);
	void SetRespawn(const FTransform& InRespawn) { Respawn = InRespawn; }

	// ---- the player
	APawn* GetPlayerPawn() const;
	float GetPlayerVisibility() const { return PlayerVisibility; }
	bool IsPlayerUntouchable() const { return bDevGhost || bSuspended || GraceTime > 0.f || RestartTime > 0.f; }
	/** A story interlude is playing: nobody sees him, nothing can be filmed or used. */
	void SetSuspended(bool bInSuspended);
	bool IsSuspended() const { return bSuspended; }
	bool IsInputBlocked() const { return RestartTime > 0.f; }
	bool IsInVanZone() const { return bInVanZone; }
	bool IsHidden() const { return bHidden; }
	bool IsFilming() const { return bFilming; }
	bool AnyChasing() const { return bAnyChasing; }
	float GetMaxSuspicion() const { return MaxSuspicion; }
	int32 GetSecondChances() const { return SecondChances; }
	void SetFilmHeld(bool bHeld) { bFilmHeld = bHeld; }
	void Interact();
	void OnCaught(ACHGuard* Guard);

	/** Filming is allowed in this segment. */
	bool bFilmAllowed = true;

private:
	void UpdatePlayer(float DeltaTime, APawn* Pawn);
	void UpdateDevBot(float DeltaTime, APawn* Pawn);
	void UpdateFilming(float DeltaTime, APawn* Pawn);
	void UpdateInteraction(APawn* Pawn);
	void UpdateZones(APawn* Pawn);
	void UpdateRestart(float DeltaTime);
	void ResetToCheckpoint();
	void PushHud();

	FCHNavGrid Grid;
	FBox2D GridBounds = FBox2D(ForceInit);
	float GridCell = 50.f;
	float GridFloor = 0.f;
	int32 GridDelayFrames = 0;
	bool bGridRequested = false;

	TArray<TWeakObjectPtr<ACHGuard>> Guards;
	TArray<TWeakObjectPtr<ACHFilmable>> Filmables;
	TArray<TWeakObjectPtr<ACHInteractable>> Interactables;
	TArray<FCHZone> LightPools;
	TArray<FCHZone> HidingSpots;
	TArray<FCHZone> CrowdZones;
	TArray<FCHZone> VanZones;
	TArray<FCHExit> Exits;
	TArray<FCHCheckpoint> Checkpoints;

	TWeakObjectPtr<ACHLevelBuilder> Builder;
	FTransform Respawn;
	FString HintFa;
	FString HintEn;
	int32 SecondChances = 0;
	bool bActive = false;
	bool bSuspended = false;

	float PlayerVisibility = 1.f;
	bool bHidden = false;
	bool bInLight = false;
	bool bInVanZone = false;
	float NoiseTimer = 0.f;
	float GraceTime = 0.f;
	float RestartTime = 0.f;
	bool bRestartDone = false;
	float MaxSuspicion = 0.f;
	bool bAnyChasing = false;

	bool bFilmHeld = false;
	bool bFilming = false;
	TWeakObjectPtr<ACHFilmable> FilmTarget;
	float FilmProgress = 0.f;
	TWeakObjectPtr<ACHInteractable> PromptTarget;

	// developer autopilot (-chbot): walks to the exit and films what it passes
	bool bDevBot = false;
	bool bDevGhost = false;
	bool bBotFilm = false;
	TArray<FVector> BotPath;
	int32 BotIndex = 0;
	float BotRepath = 0.f;
	float BotUseTime = 0.f;
	float BotStuckTime = 0.f;
	TSet<FString> UsedExits;
};
