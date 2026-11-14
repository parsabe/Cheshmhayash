// Cheshm Hayash - a moment worth filming.

#include "Game/CHFilmable.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Game/CHSegment.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/CHLevelBuilder.h"

ACHFilmable::ACHFilmable()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	Sprite = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(SceneRoot);
	Sprite->SetStaticMesh(PlaneFinder.Object);
	Sprite->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Sprite->SetCastShadow(false);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(SceneRoot);
	Light->SetIntensityUnits(ELightUnits::Candelas);
	Light->Intensity = 8.f;
	Light->LightColor = FColor(255, 214, 160);
	Light->AttenuationRadius = 380.f;
	Light->CastShadows = false;
	Light->VolumetricScatteringIntensity = 2.f;
}

void ACHFilmable::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Glow.M_CH_Glow"), nullptr, LOAD_NoWarn))
	{
		SpriteMaterial = UMaterialInstanceDynamic::Create(Base, this);
		SpriteMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.82f, 0.55f));
		SpriteMaterial->SetScalarParameterValue(TEXT("Soft"), 1.f);
		SpriteMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.55f);
		SpriteMaterial->SetScalarParameterValue(TEXT("Strength"), 3.f);
		Sprite->SetMaterial(0, SpriteMaterial);
	}
	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
	{
		Segment->AddFilmable(this);
	}
}

void ACHFilmable::Setup(const FString& InClipId, const FString& InCaptionFa, const FString& InCaptionEn, float InFilmSeconds, float InRange)
{
	ClipId = InClipId;
	CaptionFa = InCaptionFa;
	CaptionEn = InCaptionEn;
	FilmSeconds = InFilmSeconds;
	Range = InRange;
}

void ACHFilmable::SetGlowVisible(bool bVisible)
{
	Sprite->SetVisibility(bVisible);
	Light->SetVisibility(bVisible);
	if (!bVisible)
	{
		Sprite->SetHiddenInGame(true);
		Light->SetHiddenInGame(true);
	}
}

void ACHFilmable::MarkFilmed()
{
	// hidden at once so the glow is not in the captured clip
	bFilmed = true;
	Progress = 0.f;
	Fade = 0.f;
	Sprite->SetVisibility(false);
	Light->SetVisibility(false);
}

void ACHFilmable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Time += DeltaSeconds;

	if (bFilmed)
	{
		Fade = FMath::Max(0.f, Fade - DeltaSeconds / 1.2f);
	}
	if (Fade <= 0.f)
	{
		Sprite->SetVisibility(false);
		Light->SetVisibility(false);
		SetActorTickEnabled(false);
		return;
	}

	// a slow breathing glow; it brightens while he films it
	const float Breath = 0.65f + 0.35f * FMath::Sin(Time * 2.2f);
	const float Strength = CHLight::Emissive(0.9f + 1.6f * Progress) * Breath * Fade;
	if (SpriteMaterial)
	{
		SpriteMaterial->SetScalarParameterValue(TEXT("Strength"), Strength);
		SpriteMaterial->SetScalarParameterValue(TEXT("Opacity"), (0.4f + 0.3f * Progress) * Fade);
	}
	Light->SetIntensity((5.f + 10.f * Progress) * Breath * Fade);

	// face the camera
	if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector ToCamera = (Camera->GetCameraLocation() - GetActorLocation()).GetSafeNormal();
		if (!ToCamera.IsNearlyZero())
		{
			Sprite->SetWorldRotation(FRotationMatrix::MakeFromZ(ToCamera).Rotator());
		}
	}
	const float Size = GlowSize * (1.f + 0.08f * FMath::Sin(Time * 2.2f) + 0.25f * Progress) / 100.f;
	Sprite->SetRelativeScale3D(FVector(Size, Size, 1.f));
}
