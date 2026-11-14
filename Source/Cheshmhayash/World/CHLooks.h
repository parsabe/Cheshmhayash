// Cheshm Hayash - how people look. The template mannequin, tinted, plus a few attached basic
// shapes (headscarf, chador, helmet). Silhouettes over detail.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UMaterialInstanceDynamic;
class USkeletalMeshComponent;
class UStaticMeshComponent;

namespace CHLooks
{
	bool IsFemale(const FString& Look);

	/**
	 * Sets the mesh (Manny / Quinn), the tints and the accessory shapes on Body. Body must be a
	 * registered mannequin component with its feet at its origin (as on characters and npcs).
	 * Shapes previously created into InOutShapes are destroyed first. Returns the uniform scale the
	 * look wants (crowds vary a little in height); the caller decides whether to apply it.
	 */
	float Apply(AActor* Owner, USkeletalMeshComponent* Body, const FString& Look, int32 Seed,
		TArray<TObjectPtr<UStaticMeshComponent>>& InOutShapes, TArray<TObjectPtr<UMaterialInstanceDynamic>>& InOutMaterials);

	/** Re-tints the body only. Upper = torso, Lower = head, arms and legs. */
	void Tint(UObject* Outer, USkeletalMeshComponent* Body, const FLinearColor& Upper, const FLinearColor& Lower,
		TArray<TObjectPtr<UMaterialInstanceDynamic>>& InOutMaterials);

	/** Lit material (M_CH_Lit) with a colour. nullptr if the master material is missing. */
	UMaterialInstanceDynamic* MakeLit(UObject* Outer, const FLinearColor& Color, float Roughness = 0.85f);
}
