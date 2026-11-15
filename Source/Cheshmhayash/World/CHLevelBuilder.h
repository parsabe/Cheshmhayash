// Cheshm Hayash - base class for every location.
// Each map is empty; the game mode spawns one ACHLevelBuilder subclass that builds the whole
// location (geometry, materials, lights, fog, post process) in code.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CHLevelBuilder.generated.h"

class ACameraActor;
class ACHFilmable;
class ACHGuard;
class ACHInteractable;
class ACHNpc;
class FJsonObject;
enum class ECHGuardKind : uint8;
class SWidget;
class UWidgetComponent;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPointLightComponent;
class UPostProcessComponent;
class USceneComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class USpotLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Emissive values are multiplied by the camera exposure, so a glow that reads well at night burns
 * white in daylight. CHLight converts "how bright it should look on screen" into an emissive value
 * for the current location's exposure.
 */
namespace CHLight
{
	void SetExposure(float ExposureEV);
	float GetExposure();
	/** Emissive value that shows as roughly 1.0 on screen at the current exposure. */
	float DisplayScale();
	inline float Emissive(float ScreenBrightness) { return ScreenBrightness * DisplayScale(); }
}

/** Lighting / atmosphere / grading of a location. All values are art-directed, not physical. */
struct FCHEnv
{
	// Sun: the directional light that drives the sky atmosphere.
	FRotator SunRotation = FRotator(-40.f, 30.f, 0.f);
	float SunIntensity = 6.f;
	FLinearColor SunColor = FLinearColor(1.f, 0.93f, 0.82f);
	bool bSunShadows = true;
	/** false: the sun only drives the sky colour and lights nothing (night). */
	bool bSunLightsWorld = true;
	float SunVolumetric = 1.f;

	// Moon / fill: a second directional light.
	bool bMoon = false;
	FRotator MoonRotation = FRotator(-55.f, -120.f, 0.f);
	float MoonIntensity = 0.05f;
	FLinearColor MoonColor = FLinearColor(0.55f, 0.68f, 1.f);

	bool bAtmosphere = true;
	float SkyLightIntensity = 1.f;
	FLinearColor SkyLightColor = FLinearColor::White;

	// Exponential height fog.
	bool bFog = true;
	float FogDensity = 0.02f;
	float FogHeightFalloff = 0.2f;
	FLinearColor FogColor = FLinearColor(0.02f, 0.025f, 0.035f);
	float FogStartDistance = 0.f;
	float FogMaxOpacity = 1.f;
	float FogZ = 0.f;
	bool bVolumetricFog = false;
	float VolumetricExtinction = 1.f;

	// Post process.
	/** Fixed exposure in EV100. Higher = darker image. */
	float ExposureEV = 0.f;
	float Grain = 0.3f;
	float Vignette = 0.45f;
	/** Chromatic aberration. */
	float Fringe = 0.3f;
	float Bloom = 0.7f;
	float Saturation = 1.f;
	float Contrast = 1.f;
	FLinearColor Gain = FLinearColor::White;
	float Temperature = 6500.f;
};

struct FCHMark
{
	FTransform Transform;
	float FOV = 70.f;
};

UCLASS()
class ACHLevelBuilder : public AActor
{
	GENERATED_BODY()

public:
	ACHLevelBuilder();

	/** Builds the location. Called once by the game mode before the player is spawned. */
	virtual void Build(const FString& Variant);

	/** A story beat of type "event" addressed to this location (door opens, tv turns on...). */
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) {}

	/**
	 * Starts a gameplay segment (a "gameplay" beat). The default begins the stealth / filming
	 * segment with the beat's hint; the segment ends when the player reaches an exit.
	 */
	virtual void StartSegment(const FString& Segment, const TSharedPtr<FJsonObject>& Beat);
	/** The player reached an exit zone. Default: the segment is over. */
	virtual void OnSegmentExit(const FString& Id);
	/** He was caught and is back at the checkpoint. */
	virtual void OnSegmentReset() {}
	/** Default: "talk.<scene>" interactables play that interlude scene. */
	virtual void OnInteract(const FString& Id, ACHInteractable* What);
	virtual void OnFilmed(const FString& Id) {}
	/** A story interlude (an encounter) has finished and gameplay resumes. */
	virtual void OnInterludeEnded();
	/** Ends the running segment and lets the story continue. */
	void FinishSegment();

	/** Named transforms: player start, camera positions, places characters stand. */
	bool GetMark(FName Name, FCHMark& Out) const;
	FTransform GetPlayerStart() const;

	/** Returns (spawning on demand) a camera actor standing at a mark. */
	ACameraActor* GetCamera(FName MarkName);

	// ---- people
	/** Spawns a person standing at a floor position. */
	ACHNpc* SpawnNpc(const FString& Name, const FString& Look, const FVector& FeetLocation, float Yaw, int32 Seed = 0);
	ACHNpc* FindNpc(const FString& Name) const;
	void RemoveNpc(const FString& Name);
	/** Generic "npc" story beat (spawn / walk / face / sink / shake / remove). Returns the npc to wait for, if any. */
	virtual ACHNpc* HandleNpcBeat(const FJsonObject& Beat);

	// ---- gameplay pieces (they register themselves with the segment)
	/** Route points are on the floor; the guard starts at the first one. */
	ACHGuard* SpawnGuard(ECHGuardKind Kind, const FString& Look, const TArray<FVector>& Route, float Yaw = 0.f, int32 Seed = 0);
	ACHFilmable* SpawnFilmable(const FString& Id, const FString& Fa, const FString& En, const FVector& Location, float Seconds = 3.5f, float Range = 2800.f);
	ACHInteractable* SpawnInteractable(const FString& Id, const FString& Fa, const FString& En, const FVector& Location, float Radius = 170.f, bool bMarker = true);

	/**
	 * Someone to talk to: using it plays the interlude scene "<Scene>". When every talk in the
	 * location has been had, the segment ends. bChained: only usable after the one before it.
	 */
	ACHInteractable* SpawnTalk(const FString& Scene, const FString& Fa, const FString& En, const FVector& Location, float Radius = 190.f, bool bChained = false);

	const FString& GetVariant() const { return Variant; }

	/** Trailer mode only: painted text on a wall (Latin). */
	void TrailerWallText(const FString& Text, const FVector& Location, float Yaw, float FontSize, const FLinearColor& Color)
	{
		WorldText(Text, false, FontSize, Color, Location, FRotator(0.f, Yaw, 0.f), 1.f, false, true);
	}

	/** Third person camera distance and offset for this location (interiors keep it close). */
	float PlayerBoomLength = 300.f;
	FVector PlayerBoomOffset = FVector(0.f, 42.f, 62.f);

	// Live handles for animated lighting (sunrise, flicker).
	UDirectionalLightComponent* GetSun() const { return Sun; }
	UDirectionalLightComponent* GetMoon() const { return Moon; }
	USkyLightComponent* GetSkyLight() const { return SkyLight; }
	UExponentialHeightFogComponent* GetFog() const { return Fog; }
	UPostProcessComponent* GetPostProcess() const { return PostProcess; }

protected:
	// ---- materials -------------------------------------------------------------------------
	/** Opaque lit surface. */
	UMaterialInstanceDynamic* Lit(const FLinearColor& Color, float Roughness = 0.9f, float Grime = 0.3f, float Metallic = 0.f);
	/** Lit surface that also glows (lamp shades, windows at night, screens). */
	UMaterialInstanceDynamic* Emissive(const FLinearColor& Color, float Strength, const FLinearColor& BodyColor = FLinearColor(0.02f, 0.02f, 0.02f));
	/** Opaque unlit colour (sky cards, silhouettes). */
	UMaterialInstanceDynamic* Unlit(const FLinearColor& Color, float Strength = 1.f);
	/** Translucent unlit (smoke, light cones, glass). Soft = radial falloff for sprites. */
	UMaterialInstanceDynamic* Glow(const FLinearColor& Color, float Strength, float Opacity, float Soft = 0.f);

	// ---- shapes (centre position, full size in cm) ------------------------------------------
	UStaticMeshComponent* Box(const FVector& Center, const FVector& Size, UMaterialInterface* Material, bool bCollision = true, const FRotator& Rotation = FRotator::ZeroRotator, bool bMovable = false);
	/** Box given by its min corner X/Y, its base Z and its size. Handy for architecture. */
	UStaticMeshComponent* BoxAt(float MinX, float MinY, float BaseZ, float SizeX, float SizeY, float SizeZ, UMaterialInterface* Material, bool bCollision = true);
	UStaticMeshComponent* Cyl(const FVector& Center, float Radius, float Height, UMaterialInterface* Material, bool bCollision = false, const FRotator& Rotation = FRotator::ZeroRotator, bool bMovable = false);
	UStaticMeshComponent* Ball(const FVector& Center, const FVector& Size, UMaterialInterface* Material, bool bCollision = false, const FRotator& Rotation = FRotator::ZeroRotator, bool bMovable = false);
	UStaticMeshComponent* Cone(const FVector& Center, float Radius, float Height, UMaterialInterface* Material, bool bCollision = false, const FRotator& Rotation = FRotator::ZeroRotator, bool bMovable = false);
	/** Flat quad. With zero rotation it lies in XY facing up. */
	UStaticMeshComponent* Quad(const FVector& Center, const FVector2D& Size, const FRotator& Rotation, UMaterialInterface* Material, bool bMovable = false);
	UStaticMeshComponent* AddMesh(UStaticMesh* Mesh, const FTransform& Transform, UMaterialInterface* Material, bool bCollision, bool bMovable = false, USceneComponent* Parent = nullptr);

	/** Decorative instanced box (no collision). Use for large counts: windows, bricks, graves. */
	void BoxInstance(const FVector& Center, const FVector& Size, UMaterialInterface* Material, const FRotator& Rotation = FRotator::ZeroRotator);

	/** Empty movable pivot to hang animated parts on (doors, shutters). */
	USceneComponent* Pivot(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
	/** Movable box attached to a pivot; the centre is relative to the pivot. */
	UStaticMeshComponent* BoxOn(USceneComponent* Parent, const FVector& RelativeCenter, const FVector& Size, UMaterialInterface* Material, bool bCollision = false, const FRotator& RelativeRotation = FRotator::ZeroRotator);

	// ---- lights -----------------------------------------------------------------------------
	UPointLightComponent* PointLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, bool bShadows = false, float Volumetric = 1.f);
	USpotLightComponent* SpotLight(const FVector& Location, const FRotator& Rotation, const FLinearColor& Color, float Intensity, float Radius, float OuterCone, bool bShadows = false, float Volumetric = 1.f);

	// ---- environment ------------------------------------------------------------------------
	/** Creates (first call) or updates the sun, sky, fog and post process. */
	void ApplyEnv(const FCHEnv& Env);

	/**
	 * The Alborz range on the horizon with Damavand at its centre, in compass direction YawDegrees.
	 * Unlit silhouettes: the colours are the look.
	 */
	void BuildAlborz(float YawDegrees, const FLinearColor& RockColor, const FLinearColor& SnowColor, float Distance = 300000.f, bool bLit = false);

	// ---- world-space Slate (Farsi shapes correctly here; engine text render components do not)
	/**
	 * Draws a Slate widget on a quad in the world. The quad faces along +X of Rotation.
	 * bGlow: unlit and emissive (screens, lit signs). Otherwise it is lit like paint on a surface.
	 * DrawSize in pixels; zero = the widget's desired size.
	 */
	UWidgetComponent* WorldWidget(const TSharedRef<SWidget>& Widget, const FVector& Location, const FRotator& Rotation,
		float CmPerPixel, bool bGlow = false, const FVector2D& DrawSize = FVector2D::ZeroVector);
	UWidgetComponent* WorldText(const FString& Text, bool bFarsi, float FontSize, const FLinearColor& Color, const FVector& Location,
		const FRotator& Rotation, float CmPerPixel, bool bGlow = false, bool bBold = false);

	// ---- marks ------------------------------------------------------------------------------
	void Mark(FName Name, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float FOV = 70.f);
	/** Camera mark that looks from Location at Target. */
	void MarkLookAt(FName Name, const FVector& Location, const FVector& Target, float FOV = 70.f);

	FString Variant;

	UPROPERTY()
	TArray<TObjectPtr<ACHInteractable>> Talks;

	int32 TalksLeft = 0;
	bool bFinishAfterTalks = true;

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> Sun;

	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> Moon;

	UPROPERTY()
	TObjectPtr<USkyAtmosphereComponent> Atmosphere;

	UPROPERTY()
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY()
	TObjectPtr<UExponentialHeightFogComponent> Fog;

	UPROPERTY()
	TObjectPtr<UPostProcessComponent> PostProcess;

private:
	UMaterialInstanceDynamic* MakeMaterial(const FString& Key, UMaterialInterface* Base);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> ConeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> PlaneMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> LitBase;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> UnlitBase;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> GlowBase;

	UPROPERTY()
	TMap<FString, TObjectPtr<UMaterialInstanceDynamic>> MaterialCache;

	UPROPERTY()
	TMap<TObjectPtr<UMaterialInterface>, TObjectPtr<UInstancedStaticMeshComponent>> BoxInstancers;

	UPROPERTY()
	TMap<FName, TObjectPtr<ACameraActor>> Cameras;

	UPROPERTY()
	TMap<FString, TObjectPtr<ACHNpc>> Npcs;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> WidgetLitBase;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> WidgetGlowBase;

	TMap<FName, FCHMark> Marks;

	bool bLitBaseIsFallback = false;
};
