// Cheshm Hayash - a vertical list of Farsi/English options.

#include "UI/SCHChoiceList.h"
#include "UI/SCHShapes.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SOverlay.h"
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
	Footer.Reset();
	Rows.Reset();
	Selected = 0;
	Alpha = 0.f;
	bShown = true;

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	for (int32 Index = 0; Index < Options.Num(); ++Index)
	{
		TSharedPtr<SVerticalBox> Row;
		Box->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, Index == 0 ? 0.f : 18.f, 0.f, 0.f)
			[
				SAssignNew(Row, SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this, Index]() { return Options.IsValidIndex(Index) ? FText::FromString(Options[Index].Fa) : FText::GetEmpty(); })
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
					.Text_Lambda([this, Index]() { return Options.IsValidIndex(Index) ? FText::FromString(Options[Index].En) : FText::GetEmpty(); })
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
		Rows.Add(Row);
	}

	// small hint under the options
	Box->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.f, 30.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { return FText::FromString(Footer); })
			.Font(CH::LatinFont(14.f, CH::EWeight::Regular, 200))
			.ColorAndOpacity_Lambda([this]() { return FSlateColor(FLinearColor(0.75f, 0.75f, 0.75f, Alpha * (0.4f + 0.18f * FMath::Cos(Pulse * 3.f)))); })
		];

	// choices at the bottom of the screen get the same soft dark band as the dialogue
	const bool bBand = VAlign == VAlign_Bottom;

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			[
				SNew(SBox)
				.HeightOverride(470.f)
				.Visibility(bBand ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
				[
					SNew(SCHGradient)
					.StartColor(FLinearColor::Transparent)
					.EndColor(this, &SCHChoiceList::GetBandColor)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign)
			.Padding(Padding)
			[
				Box
			]
		];
}

FLinearColor SCHChoiceList::GetBandColor() const
{
	return FLinearColor(0.f, 0.f, 0.f, 0.82f * Alpha);
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

void SCHChoiceList::UpdateOption(int32 Index, const FString& Fa, const FString& En)
{
	if (Options.IsValidIndex(Index))
	{
		Options[Index].Fa = Fa;
		Options[Index].En = En;
	}
}

int32 SCHChoiceList::HitTest() const
{
	if (!bShown || !FSlateApplication::IsInitialized())
	{
		return INDEX_NONE;
	}
	const FVector2D Cursor = FSlateApplication::Get().GetCursorPos();
	for (int32 Index = 0; Index < Rows.Num(); ++Index)
	{
		if (Rows[Index].IsValid())
		{
			// a generous hit box: the whole row, widened and padded
			const FGeometry& Geo = Rows[Index]->GetTickSpaceGeometry();
			const FVector2D Size = Geo.GetAbsoluteSize();
			const FVector2D Min = Geo.GetAbsolutePosition() - FVector2D(Size.X * 0.5f + 80.f, 8.f);
			const FVector2D Max = Geo.GetAbsolutePosition() + Size + FVector2D(Size.X * 0.5f + 80.f, 8.f);
			if (Cursor.X >= Min.X && Cursor.X <= Max.X && Cursor.Y >= Min.Y && Cursor.Y <= Max.Y)
			{
				return Index;
			}
		}
	}
	return INDEX_NONE;
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

