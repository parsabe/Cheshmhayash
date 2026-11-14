// Cheshm Hayash - something he (or she) can use.

#include "Game/CHInteractable.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Game/CHSegment.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/CHLevelBuilder.h"

ACHInteractable::ACHInteractable()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
	Marker->SetupAttachment(SceneRoot);
	Marker->SetStaticMesh(PlaneFinder.Object);
	Marker->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Marker->SetCastShadow(false);
	Marker->SetRelativeScale3D(FVector(0.32f, 0.32f, 1.f));
}

void ACHInteractable::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Glow.M_CH_Glow"), nullptr, LOAD_NoWarn))
	{
		MarkerMaterial = UMaterialInstanceDynamic::Create(Base, this);
		MarkerMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.93f, 0.8f));
		MarkerMaterial->SetScalarParameterValue(TEXT("Soft"), 1.f);
		MarkerMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.6f);
		MarkerMaterial->SetScalarParameterValue(TEXT("Strength"), 2.f);
		Marker->SetMaterial(0, MarkerMaterial);
	}
	if (UCHSegmentSubsystem* Segment = UCHSegmentSubsystem::Get(this))
	{
		Segment->AddInteractable(this);
	}
}

void ACHInteractable::Setup(const FString& InId, const FString& InPromptFa, const FString& InPromptEn, float InRadius, bool bInMarker)
{
	Id = InId;
	PromptFa = InPromptFa;
	PromptEn = InPromptEn;
	Radius = InRadius;
	bShowMarker = bInMarker;
	Marker->SetVisibility(bShowMarker && bEnabled);
}

void ACHInteractable::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	Marker->SetVisibility(bShowMarker && bEnabled);
}

void ACHInteractable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Time += DeltaSeconds;
	if (!bEnabled || !bShowMarker)
	{
		return;
	}
	if (APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		const FVector ToCamera = (Camera->GetCameraLocation() - GetActorLocation()).GetSafeNormal();
		if (!ToCamera.IsNearlyZero())
		{
			Marker->SetWorldRotation(FRotationMatrix::MakeFromZ(ToCamera).Rotator());
		}
	}
	if (MarkerMaterial)
	{
		MarkerMaterial->SetScalarParameterValue(TEXT("Strength"), CHLight::Emissive(0.9f + 0.5f * FMath::Sin(Time * 3.f)));
	}
}
