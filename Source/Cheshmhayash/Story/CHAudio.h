// Cheshm Hayash - sound. One music track and one ambience at a time, cross-faded; a heartbeat
// that takes over near the morality police van; fast percussion when he is spotted; the soft
// key click that stands in for every voice. All sounds are /Game/Audio/<name> sound waves
// (generated, never recorded). A missing sound is simply silent.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CHAudio.generated.h"

class UAudioComponent;
class USoundWave;

UCLASS()
class UCHAudio : public UObject
{
	GENERATED_BODY()

public:
	/** Empty name (or "none") fades the music out. */
	void SetMusic(const UObject* WorldContext, const FString& Name, float Fade = 2.5f, float Volume = 0.7f);
	void SetAmbient(const UObject* WorldContext, const FString& Name, float Fade = 2.f, float Volume = 0.6f);
	/** The sound cuts: everything stops at once. */
	void Cut(float Fade = 0.04f);
	void PlaySfx(const UObject* WorldContext, const FString& Name, float Volume = 1.f);
	void TypeTick(const UObject* WorldContext, bool bPhone);
	/** Stealth layers. bNearVan: only a heartbeat. bChased: fast percussion over a ducked score. */
	void Update(const UObject* WorldContext, bool bNearVan, float Suspicion, bool bChased);

private:
	USoundWave* Find(const FString& Name);
	UAudioComponent* Start(const UObject* WorldContext, const FString& Name, float Volume, float Fade);
	void Retire(TObjectPtr<UAudioComponent>& Component, float Fade);

	UPROPERTY()
	TMap<FString, TObjectPtr<USoundWave>> Waves;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Music;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Ambient;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Heart;

	UPROPERTY()
	TObjectPtr<UAudioComponent> SuspicionLayer;

	UPROPERTY()
	TObjectPtr<UAudioComponent> Chase;

	bool bWasNearVan = false;
	bool bWasChased = false;

	/** Fading out; kept alive until they stop. */
	UPROPERTY()
	TArray<TObjectPtr<UAudioComponent>> Retired;

	FString MusicName;
	FString AmbientName;
	float MusicVolume = 0.7f;
	float MusicDuck = 1.f;
	double LastTypeSeconds = 0.0;
	int32 TypeIndex = 0;
};
