// Cheshm Hayash - full screen text cards.

#include "UI/SCHCardLayer.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

void SCHCardLayer::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);
	ChildSlot[SNullWidget::NullWidget];
}

void SCHCardLayer::Play(const FCHCard& InCard)
{
	Card = InCard;
	State = EState::Showing;
	Time = 0.f;
	OutAlpha = 1.f;
	FadeOutTime = Card.FadeOut;

	ShownAt = 0.f;
	for (const FCHCardLine& Line : Card.Lines)
	{
		float End = Line.Delay + Line.FadeIn;
		if (Line.TypeRate > 0.f)
		{
			End = Line.Delay + Line.Text.Len() / Line.TypeRate;
		}
		ShownAt = FMath::Max(ShownAt, End);
	}

	const ETextJustify::Type Justify =
		Card.HAlign == HAlign_Left ? ETextJustify::Left :
		Card.HAlign == HAlign_Right ? ETextJustify::Right : ETextJustify::Center;

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	for (int32 Index = 0; Index < Card.Lines.Num(); ++Index)
	{
		const FCHCardLine& Line = Card.Lines[Index];
		Box->AddSlot()
			.AutoHeight()
			.HAlign(Card.HAlign)
			.Padding(0.f, Line.TopPadding, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(this, &SCHCardLayer::GetLineText, Index)
				.Font(Line.bFarsi ? CH::FarsiFont(Line.Size, Line.Weight) : CH::LatinFont(Line.Size, Line.Weight, Line.LetterSpacing))
				.ColorAndOpacity(this, &SCHCardLayer::GetLineColor, Index)
				.ShadowOffset(FVector2D(1.5f, 1.5f))
				.ShadowColorAndOpacity(this, &SCHCardLayer::GetLineShadow, Index)
				.Justification(Justify)
				// A fixed wrap width: auto-wrap inside a centred slot can latch onto a too-narrow width.
				.WrapTextAt(Card.MaxWidth)
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(Line.bFarsi ? ETextFlowDirection::RightToLeft : ETextFlowDirection::LeftToRight)
			];
	}

	ChildSlot
		.HAlign(Card.HAlign)
		.VAlign(Card.VAlign)
		.Padding(Card.Padding)
		[
			Box
		];
}

void SCHCardLayer::Advance(float DeltaTime)
{
	if (State == EState::Idle)
	{
		return;
	}

	Time += DeltaTime;

	if (State == EState::Showing)
	{
		if (Card.Hold >= 0.f && Time >= ShownAt + Card.Hold)
		{
			Dismiss();
		}
	}
	else if (State == EState::FadingOut)
	{
		OutAlpha -= DeltaTime / FMath::Max(FadeOutTime, 0.01f);
		if (OutAlpha <= 0.f)
		{
			Clear();
		}
	}
}

bool SCHCardLayer::Skip()
{
	if (State != EState::Showing || !Card.bSkippable || Time < Card.MinSkipTime)
	{
		return false;
	}
	if (Time < ShownAt)
	{
		Time = ShownAt;
		return true;
	}
	Dismiss(FMath::Min(Card.FadeOut, 0.8f));
	return true;
}

void SCHCardLayer::Dismiss(float InFadeOut)
{
	if (State == EState::Showing)
	{
		State = EState::FadingOut;
		FadeOutTime = InFadeOut >= 0.f ? InFadeOut : Card.FadeOut;
		if (FadeOutTime <= 0.f)
		{
			Clear();
		}
	}
}

void SCHCardLayer::Clear()
{
	State = EState::Idle;
	Time = 0.f;
	Card = FCHCard();
	ChildSlot[SNullWidget::NullWidget];
}

float SCHCardLayer::LineAlpha(int32 Index) const
{
	if (!Card.Lines.IsValidIndex(Index) || State == EState::Idle)
	{
		return 0.f;
	}
	const FCHCardLine& Line = Card.Lines[Index];
	float Alpha;
	if (Line.TypeRate > 0.f)
	{
		Alpha = Time >= Line.Delay ? 1.f : 0.f;
	}
	else
	{
		Alpha = Line.FadeIn > 0.f ? FMath::Clamp((Time - Line.Delay) / Line.FadeIn, 0.f, 1.f) : (Time >= Line.Delay ? 1.f : 0.f);
		Alpha = FMath::SmoothStep(0.f, 1.f, Alpha);
	}
	return Alpha * FMath::Clamp(OutAlpha, 0.f, 1.f);
}

FSlateColor SCHCardLayer::GetLineColor(int32 Index) const
{
	if (!Card.Lines.IsValidIndex(Index))
	{
		return FSlateColor(FLinearColor::Transparent);
	}
	FLinearColor Color = Card.Lines[Index].Color;
	Color.A *= LineAlpha(Index);
	return FSlateColor(CH::Readable(Color));
}

FLinearColor SCHCardLayer::GetLineShadow(int32 Index) const
{
	return CH::ReadableShadow(LineAlpha(Index));
}

FText SCHCardLayer::GetLineText(int32 Index) const
{
	if (!Card.Lines.IsValidIndex(Index))
	{
		return FText::GetEmpty();
	}
	const FCHCardLine& Line = Card.Lines[Index];
	if (Line.TypeRate > 0.f)
	{
		const int32 Count = FMath::Clamp(FMath::FloorToInt32((Time - Line.Delay) * Line.TypeRate), 0, Line.Text.Len());
		return FText::FromString(Line.Text.Left(Count));
	}
	return FText::FromString(Line.Text);
}
