// Cheshm Hayash - full screen movie.

#include "Story/CHVideo.h"
#include "Cheshmhayash.h"
#include "Engine/World.h"
#include "FileMediaSource.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#include "Misc/Paths.h"
#include "UI/CHStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SOverlay.h"

bool UCHVideo::Play(UWorld* World, const FString& File)
{
	Stop();
	const FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Movies") / File);
	if (!IFileManager::Get().FileExists(*Path) || !World)
	{
		UE_LOG(LogCheshmhayash, Warning, TEXT("Video: %s not found"), *Path);
		bFinished = true;
		return false;
	}

	Player = NewObject<UMediaPlayer>(this);
	Player->PlayOnOpen = true;
	Player->SetLooping(false);

	Texture = NewObject<UMediaTexture>(this);
	Texture->AutoClear = true;
	Texture->ClearColor = FLinearColor::Black;
	Texture->SetMediaPlayer(Player);
	Texture->UpdateResource();

	// the sound of the movie, played as a 2D sound
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	SoundHost = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
	if (SoundHost)
	{
		Sound = NewObject<UMediaSoundComponent>(SoundHost);
		Sound->bIsUISound = true;
		Sound->bAllowSpatialization = false;
		Sound->SetMediaPlayer(Player);
		Sound->RegisterComponent();
		Sound->Start();
	}

	Source = NewObject<UFileMediaSource>(this);
	Source->SetFilePath(Path);
	const bool bOpened = Player->OpenSource(Source);
	UE_LOG(LogCheshmhayash, Log, TEXT("Video: %s %s"), *Path, bOpened ? TEXT("opening") : TEXT("FAILED to open"));

	Brush = MakeShared<FSlateBrush>();
	Brush->SetResourceObject(Texture);
	Brush->ImageSize = FVector2D(1920.0, 1080.0);

	bActive = bOpened;
	bFinished = !bOpened;
	bStarted = false;
	Elapsed = 0.f;
	Alpha = 0.f;
	return bOpened;
}

void UCHVideo::Stop()
{
	if (Player)
	{
		Player->Close();
	}
	if (Sound)
	{
		Sound->Stop();
		Sound->SetMediaPlayer(nullptr);
	}
	if (SoundHost)
	{
		SoundHost->Destroy();
	}
	Sound = nullptr;
	SoundHost = nullptr;
	bActive = false;
}

float UCHVideo::GetTime() const
{
	return Player ? static_cast<float>(Player->GetTime().GetTotalSeconds()) : 0.f;
}

float UCHVideo::GetDuration() const
{
	return Player ? static_cast<float>(Player->GetDuration().GetTotalSeconds()) : 0.f;
}

void UCHVideo::Update(float DeltaTime)
{
	if (!bActive || !Player)
	{
		Alpha = FMath::Max(0.f, Alpha - DeltaTime);
		return;
	}
	Elapsed += DeltaTime;
	Alpha = FMath::Min(1.f, Alpha + DeltaTime / 1.5f);
	if (Player->IsPlaying())
	{
		bStarted = true;
	}
	const float Duration = GetDuration();
	const bool bAtEnd = bStarted && ((!Player->IsPlaying() && !Player->IsPreparing()) || (Duration > 1.f && GetTime() >= Duration - 0.05f));
	// a file that never starts must not hold the story forever
	const bool bStuck = !bStarted && Elapsed > 15.f;
	if (bAtEnd || bStuck)
	{
		if (bStuck)
		{
			UE_LOG(LogCheshmhayash, Warning, TEXT("Video: did not start, skipping"));
		}
		bFinished = true;
		Stop();
	}
}

TSharedRef<SWidget> UCHVideo::MakeWidget()
{
	return SNew(SOverlay)
		.Visibility(EVisibility::HitTestInvisible)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(CH::WhiteBrush())
			.ColorAndOpacity_Lambda([this]() { return FSlateColor(FLinearColor(0.f, 0.f, 0.f, Alpha)); })
		]
		+ SOverlay::Slot()
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				SNew(SImage)
				.Image_Lambda([this]() { return Brush.IsValid() ? Brush.Get() : CH::WhiteBrush(); })
				.ColorAndOpacity_Lambda([this]() { return FSlateColor(FLinearColor(1.f, 1.f, 1.f, Brush.IsValid() && bStarted ? Alpha : 0.f)); })
			]
		];
}
