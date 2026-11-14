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

/**
 * His right eye closing: the right half of the picture goes dark from the outer edge inwards,
 * with a soft, slightly trembling edge. Close 0 = open, 1 = the whole right half is black.
 */
class SCHEyeShade : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCHEyeShade) : _Close(0.f) {}
		SLATE_ATTRIBUTE(float, Close)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(8.0, 8.0); }

private:
	TAttribute<float> Close;
};

/** Engraved on his stone: a sun rising over mountains. */
class SCHSunEngraving : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCHSunEngraving) : _Color(FLinearColor::Black) {}
		SLATE_ARGUMENT(FLinearColor, Color)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs) { Color = InArgs._Color; SetVisibility(EVisibility::HitTestInvisible); }

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(260.0, 120.0); }

private:
	FLinearColor Color;
};

/** An eye: two lids and an iris. Used as the "being seen" meter. */
class SCHEye : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCHEye)
		: _Open(1.f)
		, _Fill(0.f)
		, _LineColor(FLinearColor::White)
		, _IrisColor(FLinearColor::White)
	{
	}
		/** 0 = closed, 1 = wide open. */
		SLATE_ATTRIBUTE(float, Open)
		/** How solid the iris is (0..1). */
		SLATE_ATTRIBUTE(float, Fill)
		SLATE_ATTRIBUTE(FLinearColor, LineColor)
		SLATE_ATTRIBUTE(FLinearColor, IrisColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(84.0, 40.0); }

private:
	TAttribute<float> Open;
	TAttribute<float> Fill;
	TAttribute<FLinearColor> LineColor;
	TAttribute<FLinearColor> IrisColor;
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
