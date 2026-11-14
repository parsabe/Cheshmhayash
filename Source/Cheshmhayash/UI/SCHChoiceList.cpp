// Cheshm Hayash - a vertical list of Farsi/English options.

#include "UI/SCHChoiceList.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

void SCHChoiceList::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);
	ChildSlot[SNullWidget::NullWidget];
}

void SCHChoiceList::Show(const TArray<FCHOption>& InOptions, EVerticalAlignment VAlign, const FMargin& Padding, float FaSize, float EnSize)
{
	Options = InOptions;
	Selected = 0;
	Alpha = 0.f;
	bShown = true;

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	for (int32 Index = 0; Index < Options.Num(); ++Index)
	{
		const FCHOption& Option = Options[Index];
		Box->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, Index == 0 ? 0.f : 26.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Option.Fa))
					.Font(CH::FarsiFont(FaSize, CH::EWeight::Regular))
					.ColorAndOpacity(this, &SCHChoiceList::GetFaColor, Index)
					.Justification(ETextJustify::Center)
					.TextShapingMethod(ETextShapingMethod::FullShaping)
					.TextFlowDirection(ETextFlowDirection::RightToLeft)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 2.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Option.En))
					.Font(CH::LatinFont(EnSize, CH::EWeight::Light))
					.ColorAndOpacity(this, &SCHChoiceList::GetEnColor, Index)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(46.f)
					.HeightOverride(2.f)
					[
						SNew(SImage)
						.Image(CH::WhiteBrush())
						.ColorAndOpacity(this, &SCHChoiceList::GetMarkColor, Index)
					]
				]
			];
	}

	ChildSlot
		.HAlign(HAlign_Center)
		.VAlign(VAlign)
		.Padding(Padding)
		[
			Box
		];
}

void SCHChoiceList::Hide()
{
	bShown = false;
}

void SCHChoiceList::Advance(float DeltaTime)
{
	Pulse += DeltaTime;
	if (bShown)
	{
		Alpha = FMath::Min(1.f, Alpha + DeltaTime / 0.7f);
	}
	else if (Alpha > 0.f)
	{
		Alpha = FMath::Max(0.f, Alpha - DeltaTime / 0.5f);
		if (Alpha <= 0.f)
		{
			Options.Reset();
			ChildSlot[SNullWidget::NullWidget];
		}
	}
}

void SCHChoiceList::Move(int32 Direction)
{
	if (Options.Num() > 0)
	{
		Selected = (Selected + Direction + Options.Num()) % Options.Num();
		Pulse = 0.f;
	}
}

void SCHChoiceList::SetSelected(int32 Index)
{
	if (Options.IsValidIndex(Index))
	{
		Selected = Index;
		Pulse = 0.f;
	}
}

FSlateColor SCHChoiceList::GetFaColor(int32 Index) const
{
	FLinearColor Color = Index == Selected ? CH::Col::Paper : CH::Col::Faint;
	Color.A *= Alpha;
	return FSlateColor(Color);
}

FSlateColor SCHChoiceList::GetEnColor(int32 Index) const
{
	FLinearColor Color = Index == Selected ? CH::Col::PaperDim : CH::Col::Faint;
	Color.A *= Alpha * (Index == Selected ? 1.f : 0.8f);
	return FSlateColor(Color);
}

FSlateColor SCHChoiceList::GetMarkColor(int32 Index) const
{
	FLinearColor Color = CH::Col::Ember;
	const float Breath = 0.75f + 0.25f * FMath::Cos(Pulse * 2.4f);
	Color.A = Index == Selected ? Alpha * Breath : 0.f;
	return FSlateColor(Color);
}
