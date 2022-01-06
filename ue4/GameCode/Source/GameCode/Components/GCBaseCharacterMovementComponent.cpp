// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "../GameCodeTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseCharacterMovement, Display, Display)

void UGCBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
		return;
	}

	if (IsOnLadder())
	{
		return;
	}
	Super::PhysicsRotation(DeltaTime);

}

void UGCBaseCharacterMovementComponent::SetIsOutOfStamina(bool bIsOutOfStamina_In)
{
	bIsOutOfStamina = bIsOutOfStamina_In;
	if(bIsOutOfStamina)
	{
		StopSprint();
	}	
	SetJumpAllowed(!bIsOutOfStamina_In);
}

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	if (MovementMode == EMovementMode::MOVE_Falling)
	{
		return Result;
	}
	else if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		if (bIsSprinting)
		{
			Result = ClimbingOnLadderMaxSpeed;
		}
		else
		{
			Result = ClimbingOnLadderRegularSpeed;
		}
	}
	else if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRun)
	{
		if (bIsSprinting)
		{
			Result = MaxWallRunSpeed;
		}
	}
	else if (GetBaseCharacterOwner()->IsAiming())
	{
		Result = GetBaseCharacterOwner()->GetAimingMovementSpeed();
	}
	else
	{
		if (bIsOutOfStamina)
		{
			Result = OutOfStaminaSpeed;
		}
		else if (bIsSprinting)
		{
			Result = SprintSpeed;
		}
	}
	return Result;
}

void UGCBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

void UGCBaseCharacterMovementComponent::StartSlide(FSlideSettings SlideSettings)
{
	bIsSliding = true;
	SlideSlowDownTimeline.PlayFromStart();
	GetWorld()->GetTimerManager().SetTimer(SlidingTimer, GetBaseCharacterOwner(), &AGCBaseCharacter::StopSlide, SlideMaxTime, false);
	SetMovementMode(EMovementMode::MOVE_None);
}

void UGCBaseCharacterMovementComponent::UpdateSlide(float DeltaTime, FSlideSettings SlideSettings)
{
	if (IsSliding())
	{
		FHitResult LineTraceHit;
		FVector LineTraceStart = GetBaseCharacterOwner()->GetActorLocation() - SlideSettings.SlideDirection * SlideOverLedgeOffset;
		FVector LineTraceEnd = LineTraceStart + (GetBaseCharacterOwner()->GetDefaultHalfHeight() + SlideDownLineTraceLength) * FVector::DownVector;
		FCollisionQueryParams QueryParams;
		if (!GetWorld()->LineTraceSingleByChannel(LineTraceHit, LineTraceStart, LineTraceEnd, ECC_Visibility, QueryParams))
		{
			GetBaseCharacterOwner()->StopSlide();
			Launch(CurrentSlideSpeed * SlideSettings.SlideDirection);
		}

		SlideSlowDownTimeline.TickTimeline(DeltaTime);
		FVector Delta = SlideSettings.SlideDirection * CurrentSlideSpeed * DeltaTime;
		FHitResult MoveHit;
		
		SafeMoveUpdatedComponent(Delta, SlideSettings.SlideRotation, true, MoveHit);
	}
}

void UGCBaseCharacterMovementComponent::UpdateSlideSpeed(float Alpha)
{
	float SlideTargetSpeed = FMath::Lerp(GetMaxSpeed(), SlideMaxSpeed, Alpha);
	CurrentSlideSpeed = SlideTargetSpeed;
}

void UGCBaseCharacterMovementComponent::StopSlide(FSlideSettings SlideSettings)
{
	bIsSliding = false;
	SetMovementMode(EMovementMode::MOVE_Walking);
	SlideSlowDownTimeline.Stop();
	GetWorld()->GetTimerManager().ClearTimer(SlidingTimer);
	CurrentSlideSpeed = SlideMaxSpeed;
}

void UGCBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
}

void UGCBaseCharacterMovementComponent::EndMantle()
{
	SetMovementMode(EMovementMode::MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling;
}

void UGCBaseCharacterMovementComponent::AttachToLadder(const ALadder* Ladder)
{
	CurrentLadder = Ladder;
	FRotator TargetOrientationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.0f;

	
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardVector = CurrentLadder->GetActorForwardVector();
	float Projection = GetActorToCurrentLadderProjection(GetActorLocation());

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + Projection * LadderUpVector + LadderToCharacterOffset * LadderForwardVector;
	if (CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}


	GetOwner()->SetActorRotation(TargetOrientationRotation);
	GetOwner()->SetActorLocation(NewCharacterLocation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

float UGCBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentLadder), TEXT("float UGCBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection() can't be invoked, when current ladder is null"));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderCharacterDistance = Location - CurrentLadder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderCharacterDistance);
}

void UGCBaseCharacterMovementComponent::DettachFromLadder(EDettachFromLadderMethod DettachFromLadderMethod /*= EDettachFromLadderMethod::Fall*/)
{
	switch (DettachFromLadderMethod)
	{
		case EDettachFromLadderMethod::JumpOff:
		{
			FVector JumpDirection = CurrentLadder->GetActorForwardVector();
			SetMovementMode(MOVE_Falling);

			FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;

			ForceTargetRotation = JumpDirection.ToOrientationRotator();
			bForceRotation = true;

			Launch(JumpVelocity);
			break;
		}
		case EDettachFromLadderMethod::ReachingTheTop:
		{
			GetBaseCharacterOwner()->Mantle(true);
			break;
		}
		case EDettachFromLadderMethod::ReachingTheBottom:
		{
			SetMovementMode(MOVE_Walking);
			break;
		}
		case EDettachFromLadderMethod::Fall:
		default:
		{
			SetMovementMode(MOVE_Falling);
			break;
		}
	}
}

bool UGCBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder;
}

const class ALadder* UGCBaseCharacterMovementComponent::GetCurrentLadder()
{
	return CurrentLadder;
}

float UGCBaseCharacterMovementComponent::GetLadderSpeedRatio() const
{
	checkf(IsValid(CurrentLadder), TEXT("float UGCBaseCharacterMovementComponent::GetLadderSpeedRatio() can't be invoked, when current ladder is null"));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

void UGCBaseCharacterMovementComponent::AttachToZipline(const AZipline* Zipline)
{
	CurrentZipline = Zipline;
	ZiplineDirection = GetCurrentZipline()->GetZiplineVector();

	FRotator TargetOrientationRotation = ZiplineDirection.ToOrientationRotator();
	TargetOrientationRotation.Pitch = 0.0f;
	
	FVector AttachPoint = CurrentZipline->GetZiplineAttachPoint(CharacterOwner);

	float CharacterOwnerCapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float ZiplineToCharacterOffset = 35.0f;

	FVector NewCharacterLocation = AttachPoint - (CharacterOwnerCapsuleHalfHeight + ZiplineToCharacterOffset) * FVector::UpVector;

	GetOwner()->SetActorRotation(TargetOrientationRotation);
	GetOwner()->SetActorLocation(NewCharacterLocation);

	InitialZiplineSpeed = GetOwner()->GetVelocity().ProjectOnTo(ZiplineDirection).Size();

	ZiplineAccelerationTimeline.PlayFromStart();
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

void UGCBaseCharacterMovementComponent::DettachFromZipline()
{
	ZiplineAccelerationTimeline.Stop();
	InitialZiplineSpeed = 0.0f;
	CurrentZiplineSpeed = 0.0f;

	SetMovementMode(MOVE_Falling);
}

bool UGCBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline;
}

const class AZipline* UGCBaseCharacterMovementComponent::GetCurrentZipline()
{
	return CurrentZipline;
}

void UGCBaseCharacterMovementComponent::StartWallRun(const FVector& HitNormal)
{
	if (IsWallRunning() || !IsSurfaceWallRunable(HitNormal) || !IsFalling())
	{
		return;
	}

	if ((GetBaseCharacterOwner()->GetActorForwardVector() + HitNormal).IsNearlyZero(0.7f))
	{
		return;
	}

	CurrentWallRunParameters.Side = EWallRunSide::None;
	CurrentWallRunParameters.Direction = FVector::ZeroVector;

	GetWallRunSideAndDirection(HitNormal, CurrentWallRunParameters.Side, CurrentWallRunParameters.Direction);
	CurrentWallRunParameters.WallNormal = HitNormal;
	
	FRotator TargetActorRotation = CurrentWallRunParameters.Direction.ToOrientationRotator();
	GetOwner()->SetActorRotation(TargetActorRotation);

	SetPlaneConstraintNormal(FVector::UpVector);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_WallRun);

	CurrentWallRunParameters.Speed = GetMaxSpeed();
}

void UGCBaseCharacterMovementComponent::StopWallRun(EStopWallRunMethod StopWallRunMethod/* = EStopWallRunMethod::Fall*/)
{
	switch (StopWallRunMethod)
	{
		case EStopWallRunMethod::JumpOff:
		{	
			FVector JumpDirection = CurrentWallRunParameters.WallNormal + GetBaseCharacterOwner()->GetActorForwardVector();
			JumpDirection += FVector::UpVector;
			SetMovementMode(MOVE_Falling);

			CurrentWallRunParameters.Side = EWallRunSide::None;
			CurrentWallRunParameters.Direction = FVector::ZeroVector;
			CurrentWallRunParameters.Speed = 0.0f;
			SetPlaneConstraintNormal(FVector::ZeroVector);

			FVector JumpVelocity = JumpDirection * JumpOffFromWallRunSpeed;

			ForceTargetRotation = JumpDirection.GetUnsafeNormal2D().ToOrientationRotator();
			bForceRotation = true;

			Launch(JumpVelocity);
			break;
		}
		case EStopWallRunMethod::Fall:
		{
			SetMovementMode(MOVE_Falling);

			CurrentWallRunParameters.Side = EWallRunSide::None;
			CurrentWallRunParameters.Direction = FVector::ZeroVector;
			CurrentWallRunParameters.Speed = 0.0f;
			SetPlaneConstraintNormal(FVector::ZeroVector);
			break;
		}
		default:
			break;
	}
}

bool UGCBaseCharacterMovementComponent::IsWallRunning() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRun;
}

FWallRunParameters UGCBaseCharacterMovementComponent::GetWallRunParameters() const
{
	return CurrentWallRunParameters;
}

void UGCBaseCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ZiplineAccelTimelineCurve))
	{
		FOnTimelineFloatStatic ZiplineMovementTimelineUpdate;
		ZiplineMovementTimelineUpdate.BindUObject(this, &UGCBaseCharacterMovementComponent::ZiplineTimelineUpdate);
		ZiplineAccelerationTimeline.AddInterpFloat(ZiplineAccelTimelineCurve, ZiplineMovementTimelineUpdate);
	}

	if (IsValid(SlideSlowDownTimelineCurve))
	{
		FOnTimelineFloatStatic SlideMovementTimelineUpdate;
		SlideMovementTimelineUpdate.BindUObject(this, &UGCBaseCharacterMovementComponent::UpdateSlideSpeed);
		SlideSlowDownTimeline.AddInterpFloat(SlideSlowDownTimelineCurve, SlideMovementTimelineUpdate);
	}
}

void UGCBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	switch (CustomMovementMode)
	{
	case (uint8)ECustomMovementMode::CMOVE_Mantling:
	{
		PhysMantling(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_Ladder:
	{
		PhysLadder(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_Zipline:
	{
		PhysZipline(DeltaTime, Iterations);
		break;
	}
	case (uint8)ECustomMovementMode::CMOVE_WallRun:
	{
		PhysWallRun(DeltaTime, Iterations);
		break;
	}
	default:
		break;
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

void UGCBaseCharacterMovementComponent::PhysMantling(float DeltaTime, uint32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;

	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);

	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;

	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);

	FVector TargetLedgeLocation = CurrentMantlingParameters.TargetLedgePrimitiveComponent->GetComponentLocation();
	FVector CorrectedTargetLocation = CurrentMantlingParameters.TargetLocation + TargetLedgeLocation;

	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CorrectedTargetLocation, PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);

	FVector Delta = NewLocation - GetActorLocation();

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}

void UGCBaseCharacterMovementComponent::PhysLadder(float DeltaTime, uint32 Iterations)
{
	CalcVelocity(DeltaTime, 1.0f, false, ClimbingOnLadderBreakingDeceleration);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}

	FVector NewPos = GetActorLocation() + Delta;
	float NewPosProjection = GetActorToCurrentLadderProjection(NewPos);

	if (NewPosProjection < MinLadderBottomOffset)
	{
		DettachFromLadder(EDettachFromLadderMethod::ReachingTheBottom);
		return;
	}
	else if (NewPosProjection > (CurrentLadder->GetLadderHeight() - MaxLadderTopOffset))
	{
		DettachFromLadder(EDettachFromLadderMethod::ReachingTheTop);
		return;
	}
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UGCBaseCharacterMovementComponent::PhysZipline(float DeltaTime, uint32 Iterations)
{
	ZiplineAccelerationTimeline.TickTimeline(DeltaTime);
	FVector Delta = ZiplineDirection * CurrentZiplineSpeed * DeltaTime;
	
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);

	if (Hit.Actor == CurrentZipline)
	{
		DettachFromZipline();
	}

	UE_LOG(LogBaseCharacterMovement, Display, TEXT("Current Zipline Speed | %f"), CurrentZiplineSpeed);
}

void UGCBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, uint32 Iterations)
{		
	FHitResult LineTraceHitResult;

	FVector BaseCharacterRightVector = GetBaseCharacterOwner()->GetActorRightVector();
	FVector LineTraceDirection = CurrentWallRunParameters.Side == EWallRunSide::Right ? BaseCharacterRightVector : -BaseCharacterRightVector;

	FVector LineTraceStart = GetBaseCharacterOwner()->GetActorLocation();
	FVector LineTraceEnd = LineTraceStart + WallRunUpdateLinetraceLength * LineTraceDirection;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetBaseCharacterOwner());

	if (GetWorld()->LineTraceSingleByChannel(LineTraceHitResult, LineTraceStart, LineTraceEnd, ECC_WallRunnable, QueryParams))
	{
		EWallRunSide Side = EWallRunSide::None;
		FVector Direction = FVector::ZeroVector;
		GetWallRunSideAndDirection(LineTraceHitResult.ImpactNormal, Side, Direction);

		if (Side != CurrentWallRunParameters.Side)
		{
			StopWallRun();
		}
		else
		{
			CurrentWallRunParameters.Direction = Direction;
			FVector Delta = CurrentWallRunParameters.Direction * CurrentWallRunParameters.Speed * DeltaTime;
			FHitResult MoveHit;
			SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, MoveHit);
		}
	}
	else
	{
		StopWallRun();
	}

}

void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		CurrentLadder = nullptr;
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		CurrentZipline = nullptr;
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_WallRun)
	{
		GetWorld()->GetTimerManager().ClearTimer(WallRunTimer);
	}

	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
			case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UGCBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
				break;
			}
			case (uint8)ECustomMovementMode::CMOVE_WallRun:
			{
				FTimerDelegate WallRunTimerDel;
				WallRunTimerDel.BindUObject(this, &UGCBaseCharacterMovementComponent::StopWallRun, EStopWallRunMethod::Fall);
				GetWorld()->GetTimerManager().SetTimer(WallRunTimer, WallRunTimerDel, MaxWallRunTime, false);
				break;
			}
			default:
				break;
		}
	}
}

AGCBaseCharacter* UGCBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return StaticCast<AGCBaseCharacter*>(CharacterOwner);
}

void UGCBaseCharacterMovementComponent::ZiplineTimelineUpdate(float Alpha)
{
	float ZiplineTargetSpeed = FMath::Lerp(InitialZiplineSpeed, MaxZiplineSpeed, Alpha);
	CurrentZiplineSpeed = ZiplineTargetSpeed;
}

bool UGCBaseCharacterMovementComponent::IsSurfaceWallRunable(const FVector& SurfaceNormal) const
{
	return !(SurfaceNormal.Z > GetWalkableFloorZ() || SurfaceNormal.Z < -0.005f);
}

void UGCBaseCharacterMovementComponent::GetWallRunSideAndDirection(const FVector& HitNormal, EWallRunSide& OutSide, FVector& OutDirection) const
{
	if (FVector::DotProduct(HitNormal, GetBaseCharacterOwner()->GetActorRightVector()) > 0.f)
	{
		OutSide = EWallRunSide::Left;
		OutDirection = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
	}
	else
	{
		OutSide = EWallRunSide::Right;
		OutDirection = FVector::CrossProduct(FVector::UpVector, HitNormal).GetSafeNormal();
	}
}