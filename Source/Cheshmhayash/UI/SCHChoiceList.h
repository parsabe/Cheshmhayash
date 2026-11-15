// Cheshm Hayash - a vertical list of Farsi/English options (title menu, pause menu, story choices).

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UI/CHStyle.h"

struct FCHOption
{
	FString Fa;
	FString En;

	FCHOption() {}
	FCHOption(const FString& InFa, const FString& InEn) : Fa(InFa), En(InEn) {}
};

class SCHChoiceList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHChoiceList) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Show(const TArray<FCHOption>& InOptions, EVerticalAlignment VAlign, const FMargin& Padding, float FaSize = 30.f, float EnSize = 17.f);
	void Hide();
	/** A small hint line under the options (clears when the list is shown again). */
	void SetFooter(const FString& InFooter) { Footer = InFooter; }
	void Advance(float DeltaTime);

	/** Moves the highlight (wraps around). */
	void Move(int32 Direction);
	void SetSelected(int32 Index);
	/** Changes the text of one row while it is shown (settings). */
	void UpdateOption(int32 Index, const FString& Fa, const FString& En);
	/** The row under the mouse cursor, or INDEX_NONE. */
	int32 HitTest() const;
	int32 GetSelected() const { return Selected; }
	int32 Num() const { return Options.Num(); }

	bool IsShown() const { return bShown; }
	/** True once faded in enough to accept a selection. */
	bool IsReady() const { return bShown && Alpha > 0.7f; }
	bool IsGone() const { return !bShown && Alpha <= 0.f; }

private:
	FSlateColor GetFaColor(int32 Index) const;
	FSlateColor GetEnColor(int32 Index) const;
	FSlateColor GetMarkColor(int32 Index) const;
	FLinearColor GetBandColor() const;

	TArray<FCHOption> Options;
	TArray<TSharedPtr<SWidget>> Rows;
	FString Footer;
	int32 Selected = 0;
	float Alpha = 0.f;
	float Pulse = 0.f;
	bool bShown = false;
};
