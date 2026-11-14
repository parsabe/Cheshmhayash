// Cheshm Hayash - dialogue. There are no voices: every line is text, Farsi on top, English below,
// typed out letter by letter.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCHDialogueOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHDialogueOverlay) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Starts typing a new line. Speaker names may be empty (narration / thought). */
	void ShowLine(const FString& InSpeakerFa, const FString& InSpeakerEn, const FString& InFa, const FString& InEn, float InRate = 30.f);
	void Advance(float DeltaTime);

	bool IsShown() const { return bShown; }
	bool IsTyping() const { return bShown && Revealed < 1.f; }
	/** Reveals the whole line at once. */
	void CompleteNow();
	/** Seconds since the line finished typing. */
	float GetTimeSinceComplete() const { return DoneTime; }
	/** Fades the line out. */
	void Hide();
	bool IsGone() const { return !bShown && Alpha <= 0.f; }

	/** True once per newly revealed letter batch: the cue for the soft typing sound. */
	bool ConsumeTypeTick();

private:
	FText GetSpeakerFa() const;
	FText GetSpeakerEn() const;
	FText GetFaText() const;
	FText GetEnText() const;
	FSlateColor GetSpeakerColor() const;
	FSlateColor GetSpeakerEnColor() const;
	FSlateColor GetFaColor() const;
	FSlateColor GetEnColor() const;
	FSlateColor GetCursorColor() const;
	FLinearColor GetBandColor() const;
	FLinearColor GetShadowColor() const;

	FString SpeakerFa;
	FString SpeakerEn;
	FString Fa;
	FString En;

	float Rate = 30.f;
	float Revealed = 0.f;   // 0..1 of the line
	float Alpha = 0.f;
	float DoneTime = 0.f;
	float Blink = 0.f;
	int32 LastFaCount = 0;
	bool bShown = false;
	bool bTypeTick = false;
};
