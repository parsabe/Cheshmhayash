// Cheshm Hayash - the story director.

#include "Story/CHStoryDirector.h"
#include "Cheshmhayash.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Core/CHPlayerCharacter.h"
#include "Core/CHPlayerController.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Game/CHSegment.h"
#include "Story/CHAudio.h"
#include "Story/CHVideo.h"
#include "Widgets/Layout/SBox.h"
#include "UI/SCHHud.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Story/CHJson.h"
#include "Story/CHSave.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/CHStyle.h"
#include "UI/SCHCardLayer.h"
#include "UI/SCHChoiceList.h"
#include "UI/SCHDialogueOverlay.h"
#include "UI/SCHLetter.h"
#include "UI/SCHMontage.h"
#include "UI/SCHPhoneOverlay.h"
#include "UI/SCHRootOverlay.h"
#include "UI/SCHSlogans.h"
#include "UnrealClient.h"
#include "World/CHLevelBuilder.h"
#include "World/CHNpc.h"

namespace
{
	FString JStr(const FJsonObject& Object, const TCHAR* Key, const FString& Default = FString())
	{
		FString Out;
		return Object.TryGetStringField(Key, Out) ? Out : Default;
	}

	double JNum(const FJsonObject& Object, const TCHAR* Key, double Default)
	{
		double Out = 0.0;
		return Object.TryGetNumberField(Key, Out) ? Out : Default;
	}

	bool JBool(const FJsonObject& Object, const TCHAR* Key, bool Default)
	{
		bool Out = false;
		return Object.TryGetBoolField(Key, Out) ? Out : Default;
	}

	/** Accepts either a single string or an array of strings. */
	TArray<FString> JLines(const FJsonObject& Object, const TCHAR* Key)
	{
		TArray<FString> Out;
		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (Object.HasTypedField<EJson::Array>(Key) && Object.TryGetArrayField(Key, Array) && Array)
		{
			for (const TSharedPtr<FJsonValue>& Value : *Array)
			{
				FString Line;
				if (Value.IsValid() && Value->TryGetString(Line))
				{
					Out.Add(Line);
				}
			}
		}
		else if (Object.HasTypedField<EJson::String>(Key))
		{
			Out.Add(Object.GetStringField(Key));
		}
		return Out;
	}

	/** Speaker labels. There are no voices, so a name tag tells who is talking. "he" is never named. */
	void SpeakerNames(const FString& Who, FString& OutFa, FString& OutEn)
	{
		struct FSpeaker
		{
			const TCHAR* Key;
			const TCHAR* Fa;
			const TCHAR* En;
		};
		static const FSpeaker Speakers[] = {
			{ TEXT("he"), TEXT("او"), TEXT("He") },
			{ TEXT("kian"), TEXT("کیان"), TEXT("Kian") },
			{ TEXT("sister"), TEXT("خواهر"), TEXT("Sister") },
			{ TEXT("brother"), TEXT("برادر"), TEXT("Brother") },
			{ TEXT("mom"), TEXT("مامان"), TEXT("Maman") },
			{ TEXT("guard"), TEXT("مأمور"), TEXT("Guard") },
			{ TEXT("oldwoman"), TEXT("پیرزن"), TEXT("Old woman") },
			{ TEXT("lawyer"), TEXT("وکیل"), TEXT("Lawyer") },
			{ TEXT("driver"), TEXT("راننده"), TEXT("Taxi driver") },
			{ TEXT("professor"), TEXT("استاد"), TEXT("Professor") },
			{ TEXT("girl"), TEXT("دختر"), TEXT("Girl") },
			{ TEXT("boy"), TEXT("پسر"), TEXT("Boy") },
			{ TEXT("friend"), TEXT("دوستِ خواهر"), TEXT("Her friend") },
			{ TEXT("stranger"), TEXT("غریبه"), TEXT("Stranger") },
			{ TEXT("oldman"), TEXT("پیرمرد"), TEXT("Old man") },
			{ TEXT("student"), TEXT("دانشجو"), TEXT("Student") },
			{ TEXT("dad"), TEXT("بابا"), TEXT("Baba") },
			{ TEXT("police"), TEXT("گشت ارشاد"), TEXT("Morality police") },
			{ TEXT("official"), TEXT("مأمور"), TEXT("Official") },
			{ TEXT("neighbour"), TEXT("همسایه"), TEXT("Neighbour") },
			{ TEXT("relative"), TEXT("فامیل"), TEXT("Relative") },
			{ TEXT("crowd"), TEXT("جمعیت"), TEXT("Crowd") },
		};
		OutFa.Reset();
		OutEn.Reset();
		for (const FSpeaker& Speaker : Speakers)
		{
			if (Who.Equals(Speaker.Key, ESearchCase::IgnoreCase))
			{
				OutFa = Speaker.Fa;
				OutEn = Speaker.En;
				return;
			}
		}
	}

	/** One colour per speaker, the same everywhere in the game. */
	FLinearColor SpeakerColor(const FString& Who)
	{
		auto Srgb = [](int32 R, int32 G, int32 B) { return FLinearColor::FromSRGBColor(FColor(R, G, B)); };
		if (Who.IsEmpty() || Who.Equals(TEXT("he"), ESearchCase::IgnoreCase))
		{
			return Srgb(255, 255, 255);
		}
		if (Who.Equals(TEXT("sister"), ESearchCase::IgnoreCase))
		{
			return Srgb(255, 150, 190);
		}
		if (Who.Equals(TEXT("brother"), ESearchCase::IgnoreCase))
		{
			return Srgb(150, 200, 255);
		}
		if (Who.Equals(TEXT("mom"), ESearchCase::IgnoreCase))
		{
			return Srgb(255, 214, 90);
		}
		if (Who.Equals(TEXT("dad"), ESearchCase::IgnoreCase))
		{
			return Srgb(165, 165, 165);
		}
		if (Who.Equals(TEXT("kian"), ESearchCase::IgnoreCase))
		{
			return Srgb(150, 235, 150);
		}
		if (Who.Equals(TEXT("lawyer"), ESearchCase::IgnoreCase) || Who.Equals(TEXT("guard"), ESearchCase::IgnoreCase) ||
			Who.Equals(TEXT("official"), ESearchCase::IgnoreCase) || Who.Equals(TEXT("police"), ESearchCase::IgnoreCase))
		{
			return Srgb(255, 100, 55);
		}
		if (Who.Equals(TEXT("system"), ESearchCase::IgnoreCase) || Who.Equals(TEXT("narration"), ESearchCase::IgnoreCase))
		{
			return Srgb(190, 190, 195);
		}
		// strangers, crowd, every other face
		return Srgb(235, 215, 170);
	}

	/** Registers any English -> German pairs present in a JSON object tree. */
	void RegisterJsonGerman(const TSharedPtr<FJsonObject>& Obj)
	{
		if (!Obj.IsValid())
		{
			return;
		}
		const TArray<FString> EnLines = JLines(*Obj, TEXT("en"));
		const TArray<FString> DeLines = JLines(*Obj, TEXT("de"));
		for (int32 i = 0; i < FMath::Min(EnLines.Num(), DeLines.Num()); ++i)
		{
			CHSettings::RegisterGerman(EnLines[i], DeLines[i]);
		}
		auto Pair = [&Obj](const TCHAR* EnKey, const TCHAR* DeKey)
		{
			const FString En = JStr(*Obj, EnKey);
			const FString De = JStr(*Obj, DeKey);
			if (!En.IsEmpty() && !De.IsEmpty())
			{
				CHSettings::RegisterGerman(En, De);
			}
		};
		Pair(TEXT("en_title"), TEXT("de_title"));
		Pair(TEXT("en_sub"), TEXT("de_sub"));
		Pair(TEXT("source_en"), TEXT("source_de"));
		Pair(TEXT("hint_en"), TEXT("hint_de"));
		Pair(TEXT("title"), TEXT("title_de"));

		for (const auto& Kvp : Obj->Values)
		{
			if (!Kvp.Value.IsValid())
			{
				continue;
			}
			if (Kvp.Value->Type == EJson::Object)
			{
				RegisterJsonGerman(Kvp.Value->AsObject());
			}
			else if (Kvp.Value->Type == EJson::Array)
			{
				for (const TSharedPtr<FJsonValue>& Elem : Kvp.Value->AsArray())
				{
					if (Elem.IsValid() && Elem->Type == EJson::Object)
					{
						RegisterJsonGerman(Elem->AsObject());
					}
				}
			}
		}
	}

	/** A poem couplet at the bottom of the screen in the single active language. */
	FCHCard MakePoemCard(const FString& Fa, const FString& En, const FString& De, float Hold)
	{
		FCHCard Card;
		Card.HAlign = HAlign_Center;
		Card.VAlign = VAlign_Bottom;
		Card.Padding = FMargin(120.f, 0.f, 120.f, 150.f);
		Card.MaxWidth = 1500.f;
		Card.bSkippable = true;
		Card.MinSkipTime = 1.0f;
		Card.Hold = Hold;
		Card.FadeOut = 1.6f;

		if (CHSettings::IsPersian())
		{
			FCHCardLine FaLine(Fa, true, 38.f, CH::EWeight::Regular, CH::Col::Paper);
			FaLine.Delay = 0.4f;
			FaLine.FadeIn = 2.0f;
			Card.Lines.Add(FaLine);
		}
		else
		{
			FCHCardLine EnLine(CHSettings::Latin(En, De), false, 34.f, CH::EWeight::Regular, CH::Col::Paper);
			EnLine.Delay = 0.4f;
			EnLine.FadeIn = 2.0f;
			Card.Lines.Add(EnLine);
		}
		return Card;
	}

	/** "Click or press any key to continue" on the main menu in the single active language. */
	FCHCard MakePromptCard()
	{
		FCHCard Card;
		Card.HAlign = HAlign_Center;
		Card.VAlign = VAlign_Bottom;
		Card.Padding = FMargin(60.f, 0.f, 60.f, 110.f);
		Card.bSkippable = false;
		Card.Hold = -1.f;
		Card.FadeOut = 0.6f;

		if (CHSettings::IsPersian())
		{
			FCHCardLine FaLine(TEXT("برای ادامه کلیک کنید یا کلیدی را فشار دهید"), true, 24.f, CH::EWeight::Regular, CH::Col::PaperDim);
			FaLine.Delay = 3.2f;
			FaLine.FadeIn = 1.6f;
			Card.Lines.Add(FaLine);
		}
		else
		{
			const FString Prompt = CHSettings::IsGerman()
				? TEXT("Klicken oder beliebige Taste drücken zum Fortfahren")
				: TEXT("Click or press any key to continue");
			FCHCardLine EnLine(Prompt, false, 20.f, CH::EWeight::Light, CH::Col::PaperDim);
			EnLine.LetterSpacing = 200;
			EnLine.Delay = 3.2f;
			EnLine.FadeIn = 1.6f;
			Card.Lines.Add(EnLine);
		}
		return Card;
	}

	FCHCard MakeMenuTitleCard()
	{
		FCHCard Card;
		if (CHSettings::IsPersian())
		{
			FCHCardLine Title(TEXT("چشم‌هایش"), true, 140.f, CH::EWeight::Bold, CH::Col::Paper);
			Title.Delay = 0.6f;
			Title.FadeIn = 2.6f;
			Card.Lines.Add(Title);
		}
		else
		{
			const FString MainTitle = CHSettings::IsGerman() ? TEXT("SEINE AUGEN") : TEXT("HIS EYES");
			FCHCardLine Title(MainTitle, false, 92.f, CH::EWeight::Bold, CH::Col::Paper);
			Title.LetterSpacing = 480;
			Title.Delay = 0.6f;
			Title.FadeIn = 2.6f;
			Card.Lines.Add(Title);
		}
		Card.Hold = -1.f;
		Card.bSkippable = false;
		Card.FadeOut = 1.2f;
		Card.VAlign = VAlign_Center;
		Card.Padding = FMargin(80.f, 0.f, 80.f, 330.f);
		return Card;
	}

	FCHCard MakeOpeningCard(const TArray<FString>& Fa, const TArray<FString>& En, const TArray<FString>& De, float Hold)
	{
		FCHCard Card;
		float Time = 0.5f;
		if (CHSettings::IsPersian())
		{
			for (int32 i = 0; i < Fa.Num(); ++i)
			{
				FCHCardLine Line(Fa[i], true, 36.f, CH::EWeight::Regular, CH::Col::Paper);
				Line.TopPadding = i == 0 ? 0.f : 14.f;
				Line.Delay = Time;
				Line.FadeIn = 1.6f;
				Time += 1.6f;
				Card.Lines.Add(Line);
			}
		}
		else
		{
			const TArray<FString>& Src = (CHSettings::IsGerman() && De.Num() > 0) ? De : En;
			for (int32 i = 0; i < Src.Num(); ++i)
			{
				const FString Text = (CHSettings::IsGerman() && De.IsValidIndex(i)) ? De[i] : CHSettings::Latin(Src[i]);
				FCHCardLine Line(Text, false, 30.f, CH::EWeight::Regular, CH::Col::Paper);
				Line.TopPadding = i == 0 ? 0.f : 14.f;
				Line.Delay = Time;
				Line.FadeIn = 1.6f;
				Time += 1.5f;
				Card.Lines.Add(Line);
			}
		}
		Card.Hold = Hold;
		Card.FadeOut = 1.8f;
		return Card;
	}

	FCHCard MakePlainCard(const TArray<FString>& Fa, const TArray<FString>& En, const TArray<FString>& De, float Hold, float FaSize, float EnSize)
	{
		FCHCard Card;
		float Time = 0.3f;
		if (CHSettings::IsPersian())
		{
			for (int32 i = 0; i < Fa.Num(); ++i)
			{
				FCHCardLine Line(Fa[i], true, FaSize, CH::EWeight::Regular, CH::Col::Paper);
				Line.TopPadding = i == 0 ? 0.f : 12.f;
				Line.Delay = Time;
				Line.FadeIn = 1.4f;
				Time += 0.9f;
				Card.Lines.Add(Line);
			}
		}
		else
		{
			const TArray<FString>& Src = (CHSettings::IsGerman() && De.Num() > 0) ? De : En;
			const float PrimaryEnSize = FMath::Max(EnSize * 1.35f, 28.f);
			for (int32 i = 0; i < Src.Num(); ++i)
			{
				const FString Text = (CHSettings::IsGerman() && De.IsValidIndex(i)) ? De[i] : CHSettings::Latin(Src[i]);
				FCHCardLine Line(Text, false, PrimaryEnSize, CH::EWeight::Regular, CH::Col::Paper);
				Line.TopPadding = i == 0 ? 0.f : 12.f;
				Line.Delay = Time;
				Line.FadeIn = 1.4f;
				Time += 0.9f;
				Card.Lines.Add(Line);
			}
		}
		Card.Hold = Hold;
		return Card;
	}

	FCHCard MakeChapterCard(const FJsonObject& Beat)
	{
		FCHCard Card;
		if (CHSettings::IsPersian())
		{
			FCHCardLine FaSub(JStr(Beat, TEXT("fa_sub")), true, 24.f, CH::EWeight::Regular, CH::Col::PaperDim);
			FaSub.Delay = 0.3f;
			FaSub.FadeIn = 1.4f;
			Card.Lines.Add(FaSub);

			FCHCardLine FaTitle(JStr(Beat, TEXT("fa_title")), true, 96.f, CH::EWeight::Bold, CH::Col::Paper);
			FaTitle.TopPadding = 14.f;
			FaTitle.Delay = 1.0f;
			FaTitle.FadeIn = 2.0f;
			Card.Lines.Add(FaTitle);
		}
		else
		{
			const FString SubText = CHSettings::Latin(JStr(Beat, TEXT("en_sub")), JStr(Beat, TEXT("de_sub")));
			const FString TitleText = CHSettings::Latin(JStr(Beat, TEXT("en_title")), JStr(Beat, TEXT("de_title")));

			FCHCardLine EnSub(SubText, false, 18.f, CH::EWeight::Regular, CH::Col::PaperDim);
			EnSub.LetterSpacing = 420;
			EnSub.Delay = 0.3f;
			EnSub.FadeIn = 1.4f;
			Card.Lines.Add(EnSub);

			FCHCardLine EnTitle(TitleText, false, 68.f, CH::EWeight::Bold, CH::Col::Paper);
			EnTitle.TopPadding = 16.f;
			EnTitle.Delay = 1.0f;
			EnTitle.FadeIn = 2.0f;
			Card.Lines.Add(EnTitle);
		}

		Card.Hold = static_cast<float>(JNum(Beat, TEXT("hold"), 3.2));
		Card.FadeOut = 1.8f;
		return Card;
	}

	FCHCard MakeTitleCard()
	{
		FCHCard Card;
		if (CHSettings::IsPersian())
		{
			FCHCardLine Title(TEXT("چشم‌هایش"), true, 150.f, CH::EWeight::Bold, CH::Col::Paper);
			Title.Delay = 0.6f;
			Title.FadeIn = 3.2f;
			Card.Lines.Add(Title);
		}
		else
		{
			const FString MainTitle = CHSettings::IsGerman() ? TEXT("SEINE AUGEN") : TEXT("HIS EYES");
			FCHCardLine Title(MainTitle, false, 96.f, CH::EWeight::Bold, CH::Col::Paper);
			Title.LetterSpacing = 520;
			Title.Delay = 0.6f;
			Title.FadeIn = 3.2f;
			Card.Lines.Add(Title);
		}

		Card.Hold = -1.f;
		Card.bSkippable = false;
		Card.FadeOut = 2.0f;
		Card.Padding = FMargin(80.f, 0.f, 80.f, 240.f);
		return Card;
	}

	FCHCard MakeCaptionCard(const FString& Fa, const FString& En, const FString& De, float Hold)
	{
		FCHCard Card;
		Card.HAlign = HAlign_Left;
		Card.VAlign = VAlign_Bottom;
		Card.Padding = FMargin(72.f, 0.f, 72.f, 64.f);
		Card.MaxWidth = 1000.f;
		Card.bSkippable = false;
		Card.Hold = Hold;
		Card.FadeOut = 2.f;

		if (CHSettings::IsPersian())
		{
			if (!Fa.IsEmpty())
			{
				FCHCardLine Line(Fa, true, 26.f, CH::EWeight::Regular, CH::Col::Paper);
				Line.Delay = 0.8f;
				Line.FadeIn = 1.5f;
				Card.Lines.Add(Line);
			}
		}
		else
		{
			const FString Text = CHSettings::Latin(En, De);
			if (!Text.IsEmpty())
			{
				FCHCardLine Line(Text, false, 20.f, CH::EWeight::Regular, CH::Col::Paper);
				Line.LetterSpacing = 220;
				Line.Delay = 0.8f;
				Line.FadeIn = 1.5f;
				Card.Lines.Add(Line);
			}
		}
		return Card;
	}
}

// ---------------------------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------------------------

UCHStoryDirector* UCHStoryDirector::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !GEngine)
	{
		return nullptr;
	}
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UCHStoryDirector>() : nullptr;
}

void UCHStoryDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Audio = NewObject<UCHAudio>(this);
	Video = NewObject<UCHVideo>(this);
	ParseDevOptions();
	LoadStory();
}

void UCHStoryDirector::Deinitialize()
{
	if (UI.IsValid())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UGameViewportClient* Viewport = GameInstance->GetGameViewportClient())
			{
				Viewport->RemoveViewportWidgetContent(UI.ToSharedRef());
			}
		}
		UI.Reset();
	}
	CH::ShutdownStyle();
	Super::Deinitialize();
}

ETickableTickType UCHStoryDirector::GetTickableTickType() const
{
	return IsTemplate() ? ETickableTickType::Never : ETickableTickType::Always;
}

bool UCHStoryDirector::IsTickable() const
{
	return !IsTemplate();
}

TStatId UCHStoryDirector::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCHStoryDirector, STATGROUP_Tickables);
}

UWorld* UCHStoryDirector::GetTickableGameObjectWorld() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetWorld() : nullptr;
}

// ---------------------------------------------------------------------------------------------
// Story data
// ---------------------------------------------------------------------------------------------

void UCHStoryDirector::LoadStory()
{
	Scenes.Reset();
	for (const TCHAR* Name : { TEXT("boot"), TEXT("ch1"), TEXT("ch2"), TEXT("ch3"), TEXT("ending") })
	{
		LoadStoryFile(Name);
	}
	UE_LOG(LogCheshmhayash, Log, TEXT("Story loaded: %d scenes"), Scenes.Num());
}

bool UCHStoryDirector::LoadStoryFile(const FString& Name)
{
	const FString Path = FPaths::ProjectContentDir() / TEXT("Story") / (Name + TEXT(".json"));
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		UE_LOG(LogCheshmhayash, Log, TEXT("Story file not present: %s"), *Path);
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogCheshmhayash, Error, TEXT("Story file %s is not valid JSON: %s"), *Path, *Reader->GetErrorMessage());
		return false;
	}
	RegisterJsonGerman(RootObject);

	const TArray<TSharedPtr<FJsonValue>>* SceneArray = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("scenes"), SceneArray) || !SceneArray)
	{
		UE_LOG(LogCheshmhayash, Error, TEXT("Story file %s has no \"scenes\" array"), *Path);
		return false;
	}

	for (const TSharedPtr<FJsonValue>& SceneValue : *SceneArray)
	{
		const TSharedPtr<FJsonObject>* SceneObject = nullptr;
		if (!SceneValue.IsValid() || !SceneValue->TryGetObject(SceneObject) || !SceneObject || !SceneObject->IsValid())
		{
			continue;
		}
		const FJsonObject& Json = **SceneObject;

		FCHScene Scene;
		Scene.Id = JStr(Json, TEXT("id"));
		Scene.Chapter = static_cast<int32>(JNum(Json, TEXT("chapter"), 0.0));
		Scene.Location = JStr(Json, TEXT("location"));
		Scene.Variant = JStr(Json, TEXT("variant"));
		Scene.Player = JStr(Json, TEXT("player"), TEXT("he"));
		Scene.Music = JStr(Json, TEXT("music"));
		Scene.Ambient = JStr(Json, TEXT("ambient"));
		Scene.bInterlude = JBool(Json, TEXT("interlude"), false);
		Scene.bDev = JBool(Json, TEXT("dev"), false) || JBool(Json, TEXT("interlude"), false);

		const TArray<TSharedPtr<FJsonValue>>* BeatArray = nullptr;
		if (Json.TryGetArrayField(TEXT("beats"), BeatArray) && BeatArray)
		{
			for (const TSharedPtr<FJsonValue>& BeatValue : *BeatArray)
			{
				const TSharedPtr<FJsonObject>* BeatObject = nullptr;
				if (BeatValue.IsValid() && BeatValue->TryGetObject(BeatObject) && BeatObject && BeatObject->IsValid())
				{
					Scene.Beats.Add(*BeatObject);
				}
			}
		}
		Scenes.Add(MoveTemp(Scene));
	}
	return true;
}

int32 UCHStoryDirector::FindScene(const FString& Id) const
{
	for (int32 Index = 0; Index < Scenes.Num(); ++Index)
	{
		if (Scenes[Index].Id.Equals(Id, ESearchCase::IgnoreCase))
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

// ---------------------------------------------------------------------------------------------
// Flow
// ---------------------------------------------------------------------------------------------

void UCHStoryDirector::OnLocationReady(ACHLevelBuilder* InBuilder, const FString& LocationName)
{
	Builder = InBuilder;
	CurrentLocation = LocationName;
	CurrentVariant = InBuilder ? InBuilder->GetVariant() : FString();
	EnsureUI();

	if (!bStarted)
	{
		bStarted = true;
		DevStartSeconds = FPlatformTime::Seconds();
		StartStory();
		return;
	}

	if (bTraveling)
	{
		bTraveling = false;
		ApplySceneSetup();
	}
}

void UCHStoryDirector::StartStory()
{
	CHSettings::ApplyVolume();
	FadeAlpha = FadeTarget = 1.f;
	FadeColor = FLinearColor::Black;

	if (Scenes.Num() == 0)
	{
		if (UI.IsValid())
		{
			UI->SetNotice(TEXT("No story files found in Content/Story (boot.json, ch1.json ...)."));
		}
		return;
	}

	int32 Index = 0;
	if (!DevStartScene.IsEmpty())
	{
		const int32 Found = FindScene(DevStartScene);
		if (Found != INDEX_NONE)
		{
			Index = Found;
		}
		else
		{
			UE_LOG(LogCheshmhayash, Warning, TEXT("-chscene=%s: no such scene"), *DevStartScene);
		}
	}
	StartScene(Index);
}

void UCHStoryDirector::StartScene(int32 Index)
{
	if (!Scenes.IsValidIndex(Index))
	{
		return;
	}
	SceneIndex = Index;
	BeatIndex = 0;
	bBeatStarted = false;
	bPlayerControl = false;
	bSegmentDone = false;
	bMoveAllowed = true;
	bInInterlude = false;
	if (UI.IsValid())
	{
		UI->EyeTarget = 0.f;
		UI->EyeClose = 0.f;
		UI->SceneBrightness = 0.f;
	}

	const FCHScene& Scene = Scenes[Index];
	UE_LOG(LogCheshmhayash, Log, TEXT("Scene %s (location '%s' variant '%s')"), *Scene.Id, *Scene.Location, *Scene.Variant);
	// a checkpoint at the start of every story scene (not when resuming a saved beat)
	if (SkipToBeat == 0 && Scene.Chapter >= 1 && !Scene.bDev)
	{
		AutoSave(0);
	}
	if (Audio && !Scene.bInterlude)
	{
		Audio->SetMusic(this, Scene.Music);
		Audio->SetAmbient(this, Scene.Ambient);
	}

	const bool bNeedsTravel = !Scene.Location.IsEmpty() && (Scene.Location != CurrentLocation || Scene.Variant != CurrentVariant);
	if (bNeedsTravel)
	{
		TravelTo(Scene.Location, Scene.Variant);
	}
	else
	{
		ApplySceneSetup();
	}
}

void UCHStoryDirector::NextScene()
{
	for (int32 Index = SceneIndex + 1; Index < Scenes.Num(); ++Index)
	{
		if (!Scenes[Index].bDev)
		{
			StartScene(Index);
			return;
		}
	}

	// the story is over: back to the main menu with a clean slate
	GoToMainMenu();
}

void UCHStoryDirector::TravelTo(const FString& Location, const FString& Variant)
{
	// a location change is always a cut through black
	FadeAlpha = FadeTarget = 1.f;
	FadeColor = FLinearColor::Black;
	if (UI.IsValid())
	{
		UI->FadeAlpha = 1.f;
		UI->FadeColor = FadeColor;
	}

	const FString MapPath = TEXT("/Game/Maps/") + Location;
	if (!FPackageName::DoesPackageExist(MapPath))
	{
		UE_LOG(LogCheshmhayash, Error, TEXT("Map %s does not exist. Run Scripts/setup_assets.bat"), *MapPath);
		if (UI.IsValid())
		{
			UI->SetNotice(FString::Printf(TEXT("Map %s is missing. Close the game and run Scripts\\setup_assets.bat"), *MapPath));
		}
		return;
	}

	if (UI.IsValid())
	{
		UI->Dialogue->Hide();
		UI->Phone->Close();
	UI->Slogans->Stop(0.f);
		UI->Slogans->Stop(0.f);
		UI->Menu->Hide();
		UI->Hud->ClearState();
	}

	PendingVariant = Variant;
	TravelTarget = Location;
	bTraveling = true;
	bPlayerControl = false;
	WaitNpc.Reset();
	UGameplayStatics::OpenLevel(GetWorld(), FName(*MapPath));
}

void UCHStoryDirector::ApplySceneSetup()
{
	if (!Scenes.IsValidIndex(SceneIndex))
	{
		return;
	}
	const FCHScene& Scene = Scenes[SceneIndex];
	if (ACHPlayerController* Controller = GetController())
	{
		Controller->SetSceneCamera(nullptr, 0.f);
		if (ACHPlayerCharacter* Hero = Cast<ACHPlayerCharacter>(Controller->GetPawn()))
		{
			Hero->SetLook(Scene.Player);
			if (Builder.IsValid())
			{
				Hero->SetBoom(Builder->PlayerBoomLength, Builder->PlayerBoomOffset);
			}
		}
	}
}

ACHPlayerController* UCHStoryDirector::GetController() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? Cast<ACHPlayerController>(GameInstance->GetFirstLocalPlayerController()) : nullptr;
}

void UCHStoryDirector::EnsureUI()
{
	if (UI.IsValid())
	{
		return;
	}
	UGameInstance* GameInstance = GetGameInstance();
	UGameViewportClient* Viewport = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;
	if (!Viewport)
	{
		return;
	}
	SAssignNew(UI, SCHRootOverlay);
	UI->FadeAlpha = FadeAlpha;
	UI->FadeColor = FadeColor;
	Viewport->AddViewportWidgetContent(UI.ToSharedRef(), 50);
}

// ---------------------------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------------------------

void UCHStoryDirector::Tick(float DeltaTime)
{
	if (!bStarted)
	{
		return;
	}

	DevTick();

	if (UI.IsValid())
	{
		UI->AdvancePause(FMath::Min(0.1f, static_cast<float>(FApp::GetDeltaTime())));
		if (bPauseMenu)
		{
			UpdatePause();
			return;
		}
	}

	if (UI.IsValid())
	{
		UpdateFade(DeltaTime);
		if (bRec)
		{
			RecSeconds += DeltaTime;
		}
		UI->Hud->SetRec(bRec, RecSeconds, !UI->Cards->IsFinished());
		UI->Advance(DeltaTime);
		if (Video)
		{
			Video->Update(DeltaTime);
		}

		// one soft click per typed letter (hooked up to the audio manager in the audio step)
		const bool bDialogueTick = UI->Dialogue->ConsumeTypeTick();
		const bool bPhoneTick = UI->Phone->ConsumeTypeTick();
		if (bDialogueTick || bPhoneTick)
		{
			OnTypeTick(bPhoneTick);
		}
	}

	if (Audio)
	{
		const UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this);
		const bool bStealth = Segment && Segment->IsActive() && !Segment->IsSuspended();
		Audio->Update(this, bStealth && Segment->IsInVanZone(), bStealth ? Segment->GetMaxSuspicion() : 0.f, bStealth && Segment->AnyChasing());
	}

	if (!bTraveling && Scenes.IsValidIndex(SceneIndex))
	{
		bool bFirst = true;
		for (int32 Guard = 0; Guard < 64 && !bTraveling; ++Guard)
		{
			const FCHScene& Scene = Scenes[SceneIndex];
			if (!Scene.Beats.IsValidIndex(BeatIndex))
			{
				if (bInInterlude)
				{
					EndInterlude();
				}
				else
				{
					NextScene();
				}
				bFirst = false;
				continue;
			}

			const TSharedPtr<FJsonObject> Beat = Scene.Beats[BeatIndex];
			if (SkipToBeat > 0 && BeatIndex >= SkipToBeat)
			{
				// the scene is set up again: fade in on the checkpoint
				SkipToBeat = 0;
				SetFade(0.f, 1.4f, FLinearColor::Black);
			}
			if (SkipToBeat > 0 && !bInInterlude)
			{
				// continuing: only the beats that set the scene up run, nothing is played again
				if (PassesCondition(*Beat))
				{
					const FString SkipType = JStr(*Beat, TEXT("type"));
					if (SkipType == TEXT("camera") || SkipType == TEXT("event") || SkipType == TEXT("flag") || SkipType == TEXT("npc") ||
						SkipType == TEXT("place") || SkipType == TEXT("control") || SkipType == TEXT("rec") || SkipType == TEXT("audio"))
					{
						BeginBeat(*Beat);
					}
				}
				++BeatIndex;
				bBeatStarted = false;
				continue;
			}
			if (!bBeatStarted)
			{
				bBeatStarted = true;
				BeatTime = 0.f;
				BeatPhase = 0;
				PhaseTime = 0.f;
				IdleTime = 0.f;
				if (!PassesCondition(*Beat))
				{
					++BeatIndex;
					bBeatStarted = false;
					continue;
				}
				UE_LOG(LogCheshmhayash, Verbose, TEXT("Beat %s[%d] %s"), *Scene.Id, BeatIndex, *JStr(*Beat, TEXT("type")));
				BeginBeat(*Beat);
			}
			else if (bFirst)
			{
				BeatTime += DeltaTime;
				PhaseTime += DeltaTime;
			}
			bFirst = false;

			if (UpdateBeat(*Beat, DeltaTime))
			{
				++BeatIndex;
				bBeatStarted = false;
				continue;
			}
			break;
		}
	}

	bAdvancePressed = false;
	bClickPressed = false;
	bAnyKeyPressed = false;
	NavInput = 0;
	DirectChoice = INDEX_NONE;
}

void UCHStoryDirector::OnTypeTick(bool bPhone)
{
	if (Audio)
	{
		Audio->TypeTick(this, bPhone);
	}
}

void UCHStoryDirector::PlaySfx(const FString& Name, float Volume)
{
	if (Audio)
	{
		Audio->PlaySfx(this, Name, Volume);
	}
}

FString UCHStoryDirector::PeekNextType() const
{
	if (Scenes.IsValidIndex(SceneIndex))
	{
		const FCHScene& Scene = Scenes[SceneIndex];
		for (int32 Index = BeatIndex + 1; Index < Scene.Beats.Num(); ++Index)
		{
			if (Scene.Beats[Index].IsValid() && PassesCondition(*Scene.Beats[Index]))
			{
				return JStr(*Scene.Beats[Index], TEXT("type"));
			}
		}
	}
	return FString();
}

void UCHStoryDirector::SetFade(float Target, float Time, const FLinearColor& Color)
{
	FadeTarget = FMath::Clamp(Target, 0.f, 1.f);
	FadeColor = Color;
	if (Time <= 0.f)
	{
		FadeAlpha = FadeTarget;
		FadeRate = 0.f;
	}
	else
	{
		FadeRate = 1.f / Time;
	}
}

void UCHStoryDirector::UpdateFade(float DeltaTime)
{
	if (FadeRate > 0.f && !FMath::IsNearlyEqual(FadeAlpha, FadeTarget))
	{
		FadeAlpha = FMath::FInterpConstantTo(FadeAlpha, FadeTarget, DeltaTime, FadeRate);
	}
	UI->FadeColor = FadeColor;
	UI->FadeAlpha = FMath::SmoothStep(0.f, 1.f, FadeAlpha);
}

bool UCHStoryDirector::ConsumeAdvance()
{
	const bool bPressed = bAdvancePressed || bClickPressed;
	bAdvancePressed = false;
	bClickPressed = false;
	return bPressed;
}

bool UCHStoryDirector::WantsCursor() const
{
	if (!UI.IsValid())
	{
		return false;
	}
	if (bPauseMenu || UI->PauseMenu->IsShown() || UI->Menu->IsShown())
	{
		return true;
	}
	// the main menu's "click or press any key"
	if (Scenes.IsValidIndex(SceneIndex) && Scenes[SceneIndex].Beats.IsValidIndex(BeatIndex))
	{
		return JStr(*Scenes[SceneIndex].Beats[BeatIndex], TEXT("type")) == TEXT("mainmenu");
	}
	return false;
}

int32 UCHStoryDirector::PollMenu(SCHChoiceList& Menu)
{
	// the mouse moves the highlight
	if (Menu.IsShown() && FSlateApplication::IsInitialized())
	{
		const FVector2D Cursor = FSlateApplication::Get().GetCursorPos();
		if (!Cursor.Equals(LastCursor, 1.f))
		{
			LastCursor = Cursor;
			const int32 Hovered = Menu.HitTest();
			if (Hovered != INDEX_NONE)
			{
				Menu.SetSelected(Hovered);
			}
		}
	}
	if (NavInput != 0)
	{
		Menu.Move(NavInput > 0 ? 1 : -1);
		NavInput = 0;
	}
	if (!Menu.IsReady())
	{
		return INDEX_NONE;
	}
	if (DirectChoice != INDEX_NONE && DirectChoice < Menu.Num())
	{
		return DirectChoice;
	}
	if (bClickPressed)
	{
		bClickPressed = false;
		const int32 Hovered = Menu.HitTest();
		if (Hovered != INDEX_NONE)
		{
			return Hovered;
		}
	}
	if (bAdvancePressed)
	{
		bAdvancePressed = false;
		return Menu.GetSelected();
	}
	return INDEX_NONE;
}

// ---------------------------------------------------------------------------------------------
// Beats
// ---------------------------------------------------------------------------------------------

bool UCHStoryDirector::PassesCondition(const FJsonObject& Beat) const
{
	// "if": "flag=value" or "flag!=value"
	return PassesCondition(JStr(Beat, TEXT("if")));
}

bool UCHStoryDirector::PassesCondition(const FString& Condition) const
{
	if (Condition.IsEmpty())
	{
		return true;
	}
	FString Name;
	FString Value;
	if (Condition.Split(TEXT("!="), &Name, &Value))
	{
		return !GetFlag(Name.TrimStartAndEnd()).Equals(Value.TrimStartAndEnd(), ESearchCase::IgnoreCase);
	}
	if (Condition.Split(TEXT("="), &Name, &Value))
	{
		return GetFlag(Name.TrimStartAndEnd()).Equals(Value.TrimStartAndEnd(), ESearchCase::IgnoreCase);
	}
	return !GetFlag(Condition.TrimStartAndEnd()).IsEmpty();
}

void UCHStoryDirector::ShowMemorial()
{
	if (!UI.IsValid())
	{
		return;
	}
	FCHCard Card;
	Card.HAlign = HAlign_Center;
	Card.VAlign = VAlign_Top;
	Card.Padding = FMargin(80.f, 170.f, 80.f, 0.f);
	Card.bSkippable = false;
	Card.Hold = -1.f;
	Card.FadeOut = 4.0f;

	const float Rate = 2.6f;
	if (CHSettings::IsPersian())
	{
		const FString Fa = TEXT("به یاد حسام");
		FCHCardLine FaLine(Fa, true, 58.f, CH::EWeight::Regular, CH::Col::Paper);
		FaLine.Delay = 1.0f;
		FaLine.TypeRate = Rate;
		Card.Lines.Add(FaLine);
	}
	else
	{
		const FString En = CHSettings::IsGerman()
			? TEXT("In liebevoller Erinnerung an Hesam")
			: TEXT("In loving memory of Hesam");
		FCHCardLine EnLine(En, false, 40.f, CH::EWeight::Regular, CH::Col::Paper);
		EnLine.Delay = 1.0f;
		EnLine.TypeRate = Rate;
		Card.Lines.Add(EnLine);
	}
	UI->Captions->Play(Card);
}

void UCHStoryDirector::BeginBeat(const FJsonObject& Beat)
{
	const FString Type = JStr(Beat, TEXT("type"));
	ACHPlayerController* Controller = GetController();

	if (Type == TEXT("fade"))
	{
		const bool bWhite = JStr(Beat, TEXT("color")) == TEXT("white");
		SetFade(static_cast<float>(JNum(Beat, TEXT("to"), 0.0)), static_cast<float>(JNum(Beat, TEXT("time"), 1.5)), bWhite ? FLinearColor::White : FLinearColor::Black);
	}
	else if (Type == TEXT("language_select"))
	{
		TArray<FCHOption> Options;
		Options.Emplace(TEXT(""), TEXT("English"));
		Options.Emplace(TEXT(""), TEXT("Deutsch"));
		Options.Emplace(TEXT("فارسی"), TEXT(""));
		UI->Menu->Show(Options, VAlign_Center, FMargin(0.f), 34.f, 28.f);
		UI->Menu->SetSelected(static_cast<int32>(CHSettings::GetLanguage()));
	}
	else if (Type == TEXT("card"))
	{
		const FString Style = JStr(Beat, TEXT("style"), TEXT("plain"));
		const float Hold = static_cast<float>(JNum(Beat, TEXT("hold"), 4.0));
		FCHCard Card = Style == TEXT("opening")
			? MakeOpeningCard(JLines(Beat, TEXT("fa")), JLines(Beat, TEXT("en")), JLines(Beat, TEXT("de")), Hold)
			: MakePlainCard(JLines(Beat, TEXT("fa")), JLines(Beat, TEXT("en")), JLines(Beat, TEXT("de")), Hold,
				static_cast<float>(JNum(Beat, TEXT("fa_size"), 40.0)), static_cast<float>(JNum(Beat, TEXT("en_size"), 22.0)));
		Card.bSkippable = JBool(Beat, TEXT("skippable"), true);
		if (JBool(Beat, TEXT("below"), false))
		{
			Card.VAlign = VAlign_Center;
			Card.Padding = FMargin(60.f, 220.f, 60.f, 0.f);
		}
		const float TypeRate = static_cast<float>(JNum(Beat, TEXT("type_rate"), 0.0));
		if (TypeRate > 0.f)
		{
			float Delay = static_cast<float>(JNum(Beat, TEXT("delay"), 0.8));
			for (FCHCardLine& Line : Card.Lines)
			{
				Line.TypeRate = TypeRate;
				Line.Delay = Delay;
				Delay += Line.Text.Len() / TypeRate + 0.9f;
			}
		}
		if (JBool(Beat, TEXT("ember"), false) && Card.Lines.Num() > 0)
		{
			Card.Lines[0].Color = CH::Col::Paper;
		}
		UI->Cards->Play(Card);
	}
	else if (Type == TEXT("chapter"))
	{
		UI->Cards->Play(MakeChapterCard(Beat));
	}
	else if (Type == TEXT("caption"))
	{
		UI->Captions->Play(MakeCaptionCard(JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")), JStr(Beat, TEXT("de")), static_cast<float>(JNum(Beat, TEXT("hold"), 4.5))));
	}
	else if (Type == TEXT("memorial"))
	{
		if (!JBool(Beat, TEXT("on"), true))
		{
			UI->Captions->Dismiss(static_cast<float>(JNum(Beat, TEXT("fade"), 4.0)));
		}
		else
		{
			ShowMemorial();
		}
	}
	else if (Type == TEXT("backdrop"))
	{
		// a bright shot (snow, white sky): the text turns dark to stay readable
		UI->SceneBrightness = static_cast<float>(JNum(Beat, TEXT("bright"), 0.0));
	}
	else if (Type == TEXT("video"))
	{
		// the closing video; the name appears over it at "memorial_at" seconds and stays
		BeatPhase = 0;
		if (Audio)
		{
			Audio->Cut(1.5f);
		}
		if (Video && Video->Play(GetWorld(), JStr(Beat, TEXT("file"))) && UI.IsValid())
		{
			UI->VideoLayer->SetContent(Video->MakeWidget());
		}
	}
	else if (Type == TEXT("title"))
	{
		UI->Cards->Play(MakeTitleCard());
	}
	else if (Type == TEXT("poem"))
	{
		BeatPhase = 0;
		const TArray<TSharedPtr<FJsonObject>> Couplets = CHJson::Objects(Beat, TEXT("lines"));
		if (Couplets.Num() > 0)
		{
			const FString Fa = JStr(*Couplets[0], TEXT("fa"));
			const FString En = JStr(*Couplets[0], TEXT("en"));
			const FString De = JStr(*Couplets[0], TEXT("de"));
			UI->Cards->Play(MakePoemCard(Fa, En, De, 2.4f + 0.055f * FMath::Max(Fa.Len(), En.Len())));
			BeatPhase = 1;
		}
	}
	else if (Type == TEXT("loading"))
	{
		UI->Hud->SetLoading(true);
	}
	else if (Type == TEXT("mainmenu"))
	{
		UI->Hud->SetLoading(false);
		bMenuSettings = false;
		UI->Cards->Play(MakeMenuTitleCard());
		UI->Captions->Play(MakePromptCard());
	}
	else if (Type == TEXT("control"))
	{
		bPlayerControl = JBool(Beat, TEXT("enabled"), true);
	}
	else if (Type == TEXT("camera"))
	{
		const FString MarkName = JStr(Beat, TEXT("mark"), TEXT("player"));
		const float Blend = static_cast<float>(JNum(Beat, TEXT("blend"), 0.0));
		if (Controller)
		{
			AActor* Camera = (MarkName == TEXT("player") || !Builder.IsValid()) ? nullptr : Cast<AActor>(Builder->GetCamera(FName(*MarkName)));
			Controller->SetSceneCamera(Camera, Blend);
		}
	}
	else if (Type == TEXT("event"))
	{
		if (Builder.IsValid())
		{
			Builder->OnStoryEvent(JStr(Beat, TEXT("name")), Scenes[SceneIndex].Beats[BeatIndex]);
		}
	}
	else if (Type == TEXT("flag"))
	{
		SetFlag(JStr(Beat, TEXT("name")), JStr(Beat, TEXT("value"), TEXT("1")));
	}
	else if (Type == TEXT("gameplay"))
	{
		// a checkpoint right before every gameplay segment
		if (!bInInterlude)
		{
			AutoSave(BeatIndex);
		}
		bSegmentDone = false;
		bPlayerControl = true;
		bMoveAllowed = JBool(Beat, TEXT("move"), true);

		// second chances: a base number plus one for every kindness the player chose
		SegmentChances = static_cast<int32>(JNum(Beat, TEXT("chances"), 0.0));
		for (const FString& Condition : CHJson::Lines(Beat, TEXT("chance_flags")))
		{
			if (PassesCondition(Condition))
			{
				++SegmentChances;
			}
		}
		if (!bControlsShown && JBool(Beat, TEXT("controls"), true))
		{
			bControlsShown = true;
			UI->Hud->ShowControls(14.f);
		}
		// "keep_camera": stay on the scene camera (filming the sunrise from where he sits)
		if (Controller && !JBool(Beat, TEXT("keep_camera"), false))
		{
			Controller->SetSceneCamera(nullptr, static_cast<float>(JNum(Beat, TEXT("blend"), 0.0)));
		}
		if (Builder.IsValid())
		{
			Builder->StartSegment(JStr(Beat, TEXT("segment")), Scenes[SceneIndex].Beats[BeatIndex]);
		}
	}
	else if (Type == TEXT("text"))
	{
		FString SpeakerFa;
		FString SpeakerEn;
		SpeakerNames(JStr(Beat, TEXT("who")), SpeakerFa, SpeakerEn);
		UI->Dialogue->SetHint(HintLinesLeft > 0);
		if (HintLinesLeft > 0)
		{
			--HintLinesLeft;
		}
		UI->Dialogue->ShowLine(SpeakerFa, SpeakerEn, JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")), static_cast<float>(JNum(Beat, TEXT("rate"), 30.0)), SpeakerColor(JStr(Beat, TEXT("who"))));
	}
	else if (Type == TEXT("hide_text"))
	{
		UI->Dialogue->Hide();
	}
	else if (Type == TEXT("choice"))
	{
		UI->Dialogue->Hide();
		TArray<FCHOption> Options;
		for (const TSharedPtr<FJsonObject>& Option : CHJson::Objects(Beat, TEXT("options")))
		{
			Options.Emplace(JStr(*Option, TEXT("fa")), JStr(*Option, TEXT("en")));
		}
		UI->Menu->Show(Options, VAlign_Bottom, FMargin(0.f, 0.f, 0.f, 96.f), 30.f, 20.f);
		UI->Menu->SetFooter(TEXT("Click an answer  /  1 2 3  /  Enter"));
	}
	else if (Type == TEXT("phone"))
	{
		BeginPhoneBeat(Beat);
	}
	else if (Type == TEXT("npc"))
	{
		WaitNpc = Builder.IsValid() ? Builder->HandleNpcBeat(Beat) : nullptr;
	}
	else if (Type == TEXT("place"))
	{
		// puts the player on a mark (he is "him" in the fixed-camera scenes)
		FCHMark Where;
		APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
		if (Pawn && Builder.IsValid() && Builder->GetMark(FName(*JStr(Beat, TEXT("mark"))), Where))
		{
			const FVector Location = Where.Transform.GetLocation() + FVector(0.f, 0.f, 94.f);
			const FRotator Rotation(0.f, Where.Transform.Rotator().Yaw + static_cast<float>(JNum(Beat, TEXT("yaw"), 0.0)), 0.f);
			Pawn->TeleportTo(Location, Rotation, false, true);
			Controller->SetControlRotation(Rotation);
		}
		if (Pawn)
		{
			const bool bHide = JBool(Beat, TEXT("hidden"), false);
			Pawn->SetActorHiddenInGame(bHide);
			// first-person bodies cast a shadow even when hidden: not when he is out of the scene
			TInlineComponentArray<UPrimitiveComponent*> Parts(Pawn);
			for (UPrimitiveComponent* Part : Parts)
			{
				Part->SetCastShadow(!bHide);
			}
		}
	}
	else if (Type == TEXT("cut"))
	{
		SetFade(1.f, 0.f, FLinearColor::Black);
		UI->Dialogue->Hide();
		UI->Captions->Clear();
		UI->Menu->Hide();
	}
	else if (Type == TEXT("rec"))
	{
		bRec = JBool(Beat, TEXT("on"), true);
	}
	else if (Type == TEXT("clip"))
	{
		// a scripted clip: whatever the current camera sees
		CaptureClip(JStr(Beat, TEXT("id")), JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")));
	}
	else if (Type == TEXT("toast"))
	{
		Toast(JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")), CH::Col::Paper, static_cast<float>(JNum(Beat, TEXT("seconds"), 2.8)));
	}
	else if (Type == TEXT("eye"))
	{
		// his LEFT eye: "close" 1 = the left half goes dark
		UI->EyeTarget = static_cast<float>(JNum(Beat, TEXT("close"), 1.0));
		const float Time = static_cast<float>(JNum(Beat, TEXT("time"), 2.5));
		UI->EyeRate = Time > 0.f ? 1.f / Time : 100.f;
		if (Time <= 0.f)
		{
			UI->EyeClose = UI->EyeTarget;
		}
	}
	else if (Type == TEXT("shake"))
	{
		if (Controller)
		{
			Controller->Shake(static_cast<float>(JNum(Beat, TEXT("seconds"), 1.0)), static_cast<float>(JNum(Beat, TEXT("strength"), 1.5)));
		}
	}
	else if (Type == TEXT("flash"))
	{
		Flash(JStr(Beat, TEXT("color")) == TEXT("red") ? FLinearColor(0.7f, 0.03f, 0.02f) : FLinearColor::White, static_cast<float>(JNum(Beat, TEXT("time"), 0.6)));
	}
	else if (Type == TEXT("montage"))
	{
		// the director's cut: every clip he filmed, in order; the last one freezes
		SetFade(1.f, 0.f, FLinearColor::Black);
		UI->Dialogue->Hide();
		UI->Captions->Clear();
		bRec = false;
		TArray<FCHMontageClip> Montage;
		for (const FCHClip& Clip : Clips)
		{
			FCHMontageClip Item;
			Item.Image = Clip.Image;
			Item.Fa = Clip.CaptionFa;
			Item.En = Clip.CaptionEn;
			Item.Stamp = Clip.Chapter <= 1 ? TEXT("SHAHRIVAR 1401") : TEXT("MEHR 1401");
			Montage.Add(Item);
		}
		if (Montage.Num() > 0)
		{
			Montage.Last().Stamp = TEXT("24 MEHR 1401  06:04");
		}
		UI->Montage->Play(Montage, JStr(Beat, TEXT("fa"), TEXT("آنچه چشم‌هایش دید")), JStr(Beat, TEXT("en"), TEXT("WHAT HIS EYES SAW")));
	}
	else if (Type == TEXT("interlude"))
	{
		PlayInterlude(JStr(Beat, TEXT("scene")));
	}
	else if (Type == TEXT("notify"))
	{
		PlaySfx(TEXT("sfx_notify"), 0.6f);
		UI->Hud->Notify(JStr(Beat, TEXT("title")), JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")), static_cast<float>(JNum(Beat, TEXT("seconds"), 8.0)));
	}
	else if (Type == TEXT("letter"))
	{
		TArray<FCHLetterLine> Lines;
		for (const TSharedPtr<FJsonObject>& Line : CHJson::Objects(Beat, TEXT("lines")))
		{
			Lines.Add({ JStr(*Line, TEXT("fa")), JStr(*Line, TEXT("en")) });
		}
		UI->Letter->Show(Lines);
	}
	else if (Type == TEXT("audio"))
	{
		// "cut": everything stops. "music" / "ambient": change the track ("none" = silence). "sfx": one sound.
		if (Audio)
		{
			const float FadeTime = static_cast<float>(JNum(Beat, TEXT("fade"), 2.5));
			if (JBool(Beat, TEXT("cut"), false))
			{
				Audio->Cut();
			}
			if (Beat.HasField(TEXT("music")))
			{
				Audio->SetMusic(this, JStr(Beat, TEXT("music")), FadeTime, static_cast<float>(JNum(Beat, TEXT("volume"), 0.7)));
			}
			if (Beat.HasField(TEXT("ambient")))
			{
				Audio->SetAmbient(this, JStr(Beat, TEXT("ambient")), FadeTime);
			}
			if (Beat.HasField(TEXT("sfx")))
			{
				Audio->PlaySfx(this, TEXT("sfx_") + JStr(Beat, TEXT("sfx")), static_cast<float>(JNum(Beat, TEXT("volume"), 1.0)));
			}
		}
	}
	else if (Type == TEXT("sfx"))
	{
		PlaySfx(TEXT("sfx_") + JStr(Beat, TEXT("name")), static_cast<float>(JNum(Beat, TEXT("volume"), 1.0)));
	}
	else if (Type == TEXT("slogans"))
	{
		if (JBool(Beat, TEXT("on"), true))
		{
			const FString Slogan = CHSettings::Pick(
				JStr(Beat, TEXT("fa")),
				JStr(Beat, TEXT("en"), TEXT("Woman, Life, Freedom")),
				JStr(Beat, TEXT("de"), TEXT("Frau, Leben, Freiheit")));
			UI->Slogans->Start(Slogan, static_cast<int32>(JNum(Beat, TEXT("count"), 80.0)), static_cast<float>(JNum(Beat, TEXT("time"), 16.0)));
		}
		else
		{
			UI->Slogans->Stop(static_cast<float>(JNum(Beat, TEXT("time"), 2.0)));
		}
	}
	else if (Type == TEXT("show_clip"))
	{
		// one clip he filmed, shown on its own (his phone, placed on the stone)
		const FString Id = JStr(Beat, TEXT("id"));
		TArray<FCHMontageClip> One;
		for (int32 Index = Clips.Num() - 1; Index >= 0; --Index)
		{
			if (Clips[Index].Id == Id)
			{
				FCHMontageClip Item;
				Item.Image = Clips[Index].Image;
				Item.Fa = Clips[Index].CaptionFa;
				Item.En = Clips[Index].CaptionEn;
				Item.Stamp = TEXT("24 MEHR 1401  06:04");
				One.Add(Item);
				break;
			}
		}
		if (One.Num() > 0)
		{
			UI->Montage->Play(One, FString(), FString());
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Interludes: a short story moment inside a running gameplay segment
// ---------------------------------------------------------------------------------------------

void UCHStoryDirector::PlayInterlude(const FString& SceneId)
{
	const int32 Index = FindScene(SceneId);
	if (Index == INDEX_NONE || bInInterlude || bTraveling)
	{
		UE_LOG(LogCheshmhayash, Warning, TEXT("Interlude '%s' not played"), *SceneId);
		return;
	}
	bInInterlude = true;
	ReturnScene = SceneIndex;
	ReturnBeat = BeatIndex;
	bReturnControl = bPlayerControl;
	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(GetWorld()))
	{
		Segment->SetSuspended(true);
	}
	SceneIndex = Index;
	BeatIndex = 0;
	bBeatStarted = false;
	bPlayerControl = false;
	UE_LOG(LogCheshmhayash, Log, TEXT("Interlude %s"), *SceneId);
}

void UCHStoryDirector::EndInterlude()
{
	bInInterlude = false;
	SceneIndex = ReturnScene;
	BeatIndex = ReturnBeat;
	// the beat we came from (the gameplay segment) keeps running
	bBeatStarted = true;
	bPlayerControl = bReturnControl;
	UI->Dialogue->Hide();
	if (ACHPlayerController* Controller = GetController())
	{
		Controller->SetSceneCamera(nullptr, 0.8f);
	}
	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(GetWorld()))
	{
		Segment->SetSuspended(false);
	}
	if (Builder.IsValid())
	{
		Builder->OnInterludeEnded();
	}
}

// ---------------------------------------------------------------------------------------------
// Screen and clips
// ---------------------------------------------------------------------------------------------

void UCHStoryDirector::Flash(const FLinearColor& Color, float Seconds)
{
	if (UI.IsValid())
	{
		UI->Flash(Color, Seconds);
	}
}

void UCHStoryDirector::Toast(const FString& Fa, const FString& En, const FLinearColor& Color, float Seconds, UTexture* Image)
{
	if (UI.IsValid())
	{
		UI->Hud->Toast(Fa, En, Color, Seconds, Image);
	}
}

UTexture* UCHStoryDirector::CaptureClip(const FString& Id, const FString& CaptionFa, const FString& CaptionEn)
{
	ACHPlayerController* Controller = GetController();
	UWorld* World = GetWorld();
	if (!Controller || !Controller->PlayerCameraManager || !World)
	{
		return nullptr;
	}
	const FMinimalViewInfo& View = Controller->PlayerCameraManager->GetCameraCacheView();

	UTextureRenderTarget2D* Target = NewObject<UTextureRenderTarget2D>(this);
	// the final colour is already gamma encoded: an sRGB target makes Slate show it as filmed
	Target->RenderTargetFormat = RTF_RGBA8_SRGB;
	Target->ClearColor = FLinearColor::Black;
	Target->InitAutoFormat(1280, 720);
	Target->UpdateResourceImmediate(true);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASceneCapture2D* Capture = World->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass(), View.Location, View.Rotation, Params);
	if (!Capture)
	{
		return nullptr;
	}
	USceneCaptureComponent2D* Component = Capture->GetCaptureComponent2D();
	Component->bCaptureEveryFrame = false;
	Component->bCaptureOnMovement = false;
	Component->FOVAngle = View.FOV;
	Component->CaptureSource = SCS_FinalColorLDR;
	Component->TextureTarget = Target;

	// one-off captures have no eye adaptation history: give them the scene's fixed exposure
	FPostProcessSettings& Settings = Component->PostProcessSettings;
	Settings.bOverride_AutoExposureMethod = true;
	Settings.AutoExposureMethod = AEM_Manual;
	Settings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
	Settings.AutoExposureApplyPhysicalCameraExposure = false;
	Settings.bOverride_AutoExposureBias = true;
	Settings.AutoExposureBias = -CHLight::GetExposure() - FMath::Log2(1.2f);
	Component->PostProcessBlendWeight = 1.f;
	Component->CaptureScene();
	Capture->Destroy();

	FCHClip Clip;
	Clip.Id = Id;
	Clip.CaptionFa = CaptionFa;
	Clip.CaptionEn = CaptionEn;
	Clip.Chapter = Scenes.IsValidIndex(SceneIndex) ? Scenes[SceneIndex].Chapter : 0;
	Clip.Image = Target;
	Clips.Add(Clip);
	PlaySfx(TEXT("sfx_saved"), 0.6f);
	UE_LOG(LogCheshmhayash, Log, TEXT("Clip %d filmed: %s (%s)"), Clips.Num(), *Id, *CaptionEn);
	return Target;
}

void UCHStoryDirector::BeginPhoneBeat(const FJsonObject& Beat)
{
	const FString Action = JStr(Beat, TEXT("action"));
	const FString Fa = JStr(Beat, TEXT("fa"));
	const FString En = JStr(Beat, TEXT("en"));
	SCHPhoneOverlay& Phone = *UI->Phone;

	if (Action == TEXT("call"))
	{
		Phone.OpenCall(Fa, En);
	}
	else if (Action == TEXT("chat"))
	{
		Phone.OpenChat(Fa, En, JBool(Beat, TEXT("clear"), true));
	}
	else if (Action == TEXT("close"))
	{
		Phone.Close();
	}
	else if (Action == TEXT("msg"))
	{
		Phone.Receive(Fa, En, static_cast<float>(JNum(Beat, TEXT("typing"), 1.1)));
		PlaySfx(TEXT("sfx_notify"), 0.5f);
	}
	else if (Action == TEXT("draft"))
	{
		Phone.TypeDraft(Fa, En, false, static_cast<float>(JNum(Beat, TEXT("pause"), 1.2)));
	}
	else if (Action == TEXT("send"))
	{
		Phone.TypeDraft(Fa, En, true);
	}
	else if (Action == TEXT("note"))
	{
		Phone.AddNote(Fa, En);
	}
	else if (Action == TEXT("status"))
	{
		Phone.SetStatus(Fa, En);
	}
	else if (Action == TEXT("noservice"))
	{
		Phone.SetNoService(JBool(Beat, TEXT("on"), true));
	}
	else if (Action == TEXT("clock"))
	{
		Phone.SetClock(En);
	}
	else if (Action == TEXT("freeze"))
	{
		Phone.SetCallFrozen(JBool(Beat, TEXT("on"), true));
	}
	else if (Action == TEXT("alert"))
	{
		Phone.ShowAlert(JStr(Beat, TEXT("source_fa")), JStr(Beat, TEXT("source_en"), TEXT("News")), Fa, En);
		PlaySfx(TEXT("sfx_notify"), 0.7f);
	}
	else if (Action == TEXT("alert_hide"))
	{
		Phone.HideAlert();
	}
	else if (Action == TEXT("voice"))
	{
		Phone.StartVoice(JStr(Beat, TEXT("duration"), TEXT("0:47")), CHJson::Objects(Beat, TEXT("lines")).Num());
	}
}

bool UCHStoryDirector::UpdatePhoneBeat(const FJsonObject& Beat, float DeltaTime)
{
	SCHPhoneOverlay& Phone = *UI->Phone;
	if (Phone.IsBusy())
	{
		PhaseTime = 0.f;
		return false;
	}

	if (JStr(Beat, TEXT("action")) == TEXT("voice"))
	{
		// BeatPhase = transcript lines shown so far. Each line waits for the player (or a few seconds).
		const TArray<TSharedPtr<FJsonObject>> Lines = CHJson::Objects(Beat, TEXT("lines"));
		const float LineTime = static_cast<float>(JNum(Beat, TEXT("line_time"), 3.4));
		const bool bNext = BeatPhase == 0 || PhaseTime >= LineTime || (PhaseTime > 0.7f && ConsumeAdvance());
		if (!bNext)
		{
			return false;
		}
		if (Lines.IsValidIndex(BeatPhase))
		{
			Phone.AddTranscriptLine(JStr(*Lines[BeatPhase], TEXT("fa")), JStr(*Lines[BeatPhase], TEXT("en")));
			++BeatPhase;
			PhaseTime = 0.f;
			return false;
		}
		Phone.EndVoice();
		return true;
	}

	IdleTime += DeltaTime;
	return IdleTime >= static_cast<float>(JNum(Beat, TEXT("hold"), 0.0));
}

bool UCHStoryDirector::UpdateDialogueLine(const FJsonObject& Beat)
{
	SCHDialogueOverlay& Dialogue = *UI->Dialogue;
	const bool bPressed = ConsumeAdvance();

	if (Dialogue.IsTyping())
	{
		if (bPressed)
		{
			Dialogue.CompleteNow();
		}
		return false;
	}

	// Auto-advance: about 2 s + 0.06 s per character in total (the longer of the two lines), counted
	// from the first letter. A scripted "auto" (seconds after the line is complete) overrides it.
	const float Explicit = static_cast<float>(JNum(Beat, TEXT("auto"), -1.0));
	const float Len = static_cast<float>(FMath::Max(JStr(Beat, TEXT("fa")).Len(), JStr(Beat, TEXT("en")).Len()));
	const float TypeRate = FMath::Max(1.f, static_cast<float>(JNum(Beat, TEXT("rate"), 30.0)));
	const float Auto = Explicit >= 0.f ? Explicit : FMath::Max(1.6f, 2.f + 0.06f * Len - Len / TypeRate);
	const float Waited = Dialogue.GetTimeSinceComplete();
	const bool bAuto = Waited >= Auto || (bDevAuto && Waited >= 1.1f);
	if ((bPressed && Waited > 0.12f) || bAuto)
	{
		const FString Next = PeekNextType();
		if (Next != TEXT("text"))
		{
			Dialogue.Hide();
		}
		return true;
	}
	return false;
}

bool UCHStoryDirector::UpdateBeat(const FJsonObject& Beat, float DeltaTime)
{
	const FString Type = JStr(Beat, TEXT("type"));

	if (Type == TEXT("fade"))
	{
		return !JBool(Beat, TEXT("wait"), true) || FMath::IsNearlyEqual(FadeAlpha, FadeTarget, 0.001f);
	}
	if (Type == TEXT("silence") || Type == TEXT("wait"))
	{
		return BeatTime >= static_cast<float>(JNum(Beat, TEXT("seconds"), 1.0));
	}
	if (Type == TEXT("card") || Type == TEXT("chapter"))
	{
		if (ConsumeAdvance())
		{
			UI->Cards->Skip();
		}
		return UI->Cards->IsFinished();
	}
	if (Type == TEXT("title"))
	{
		// the title, alone in its own scene: it fades in, holds, then fades out on its own
		if (BeatPhase == 0)
		{
			if (BeatTime > 6.f || (UI->Cards->GetTime() > 2.5f && ConsumeAdvance()))
			{
				UI->Cards->Dismiss(2.0f);
				BeatPhase = 1;
			}
			return false;
		}
		return UI->Cards->IsFinished();
	}
	if (Type == TEXT("language_select"))
	{
		if (BeatPhase == 0)
		{
			int32 Picked = PollMenu(*UI->Menu);
			if (Picked == INDEX_NONE && bDevAuto && BeatTime > 1.5f)
			{
				Picked = static_cast<int32>(CHSettings::GetLanguage());
			}
			if (Picked != INDEX_NONE && Picked >= 0 && Picked <= 2)
			{
				CHSettings::SetLanguage(static_cast<ECHLanguage>(Picked));
				UI->Menu->Hide();
				BeatPhase = 1;
			}
			return false;
		}
		return UI->Menu->IsGone();
	}
	if (Type == TEXT("poem"))
	{
		if (ConsumeAdvance())
		{
			UI->Cards->Skip();
		}
		if (!UI->Cards->IsFinished())
		{
			return false;
		}
		const TArray<TSharedPtr<FJsonObject>> Couplets = CHJson::Objects(Beat, TEXT("lines"));
		if (Couplets.IsValidIndex(BeatPhase))
		{
			const FString Fa = JStr(*Couplets[BeatPhase], TEXT("fa"));
			const FString En = JStr(*Couplets[BeatPhase], TEXT("en"));
			const FString De = JStr(*Couplets[BeatPhase], TEXT("de"));
			UI->Cards->Play(MakePoemCard(Fa, En, De, 2.4f + 0.055f * FMath::Max(Fa.Len(), En.Len())));
			++BeatPhase;
			return false;
		}
		return true;
	}
	if (Type == TEXT("loading"))
	{
		return BeatTime >= static_cast<float>(JNum(Beat, TEXT("seconds"), 3.0));
	}
	if (Type == TEXT("mainmenu"))
	{
		// phase 0: "press any key". 1: the menu. 2: new game (fading out). 3: settings.
		if (BeatPhase == 0)
		{
			const bool bKey = (bAnyKeyPressed || bClickPressed) && BeatTime > 3.f;
			if (bKey || (bDevAuto && BeatTime > 6.f))
			{
				UI->Captions->Dismiss(0.4f);
				ShowMainMenuOptions();
				BeatPhase = 1;
				BeatTime = 0.f;
			}
			return false;
		}
		if (BeatPhase == 1 || BeatPhase == 3)
		{
			int32 Picked = PollMenu(*UI->Menu);
			if (BeatPhase == 1 && bDevAuto && BeatTime > 3.f && Picked == INDEX_NONE)
			{
				Picked = bMenuHasContinue ? 1 : 0;
			}
			if (Picked == INDEX_NONE)
			{
				return false;
			}
			if (BeatPhase == 3)
			{
				if (ActivateSetting(Picked, *UI->Menu))
				{
					ShowMainMenuOptions();
					BeatPhase = 1;
					BeatTime = 0.f;
				}
				return false;
			}
			const int32 Action = bMenuHasContinue ? Picked : Picked + 1;
			if (Action == 0)
			{
				// Continue: the last checkpoint, no intro cards
				LoadCheckpoint();
				return false;
			}
			if (Action == 1)
			{
				ResetRun();
				UI->Menu->Hide();
				UI->Cards->Dismiss(1.6f);
				BeatPhase = 2;
			}
			else if (Action == 2)
			{
				ShowSettings(*UI->Menu);
				BeatPhase = 3;
			}
			else
			{
				UKismetSystemLibrary::QuitGame(GetWorld(), GetController(), EQuitPreference::Quit, false);
			}
			return false;
		}
		return UI->Cards->IsFinished() && UI->Menu->IsGone();
	}
	if (Type == TEXT("gameplay"))
	{
		if (bSegmentDone)
		{
			bPlayerControl = false;
			return true;
		}
		return false;
	}
	if (Type == TEXT("text"))
	{
		return UpdateDialogueLine(Beat);
	}
	if (Type == TEXT("choice"))
	{
		// phase 0: choosing. 1: his chosen words are typed out. 2: the list fades away.
		if (BeatPhase == 0)
		{
			int32 Picked = PollMenu(*UI->Menu);
			if (Picked == INDEX_NONE && UI->Menu->IsReady() && bDevAuto && BeatTime > 2.5f)
			{
				Picked = FMath::Clamp(DevPick, 0, UI->Menu->Num() - 1);
			}
			const TArray<TSharedPtr<FJsonObject>> Options = CHJson::Objects(Beat, TEXT("options"));
			if (Options.IsValidIndex(Picked))
			{
				const FJsonObject& Option = *Options[Picked];
				SetFlag(JStr(Beat, TEXT("flag")), JStr(Option, TEXT("value"), FString::FromInt(Picked)));
				UI->Menu->Hide();

				const bool bSpeak = JBool(Beat, TEXT("speak"), true) && !JBool(Option, TEXT("silent"), false) && !JStr(Option, TEXT("fa")).IsEmpty();
				if (bSpeak)
				{
					FString SpeakerFa;
					FString SpeakerEn;
					SpeakerNames(JStr(Beat, TEXT("who"), TEXT("he")), SpeakerFa, SpeakerEn);
					UI->Dialogue->ShowLine(SpeakerFa, SpeakerEn, JStr(Option, TEXT("fa")), JStr(Option, TEXT("en")));
					BeatPhase = 1;
				}
				else
				{
					BeatPhase = 2;
				}
			}
			return false;
		}
		if (BeatPhase == 1)
		{
			return UpdateDialogueLine(Beat);
		}
		return UI->Menu->IsGone();
	}
	if (Type == TEXT("phone"))
	{
		return UpdatePhoneBeat(Beat, DeltaTime);
	}
	if (Type == TEXT("npc"))
	{
		return !WaitNpc.IsValid() || !WaitNpc->IsMoving();
	}
	if (Type == TEXT("cut"))
	{
		return BeatTime >= static_cast<float>(JNum(Beat, TEXT("hold"), 0.0));
	}
	if (Type == TEXT("montage"))
	{
		if (ConsumeAdvance() && BeatTime > 2.f)
		{
			UI->Montage->Skip();
		}
		return UI->Montage->IsFinished();
	}
	if (Type == TEXT("show_clip"))
	{
		return UI->Montage->IsFinished();
	}
	if (Type == TEXT("video"))
	{
		const float MemorialAt = static_cast<float>(JNum(Beat, TEXT("memorial_at"), -1.0));
		const bool bDevDone = bDevAuto && BeatTime > 6.f;
		if (BeatPhase == 0 && MemorialAt >= 0.f && Video && (Video->GetTime() >= MemorialAt || bDevDone || Video->IsFinished()))
		{
			ShowMemorial();
			BeatPhase = 1;
		}
		if (bDevDone && Video && !Video->IsFinished())
		{
			Video->Stop();
			return true;
		}
		return !Video || Video->IsFinished();
	}
	if (Type == TEXT("letter"))
	{
		if (BeatPhase == 0)
		{
			// the whole letter is on the page; it folds away by itself once there was time to read it
			// (Enter / Space may close it sooner, never before a few seconds)
			const bool bDone = PhaseTime > UI->Letter->GetReadSeconds() || (ConsumeAdvance() && PhaseTime > 6.f) || (bDevAuto && PhaseTime > 8.f);
			if (bDone)
			{
				UI->Letter->Hide();
	if (Video)
	{
		Video->Stop();
	}
				BeatPhase = 1;
			}
			return false;
		}
		return UI->Letter->IsGone();
	}
	if (Type == TEXT("eye"))
	{
		return !JBool(Beat, TEXT("wait"), false) || FMath::IsNearlyEqual(UI->EyeClose, UI->EyeTarget, 0.001f);
	}

	// instant beats: caption, control, camera, event, flag, place, hide_text and anything unknown
	return true;
}

void UCHStoryDirector::FinishSegment()
{
	bSegmentDone = true;
}

void UCHStoryDirector::SetFlag(const FString& Name, const FString& Value)
{
	if (!Name.IsEmpty())
	{
		Flags.Add(Name, Value);
		UE_LOG(LogCheshmhayash, Log, TEXT("Flag %s = %s"), *Name, *Value);
	}
}

FString UCHStoryDirector::GetFlag(const FString& Name) const
{
	const FString* Found = Flags.Find(Name);
	return Found ? *Found : FString();
}

void UCHStoryDirector::InputPause()
{
	// not on the menus, not between locations
	if (!UI.IsValid() || bTraveling || !Scenes.IsValidIndex(SceneIndex) || Scenes[SceneIndex].Chapter < 1)
	{
		return;
	}
	SetPaused(!bPauseMenu);
}

void UCHStoryDirector::ShowPauseOptions()
{
	bPauseSettings = false;
	TArray<FCHOption> Options;
	Options.Emplace(TEXT("ادامه"), TEXT("Resume"));
	Options.Emplace(TEXT("تنظیمات"), TEXT("Settings"));
	Options.Emplace(TEXT("منوی اصلی"), TEXT("Main Menu"));
	Options.Emplace(TEXT("خروج"), TEXT("Quit"));
	UI->PauseMenu->Show(Options, VAlign_Center, FMargin(0.f), 30.f, 18.f);
}

void UCHStoryDirector::ShowMainMenuOptions()
{
	bMenuSettings = false;
	bMenuHasContinue = CHSave::Exists();
	TArray<FCHOption> Options;
	if (bMenuHasContinue)
	{
		Options.Emplace(TEXT("ادامه"), TEXT("Continue"));
	}
	Options.Emplace(TEXT("بازی جدید"), TEXT("New Game"));
	Options.Emplace(TEXT("تنظیمات"), TEXT("Settings"));
	Options.Emplace(TEXT("خروج"), TEXT("Quit"));
	UI->Menu->Show(Options, VAlign_Center, FMargin(0.f, 300.f, 0.f, 0.f), 30.f, 19.f);
}

namespace
{
	const float SensitivitySteps[] = { 0.5f, 0.75f, 1.f, 1.5f, 2.f, 3.f };
	const float VolumeSteps[] = { 0.f, 0.2f, 0.4f, 0.6f, 0.8f, 1.f };

	int32 NearestStep(const float* Steps, int32 Num, float Value)
	{
		int32 Best = 0;
		for (int32 Index = 1; Index < Num; ++Index)
		{
			if (FMath::Abs(Steps[Index] - Value) < FMath::Abs(Steps[Best] - Value))
			{
				Best = Index;
			}
		}
		return Best;
	}
}

void UCHStoryDirector::RefreshSettings(SCHChoiceList& Menu)
{
	const ECHLanguage Lang = CHSettings::GetLanguage();
	const float Sens = CHSettings::GetSensitivity();
	const bool bInvert = CHSettings::GetInvertY();
	const int32 Volume = FMath::RoundToInt(CHSettings::GetVolume() * 100.f);

	const FString LangEn = Lang == ECHLanguage::German ? TEXT("Sprache: Deutsch") : TEXT("Language: English");
	Menu.UpdateOption(0, TEXT("زبان: فارسی"), LangEn);

	if (Lang == ECHLanguage::German)
	{
		Menu.UpdateOption(1, FString::Printf(TEXT("حساسیت ماوس: %s"), *CH::PersianDigits(FString::Printf(TEXT("%.2f"), Sens))),
			FString::Printf(TEXT("Mausempfindlichkeit: %.2fx"), Sens));
		Menu.UpdateOption(2, FString::Printf(TEXT("وارونه کردن محور عمودی: %s"), bInvert ? TEXT("روشن") : TEXT("خاموش")),
			FString::Printf(TEXT("Y-Achse umkehren: %s"), bInvert ? TEXT("Ein") : TEXT("Aus")));
		Menu.UpdateOption(3, FString::Printf(TEXT("صدا: %s٪"), *CH::PersianDigits(FString::FromInt(Volume))),
			FString::Printf(TEXT("Lautstärke: %d%%"), Volume));
		Menu.UpdateOption(4, TEXT("بازگشت"), TEXT("Zurück"));
	}
	else
	{
		Menu.UpdateOption(1, FString::Printf(TEXT("حساسیت ماوس: %s"), *CH::PersianDigits(FString::Printf(TEXT("%.2f"), Sens))),
			FString::Printf(TEXT("Mouse sensitivity: %.2fx"), Sens));
		Menu.UpdateOption(2, FString::Printf(TEXT("وارونه کردن محور عمودی: %s"), bInvert ? TEXT("روشن") : TEXT("خاموش")),
			FString::Printf(TEXT("Invert Y: %s"), bInvert ? TEXT("On") : TEXT("Off")));
		Menu.UpdateOption(3, FString::Printf(TEXT("صدا: %s٪"), *CH::PersianDigits(FString::FromInt(Volume))),
			FString::Printf(TEXT("Volume: %d%%"), Volume));
		Menu.UpdateOption(4, TEXT("بازگشت"), TEXT("Back"));
	}
}

void UCHStoryDirector::ShowSettings(SCHChoiceList& Menu)
{
	TArray<FCHOption> Options;
	Options.Emplace(TEXT("."), TEXT("."));
	Options.Emplace(TEXT("."), TEXT("."));
	Options.Emplace(TEXT("."), TEXT("."));
	Options.Emplace(TEXT("."), TEXT("."));
	Options.Emplace(TEXT("بازگشت"), TEXT("Back"));
	Menu.Show(Options, VAlign_Center, FMargin(0.f, 80.f, 0.f, 0.f), 26.f, 20.f);
	RefreshSettings(Menu);
}

bool UCHStoryDirector::ActivateSetting(int32 Row, SCHChoiceList& Menu)
{
	if (Row == 0)
	{
		const int32 Next = (static_cast<int32>(CHSettings::GetLanguage()) + 1) % 3;
		CHSettings::SetLanguage(static_cast<ECHLanguage>(Next));
		if (!bPauseMenu && UI.IsValid())
		{
			UI->Cards->Play(MakeMenuTitleCard());
		}
	}
	else if (Row == 1)
	{
		const int32 Step = NearestStep(SensitivitySteps, UE_ARRAY_COUNT(SensitivitySteps), CHSettings::GetSensitivity());
		CHSettings::SetSensitivity(SensitivitySteps[(Step + 1) % UE_ARRAY_COUNT(SensitivitySteps)]);
	}
	else if (Row == 2)
	{
		CHSettings::SetInvertY(!CHSettings::GetInvertY());
	}
	else if (Row == 3)
	{
		const int32 Step = NearestStep(VolumeSteps, UE_ARRAY_COUNT(VolumeSteps), CHSettings::GetVolume());
		CHSettings::SetVolume(VolumeSteps[(Step + 1) % UE_ARRAY_COUNT(VolumeSteps)]);
	}
	else
	{
		return true;
	}
	RefreshSettings(Menu);
	return false;
}

void UCHStoryDirector::SetPaused(bool bInPaused)
{
	bPauseMenu = bInPaused;
	// the world all but stops (a true engine pause would stop this director ticking too)
	UGameplayStatics::SetGlobalTimeDilation(this, bInPaused ? 0.0001f : 1.f);
	bAdvancePressed = false;
	bClickPressed = false;
	NavInput = 0;
	DirectChoice = INDEX_NONE;
	if (!UI.IsValid())
	{
		return;
	}
	UI->PauseDimTarget = bInPaused ? 1.f : 0.f;
	if (bInPaused)
	{
		ShowPauseOptions();
	}
	else
	{
		bPauseSettings = false;
		UI->PauseMenu->Hide();
	}
}

void UCHStoryDirector::UpdatePause()
{
	const int32 Picked = PollMenu(*UI->PauseMenu);
	DirectChoice = INDEX_NONE;
	bAdvancePressed = false;
	bClickPressed = false;
	if (Picked == INDEX_NONE)
	{
		return;
	}
	if (bPauseSettings)
	{
		if (ActivateSetting(Picked, *UI->PauseMenu))
		{
			ShowPauseOptions();
		}
		return;
	}
	if (Picked == 0)
	{
		SetPaused(false);
	}
	else if (Picked == 1)
	{
		bPauseSettings = true;
		ShowSettings(*UI->PauseMenu);
	}
	else if (Picked == 2)
	{
		SetPaused(false);
		GoToMainMenu();
	}
	else if (Picked == 3)
	{
		FPlatformMisc::RequestExit(false);
	}
}

void UCHStoryDirector::ResetRun()
{
	Flags.Reset();
	Clips.Reset();
	bRec = false;
	RecSeconds = 0.f;
	Molotovs = 0;
	HintLinesLeft = 5;
	SkipToBeat = 0;
	bControlsShown = false;
}

void UCHStoryDirector::GoToMainMenu()
{
	ResetRun();
	ClearScreen();
	if (Audio)
	{
		Audio->Cut(1.f);
	}
	int32 Index = FindScene(TEXT("menu"));
	if (Index == INDEX_NONE)
	{
		Index = 0;
	}
	// always a fresh load of the menu location
	CurrentLocation.Reset();
	StartScene(Index);
}

bool UCHStoryDirector::LoadCheckpoint()
{
	UCHSaveGame* Save = CHSave::Load();
	if (!Save)
	{
		return false;
	}
	const int32 Index = FindScene(Save->SceneId);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	ResetRun();
	ClearScreen();
	if (Audio)
	{
		Audio->Cut(0.8f);
	}
	Flags = Save->Flags;
	for (int32 Id = 0; Id < Save->ClipIds.Num(); ++Id)
	{
		FCHClip Clip;
		Clip.Id = Save->ClipIds[Id];
		Clip.CaptionFa = Save->ClipCaptionsFa.IsValidIndex(Id) ? Save->ClipCaptionsFa[Id] : FString();
		Clip.CaptionEn = Save->ClipCaptionsEn.IsValidIndex(Id) ? Save->ClipCaptionsEn[Id] : FString();
		Clip.Chapter = Save->ClipChapters.IsValidIndex(Id) ? Save->ClipChapters[Id] : 1;
		Clips.Add(Clip);
	}
	Molotovs = Save->Molotovs;
	bRec = Save->bRec;
	RecSeconds = Save->RecSeconds;
	HintLinesLeft = 0;
	bControlsShown = true;
	SkipToBeat = Save->BeatIndex;
	// always rebuild the location
	CurrentLocation.Reset();
	UE_LOG(LogCheshmhayash, Log, TEXT("Continue: scene %s beat %d"), *Save->SceneId, Save->BeatIndex);
	StartScene(Index);
	return true;
}

void UCHStoryDirector::AutoSave(int32 Beat)
{
	if (!Scenes.IsValidIndex(SceneIndex) || bInInterlude)
	{
		return;
	}
	const FCHScene& Scene = Scenes[SceneIndex];
	if (Scene.bDev || Scene.bInterlude || Scene.Chapter < 1)
	{
		return;
	}
	UCHSaveGame* Save = NewObject<UCHSaveGame>(this);
	Save->Chapter = Scene.Chapter;
	Save->SceneId = Scene.Id;
	Save->BeatIndex = Beat;
	Save->Flags = Flags;
	for (const FCHClip& Clip : Clips)
	{
		Save->ClipIds.Add(Clip.Id);
		Save->ClipCaptionsFa.Add(Clip.CaptionFa);
		Save->ClipCaptionsEn.Add(Clip.CaptionEn);
		Save->ClipChapters.Add(Clip.Chapter);
	}
	Save->Molotovs = Molotovs;
	Save->bRec = bRec;
	Save->RecSeconds = RecSeconds;
	CHSave::Write(Save);
	if (UI.IsValid())
	{
		UI->Hud->ShowSaving();
	}
}
void UCHStoryDirector::ClearScreen()
{
	if (!UI.IsValid())
	{
		return;
	}
	UI->Cards->Clear();
	UI->Captions->Clear();
	UI->Menu->Hide();
	UI->Dialogue->Hide();
	UI->Phone->Close();
	UI->Letter->Hide();
	UI->Montage->Stop();
	UI->Slogans->Stop(0.f);
	UI->EyeTarget = UI->EyeClose = 0.f;
}

// ---------------------------------------------------------------------------------------------
// Developer helpers
// ---------------------------------------------------------------------------------------------

void UCHStoryDirector::ParseDevOptions()
{
	const TCHAR* CommandLine = FCommandLine::Get();
	FParse::Value(CommandLine, TEXT("chscene="), DevStartScene);
	FParse::Value(CommandLine, TEXT("chpick="), DevPick);
	bDevAuto = FParse::Param(CommandLine, TEXT("chauto"));
	bDevBot = FParse::Param(CommandLine, TEXT("chbot"));
	bDevGhost = FParse::Param(CommandLine, TEXT("chghost"));
	FString Interludes;
	if (FParse::Value(CommandLine, TEXT("chinterlude="), Interludes, false))
	{
		Interludes.ParseIntoArray(DevInterludes, TEXT(","), true);
	}

	// -chflags=window_seat=A,walk_offer=B : pretend earlier choices were made
	FString DevFlags;
	if (FParse::Value(CommandLine, TEXT("chflags="), DevFlags, false))
	{
		TArray<FString> Pairs;
		DevFlags.ParseIntoArray(Pairs, TEXT(","), true);
		for (const FString& Pair : Pairs)
		{
			FString Name;
			FString Value;
			if (Pair.Split(TEXT("="), &Name, &Value))
			{
				SetFlag(Name, Value);
			}
		}
	}
	bDevQuit = FParse::Param(CommandLine, TEXT("chquit"));
	FParse::Value(CommandLine, TEXT("chpause="), DevPauseAt);
	bDevKeys = WITH_EDITOR || FParse::Param(CommandLine, TEXT("chdev"));

	FString Shots;
	if (FParse::Value(CommandLine, TEXT("chshots="), Shots, false))
	{
		TArray<FString> Parts;
		Shots.ParseIntoArray(Parts, TEXT(","), true);
		for (const FString& Part : Parts)
		{
			DevShotTimes.Add(FCString::Atof(*Part));
		}
		DevShotTimes.Sort();
	}
}

void UCHStoryDirector::DevTick()
{
	// -chinterlude=a,b : play encounters once a gameplay segment has been running for a moment
	// -chpause=12 : open the pause menu after that many seconds (to look at it)
	if (DevPauseAt > 0.f && FPlatformTime::Seconds() - DevStartSeconds >= DevPauseAt)
	{
		DevPauseAt = 0.f;
		InputPause();
	}
	if (bPauseMenu)
	{
		// only the screenshots below
	}
	else if (DevInterludes.Num() > 0 && bPlayerControl && !bInInterlude && !bTraveling && BeatTime > 3.f && Scenes.IsValidIndex(SceneIndex)
		&& Scenes[SceneIndex].Beats.IsValidIndex(BeatIndex) && JStr(*Scenes[SceneIndex].Beats[BeatIndex], TEXT("type")) == TEXT("gameplay"))
	{
		PlayInterlude(DevInterludes[0]);
		DevInterludes.RemoveAt(0);
	}

	if (DevShotTimes.Num() == 0)
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	if (DevShotTimes.IsValidIndex(DevNextShot))
	{
		if (Now - DevStartSeconds >= DevShotTimes[DevNextShot])
		{
			const FString File = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Shots") /
				FString::Printf(TEXT("shot_%02d_%04ds.png"), DevNextShot, FMath::RoundToInt32(DevShotTimes[DevNextShot])));
			FScreenshotRequest::RequestScreenshot(File, true, false);
			UE_LOG(LogCheshmhayash, Log, TEXT("Dev screenshot -> %s"), *File);
			++DevNextShot;
			DevLastShotSeconds = Now;
		}
	}
	else if (bDevQuit && Now - DevLastShotSeconds > 2.0)
	{
		bDevQuit = false;
		FPlatformMisc::RequestExit(false);
	}
}

void UCHStoryDirector::DevSkipScene()
{
	if (!bDevKeys || bTraveling || !UI.IsValid())
	{
		return;
	}
	ClearScreen();
	NextScene();
}
