// Cheshm Hayash - his letter.

#include "UI/SCHLetter.h"
#include "UI/CHStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SCHLetter::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	ChildSlot
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(CH::RoundedBrush(6.f))
		.BorderBackgroundColor(this, &SCHLetter::GetPaperColor)
		.Padding(FMargin(70.f, 56.f, 70.f, 60.f))
		.Visibility_Lambda([this]() { return Alpha > 0.f ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
		[
			SNew(SBox)
			.WidthOverride(860.f)
			[
				SAssignNew(Box, SVerticalBox)
			]
		]
	];
}

void SCHLetter::Show(const TArray<FCHLetterLine>& InLines)
{
	Lines = InLines;
	LineAlpha.Init(0.f, Lines.Num());
	Shown = 0;
	bOpen = true;

	Box->ClearChildren();
	for (int32 Index = 0; Index < Lines.Num(); ++Index)
	{
		const bool bSignature = Index == Lines.Num() - 1;
		Box->AddSlot()
			.AutoHeight()
			.HAlign(bSignature ? HAlign_Left : HAlign_Right)
			.Padding(0.f, Index == 0 ? 0.f : 22.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(bSignature ? HAlign_Left : HAlign_Right)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Lines[Index].Fa))
					.Font(CH::FarsiFont(bSignature ? 27.f : 25.f, bSignature ? CH::EWeight::Bold : CH::EWeight::Regular))
					.ColorAndOpacity(this, &SCHLetter::GetInk, Index, true)
					.WrapTextAt(860.f)
					.Justification(ETextJustify::Right)
					.TextShapingMethod(ETextShapingMethod::FullShaping)
					.TextFlowDirection(ETextFlowDirection::RightToLeft)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(bSignature ? HAlign_Left : HAlign_Right)
				.Padding(0.f, 3.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Lines[Index].En))
					.Font(CH::LatinFont(15.f, CH::EWeight::Light))
					.ColorAndOpacity(this, &SCHLetter::GetInk, Index, false)
					.WrapTextAt(860.f)
				]
			];
	}
	RevealNext();
}

bool SCHLetter::RevealNext()
{
	if (Shown >= Lines.Num())
	{
		return false;
	}
	++Shown;
	return true;
}

void SCHLetter::Hide()
{
	bOpen = false;
}

void SCHLetter::Advance(float DeltaTime)
{
	Alpha = FMath::Clamp(Alpha + (bOpen ? DeltaTime / 1.2f : -DeltaTime / 1.5f), 0.f, 1.f);
	for (int32 Index = 0; Index < LineAlpha.Num(); ++Index)
	{
		if (Index < Shown)
		{
			LineAlpha[Index] = FMath::Min(1.f, LineAlpha[Index] + DeltaTime / 1.6f);
		}
	}
}

FSlateColor SCHLetter::GetPaperColor() const
{
	return FSlateColor(FLinearColor(0.78f, 0.74f, 0.64f, 0.97f * Alpha));
}

FSlateColor SCHLetter::GetInk(int32 Index, bool bFarsi) const
{
	const float LineA = LineAlpha.IsValidIndex(Index) ? LineAlpha[Index] : 0.f;
	FLinearColor Color = bFarsi ? FLinearColor(0.035f, 0.04f, 0.07f) : FLinearColor(0.16f, 0.15f, 0.15f);
	Color.A = Alpha * FMath::SmoothStep(0.f, 1.f, LineA);
	return FSlateColor(Color);
}
