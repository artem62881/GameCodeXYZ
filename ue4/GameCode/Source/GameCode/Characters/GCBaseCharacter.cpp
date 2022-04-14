// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Components/GCBaseCharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/LedgeDetectorComponent.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "GameCodeTypes.h"
#include "Utils/GCTraceUtils.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include <GameFramework/PhysicsVolume.h>
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include <Actors/Equipment/Weapons/RangeWeaponItem.h>

#include "AIController.h"


AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GCBaseCharacterMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());
	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("LedgeDetector"));
	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributesComponent>(TEXT("CharacterAttributes"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));

	DefaultCapsuleHalfHeight = GetDefaultHalfHeight();
	DefaultCapsuleRadius = GetDefaultCapsuleRadius();

	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AGCBaseCharacter::OnPlayerCapsuleHit);
	CharacterAttributesComponent->OnDeathEvent.AddUObject(this, &AGCBaseCharacter::OnDeath);
	CharacterAttributesComponent->OutOfStaminaEvent.AddUObject(GetBaseCharacterMovementComponent(), &UGCBaseCharacterMovementComponent::SetIsOutOfStamina);
}

void AGCBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
}

void AGCBaseCharacter::ChangeCrouchState()
{
	
	if (!GetBaseCharacterMovementComponent()->IsSprinting())
	{
		if (GetCharacterMovement()->IsCrouching())
		{
			UnCrouch();
		}
		else
		{
			Crouch();
		}
	}
}

void AGCBaseCharacter::StartSlide()
{
	if (GetBaseCharacterMovementComponent()->IsSprinting() && !GetBaseCharacterMovementComponent()->IsSwimming())
	{
		CurrentSlideSettings.SlideDirection = GetActorForwardVector();
		CurrentSlideSettings.SlideRotation = GetActorRotation();
		CurrentSlideSettings.CapsuleHeightAdjustment = DefaultCapsuleHalfHeight - SlideCaspsuleHalfHeight;
		
		StopSprint();

		FHitResult* MoveCapsuleHit = nullptr;
		FRotator MeshRotation = CurrentSlideSettings.SlideRotation + FRotator(0.f, -90.f, 0.f);

		GetCapsuleComponent()->MoveComponent(CurrentSlideSettings.CapsuleHeightAdjustment * FVector::DownVector, CurrentSlideSettings.SlideRotation, false, MoveCapsuleHit);
		GetMesh()->MoveComponent(CurrentSlideSettings.CapsuleHeightAdjustment * FVector::UpVector, MeshRotation, false);
		OnStartSlide(CurrentSlideSettings.CapsuleHeightAdjustment);

		GetCapsuleComponent()->SetCapsuleSize(DefaultCapsuleRadius, SlideCaspsuleHalfHeight);
		GetBaseCharacterMovementComponent()->StartSlide(CurrentSlideSettings);
	}
}

void AGCBaseCharacter::StopSlide()
{
	if (GetBaseCharacterMovementComponent()->IsSliding())
	{
		GetBaseCharacterMovementComponent()->StopSlide(CurrentSlideSettings);

		FHitResult* MoveCapsuleHit = nullptr;
		FRotator MeshRotation = CurrentSlideSettings.SlideRotation + FRotator(0.f, -90.f, 0.f);
		
		FVector OverlapLocation = GetActorLocation() + CurrentSlideSettings.CapsuleHeightAdjustment * FVector::UpVector;
		FCollisionQueryParams QueryParams;
		if (GCTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, DefaultCapsuleRadius, DefaultCapsuleHalfHeight, FQuat::Identity, CollisionProfileIgnorePawn, QueryParams, true, 4.f))
		{
			Crouch();
		}
		else
		{
			GetCapsuleComponent()->MoveComponent(CurrentSlideSettings.CapsuleHeightAdjustment * FVector::UpVector, CurrentSlideSettings.SlideRotation, false, MoveCapsuleHit);
			GetCapsuleComponent()->SetCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);
			GetMesh()->MoveComponent(CurrentSlideSettings.CapsuleHeightAdjustment * FVector::DownVector, MeshRotation, false);
		}
		OnEndSlide(CurrentSlideSettings.CapsuleHeightAdjustment);

		CurrentSlideSettings.SlideDirection = FVector::ZeroVector;
		CurrentSlideSettings.SlideRotation = FRotator::ZeroRotator;
		CurrentSlideSettings.CapsuleHeightAdjustment = 0.f;
	}
}

void AGCBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AGCBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TryChangeSprintState();
	UpdateIKSettings(DeltaTime);
	GetBaseCharacterMovementComponent()->UpdateSlide(DeltaTime, CurrentSlideSettings);
}

void AGCBaseCharacter::StartFire()
{
	if (CharacterEquipmentComponent->IsEquipping())
	{
		return;
	}
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon) && CanFire())
	{
		CurrentRangeWeapon->StartFire();
	}
}

void AGCBaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
	}
}

void AGCBaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon) || !CanFire())
	{
		return;
	}
	if (GetBaseCharacterMovementComponent()->IsSprinting())
	{
		StopSprint();
	}
	bIsAiming = true;
	CurrentAimingMovementSpeed = CurrentRangeWeapon->GetAimMovementMaxSpeed();
	CurrentRangeWeapon->StartAim();
	OnStartAiming();
}

void AGCBaseCharacter::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}

	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopAim();
	}
	bIsAiming = false;
	CurrentAimingMovementSpeed = 0.f;
	OnStopAiming();
}

void AGCBaseCharacter::Reload()
{
	if (IsValid(CharacterEquipmentComponent->GetCurrentRangeWeapon()))
	{
		CharacterEquipmentComponent->ReloadCurrentWeapon();
	}
}

void AGCBaseCharacter::EquipNextItem()
{
	CharacterEquipmentComponent->EquipNextItem();
}

void AGCBaseCharacter::EquipPreviousItem()
{
	CharacterEquipmentComponent->EquipPreviousItem();
}

void AGCBaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();
}

void AGCBaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

bool AGCBaseCharacter::IsAiming() const
{
	return bIsAiming;
}

float AGCBaseCharacter::GetAimingMovementSpeed() const
{
	return CurrentAimingMovementSpeed;
}

void AGCBaseCharacter::Mantle(bool bForce /*= false*/)
{
	if (!(CanMantle() || bForce))
	{
		return;
	}

	if (GetBaseCharacterMovementComponent()->IsCrouching())
	{
		UnCrouch();
	}

	FLedgeDescription LedgeDescription;
	if (LedgeDetectorComponent->DetectLedge(LedgeDescription))
	{
		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.InitialLocation = GetActorLocation();
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.Location;
		MantlingParameters.TargetRotation = LedgeDescription.Rotation;
		MantlingParameters.TargetLedgePrimitiveComponent = LedgeDescription.LedgePrimitiveComponent;

		float MantlingHeight = (LedgeDescription.LedgeImpactLocation - MantlingParameters.InitialLocation).Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		const FMantlingSettings& MantlingSettings = GetMantlingSettings(MantlingHeight);

		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);

		MantlingParameters.Duration = MaxRange - MinRange;

		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;
		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);

		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation + MantlingParameters.TargetLedgePrimitiveComponent->GetComponentLocation() - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;

		GetBaseCharacterMovementComponent()->StartMantle(MantlingParameters);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);

		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

void AGCBaseCharacter::WallRun()
{
	if (GetBaseCharacterMovementComponent()->IsWallRunning())
	{
		GetBaseCharacterMovementComponent()->StopWallRun(EStopWallRunMethod::JumpOff);
	}
	else if (GetBaseCharacterMovementComponent()->IsWalking())
	{
		SetIsWallRunRequested(true);
	}
}

void AGCBaseCharacter::HardLanding()
{
	FTimerHandle HardLandingTimer;
	GetWorld()->GetTimerManager().SetTimer(HardLandingTimer, this, &AGCBaseCharacter::EndHardLanding, 2.f, false);
	GetBaseCharacterMovementComponent()->SetMovementMode(MOVE_None);
	DisableMeshRotation();
	PlayAnimMontage(HardLandingMontage);
}

void AGCBaseCharacter::EndHardLanding()
{
	GetBaseCharacterMovementComponent()->SetMovementMode(MOVE_Walking);
	EnableMeshRotation();
}

void AGCBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.AddUnique(InteractiveActor);
}

void AGCBaseCharacter::UnregisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.RemoveSingleSwap(InteractiveActor);
}

void AGCBaseCharacter::ClimbLadderUp(float Value)
{
	if (GetBaseCharacterMovementComponent()->IsOnLadder() &&!FMath::IsNearlyZero(Value))
	{
		FVector LadderUpVector = GetBaseCharacterMovementComponent()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void AGCBaseCharacter::InteractWithLadder()
{
	if (GetBaseCharacterMovementComponent()->IsSliding())
	{
		return;
	}
	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		GetBaseCharacterMovementComponent()->DettachFromLadder(EDettachFromLadderMethod::JumpOff);
	}
	else
	{
		const ALadder* AvailableLadder = GetAvailableLadder();
		if (IsValid(AvailableLadder))
		{
			if (AvailableLadder->GetIsOnTop())
			{
				PlayAnimMontage(AvailableLadder->GetAttachFromTopAnimMontage());
			}
			GetBaseCharacterMovementComponent()->AttachToLadder(AvailableLadder);
		}
	}
}

const class ALadder* AGCBaseCharacter::GetAvailableLadder()
{
	const ALadder* Result = nullptr;
	
	for (const AInteractiveActor* InteractiveActor : AvailableInteractiveActors)
	{
		if (InteractiveActor->IsA<ALadder>())
		{
			Result = StaticCast<const ALadder*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

void AGCBaseCharacter::InteractWithZipline()
{
	if (GetBaseCharacterMovementComponent()->IsSwimming() || GetBaseCharacterMovementComponent()->IsSliding() || GetBaseCharacterMovementComponent()->IsMantling())
	{
		return;
	}

	if (GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		GetBaseCharacterMovementComponent()->DettachFromZipline();
	}
	else
	{	
		const AZipline* AvailableZipline = GetAvailableZipline();
		if (IsValid(AvailableZipline))
		{
			GetBaseCharacterMovementComponent()->AttachToZipline(AvailableZipline);
		}
	}
}

const class AZipline* AGCBaseCharacter::GetAvailableZipline()
{
	const AZipline* Result = nullptr;
	for (const AInteractiveActor* InteractiveActor : AvailableInteractiveActors)
	{
		if (InteractiveActor->IsA<AZipline>())
		{
			Result = StaticCast<const AZipline*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

bool AGCBaseCharacter::IsWallRunRequested() const
{
	return bIsWallRunRequested;
}

void AGCBaseCharacter::SetIsWallRunRequested(bool bIsRequested)
{
	if (CanWallRun())
	{
		bIsWallRunRequested = bIsRequested;
	}
}

void AGCBaseCharacter::Falling()
{
	GetBaseCharacterMovementComponent()->bNotifyApex = true;
	Super::Falling();	
}

void AGCBaseCharacter::NotifyJumpApex()
{	
	Super::NotifyJumpApex();
	CurrentFallApex = GetActorLocation() - FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void AGCBaseCharacter::Landed(const FHitResult& Hit)
{	
	Super::Landed(Hit);

	float FallHeight = (CurrentFallApex - Hit.Location).Z;
	if (IsValid(FallDamageCurve))
	{
		float DamageAmount = FallDamageCurve->GetFloatValue(FallHeight);
		TakeDamage(DamageAmount, FDamageEvent(), GetController(), Hit.Actor.Get());
	}

	if (FallHeight > MinHardLandingHeight && CharacterAttributesComponent->IsAlive())
	{
		HardLanding();
	}
	CurrentFallApex = FVector::ZeroVector;
}

float AGCBaseCharacter::GetDefaultCapsuleRadius() const
{
	float CapsuleRadius;
	UCapsuleComponent* DefaultCapsule = GetClass()->GetDefaultObject<AGCBaseCharacter>()->GetCapsuleComponent();
	CapsuleRadius = DefaultCapsule->GetScaledCapsuleRadius();

	return CapsuleRadius;
}

bool AGCBaseCharacter::IsSwimmingUnderwater() const
{
	bool Result = false;

	if (GetCharacterMovement()->IsSwimming())
	{	
		FVector HeadPosition = GetMesh()->GetSocketLocation(FName("head"));
		APhysicsVolume* Volume = GetCharacterMovement()->GetPhysicsVolume();
		float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;

		if (HeadPosition.Z < VolumeTopPlane)
		{
			Result = true;
		}
	}
	return Result;
}

UCharacterEquipmentComponent* AGCBaseCharacter::GetCharacterEquipmentComponent_Mutable() const
{
	return CharacterEquipmentComponent;
}

const UCharacterEquipmentComponent* AGCBaseCharacter::GetCharacterEquipmentComponent() const
{
	return CharacterEquipmentComponent;
}

UCharacterAttributesComponent* AGCBaseCharacter::GetCharacterAttributesComponent() const
{
	return CharacterAttributesComponent;
}

FGenericTeamId AGCBaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && !GetBaseCharacterMovementComponent()->IsMantling() && !GetBaseCharacterMovementComponent()->IsOnZipline();
}

void AGCBaseCharacter::OnSprintStart_Implementation()
{
	if (IsAiming())
	{
		StopAiming();
	}
	UE_LOG(LogTemp, Log, TEXT("void AGCBaseCharacter::OnSprintStart_Implementation()"));
}

void AGCBaseCharacter::OnSprintEnd_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("void AGCBaseCharacter::OnSprintEnd_Implementation()"));
}

void AGCBaseCharacter::OnStartAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AGCBaseCharacter::OnStopAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}

bool AGCBaseCharacter::CanSprint()
{
	if (!GetBaseCharacterMovementComponent()->IsOutOfStamina() && !GetBaseCharacterMovementComponent()->IsSliding() && !GetBaseCharacterMovementComponent()->IsMantling() && !GetBaseCharacterMovementComponent()->IsWallRunning() && !GetBaseCharacterMovementComponent()->IsFalling() && !GetBaseCharacterMovementComponent()->IsCrouching())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool AGCBaseCharacter::CanSlide()
{
	//return GetBaseCharacterMovementComponent()->IsSprinting() && !GetBaseCharacterMovementComponent()->IsSwimming() && !GetBaseCharacterMovementComponent()->IsWallRunning();
	return GetBaseCharacterMovementComponent()->IsWalking() && GetBaseCharacterMovementComponent()->IsSprinting();
}

bool AGCBaseCharacter::CanMantle() const
{
	return !GetBaseCharacterMovementComponent()->IsOnLadder() && !GetBaseCharacterMovementComponent()->IsMantling() && !GetBaseCharacterMovementComponent()->IsOnZipline() && !GetBaseCharacterMovementComponent()->IsOutOfStamina() && !GetBaseCharacterMovementComponent()->IsSliding() && !GetBaseCharacterMovementComponent()->IsFalling();
}

bool AGCBaseCharacter::CanWallRun() const
{
	return !GetBaseCharacterMovementComponent()->IsOnLadder() && !GetBaseCharacterMovementComponent()->IsMantling()
	&& !GetBaseCharacterMovementComponent()->IsOnZipline() && !GetBaseCharacterMovementComponent()->IsSwimming()
	&& !GetBaseCharacterMovementComponent()->IsOnZipline() && !GetBaseCharacterMovementComponent()->IsOutOfStamina();
}

bool AGCBaseCharacter::CanFire() const
{
	UAnimInstance * AnimInstance = (GetMesh())? GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage* AnimMontage = (AnimInstance) ? AnimInstance->GetCurrentActiveMontage() : nullptr;
	bool bAnimMontage = false;
	if (AnimInstance && AnimMontage)
	{
		if (AnimInstance->Montage_IsPlaying(AnimMontage))
		{
			bAnimMontage = true;
		}
	}
	
	return GetCharacterAttributesComponent()->IsAlive() && !GetBaseCharacterMovementComponent()->IsOnLadder() && !GetBaseCharacterMovementComponent()->IsMantling()
	&& !GetBaseCharacterMovementComponent()->IsOnZipline() && !GetBaseCharacterMovementComponent()->IsSwimming() 
	&& !GetBaseCharacterMovementComponent()->IsFalling() && !GetBaseCharacterMovementComponent()->IsOutOfStamina()
	&& !GetBaseCharacterMovementComponent()->IsSliding() && !bAnimMontage;
}

void AGCBaseCharacter::OnDeath()
{
	GetCharacterMovement()->DisableMovement();
	DisableMeshRotation();
	if (GetCharacterMovement()->IsInWater())
	{
		EnableRagdoll();
	}
	else
	{
		float Duration = PlayAnimMontage(OnDeathAnimMontage);
		if (Duration == 0.f)
		{
			EnableRagdoll();
		}
	}
}

void AGCBaseCharacter::TryChangeSprintState()
{
	if (bIsSprintRequested && !GCBaseCharacterMovementComponent->IsSprinting() && CanSprint())
	{
		GCBaseCharacterMovementComponent->StartSprint();
		OnSprintStart();
	}

	if (!bIsSprintRequested && GCBaseCharacterMovementComponent->IsSprinting())
	{
		GCBaseCharacterMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

void AGCBaseCharacter::OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(IsWallRunRequested())
	{
		FVector HitNormal = Hit.ImpactNormal;
		GetBaseCharacterMovementComponent()->StartWallRun(HitNormal);
		SetIsWallRunRequested(false);
	}
}

void AGCBaseCharacter::UpdateIKSettings(float DeltaSeconds)
{
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForASocket(RightFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForASocket(LeftFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKPelvisOffset = FMath::FInterpTo(IKPelvisOffset, CalculateIKPelvisOffset(), DeltaSeconds, IKInterpSpeed);
}

float AGCBaseCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.0f;

	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		
	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - (CapsuleHalfHeight + IKTraceDistance) * FVector::UpVector;

	FHitResult HitResult;
	TArray<AActor*> IgnoredActors;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	FVector FootSizeBox = FVector(1.f, 10.f, 4.f);
	if (UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, FootSizeBox, GetMesh()->GetSocketRotation(SocketName), TraceType, true, IgnoredActors, EDrawDebugTrace::None, HitResult, true))
	{
		Result = TraceStart.Z - CapsuleHalfHeight - HitResult.Location.Z;
	}
	return Result;
}

float AGCBaseCharacter::CalculateIKPelvisOffset()
{
	return -FMath::Max(IKRightFootOffset, IKLeftFootOffset);
}

const FMantlingSettings& AGCBaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

void AGCBaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}