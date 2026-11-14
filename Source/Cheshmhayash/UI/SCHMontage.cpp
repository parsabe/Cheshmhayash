// Cheshm Hayash - the director's cut.

#include "UI/SCHMontage.h"
#include "Engine/Texture.h"
#include "Styling/SlateBrush.h"
#include "UI/CHStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const float TitleTime = 4.2f;
	const float ClipTime = 4.8f;
	const float LastHold = 8.f;
	const float FadeTime = 0.7f;
}

void SCHMontage::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHMontage::GetBackColor)
		]

		// title card
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return FText::FromString(TitleFa); })
				.Font(CH::FarsiFont(56.f, CH::EWeight::Bold))
				.ColorAndOpacity_Lambda([this]() { return GetTitleColor(true); })
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(ETextFlowDirection::RightToLeft)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return FText::FromString(TitleEn); })
				.Font(CH::LatinFont(16.f, CH::EWeight::Light, 520))
				.ColorAndOpacity_Lambda([this]() { return GetTitleColor(false); })
			]
		]

		// the clip, framed like a phone video
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 0.f, 70.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SAssignNew(ImageBox, SBox)
				.WidthOverride(1216.f)
				.HeightOverride(684.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHMontage::GetFrameColor)
					]
					+ SOverlay::Slot()
					.Padding(2.f)
					[
						SNew(SBox)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SImage)
							.Image(this, &SCHMontage::GetBrush)
							.ColorAndOpacity(this, &SCHMontage::GetImageTint)
							.RenderTransform(this, &SCHMontage::GetImageTransform)
							.RenderTransformPivot(FVector2D(0.5, 0.5))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.Padding(28.f, 24.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SBox).WidthOverride(14.f).HeightOverride(14.f)
							[
								SNew(SImage).Image(CH::RoundedBrush(7.f)).ColorAndOpacity(this, &SCHMontage::GetRecColor)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(10.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(this, &SCHMontage::GetClipTime)
							.Font(CH::LatinFont(16.f, CH::EWeight::Regular, 160))
							.ColorAndOpacity_Lambda([this]() { return GetTextColor(false); })
							.ShadowOffset(FVector2D(1.f, 1.f))
							.ShadowColorAndOpacity_Lambda([this]() { return FLinearColor(0.f, 0.f, 0.f, 0.7f * ClipAlpha() * OverallAlpha); })
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(28.f, 22.f)
					[
						SNew(STextBlock)
						.Text(this, &SCHMontage::GetStamp)
						.Font(CH::LatinFont(15.f, CH::EWeight::Regular, 200))
						.ColorAndOpacity_Lambda([this]() { return GetTextColor(false); })
						.ShadowOffset(FVector2D(1.f, 1.f))
						.ShadowColorAndOpacity_Lambda([this]() { return FLinearColor(0.f, 0.f, 0.f, 0.7f * ClipAlpha() * OverallAlpha); })
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 28.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(this, &SCHMontage::GetFa)
				.Font(CH::FarsiFont(30.f))
				.ColorAndOpacity_Lambda([this]() { return GetTextColor(true); })
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(ETextFlowDirection::RightToLeft)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(this, &SCHMontage::GetEn)
				.Font(CH::LatinFont(15.f, CH::EWeight::Light, 260))
				.ColorAndOpacity_Lambda([this]() { return GetTextColor(false); })
			]
		]
	];
}

void SCHMontage::Play(const TArray<FCHMontageClip>& InClips, const FString& InTitleFa, const FString& InTitleEn)
{
	Clips = InClips;
	TitleFa = InTitleFa;
	TitleEn = InTitleEn;
	Brushes.Reset();
	for (const FCHMontageClip& Clip : Clips)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		if (Clip.Image.IsValid())
		{
			Brush->SetResourceObject(Clip.Image.Get());
		}
		Brush->ImageSize = FVector2D(1212.0, 680.0);
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brushes.Add(Brush);
	}
	// no title: straight to the first clip (a single clip shown on its own)
	Index = TitleFa.IsEmpty() ? 0 : -1;
	Time = 0.f;
	bPlaying = Clips.Num() > 0 || Index < 0;
}

void SCHMontage::Skip()
{
	if (bPlaying && Clips.Num() > 0 && Index < Clips.Num() - 1)
	{
		Index = Clips.Num() - 1;
		Time = 0.f;
	}
}

void SCHMontage::Stop()
{
	bPlaying = false;
}

void SCHMontage::Advance(float DeltaTime)
{
	if (!bPlaying)
	{
		OverallAlpha = FMath::Max(0.f, OverallAlpha - DeltaTime / 1.2f);
		return;
	}
	OverallAlpha = FMath::Min(1.f, OverallAlpha + DeltaTime / 0.8f);
	Time += DeltaTime;

	if (Index < 0)
	{
		if (Time >= TitleTime)
		{
			Index = 0;
			Time = 0.f;
			if (Clips.Num() == 0)
			{
				bPlaying = false;
			}
		}
		return;
	}

	const bool bLast = Index == Clips.Num() - 1;
	if (Time >= (bLast ? LastHold : ClipTime))
	{
		if (bLast)
		{
			bPlaying = false;
		}
		else
		{
			++Index;
			Time = 0.f;
		}
	}
}

float SCHMontage::ClipAlpha() const
{
	if (Index < 0 || !Clips.IsValidIndex(Index))
	{
		return 0.f;
	}
	const bool bLast = Index == Clips.Num() - 1;
	const float In = FMath::Clamp(Time / FadeTime, 0.f, 1.f);
	const float Out = bLast ? 1.f : FMath::Clamp((ClipTime - Time) / FadeTime, 0.f, 1.f);
	return FMath::SmoothStep(0.f, 1.f, In * Out);
}

FSlateColor SCHMontage::GetBackColor() const
{
	return FSlateColor(FLinearColor(0.f, 0.f, 0.f, OverallAlpha));
}

FSlateColor SCHMontage::GetImageTint() const
{
	return FSlateColor(FLinearColor(1.f, 1.f, 1.f, ClipAlpha() * OverallAlpha));
}

FSlateColor SCHMontage::GetFrameColor() const
{
	return FSlateColor(FLinearColor(0.85f, 0.82f, 0.76f, 0.5f * ClipAlpha() * OverallAlpha));
}

FSlateColor SCHMontage::GetTextColor(bool bFarsi) const
{
	FLinearColor Color = bFarsi ? CH::Col::Paper : CH::Col::PaperDim;
	Color.A = ClipAlpha() * OverallAlpha;
	return FSlateColor(Color);
}

FSlateColor SCHMontage::GetTitleColor(bool bFarsi) const
{
	float Alpha = 0.f;
	if (bPlaying && Index < 0)
	{
		const float In = FMath::Clamp((Time - 0.4f) / 1.2f, 0.f, 1.f);
		const float Out = FMath::Clamp((TitleTime - Time) / 0.9f, 0.f, 1.f);
		Alpha = In * Out;
	}
	FLinearColor Color = bFarsi ? CH::Col::Paper : CH::Col::PaperDim;
	Color.A = Alpha * OverallAlpha;
	return FSlateColor(Color);
}

FSlateColor SCHMontage::GetRecColor() const
{
	FLinearColor Color = CH::Col::Rec;
	Color.A = ClipAlpha() * OverallAlpha * (FMath::Fmod(Time, 1.f) < 0.55f ? 1.f : 0.15f);
	return FSlateColor(Color);
}

FText SCHMontage::GetFa() const
{
	return Clips.IsValidIndex(Index) ? FText::FromString(Clips[Index].Fa) : FText::GetEmpty();
}

FText SCHMontage::GetEn() const
{
	return Clips.IsValidIndex(Index) ? FText::FromString(Clips[Index].En.ToUpper()) : FText::GetEmpty();
}

FText SCHMontage::GetStamp() const
{
	return Clips.IsValidIndex(Index) ? FText::FromString(Clips[Index].Stamp) : FText::GetEmpty();
}

FText SCHMontage::GetClipTime() const
{
	const int32 Seconds = FMath::FloorToInt32(Time);
	return FText::FromString(FString::Printf(TEXT("REC  00:%02d"), FMath::Clamp(Seconds, 0, 59)));
}

const FSlateBrush* SCHMontage::GetBrush() const
{
	return Brushes.IsValidIndex(Index) ? Brushes[Index].Get() : nullptr;
}

TOptional<FSlateRenderTransform> SCHMontage::GetImageTransform() const
{
	// a slow push in, like a hand holding the phone; the last clip is frozen
	const bool bLast = Index == Clips.Num() - 1;
	const float Scale = bLast ? 1.f : 1.f + 0.06f * FMath::Clamp(Time / ClipTime, 0.f, 1.f);
	return FSlateRenderTransform(Scale);
}
