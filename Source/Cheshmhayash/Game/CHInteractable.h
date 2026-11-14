// Cheshm Hayash - something he (or she) can use: pick up the ID card, open the book, pour the
// rosewater. Shows a small prompt when close; the location reacts in OnInteract.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHInteractable.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS()
class ACHInteractable : public AActor
{
	GENERATED_BODY()

public:
	ACHInteractable();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void Setup(const FString& InId, const FString& InPromptFa, const FString& InPromptEn, float InRadius = 170.f, bool bInMarker = true);
	void SetEnabled(bool bInEnabled);
	bool IsEnabled() const { return bEnabled; }

	FString Id;
	FString PromptFa;
	FString PromptEn;
	float Radius = 170.f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Interactable")
	TObjectPtr<UStaticMeshComponent> Marker;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MarkerMaterial;

	float Time = 0.f;
	bool bEnabled = true;
	bool bShowMarker = true;
};
