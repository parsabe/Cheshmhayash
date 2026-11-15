// Cheshm Hayash - dialogue overlay (single active language: English, German, or Persian).

#include "UI/SCHDialogueOverlay.h"
#include "Story/CHSave.h"
#include "UI/CHStyle.h"
#include "UI/SCHShapes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void SCHDialogueOverlay::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	ChildSlot
	[
		SNew(SOverlay)

		// soft dark band so text stays readable over any scene
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		[
			SNew(SBox)
			.HeightOverride(420.f)
			[
				SNew(SCHGradient)
				.StartColor(FLinearColor::Transparent)
				.EndColor(this, &SCHDialogueOverlay::GetBandColor)
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(60.f, 0.f, 60.f, 86.f)
		[
			SNew(SVerticalBox)

			// speaker (only active language shown)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
					.Text(this, &SCHDialogueOverlay::GetSpeakerEn)
					.Font(CH::LatinFont(18.f, CH::EWeight::Bold, 320))
					.ColorAndOpacity(this, &SCHDialogueOverlay::GetSpeakerColor)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
					.Text(this, &SCHDialogueOverlay::GetSpeakerFa)
					.Font(CH::FarsiFont(24.f, CH::EWeight::Bold))
					.ColorAndOpacity(this, &SCHDialogueOverlay::GetSpeakerColor)
					.TextShapingMethod(ETextShapingMethod::FullShaping)
					.TextFlowDirection(ETextFlowDirection::RightToLeft)
				]
			]

			// Farsi line (only when Persian is selected)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
				.Text(this, &SCHDialogueOverlay::GetFaText)
				.Font(CH::FarsiFont(34.f, CH::EWeight::Regular))
				.ColorAndOpacity(this, &SCHDialogueOverlay::GetFaColor)
				.ShadowOffset(FVector2D(1.5f, 1.5f))
				.ShadowColorAndOpacity(this, &SCHDialogueOverlay::GetShadowColor)
				.Justification(ETextJustify::Center)
				.WrapTextAt(1250.f)
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(ETextFlowDirection::RightToLeft)
			]

			// English / German line (only when English or German is selected)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
				.Text(this, &SCHDialogueOverlay::GetEnText)
				.Font(CH::LatinFont(34.f, CH::EWeight::Regular))
				.ColorAndOpacity(this, &SCHDialogueOverlay::GetEnColor)
				.ShadowOffset(FVector2D(1.5f, 1.5f))
				.ShadowColorAndOpacity(this, &SCHDialogueOverlay::GetShadowColor)
				.Justification(ETextJustify::Center)
				.WrapTextAt(1250.f)
			]

			// "continue" mark
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(26.f)
				.HeightOverride(2.f)
				[
					SNew(SImage)
					.Image(CH::WhiteBrush())
					.ColorAndOpacity(this, &SCHDialogueOverlay::GetCursorColor)
				]
			]

			// hint (localized to active language)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
				.Text_Lambda([]() { return FText::FromString(CHSettings::Latin(TEXT("Enter  /  Click to continue"))); })
				.Font(CH::LatinFont(14.f, CH::EWeight::Regular, 200))
				.ColorAndOpacity(this, &SCHDialogueOverlay::GetHintColor)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
				.Text(FText::FromString(TEXT("برای ادامه کلیک کنید یا Enter بزنید")))
				.Font(CH::FarsiFont(16.f, CH::EWeight::Regular))
				.ColorAndOpacity(this, &SCHDialogueOverlay::GetHintColor)
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(ETextFlowDirection::RightToLeft)
			]
		]
	];
}

void SCHDialogueOverlay::ShowLine(const FString& InSpeakerFa, const FString& InSpeakerEn, const FString& InFa, const FString& InEn, float InRate, const FLinearColor& InTint)
{
	SpeakerFa = InSpeakerFa;
	SpeakerEn = InSpeakerEn.ToUpper();
	Tint = InTint;
	Fa = InFa;
	En = InEn;
	Rate = FMath::Max(1.f, InRate);
	Revealed = 0.f;
	DoneTime = 0.f;
	LastFaCount = 0;
	bShown = true;
}

void SCHDialogueOverlay::Advance(float DeltaTime)
{
	Blink += DeltaTime;

	if (bShown)
	{
		Alpha = FMath::Min(1.f, Alpha + DeltaTime / 0.25f);
		if (Revealed < 1.f)
		{
			const FString ActiveLine = CHSettings::IsPersian() ? Fa : CHSettings::Latin(En);
			const int32 Length = FMath::Max(1, ActiveLine.Len());
			Revealed = FMath::Min(1.f, Revealed + DeltaTime * Rate / Length);
			const int32 Count = FMath::FloorToInt32(Revealed * ActiveLine.Len());
			if (Count > LastFaCount)
			{
				if (ActiveLine.IsValidIndex(Count - 1) && !FChar::IsWhitespace(ActiveLine[Count - 1]))
				{
					bTypeTick = true;
				}
				LastFaCount = Count;
			}
		}
		else
		{
			DoneTime += DeltaTime;
		}
	}
	else if (Alpha > 0.f)
	{
		Alpha = FMath::Max(0.f, Alpha - DeltaTime / 0.35f);
	}
}

void SCHDialogueOverlay::CompleteNow()
{
	if (bShown)
	{
		Revealed = 1.f;
	}
}

void SCHDialogueOverlay::Hide()
{
	bShown = false;
}

bool SCHDialogueOverlay::ConsumeTypeTick()
{
	const bool bTick = bTypeTick;
	bTypeTick = false;
	return bTick;
}

FText SCHDialogueOverlay::GetSpeakerFa() const
{
	return FText::FromString(SpeakerFa);
}

FText SCHDialogueOverlay::GetSpeakerEn() const
{
	return FText::FromString(CHSettings::Latin(SpeakerEn));
}

FText SCHDialogueOverlay::GetFaText() const
{
	const int32 Count = FMath::Clamp(FMath::CeilToInt32(Revealed * Fa.Len()), 0, Fa.Len());
	return FText::FromString(Fa.Left(Count));
}

FText SCHDialogueOverlay::GetEnText() const
{
	const FString ActiveEn = CHSettings::Latin(En);
	const int32 Count = FMath::Clamp(FMath::CeilToInt32(Revealed * ActiveEn.Len()), 0, ActiveEn.Len());
	return FText::FromString(ActiveEn.Left(Count));
}

FSlateColor SCHDialogueOverlay::GetSpeakerColor() const
{
	FLinearColor Color = Tint;
	Color.A = Alpha * 0.95f;
	return FSlateColor(Color);
}

FSlateColor SCHDialogueOverlay::GetSpeakerEnColor() const
{
	FLinearColor Color = Tint;
	Color.A = Alpha * 0.95f;
	return FSlateColor(Color);
}

FSlateColor SCHDialogueOverlay::GetFaColor() const
{
	FLinearColor Color = Tint;
	Color.A = Alpha;
	return FSlateColor(Color);
}

FSlateColor SCHDialogueOverlay::GetEnColor() const
{
	FLinearColor Color = Tint;
	Color.A = Alpha;
	return FSlateColor(Color);
}

FSlateColor SCHDialogueOverlay::GetCursorColor() const
{
	FLinearColor Color = CH::Col::Paper;
	const bool bDone = bShown && Revealed >= 1.f;
	Color.A = bDone ? Alpha * (0.35f + 0.35f * FMath::Cos(Blink * 3.2f)) : 0.f;
	return FSlateColor(Color);
}

FLinearColor SCHDialogueOverlay::GetBandColor() const
{
	return FLinearColor(0.f, 0.f, 0.f, 0.78f * Alpha);
}

FLinearColor SCHDialogueOverlay::GetShadowColor() const
{
	return FLinearColor(0.f, 0.f, 0.f, 0.7f * Alpha);
}

FSlateColor SCHDialogueOverlay::GetHintColor() const
{
	FLinearColor Color = FLinearColor(0.75f, 0.75f, 0.75f, 1.f);
	Color.A = bHint ? Alpha * (0.45f + 0.2f * FMath::Cos(Blink * 3.f)) : 0.f;
	return FSlateColor(Color);
}
