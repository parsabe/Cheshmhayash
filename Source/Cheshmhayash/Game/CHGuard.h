// Cheshm Hayash - the people who hunt him: riot police on foot, Basij on motorcycles (fast, loud),
// plainclothes agents (look like anyone until you are close), and watchers at the vans.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CHGuard.generated.h"

class UCHSegmentSubsystem;
class UCHVisionCone;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class USpotLightComponent;
class UStaticMeshComponent;

UENUM()
enum class ECHGuardKind : uint8
{
	Foot,
	Motorcycle,
	Plainclothes,
	Watcher
};

UENUM()
enum class ECHGuardState : uint8
{
	Patrol,
	Suspicious,
	Chase,
	Search,
	Return,
	Stunned
};

UCLASS()
class ACHGuard : public ACharacter
{
	GENERATED_BODY()

public:
	ACHGuard();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Kind, look (CHLooks name) and route (points on the floor, walked in a loop). */
	void Setup(ECHGuardKind InKind, const FString& InLook, const TArray<FVector>& InRoute, int32 InSeed = 0);
	/** Watchers stand at their post and sweep their gaze between two yaws. */
	void SetScan(float YawA, float YawB, float Period = 7.f);
	void ResetGuard();
	void Stun(float Seconds);
	void HearNoise(const FVector& Where, float Amount);
	/** Straight into a chase towards a place (he drew them away, shots were fired). */
	void Alert(const FVector& Where);
	/** Guards that only wake up later in a segment (the last night). */
	void SetDormant(bool bInDormant);

	float GetSuspicion() const { return Suspicion; }
	bool IsChasing() const { return State == ECHGuardState::Chase; }
	bool SeesPlayer() const { return bSeesPlayer; }
	ECHGuardKind GetKind() const { return Kind; }
	ECHGuardState GetState() const { return State; }

	// tuning, set per kind by Setup; locations may override after Setup
	float SightRange = 1300.f;
	float SightHalfAngle = 34.f;
	float NearRange = 230.f;
	float WalkSpeed = 140.f;
	float ChaseSpeed = 440.f;
	float CatchRadius = 120.f;
	/** Seconds to fully notice a clearly visible player at middle distance. */
	float NoticeTime = 1.25f;
	float WaitAtPoint = 2.f;

private:
	void Perceive(float DeltaSeconds, UCHSegmentSubsystem* Segment);
	void Think(float DeltaSeconds, UCHSegmentSubsystem* Segment, bool bLive);
	void UpdateVisuals(float DeltaSeconds);
	void SetState(ECHGuardState NewState);
	/** Walks along a grid path towards Target. Returns true once there. */
	bool MoveTo(const FVector& Target, float Speed, float DeltaSeconds, float RepathInterval = 1.f);
	void FaceTowards(const FVector& Target, float DeltaSeconds, float DegreesPerSecond = 240.f);
	bool CanSee(const APawn* Player) const;
	void Reveal(bool bInRevealed);
	void BuildMotorcycle();
	UStaticMeshComponent* AddPart(UStaticMesh* Mesh, const FVector& Center, const FVector& Size, UMaterialInterface* Material, const FRotator& Rotation = FRotator::ZeroRotator);

	UPROPERTY(VisibleAnywhere, Category = "Guard")
	TObjectPtr<UCHVisionCone> Cone;

	UPROPERTY(VisibleAnywhere, Category = "Guard")
	TObjectPtr<USpotLightComponent> Flashlight;

	UPROPERTY(VisibleAnywhere, Category = "Guard")
	TObjectPtr<UStaticMeshComponent> AlertMark;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> AlertMaterial;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> TailLight;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Shapes;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	ECHGuardKind Kind = ECHGuardKind::Foot;
	ECHGuardState State = ECHGuardState::Patrol;
	FString Look;
	int32 Seed = 0;

	TArray<FVector> Route;
	int32 RouteIndex = 0;
	FTransform StartTransform;

	TArray<FVector> Path;
	int32 PathIndex = 0;
	FVector PathGoal = FVector::ZeroVector;
	float RepathTime = 0.f;

	FVector LastKnown = FVector::ZeroVector;
	float Suspicion = 0.f;
	float LostTime = 0.f;
	float StateTime = 0.f;
	float WaitTime = 0.f;
	float StunTime = 0.f;
	float LookAroundYaw = 0.f;
	float Time = 0.f;

	float ScanYawA = 0.f;
	float ScanYawB = 0.f;
	float ScanPeriod = 7.f;
	bool bScans = false;

	bool bSeesPlayer = false;
	bool bRevealed = true;
	bool bHasLight = true;
	bool bDormant = false;
	bool bStartDormant = false;
};
