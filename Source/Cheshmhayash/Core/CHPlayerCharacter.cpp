// Cheshm Hayash - the player character.

#include "Core/CHPlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ACHPlayerCharacter::ACHPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 92.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 560.f, 0.f);
	Movement->JumpZVelocity = 520.f;
	Movement->AirControl = 0.35f;
	Movement->MaxWalkSpeed = JogSpeed;
	Movement->MinAnalogWalkSpeed = 20.f;
	Movement->BrakingDecelerationWalking = 1800.f;
	Movement->BrakingDecelerationFalling = 1200.f;
	Movement->MaxAcceleration = 1600.f;
	Movement->GravityScale = 1.4f;
	Movement->MaxStepHeight = 40.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->SocketOffset = FVector(0.f, 42.f, 62.f);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 11.f;
	CameraBoom->ProbeSize = 10.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetFieldOfView(78.f);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> QuinnFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	MannyMesh = MannyFinder.Object;
	QuinnMesh = QuinnFinder.Object;

	USkeletalMeshComponent* Body = GetMesh();
	Body->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -92.f), FRotator(0.f, -90.f, 0.f));
	if (MannyMesh)
	{
		Body->SetSkeletalMeshAsset(MannyMesh);
	}
	if (AnimFinder.Class)
	{
		Body->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Body->SetAnimInstanceClass(AnimFinder.Class);
	}
}

void ACHPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetLook(TEXT("he"));
}

void ACHPlayerCharacter::SetLook(const FString& Look)
{
	USkeletalMeshComponent* Body = GetMesh();
	if (Look == TEXT("sister"))
	{
		if (QuinnMesh)
		{
			Body->SetSkeletalMeshAsset(QuinnMesh);
		}
		ApplyTint(FLinearColor(0.11f, 0.10f, 0.12f), FLinearColor(0.045f, 0.045f, 0.055f));
	}
	else
	{
		if (MannyMesh)
		{
			Body->SetSkeletalMeshAsset(MannyMesh);
		}
		ApplyTint(FLinearColor(0.055f, 0.065f, 0.085f), FLinearColor(0.035f, 0.035f, 0.04f));
	}
}

void ACHPlayerCharacter::SetPace(bool bSprint, bool bSneak)
{
	GetCharacterMovement()->MaxWalkSpeed = bSneak ? WalkSpeed : (bSprint ? SprintSpeed : JogSpeed);
}

void ACHPlayerCharacter::SetBoom(float Length, const FVector& Offset)
{
	// Length 0 = first person (small interiors): the camera sits at eye height and the body is
	// hidden from its own view but still casts a shadow.
	const bool bFirstPerson = Length < 1.f;

	CameraBoom->TargetArmLength = bFirstPerson ? 0.f : Length;
	CameraBoom->SocketOffset = bFirstPerson ? FVector::ZeroVector : Offset;
	CameraBoom->SetRelativeLocation(bFirstPerson ? FVector(0.f, 0.f, 70.f) : FVector::ZeroVector);
	CameraBoom->bEnableCameraLag = !bFirstPerson;
	CameraBoom->bDoCollisionTest = !bFirstPerson;
	FollowCamera->SetFieldOfView(bFirstPerson ? 84.f : 78.f);

	GetMesh()->SetOwnerNoSee(bFirstPerson);
	GetMesh()->bCastHiddenShadow = bFirstPerson;
	bUseControllerRotationYaw = bFirstPerson;
	GetCharacterMovement()->bOrientRotationToMovement = !bFirstPerson;
}

void ACHPlayerCharacter::ApplyTint(const FLinearColor& Upper, const FLinearColor& Lower)
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CH_Lit.M_CH_Lit"), nullptr, LOAD_NoWarn);
	if (!Base)
	{
		return; // keep the mannequin's own materials
	}

	USkeletalMeshComponent* Body = GetMesh();
	TintMaterials.Reset();
	const int32 NumSlots = Body->GetNumMaterials();
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, this);
		Material->SetVectorParameterValue(TEXT("Color"), Slot == 0 ? Upper : Lower);
		Material->SetScalarParameterValue(TEXT("Roughness"), 0.85f);
		Material->SetScalarParameterValue(TEXT("Grime"), 0.f);
		Body->SetMaterial(Slot, Material);
		TintMaterials.Add(Material);
	}
}
