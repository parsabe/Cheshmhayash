// Cheshm Hayash - trailer mode (console: TrailerMode, or the -chtrailer command line option).
// Plays the "trailer.*" scenes from Content/Story/trailer.json one after another: short shots
// from the real game with smooth camera moves, no HUD (subtitles stay), no saving, no score
// (the trailer's own music is laid over the recording afterwards, see Scripts/make_trailer.ps1).
// Every shot starts and ends on black, so the editing script can find the cuts by itself.
// Nothing here runs outside trailer mode.

#include "Story/CHStoryDirector.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Cheshmhayash.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Engine.h"
#include "UI/SCHSlogans.h"
#include "Components/PostProcessComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/CHPlayerController.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Story/CHAudio.h"
#include "Story/CHJson.h"
#include "UI/CHStyle.h"
#include "UI/SCHCardLayer.h"
#include "UI/SCHDialogueOverlay.h"
#include "UI/SCHHud.h"
#include "UI/SCHRootOverlay.h"
#include "World/CHFire.h"
#include "World/CHLevelBuilder.h"
#include "World/CHNpc.h"

namespace
{
	FString JStr(const FJsonObject& O, const TCHAR* K, const FString& D = FString()) { return CHJson::Str(O, K, D); }
	double JNum(const FJsonObject& O, const TCHAR* K, double D) { return CHJson::Num(O, K, D); }
	bool JBool(const FJsonObject& O, const TCHAR* K, bool D) { return CHJson::Bool(O, K, D); }

	FVector JVec(const FJsonObject& Beat, const TCHAR* Field, const FVector& Default = FVector::ZeroVector)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (Beat.TryGetArrayField(Field, Values) && Values && Values->Num() >= 3)
		{
			return FVector((*Values)[0]->AsNumber(), (*Values)[1]->AsNumber(), (*Values)[2]->AsNumber());
		}
		return Default;
	}

	UMaterialInstanceDynamic* GlowMaterial(UObject* Outer, const FLinearColor& Color, float Strength, float Opacity)
	{
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Glow.M_CH_Glow"));
		UMaterialInstanceDynamic* Material = Base ? UMaterialInstanceDynamic::Create(Base, Outer) : nullptr;
		if (Material)
		{
			Material->SetVectorParameterValue(TEXT("Color"), Color);
			Material->SetScalarParameterValue(TEXT("Strength"), Strength);
			Material->SetScalarParameterValue(TEXT("Opacity"), Opacity);
			Material->SetScalarParameterValue(TEXT("Soft"), 1.f);
		}
		return Material;
	}

	FAutoConsoleCommandWithWorld GTrailerModeCommand(
		TEXT("TrailerMode"),
		TEXT("Plays the jam trailer: about 12 short shots from the game, no HUD."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UCHStoryDirector* Director = UCHStoryDirector::Get(World))
			{
				Director->StartTrailer();
			}
		}));
}

void UCHStoryDirector::StartTrailer()
{
	// -chscene=trailer.06.molotov starts the trailer from that shot (to look at one shot)
	int32 First = DevStartScene.StartsWith(TEXT("trailer.")) ? FindScene(DevStartScene) : INDEX_NONE;
	for (int32 Index = 0; Index < Scenes.Num() && First == INDEX_NONE; ++Index)
	{
		if (Scenes[Index].Id.StartsWith(TEXT("trailer.")))
		{
			First = Index;
		}
	}
	if (First == INDEX_NONE)
	{
		UE_LOG(LogCheshmhayash, Warning, TEXT("TrailerMode: Content/Story/trailer.json has no trailer.* scenes"));
		return;
	}
	UE_LOG(LogCheshmhayash, Log, TEXT("TrailerMode: starting"));
	bTrailer = true;
	// nothing but the picture: no engine warnings printed over the shots
	if (GEngine)
	{
		GEngine->bEnableOnScreenDebugMessages = false;
		GAreScreenMessagesEnabled = false;
	}
	if (bPauseMenu)
	{
		SetPaused(false);
	}
	ClearScreen();
	if (Audio)
	{
		Audio->Cut(0.5f);
	}
	bRec = false;
	StartScene(First);
}

bool UCHStoryDirector::NextTrailerScene()
{
	for (int32 Index = SceneIndex + 1; Index < Scenes.Num(); ++Index)
	{
		if (Scenes[Index].Id.StartsWith(TEXT("trailer.")))
		{
			StartScene(Index);
			return true;
		}
	}
	// the end of the trailer: stay on black a moment, then back to the menu
	UE_LOG(LogCheshmhayash, Log, TEXT("TrailerMode: finished"));
	bTrailer = false;
	ClearTrailerRig();
	if (GEngine)
	{
		GEngine->bEnableOnScreenDebugMessages = true;
		GAreScreenMessagesEnabled = true;
	}
	return false;
}

void UCHStoryDirector::ClearTrailerRig()
{
	for (const TPair<FString, TObjectPtr<ACameraActor>>& Pair : TrailerCams)
	{
		if (Pair.Value)
		{
			if (IsValid(Pair.Value)) { Pair.Value->Destroy(); }
		}
	}
	TrailerCams.Reset();
	if (IsValid(TrailerDollyCam))
	{
		TrailerDollyCam->Destroy();
	}
	TrailerDollyCam = nullptr;
	if (IsValid(TrailerHeadlight))
	{
		TrailerHeadlight->Destroy();
	}
	TrailerHeadlight = nullptr;
	for (AActor* Actor : TrailerProps)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	TrailerProps.Reset();
	TrailerArc.Reset();
	if (IsValid(TrailerBottle))
	{
		TrailerBottle->Destroy();
	}
	TrailerBottle = nullptr;
	DollyTime = -1.f;
	MolotovTime = -1.f;
}

ACameraActor* UCHStoryDirector::FindTrailerCamera(const FString& Name)
{
	if (const TObjectPtr<ACameraActor>* Found = TrailerCams.Find(Name))
	{
		return Found->Get();
	}
	return Builder.IsValid() ? Builder->GetCamera(FName(*Name)) : nullptr;
}

AActor* UCHStoryDirector::SpawnTrailerMesh(const TCHAR* MeshPath, const FVector& Location, const FVector& Scale, UMaterialInterface* Material)
{
	UWorld* World = GetWorld();
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
	if (!World || !Mesh)
	{
		return nullptr;
	}
	AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(Location));
	if (!Actor)
	{
		return nullptr;
	}
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Actor);
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetStaticMesh(Mesh);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetCastShadow(false);
	if (Material)
	{
		Component->SetMaterial(0, Material);
	}
	Actor->SetRootComponent(Component);
	Component->RegisterComponent();
	Actor->SetActorLocation(Location);
	Actor->SetActorScale3D(Scale);
	TrailerProps.Add(Actor);
	return Actor;
}

bool UCHStoryDirector::BeginTrailerBeat(const FString& Type, const FJsonObject& Beat)
{
	if (!Type.StartsWith(TEXT("t_")))
	{
		return false;
	}
	UWorld* World = GetWorld();
	ACHPlayerController* Controller = GetController();

	if (Type == TEXT("t_grade"))
	{
		// the strongest look: more fog, more grain, a deeper vignette
		if (Builder.IsValid())
		{
			if (UPostProcessComponent* Post = Builder->GetPostProcess())
			{
				FPostProcessSettings& S = Post->Settings;
				S.bOverride_FilmGrainIntensity = true;
				S.FilmGrainIntensity = static_cast<float>(JNum(Beat, TEXT("grain"), 0.45));
				S.bOverride_VignetteIntensity = true;
				S.VignetteIntensity = static_cast<float>(JNum(Beat, TEXT("vignette"), 0.6));
				S.bOverride_BloomIntensity = true;
				S.BloomIntensity = FMath::Max(S.BloomIntensity, static_cast<float>(JNum(Beat, TEXT("bloom"), 1.0)));
				const float Contrast = static_cast<float>(JNum(Beat, TEXT("contrast"), 1.1));
				S.bOverride_ColorContrast = true;
				S.ColorContrast = FVector4(Contrast, Contrast, Contrast, 1.0);
			}
			if (UExponentialHeightFogComponent* Fog = Builder->GetFog())
			{
				Fog->SetFogDensity(Fog->FogDensity * static_cast<float>(JNum(Beat, TEXT("fog"), 1.3)));
			}
			// "night": a real night for the trailer (the game's nights are kept bright for play)
			if (JBool(Beat, TEXT("night"), false))
			{
				if (UDirectionalLightComponent* Sun = Builder->GetSun())
				{
					Sun->SetIntensity(Sun->Intensity * 0.18f);
				}
				if (UDirectionalLightComponent* Moon = Builder->GetMoon())
				{
					Moon->SetIntensity(Moon->Intensity * 0.3f);
					Moon->ForwardShadingPriority = -1;
				}
				if (USkyLightComponent* Sky = Builder->GetSkyLight())
				{
					Sky->SetIntensity(Sky->Intensity * 0.25f);
				}
				if (UPostProcessComponent* Post = Builder->GetPostProcess())
				{
					const float Darker = static_cast<float>(JNum(Beat, TEXT("darker"), 0.6));
					Post->Settings.AutoExposureMinBrightness += Darker;
					Post->Settings.AutoExposureMaxBrightness += Darker;
				}
				if (UExponentialHeightFogComponent* Fog = Builder->GetFog())
				{
					Fog->SetFogInscatteringColor(FLinearColor(0.03f, 0.035f, 0.06f));
				}
			}
		}
	}
	else if (Type == TEXT("t_cam"))
	{
		// a camera placed by coordinates: "pos", "look", "fov"
		const FString Name = JStr(Beat, TEXT("name"));
		const FVector Pos = JVec(Beat, TEXT("pos"));
		const FVector Look = JVec(Beat, TEXT("look"), Pos + FVector(100.f, 0.f, 0.f));
		ACameraActor* Camera = nullptr;
		if (const TObjectPtr<ACameraActor>* Found = TrailerCams.Find(Name))
		{
			Camera = Found->Get();
		}
		if (!Camera && World)
		{
			Camera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform(Pos));
			TrailerCams.Add(Name, Camera);
		}
		if (Camera)
		{
			Camera->SetActorLocationAndRotation(Pos, (Look - Pos).Rotation());
			Camera->GetCameraComponent()->SetFieldOfView(static_cast<float>(JNum(Beat, TEXT("fov"), 60.0)));
			Camera->GetCameraComponent()->bConstrainAspectRatio = false;
		}
	}
	else if (Type == TEXT("t_dolly"))
	{
		// a smooth move between two cameras over "time" seconds; "bob" adds the sway of running
		ACameraActor* From = FindTrailerCamera(JStr(Beat, TEXT("from")));
		ACameraActor* To = FindTrailerCamera(JStr(Beat, TEXT("to")));
		if (From && To && World)
		{
			if (!TrailerDollyCam)
			{
				TrailerDollyCam = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), From->GetActorTransform());
			}
			DollyFrom = From->GetActorTransform();
			DollyTo = To->GetActorTransform();
			DollyFovFrom = From->GetCameraComponent()->FieldOfView;
			DollyFovTo = To->GetCameraComponent()->FieldOfView;
			DollyDuration = FMath::Max(0.1f, static_cast<float>(JNum(Beat, TEXT("time"), 5.0)));
			DollyBob = static_cast<float>(JNum(Beat, TEXT("bob"), 0.0));
			DollyEase = JBool(Beat, TEXT("ease"), true);
			DollyTime = 0.f;
			TrailerDollyCam->SetActorTransform(DollyFrom);
			TrailerDollyCam->GetCameraComponent()->SetFieldOfView(DollyFovFrom);
			TrailerDollyCam->GetCameraComponent()->bConstrainAspectRatio = false;
			if (Controller)
			{
				Controller->SetSceneCamera(TrailerDollyCam, 0.f);
			}
		}
	}
	else if (Type == TEXT("t_headlight"))
	{
		// motorbike lights behind the camera, coming after him
		if (JBool(Beat, TEXT("on"), true) && World && !TrailerHeadlight)
		{
			TrailerHeadlight = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
			USpotLightComponent* Light = NewObject<USpotLightComponent>(TrailerHeadlight);
			Light->SetMobility(EComponentMobility::Movable);
			Light->SetIntensityUnits(ELightUnits::Candelas);
			Light->Intensity = static_cast<float>(JNum(Beat, TEXT("intensity"), 1800.0));
			Light->LightColor = FColor(230, 236, 255);
			Light->AttenuationRadius = 4500.f;
			Light->OuterConeAngle = 30.f;
			Light->InnerConeAngle = 12.f;
			Light->CastShadows = true;
			Light->VolumetricScatteringIntensity = 4.f;
			TrailerHeadlight->SetRootComponent(Light);
			Light->RegisterComponent();
			HeadlightOffset = JVec(Beat, TEXT("offset"), FVector(-650.f, 90.f, -60.f));
		}
		else if (!JBool(Beat, TEXT("on"), true) && TrailerHeadlight)
		{
			TrailerHeadlight->Destroy();
			TrailerHeadlight = nullptr;
		}
	}
	else if (Type == TEXT("t_npc"))
	{
		// people placed by coordinates: "at", "yaw", "look", "pose", and optionally "walk" + "speed"
		if (Builder.IsValid())
		{
			const FString Name = JStr(Beat, TEXT("name"));
			ACHNpc* Npc = Builder->FindNpc(Name);
			if (!Npc && Beat.HasField(TEXT("at")))
			{
				Npc = Builder->SpawnNpc(Name, JStr(Beat, TEXT("look"), TEXT("man")), JVec(Beat, TEXT("at")), static_cast<float>(JNum(Beat, TEXT("yaw"), 0.0)), static_cast<int32>(JNum(Beat, TEXT("seed"), 0.0)));
			}
			if (Npc)
			{
				if (Beat.HasField(TEXT("pose")))
				{
					Npc->SetPose(JStr(Beat, TEXT("pose")));
				}
				if (Beat.HasField(TEXT("walk")))
				{
					Npc->SetPose(TEXT(""));
					Npc->WalkTo(JVec(Beat, TEXT("walk")), static_cast<float>(JNum(Beat, TEXT("speed"), 150.0)));
				}
			}
		}
	}
	else if (Type == TEXT("t_molotov"))
	{
		// the arc shows first ("arc" seconds), then the bottle flies it and the street catches fire
		MolotovFrom = JVec(Beat, TEXT("from"));
		MolotovTo = JVec(Beat, TEXT("to"));
		MolotovArcTime = static_cast<float>(JNum(Beat, TEXT("arc"), 1.0));
		MolotovFlight = static_cast<float>(JNum(Beat, TEXT("time"), 1.1));
		MolotovHeight = static_cast<float>(JNum(Beat, TEXT("height"), 260.0));
		MolotovFlee.Reset();
		MolotovFlee = CHJson::Lines(Beat, TEXT("flee"));
		MolotovFleeDistance = static_cast<float>(JNum(Beat, TEXT("flee_dx"), 900.0));
		MolotovTime = 0.f;
		TrailerArc.Reset();
		UMaterialInstanceDynamic* Dot = GlowMaterial(this, FLinearColor(1.f, 0.85f, 0.55f), CHLight::Emissive(6.f), 0.9f);
		for (int32 Index = 1; Index <= 16; ++Index)
		{
			const float T = Index / 16.f;
			const FVector At = FMath::Lerp(MolotovFrom, MolotovTo, T) + FVector(0.f, 0.f, 4.f * MolotovHeight * T * (1.f - T));
			if (AActor* Mark = SpawnTrailerMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"), At, FVector(Index == 16 ? 0.5f : 0.07f), Dot))
			{
				TrailerArc.Add(Mark);
			}
		}
		UMaterialInstanceDynamic* Rag = GlowMaterial(this, FLinearColor(1.f, 0.55f, 0.15f), CHLight::Emissive(10.f), 0.95f);
		TrailerBottle = SpawnTrailerMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), MolotovFrom, FVector(0.07f, 0.07f, 0.22f), Rag);
	}
	else if (Type == TEXT("t_wall_text"))
	{
		if (Builder.IsValid())
		{
			Builder->TrailerWallText(JStr(Beat, TEXT("text")), JVec(Beat, TEXT("at")), static_cast<float>(JNum(Beat, TEXT("yaw"), 0.0)), static_cast<float>(JNum(Beat, TEXT("size"), 60.0)), FLinearColor(0.6f, 0.02f, 0.015f));
		}
	}
	else if (Type == TEXT("t_light"))
	{
		// an extra light for the shot (the strongest look)
		if (World)
		{
			const FVector At = JVec(Beat, TEXT("at"));
			const FVector Look = JVec(Beat, TEXT("look"), At - FVector(0.f, 0.f, 100.f));
			AActor* Rig = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform((Look - At).Rotation(), At));
			if (Rig)
			{
				USpotLightComponent* Light = NewObject<USpotLightComponent>(Rig);
				Light->SetMobility(EComponentMobility::Movable);
				Light->SetIntensityUnits(ELightUnits::Candelas);
				Light->Intensity = static_cast<float>(JNum(Beat, TEXT("intensity"), 800.0));
				Light->LightColor = FColor(255, 170, 90);
				Light->AttenuationRadius = 3000.f;
				Light->OuterConeAngle = 40.f;
				Light->InnerConeAngle = 20.f;
				Light->CastShadows = true;
				Rig->SetRootComponent(Light);
				Light->RegisterComponent();
				Rig->SetActorLocationAndRotation(At, (Look - At).Rotation());
				TrailerProps.Add(Rig);
			}
		}
	}
	else if (Type == TEXT("t_slogans"))
	{
		// the rooftop chants, always in Farsi (rendered by the game)
		UI->Slogans->Start(JStr(Beat, TEXT("fa")), static_cast<int32>(JNum(Beat, TEXT("count"), 120.0)), static_cast<float>(JNum(Beat, TEXT("time"), 4.0)), true);
	}
	else if (Type == TEXT("t_clear"))
	{
		// clears the people named <prefix>0..N out of the shot
		if (Builder.IsValid())
		{
			const FString Prefix = JStr(Beat, TEXT("prefix"), TEXT("crowd"));
			for (int32 Index = 0; Index < 120; ++Index)
			{
				Builder->RemoveNpc(FString::Printf(TEXT("%s%d"), *Prefix, Index));
			}
		}
	}
	else if (Type == TEXT("t_interact"))
	{
		if (Builder.IsValid())
		{
			Builder->OnInteract(JStr(Beat, TEXT("id")), nullptr);
		}
	}
	else if (Type == TEXT("t_title"))
	{
		// the closing title of the trailer: both languages, always, rendered by the game
		FCHCard Card;
		Card.bSkippable = false;
		Card.Hold = static_cast<float>(JNum(Beat, TEXT("hold"), 3.5));
		Card.FadeOut = 1.2f;
		FCHCardLine Fa(TEXT("چشم‌هایش"), true, 120.f, CH::EWeight::Bold, CH::Col::Paper);
		Fa.FadeIn = 0.4f;
		FCHCardLine En(TEXT("CHESHM HAYASH"), false, 26.f, CH::EWeight::Regular, CH::Col::PaperDim);
		En.LetterSpacing = 600;
		En.TopPadding = 6.f;
		En.FadeIn = 0.6f;
		En.Delay = 0.3f;
		FCHCardLine Memory(TEXT("A game in loving memory of Hesam"), false, 22.f, CH::EWeight::Light, CH::Col::Paper);
		Memory.TopPadding = 34.f;
		Memory.FadeIn = 0.9f;
		Memory.Delay = 1.0f;
		Card.Lines = { Fa, En, Memory };
		UI->Cards->Play(Card);
	}
	return true;
}

bool UCHStoryDirector::UpdateTrailerBeat(const FString& Type, const FJsonObject& Beat, float DeltaTime, bool& bOutDone)
{
	if (!Type.StartsWith(TEXT("t_")))
	{
		return false;
	}
	if (Type == TEXT("t_title"))
	{
		bOutDone = UI->Cards->IsFinished();
	}
	else if (Type == TEXT("t_dolly") && JBool(Beat, TEXT("wait"), false))
	{
		bOutDone = DollyTime < 0.f || DollyTime >= DollyDuration;
	}
	else
	{
		bOutDone = true;
	}
	return true;
}

void UCHStoryDirector::TickTrailer(float DeltaTime)
{
	if (!bTrailer)
	{
		return;
	}
	if (UI.IsValid())
	{
		// no HUD, no hints, no REC, no saving icon; subtitles stay
		UI->Hud->SetVisibility(EVisibility::Collapsed);
		UI->Dialogue->SetHint(false);
	}

	if (TrailerDollyCam && DollyTime >= 0.f)
	{
		DollyTime = FMath::Min(DollyTime + DeltaTime, DollyDuration);
		const float Raw = DollyTime / DollyDuration;
		const float T = DollyEase ? FMath::InterpEaseInOut(0.f, 1.f, Raw, 1.6f) : Raw;
		FVector Location = FMath::Lerp(DollyFrom.GetLocation(), DollyTo.GetLocation(), T);
		FQuat Rotation = FQuat::Slerp(DollyFrom.GetRotation(), DollyTo.GetRotation(), T);
		if (DollyBob > 0.f)
		{
			// a runner's sway: the head bobs twice per stride, rolls a little
			const float Stride = DollyTime * 2.6f * UE_TWO_PI;
			Location.Z += FMath::Sin(Stride) * DollyBob;
			Location += Rotation.GetRightVector() * FMath::Sin(Stride * 0.5f) * DollyBob * 0.6f;
			Rotation = Rotation * FQuat(FRotator(FMath::Sin(Stride) * 0.6f, 0.f, FMath::Sin(Stride * 0.5f) * 1.2f));
		}
		TrailerDollyCam->SetActorLocationAndRotation(Location, Rotation);
		TrailerDollyCam->GetCameraComponent()->SetFieldOfView(FMath::Lerp(DollyFovFrom, DollyFovTo, T));
	}

	if (TrailerHeadlight)
	{
		AActor* View = TrailerDollyCam ? static_cast<AActor*>(TrailerDollyCam) : nullptr;
		if (View)
		{
			const FTransform Eye = View->GetActorTransform();
			const float Flicker = 1.f + 0.04f * FMath::Sin(DollyTime * 37.f);
			TrailerHeadlight->SetActorLocationAndRotation(Eye.TransformPosition(HeadlightOffset * Flicker), Eye.GetRotation());
		}
	}

	if (MolotovTime >= 0.f)
	{
		MolotovTime += DeltaTime;
		const float FlyT = (MolotovTime - MolotovArcTime) / FMath::Max(0.1f, MolotovFlight);
		if (FlyT >= 0.f && TrailerBottle)
		{
			const float T = FMath::Min(FlyT, 1.f);
			const FVector At = FMath::Lerp(MolotovFrom, MolotovTo, T) + FVector(0.f, 0.f, 4.f * MolotovHeight * T * (1.f - T));
			TrailerBottle->SetActorLocationAndRotation(At, FRotator(MolotovTime * 540.f, 0.f, 0.f));
			// the arc fades away as the bottle flies it
			for (AActor* Mark : TrailerArc)
			{
				if (Mark)
				{
					Mark->SetActorHiddenInGame(true);
				}
			}
			if (FlyT >= 1.f)
			{
				TrailerBottle->Destroy();
				TrailerBottle = nullptr;
				MolotovTime = -1.f;
				if (UWorld* World = GetWorld())
				{
					if (ACHFire* Fire = World->SpawnActor<ACHFire>(ACHFire::StaticClass(), FTransform(MolotovTo * FVector(1.f, 1.f, 0.f))))
					{
						Fire->Setup(1.5f, true, true, 31);
						TrailerProps.Add(Fire);
					}
					if (ACHFire* Spread = World->SpawnActor<ACHFire>(ACHFire::StaticClass(), FTransform(MolotovTo * FVector(1.f, 1.f, 0.f) + FVector(60.f, 260.f, 0.f))))
					{
						Spread->Setup(1.0f, false, false, 32);
						TrailerProps.Add(Spread);
					}
				}
				Flash(FLinearColor(1.f, 0.7f, 0.35f), 0.4f);
				if (ACHPlayerController* Controller = GetController())
				{
					Controller->Shake(0.8f, 1.6f);
				}
				// the fire stops them: they fall back
				if (Builder.IsValid())
				{
					for (const FString& Name : MolotovFlee)
					{
						if (ACHNpc* Npc = Builder->FindNpc(Name))
						{
							Npc->SetPose(TEXT(""));
							Npc->WalkTo(Npc->GetActorLocation() + FVector(MolotovFleeDistance, FMath::FRandRange(-150.f, 150.f), 0.f), FMath::FRandRange(260.f, 360.f));
						}
					}
				}
			}
		}
	}
}
