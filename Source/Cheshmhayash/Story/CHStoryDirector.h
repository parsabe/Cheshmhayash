// Cheshm Hayash - the story director.
// Loads the chapter JSON files from Content/Story, runs their beats one after another, owns the UI,
// travels between locations and remembers the player's choices and filmed clips.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "CHStoryDirector.generated.h"

class ACHLevelBuilder;
class ACHPlayerController;
class FJsonObject;
class SCHRootOverlay;

struct FCHScene
{
	FString Id;
	int32 Chapter = 0;
	/** Map / location name ("Room", "Streets"...). Empty = stay where we are. */
	FString Location;
	FString Variant;
	/** "he" (default) or "sister". */
	FString Player;
	TArray<TSharedPtr<FJsonObject>> Beats;
};

UCLASS()
class UCHStoryDirector : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static UCHStoryDirector* Get(const UObject* WorldContextObject);

	// UGameInstanceSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

	// ---- world
	/** Called by the game mode when the location of the freshly loaded map has been built. */
	void OnLocationReady(ACHLevelBuilder* InBuilder, const FString& LocationName);
	const FString& GetPendingVariant() const { return PendingVariant; }
	ACHLevelBuilder* GetBuilder() const { return Builder.Get(); }

	// ---- input (from the player controller)
	void InputAdvance() { bAdvancePressed = true; }
	void InputNav(int32 Direction) { NavInput += Direction; }
	void InputPause();
	void DevSkipScene();
	bool IsPlayerControlEnabled() const { return bPlayerControl && !bTraveling; }

	// ---- choices
	void SetFlag(const FString& Name, const FString& Value);
	FString GetFlag(const FString& Name) const;

	// ---- gameplay segments (started by a "gameplay" beat, finished by the location)
	void FinishSegment();

	TSharedPtr<SCHRootOverlay> GetUI() const { return UI; }

private:
	// story data
	void LoadStory();
	bool LoadStoryFile(const FString& Name);
	int32 FindScene(const FString& Id) const;

	// flow
	void StartStory();
	void StartScene(int32 Index);
	void NextScene();
	void TravelTo(const FString& Location, const FString& Variant);
	void ApplySceneSetup();

	// beats
	bool PassesCondition(const FJsonObject& Beat) const;
	void BeginBeat(const FJsonObject& Beat);
	/** Returns true when the beat is finished. */
	bool UpdateBeat(const FJsonObject& Beat, float DeltaTime);

	void EnsureUI();
	void UpdateFade(float DeltaTime);
	void SetFade(float Target, float Time, const FLinearColor& Color);
	ACHPlayerController* GetController() const;
	bool ConsumeAdvance();

	// developer helpers (command line: -chscene=ID -chauto -chshots=3,8,15 -chquit)
	void ParseDevOptions();
	void DevTick();

	TArray<FCHScene> Scenes;
	TMap<FString, FString> Flags;

	TSharedPtr<SCHRootOverlay> UI;
	TWeakObjectPtr<ACHLevelBuilder> Builder;

	FString CurrentLocation;
	FString CurrentVariant;
	FString PendingVariant;
	FString TravelTarget;

	int32 SceneIndex = INDEX_NONE;
	int32 BeatIndex = 0;
	bool bBeatStarted = false;
	float BeatTime = 0.f;
	int32 BeatPhase = 0;

	bool bStarted = false;
	bool bTraveling = false;
	bool bPlayerControl = false;
	bool bSegmentDone = false;

	bool bAdvancePressed = false;
	int32 NavInput = 0;

	float FadeAlpha = 1.f;
	float FadeTarget = 1.f;
	float FadeRate = 0.f;
	FLinearColor FadeColor = FLinearColor::Black;

	// dev
	FString DevStartScene;
	bool bDevAuto = false;
	bool bDevQuit = false;
	bool bDevKeys = false;
	TArray<float> DevShotTimes;
	int32 DevNextShot = 0;
	double DevStartSeconds = 0.0;
	double DevLastShotSeconds = 0.0;
};
