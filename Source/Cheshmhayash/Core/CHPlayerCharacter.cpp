// Cheshm Hayash - the player character.

#include "Core/CHPlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "World/CHLooks.h"

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
	Movement->JumpZVelocity = 600.f;
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
	FollowCamera->SetFieldOfView(BaseFOV);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyFinder(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));

	USkeletalMeshComponent* Body = GetMesh();
	Body->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -92.f), FRotator(0.f, -90.f, 0.f));
	if (MannyFinder.Object)
	{
		Body->SetSkeletalMeshAsset(MannyFinder.Object);
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
	CHLooks::Apply(this, GetMesh(), Look.IsEmpty() ? FString(TEXT("he")) : Look, 0, Shapes, Materials);
	for (UStaticMeshComponent* Shape : Shapes)
	{
		if (Shape)
		{
			Shape->SetOwnerNoSee(bFirstPerson);
			Shape->bCastHiddenShadow = bFirstPerson;
		}
	}
}

void ACHPlayerCharacter::SetPace(bool bSprint, bool bSneak)
{
	bSneaking = bSneak && !bFilming;
	bSprinting = bSprint && !bSneak && !bFilming;
	GetCharacterMovement()->MaxWalkSpeed = (bSneak || bFilming) ? WalkSpeed : (bSprint ? SprintSpeed : JogSpeed);
}

void ACHPlayerCharacter::SetBoom(float Length, const FVector& Offset)
{
	// first person in small interiors: the camera sits at eye height, the body is hidden from its
	// own view but still casts a shadow
	bFirstPerson = Length < 1.f;
	BaseArm = bFirstPerson ? 0.f : Length;

	CameraBoom->TargetArmLength = BaseArm;
	CameraBoom->SocketOffset = bFirstPerson ? FVector::ZeroVector : Offset;
	CameraBoom->SetRelativeLocation(bFirstPerson ? FVector(0.f, 0.f, 70.f) : FVector::ZeroVector);
	CameraBoom->bEnableCameraLag = !bFirstPerson;
	CameraBoom->bDoCollisionTest = !bFirstPerson;
	BaseFOV = bFirstPerson ? 84.f : 78.f;
	FollowCamera->SetFieldOfView(BaseFOV);

	GetMesh()->SetOwnerNoSee(bFirstPerson);
	GetMesh()->bCastHiddenShadow = bFirstPerson;
	for (UStaticMeshComponent* Shape : Shapes)
	{
		if (Shape)
		{
			Shape->SetOwnerNoSee(bFirstPerson);
			Shape->bCastHiddenShadow = bFirstPerson;
		}
	}
	bUseControllerRotationYaw = bFirstPerson;
	GetCharacterMovement()->bOrientRotationToMovement = !bFirstPerson;
}

void ACHPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// the phone camera: a narrower, steadier view while filming
	const float TargetFOV = bFilming ? 46.f : BaseFOV;
	const float FOV = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaSeconds, 6.f);
	if (!FMath::IsNearlyEqual(FOV, FollowCamera->FieldOfView, 0.01f))
	{
		FollowCamera->SetFieldOfView(FOV);
	}
	if (!bFirstPerson)
	{
		const float TargetArm = bFilming ? FMath::Min(BaseArm, 150.f) : BaseArm;
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArm, DeltaSeconds, 5.f);
	}
}
