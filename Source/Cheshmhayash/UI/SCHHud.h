// Cheshm Hayash - the HUD: blinking REC dot, the "being seen" eye, the phone-camera viewfinder
// while filming, prompts, the objective line and short toasts ("Saved", "Run").

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UTexture;
struct FSlateBrush;

struct FCHHudState
{
	bool bActive = false;
	float Suspicion = 0.f;
	bool bChased = false;
	bool bFilming = false;
	bool bCanFilm = false;
	float FilmProgress = 0.f;
	bool bHasTarget = false;
	FString TargetFa;
	FString TargetEn;
	FString PromptFa;
	FString PromptEn;
	FString HintFa;
	FString HintEn;
	int32 Chances = 0;
	bool bHidden = false;
};

class SCHHud : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHHud) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Advance(float DeltaTime);

	void SetState(const FCHHudState& InState) { State = InState; }
	void ClearState() { State = FCHHudState(); }

	/** The REC dot (chapters one and two). Suppressed while a full-screen card is up. */
	void SetRec(bool bOn, float Seconds, bool bSuppressed);

	void Toast(const FString& Fa, const FString& En, const FLinearColor& Color, float Seconds = 2.8f, UTexture* Image = nullptr);
	void ShowControls(float Seconds) { ControlsTime = Seconds; }

private:
	TSharedRef<SWidget> MakeCorner(EHorizontalAlignment H, EVerticalAlignment V);

	EVisibility GetRecVisibility() const;
	FSlateColor GetRecDotColor() const;
	FSlateColor GetRecTextColor() const;
	FText GetRecTime() const;
	FSlateColor GetHintColor(bool bFarsi) const;
	FText GetHintFa() const;
	FText GetHintEn() const;

	float GetEyeOpen() const;
	float GetEyeFill() const;
	FLinearColor GetEyeLine() const;
	FLinearColor GetEyeIris() const;
	FText GetEyeStatusFa() const;
	FText GetEyeStatusEn() const;
	FSlateColor GetEyeStatusColor() const;

	FSlateColor GetFilmColor() const;
	FSlateColor GetFilmRecColor() const;
	FText GetFilmTime() const;
	FText GetTargetFa() const;
	FText GetTargetEn() const;
	FOptionalSize GetFilmBarWidth() const;
	FSlateColor GetFilmBarColor() const;
	FSlateColor GetFilmTrackColor() const;
	FSlateColor GetFilmHintColor() const;

	FSlateColor GetPromptColor(bool bFarsi) const;
	FSlateColor GetKeyColor() const;
	FSlateColor GetKeyTextColor() const;
	FText GetPromptFa() const;
	FText GetPromptEn() const;

	FSlateColor GetToastColor(bool bFarsi) const;
	FText GetToastFa() const;
	FText GetToastEn() const;
	const FSlateBrush* GetToastBrush() const;
	EVisibility GetToastImageVisibility() const;
	FSlateColor GetToastImageTint() const;

	FSlateColor GetControlsColor() const;

	FCHHudState State;
	float Time = 0.f;

	bool bRec = false;
	bool bRecSuppressed = false;
	float RecSeconds = 0.f;
	float RecAlpha = 0.f;

	float FilmAlpha = 0.f;
	float FilmSeconds = 0.f;
	float EyeAlpha = 0.f;
	float EyeOpen = 0.f;
	float PromptAlpha = 0.f;
	float HintAlpha = 0.f;
	FString LastPromptFa;
	FString LastPromptEn;

	FString ToastFa;
	FString ToastEn;
	FLinearColor ToastTint = FLinearColor::White;
	float ToastTime = 0.f;
	float ToastDuration = 0.f;
	TSharedPtr<FSlateBrush> ToastBrush;

	float ControlsTime = 0.f;
};
