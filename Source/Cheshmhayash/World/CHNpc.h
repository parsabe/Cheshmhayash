// Cheshm Hayash - a person in the world who is not the player: family, friends, crowds.
// Moves by simple scripted walks; looks come from CHLooks.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHNpc.generated.h"

class UAnimSequence;
class UMaterialInstanceDynamic;
class USkeletalMeshComponent;
class UStaticMeshComponent;

UCLASS()
class ACHNpc : public AActor
{
	GENERATED_BODY()

public:
	ACHNpc();

	virtual void Tick(float DeltaSeconds) override;

	/** See CHLooks: sister, brother, mother, woman, man, chador, oldwoman, student_f, guard... */
	void SetLook(const FString& InLook, int32 Seed = 0);

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
	/** Plays the fall animation once and holds the last frame. */
	void PlayFall();
	/** Freezes the current pose. */
	void Freeze();

	USkeletalMeshComponent* GetBody() const { return Body; }
	const FString& GetLook() const { return Look; }

	/** Story name ("sister", "brother"...). */
	FString NpcName;

private:
	void PlayLoop(UAnimSequence* Sequence, float Rate = 1.f);

	UPROPERTY(VisibleAnywhere, Category = "Npc")
	TObjectPtr<USkeletalMeshComponent> Body;

	UPROPERTY()
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> WalkAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> JogAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> FallAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequence> CurrentAnim;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Shapes;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

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
