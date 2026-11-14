// Cheshm Hayash - full screen text cards (opening card, title, chapter cards, ending cards, captions).

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UI/CHStyle.h"

struct FCHCardLine
{
	FString Text;
	bool bFarsi = false;
	float Size = 28.f;
	CH::EWeight Weight = CH::EWeight::Regular;
	FLinearColor Color = CH::Col::Paper;
	float TopPadding = 0.f;
	int32 LetterSpacing = 0;
	/** Seconds from the start of the card until this line starts to appear. */
	float Delay = 0.f;
	float FadeIn = 1.2f;
	/** > 0: revealed letter by letter (characters per second) instead of fading in. */
	float TypeRate = 0.f;

	FCHCardLine() {}
	FCHCardLine(const FString& InText, bool bInFarsi, float InSize, CH::EWeight InWeight = CH::EWeight::Regular, const FLinearColor& InColor = CH::Col::Paper)
		: Text(InText), bFarsi(bInFarsi), Size(InSize), Weight(InWeight), Color(InColor)
	{
	}
};

struct FCHCard
{
	TArray<FCHCardLine> Lines;
	/** Seconds to stay after the last line is fully visible. < 0 = stay until dismissed or skipped. */
	float Hold = 4.f;
	float FadeOut = 1.5f;
	/** The card cannot be skipped before this time. */
	float MinSkipTime = 1.0f;
	bool bSkippable = true;
	EHorizontalAlignment HAlign = HAlign_Center;
	EVerticalAlignment VAlign = VAlign_Center;
	FMargin Padding = FMargin(60.f);
	/** Lines longer than this (in slate units, 1080p layout) wrap. */
	float MaxWidth = 1300.f;
};

class SCHCardLayer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHCardLayer) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Play(const FCHCard& InCard);
	void Advance(float DeltaTime);

	/** Player pressed advance. First press reveals everything, second press fades the card out. Returns true if used. */
	bool Skip();

	/** Start fading out now. */
	void Dismiss(float InFadeOut = -1.f);

	void Clear();

	bool IsFinished() const { return State == EState::Idle; }
	bool IsFullyShown() const { return State == EState::Showing && Time >= ShownAt; }
	float GetTime() const { return Time; }

private:
	enum class EState : uint8 { Idle, Showing, FadingOut };

	float LineAlpha(int32 Index) const;
	FSlateColor GetLineColor(int32 Index) const;
	FLinearColor GetLineShadow(int32 Index) const;
	FText GetLineText(int32 Index) const;

	FCHCard Card;
	EState State = EState::Idle;
	float Time = 0.f;
	float ShownAt = 0.f;
	float OutAlpha = 1.f;
	float FadeOutTime = 1.5f;
};
