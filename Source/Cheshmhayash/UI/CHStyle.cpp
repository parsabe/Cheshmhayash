// Cheshm Hayash - shared UI fonts, brushes and palette.

#include "UI/CHStyle.h"
#include "Cheshmhayash.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/CompositeFont.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	TSharedPtr<FStandaloneCompositeFont> GFarsiFont;
	TSet<FName> GFarsiFaces;
	bool bFarsiFontInitialized = false;

	const TCHAR* WeightName(CH::EWeight Weight)
	{
		switch (Weight)
		{
		case CH::EWeight::Light: return TEXT("Light");
		case CH::EWeight::Bold: return TEXT("Bold");
		default: return TEXT("Regular");
		}
	}

	void InitFarsiFont()
	{
		if (bFarsiFontInitialized)
		{
			return;
		}
		bFarsiFontInitialized = true;

		TSharedRef<FStandaloneCompositeFont> Composite = MakeShared<FStandaloneCompositeFont>();
		int32 NumAssets = 0;
		int32 NumFiles = 0;

		for (const TCHAR* Weight : { TEXT("Regular"), TEXT("Bold"), TEXT("Light") })
		{
			const FName FaceName(Weight);

			// 1) Font Face asset created by Scripts/import_font.py
			const FString PackageName = FString::Printf(TEXT("/Game/FontAssets/Vazirmatn_%s"), Weight);
			if (FPackageName::DoesPackageExist(PackageName))
			{
				const FString ObjectPath = FString::Printf(TEXT("%s.Vazirmatn_%s"), *PackageName, Weight);
				if (UObject* Face = StaticLoadObject(UObject::StaticClass(), nullptr, *ObjectPath))
				{
					FTypefaceEntry Entry(FaceName);
					Entry.Font = FFontData(Face);
					Composite->DefaultTypeface.Fonts.Add(MoveTemp(Entry));
					GFarsiFaces.Add(FaceName);
					++NumAssets;
					continue;
				}
			}

			// 2) Raw .ttf in Content/Fonts
			const FString File = FPaths::ProjectContentDir() / FString::Printf(TEXT("Fonts/Vazirmatn-%s.ttf"), Weight);
			if (FPaths::FileExists(File))
			{
				Composite->DefaultTypeface.AppendFont(FaceName, File, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
				GFarsiFaces.Add(FaceName);
				++NumFiles;
			}
		}

		if (GFarsiFaces.Num() > 0)
		{
			const FString Fallback = FPaths::EngineContentDir() / TEXT("Slate/Fonts/DroidSansFallback.ttf");
			if (FPaths::FileExists(Fallback))
			{
				Composite->FallbackTypeface.Typeface.AppendFont(TEXT("Regular"), Fallback, EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
			}
			GFarsiFont = Composite;
			UE_LOG(LogCheshmhayash, Log, TEXT("Vazirmatn loaded: %d font face assets, %d raw ttf files"), NumAssets, NumFiles);
		}
		else
		{
			UE_LOG(LogCheshmhayash, Warning, TEXT("Vazirmatn not found (Content/Fonts/Vazirmatn-*.ttf). Using the engine font for Farsi."));
		}
	}
}

FSlateFontInfo CH::FarsiFont(float Size, EWeight Weight)
{
	InitFarsiFont();
	if (GFarsiFont.IsValid())
	{
		FName Face(WeightName(Weight));
		if (!GFarsiFaces.Contains(Face))
		{
			Face = GFarsiFaces.Contains(TEXT("Regular")) ? FName(TEXT("Regular")) : *GFarsiFaces.CreateConstIterator();
		}
		return FSlateFontInfo(GFarsiFont, Size, Face);
	}
	return FCoreStyle::GetDefaultFontStyle(WeightName(Weight), Size);
}

FSlateFontInfo CH::LatinFont(float Size, EWeight Weight, int32 LetterSpacing)
{
	FSlateFontInfo Info = FCoreStyle::GetDefaultFontStyle(WeightName(Weight), Size);
	Info.LetterSpacing = LetterSpacing;
	return Info;
}

const FSlateBrush* CH::WhiteBrush()
{
	static const FSlateColorBrush Brush(FLinearColor::White);
	return &Brush;
}

namespace
{
	TMap<int32, TSharedPtr<FSlateRoundedBoxBrush>> GRoundedBrushes;
}

const FSlateBrush* CH::RoundedBrush(float Radius)
{
	const int32 Key = FMath::RoundToInt32(Radius * 2.f);
	TSharedPtr<FSlateRoundedBoxBrush>& Brush = GRoundedBrushes.FindOrAdd(Key);
	if (!Brush.IsValid())
	{
		Brush = MakeShared<FSlateRoundedBoxBrush>(FLinearColor::White, Radius);
	}
	return Brush.Get();
}

const FSlateBrush* CH::RingBrush(float Radius, float Thickness, const FLinearColor& Color)
{
	// the outline colour is part of the brush (an image tint only affects the fill)
	const FColor Packed = Color.ToFColor(true);
	const int32 Key = -static_cast<int32>(GetTypeHash(Packed) % 1000003u) - FMath::RoundToInt32(Radius * 2.f) * 1000 - FMath::RoundToInt32(Thickness * 10.f) - 1;
	TSharedPtr<FSlateRoundedBoxBrush>& Brush = GRoundedBrushes.FindOrAdd(Key);
	if (!Brush.IsValid())
	{
		Brush = MakeShared<FSlateRoundedBoxBrush>(FLinearColor::Transparent, Radius, Color, Thickness);
	}
	return Brush.Get();
}

void CH::ShutdownStyle()
{
	GRoundedBrushes.Reset();
	GFarsiFont.Reset();
	GFarsiFaces.Reset();
	bFarsiFontInitialized = false;
}

namespace
{
	float GBackdrop = 0.f;
}

void CH::SetBackdrop(float Brightness)
{
	GBackdrop = FMath::Clamp(Brightness, 0.f, 1.f);
}

float CH::GetBackdrop()
{
	return GBackdrop;
}

FLinearColor CH::Readable(const FLinearColor& Color)
{
	const float Dark = FMath::SmoothStep(0.35f, 0.75f, GBackdrop);
	if (Dark <= 0.f)
	{
		return Color;
	}
	// keep a trace of the hue (speaker colours stay recognisable), but deep enough to read on white
	const FLinearColor Ink(Color.R * 0.18f + 0.02f, Color.G * 0.18f + 0.02f, Color.B * 0.18f + 0.025f, Color.A);
	FLinearColor Out = FMath::Lerp(Color, Ink, Dark);
	Out.A = Color.A;
	return Out;
}

FLinearColor CH::ReadableShadow(float Alpha)
{
	const float Dark = FMath::SmoothStep(0.35f, 0.75f, GBackdrop);
	return FMath::Lerp(FLinearColor(0.f, 0.f, 0.f, 0.75f * Alpha), FLinearColor(1.f, 0.98f, 0.95f, 0.55f * Alpha), Dark);
}

FString CH::PersianDigits(const FString& In)
{
	FString Out = In;
	for (int32 i = 0; i < Out.Len(); ++i)
	{
		const TCHAR C = Out[i];
		if (C >= TEXT('0') && C <= TEXT('9'))
		{
			Out[i] = static_cast<TCHAR>(0x06F0 + (C - TEXT('0')));
		}
	}
	return Out;
}
