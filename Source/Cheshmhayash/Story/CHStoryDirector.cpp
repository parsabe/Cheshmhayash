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
#include "UI/CHStyle.h"
#include "UI/SCHCardLayer.h"
#include "UI/SCHChoiceList.h"
#include "UI/SCHDialogueOverlay.h"
#include "UI/SCHMontage.h"
#include "UI/SCHPhoneOverlay.h"
#include "UI/SCHRootOverlay.h"
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

	FCHCard MakeOpeningCard(const TArray<FString>& Fa, const TArray<FString>& En, float Hold)
	{
		FCHCard Card;
		float Time = 0.5f;
		for (int32 i = 0; i < Fa.Num(); ++i)
		{
			FCHCardLine Line(Fa[i], true, 34.f, CH::EWeight::Regular, CH::Col::Paper);
			Line.TopPadding = i == 0 ? 0.f : 12.f;
			Line.Delay = Time;
			Line.FadeIn = 1.6f;
			Time += 1.7f;
			Card.Lines.Add(Line);
		}
		Time += 0.5f;
		for (int32 i = 0; i < En.Num(); ++i)
		{
			FCHCardLine Line(En[i], false, 21.f, CH::EWeight::Light, CH::Col::PaperDim);
			Line.TopPadding = i == 0 ? 50.f : 9.f;
			Line.Delay = Time;
			Line.FadeIn = 1.4f;
			Time += 1.1f;
			Card.Lines.Add(Line);
		}
		Card.Hold = Hold;
		Card.FadeOut = 1.8f;
		return Card;
	}

	FCHCard MakePlainCard(const TArray<FString>& Fa, const TArray<FString>& En, float Hold, float FaSize, float EnSize)
	{
		FCHCard Card;
		float Time = 0.3f;
		for (int32 i = 0; i < Fa.Num(); ++i)
		{
			FCHCardLine Line(Fa[i], true, FaSize, CH::EWeight::Regular, CH::Col::Paper);
			Line.TopPadding = i == 0 ? 0.f : 10.f;
			Line.Delay = Time;
			Line.FadeIn = 1.4f;
			Time += 0.9f;
			Card.Lines.Add(Line);
		}
		for (int32 i = 0; i < En.Num(); ++i)
		{
			FCHCardLine Line(En[i], false, EnSize, CH::EWeight::Light, CH::Col::PaperDim);
			Line.TopPadding = i == 0 ? 26.f : 8.f;
			Line.Delay = Time;
			Line.FadeIn = 1.4f;
			Time += 0.7f;
			Card.Lines.Add(Line);
		}
		Card.Hold = Hold;
		return Card;
	}

	FCHCard MakeChapterCard(const FJsonObject& Beat)
	{
		FCHCard Card;

		FCHCardLine FaSub(JStr(Beat, TEXT("fa_sub")), true, 23.f, CH::EWeight::Regular, CH::Col::PaperDim);
		FaSub.Delay = 0.3f;
		FaSub.FadeIn = 1.4f;
		Card.Lines.Add(FaSub);

		FCHCardLine FaTitle(JStr(Beat, TEXT("fa_title")), true, 96.f, CH::EWeight::Bold, CH::Col::Paper);
		FaTitle.TopPadding = 14.f;
		FaTitle.Delay = 1.0f;
		FaTitle.FadeIn = 2.0f;
		Card.Lines.Add(FaTitle);

		FCHCardLine EnTitle(JStr(Beat, TEXT("en_title")), false, 30.f, CH::EWeight::Light, CH::Col::Paper);
		EnTitle.TopPadding = 10.f;
		EnTitle.Delay = 1.9f;
		EnTitle.FadeIn = 1.6f;
		Card.Lines.Add(EnTitle);

		FCHCardLine EnSub(JStr(Beat, TEXT("en_sub")), false, 14.f, CH::EWeight::Regular, CH::Col::PaperDim);
		EnSub.TopPadding = 30.f;
		EnSub.LetterSpacing = 420;
		EnSub.Delay = 2.4f;
		EnSub.FadeIn = 1.4f;
		Card.Lines.Add(EnSub);

		Card.Hold = static_cast<float>(JNum(Beat, TEXT("hold"), 3.2));
		Card.FadeOut = 1.8f;
		return Card;
	}

	FCHCard MakeTitleCard()
	{
		FCHCard Card;

		FCHCardLine Title(TEXT("چشم‌هایش"), true, 150.f, CH::EWeight::Bold, CH::Col::Paper);
		Title.Delay = 0.6f;
		Title.FadeIn = 3.2f;
		Card.Lines.Add(Title);

		FCHCardLine Sub(TEXT("CHESHM HAYASH   ·   HIS EYES"), false, 17.f, CH::EWeight::Light, CH::Col::PaperDim);
		Sub.TopPadding = 4.f;
		Sub.LetterSpacing = 560;
		Sub.Delay = 2.8f;
		Sub.FadeIn = 2.2f;
		Card.Lines.Add(Sub);

		Card.Hold = -1.f;
		Card.bSkippable = false;
		Card.FadeOut = 2.0f;
		Card.Padding = FMargin(80.f, 0.f, 80.f, 240.f);
		return Card;
	}

	FCHCard MakeCaptionCard(const FString& Fa, const FString& En, float Hold)
	{
		FCHCard Card;
		Card.HAlign = HAlign_Left;
		Card.VAlign = VAlign_Bottom;
		Card.Padding = FMargin(72.f, 0.f, 72.f, 64.f);
		Card.MaxWidth = 1000.f;
		Card.bSkippable = false;
		Card.Hold = Hold;
		Card.FadeOut = 2.f;

		if (!Fa.IsEmpty())
		{
			FCHCardLine Line(Fa, true, 24.f, CH::EWeight::Regular, CH::Col::Paper);
			Line.Delay = 0.8f;
			Line.FadeIn = 1.5f;
			Card.Lines.Add(Line);
		}
		if (!En.IsEmpty())
		{
			FCHCardLine Line(En, false, 14.f, CH::EWeight::Regular, CH::Col::PaperDim);
			Line.TopPadding = 4.f;
			Line.LetterSpacing = 220;
			Line.Delay = 1.3f;
			Line.FadeIn = 1.5f;
			Card.Lines.Add(Line);
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
	}

	const FCHScene& Scene = Scenes[Index];
	UE_LOG(LogCheshmhayash, Log, TEXT("Scene %s (location '%s' variant '%s')"), *Scene.Id, *Scene.Location, *Scene.Variant);

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

	// the story is over: back to the title with a clean slate
	Flags.Reset();
	Clips.Reset();
	bRec = false;
	RecSeconds = 0.f;
	StartScene(0);
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
		UpdateFade(DeltaTime);
		if (bRec)
		{
			RecSeconds += DeltaTime;
		}
		UI->Hud->SetRec(bRec, RecSeconds, !UI->Cards->IsFinished());
		UI->Advance(DeltaTime);

		// one soft click per typed letter (hooked up to the audio manager in the audio step)
		const bool bDialogueTick = UI->Dialogue->ConsumeTypeTick();
		const bool bPhoneTick = UI->Phone->ConsumeTypeTick();
		if (bDialogueTick || bPhoneTick)
		{
			OnTypeTick(bPhoneTick);
		}
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
	NavInput = 0;
	DirectChoice = INDEX_NONE;
}

void UCHStoryDirector::OnTypeTick(bool bPhone)
{
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
	const bool bPressed = bAdvancePressed;
	bAdvancePressed = false;
	return bPressed;
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

void UCHStoryDirector::BeginBeat(const FJsonObject& Beat)
{
	const FString Type = JStr(Beat, TEXT("type"));
	ACHPlayerController* Controller = GetController();

	if (Type == TEXT("fade"))
	{
		const bool bWhite = JStr(Beat, TEXT("color")) == TEXT("white");
		SetFade(static_cast<float>(JNum(Beat, TEXT("to"), 0.0)), static_cast<float>(JNum(Beat, TEXT("time"), 1.5)), bWhite ? FLinearColor::White : FLinearColor::Black);
	}
	else if (Type == TEXT("card"))
	{
		const FString Style = JStr(Beat, TEXT("style"), TEXT("plain"));
		const float Hold = static_cast<float>(JNum(Beat, TEXT("hold"), 4.0));
		FCHCard Card = Style == TEXT("opening")
			? MakeOpeningCard(JLines(Beat, TEXT("fa")), JLines(Beat, TEXT("en")), Hold)
			: MakePlainCard(JLines(Beat, TEXT("fa")), JLines(Beat, TEXT("en")), Hold,
				static_cast<float>(JNum(Beat, TEXT("fa_size"), 40.0)), static_cast<float>(JNum(Beat, TEXT("en_size"), 22.0)));
		Card.bSkippable = JBool(Beat, TEXT("skippable"), true);
		UI->Cards->Play(Card);
	}
	else if (Type == TEXT("chapter"))
	{
		UI->Cards->Play(MakeChapterCard(Beat));
	}
	else if (Type == TEXT("caption"))
	{
		UI->Captions->Play(MakeCaptionCard(JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")), static_cast<float>(JNum(Beat, TEXT("hold"), 4.5))));
	}
	else if (Type == TEXT("title"))
	{
		UI->Cards->Play(MakeTitleCard());
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
		UI->Dialogue->ShowLine(SpeakerFa, SpeakerEn, JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")), static_cast<float>(JNum(Beat, TEXT("rate"), 30.0)));
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
		UI->Menu->Show(Options, VAlign_Bottom, FMargin(0.f, 0.f, 0.f, 96.f), 30.f, 17.f);
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
			Pawn->SetActorHiddenInGame(JBool(Beat, TEXT("hidden"), false));
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
		// his right eye: "close" 1 = the right half goes dark
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
		UI->Hud->Notify(JStr(Beat, TEXT("title")), JStr(Beat, TEXT("fa")), JStr(Beat, TEXT("en")), static_cast<float>(JNum(Beat, TEXT("seconds"), 8.0)));
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

	// "auto": seconds after the line is complete before moving on without input
	const float Auto = static_cast<float>(JNum(Beat, TEXT("auto"), -1.0));
	const float Waited = Dialogue.GetTimeSinceComplete();
	const bool bAuto = (Auto >= 0.f && Waited >= Auto) || (bDevAuto && Waited >= 1.1f);
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
		// phase 0: title fades in. 1: menu. 2: fading out.
		if (BeatPhase == 0)
		{
			if (UI->Cards->GetTime() > 4.4f || (UI->Cards->GetTime() > 1.5f && ConsumeAdvance()))
			{
				TArray<FCHOption> Options;
				Options.Emplace(TEXT("شروع"), TEXT("Start"));
				Options.Emplace(TEXT("خروج"), TEXT("Quit"));
				UI->Menu->Show(Options, VAlign_Bottom, FMargin(0.f, 0.f, 0.f, 120.f), 30.f, 15.f);
				BeatPhase = 1;
				BeatTime = 0.f;
			}
			return false;
		}
		if (BeatPhase == 1)
		{
			if (NavInput != 0)
			{
				UI->Menu->Move(NavInput > 0 ? 1 : -1);
			}
			const bool bAutoStart = bDevAuto && BeatTime > 3.f;
			if (UI->Menu->IsReady() && (ConsumeAdvance() || bAutoStart))
			{
				if (UI->Menu->GetSelected() == 1 && !bAutoStart)
				{
					UKismetSystemLibrary::QuitGame(GetWorld(), GetController(), EQuitPreference::Quit, false);
					return false;
				}
				UI->Menu->Hide();
				UI->Cards->Dismiss(2.0f);
				BeatPhase = 2;
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
			if (NavInput != 0)
			{
				UI->Menu->Move(NavInput > 0 ? 1 : -1);
			}
			int32 Picked = INDEX_NONE;
			if (UI->Menu->IsReady())
			{
				if (DirectChoice != INDEX_NONE && DirectChoice < UI->Menu->Num())
				{
					Picked = DirectChoice;
				}
				else if (ConsumeAdvance())
				{
					Picked = UI->Menu->GetSelected();
				}
				else if (bDevAuto && BeatTime > 2.5f)
				{
					Picked = FMath::Clamp(DevPick, 0, UI->Menu->Num() - 1);
				}
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
	if (DevInterludes.Num() > 0 && bPlayerControl && !bInInterlude && !bTraveling && BeatTime > 3.f && Scenes.IsValidIndex(SceneIndex)
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
	UI->Cards->Clear();
	UI->Captions->Clear();
	UI->Menu->Hide();
	UI->Dialogue->Hide();
	UI->Phone->Close();
	NextScene();
}
