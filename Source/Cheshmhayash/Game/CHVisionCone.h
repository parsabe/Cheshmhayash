// Cheshm Hayash - what a guard can see, drawn on the ground as a soft fan clipped by walls.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "CHVisionCone.generated.h"

class UMaterialInstanceDynamic;

UCLASS()
class UCHVisionCone : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	UCHVisionCone(const FObjectInitializer& ObjectInitializer);

	void Configure(float InRange, float InHalfAngle);

	/** Rebuilds the fan from the component's world location along its forward (yaw only). */
	void Refresh(const FLinearColor& Color, float Opacity);

	float GetRange() const { return Range; }

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> Colors;
	TArray<FProcMeshTangent> Tangents;

	float Range = 1200.f;
	float HalfAngle = 34.f;
	int32 Segments = 28;
	bool bCreated = false;
};
