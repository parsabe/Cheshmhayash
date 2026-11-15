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

/** Active display language. Only the single chosen language is shown on screen. */
enum class ECHLanguage : uint8
{
	English = 0,
	German = 1,
	Persian = 2,
};

/** Player settings, kept in GameUserSettings.ini. */
namespace CHSettings
{
	ECHLanguage GetLanguage();
	void SetLanguage(ECHLanguage Lang);
	bool HasLanguageSetting();
	bool IsPersian();
	bool IsGerman();
	bool IsEnglish();

	/** Registers an English -> German pair (from JSON "en"/"de" fields or C++ tables). */
	void RegisterGerman(const FString& En, const FString& De);
	/** Returns En when English is active, or the German translation when German is active. */
	FString Latin(const FString& En, const FString& De = FString());
	/** Returns Fa when Persian is active, De when German is active, or En when English is active. */
	FString Pick(const FString& Fa, const FString& En, const FString& De = FString());

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
