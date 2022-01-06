// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/GCBaseCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GCBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include <Actors/Equipment/Weapons/RangeWeaponItem.h>


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	Team = ETeams::Player;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultArmLength = SpringArmComponent->TargetArmLength;
	if (IsValid(SprintCameraTimelineCurve))
	{
		FOnTimelineFloatStatic SprintCameraMovementUpdate;
		SprintCameraMovementUpdate.BindUObject(this, &APlayerCharacter::SprintCameraMovementUpdate);
		SprintCameraTimeline.AddInterpFloat(SprintCameraTimelineCurve, SprintCameraMovementUpdate);
		SprintCameraTimeline.SetPlayRate(SprintCameraMovementRate);
	}

	if (IsValid(AimingCameraTimelineCurve))
	{
		FOnTimelineFloatStatic AimingCameraMovementUpdate;
		AimingCameraMovementUpdate.BindUObject(this, &APlayerCharacter::AimingCameraMovementUpdate);
		AimingCameraTimeline.AddInterpFloat(AimingCameraTimelineCurve, AimingCameraMovementUpdate);
		AimingCameraTimeline.SetPlayRate(AimingCameraMovementRate);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SprintCameraTimeline.TickTimeline(DeltaTime);
	AimingCameraTimeline.TickTimeline(DeltaTime);
}

void APlayerCharacter::MoveForward(float Value)
{
	if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	if (IsAiming())
	{
		ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
		AddControllerYawInput(Value * CurrentRangeWeapon->GetAimTurnModifier());
	}
	else
	{
		AddControllerYawInput(Value);
	}
}

void APlayerCharacter::LookUp(float Value)
{
	if (IsAiming())
	{
		ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
		AddControllerPitchInput(Value * CurrentRangeWeapon->GetAimLookUpModifier());
	}
	else
	{
		AddControllerPitchInput(Value);
	}
}

void APlayerCharacter::TurnAtRate(float Value)
{
	if (IsAiming())
	{
		ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
		AddControllerYawInput(Value * CurrentRangeWeapon->GetAimTurnModifier() * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
	else
	{
		AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void APlayerCharacter::LookUpAtRate(float Value)
{
	if (IsAiming())
	{
		ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
		AddControllerPitchInput(Value * CurrentRangeWeapon->GetAimLookUpModifier() * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
	else
	{
		AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void APlayerCharacter::SwimForward(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator PitchYawRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = PitchYawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::SwimRight(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::SwimUp(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value, 1e-6f))
	{
		AddMovementInput(FVector::UpVector, Value);
	}
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnStartSlide(float CapsuleHeightAdjust)
{	
	Super::OnStartSlide(CapsuleHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, CapsuleHeightAdjust);
}

void APlayerCharacter::OnEndSlide(float CapsuleHeightAdjust)
{
	Super::OnEndSlide(CapsuleHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, CapsuleHeightAdjust);
}

void APlayerCharacter::OnSprintStart_Implementation()
{
	Super::OnSprintStart_Implementation();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}
	SprintCameraTimeline.Play();
}

void APlayerCharacter::OnSprintEnd_Implementation()
{
	Super::OnSprintEnd_Implementation();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}
	SprintCameraTimeline.Reverse();
}

void APlayerCharacter::OnStartAimingInternal()
{
	Super::OnStartAimingInternal();
	if (IsValid(AimingCameraTimelineCurve))
	{
		AimingCameraTimeline.Play();
	}
	else
	{
		APlayerController* PlayerController = GetController<APlayerController>();
		if (!IsValid(PlayerController))
		{
			return;
		}
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		if (IsValid(CameraManager))
		{
			ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
			CameraManager->SetFOV(CurrentRangeWeapon->GetAimFOV());
		}
	}
}

void APlayerCharacter::OnStopAimingInternal()
{
	Super::OnStopAimingInternal();
	if (IsValid(AimingCameraTimelineCurve))
	{
		AimingCameraTimeline.Reverse();
	}
	else
	{
		APlayerController* PlayerController = GetController<APlayerController>();
		if (!IsValid(PlayerController))
		{
			return;
		}
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		if (IsValid(CameraManager))
		{
			CameraManager->UnlockFOV();
		}
	}
}

void APlayerCharacter::SprintCameraMovementUpdate(float Alpha)
{
	SpringArmComponent->TargetArmLength = FMath::Lerp(DefaultArmLength, SprintArmLength, Alpha);
}

void APlayerCharacter::AimingCameraMovementUpdate(float Alpha)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}
	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if (IsValid(CameraManager))
	{
		ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
		CameraManager->SetFOV(FMath::Lerp(CameraManager->DefaultFOV, CurrentRangeWeapon->GetAimFOV(), Alpha));
	}
}

bool APlayerCharacter::CanJumpInternal_Implementation() const
{
	return (bIsCrouched || Super::CanJumpInternal_Implementation()) && !GetBaseCharacterMovementComponent()->IsMantling();
}

void APlayerCharacter::OnJumped_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}
