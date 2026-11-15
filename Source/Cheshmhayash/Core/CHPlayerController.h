// Cheshm Hayash - player controller. Keyboard + mouse are polled directly here and routed either
// to the story director (advance text, menus, choices) or to the pawn (move, look, film).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CHPlayerController.generated.h"

UCLASS()
class ACHPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACHPlayerController();

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	/** Fixed camera for narrative scenes. nullptr returns the view to the player's own camera. */
	void SetSceneCamera(AActor* Camera, float BlendTime);

	/** Shakes the player's view (shots nearby). Zero-mean, so the aim does not drift. */
	void Shake(float Seconds, float Strength);

	/** Multiplies mouse look speed. */
	float MouseSensitivity = 1.f;

private:
	void UpdateCursorMode(bool bWantCursor);
	int8 CursorState = -1;
	bool bHasLastCursor = false;
	FVector2D LastCursorPos = FVector2D::ZeroVector;
	TWeakObjectPtr<AActor> ActiveSceneCamera;
	FRotator SceneCameraBaseRot = FRotator::ZeroRotator;
	FRotator SceneLookOffset = FRotator::ZeroRotator;
	void UpdateShake(float DeltaTime);

	float ShakeTime = 0.f;
	float ShakeDuration = 0.f;
	float ShakeStrength = 0.f;
	FRotator ShakeOffset = FRotator::ZeroRotator;
};
