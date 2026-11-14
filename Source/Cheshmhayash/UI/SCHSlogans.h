// Cheshm Hayash - words shouted from windows and rooftops at night (1.7). There are no voices,
// so the shout is written: one line at the edge of the screen, then another, then the whole
// screen full of them.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SConstraintCanvas;

class SCHSlogans : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCHSlogans) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Count lines appear over Seconds, slowly at first and then all at once. */
	void Start(const FString& Text, int32 Count, float Seconds);
	/** Fades everything out. */
	void Stop(float FadeSeconds = 2.f);
	void Advance(float DeltaTime);

private:
	TSharedPtr<SConstraintCanvas> Canvas;
	TArray<float> AppearAt;
	float Time = 0.f;
	float Alpha = 0.f;
	float FadeRate = 0.f;
	bool bActive = false;
};
