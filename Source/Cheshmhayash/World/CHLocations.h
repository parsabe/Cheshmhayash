// Cheshm Hayash - the locations of the story. One class per map; each builds itself in code.

#pragma once

#include "CoreMinimal.h"
#include "World/CHCity.h"
#include "World/CHLevelBuilder.h"
#include "CHLocations.generated.h"

/** Black void behind the opening cards, the title and the ending cards. */
UCLASS()
class ACHLoc_Boot : public ACHLevelBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
};

/** His room (1.1, 1.9, 3.2). Variants: "morning", "night", "after". */
UCLASS()
class ACHLoc_Room : public ACHLevelBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) override;
	virtual void OnInteract(const FString& Id, ACHInteractable* What) override;

private:
	void BuildShell();
	void BuildFurniture();

	UPROPERTY()
	TObjectPtr<USceneComponent> DoorPivot;

	/** The letter folded into the grammar book; it falls out when she opens it (after). */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LetterPaper;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CalendarPage;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> CalendarSheet;

	float DoorAngle = 0.f;
	float DoorTarget = 0.f;
};

/** A long wall, wire, a gate. The women's prison outside the city (3.1); "vozara": the detention centre (1.3). */
UCLASS()
class ACHLoc_PrisonGate : public ACHCityBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) override;

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> DoorPivot;

	void BuildVozara();

	float DoorAngle = 0.f;
	float DoorTarget = 0.f;
	float DuskTime = -1.f;
	float DuskDuration = 14.f;
};

/** Interiors: the living room "dinner" (1.4) and "years" (3.3), the lawyer's "office" (1.6), the "stairs" (1.8). */
UCLASS()
class ACHLoc_Home : public ACHLevelBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) override;
	virtual void OnInteract(const FString& Id, ACHInteractable* What) override;

private:
	TSharedRef<SWidget> MakeNewsScreen() const;
	void BuildShell(const FLinearColor& WallColor, const FLinearColor& Outside, float OutsideStrength);
	void BuildLiving();
	void BuildOffice();
	void BuildStairs();

	UPROPERTY()
	TMap<FString, TObjectPtr<UStaticMeshComponent>> Items;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Deed;

	FVector DeedFrom = FVector::ZeroVector;
	FVector DeedTo = FVector::ZeroVector;
	float DeedTime = -1.f;
	float FinishDelay = 0.f;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> TvScreen;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Steam;

	TArray<FVector> SteamBase;
	TArray<float> SteamAge;
	int32 ItemsLeft = 0;
	float HomeTime = 0.f;
};

/** The cemetery at sunrise (3.4): rows of graves, many from 1401; his stone. */
UCLASS()
class ACHLoc_Cemetery : public ACHCityBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) override;
	virtual void OnInteract(const FString& Id, ACHInteractable* What) override;

private:
	void SetSun(float Elevation);
	void ScatterPetals(int32 Count, float Spread, int32 Seed);
	void ApplyWet();
	TSharedRef<SWidget> MakeTombstone() const;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Stone;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> StoneMaterial;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Phone;

	UPROPERTY()
	TArray<TObjectPtr<ACHInteractable>> Steps;

	float SunElevation = -2.f;
	float SunFrom = -2.f;
	float SunTo = -2.f;
	float SunTime = 0.f;
	float SunDuration = 1.f;
	float WetAmount = 0.f;
	bool bWetting = false;
};

/**
 * The rooftop at the city's edge, facing east toward the mountains (2.6). Pre-dawn roofs to
 * cross, the sunrise he films, the phone falling, and the last shot: the sun rising over
 * Damavand, the light crossing the roofs until it reaches his face.
 */
UCLASS()
class ACHLoc_FinalRoof : public ACHCityBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) override;
	virtual void StartSegment(const FString& Segment, const TSharedPtr<FJsonObject>& Beat) override;
	virtual void OnInteract(const FString& Id, ACHInteractable* What) override;
	virtual void OnFilmed(const FString& Id) override;

private:
	void SetSun(float Elevation, float Warmth);
	void BuildRoof(float MinX, float MinY, float SizeX, float SizeY, float Height, uint8 LowSides, int32 Seed);
	void BuildDistantCity();
	TSharedRef<SWidget> MakePhoneScreen() const;

	/** Sun elevation in degrees: below zero it is still under the horizon. */
	float SunElevation = -4.f;
	float SunFrom = -4.f;
	float SunTo = -4.f;
	float SunTime = 0.f;
	float SunDuration = 1.f;

	/** Phone falling from his hand. */
	float FallTime = -1.f;
	FTransform FallFrom;

	float DevTimer = -1.f;
	FString Segment;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> PhoneScreen;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> PhoneBody;
};

/**
 * The city at night. Variants:
 *   "test"      one street to try the stealth
 *   "nights"    the nights of Mehr (2.4) and the alley where Mom's message arrives (2.5)
 *   "lastnight" the biggest street, the crowd, the shots, the run (2.6)
 */
UCLASS()
class ACHLoc_Streets : public ACHCityBuilder
{
	GENERATED_BODY()

public:
	virtual void Build(const FString& InVariant) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStoryEvent(const FString& Event, const TSharedPtr<FJsonObject>& Beat) override;
	virtual void StartSegment(const FString& Segment, const TSharedPtr<FJsonObject>& Beat) override;
	virtual void OnInteract(const FString& Id, ACHInteractable* What) override;
	virtual void OnInterludeEnded() override;
	virtual void OnSegmentExit(const FString& Id) override;
	virtual void OnSegmentReset() override;

private:
	void BuildTest();
	void BuildNights();
	void BuildLastNight();
	void BuildDay();
	void BuildCampus();
	void BuildGarage();
	void SendStudentOut(const FString& Id, ACHInteractable* What);
	void TickDay(float DeltaSeconds);
	/** Shots, smoke, the crowd breaks, they come. */
	void StartShots();
	void SpawnCrowd(const FBox2D& Area, int32 Count, int32 Seed);

	// nights of Mehr
	UPROPERTY()
	TArray<TObjectPtr<ACHGuard>> StudentGuards;

	UPROPERTY()
	TArray<TObjectPtr<ACHNpc>> Students;

	UPROPERTY()
	TObjectPtr<USceneComponent> ShutterPivot;

	UPROPERTY()
	TArray<TObjectPtr<ACHNpc>> ShopStudents;

	FVector ShopDoor = FVector::ZeroVector;
	FVector ShopCurb = FVector::ZeroVector;
	float ShutterOpen = 0.f;
	float ShopLoop = 0.f;
	FString PendingInterlude;

	// the last night
	UPROPERTY()
	TArray<TObjectPtr<ACHNpc>> Crowd;

	UPROPERTY()
	TArray<TObjectPtr<UWidgetComponent>> Chants;

	UPROPERTY()
	TArray<TObjectPtr<ACHGuard>> Chasers;

	TArray<float> ChantAge;
	TArray<FVector> ChantBase;
	TArray<FVector> CrowdGoals;
	bool bShotsFired = false;

	// day, campus, garage
	UPROPERTY()
	TArray<TObjectPtr<ACHNpc>> GarageRunners;

	FVector GarageOut = FVector::ZeroVector;
	int32 GarageLeft = 0;
	bool bTaxi = false;
	float StreetsTime = 0.f;
};
