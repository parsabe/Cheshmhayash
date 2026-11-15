// Cheshm Hayash - shared UI fonts, brushes and palette.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"

namespace CH
{
	enum class EWeight : uint8
	{
		Light,
		Regular,
		Bold
	};

	/** Vazirmatn (Persian). Falls back to the engine font (which has an Arabic-script fallback) if the font is missing. */
	FSlateFontInfo FarsiFont(float Size, EWeight Weight = EWeight::Regular);

	/** Roboto (Latin). LetterSpacing is in 1/1000 em. */
	FSlateFontInfo LatinFont(float Size, EWeight Weight = EWeight::Regular, int32 LetterSpacing = 0);

	/** Solid white brush, tint it with ColorAndOpacity. */
	const FSlateBrush* WhiteBrush();

	/** White box with rounded corners (cached per radius), tint it with ColorAndOpacity. */
	const FSlateBrush* RoundedBrush(float Radius);

	/** Outline only, no fill (a ring when the radius is half the size). */
	const FSlateBrush* RingBrush(float Radius, float Thickness, const FLinearColor& Color);

	/** Releases cached font objects (call when the game instance shuts down). */
	void ShutdownStyle();

	/**
	 * Text that stays readable on whatever is behind it. The root overlay measures how bright the
	 * picture is (white fades, bright daylight); light text turns dark on a bright background and
	 * its shadow turns light.
	 */
	void SetBackdrop(float Brightness);
	float GetBackdrop();
	FLinearColor Readable(const FLinearColor& Color);
	/** The shadow / halo that separates text from the background. */
	FLinearColor ReadableShadow(float Alpha);

	/** Converts latin digits to Persian digits (0-9 -> ۰-۹). */
	FString PersianDigits(const FString& In);

	// Palette (linear colours)
	namespace Col
	{
		inline const FLinearColor Paper(0.90f, 0.86f, 0.78f, 1.f);     // main text, warm off-white
		inline const FLinearColor PaperDim(0.55f, 0.53f, 0.50f, 1.f);  // secondary text (English lines)
		inline const FLinearColor Faint(0.28f, 0.27f, 0.26f, 1.f);     // hints
		inline const FLinearColor Ember(0.95f, 0.30f, 0.08f, 1.f);     // fire orange
		inline const FLinearColor Rec(0.90f, 0.03f, 0.02f, 1.f);       // REC red
		inline const FLinearColor Ink(0.f, 0.f, 0.f, 1.f);
	}
}
