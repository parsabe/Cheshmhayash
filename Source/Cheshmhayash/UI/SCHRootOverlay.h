// Cheshm Hayash - the single root widget added to the game viewport. Owns every UI layer.
// It survives level changes, so fades and cards carry across map loads.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCHCardLayer;
class SCHChoiceList;
class SCHDialogueOverlay;
class SCHHud;
class SCHPhoneOverlay;

class SCHRootOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHRootOverlay) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Advances every layer's animation. Driven by the story director (game time, stops while paused). */
	void Advance(float DeltaTime);

	/** Developer notice in the top-left corner (missing map, missing story file...). Empty hides it. */
	void SetNotice(const FString& InNotice) { Notice = InNotice; }

	/** A short coloured flash over the picture (hit, alarm). */
	void Flash(const FLinearColor& Color, float Seconds);

	// Screen fade, driven by the story director.
	FLinearColor FadeColor = FLinearColor::Black;
	float FadeAlpha = 1.f;

	TSharedPtr<SCHCardLayer> Cards;
	TSharedPtr<SCHCardLayer> Captions;
	TSharedPtr<SCHChoiceList> Menu;
	TSharedPtr<SCHDialogueOverlay> Dialogue;
	TSharedPtr<SCHPhoneOverlay> Phone;
	TSharedPtr<SCHHud> Hud;

private:
	FSlateColor GetFadeColor() const;
	FSlateColor GetFlashColor() const;
	FText GetNoticeText() const;

	FString Notice;
	FLinearColor FlashTint = FLinearColor::Transparent;
	float FlashAlpha = 0.f;
	float FlashRate = 1.f;
};
