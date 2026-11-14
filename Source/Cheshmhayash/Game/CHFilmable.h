// Cheshm Hayash - a moment worth filming. It glows softly until he has filmed it.
// Every filmed moment becomes a clip in the director's cut at the end of chapter two.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHFilmable.generated.h"

class UMaterialInstanceDynamic;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS()
class ACHFilmable : public AActor
{
	GENERATED_BODY()

public:
	ACHFilmable();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void Setup(const FString& InClipId, const FString& InCaptionFa, const FString& InCaptionEn, float InFilmSeconds = 3.5f, float InRange = 2800.f);

	/** 0..1 while he is filming it. */
	void SetProgress(float InProgress) { Progress = InProgress; }
	void MarkFilmed();
	bool IsFilmed() const { return bFilmed; }

	FString ClipId;
	FString CaptionFa;
	FString CaptionEn;
	float FilmSeconds = 3.5f;
	float Range = 2800.f;
	/** How close to the centre of the frame it must be, in degrees. */
	float AimAngle = 12.f;
	/** Size of the glow sprite in cm. */
	float GlowSize = 140.f;
	/** Far away (the sun): skip the line of sight test. */
	bool bIgnoreSight = false;

	void SetGlowVisible(bool bVisible);

private:
	UPROPERTY(VisibleAnywhere, Category = "Filmable")
	TObjectPtr<UStaticMeshComponent> Sprite;

	UPROPERTY(VisibleAnywhere, Category = "Filmable")
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SpriteMaterial;

	float Time = 0.f;
	float Progress = 0.f;
	float Fade = 1.f;
	bool bFilmed = false;
};
