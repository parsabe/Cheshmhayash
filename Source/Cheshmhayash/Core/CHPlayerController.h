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

	/** Multiplies mouse look speed. */
	float MouseSensitivity = 1.f;
};
