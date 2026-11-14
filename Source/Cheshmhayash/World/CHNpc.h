// Cheshm Hayash - a person in the world who is not the player: family, friends, crowds.
// The template mannequin, tinted, with simple attached shapes (headscarf, chador, helmet).
// Silhouettes over detail.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHNpc.generated.h"

class UAnimSequence;
class UMaterialInstanceDynamic;
class USkeletalMesh;
class USkeletalMeshComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class ACHNpc : public AActor
{
	GENERATED_BODY()

public:
	ACHNpc();

	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Mesh, colours and accessories by role: sister, sister_after, brother, he, kian, mother, father,
	 * woman, man, chador, oldwoman, student_f, student_m, guard, basij, plain, lawyer, driver.
	 * Seed varies the colours of crowd roles.
	 */
	void SetLook(const FString& Look, int32 Seed = 0);
	void SetColors(const FLinearColor& Upper, const FLinearColor& Lower);

	/** Walks in a straight line on flat ground. Speed in cm/s. */
	void WalkTo(const FVector& Target, float Speed = 120.f);
	bool IsMoving() const { return bMoving; }
	void Stop();
	void FaceLocation(const FVector& Target, bool bInstant = false);
	void FaceYaw(float Yaw, bool bInstant = false);

	/** Lowers the body so it reads as sitting when the legs are hidden by furniture or steps. */
	void SetSink(float Centimetres);
	/** Trembling shoulders. */
	void SetShaking(bool bInShaking) { bShaking = bInShaking; }
	/** Plays a death animation and holds the last frame (lying on the ground). */
	void PlayFall();
	/** Freezes the current pose. */
	void Freeze();

	USkeletalMeshComponent* GetBody() const { return Body; }
	const FString& GetLook() const { return Look; }

	/** Story name ("sister", "brother"...). */
	FString NpcName;

private:
	UStaticMeshComponent* AddShape(UStaticMesh* Mesh, const FVector& Center, const FVector& Size, const FLinearColor& Color, FName Bone = NAME_None, const FRotator& Rotation = FRotator::ZeroRotator);
	void ClearShapes();
	void PlayLoop(UAnimSequence* Sequence, float Rate = 1.f);
	UMaterialInstanceDynamic* MakeMaterial(const FLinearColor& Color, float Roughness = 0.85f);

	UPROPERTY(VisibleAnywhere, Category = "Npc")
	TObjectPtr<USkeletalMeshComponent> Body;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> MannyMesh;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> QuinnMesh;

	UPROPERTY()
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> WalkAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> JogAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> FallAnim;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> ConeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Shapes;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	UPROPERTY()
	TObjectPtr<UAnimSequence> CurrentAnim;

	FString Look;
	FVector MoveTarget = FVector::ZeroVector;
	float MoveSpeed = 0.f;
	float TargetYaw = 0.f;
	float Sink = 0.f;
	float ShakeTime = 0.f;
	bool bMoving = false;
	bool bTurning = false;
	bool bShaking = false;
	bool bFrozen = false;
};
