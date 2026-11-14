// Cheshm Hayash - the HUD.

#include "UI/SCHHud.h"
#include "Engine/Texture.h"
#include "Styling/SlateBrush.h"
#include "UI/CHStyle.h"
#include "UI/SCHShapes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	/** The drop shadow fades with the text (a fixed shadow would stay visible when the text is hidden). */
	TAttribute<FLinearColor> ShadowFor(const TAttribute<FSlateColor>& Color)
	{
		return TAttribute<FLinearColor>::CreateLambda([Color]()
		{
			return FLinearColor(0.f, 0.f, 0.f, 0.6f * Color.Get().GetSpecifiedColor().A);
		});
	}

	TSharedRef<STextBlock> HudFa(const TAttribute<FText>& Text, float Size, const TAttribute<FSlateColor>& Color, CH::EWeight Weight = CH::EWeight::Regular)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(CH::FarsiFont(Size, Weight))
			.ColorAndOpacity(Color)
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(ShadowFor(Color))
			.TextShapingMethod(ETextShapingMethod::FullShaping)
			.TextFlowDirection(ETextFlowDirection::RightToLeft);
	}

	TSharedRef<STextBlock> HudEn(const TAttribute<FText>& Text, float Size, const TAttribute<FSlateColor>& Color, int32 Spacing = 0, CH::EWeight Weight = CH::EWeight::Regular)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(CH::LatinFont(Size, Weight, Spacing))
			.ColorAndOpacity(Color)
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(ShadowFor(Color));
	}
}

TSharedRef<SWidget> SCHHud::MakeCorner(EHorizontalAlignment H, EVerticalAlignment V)
{
	const float Length = 58.f;
	const float Thick = 3.f;
	return SNew(SBox)
		.WidthOverride(Length)
		.HeightOverride(Length)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(V)
			[
				SNew(SBox).HeightOverride(Thick)
				[
					SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHHud::GetFilmColor)
				]
			]
			+ SOverlay::Slot()
			.HAlign(H)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox).WidthOverride(Thick)
				[
					SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHHud::GetFilmColor)
				]
			]
		];
}

void SCHHud::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	ChildSlot
	[
		SNew(SOverlay)

		// ---- REC and the objective line, top left
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(48.f, 40.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SCHHud::GetRecVisibility)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(15.f).HeightOverride(15.f)
					[
						SNew(SImage).Image(CH::RoundedBrush(7.5f)).ColorAndOpacity(this, &SCHHud::GetRecDotColor)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(10.f, 0.f, 0.f, 0.f)
				[
					HudEn(FText::FromString(TEXT("REC")), 16.f, TAttribute<FSlateColor>(this, &SCHHud::GetRecTextColor), 240, CH::EWeight::Bold)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(16.f, 0.f, 0.f, 0.f)
				[
					HudEn(TAttribute<FText>(this, &SCHHud::GetRecTime), 15.f, TAttribute<FSlateColor>(this, &SCHHud::GetRecTextColor), 120)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
				HudFa(TAttribute<FText>(this, &SCHHud::GetHintFa), 19.f, TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCHHud::GetHintColor, true)))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				HudEn(TAttribute<FText>(this, &SCHHud::GetHintEn), 12.f, TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCHHud::GetHintColor, false)), 160)
			]
		]

		// ---- the eye, top centre
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 34.f, 0.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(SBox).WidthOverride(86.f).HeightOverride(40.f)
				[
					SNew(SCHEye)
					.Open(this, &SCHHud::GetEyeOpen)
					.Fill(this, &SCHHud::GetEyeFill)
					.LineColor(this, &SCHHud::GetEyeLine)
					.IrisColor(this, &SCHHud::GetEyeIris)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 6.f, 0.f, 0.f)
			[
				HudFa(TAttribute<FText>(this, &SCHHud::GetEyeStatusFa), 15.f, TAttribute<FSlateColor>(this, &SCHHud::GetEyeStatusColor))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				HudEn(TAttribute<FText>(this, &SCHHud::GetEyeStatusEn), 10.f, TAttribute<FSlateColor>(this, &SCHHud::GetEyeStatusColor), 260)
			]
		]

		// ---- the phone camera viewfinder
		+ SOverlay::Slot()
		.Padding(70.f, 64.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)[MakeCorner(HAlign_Left, VAlign_Top)]
			+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top)[MakeCorner(HAlign_Right, VAlign_Top)]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom)[MakeCorner(HAlign_Left, VAlign_Bottom)]
			+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom)[MakeCorner(HAlign_Right, VAlign_Bottom)]

			// REC and clip time, top right of the frame
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.f, 22.f, 26.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(18.f).HeightOverride(18.f)
					[
						SNew(SImage).Image(CH::RoundedBrush(9.f)).ColorAndOpacity(this, &SCHHud::GetFilmRecColor)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(10.f, 0.f, 0.f, 0.f)
				[
					HudEn(TAttribute<FText>(this, &SCHHud::GetFilmTime), 18.f, TAttribute<FSlateColor>(this, &SCHHud::GetFilmColor), 140)
				]
			]

			// centre mark
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SOverlay)
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(30.f).HeightOverride(1.5f)[SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHHud::GetFilmColor)]
				]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(1.5f).HeightOverride(30.f)[SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHHud::GetFilmColor)]
				]
			]

			// what is being filmed, and how much
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(0.f, 0.f, 0.f, 30.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					HudFa(TAttribute<FText>(this, &SCHHud::GetTargetFa), 20.f, TAttribute<FSlateColor>(this, &SCHHud::GetFilmColor))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 2.f, 0.f, 10.f)
				[
					HudEn(TAttribute<FText>(this, &SCHHud::GetTargetEn), 12.f, TAttribute<FSlateColor>(this, &SCHHud::GetFilmHintColor), 160)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(460.f)
					.HeightOverride(5.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHHud::GetFilmTrackColor)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						[
							SNew(SBox).WidthOverride(this, &SCHHud::GetFilmBarWidth)
							[
								SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHHud::GetFilmBarColor)
							]
						]
					]
				]
			]
		]

		// ---- prompt, bottom centre
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 150.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(CH::RoundedBrush(6.f))
				.BorderBackgroundColor(this, &SCHHud::GetKeyColor)
				.Padding(FMargin(11.f, 4.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("E")))
					.Font(CH::LatinFont(16.f, CH::EWeight::Bold))
					.ColorAndOpacity(this, &SCHHud::GetKeyTextColor)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(16.f, 0.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					HudFa(TAttribute<FText>(this, &SCHHud::GetPromptFa), 21.f, TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCHHud::GetPromptColor, true)))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					HudEn(TAttribute<FText>(this, &SCHHud::GetPromptEn), 12.f, TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCHHud::GetPromptColor, false)), 160)
				]
			]
		]

		// ---- toast, centre
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 0.f, 140.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 18.f)
			[
				SNew(SBox)
				.WidthOverride(400.f)
				.HeightOverride(233.f)
				.Visibility(this, &SCHHud::GetToastImageVisibility)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage).Image(CH::WhiteBrush()).ColorAndOpacity(this, &SCHHud::GetToastImageTint)
					]
					+ SOverlay::Slot()
					.Padding(4.f)
					[
						SNew(SImage).Image(this, &SCHHud::GetToastBrush).ColorAndOpacity(this, &SCHHud::GetToastImageTint)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				HudFa(TAttribute<FText>(this, &SCHHud::GetToastFa), 36.f, TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCHHud::GetToastColor, true)), CH::EWeight::Bold)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				HudEn(TAttribute<FText>(this, &SCHHud::GetToastEn), 14.f, TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateSP(this, &SCHHud::GetToastColor, false)), 300)
			]
		]

		// ---- controls, bottom right (first time only)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 48.f, 40.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				HudFa(FText::FromString(TEXT("حرکت WASD  ·  دویدن Shift  ·  آهسته Ctrl  ·  استفاده E  ·  فیلم‌برداری: کلیک راست را نگه دار")), 15.f, TAttribute<FSlateColor>(this, &SCHHud::GetControlsColor))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0.f, 3.f, 0.f, 0.f)
			[
				HudEn(FText::FromString(TEXT("WASD MOVE  ·  SHIFT RUN  ·  CTRL SNEAK  ·  E USE  ·  HOLD RIGHT MOUSE TO FILM")), 11.f, TAttribute<FSlateColor>(this, &SCHHud::GetControlsColor), 160)
			]
		]
	];
}

// ---------------------------------------------------------------------------------------------
// Animation
// ---------------------------------------------------------------------------------------------

void SCHHud::SetRec(bool bOn, float Seconds, bool bSuppressed)
{
	bRec = bOn;
	RecSeconds = Seconds;
	bRecSuppressed = bSuppressed;
}

void SCHHud::Toast(const FString& Fa, const FString& En, const FLinearColor& Color, float Seconds, UTexture* Image)
{
	ToastFa = Fa;
	ToastEn = En;
	ToastTint = Color;
	ToastTime = 0.f;
	ToastDuration = FMath::Max(0.5f, Seconds);
	if (Image)
	{
		ToastBrush = MakeShared<FSlateBrush>();
		ToastBrush->SetResourceObject(Image);
		ToastBrush->ImageSize = FVector2D(392.0, 225.0);
		ToastBrush->DrawAs = ESlateBrushDrawType::Image;
	}
	else
	{
		ToastBrush.Reset();
	}
}

void SCHHud::Advance(float DeltaTime)
{
	Time += DeltaTime;

	auto Approach = [DeltaTime](float& Value, bool bOn, float InTime, float OutTime)
	{
		Value = FMath::Clamp(Value + (bOn ? DeltaTime / InTime : -DeltaTime / OutTime), 0.f, 1.f);
	};

	Approach(RecAlpha, bRec && !bRecSuppressed, 0.6f, 0.4f);
	Approach(FilmAlpha, State.bFilming, 0.25f, 0.35f);
	FilmSeconds = State.bFilming ? FilmSeconds + DeltaTime : 0.f;

	const bool bEye = State.bActive && (State.Suspicion > 0.01f || State.bChased || State.bHidden);
	Approach(EyeAlpha, bEye, 0.3f, 0.8f);
	const float TargetOpen = State.bHidden ? 0.08f : (State.bChased ? 1.f : FMath::Clamp(0.25f + State.Suspicion, 0.f, 1.f));
	EyeOpen = FMath::FInterpTo(EyeOpen, TargetOpen, DeltaTime, 6.f);

	const bool bPrompt = State.bActive && !State.PromptFa.IsEmpty();
	if (bPrompt)
	{
		LastPromptFa = State.PromptFa;
		LastPromptEn = State.PromptEn;
	}
	Approach(PromptAlpha, bPrompt, 0.2f, 0.25f);
	Approach(HintAlpha, State.bActive && !State.HintFa.IsEmpty(), 1.f, 0.6f);

	if (ToastDuration > 0.f)
	{
		ToastTime += DeltaTime;
		if (ToastTime > ToastDuration)
		{
			ToastDuration = 0.f;
			ToastBrush.Reset();
		}
	}
	if (ControlsTime > 0.f)
	{
		ControlsTime -= DeltaTime;
	}
}

// ---------------------------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------------------------

EVisibility SCHHud::GetRecVisibility() const
{
	return RecAlpha > 0.f ? EVisibility::HitTestInvisible : EVisibility::Hidden;
}

FSlateColor SCHHud::GetRecDotColor() const
{
	FLinearColor Color = CH::Col::Rec;
	Color.A = RecAlpha * (FMath::Fmod(Time, 1.f) < 0.55f ? 1.f : 0.12f);
	return FSlateColor(Color);
}

FSlateColor SCHHud::GetRecTextColor() const
{
	FLinearColor Color = CH::Col::Paper;
	Color.A = RecAlpha * 0.85f;
	return FSlateColor(Color);
}

FText SCHHud::GetRecTime() const
{
	const int32 Total = FMath::FloorToInt32(RecSeconds);
	return FText::FromString(FString::Printf(TEXT("%02d:%02d:%02d"), Total / 3600, (Total / 60) % 60, Total % 60));
}

FSlateColor SCHHud::GetHintColor(bool bFarsi) const
{
	FLinearColor Color = bFarsi ? CH::Col::Paper : CH::Col::PaperDim;
	Color.A = HintAlpha * (bFarsi ? 0.9f : 0.75f);
	return FSlateColor(Color);
}

FText SCHHud::GetHintFa() const
{
	return FText::FromString(State.HintFa);
}

FText SCHHud::GetHintEn() const
{
	return FText::FromString(State.HintEn.ToUpper());
}

float SCHHud::GetEyeOpen() const
{
	return EyeOpen;
}

float SCHHud::GetEyeFill() const
{
	return State.bChased ? 1.f : FMath::Clamp(State.Suspicion * 1.3f, 0.f, 1.f);
}

FLinearColor SCHHud::GetEyeLine() const
{
	FLinearColor Color = State.bChased ? FLinearColor(1.f, 0.12f, 0.06f) : CH::Col::Paper;
	Color.A = EyeAlpha * (State.bChased ? (0.7f + 0.3f * FMath::Sin(Time * 12.f)) : 0.85f);
	return Color;
}

FLinearColor SCHHud::GetEyeIris() const
{
	FLinearColor Color = State.bChased ? FLinearColor(1.f, 0.08f, 0.04f) : FMath::Lerp(FLinearColor(1.f, 0.85f, 0.6f), CH::Col::Ember, FMath::Clamp(State.Suspicion, 0.f, 1.f));
	Color.A = EyeAlpha;
	return Color;
}

FText SCHHud::GetEyeStatusFa() const
{
	if (State.bChased)
	{
		return FText::FromString(TEXT("دنبالت هستند"));
	}
	if (State.bHidden)
	{
		return FText::FromString(TEXT("پنهانی"));
	}
	return FText::FromString(State.Suspicion > 0.5f ? TEXT("دارند نگاه می‌کنند") : TEXT(""));
}

FText SCHHud::GetEyeStatusEn() const
{
	if (State.bChased)
	{
		return FText::FromString(TEXT("THEY ARE AFTER YOU"));
	}
	if (State.bHidden)
	{
		return FText::FromString(TEXT("HIDDEN"));
	}
	return FText::FromString(State.Suspicion > 0.5f ? TEXT("THEY ARE LOOKING") : TEXT(""));
}

FSlateColor SCHHud::GetEyeStatusColor() const
{
	FLinearColor Color = State.bChased ? FLinearColor(1.f, 0.25f, 0.15f) : CH::Col::PaperDim;
	Color.A = EyeAlpha;
	return FSlateColor(Color);
}

FSlateColor SCHHud::GetFilmColor() const
{
	FLinearColor Color = CH::Col::Paper;
	Color.A = FilmAlpha * 0.9f;
	return FSlateColor(Color);
}

FSlateColor SCHHud::GetFilmRecColor() const
{
	FLinearColor Color = CH::Col::Rec;
	Color.A = FilmAlpha * (FMath::Fmod(Time, 0.8f) < 0.45f ? 1.f : 0.2f);
	return FSlateColor(Color);
}

FText SCHHud::GetFilmTime() const
{
	const int32 Total = FMath::FloorToInt32(FilmSeconds);
	return FText::FromString(FString::Printf(TEXT("REC  %02d:%02d"), Total / 60, Total % 60));
}

FText SCHHud::GetTargetFa() const
{
	if (State.bHasTarget)
	{
		return FText::FromString(State.TargetFa);
	}
	return FText::FromString(TEXT(""));
}

FText SCHHud::GetTargetEn() const
{
	if (State.bHasTarget)
	{
		return FText::FromString(State.TargetEn.ToUpper());
	}
	return FText::FromString(TEXT("AIM AT SOMETHING THAT MATTERS"));
}

FOptionalSize SCHHud::GetFilmBarWidth() const
{
	return FOptionalSize(460.f * FMath::Clamp(State.bHasTarget ? State.FilmProgress : 0.f, 0.f, 1.f));
}

FSlateColor SCHHud::GetFilmBarColor() const
{
	FLinearColor Color = CH::Col::Rec;
	Color.A = FilmAlpha;
	return FSlateColor(Color);
}

FSlateColor SCHHud::GetFilmTrackColor() const
{
	return FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.16f * FilmAlpha));
}

FSlateColor SCHHud::GetFilmHintColor() const
{
	FLinearColor Color = CH::Col::PaperDim;
	Color.A = FilmAlpha * 0.8f;
	return FSlateColor(Color);
}

FSlateColor SCHHud::GetPromptColor(bool bFarsi) const
{
	FLinearColor Color = bFarsi ? CH::Col::Paper : CH::Col::PaperDim;
	Color.A = PromptAlpha;
	return FSlateColor(Color);
}

FSlateColor SCHHud::GetKeyColor() const
{
	return FSlateColor(FLinearColor(0.9f, 0.86f, 0.78f, PromptAlpha));
}

FSlateColor SCHHud::GetKeyTextColor() const
{
	return FSlateColor(FLinearColor(0.02f, 0.02f, 0.02f, PromptAlpha));
}

FText SCHHud::GetPromptFa() const
{
	return FText::FromString(LastPromptFa);
}

FText SCHHud::GetPromptEn() const
{
	return FText::FromString(LastPromptEn.ToUpper());
}

FSlateColor SCHHud::GetToastColor(bool bFarsi) const
{
	if (ToastDuration <= 0.f)
	{
		return FSlateColor(FLinearColor::Transparent);
	}
	const float In = FMath::Clamp(ToastTime / 0.25f, 0.f, 1.f);
	const float Out = FMath::Clamp((ToastDuration - ToastTime) / 0.6f, 0.f, 1.f);
	FLinearColor Color = bFarsi ? ToastTint : CH::Col::PaperDim;
	Color.A = In * Out;
	return FSlateColor(Color);
}

FText SCHHud::GetToastFa() const
{
	return FText::FromString(ToastFa);
}

FText SCHHud::GetToastEn() const
{
	return FText::FromString(ToastEn);
}

const FSlateBrush* SCHHud::GetToastBrush() const
{
	return ToastBrush.IsValid() ? ToastBrush.Get() : nullptr;
}

EVisibility SCHHud::GetToastImageVisibility() const
{
	return (ToastBrush.IsValid() && ToastDuration > 0.f) ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FSlateColor SCHHud::GetToastImageTint() const
{
	const float In = FMath::Clamp(ToastTime / 0.25f, 0.f, 1.f);
	const float Out = FMath::Clamp((ToastDuration - ToastTime) / 0.6f, 0.f, 1.f);
	return FSlateColor(FLinearColor(1.f, 1.f, 1.f, In * Out));
}

FSlateColor SCHHud::GetControlsColor() const
{
	FLinearColor Color = CH::Col::PaperDim;
	Color.A = FMath::Clamp(ControlsTime / 1.5f, 0.f, 1.f) * 0.85f;
	return FSlateColor(Color);
}
