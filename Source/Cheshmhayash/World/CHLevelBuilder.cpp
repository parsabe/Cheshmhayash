// Cheshm Hayash - base class for every location.

#include "World/CHLevelBuilder.h"
#include "Cheshmhayash.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Game/CHFilmable.h"
#include "Game/CHGuard.h"
#include "Game/CHInteractable.h"
#include "Game/CHSegment.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Story/CHJson.h"
#include "Story/CHStoryDirector.h"
#include "UI/CHStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "World/CHNpc.h"

namespace
{
	float GExposureEV = 0.f;
}

void CHLight::SetExposure(float ExposureEV)
{
	GExposureEV = ExposureEV;
}

float CHLight::GetExposure()
{
	return GExposureEV;
}

float CHLight::DisplayScale()
{
	return 1.2f * FMath::Pow(2.f, GExposureEV);
}

ACHLevelBuilder::ACHLevelBuilder()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	CubeMesh = CubeFinder.Object;
	CylinderMesh = CylinderFinder.Object;
	SphereMesh = SphereFinder.Object;
	ConeMesh = ConeFinder.Object;
	PlaneMesh = PlaneFinder.Object;

	Marks.Add(TEXT("PlayerStart"), FCHMark());
}

void ACHLevelBuilder::Build(const FString& InVariant)
{
	Variant = InVariant;

	// Master materials made by Scripts/create_materials.py. Fall back to the engine shape material.
	LitBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Lit.M_CH_Lit"), nullptr, LOAD_NoWarn);
	UnlitBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Unlit.M_CH_Unlit"), nullptr, LOAD_NoWarn);
	GlowBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Glow.M_CH_Glow"), nullptr, LOAD_NoWarn);
	if (!LitBase)
	{
		UE_LOG(LogCheshmhayash, Warning, TEXT("M_CH_Lit not found - run Scripts/setup_assets.bat. Using the engine shape material."));
		LitBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		bLitBaseIsFallback = true;
	}
	if (!UnlitBase)
	{
		UnlitBase = LitBase;
	}
	if (!GlowBase)
	{
		GlowBase = LitBase;
	}
	WidgetLitBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Widget.M_CH_Widget"), nullptr, LOAD_NoWarn);
	WidgetGlowBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_WidgetGlow.M_CH_WidgetGlow"), nullptr, LOAD_NoWarn);
}

// ---------------------------------------------------------------------------------------------
// People
// ---------------------------------------------------------------------------------------------

ACHNpc* ACHLevelBuilder::SpawnNpc(const FString& Name, const FString& Look, const FVector& FeetLocation, float Yaw, int32 Seed)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACHNpc* Npc = GetWorld()->SpawnActor<ACHNpc>(ACHNpc::StaticClass(), FTransform(FRotator(0.f, Yaw, 0.f), FeetLocation), Params);
	if (Npc)
	{
		Npc->NpcName = Name;
		Npc->SetLook(Look, Seed);
		if (!Name.IsEmpty())
		{
			RemoveNpc(Name);
			Npcs.Add(Name, Npc);
		}
	}
	return Npc;
}

ACHNpc* ACHLevelBuilder::FindNpc(const FString& Name) const
{
	const TObjectPtr<ACHNpc>* Found = Npcs.Find(Name);
	return Found ? Found->Get() : nullptr;
}

void ACHLevelBuilder::RemoveNpc(const FString& Name)
{
	TObjectPtr<ACHNpc> Npc;
	if (Npcs.RemoveAndCopyValue(Name, Npc) && Npc)
	{
		Npc->Destroy();
	}
}

ACHNpc* ACHLevelBuilder::HandleNpcBeat(const FJsonObject& Beat)
{
	const FString Name = CHJson::Str(Beat, TEXT("name"));
	const FString Action = CHJson::Str(Beat, TEXT("action"), TEXT("spawn"));

	FCHMark Where;
	const bool bHasMark = GetMark(FName(*CHJson::Str(Beat, TEXT("mark"))), Where);

	if (Action == TEXT("spawn"))
	{
		const float Yaw = Where.Transform.Rotator().Yaw + CHJson::Float(Beat, TEXT("yaw"), 0.f);
		ACHNpc* Npc = SpawnNpc(Name, CHJson::Str(Beat, TEXT("look"), Name), Where.Transform.GetLocation(), Yaw, static_cast<int32>(CHJson::Num(Beat, TEXT("seed"), 0.0)));
		if (Npc)
		{
			Npc->SetSink(CHJson::Float(Beat, TEXT("sink"), 0.f));
		}
		return nullptr;
	}

	ACHNpc* Npc = FindNpc(Name);
	if (!Npc)
	{
		UE_LOG(LogCheshmhayash, Warning, TEXT("npc beat: '%s' is not in this location"), *Name);
		return nullptr;
	}

	if (Action == TEXT("walk") && bHasMark)
	{
		Npc->WalkTo(Where.Transform.GetLocation(), CHJson::Float(Beat, TEXT("speed"), 120.f));
		return CHJson::Bool(Beat, TEXT("wait"), true) ? Npc : nullptr;
	}
	if (Action == TEXT("face"))
	{
		if (bHasMark)
		{
			Npc->FaceLocation(Where.Transform.GetLocation());
		}
		else
		{
			Npc->FaceYaw(CHJson::Float(Beat, TEXT("yaw"), 0.f));
		}
	}
	else if (Action == TEXT("sink"))
	{
		Npc->SetSink(CHJson::Float(Beat, TEXT("amount"), 45.f));
	}
	else if (Action == TEXT("shake"))
	{
		Npc->SetShaking(CHJson::Bool(Beat, TEXT("on"), true));
	}
	else if (Action == TEXT("remove"))
	{
		RemoveNpc(Name);
	}
	return nullptr;
}

// ---------------------------------------------------------------------------------------------
// Gameplay
// ---------------------------------------------------------------------------------------------

void ACHLevelBuilder::StartSegment(const FString& Segment, const TSharedPtr<FJsonObject>& Beat)
{
	UCHSegmentSubsystem* Seg = UCHSegmentSubsystem::Get(this);
	if (!Seg)
	{
		return;
	}
	FTransform Respawn = GetPlayerStart();
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Respawn = Pawn->GetActorTransform();
	}
	const UCHStoryDirector* Director = UCHStoryDirector::Get(this);
	Seg->Begin(this, Respawn, Director ? Director->GetSegmentChances() : 0);
	if (Beat.IsValid())
	{
		Seg->SetHint(CHJson::Str(*Beat, TEXT("hint_fa")), CHJson::Str(*Beat, TEXT("hint_en")));
		Seg->bFilmAllowed = CHJson::Bool(*Beat, TEXT("film"), true);
	}
}

void ACHLevelBuilder::OnSegmentExit(const FString& Id)
{
	FinishSegment();
}

void ACHLevelBuilder::FinishSegment()
{
	if (UCHSegmentSubsystem* Seg = UCHSegmentSubsystem::Get(this))
	{
		Seg->End();
	}
	if (UCHStoryDirector* Director = UCHStoryDirector::Get(this))
	{
		Director->FinishSegment();
	}
}

ACHGuard* ACHLevelBuilder::SpawnGuard(ECHGuardKind Kind, const FString& Look, const TArray<FVector>& Route, float Yaw, int32 Seed)
{
	if (Route.Num() == 0)
	{
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	const FVector Location = Route[0] + FVector(0.f, 0.f, 94.f);
	ACHGuard* Guard = GetWorld()->SpawnActor<ACHGuard>(ACHGuard::StaticClass(), FTransform(FRotator(0.f, Yaw, 0.f), Location), Params);
	if (Guard)
	{
		Guard->Setup(Kind, Look, Route, Seed);
	}
	return Guard;
}

ACHFilmable* ACHLevelBuilder::SpawnFilmable(const FString& Id, const FString& Fa, const FString& En, const FVector& Location, float Seconds, float Range)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACHFilmable* Filmable = GetWorld()->SpawnActor<ACHFilmable>(ACHFilmable::StaticClass(), FTransform(Location), Params);
	if (Filmable)
	{
		Filmable->Setup(Id, Fa, En, Seconds, Range);
	}
	return Filmable;
}

ACHInteractable* ACHLevelBuilder::SpawnInteractable(const FString& Id, const FString& Fa, const FString& En, const FVector& Location, float Radius, bool bMarker)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACHInteractable* Interactable = GetWorld()->SpawnActor<ACHInteractable>(ACHInteractable::StaticClass(), FTransform(Location), Params);
	if (Interactable)
	{
		Interactable->Setup(Id, Fa, En, Radius, bMarker);
	}
	return Interactable;
}

// ---------------------------------------------------------------------------------------------
// World-space Slate
// ---------------------------------------------------------------------------------------------

UWidgetComponent* ACHLevelBuilder::WorldWidget(const TSharedRef<SWidget>& Widget, const FVector& Location, const FRotator& Rotation,
	float CmPerPixel, bool bGlow, const FVector2D& DrawSize)
{
	UWidgetComponent* Component = NewObject<UWidgetComponent>(this);
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetupAttachment(Root);
	Component->SetRelativeLocationAndRotation(Location, Rotation);
	Component->SetRelativeScale3D(FVector(CmPerPixel));
	Component->SetWidgetSpace(EWidgetSpace::World);
	Component->SetPivot(FVector2D(0.5, 0.5));
	Component->SetBackgroundColor(FLinearColor::Transparent);
	Component->SetBlendMode(bGlow ? EWidgetBlendMode::Transparent : EWidgetBlendMode::Masked);
	Component->SetTwoSided(false);
	Component->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Component->SetCastShadow(false);
	Component->SetRedrawTime(0.5f);
	if (DrawSize.IsNearlyZero())
	{
		Component->SetDrawAtDesiredSize(true);
	}
	else
	{
		Component->SetDrawAtDesiredSize(false);
		Component->SetDrawSize(DrawSize);
	}
	if (UMaterialInterface* Base = bGlow ? WidgetGlowBase.Get() : WidgetLitBase.Get())
	{
		Component->SetMaterial(0, Base);
	}
	Component->SetSlateWidget(Widget);
	Component->RegisterComponent();
	return Component;
}

UWidgetComponent* ACHLevelBuilder::WorldText(const FString& Text, bool bFarsi, float FontSize, const FLinearColor& Color, const FVector& Location,
	const FRotator& Rotation, float CmPerPixel, bool bGlow, bool bBold)
{
	const CH::EWeight Weight = bBold ? CH::EWeight::Bold : CH::EWeight::Regular;
	TSharedRef<SWidget> Widget = SNew(SBox)
		.Padding(FMargin(8.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Text))
			.Font(bFarsi ? CH::FarsiFont(FontSize, Weight) : CH::LatinFont(FontSize, Weight))
			.ColorAndOpacity(FSlateColor(Color))
			.Justification(ETextJustify::Center)
			.TextShapingMethod(ETextShapingMethod::FullShaping)
			.TextFlowDirection(bFarsi ? ETextFlowDirection::RightToLeft : ETextFlowDirection::LeftToRight)
		];
	return WorldWidget(Widget, Location, Rotation, CmPerPixel, bGlow);
}

// ---------------------------------------------------------------------------------------------
// Materials
// ---------------------------------------------------------------------------------------------

UMaterialInstanceDynamic* ACHLevelBuilder::MakeMaterial(const FString& Key, UMaterialInterface* Base)
{
	if (TObjectPtr<UMaterialInstanceDynamic>* Found = MaterialCache.Find(Key))
	{
		return *Found;
	}
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, this);
	MaterialCache.Add(Key, Material);
	return Material;
}

static FString ColorKey(const FLinearColor& Color)
{
	return FString::Printf(TEXT("%d_%d_%d_%d"),
		FMath::RoundToInt32(Color.R * 1000.f), FMath::RoundToInt32(Color.G * 1000.f),
		FMath::RoundToInt32(Color.B * 1000.f), FMath::RoundToInt32(Color.A * 1000.f));
}

UMaterialInstanceDynamic* ACHLevelBuilder::Lit(const FLinearColor& Color, float Roughness, float Grime, float Metallic)
{
	const FString Key = FString::Printf(TEXT("L_%s_%d_%d_%d"), *ColorKey(Color),
		FMath::RoundToInt32(Roughness * 100.f), FMath::RoundToInt32(Grime * 100.f), FMath::RoundToInt32(Metallic * 100.f));
	const bool bNew = !MaterialCache.Contains(Key);
	UMaterialInstanceDynamic* Material = MakeMaterial(Key, LitBase);
	if (bNew && Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		if (!bLitBaseIsFallback)
		{
			Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
			Material->SetScalarParameterValue(TEXT("Grime"), Grime);
			Material->SetScalarParameterValue(TEXT("Metallic"), Metallic);
		}
	}
	return Material;
}

UMaterialInstanceDynamic* ACHLevelBuilder::Emissive(const FLinearColor& Color, float Strength, const FLinearColor& BodyColor)
{
	const FString Key = FString::Printf(TEXT("E_%s_%d_%s"), *ColorKey(Color), FMath::RoundToInt32(Strength * 100.f), *ColorKey(BodyColor));
	const bool bNew = !MaterialCache.Contains(Key);
	UMaterialInstanceDynamic* Material = MakeMaterial(Key, LitBase);
	if (bNew && Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), bLitBaseIsFallback ? Color : BodyColor);
		if (!bLitBaseIsFallback)
		{
			Material->SetVectorParameterValue(TEXT("Emissive"), Color);
			Material->SetScalarParameterValue(TEXT("EmissiveStrength"), Strength);
			Material->SetScalarParameterValue(TEXT("Grime"), 0.f);
			Material->SetScalarParameterValue(TEXT("Roughness"), 0.6f);
		}
	}
	return Material;
}

UMaterialInstanceDynamic* ACHLevelBuilder::Unlit(const FLinearColor& Color, float Strength)
{
	const FString Key = FString::Printf(TEXT("U_%s_%d"), *ColorKey(Color), FMath::RoundToInt32(Strength * 100.f));
	const bool bNew = !MaterialCache.Contains(Key);
	UMaterialInstanceDynamic* Material = MakeMaterial(Key, UnlitBase);
	if (bNew && Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetScalarParameterValue(TEXT("Strength"), Strength);
	}
	return Material;
}

UMaterialInstanceDynamic* ACHLevelBuilder::Glow(const FLinearColor& Color, float Strength, float Opacity, float Soft)
{
	const FString Key = FString::Printf(TEXT("G_%s_%d_%d_%d"), *ColorKey(Color),
		FMath::RoundToInt32(Strength * 100.f), FMath::RoundToInt32(Opacity * 1000.f), FMath::RoundToInt32(Soft * 100.f));
	const bool bNew = !MaterialCache.Contains(Key);
	UMaterialInstanceDynamic* Material = MakeMaterial(Key, GlowBase);
	if (bNew && Material)
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetScalarParameterValue(TEXT("Strength"), Strength);
		Material->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		Material->SetScalarParameterValue(TEXT("Soft"), Soft);
	}
	return Material;
}

// ---------------------------------------------------------------------------------------------
// Shapes
// ---------------------------------------------------------------------------------------------

UStaticMeshComponent* ACHLevelBuilder::AddMesh(UStaticMesh* Mesh, const FTransform& Transform, UMaterialInterface* Material, bool bCollision, bool bMovable, USceneComponent* Parent)
{
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this);
	Component->SetMobility((bMovable || Parent) ? EComponentMobility::Movable : EComponentMobility::Static);
	Component->SetStaticMesh(Mesh);
	if (Material)
	{
		Component->SetMaterial(0, Material);
	}
	Component->SetCollisionProfileName(bCollision ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
	Component->SetCanEverAffectNavigation(false);
	Component->SetupAttachment(Parent ? Parent : Root.Get());
	Component->SetRelativeTransform(Transform);
	Component->RegisterComponent();
	return Component;
}

UStaticMeshComponent* ACHLevelBuilder::Box(const FVector& Center, const FVector& Size, UMaterialInterface* Material, bool bCollision, const FRotator& Rotation, bool bMovable)
{
	return AddMesh(CubeMesh, FTransform(Rotation, Center, Size / 100.0), Material, bCollision, bMovable);
}

UStaticMeshComponent* ACHLevelBuilder::BoxAt(float MinX, float MinY, float BaseZ, float SizeX, float SizeY, float SizeZ, UMaterialInterface* Material, bool bCollision)
{
	return Box(FVector(MinX + SizeX * 0.5f, MinY + SizeY * 0.5f, BaseZ + SizeZ * 0.5f), FVector(SizeX, SizeY, SizeZ), Material, bCollision);
}

UStaticMeshComponent* ACHLevelBuilder::Cyl(const FVector& Center, float Radius, float Height, UMaterialInterface* Material, bool bCollision, const FRotator& Rotation, bool bMovable)
{
	return AddMesh(CylinderMesh, FTransform(Rotation, Center, FVector(Radius / 50.0, Radius / 50.0, Height / 100.0)), Material, bCollision, bMovable);
}

UStaticMeshComponent* ACHLevelBuilder::Ball(const FVector& Center, const FVector& Size, UMaterialInterface* Material, bool bCollision, const FRotator& Rotation, bool bMovable)
{
	return AddMesh(SphereMesh, FTransform(Rotation, Center, Size / 100.0), Material, bCollision, bMovable);
}

UStaticMeshComponent* ACHLevelBuilder::Cone(const FVector& Center, float Radius, float Height, UMaterialInterface* Material, bool bCollision, const FRotator& Rotation, bool bMovable)
{
	return AddMesh(ConeMesh, FTransform(Rotation, Center, FVector(Radius / 50.0, Radius / 50.0, Height / 100.0)), Material, bCollision, bMovable);
}

UStaticMeshComponent* ACHLevelBuilder::Quad(const FVector& Center, const FVector2D& Size, const FRotator& Rotation, UMaterialInterface* Material, bool bMovable)
{
	UStaticMeshComponent* Component = AddMesh(PlaneMesh, FTransform(Rotation, Center, FVector(Size.X / 100.0, Size.Y / 100.0, 1.0)), Material, false, bMovable);
	Component->SetCastShadow(false);
	return Component;
}

void ACHLevelBuilder::BoxInstance(const FVector& Center, const FVector& Size, UMaterialInterface* Material, const FRotator& Rotation)
{
	TObjectPtr<UInstancedStaticMeshComponent>& Instancer = BoxInstancers.FindOrAdd(Material);
	if (!Instancer)
	{
		Instancer = NewObject<UInstancedStaticMeshComponent>(this);
		Instancer->SetMobility(EComponentMobility::Static);
		Instancer->SetStaticMesh(CubeMesh);
		if (Material)
		{
			Instancer->SetMaterial(0, Material);
		}
		Instancer->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Instancer->SetCanEverAffectNavigation(false);
		Instancer->SetupAttachment(Root);
		Instancer->RegisterComponent();
	}
	Instancer->AddInstance(FTransform(Rotation, Center, Size / 100.0), true);
}

UStaticMeshComponent* ACHLevelBuilder::BoxOn(USceneComponent* Parent, const FVector& RelativeCenter, const FVector& Size, UMaterialInterface* Material, bool bCollision, const FRotator& RelativeRotation)
{
	return AddMesh(CubeMesh, FTransform(RelativeRotation, RelativeCenter, Size / 100.0), Material, bCollision, true, Parent);
}

USceneComponent* ACHLevelBuilder::Pivot(const FVector& Location, const FRotator& Rotation)
{
	USceneComponent* Component = NewObject<USceneComponent>(this);
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetupAttachment(Root);
	Component->SetRelativeLocationAndRotation(Location, Rotation);
	Component->RegisterComponent();
	return Component;
}

// ---------------------------------------------------------------------------------------------
// Lights
// ---------------------------------------------------------------------------------------------

UPointLightComponent* ACHLevelBuilder::PointLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, bool bShadows, float Volumetric)
{
	UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetupAttachment(Root);
	Light->SetRelativeLocation(Location);
	Light->SetIntensityUnits(ELightUnits::Candelas);
	Light->Intensity = Intensity;
	Light->LightColor = Color.ToFColor(true);
	Light->AttenuationRadius = Radius;
	Light->SourceRadius = 4.f;
	Light->SoftSourceRadius = 8.f;
	Light->bUseInverseSquaredFalloff = true;
	Light->CastShadows = bShadows;
	Light->VolumetricScatteringIntensity = Volumetric;
	Light->RegisterComponent();
	return Light;
}

USpotLightComponent* ACHLevelBuilder::SpotLight(const FVector& Location, const FRotator& Rotation, const FLinearColor& Color, float Intensity, float Radius, float OuterCone, bool bShadows, float Volumetric)
{
	USpotLightComponent* Light = NewObject<USpotLightComponent>(this);
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetupAttachment(Root);
	Light->SetRelativeLocationAndRotation(Location, Rotation);
	Light->SetIntensityUnits(ELightUnits::Candelas);
	Light->Intensity = Intensity;
	Light->LightColor = Color.ToFColor(true);
	Light->AttenuationRadius = Radius;
	Light->OuterConeAngle = OuterCone;
	Light->InnerConeAngle = OuterCone * 0.55f;
	Light->SourceRadius = 4.f;
	Light->bUseInverseSquaredFalloff = true;
	Light->CastShadows = bShadows;
	Light->VolumetricScatteringIntensity = Volumetric;
	Light->RegisterComponent();
	return Light;
}

// ---------------------------------------------------------------------------------------------
// Environment
// ---------------------------------------------------------------------------------------------

void ACHLevelBuilder::ApplyEnv(const FCHEnv& Env)
{
	CHLight::SetExposure(Env.ExposureEV);

	// --- sun
	if (!Sun)
	{
		Sun = NewObject<UDirectionalLightComponent>(this);
		Sun->SetMobility(EComponentMobility::Movable);
		Sun->SetupAttachment(Root);
		Sun->SetAtmosphereSunLight(true);
		Sun->SetAtmosphereSunLightIndex(0);
		Sun->RegisterComponent();
	}
	Sun->SetWorldRotation(Env.SunRotation);
	Sun->SetIntensity(Env.SunIntensity);
	Sun->SetLightColor(Env.SunColor);
	Sun->SetCastShadows(Env.bSunShadows);
	Sun->SetVolumetricScatteringIntensity(Env.SunVolumetric);
	Sun->SetLightingChannels(Env.bSunLightsWorld, false, false);

	// --- moon / fill
	if (Env.bMoon)
	{
		if (!Moon)
		{
			Moon = NewObject<UDirectionalLightComponent>(this);
			Moon->SetMobility(EComponentMobility::Movable);
			Moon->SetupAttachment(Root);
			Moon->SetAtmosphereSunLight(false);
			Moon->RegisterComponent();
		}
		Moon->SetWorldRotation(Env.MoonRotation);
		Moon->SetIntensity(Env.MoonIntensity);
		Moon->SetLightColor(Env.MoonColor);
		Moon->SetCastShadows(true);
		Moon->SetVisibility(true);
	}
	else if (Moon)
	{
		Moon->SetVisibility(false);
	}

	// --- sky
	if (Env.bAtmosphere && !Atmosphere)
	{
		Atmosphere = NewObject<USkyAtmosphereComponent>(this);
		Atmosphere->SetMobility(EComponentMobility::Movable);
		Atmosphere->SetupAttachment(Root);
		Atmosphere->RegisterComponent();
	}
	if (!SkyLight)
	{
		SkyLight = NewObject<USkyLightComponent>(this);
		SkyLight->SetMobility(EComponentMobility::Movable);
		SkyLight->SetupAttachment(Root);
		// with no sky to capture the engine prints an on-screen warning: closed rooms go without
		SkyLight->SetRealTimeCapture(Env.bAtmosphere);
		SkyLight->bLowerHemisphereIsBlack = false;
		SkyLight->RegisterComponent();
	}
	SkyLight->SetIntensity(Env.SkyLightIntensity);
	SkyLight->SetLightColor(Env.SkyLightColor);

	// --- fog
	if (Env.bFog)
	{
		if (!Fog)
		{
			Fog = NewObject<UExponentialHeightFogComponent>(this);
			Fog->SetMobility(EComponentMobility::Movable);
			Fog->SetupAttachment(Root);
			Fog->RegisterComponent();
		}
		Fog->SetWorldLocation(FVector(0.f, 0.f, Env.FogZ));
		Fog->SetFogDensity(Env.FogDensity);
		Fog->SetFogHeightFalloff(Env.FogHeightFalloff);
		Fog->SetFogInscatteringColor(Env.FogColor);
		Fog->SetStartDistance(Env.FogStartDistance);
		Fog->SetFogMaxOpacity(Env.FogMaxOpacity);
		Fog->SetVolumetricFog(Env.bVolumetricFog);
		Fog->SetVolumetricFogExtinctionScale(Env.VolumetricExtinction);
		Fog->SetVisibility(true);
	}
	else if (Fog)
	{
		Fog->SetVisibility(false);
	}

	// --- post process
	if (!PostProcess)
	{
		PostProcess = NewObject<UPostProcessComponent>(this);
		PostProcess->SetupAttachment(Root);
		PostProcess->bUnbound = true;
		PostProcess->Priority = 1.f;
		PostProcess->RegisterComponent();
	}
	FPostProcessSettings& S = PostProcess->Settings;

	S.bOverride_AutoExposureMethod = true;
	S.AutoExposureMethod = AEM_Histogram;
	S.bOverride_AutoExposureMinBrightness = true;
	S.bOverride_AutoExposureMaxBrightness = true;
	S.AutoExposureMinBrightness = Env.ExposureEV;
	S.AutoExposureMaxBrightness = Env.ExposureEV;
	S.bOverride_AutoExposureBias = true;
	S.AutoExposureBias = 0.f;
	S.bOverride_LocalExposureHighlightContrastScale = true;
	S.LocalExposureHighlightContrastScale = 1.f;
	S.bOverride_LocalExposureShadowContrastScale = true;
	S.LocalExposureShadowContrastScale = 1.f;

	S.bOverride_FilmGrainIntensity = true;
	S.FilmGrainIntensity = Env.Grain;
	S.bOverride_VignetteIntensity = true;
	S.VignetteIntensity = Env.Vignette;
	S.bOverride_SceneFringeIntensity = true;
	S.SceneFringeIntensity = Env.Fringe;
	S.bOverride_BloomIntensity = true;
	S.BloomIntensity = Env.Bloom;
	S.bOverride_MotionBlurAmount = true;
	S.MotionBlurAmount = 0.15f;
	S.bOverride_LensFlareIntensity = true;
	S.LensFlareIntensity = 0.f;

	S.bOverride_ColorSaturation = true;
	S.ColorSaturation = FVector4(Env.Saturation, Env.Saturation, Env.Saturation, 1.0);
	S.bOverride_ColorContrast = true;
	S.ColorContrast = FVector4(Env.Contrast, Env.Contrast, Env.Contrast, 1.0);
	S.bOverride_ColorGain = true;
	S.ColorGain = FVector4(Env.Gain.R, Env.Gain.G, Env.Gain.B, 1.0);
	S.bOverride_WhiteTemp = true;
	S.WhiteTemp = Env.Temperature;
}

void ACHLevelBuilder::BuildAlborz(float YawDegrees, const FLinearColor& RockColor, const FLinearColor& SnowColor, float Distance, bool bLit)
{
	const FLinearColor FarColor = RockColor * 1.25f + FLinearColor(0.004f, 0.006f, 0.010f);
	UMaterialInterface* Rock = bLit ? Lit(RockColor, 0.95f, 0.f) : Unlit(RockColor);
	UMaterialInterface* RockFar = bLit ? Lit(FarColor, 0.95f, 0.f) : Unlit(FarColor);
	UMaterialInterface* Snow = bLit ? Lit(SnowColor, 0.6f, 0.f) : Unlit(SnowColor);

	const float BaseZ = -0.02f * Distance;
	auto Peak = [&](float OffsetDegrees, float DistanceScale, float Radius, float Height, UMaterialInterface* Material)
	{
		const FVector Direction = FRotator(0.f, YawDegrees + OffsetDegrees, 0.f).Vector();
		const FVector Base = Direction * Distance * DistanceScale;
		UStaticMeshComponent* Mesh = Cone(FVector(Base.X, Base.Y, BaseZ + Height * 0.5f), Radius, Height, Material, false);
		Mesh->SetCastShadow(false);
		return Base;
	};

	// Damavand: a broad, almost symmetric volcano with a snow cap
	const float DamavandHeight = 0.17f * Distance;
	const float DamavandRadius = 0.31f * Distance;
	const FVector DamavandBase = Peak(0.f, 1.f, DamavandRadius, DamavandHeight, Rock);
	{
		const float CapFraction = 0.36f;
		const float CapHeight = DamavandHeight * CapFraction;
		UStaticMeshComponent* Cap = Cone(
			FVector(DamavandBase.X, DamavandBase.Y, BaseZ + DamavandHeight - CapHeight * 0.5f + DamavandHeight * 0.006f),
			DamavandRadius * CapFraction * 1.05f, CapHeight, Snow, false);
		Cap->SetCastShadow(false);
	}

	// the range to both sides, lower and further back
	const float Offsets[] = { -78.f, -61.f, -47.f, -34.f, -22.f, -11.f, 12.f, 24.f, 37.f, 50.f, 64.f, 80.f };
	const float Heights[] = { 0.050f, 0.074f, 0.060f, 0.088f, 0.066f, 0.094f, 0.082f, 0.058f, 0.090f, 0.064f, 0.078f, 0.052f };
	const float Radii[] = { 0.30f, 0.34f, 0.26f, 0.36f, 0.28f, 0.33f, 0.31f, 0.27f, 0.37f, 0.29f, 0.35f, 0.30f };
	for (int32 i = 0; i < UE_ARRAY_COUNT(Offsets); ++i)
	{
		Peak(Offsets[i], (i % 2 == 0) ? 1.12f : 1.04f, Radii[i] * Distance, Heights[i] * Distance, (i % 2 == 0) ? RockFar : Rock);
	}
}

// ---------------------------------------------------------------------------------------------
// Marks and cameras
// ---------------------------------------------------------------------------------------------

void ACHLevelBuilder::Mark(FName Name, const FVector& Location, const FRotator& Rotation, float FOV)
{
	FCHMark NewMark;
	NewMark.Transform = FTransform(Rotation, Location);
	NewMark.FOV = FOV;
	Marks.Add(Name, NewMark);
}

void ACHLevelBuilder::MarkLookAt(FName Name, const FVector& Location, const FVector& Target, float FOV)
{
	Mark(Name, Location, UKismetMathLibrary::FindLookAtRotation(Location, Target), FOV);
}

bool ACHLevelBuilder::GetMark(FName Name, FCHMark& Out) const
{
	if (const FCHMark* Found = Marks.Find(Name))
	{
		Out = *Found;
		return true;
	}
	return false;
}

FTransform ACHLevelBuilder::GetPlayerStart() const
{
	// marks for people are on the floor; the player's capsule centre is 92 cm above its feet
	FCHMark Start;
	GetMark(TEXT("PlayerStart"), Start);
	FTransform Transform = Start.Transform;
	Transform.AddToTranslation(FVector(0.f, 0.f, 94.f));
	return Transform;
}

ACameraActor* ACHLevelBuilder::GetCamera(FName MarkName)
{
	if (TObjectPtr<ACameraActor>* Found = Cameras.Find(MarkName))
	{
		return *Found;
	}
	FCHMark CameraMark;
	if (!GetMark(MarkName, CameraMark))
	{
		UE_LOG(LogCheshmhayash, Warning, TEXT("%s: camera mark '%s' not found"), *GetName(), *MarkName.ToString());
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraMark.Transform, Params);
	if (Camera)
	{
		if (UCameraComponent* CameraComponent = Camera->GetCameraComponent())
		{
			CameraComponent->SetFieldOfView(CameraMark.FOV);
			CameraComponent->SetConstraintAspectRatio(false);
		}
		Cameras.Add(MarkName, Camera);
	}
	return Camera;
}
