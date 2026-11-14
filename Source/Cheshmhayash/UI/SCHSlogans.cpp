// Cheshm Hayash - the shouted words, written.

#include "UI/SCHSlogans.h"
#include "Math/RandomStream.h"
#include "UI/CHStyle.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Text/STextBlock.h"

void SCHSlogans::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::HitTestInvisible);
	ChildSlot
	[
		SAssignNew(Canvas, SConstraintCanvas)
	];
}

void SCHSlogans::Start(const FString& Text, int32 Count, float Seconds)
{
	Canvas->ClearChildren();
	AppearAt.Reset();
	Time = 0.f;
	Alpha = 1.f;
	FadeRate = 0.f;
	bActive = true;

	FRandomStream Rng(1401);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		// the first voice is small and far away, at the edge of the screen
		const FVector2D Where = Index == 0 ? FVector2D(0.84, 0.86) : FVector2D(Rng.FRandRange(0.06f, 0.94f), Rng.FRandRange(0.06f, 0.94f));
		const float Size = Index == 0 ? 17.f : Rng.FRandRange(14.f, 15.f + 26.f * Index / Count);
		const float Shade = Rng.FRandRange(0.45f, 1.f);
		AppearAt.Add(Seconds * FMath::Pow(static_cast<float>(Index) / FMath::Max(1, Count - 1), 0.4f));

		Canvas->AddSlot()
			.Anchors(FAnchors(Where.X, Where.Y))
			.Alignment(FVector2D(0.5, 0.5))
			.AutoSize(true)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Text))
				.Font(CH::FarsiFont(Size, Index % 3 == 0 ? CH::EWeight::Bold : CH::EWeight::Regular))
				.TextShapingMethod(ETextShapingMethod::FullShaping)
				.TextFlowDirection(ETextFlowDirection::RightToLeft)
				.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([this, Index, Shade]()
				{
					const float Shown = AppearAt.IsValidIndex(Index) ? FMath::Clamp((Time - AppearAt[Index]) / 0.7f, 0.f, 1.f) : 0.f;
					return FSlateColor(FLinearColor(0.92f * Shade, 0.90f * Shade, 0.86f * Shade, Shown * Alpha));
				}))
			];
	}
}

void SCHSlogans::Stop(float FadeSeconds)
{
	FadeRate = FadeSeconds > 0.f ? 1.f / FadeSeconds : 100.f;
}

void SCHSlogans::Advance(float DeltaTime)
{
	if (!bActive)
	{
		return;
	}
	Time += DeltaTime;
	if (FadeRate > 0.f)
	{
		Alpha = FMath::Max(0.f, Alpha - DeltaTime * FadeRate);
		if (Alpha <= 0.f)
		{
			bActive = false;
			Canvas->ClearChildren();
			AppearAt.Reset();
		}
	}
}
