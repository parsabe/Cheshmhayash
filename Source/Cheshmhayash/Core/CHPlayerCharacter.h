// Cheshm Hayash - the player. "He" in chapters 1 and 2, his sister in chapter 3.
// Third person; the template mannequin, tinted dark. Silhouette over detail.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CHPlayerCharacter.generated.h"

class UCameraComponent;
class UMaterialInstanceDynamic;
class USkeletalMesh;
class USpringArmComponent;

UCLASS()
class ACHPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACHPlayerCharacter();

	virtual void BeginPlay() override;

	/** "he" or "sister". */
	void SetLook(const FString& Look);

	/** Movement speed tiers. */
	void SetPace(bool bSprint, bool bSneak);

	/** Third person camera distance and shoulder offset. */
	void SetBoom(float Length, const FVector& Offset);

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	static constexpr float WalkSpeed = 170.f;
	static constexpr float JogSpeed = 390.f;
	static constexpr float SprintSpeed = 610.f;

private:
	void ApplyTint(const FLinearColor& Upper, const FLinearColor& Lower);

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> MannyMesh;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> QuinnMesh;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> TintMaterials;
};
