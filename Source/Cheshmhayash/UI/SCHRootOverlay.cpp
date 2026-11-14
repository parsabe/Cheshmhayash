// Cheshm Hayash - the single root widget added to the game viewport.

#include "UI/SCHRootOverlay.h"
#include "UI/CHStyle.h"
#include "UI/SCHCardLayer.h"
#include "UI/SCHChoiceList.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SCHRootOverlay::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	ChildSlot
	[
		SNew(SOverlay)

		// screen fade
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(CH::WhiteBrush())
			.ColorAndOpacity(this, &SCHRootOverlay::GetFadeColor)
		]

		// corner captions (dates, places) - do not block the story
		+ SOverlay::Slot()
		[
			SAssignNew(Captions, SCHCardLayer)
		]

		// centre cards
		+ SOverlay::Slot()
		[
			SAssignNew(Cards, SCHCardLayer)
		]

		// menus and choices
		+ SOverlay::Slot()
		[
			SAssignNew(Menu, SCHChoiceList)
		]

		// developer notice
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(24.f)
		[
			SNew(STextBlock)
			.Text(this, &SCHRootOverlay::GetNoticeText)
			.Font(CH::LatinFont(14.f))
			.ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.35f, 0.2f, 1.f)))
			.WrapTextAt(1200.f)
		]
	];
}

void SCHRootOverlay::Advance(float DeltaTime)
{
	if (Cards.IsValid())
	{
		Cards->Advance(DeltaTime);
	}
	if (Captions.IsValid())
	{
		Captions->Advance(DeltaTime);
	}
	if (Menu.IsValid())
	{
		Menu->Advance(DeltaTime);
	}
}

FSlateColor SCHRootOverlay::GetFadeColor() const
{
	FLinearColor Color = FadeColor;
	Color.A = FMath::Clamp(FadeAlpha, 0.f, 1.f);
	return FSlateColor(Color);
}

FText SCHRootOverlay::GetNoticeText() const
{
	return FText::FromString(Notice);
}
