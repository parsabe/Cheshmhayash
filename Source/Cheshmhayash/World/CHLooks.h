// Cheshm Hayash - how people look. Styled characters built on the mannequin skeleton with
// multi-part clothing silhouettes (jackets, coats, manteaus, collars, sleeves, trousers, shoes,
// hair, headscarves, chadors, backpacks) attached to bones so they move naturally with animations
// and poses. Faces stay in shadow without facial features (rule 3.3).

#pragma once

#include "CoreMinimal.h"

class AActor;
class UMaterialInstanceDynamic;
class USkinnedMeshComponent;
class UStaticMeshComponent;

namespace CHLooks
{
	bool IsFemale(const FString& Look);

	/**
	 * Sets the mesh (Manny / Quinn), the tints and the layered clothing/hair shapes on Body.
	 * Body must be a registered mannequin component with its feet at its origin.
	 * Shapes previously created into InOutShapes are destroyed first. Returns the uniform scale
	 * the look wants (crowds vary in height and build).
	 */
	float Apply(AActor* Owner, USkinnedMeshComponent* Body, const FString& Look, int32 Seed,
		TArray<TObjectPtr<UStaticMeshComponent>>& InOutShapes, TArray<TObjectPtr<UMaterialInstanceDynamic>>& InOutMaterials);

	/** Non-uniform scale multiplier (X, Y width/depth, Z height) for build variety. */
	FVector Scale3D(const FString& Look, int32 Seed);

	/** Re-tints the body only. Upper = torso, Lower = head, arms and legs. */
	void Tint(UObject* Outer, USkinnedMeshComponent* Body, const FLinearColor& Upper, const FLinearColor& Lower,
		TArray<TObjectPtr<UMaterialInstanceDynamic>>& InOutMaterials);

	/** Lit material (M_CH_Lit) with a colour. nullptr if the master material is missing. */
	UMaterialInstanceDynamic* MakeLit(UObject* Outer, const FLinearColor& Color, float Roughness = 0.85f);
}
