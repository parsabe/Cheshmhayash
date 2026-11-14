// Cheshm Hayash - tiny custom-painted widgets.

#include "UI/SCHShapes.h"
#include "UI/CHStyle.h"
#include "Math/RandomStream.h"
#include "Rendering/DrawElements.h"

void SCHGradient::Construct(const FArguments& InArgs)
{
	StartColor = InArgs._StartColor;
	EndColor = InArgs._EndColor;
	bTopToBottom = InArgs._TopToBottom;
	SetVisibility(EVisibility::HitTestInvisible);
}

int32 SCHGradient::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FLinearColor Start = StartColor.Get();
	const FLinearColor End = EndColor.Get();
	if (Start.A <= 0.f && End.A <= 0.f)
	{
		return LayerId;
	}

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	TArray<FSlateGradientStop> Stops;
	Stops.Emplace(FVector2D::ZeroVector, Start);
	Stops.Emplace(Size, End);

	// Orient_Horizontal draws horizontal bands (the colour changes from top to bottom).
	FSlateDrawElement::MakeGradient(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), MoveTemp(Stops),
		bTopToBottom ? Orient_Horizontal : Orient_Vertical);
	return LayerId;
}

void SCHWaveform::Construct(const FArguments& InArgs)
{
	Progress = InArgs._Progress;
	Color = InArgs._Color;
	PlayedColor = InArgs._PlayedColor;
	SetVisibility(EVisibility::HitTestInvisible);

	FRandomStream Rng(InArgs._Seed);
	const int32 Count = FMath::Max(4, InArgs._Bars);
	Heights.Reserve(Count);
	for (int32 i = 0; i < Count; ++i)
	{
		// speech-like envelope: bursts and pauses
		const float Envelope = 0.35f + 0.65f * FMath::Abs(FMath::Sin(i * 0.61f + Rng.FRand() * 0.8f));
		Heights.Add(FMath::Clamp(Envelope * Rng.FRandRange(0.25f, 1.f), 0.12f, 1.f));
	}
}

int32 SCHWaveform::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const int32 Count = Heights.Num();
	if (Count == 0 || Size.X <= 0.0)
	{
		return LayerId;
	}

	const float Step = static_cast<float>(Size.X) / Count;
	const float BarWidth = FMath::Max(1.5f, Step * 0.55f);
	const float Played = Progress.Get();
	const FLinearColor Base = Color.Get();
	const FLinearColor Lit = PlayedColor.Get();

	for (int32 i = 0; i < Count; ++i)
	{
		const float BarHeight = FMath::Max(2.f, Heights[i] * static_cast<float>(Size.Y));
		const FVector2D BarSize(BarWidth, BarHeight);
		const FVector2D BarPos(i * Step, (Size.Y - BarHeight) * 0.5);
		const bool bPlayed = (static_cast<float>(i) + 0.5f) / Count <= Played;
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(BarSize, FSlateLayoutTransform(BarPos)),
			CH::WhiteBrush(), ESlateDrawEffect::None, bPlayed ? Lit : Base);
	}
	return LayerId;
}
