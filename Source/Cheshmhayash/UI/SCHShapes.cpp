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

void SCHEyeShade::Construct(const FArguments& InArgs)
{
	Close = InArgs._Close;
	SetVisibility(EVisibility::HitTestInvisible);
}

int32 SCHEyeShade::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const float Amount = FMath::Clamp(Close.Get(), 0.f, 1.f);
	if (Amount <= 0.f)
	{
		return LayerId;
	}
	const FVector2f Size = FVector2f(AllottedGeometry.GetLocalSize());
	const float Soft = Size.X * 0.09f;
	// His LEFT eye (as in the poem): the dark comes from the left edge and reaches the centre line
	// when fully closed; the soft edge sits just outside it
	const float EdgeX = (Size.X * 0.5f + Soft) * Amount;
	const float SolidW = EdgeX - Soft;

	if (SolidW > 0.f)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(FVector2f(SolidW, Size.Y), FSlateLayoutTransform(FVector2f(0.f, 0.f))),
			CH::WhiteBrush(), ESlateDrawEffect::None, FLinearColor::Black);
	}
	TArray<FSlateGradientStop> Stops;
	Stops.Emplace(FVector2D(0.0, 0.0), FLinearColor(0.f, 0.f, 0.f, 1.f));
	Stops.Emplace(FVector2D(Soft, 0.0), FLinearColor(0.f, 0.f, 0.f, 0.f));
	FSlateDrawElement::MakeGradient(OutDrawElements, LayerId,
		AllottedGeometry.ToPaintGeometry(FVector2f(Soft, Size.Y), FSlateLayoutTransform(FVector2f(FMath::Max(0.f, SolidW), 0.f))),
		MoveTemp(Stops), Orient_Vertical);
	return LayerId;
}
int32 SCHSunEngraving::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2f Size = FVector2f(AllottedGeometry.GetLocalSize());
	const FVector2f Sun(Size.X * 0.56f, Size.Y * 0.72f);
	const float Radius = Size.Y * 0.30f;
	const float Thick = FMath::Max(2.f, Size.Y * 0.03f);

	// the half sun on the horizon
	TArray<FVector2f> Arc;
	for (int32 Step = 0; Step <= 20; ++Step)
	{
		const float Angle = UE_PI * Step / 20.f;
		Arc.Add(Sun + FVector2f(-FMath::Cos(Angle) * Radius, -FMath::Sin(Angle) * Radius));
	}
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Arc, ESlateDrawEffect::None, Color, true, Thick);

	// rays
	for (int32 Ray = 0; Ray < 7; ++Ray)
	{
		const float Angle = UE_PI * (0.1f + 0.8f * Ray / 6.f);
		const FVector2f Dir(-FMath::Cos(Angle), -FMath::Sin(Angle));
		TArray<FVector2f> Line = { Sun + Dir * (Radius * 1.25f), Sun + Dir * (Radius * 1.7f) };
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Line, ESlateDrawEffect::None, Color, true, Thick * 0.8f);
	}

	// the mountains, Damavand the tallest, the sun rising beside it
	TArray<FVector2f> Ridge = {
		FVector2f(0.f, Size.Y * 0.92f), FVector2f(Size.X * 0.14f, Size.Y * 0.60f), FVector2f(Size.X * 0.24f, Size.Y * 0.74f),
		FVector2f(Size.X * 0.36f, Size.Y * 0.30f), FVector2f(Size.X * 0.46f, Size.Y * 0.62f), FVector2f(Size.X * 0.53f, Size.Y * 0.72f),
		FVector2f(Size.X * 0.70f, Size.Y * 0.78f), FVector2f(Size.X * 0.84f, Size.Y * 0.62f), FVector2f(Size.X, Size.Y * 0.88f) };
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Ridge, ESlateDrawEffect::None, Color, true, Thick);
	// snow line on the peak
	TArray<FVector2f> Snow = { FVector2f(Size.X * 0.31f, Size.Y * 0.42f), FVector2f(Size.X * 0.36f, Size.Y * 0.38f), FVector2f(Size.X * 0.41f, Size.Y * 0.44f) };
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Snow, ESlateDrawEffect::None, Color, true, Thick * 0.7f);
	return LayerId;
}

void SCHEye::Construct(const FArguments& InArgs)
{
	Open = InArgs._Open;
	Fill = InArgs._Fill;
	LineColor = InArgs._LineColor;
	IrisColor = InArgs._IrisColor;
	SetVisibility(EVisibility::HitTestInvisible);
}

int32 SCHEye::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FLinearColor Lines = LineColor.Get();
	if (Lines.A <= 0.f)
	{
		return LayerId;
	}
	const FVector2f Size = FVector2f(AllottedGeometry.GetLocalSize());
	const float OpenAmount = FMath::Clamp(Open.Get(), 0.f, 1.f);
	const float HalfHeight = Size.Y * 0.5f * FMath::Max(OpenAmount, 0.06f);
	const float CenterY = Size.Y * 0.5f;

	TArray<FVector2f> Upper;
	TArray<FVector2f> Lower;
	const int32 Steps = 24;
	for (int32 i = 0; i <= Steps; ++i)
	{
		const float T = static_cast<float>(i) / Steps;
		const float Curve = FMath::Sin(T * UE_PI);
		Upper.Add(FVector2f(T * Size.X, CenterY - HalfHeight * Curve));
		Lower.Add(FVector2f(T * Size.X, CenterY + HalfHeight * Curve * 0.85f));
	}
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Upper, ESlateDrawEffect::None, Lines, true, 2.f);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Lower, ESlateDrawEffect::None, Lines, true, 2.f);

	// the iris shows only while the eye is open
	const FLinearColor Iris = IrisColor.Get() * FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(Fill.Get(), 0.f, 1.f));
	const float Radius = FMath::Min(Size.Y * 0.30f, HalfHeight * 0.92f);
	if (Iris.A > 0.f && Radius > 1.f)
	{
		const FVector2f IrisSize(Radius * 2.f, Radius * 2.f);
		const FVector2f IrisPos(Size.X * 0.5f - Radius, CenterY - Radius);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
			AllottedGeometry.ToPaintGeometry(IrisSize, FSlateLayoutTransform(IrisPos)),
			CH::RoundedBrush(Radius), ESlateDrawEffect::None, Iris);
	}
	return LayerId + 1;
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
