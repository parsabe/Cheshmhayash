// Cheshm Hayash - the checkpoint save (like a console game: autosaved at every scene and before every
// gameplay segment) and the player's settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CHSave.generated.h"

UCLASS()
class UCHSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 Chapter = 1;

	UPROPERTY()
	FString SceneId;

	/** Beat to resume at (a gameplay beat), 0 = the scene's beginning. */
	UPROPERTY()
	int32 BeatIndex = 0;

	UPROPERTY()
	TMap<FString, FString> Flags;

	UPROPERTY()
	TArray<FString> ClipIds;

	UPROPERTY()
	TArray<FString> ClipCaptionsFa;

	UPROPERTY()
	TArray<FString> ClipCaptionsEn;

	UPROPERTY()
	TArray<int32> ClipChapters;

	UPROPERTY()
	int32 Molotovs = 0;

	UPROPERTY()
	bool bRec = false;

	UPROPERTY()
	float RecSeconds = 0.f;
};

namespace CHSave
{
	bool Exists();
	UCHSaveGame* Load();
	bool Write(UCHSaveGame* Data);
	void Delete();
}

/** Player settings, kept in GameUserSettings.ini. */
namespace CHSettings
{
	float GetSensitivity();
	bool GetInvertY();
	/** 0..1 */
	float GetVolume();
	void SetSensitivity(float Value);
	void SetInvertY(bool bValue);
	void SetVolume(float Value);
	/** Pushes the saved volume to the audio engine. */
	void ApplyVolume();
}
