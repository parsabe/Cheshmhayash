// Cheshm Hayash - the director's cut. Every clip he filmed, in order, framed like a phone video
// with its caption. The last one - the sunrise - freezes.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UTexture;
struct FSlateBrush;

struct FCHMontageClip
{
	TWeakObjectPtr<UTexture> Image;
	FString Fa;
	FString En;
	FString Stamp;
};

class SCHMontage : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHMontage) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Play(const TArray<FCHMontageClip>& InClips, const FString& TitleFa, const FString& TitleEn);
	void Advance(float DeltaTime);
	/** Jumps to the last clip. */
	void Skip();
	void Stop();

	bool IsPlaying() const { return bPlaying; }
	bool IsFinished() const { return !bPlaying; }

private:
	float ClipAlpha() const;
	FSlateColor GetBackColor() const;
	FSlateColor GetImageTint() const;
	FSlateColor GetFrameColor() const;
	FSlateColor GetTextColor(bool bFarsi) const;
	FSlateColor GetTitleColor(bool bFarsi) const;
	FSlateColor GetRecColor() const;
	FText GetFa() const;
	FText GetEn() const;
	FText GetStamp() const;
	FText GetClipTime() const;
	const FSlateBrush* GetBrush() const;
	TOptional<FSlateRenderTransform> GetImageTransform() const;

	TArray<FCHMontageClip> Clips;
	TArray<TSharedPtr<FSlateBrush>> Brushes;
	FString TitleFa;
	FString TitleEn;

	TSharedPtr<SWidget> ImageBox;

	/** -1 = the title card. */
	int32 Index = -1;
	float Time = 0.f;
	float Total = 0.f;
	float OverallAlpha = 0.f;
	bool bPlaying = false;
	bool bEnding = false;
	float EndTime = 0.f;
};
