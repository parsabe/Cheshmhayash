// Cheshm Hayash - the phone overlay (single active language: English, German, or Persian).

#include "UI/SCHPhoneOverlay.h"
#include "Story/CHSave.h"
#include "UI/CHStyle.h"
#include "UI/SCHShapes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const float BodyWidth = 440.f;
	const float BodyHeight = 880.f;
	const float BubbleWrap = 268.f;

	const FLinearColor EdgeColor(0.10f, 0.10f, 0.115f, 1.f);
	const FLinearColor PhoneBodyColor(0.006f, 0.006f, 0.008f, 1.f);
	const FLinearColor ScreenColor(0.020f, 0.024f, 0.032f, 1.f);
	const FLinearColor TheirBubble(0.085f, 0.09f, 0.105f, 1.f);
	const FLinearColor MyBubble(0.030f, 0.150f, 0.125f, 1.f);
	const FLinearColor FieldColor(0.05f, 0.055f, 0.066f, 1.f);
	const FLinearColor FigureColor(0.085f, 0.095f, 0.115f, 1.f);

	TSharedRef<SWidget> Tinted(const FSlateBrush* Brush, const FLinearColor& Color)
	{
		return SNew(SImage).Image(Brush).ColorAndOpacity(FSlateColor(Color));
	}

	TSharedRef<SWidget> Sized(float Width, float Height, const TSharedRef<SWidget>& Content)
	{
		return SNew(SBox).WidthOverride(Width).HeightOverride(Height)[Content];
	}

	TSharedRef<STextBlock> FaText(const FString& Text, float Size, const FLinearColor& Color, float Wrap, CH::EWeight Weight = CH::EWeight::Regular)
	{
		return SNew(STextBlock)
			.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
			.Text(FText::FromString(Text))
			.Font(CH::FarsiFont(Size, Weight))
			.ColorAndOpacity(FSlateColor(Color))
			.WrapTextAt(Wrap)
			.TextShapingMethod(ETextShapingMethod::FullShaping)
			.TextFlowDirection(ETextFlowDirection::RightToLeft);
	}

	TSharedRef<STextBlock> EnText(const FString& Text, float Size, const FLinearColor& Color, float Wrap, int32 Spacing = 0, bool bAlwaysVisible = false)
	{
		return SNew(STextBlock)
			.Visibility_Lambda([bAlwaysVisible]() { return (bAlwaysVisible || !CHSettings::IsPersian()) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
			.Text_Lambda([Text]() { return FText::FromString(CHSettings::Latin(Text)); })
			.Font(CH::LatinFont(Size, CH::EWeight::Regular, Spacing))
			.ColorAndOpacity(FSlateColor(Color))
			.WrapTextAt(Wrap);
	}
}

void SCHPhoneOverlay::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);

	ChildSlot
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Center)
	.Padding(0.f, 0.f, 74.f, 0.f)
	[
		SAssignNew(PhoneBody, SBox)
		.WidthOverride(BodyWidth)
		.HeightOverride(BodyHeight)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				Tinted(CH::RoundedBrush(52.f), EdgeColor)
			]
			+ SOverlay::Slot()
			.Padding(3.f)
			[
				Tinted(CH::RoundedBrush(50.f), PhoneBodyColor)
			]
			+ SOverlay::Slot()
			.Padding(13.f)
			[
				Tinted(CH::RoundedBrush(40.f), ScreenColor)
			]

			// screen content
			+ SOverlay::Slot()
			.Padding(13.f)
			[
				SNew(SBox)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SVerticalBox)

					// status bar
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(26.f, 14.f, 24.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SCHPhoneOverlay::GetClockText)
							.Font(CH::LatinFont(13.f, CH::EWeight::Regular))
							.ColorAndOpacity(FSlateColor(CH::Col::Paper))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(SSpacer)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 10.f, 0.f)
						[
							SNew(STextBlock)
							.Text(this, &SCHPhoneOverlay::GetServiceText)
							.Font(CH::LatinFont(11.f, CH::EWeight::Regular, 60))
							.ColorAndOpacity(this, &SCHPhoneOverlay::GetServiceColor)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							Sized(24.f, 11.f, Tinted(CH::RoundedBrush(3.f), FLinearColor(0.55f, 0.55f, 0.52f, 1.f)))
						]
					]

					// chat or call
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(this, &SCHPhoneOverlay::GetViewIndex)
						+ SWidgetSwitcher::Slot()
						[
							BuildChatView()
						]
						+ SWidgetSwitcher::Slot()
						[
							BuildCallView()
						]
					]
				]
			]

			// news alert drops over the top of the screen
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			.Padding(24.f, 52.f, 24.f, 0.f)
			[
				BuildAlert()
			]
		]
	];
}

TSharedRef<SWidget> SCHPhoneOverlay::BuildChatView()
{
	TSharedRef<SHorizontalBox> Dots = SNew(SHorizontalBox);
	for (int32 i = 0; i < 3; ++i)
	{
		Dots->AddSlot()
			.AutoWidth()
			.Padding(i == 0 ? 0.f : 6.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox).WidthOverride(9.f).HeightOverride(9.f)
				[
					SNew(SImage)
					.Image(CH::RoundedBrush(4.5f))
					.ColorAndOpacity(this, &SCHPhoneOverlay::GetDotColor, i)
				]
			];
	}

	return SNew(SVerticalBox)

		// header: who we are talking to
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(22.f, 10.f, 22.f, 12.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				Sized(40.f, 40.f, Tinted(CH::RoundedBrush(20.f), FLinearColor(0.15f, 0.16f, 0.19f, 1.f)))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(14.f, 0.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
						.Text(this, &SCHPhoneOverlay::GetContactFa)
						.Font(CH::FarsiFont(19.f, CH::EWeight::Bold))
						.ColorAndOpacity(FSlateColor(CH::Col::Paper))
						.TextShapingMethod(ETextShapingMethod::FullShaping)
						.TextFlowDirection(ETextFlowDirection::RightToLeft)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
						.Text(this, &SCHPhoneOverlay::GetContactEn)
						.Font(CH::LatinFont(16.f, CH::EWeight::Bold, 120))
						.ColorAndOpacity(FSlateColor(CH::Col::Paper))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SCHPhoneOverlay::GetStatusText)
					.Font(CH::LatinFont(11.f, CH::EWeight::Regular))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.35f, 0.55f, 0.42f, 1.f)))
				]
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			Sized(BodyWidth, 1.f, Tinted(CH::WhiteBrush(), FLinearColor(1.f, 1.f, 1.f, 0.06f)))
		]

		// messages
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(16.f, 10.f, 16.f, 0.f)
		[
			SAssignNew(Scroll, SScrollBox)
			.ScrollBarVisibility(EVisibility::Collapsed)
			.ConsumeMouseWheel(EConsumeMouseWheel::Never)
			.AnimateWheelScrolling(false)
			+ SScrollBox::Slot()
			[
				SAssignNew(MessageBox, SVerticalBox)
			]
		]

		// "typing..." dots
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.Padding(16.f, 8.f, 16.f, 0.f)
		[
			SNew(SBorder)
			.BorderImage(CH::RoundedBrush(17.f))
			.BorderBackgroundColor(FSlateColor(TheirBubble))
			.Padding(FMargin(16.f, 13.f))
			.Visibility(this, &SCHPhoneOverlay::GetDotsVisibility)
			[
				Dots
			]
		]

		// input field
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(16.f, 12.f, 16.f, 22.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(CH::RoundedBrush(22.f))
				.BorderBackgroundColor(FSlateColor(FieldColor))
				.Padding(FMargin(18.f, 10.f))
				[
					SNew(SBox)
					.MinDesiredHeight(26.f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
							.Text(this, &SCHPhoneOverlay::GetDraftText)
							.Font(CH::FarsiFont(17.f))
							.ColorAndOpacity(this, &SCHPhoneOverlay::GetDraftColor)
							.WrapTextAt(BubbleWrap + 20.f)
							.TextShapingMethod(ETextShapingMethod::FullShaping)
							.TextFlowDirection(ETextFlowDirection::RightToLeft)
						]
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
							.Text(this, &SCHPhoneOverlay::GetDraftText)
							.Font(CH::LatinFont(15.f))
							.ColorAndOpacity(this, &SCHPhoneOverlay::GetDraftColor)
							.WrapTextAt(BubbleWrap + 20.f)
						]
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Bottom)
			.Padding(10.f, 0.f, 0.f, 2.f)
			[
				Sized(42.f, 42.f, Tinted(CH::RoundedBrush(21.f), FLinearColor(0.035f, 0.17f, 0.14f, 1.f)))
			]
		];
}

TSharedRef<SWidget> SCHPhoneOverlay::BuildCallView()
{
	const float W = BodyWidth - 26.f;

	return SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SCHGradient)
			.StartColor(FLinearColor(0.060f, 0.080f, 0.110f, 1.f))
			.EndColor(FLinearColor(0.014f, 0.018f, 0.026f, 1.f))
		]

		// the other side of the call: only a silhouette
		+ SOverlay::Slot()
		[
			SAssignNew(CallFigure, SCanvas)
			+ SCanvas::Slot()
			.Position(FVector2D((W - 340.f) * 0.5f, 500.f))
			.Size(FVector2D(340.f, 420.f))
			[
				Tinted(CH::RoundedBrush(120.f), FigureColor)
			]
			+ SCanvas::Slot()
			.Position(FVector2D((W - 46.f) * 0.5f, 430.f))
			.Size(FVector2D(46.f, 90.f))
			[
				Tinted(CH::WhiteBrush(), FigureColor)
			]
			+ SCanvas::Slot()
			.Position(FVector2D((W - 164.f) * 0.5f, 290.f))
			.Size(FVector2D(164.f, 172.f))
			[
				Tinted(CH::RoundedBrush(82.f), FigureColor)
			]
		]

		// dark foot so the controls read
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		[
			SNew(SBox)
			.HeightOverride(260.f)
			[
				SNew(SCHGradient)
				.StartColor(FLinearColor(0.f, 0.f, 0.f, 0.f))
				.EndColor(FLinearColor(0.f, 0.f, 0.f, 0.92f))
			]
		]

		// name and timer
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 44.f, 0.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
				.Text(this, &SCHPhoneOverlay::GetContactFa)
				.Font(CH::FarsiFont(32.f, CH::EWeight::Bold))
				.ColorAndOpacity(FSlateColor(CH::Col::Paper))
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(ETextFlowDirection::RightToLeft)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
				.Text(this, &SCHPhoneOverlay::GetContactEn)
				.Font(CH::LatinFont(22.f, CH::EWeight::Bold, 320))
				.ColorAndOpacity(FSlateColor(CH::Col::Paper))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(this, &SCHPhoneOverlay::GetCallTimer)
				.Font(CH::LatinFont(14.f, CH::EWeight::Regular, 120))
				.ColorAndOpacity(FSlateColor(CH::Col::PaperDim))
			]
		]

		// his own picture-in-picture: dark, he is never shown
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(0.f, 150.f, 18.f, 0.f)
		[
			Sized(86.f, 122.f, Tinted(CH::RoundedBrush(14.f), FLinearColor(0.012f, 0.014f, 0.018f, 1.f)))
		]

		// frozen picture
		+ SOverlay::Slot()
		[
			SNew(SOverlay)
			.Visibility(this, &SCHPhoneOverlay::GetFrozenVisibility)
			+ SOverlay::Slot()
			[
				Tinted(CH::WhiteBrush(), FLinearColor(0.f, 0.f, 0.f, 0.45f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					FaText(TEXT("اتصال ضعیف است…"), 19.f, CH::Col::Paper, 340.f)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					EnText(TEXT("POOR CONNECTION"), 14.f, CH::Col::Paper, 340.f, 260)
				]
			]
		]

		// call controls
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 0.f, 46.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				Sized(54.f, 54.f, Tinted(CH::RoundedBrush(27.f), FLinearColor(0.16f, 0.17f, 0.19f, 1.f)))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(34.f, 0.f)
			[
				Sized(64.f, 64.f, Tinted(CH::RoundedBrush(32.f), FLinearColor(0.62f, 0.05f, 0.04f, 1.f)))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				Sized(54.f, 54.f, Tinted(CH::RoundedBrush(27.f), FLinearColor(0.16f, 0.17f, 0.19f, 1.f)))
			]
		];
}

TSharedRef<SWidget> SCHPhoneOverlay::BuildAlert()
{
	return SAssignNew(AlertCard, SBorder)
		.BorderImage(CH::RoundedBrush(20.f))
		.BorderBackgroundColor(FSlateColor(FLinearColor(0.115f, 0.118f, 0.132f, 0.985f)))
		.Padding(FMargin(18.f, 14.f, 18.f, 16.f))
		.Visibility(this, &SCHPhoneOverlay::GetAlertVisibility)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					Sized(9.f, 9.f, Tinted(CH::RoundedBrush(4.5f), CH::Col::Rec))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(9.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(this, &SCHPhoneOverlay::GetAlertSource)
					.Font(CH::LatinFont(11.f, CH::EWeight::Regular, 260))
					.ColorAndOpacity(FSlateColor(CH::Col::PaperDim))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
				.Text(this, &SCHPhoneOverlay::GetAlertFa)
				.Font(CH::FarsiFont(19.f, CH::EWeight::Bold))
				.ColorAndOpacity(FSlateColor(CH::Col::Paper))
				.WrapTextAt(340.f)
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(ETextFlowDirection::RightToLeft)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Visibility_Lambda([]() { return CHSettings::IsPersian() ? EVisibility::Collapsed : EVisibility::HitTestInvisible; })
				.Text(this, &SCHPhoneOverlay::GetAlertEn)
				.Font(CH::LatinFont(16.f, CH::EWeight::Bold))
				.ColorAndOpacity(FSlateColor(CH::Col::Paper))
				.WrapTextAt(340.f)
			]
		];
}

// ---------------------------------------------------------------------------------------------
// Messages
// ---------------------------------------------------------------------------------------------

TSharedRef<SWidget> SCHPhoneOverlay::MakeMessageWidget(int32 Index)
{
	const FCHPhoneMessage& Message = Messages[Index];

	if (Message.Kind == FCHPhoneMessage::EKind::Note)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				FaText(Message.Fa, 13.f, CH::Col::PaperDim, 320.f)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				EnText(Message.En, 12.f, CH::Col::PaperDim, 320.f, 160)
			];
	}

	if (Message.Kind == FCHPhoneMessage::EKind::Transcript)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.f, 2.f, 12.f, 2.f)
			[
				SNew(SBox).WidthOverride(2.f)
				[
					Tinted(CH::WhiteBrush(), FLinearColor(0.95f, 0.30f, 0.08f, 0.55f))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					FaText(Message.Fa, 18.f, CH::Col::Paper, BubbleWrap + 30.f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					EnText(Message.En, 15.f, CH::Col::Paper, BubbleWrap + 30.f)
				]
			];
	}

	TSharedPtr<SWidget> Content;
	if (Message.Kind == FCHPhoneMessage::EKind::Voice)
	{
		Content = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				Sized(30.f, 30.f, Tinted(CH::RoundedBrush(15.f), FLinearColor(0.80f, 0.78f, 0.72f, 1.f)))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(12.f, 0.f)
			[
				SNew(SBox).WidthOverride(176.f).HeightOverride(30.f)
				[
					SNew(SCHWaveform)
					.Bars(38)
					.Seed(47)
					.Progress(this, &SCHPhoneOverlay::GetVoiceProgress)
					.Color(FLinearColor(0.32f, 0.33f, 0.36f, 1.f))
					.PlayedColor(CH::Col::Paper)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				EnText(Message.En, 12.f, CH::Col::PaperDim, 80.f, 0, true)
			];
	}
	else
	{
		TSharedRef<SVerticalBox> Lines = SNew(SVerticalBox);
		if (!Message.Fa.IsEmpty())
		{
			Lines->AddSlot().AutoHeight()
				[
					FaText(Message.Fa, 18.f, CH::Col::Paper, BubbleWrap)
				];
		}
		if (!Message.En.IsEmpty())
		{
			Lines->AddSlot().AutoHeight()
				[
					EnText(Message.En, 15.f, CH::Col::Paper, BubbleWrap)
				];
		}
		Content = Lines;
	}

	TSharedRef<SVerticalBox> Stack = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(Message.bMine ? HAlign_Right : HAlign_Left)
		[
			SNew(SBorder)
			.BorderImage(CH::RoundedBrush(18.f))
			.BorderBackgroundColor(FSlateColor(Message.bMine ? MyBubble : TheirBubble))
			.Padding(FMargin(15.f, 9.f, 15.f, 10.f))
			[
				Content.ToSharedRef()
			]
		];

	if (Message.bSent)
	{
		Stack->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0.f, 3.f, 6.f, 0.f)
			[
				EnText(TEXT("SENT  ✓"), 10.f, FLinearColor(0.35f, 0.55f, 0.42f, 1.f), 200.f, 200, true)
			];
	}
	return Stack;
}

void SCHPhoneOverlay::AddMessage(const FCHPhoneMessage& Message)
{
	const int32 Index = Messages.Add(Message);
	if (MessageBox.IsValid())
	{
		TSharedRef<SWidget> Widget = MakeMessageWidget(Index);
		Widget->SetRenderOpacity(0.f);
		MessageWidgets.Add(Widget);
		MessageBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, Index == 0 ? 0.f : 9.f, 0.f, 0.f)
			[
				Widget
			];
	}
	if (Scroll.IsValid())
	{
		Scroll->ScrollToEnd();
	}
}

void SCHPhoneOverlay::RebuildMessages()
{
	MessageWidgets.Reset();
	if (MessageBox.IsValid())
	{
		MessageBox->ClearChildren();
	}
	const TArray<FCHPhoneMessage> Old = MoveTemp(Messages);
	Messages.Reset();
	for (const FCHPhoneMessage& Message : Old)
	{
		AddMessage(Message);
	}
}

// ---------------------------------------------------------------------------------------------
// Operations
// ---------------------------------------------------------------------------------------------

void SCHPhoneOverlay::OpenChat(const FString& InContactFa, const FString& InContactEn, bool bClearMessages)
{
	ContactFa = InContactFa;
	ContactEn = InContactEn.ToUpper();
	StatusFa.Reset();
	StatusEn = TEXT("online");
	bCall = false;
	bOpen = true;
	if (bClearMessages)
	{
		Messages.Reset();
		RebuildMessages();
	}
	PendingFa.Reset();
	PendingEn.Reset();
	DraftChars = 0.f;
	Action = EAction::None;
}

void SCHPhoneOverlay::OpenCall(const FString& InContactFa, const FString& InContactEn)
{
	ContactFa = InContactFa;
	ContactEn = InContactEn.ToUpper();
	bCall = true;
	bCallFrozen = false;
	bOpen = true;
	Action = EAction::None;
}

void SCHPhoneOverlay::Close()
{
	bOpen = false;
	bAlert = false;
}

bool SCHPhoneOverlay::IsBusy() const
{
	if (Action != EAction::None || SettleTime > 0.f)
	{
		return true;
	}
	return bOpen ? OpenAlpha < 0.97f : OpenAlpha > 0.03f;
}

void SCHPhoneOverlay::Receive(const FString& Fa, const FString& En, float TypingSeconds)
{
	PendingFa = Fa;
	PendingEn = En;
	Action = EAction::Receiving;
	ActionTime = 0.f;
	ActionDuration = FMath::Max(0.f, TypingSeconds);
	StatusEn = TEXT("typing…");
}

void SCHPhoneOverlay::TypeDraft(const FString& Fa, const FString& En, bool bSend, float HoldSeconds)
{
	PendingFa = Fa;
	PendingEn = En;
	bPendingSend = bSend;
	DraftChars = 0.f;
	LastDraftCount = 0;
	Action = EAction::DraftTyping;
	ActionTime = 0.f;
	ActionDuration = HoldSeconds;
}

void SCHPhoneOverlay::AddNote(const FString& Fa, const FString& En)
{
	FCHPhoneMessage Message;
	Message.Kind = FCHPhoneMessage::EKind::Note;
	Message.Fa = Fa;
	Message.En = En;
	AddMessage(Message);
}

void SCHPhoneOverlay::SetStatus(const FString& Fa, const FString& En)
{
	StatusFa = Fa;
	StatusEn = En;
}

void SCHPhoneOverlay::StartVoice(const FString& DurationText, int32 NumLines)
{
	FCHPhoneMessage Message;
	Message.Kind = FCHPhoneMessage::EKind::Voice;
	Message.En = DurationText;
	AddMessage(Message);
	VoiceDuration = DurationText;
	VoiceLines = FMath::Max(1, NumLines);
	VoiceShown = 0;
	VoiceProgress = 0.f;
	bVoiceActive = true;
	SettleTime = 0.8f;
}

void SCHPhoneOverlay::AddTranscriptLine(const FString& Fa, const FString& En)
{
	FCHPhoneMessage Message;
	Message.Kind = FCHPhoneMessage::EKind::Transcript;
	Message.Fa = Fa;
	Message.En = En;
	AddMessage(Message);
	++VoiceShown;
	SettleTime = 0.5f;
}

void SCHPhoneOverlay::EndVoice()
{
	bVoiceActive = false;
	VoiceShown = VoiceLines;
}

void SCHPhoneOverlay::ShowAlert(const FString& SourceFa, const FString& SourceEn, const FString& Fa, const FString& En)
{
	AlertSourceFa = SourceFa;
	AlertSourceEn = SourceEn.ToUpper();
	AlertFa = Fa;
	AlertEn = En;
	bAlert = true;
	SettleTime = 0.7f;
}

void SCHPhoneOverlay::HideAlert()
{
	bAlert = false;
}

bool SCHPhoneOverlay::ConsumeTypeTick()
{
	const bool bTick = bTypeTick;
	bTypeTick = false;
	return bTick;
}

// ---------------------------------------------------------------------------------------------
// Animation
// ---------------------------------------------------------------------------------------------

void SCHPhoneOverlay::Advance(float DeltaTime)
{
	Time += DeltaTime;

	OpenAlpha = FMath::Clamp(OpenAlpha + (bOpen ? DeltaTime / 0.5f : -DeltaTime / 0.4f), 0.f, 1.f);
	if (PhoneBody.IsValid())
	{
		const float Eased = FMath::InterpEaseOut(0.f, 1.f, OpenAlpha, 2.4f);
		const float Offset = (1.f - Eased) * 1040.f + FMath::Sin(Time * 0.9f) * 3.f;
		const float Angle = FMath::DegreesToRadians(-1.6f + FMath::Sin(Time * 0.6f) * 0.35f);
		PhoneBody->SetRenderTransformPivot(FVector2D(0.5, 0.5));
		PhoneBody->SetRenderTransform(TOptional<FSlateRenderTransform>(FSlateRenderTransform(FQuat2f(Angle), FVector2f(0.f, Offset))));
		PhoneBody->SetVisibility(OpenAlpha > 0.f ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
	}

	if (bCall && bOpen && !bCallFrozen)
	{
		CallSeconds += DeltaTime;
	}
	if (CallFigure.IsValid())
	{
		const float Sway = bCallFrozen ? 0.f : 1.f;
		CallFigure->SetRenderTransform(TOptional<FSlateRenderTransform>(FSlateRenderTransform(
			FVector2f(FMath::Sin(Time * 0.8f) * 5.f * Sway, FMath::Sin(Time * 1.3f) * 3.f * Sway))));
	}

	AlertAlpha = FMath::Clamp(AlertAlpha + (bAlert ? DeltaTime / 0.45f : -DeltaTime / 0.35f), 0.f, 1.f);
	if (AlertCard.IsValid())
	{
		const float Eased = FMath::InterpEaseOut(0.f, 1.f, AlertAlpha, 2.2f);
		AlertCard->SetRenderOpacity(Eased);
		AlertCard->SetRenderTransform(TOptional<FSlateRenderTransform>(FSlateRenderTransform(FVector2f(0.f, (1.f - Eased) * -70.f))));
	}

	for (int32 Index = 0; Index < Messages.Num(); ++Index)
	{
		FCHPhoneMessage& Message = Messages[Index];
		if (Message.Age < 1.f)
		{
			Message.Age += DeltaTime;
			if (MessageWidgets.IsValidIndex(Index) && MessageWidgets[Index].IsValid())
			{
				MessageWidgets[Index]->SetRenderOpacity(FMath::Clamp(Message.Age / 0.35f, 0.f, 1.f));
			}
		}
	}

	const float VoiceTarget = VoiceLines > 0 ? static_cast<float>(VoiceShown) / VoiceLines : 0.f;
	VoiceProgress = FMath::FInterpConstantTo(VoiceProgress, bVoiceActive || VoiceShown > 0 ? VoiceTarget : 0.f, DeltaTime, 0.09f);

	if (SettleTime > 0.f)
	{
		SettleTime = FMath::Max(0.f, SettleTime - DeltaTime);
	}

	const FString ActiveDraft = CHSettings::IsPersian() ? PendingFa : CHSettings::Latin(PendingEn);
	const int32 MaxDraftLen = FMath::Max(1, ActiveDraft.Len());

	switch (Action)
	{
	case EAction::Receiving:
		ActionTime += DeltaTime;
		if (ActionTime >= ActionDuration)
		{
			FCHPhoneMessage Message;
			Message.Fa = PendingFa;
			Message.En = PendingEn;
			AddMessage(Message);
			PendingFa.Reset();
			PendingEn.Reset();
			StatusEn = TEXT("online");
			Action = EAction::None;
			SettleTime = 0.5f;
		}
		break;

	case EAction::DraftTyping:
	{
		DraftChars = FMath::Min(static_cast<float>(MaxDraftLen), DraftChars + DeltaTime * 14.f);
		const int32 Count = FMath::FloorToInt32(DraftChars);
		if (Count != LastDraftCount)
		{
			LastDraftCount = Count;
			bTypeTick = true;
		}
		if (Count >= MaxDraftLen)
		{
			Action = bPendingSend ? EAction::DraftSending : EAction::DraftHold;
			ActionTime = 0.f;
		}
		break;
	}

	case EAction::DraftHold:
		ActionTime += DeltaTime;
		if (ActionTime >= ActionDuration)
		{
			Action = EAction::DraftDeleting;
		}
		break;

	case EAction::DraftDeleting:
	{
		DraftChars = FMath::Max(0.f, DraftChars - DeltaTime * 28.f);
		const int32 Count = FMath::CeilToInt32(DraftChars);
		if (Count != LastDraftCount)
		{
			LastDraftCount = Count;
			bTypeTick = true;
		}
		if (DraftChars <= 0.f)
		{
			PendingFa.Reset();
			PendingEn.Reset();
			Action = EAction::None;
			SettleTime = 0.7f;
		}
		break;
	}

	case EAction::DraftSending:
		ActionTime += DeltaTime;
		if (ActionTime >= 0.6f)
		{
			FCHPhoneMessage Message;
			Message.bMine = true;
			Message.bSent = true;
			Message.Fa = PendingFa;
			Message.En = PendingEn;
			AddMessage(Message);
			PendingFa.Reset();
			PendingEn.Reset();
			DraftChars = 0.f;
			Action = EAction::None;
			SettleTime = 0.9f;
		}
		break;

	default:
		break;
	}
}

// ---------------------------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------------------------

int32 SCHPhoneOverlay::GetViewIndex() const
{
	return bCall ? 1 : 0;
}

FText SCHPhoneOverlay::GetContactFa() const
{
	return FText::FromString(ContactFa);
}

FText SCHPhoneOverlay::GetContactEn() const
{
	return FText::FromString(CHSettings::Latin(ContactEn));
}

FText SCHPhoneOverlay::GetStatusText() const
{
	if (CHSettings::IsPersian() && !StatusFa.IsEmpty())
	{
		return FText::FromString(StatusFa);
	}
	return FText::FromString(CHSettings::Latin(StatusEn));
}

FText SCHPhoneOverlay::GetClockText() const
{
	return FText::FromString(Clock);
}

FText SCHPhoneOverlay::GetServiceText() const
{
	if (bNoService)
	{
		return FText::FromString(CHSettings::IsGerman() ? TEXT("KEIN NETZ") : TEXT("NO SERVICE"));
	}
	return FText::FromString(TEXT("4G"));
}

FSlateColor SCHPhoneOverlay::GetServiceColor() const
{
	if (bNoService)
	{
		FLinearColor Color = CH::Col::Rec;
		Color.A = 0.65f + 0.35f * FMath::Cos(Time * 4.f);
		return FSlateColor(Color);
	}
	return FSlateColor(CH::Col::PaperDim);
}

FText SCHPhoneOverlay::GetDraftText() const
{
	const bool bDrafting = Action == EAction::DraftTyping || Action == EAction::DraftHold || Action == EAction::DraftDeleting || Action == EAction::DraftSending;
	if (!bDrafting)
	{
		if (CHSettings::IsPersian())
		{
			return FText::FromString(TEXT("پیام…"));
		}
		return FText::FromString(CHSettings::IsGerman() ? TEXT("Nachricht…") : TEXT("Message…"));
	}
	const FString ActiveDraft = CHSettings::IsPersian() ? PendingFa : CHSettings::Latin(PendingEn);
	const int32 Count = FMath::Clamp(Action == EAction::DraftDeleting ? FMath::CeilToInt32(DraftChars) : FMath::FloorToInt32(DraftChars), 0, ActiveDraft.Len());
	return FText::FromString(ActiveDraft.Left(Count));
}

FSlateColor SCHPhoneOverlay::GetDraftColor() const
{
	const bool bDrafting = Action == EAction::DraftTyping || Action == EAction::DraftHold || Action == EAction::DraftDeleting || Action == EAction::DraftSending;
	return FSlateColor(bDrafting ? CH::Col::Paper : CH::Col::Faint);
}

FText SCHPhoneOverlay::GetCallTimer() const
{
	const int32 Seconds = FMath::FloorToInt32(CallSeconds);
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60));
}

FText SCHPhoneOverlay::GetAlertSource() const
{
	return FText::FromString(CHSettings::IsPersian() ? AlertSourceFa : CHSettings::Latin(AlertSourceEn));
}

FText SCHPhoneOverlay::GetAlertFa() const
{
	return FText::FromString(AlertFa);
}

FText SCHPhoneOverlay::GetAlertEn() const
{
	return FText::FromString(CHSettings::Latin(AlertEn));
}

FSlateColor SCHPhoneOverlay::GetDotColor(int32 Index) const
{
	const float Phase = Time * 5.f - Index * 0.9f;
	const float Pulse = 0.35f + 0.65f * FMath::Max(0.f, FMath::Sin(Phase));
	return FSlateColor(FLinearColor(0.75f, 0.74f, 0.70f, Pulse));
}

EVisibility SCHPhoneOverlay::GetDotsVisibility() const
{
	return Action == EAction::Receiving ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

EVisibility SCHPhoneOverlay::GetFrozenVisibility() const
{
	return bCallFrozen ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

EVisibility SCHPhoneOverlay::GetAlertVisibility() const
{
	return AlertAlpha > 0.f ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

float SCHPhoneOverlay::GetVoiceProgress() const
{
	return VoiceProgress;
}
