// Cheshm Hayash - a person in the world who is not the player: family, friends, crowds.
// Uses a hidden USkeletalMeshComponent for walk/jog/idle/fall animation playback and a visible
// UPoseableMeshComponent with CHLooks clothing/hair attached to bones so characters can sit on
// chairs/sofas/stairs, smoke, look at phones, cry, hug, pour tea, and gesture naturally.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHNpc.generated.h"

class UAnimSequence;
class UMaterialInstanceDynamic;
class UPoseableMeshComponent;
class USkeletalMeshComponent;
class USkinnedMeshComponent;
class UStaticMeshComponent;

UCLASS()
class ACHNpc : public AActor
{
	GENERATED_BODY()

public:
	ACHNpc();

	virtual void Tick(float DeltaSeconds) override;

	/** See CHLooks: he, sister, brother, mother, father, kian, woman, man, chador, oldwoman, student_f, guard... */
	void SetLook(const FString& InLook, int32 Seed = 0);

	/** Sets an articulated pose: sit_chair, sit_sofa, sit_stairs, sit_floor, smoke, phone, cry, hug, pour_tea, talk, or empty. */
	void SetPose(const FString& InPose);
	const FString& GetPose() const { return Pose; }

	/** Walks in a straight line on flat ground. Speed in cm/s. */
	void WalkTo(const FVector& Target, float Speed = 120.f);
	bool IsMoving() const { return bMoving; }
	void Stop();
	void FaceLocation(const FVector& Target, bool bInstant = false);
	void FaceYaw(float Yaw, bool bInstant = false);

	/** Legacy sink helper: converts to a real seated pose or vertical offset. */
	void SetSink(float Centimetres);
	/** Trembling shoulders. */
	void SetShaking(bool bInShaking) { bShaking = bInShaking; }
	/** Plays the fall animation once; with bRagdoll, physics lays the body down afterwards. */
	void PlayFall(bool bRagdoll = false);
	void StartRagdoll();
	/** Freezes the current pose. */
	void Freeze();

	USkinnedMeshComponent* GetBody() const;
	const FString& GetLook() const { return Look; }

	/** Story name ("sister", "brother"...). */
	FString NpcName;

private:
	void PlayLoop(UAnimSequence* Sequence, float Rate = 1.f);
	void ApplyProceduralPose(float DeltaSeconds);
	void UpdateProps();

	UPROPERTY(VisibleAnywhere, Category = "Npc")
	TObjectPtr<UPoseableMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "Npc")
	TObjectPtr<USkeletalMeshComponent> AnimBody;

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
	TArray<TObjectPtr<UStaticMeshComponent>> PropShapes;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	FString Look;
	FString Pose;
	FVector MoveTarget = FVector::ZeroVector;
	float MoveSpeed = 0.f;
	float TargetYaw = 0.f;
	float Sink = 0.f;
	float PoseElevation = 0.f;
	float ShakeTime = 0.f;
	float PoseTime = 0.f;
	float RagdollTimer = -1.f;
	bool bMoving = false;
	bool bTurning = false;
	bool bShaking = false;
	bool bFrozen = false;
	bool bRagdolling = false;
};
