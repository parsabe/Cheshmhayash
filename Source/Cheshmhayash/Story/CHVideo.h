// Cheshm Hayash - plays a movie file full screen (with its sound): the closing video after the grave.
// Files live in Content/Movies and are staged as loose files.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CHVideo.generated.h"

class AActor;
class SWidget;
class UFileMediaSource;
class UMediaPlayer;
class UMediaSoundComponent;
class UMediaTexture;
struct FSlateBrush;

UCLASS()
class UCHVideo : public UObject
{
	GENERATED_BODY()

public:
	/** Starts Content/Movies/<File>. Returns false if the file is missing. */
	bool Play(UWorld* World, const FString& File);
	void Stop();
	/** Seconds into the movie (0 before it starts). */
	float GetTime() const;
	float GetDuration() const;
	bool IsFinished() const { return bFinished; }
	bool IsActive() const { return bActive; }
	/** Call every frame. */
	void Update(float DeltaTime);
	/** The full-screen picture, letterboxed on black. */
	TSharedRef<SWidget> MakeWidget();

private:
	UPROPERTY()
	TObjectPtr<UMediaPlayer> Player;

	UPROPERTY()
	TObjectPtr<UFileMediaSource> Source;

	UPROPERTY()
	TObjectPtr<UMediaTexture> Texture;

	UPROPERTY()
	TObjectPtr<UMediaSoundComponent> Sound;

	UPROPERTY()
	TObjectPtr<AActor> SoundHost;

	TSharedPtr<FSlateBrush> Brush;
	float Alpha = 0.f;
	float Elapsed = 0.f;
	bool bActive = false;
	bool bStarted = false;
	bool bFinished = false;
};
