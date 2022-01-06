// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LedgeDetectorComponent.h"
#include "Engine/EngineTypes.h"
#include "Curves/CurveVector.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include "Components/TimelineComponent.h"
#include "Characters/GCBaseCharacter.h"
#include "GCBaseCharacterMovementComponent.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_Ladder UMETA(DisplayName = "Ladder"),
	CMOVE_Zipline UMETA(DisplayName = "Zipline"),
	CMOVE_WallRun UMETA(DisplayName = "WallRun"),
	CMOVE_MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDettachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff
};

UENUM(BlueprintType)
enum class EWallRunSide : uint8
{
	None = 0,
	Left,
	Right
};

UENUM(BlueprintType)
enum class EStopWallRunMethod : uint8
{
	Fall = 0,
	JumpOff
};

struct FMantlingMovementParameters
{
	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	TWeakObjectPtr<class UPrimitiveComponent> TargetLedgePrimitiveComponent = nullptr;

	FVector InitialAnimationLocation = FVector::ZeroVector;

	float Duration = 1.0f;
	float StartTime = 0.0f;

	UCurveVector* MantlingCurve;
};

struct FWallRunParameters
{
	EWallRunSide Side = EWallRunSide::None;
	FVector Direction = FVector::ZeroVector;
	FVector WallNormal = FVector::ZeroVector;
	float Speed = 0.0f;
};

UCLASS()
class GAMECODE_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	virtual void PhysicsRotation(float DeltaTime) override;

	bool IsSprinting() const { return bIsSprinting; }

	bool IsSliding() const { return bIsSliding; }

	bool IsOutOfStamina() const { return bIsOutOfStamina; }
	void SetIsOutOfStamina(bool bIsOutOfStamina_In);

	virtual float GetMaxSpeed() const override;

	void StartSprint();
	void StopSprint();

	void StartSlide(FSlideSettings SlideSettings);
	void UpdateSlide(float DeltaTime, FSlideSettings SlideSettings);
	void UpdateSlideSpeed(float Alpha);
	void StopSlide(FSlideSettings SlideSettings);

	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();
	bool IsMantling() const;

	void AttachToLadder(const ALadder* Ladder);
	float GetActorToCurrentLadderProjection(const FVector& Location) const;
	void DettachFromLadder(EDettachFromLadderMethod DettachFromLadderMethod = EDettachFromLadderMethod::Fall);
	bool IsOnLadder() const;
	const class ALadder* GetCurrentLadder();
	float GetLadderSpeedRatio() const;

	void AttachToZipline(const AZipline* Zipline);
	void DettachFromZipline();
	bool IsOnZipline() const;
	const class AZipline* GetCurrentZipline();

	void StartWallRun(const FVector& HitNormal);
	void StopWallRun(EStopWallRunMethod StopWallRunMethod = EStopWallRunMethod::Fall);
	bool IsWallRunning() const;
	
	FWallRunParameters GetWallRunParameters() const;

	virtual void BeginPlay() override;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	void PhysMantling(float DeltaTime, uint32 Iterations);
	void PhysLadder(float DeltaTime, uint32 Iterations);
	void PhysZipline(float DeltaTime, uint32 Iterations);
	void PhysWallRun(float DeltaTime, uint32 Iterations);

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: Sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: Sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OutOfStaminaSpeed = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Swimming", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SwimmingCapsuleRadius = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Swimming", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SwimmingCapsuleHalfHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ClimbingOnLadderRegularSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ClimbingOnLadderMaxSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ClimbingOnLadderBreakingDeceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LadderToCharacterOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxLadderTopOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinLadderBottomOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float JumpOffFromLadderSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Zipline", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxZiplineSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: Zipline")
	UCurveFloat* ZiplineAccelTimelineCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Wall Run", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxWallRunTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: Wall Run", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxWallRunSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wall Run", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float JumpOffFromWallRunSpeed = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wall Run", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float WallRunUpdateLinetraceLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Slide", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SlideMaxSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Slide", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SlideMaxTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: Slide")
	UCurveFloat* SlideSlowDownTimelineCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Slide", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SlideOverLedgeOffset = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Slide", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SlideDownLineTraceLength = 10.0f;
	
	class AGCBaseCharacter* GetBaseCharacterOwner() const;

private:
	bool bIsSprinting;
	bool bIsOutOfStamina;

	bool bIsSliding;

	FMantlingMovementParameters CurrentMantlingParameters;
	FTimerHandle MantlingTimer;

	const ALadder* CurrentLadder = nullptr;
	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;

	FVector ZiplineDirection = FVector::ZeroVector;
	const AZipline* CurrentZipline = nullptr;
	FTimeline ZiplineAccelerationTimeline;
	void ZiplineTimelineUpdate(float Alpha);
	float InitialZiplineSpeed = 0.0f;
	float CurrentZiplineSpeed = 0.0f;

	bool IsSurfaceWallRunable(const FVector& SurfaceNormal) const;
	void GetWallRunSideAndDirection(const FVector& HitNormal, EWallRunSide& OutSide, FVector& OutDirection) const;
	FWallRunParameters CurrentWallRunParameters;
	FTimerHandle WallRunTimer;

	FTimerHandle SlidingTimer;
	FTimeline SlideSlowDownTimeline;
	float CurrentSlideSpeed = SlideMaxSpeed;
};
