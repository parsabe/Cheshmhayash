// Cheshm Hayash - the story director.
// Loads the chapter JSON files from Content/Story, runs their beats one after another, owns the UI,
// travels between locations and remembers the player's choices and filmed clips.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "CHStoryDirector.generated.h"

class ACHLevelBuilder;
class ACHNpc;
class ACHPlayerController;
class FJsonObject;
class SCHRootOverlay;
class UCHAudio;
class UTexture;
class UTextureRenderTarget2D;

struct FCHScene
{
	FString Id;
	int32 Chapter = 0;
	/** Map / location name ("Room", "Streets"...). Empty = stay where we are. */
	FString Location;
	FString Variant;
	/** "he" (default) or "sister". */
	FString Player;
	/** Score and ambience for the scene (names of /Game/Audio sounds; empty = none). */
	FString Music;
	FString Ambient;
	/** An encounter played inside another scene: it leaves the sound alone. */
	bool bInterlude = false;
	/** Developer-only scene: reachable with -chscene, skipped in the story. */
	bool bDev = false;
	TArray<TSharedPtr<FJsonObject>> Beats;
};

/** One filmed moment: what he saw, kept for the director's cut. */
USTRUCT()
struct FCHClip
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	FString CaptionFa;

	UPROPERTY()
	FString CaptionEn;

	UPROPERTY()
	int32 Chapter = 0;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> Image;
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
	virtual bool IsTickableWhenPaused() const override { return true; }
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
	/** Number keys pick a choice directly (0-based). */
	void InputChoice(int32 Index) { DirectChoice = Index; }
	void InputPause();
	void DevSkipScene();
	bool IsPlayerControlEnabled() const { return bPlayerControl && !bTraveling && !bPauseMenu; }
	/** false in segments where he may film or use things but not walk (the final rooftop). */
	bool IsMoveAllowed() const { return bMoveAllowed; }

	/** Runs a short scene (an encounter: text, a choice) inside the running gameplay segment, then returns to it. */
	void PlayInterlude(const FString& SceneId);
	/** Plays /Game/Audio/<Name> once. */
	void PlaySfx(const FString& Name, float Volume = 1.f);
	bool IsInInterlude() const { return bInInterlude; }

	// ---- choices
	void SetFlag(const FString& Name, const FString& Value);
	FString GetFlag(const FString& Name) const;

	// ---- gameplay segments (started by a "gameplay" beat, finished by the location)
	void FinishSegment();
	/** Second chances for the running segment (base + earned through choices). */
	int32 GetSegmentChances() const { return SegmentChances; }

	// ---- screen
	void Fade(float Target, float Time, const FLinearColor& Color) { SetFade(Target, Time, Color); }
	void Flash(const FLinearColor& Color, float Seconds);
	void Toast(const FString& Fa, const FString& En, const FLinearColor& Color, float Seconds = 2.8f, UTexture* Image = nullptr);

	// ---- clips
	/** Captures what the player's camera sees right now as a filmed clip. */
	UTexture* CaptureClip(const FString& Id, const FString& CaptionFa, const FString& CaptionEn);
	const TArray<FCHClip>& GetClips() const { return Clips; }

	TSharedPtr<SCHRootOverlay> GetUI() const { return UI; }
	bool IsDevBot() const { return bDevBot; }
	/** -chghost: guards never see the player (to test that a level can be finished). */
	bool IsDevGhost() const { return bDevGhost; }

private:
	// story data
	void LoadStory();
	bool LoadStoryFile(const FString& Name);
	int32 FindScene(const FString& Id) const;

	// flow
	void StartStory();
	void StartScene(int32 Index);
	void NextScene();
	void EndInterlude();
	void TravelTo(const FString& Location, const FString& Variant);
	void ApplySceneSetup();

	// beats
	bool PassesCondition(const FJsonObject& Beat) const;
	bool PassesCondition(const FString& Condition) const;
	void BeginBeat(const FJsonObject& Beat);
	/** Returns true when the beat is finished. */
	bool UpdateBeat(const FJsonObject& Beat, float DeltaTime);
	void BeginPhoneBeat(const FJsonObject& Beat);
	bool UpdatePhoneBeat(const FJsonObject& Beat, float DeltaTime);
	/** Shared by "text" beats and spoken choices: handles skip / advance. True when the line is done. */
	bool UpdateDialogueLine(const FJsonObject& Beat);
	/** Type of the next beat that will actually run in this scene. */
	FString PeekNextType() const;
	/** A letter was typed in the dialogue (or on the phone). */
	void OnTypeTick(bool bPhone);

	void EnsureUI();
	void SetPaused(bool bInPaused);
	void UpdatePause();
	/** Clears every overlay: cards, text, phone, letter, montage. */
	void ClearScreen();
	bool bPauseMenu = false;
	float DevPauseAt = 0.f;
	void UpdateFade(float DeltaTime);
	void SetFade(float Target, float Time, const FLinearColor& Color);
	ACHPlayerController* GetController() const;
	bool ConsumeAdvance();

	// developer helpers (command line: -chscene=ID -chauto -chbot -chpick=N -chshots=3,8,15 -chquit)
	void ParseDevOptions();
	void DevTick();

	TArray<FCHScene> Scenes;
	TMap<FString, FString> Flags;

	UPROPERTY()
	TArray<FCHClip> Clips;

	UPROPERTY()
	TObjectPtr<UCHAudio> Audio;

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
	bool bMoveAllowed = true;
	int32 SegmentChances = 0;

	bool bInInterlude = false;
	int32 ReturnScene = INDEX_NONE;
	int32 ReturnBeat = 0;
	bool bReturnControl = false;

	bool bAdvancePressed = false;
	int32 NavInput = 0;
	int32 DirectChoice = INDEX_NONE;

	/** Time spent in the current phase of a beat, and time spent waiting after its animation ended. */
	float PhaseTime = 0.f;
	float IdleTime = 0.f;
	TWeakObjectPtr<ACHNpc> WaitNpc;

	float FadeAlpha = 1.f;
	float FadeTarget = 1.f;
	float FadeRate = 0.f;
	FLinearColor FadeColor = FLinearColor::Black;

	/** The REC dot: on from the first rooftop until his last morning. */
	bool bRec = false;
	float RecSeconds = 0.f;
	bool bControlsShown = false;

	// dev
	FString DevStartScene;
	int32 DevPick = 0;
	bool bDevAuto = false;
	bool bDevBot = false;
	bool bDevGhost = false;
	bool bDevQuit = false;
	bool bDevKeys = false;
	TArray<float> DevShotTimes;
	TArray<FString> DevInterludes;
	int32 DevNextShot = 0;
	double DevStartSeconds = 0.0;
	double DevLastShotSeconds = 0.0;
};
