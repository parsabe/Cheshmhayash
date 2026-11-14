// Cheshm Hayash - tiny custom-painted widgets: gradients, a blinking dot, an animated waveform.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SLeafWidget.h"

/** A two-colour linear gradient filling its area. */
class SCHGradient : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCHGradient)
		: _StartColor(FLinearColor::Transparent)
		, _EndColor(FLinearColor::Black)
		, _TopToBottom(true)
	{
	}
		SLATE_ATTRIBUTE(FLinearColor, StartColor)
		SLATE_ATTRIBUTE(FLinearColor, EndColor)
		/** true: start at the top, end at the bottom. false: left to right. */
		SLATE_ARGUMENT(bool, TopToBottom)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(8.0, 8.0); }

private:
	TAttribute<FLinearColor> StartColor;
	TAttribute<FLinearColor> EndColor;
	bool bTopToBottom = true;
};

/** Vertical bars like a voice-message waveform. Progress (0..1) tints the played part. */
class SCHWaveform : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCHWaveform)
		: _Bars(34)
		, _Seed(7)
		, _Progress(0.f)
		, _Color(FLinearColor(0.4f, 0.4f, 0.4f, 1.f))
		, _PlayedColor(FLinearColor::White)
	{
	}
		SLATE_ARGUMENT(int32, Bars)
		SLATE_ARGUMENT(int32, Seed)
		SLATE_ATTRIBUTE(float, Progress)
		SLATE_ATTRIBUTE(FLinearColor, Color)
		SLATE_ATTRIBUTE(FLinearColor, PlayedColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(200.0, 30.0); }

private:
	TArray<float> Heights;
	TAttribute<float> Progress;
	TAttribute<FLinearColor> Color;
	TAttribute<FLinearColor> PlayedColor;
};
