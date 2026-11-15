// Cheshm Hayash - his letter, all of it at once. Paper, his words in Farsi, the English beneath.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;

struct FCHLetterLine
{
	FString Fa;
	FString En;
};

class SCHLetter : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHLetter) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Show(const TArray<FCHLetterLine>& InLines);
	/** Shows the next line. Returns false when every line is already showing. */
	bool RevealNext();
	bool IsAllShown() const { return Shown >= Lines.Num(); }
	/** How long the open letter stays before it folds away by itself. */
	float GetReadSeconds() const;
	void Hide();
	bool IsOpen() const { return bOpen; }
	bool IsGone() const { return !bOpen && Alpha <= 0.f; }
	void Advance(float DeltaTime);

private:
	FSlateColor GetPaperColor() const;
	FSlateColor GetInk(int32 Index, bool bFarsi) const;

	TSharedPtr<SVerticalBox> Box;
	TArray<FCHLetterLine> Lines;
	TArray<float> LineAlpha;
	int32 Shown = 0;
	float LineTime = 0.f;
	float Alpha = 0.f;
	bool bOpen = false;
};
