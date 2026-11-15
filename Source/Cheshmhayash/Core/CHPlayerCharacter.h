// Cheshm Hayash - the player. "He" in chapters 1 and 2, his sister in chapter 3.
// Third person outdoors, first person in small rooms. The template mannequin, tinted dark.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CHPlayerCharacter.generated.h"

class UCameraComponent;
class UMaterialInstanceDynamic;
class USpringArmComponent;
class UStaticMeshComponent;

UCLASS()
class ACHPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACHPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** "he" or "sister" (any CHLooks look). */
	void SetLook(const FString& Look);

	/** Movement speed tiers. Filming forces a slow walk. */
	void SetPace(bool bSprint, bool bSneak);
	bool IsSneaking() const { return bSneaking; }
	bool IsSprinting() const { return bSprinting && GetVelocity().Size2D() > JogSpeed + 20.f; }

	/** Third person camera distance and shoulder offset. Length 0 = first person. */
	void SetBoom(float Length, const FVector& Offset);
	bool IsFirstPerson() const { return bFirstPerson; }

	/** The phone camera is up: narrower view. */
	void SetFilming(bool bInFilming) { bFilming = bInFilming; }

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	static constexpr float WalkSpeed = 170.f;
	static constexpr float JogSpeed = 390.f;
	static constexpr float SprintSpeed = 610.f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Shapes;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	float BaseFOV = 78.f;
	float BaseArm = 300.f;
	bool bFirstPerson = true;
	float BobPhase = 0.f;
	float BobBlend = 0.f;
	bool bFilming = false;
	bool bSneaking = false;
	bool bSprinting = false;
};
