// Cheshm Hayash - sound.

#include "Story/CHAudio.h"
#include "Cheshmhayash.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"

USoundWave* UCHAudio::Find(const FString& Name)
{
	if (Name.IsEmpty() || Name == TEXT("none"))
	{
		return nullptr;
	}
	if (const TObjectPtr<USoundWave>* Known = Waves.Find(Name))
	{
		return Known->Get();
	}
	USoundWave* Wave = LoadObject<USoundWave>(nullptr, *FString::Printf(TEXT("/Game/Audio/%s.%s"), *Name, *Name), nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (Wave)
	{
		Wave->bLooping = Name.StartsWith(TEXT("music_")) || Name.StartsWith(TEXT("amb_")) || Name.StartsWith(TEXT("loop_"));
	}
	else
	{
		UE_LOG(LogCheshmhayash, Log, TEXT("Audio: no sound '%s' (run Scripts\\gen_audio.bat, then Scripts\\setup_assets.bat)"), *Name);
	}
	Waves.Add(Name, Wave);
	return Wave;
}

UAudioComponent* UCHAudio::Start(const UObject* WorldContext, const FString& Name, float Volume, float Fade)
{
	USoundWave* Wave = Find(Name);
	if (!Wave || !WorldContext)
	{
		return nullptr;
	}
	// 2D, survives map changes: the score carries across cuts between locations
	UAudioComponent* Component = UGameplayStatics::CreateSound2D(WorldContext, Wave, 1.f, 1.f, 0.f, nullptr, true, false);
	if (Component)
	{
		Component->bIsUISound = true;
		if (Fade > 0.f)
		{
			Component->FadeIn(Fade, Volume);
		}
		else
		{
			Component->SetVolumeMultiplier(Volume);
			Component->Play();
		}
	}
	return Component;
}

void UCHAudio::Retire(TObjectPtr<UAudioComponent>& Component, float Fade)
{
	if (Component)
	{
		if (Fade > 0.f)
		{
			Component->FadeOut(Fade, 0.f);
			Retired.Add(Component);
		}
		else
		{
			Component->Stop();
		}
		Component = nullptr;
	}
}

void UCHAudio::SetMusic(const UObject* WorldContext, const FString& Name, float Fade, float Volume)
{
	const FString Wanted = Name == TEXT("none") ? FString() : Name;
	if (Wanted == MusicName && (Music || Wanted.IsEmpty()))
	{
		return;
	}
	Retire(Music, Fade);
	MusicName = Wanted;
	MusicVolume = Volume;
	MusicDuck = 1.f;
	Music = Start(WorldContext, Wanted, Volume, Fade);
}

void UCHAudio::SetAmbient(const UObject* WorldContext, const FString& Name, float Fade, float Volume)
{
	const FString Wanted = Name == TEXT("none") ? FString() : Name;
	if (Wanted == AmbientName && (Ambient || Wanted.IsEmpty()))
	{
		return;
	}
	Retire(Ambient, Fade);
	AmbientName = Wanted;
	Ambient = Start(WorldContext, Wanted, Volume, Fade);
}

void UCHAudio::Cut(float Fade)
{
	Retire(Music, Fade);
	Retire(Ambient, Fade);
	Retire(Heart, Fade);
	Retire(SuspicionLayer, Fade);
	Retire(Chase, Fade);
	bWasNearVan = false;
	bWasChased = false;
	MusicName.Reset();
	AmbientName.Reset();
}

void UCHAudio::PlaySfx(const UObject* WorldContext, const FString& Name, float Volume)
{
	if (USoundWave* Wave = Find(Name))
	{
		if (WorldContext)
		{
			UGameplayStatics::PlaySound2D(WorldContext, Wave, Volume);
		}
	}
}

void UCHAudio::TypeTick(const UObject* WorldContext, bool bPhone)
{
	// not every letter: a soft patter, never a rattle
	const double Now = FPlatformTime::Seconds();
	if (Now - LastTypeSeconds < 0.045)
	{
		return;
	}
	LastTypeSeconds = Now;
	TypeIndex = (TypeIndex + 1 + FMath::RandRange(0, 1)) % 3;
	PlaySfx(WorldContext, FString::Printf(TEXT("sfx_type%d"), TypeIndex + 1), bPhone ? 0.35f : 0.5f);
}

void UCHAudio::Update(const UObject* WorldContext, bool bNearVan, float Suspicion, bool bChased)
{
	for (int32 Index = Retired.Num() - 1; Index >= 0; --Index)
	{
		if (!Retired[Index] || !Retired[Index]->IsPlaying())
		{
			Retired.RemoveAtSwap(Index);
		}
	}

	// stingers on entering a van zone or being spotted
	if (bNearVan && !bWasNearVan)
	{
		PlaySfx(WorldContext, TEXT("sfx_sting_van"), 0.75f);
	}
	if (bChased && !bWasChased)
	{
		PlaySfx(WorldContext, TEXT("sfx_sting_spotted"), 0.85f);
	}
	bWasNearVan = bNearVan;
	bWasChased = bChased;

	// near the van the score stops; only his heart
	if (bNearVan && !Heart)
	{
		Heart = Start(WorldContext, TEXT("loop_heartbeat"), 0.9f, 0.6f);
	}
	else if (!bNearVan && Heart)
	{
		Retire(Heart, 1.2f);
	}

	// suspicion: ticking pulse + dissonant strings before he is fully seen
	const bool bWantSuspicion = Suspicion > 0.12f && !bChased && !bNearVan;
	if (bWantSuspicion && !SuspicionLayer)
	{
		SuspicionLayer = Start(WorldContext, TEXT("music_suspicion"), 0.65f, 0.5f);
	}
	else if (!bWantSuspicion && SuspicionLayer)
	{
		Retire(SuspicionLayer, 1.4f);
	}

	// spotted / fight: heavy taiko drums and rapid daf
	const bool bWantChase = bChased && !bNearVan;
	if (bWantChase && !Chase)
	{
		Chase = Start(WorldContext, TEXT("music_chase"), 0.85f, 0.35f);
	}
	else if (!bWantChase && Chase)
	{
		Retire(Chase, 2.f);
	}

	const float Duck = bNearVan ? 0.f : (bWantChase ? 0.25f : (bWantSuspicion ? 0.55f : 1.f));
	if (!FMath::IsNearlyEqual(Duck, MusicDuck))
	{
		MusicDuck = Duck;
		if (Music)
		{
			Music->AdjustVolume(0.8f, FMath::Max(0.001f, MusicVolume * Duck));
		}
	}
}
