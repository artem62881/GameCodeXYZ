// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveVector.h"
#include "Animation/AnimMontage.h"
#include "GCBaseCharacter.generated.h"

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveVector* MantlingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeightStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeightStartTime = 0.5f;
};

USTRUCT(BlueprintType)
struct FSlideSettings
{
	GENERATED_BODY()

	float CapsuleHeightAdjustment = 0.f;

	FVector SlideDirection;

	FRotator SlideRotation;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool);

class AInteractiveActor;
class UGCBaseCharacterMovementComponent;
class UCharacterAttributesComponent;
class UCharacterEquipmentComponent;
UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void MoveForward(float Value) {};
	virtual void MoveRight(float Value) {};

	virtual void Turn(float Value) {};
	virtual void LookUp(float Value) {};
	virtual void TurnAtRate(float Value) {};
	virtual void LookUpAtRate(float Value) {};

	virtual void ChangeCrouchState();

	virtual void StartSlide();
	virtual void StopSlide();

	virtual void OnStartSlide(float HalfHeightAdjust) {};
	virtual void OnEndSlide(float HalfHeightAdjust) {};

	virtual void StartSprint();
	virtual void StopSprint();

	virtual void Tick(float DeltaTime) override;

	virtual void SwimForward(float Value) {};
	virtual void SwimRight(float Value) {};
	virtual void SwimUp(float Value) {};

	void StartFire();
	void StopFire();

	void StartAiming();
	void StopAiming();

	void Reload();

	void EquipNextItem();
	void EquipPreviousItem();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStartAiming();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStopAiming();

	bool IsAiming() const;
	float GetAimingMovementSpeed() const;

	FOnAimingStateChanged OnAimingStateChanged;

	UFUNCTION(BlueprintCallable)
	void Mantle(bool bForce = false);

	virtual void WallRun();

	virtual void HardLanding();
	virtual void EndHardLanding();

	virtual bool CanJumpInternal_Implementation() const override;

	void DisableMeshRotation() { bUseControllerRotationYaw = false; };
	void EnableMeshRotation() { bUseControllerRotationYaw = true; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFootOffset() const { return IKRightFootOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFootOffset() const { return IKLeftFootOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKPelvisOffset() const { return IKPelvisOffset; }

	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);

	void UnregisterInteractiveActor(AInteractiveActor* InteractiveActor);

	void ClimbLadderUp(float Value);

	void InteractWithLadder();

	const class ALadder* GetAvailableLadder();

	void InteractWithZipline();

	const class AZipline* GetAvailableZipline();

	bool IsWallRunRequested() const;

	void SetIsWallRunRequested(bool bIsRequested);

	virtual void Falling() override;
	virtual void NotifyJumpApex() override;
	virtual void Landed(const FHitResult& Hit) override;

	float GetDefaultCapsuleRadius() const;

	UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const { return GCBaseCharacterMovementComponent; }

	bool IsSwimmingUnderwater() const;

	UCharacterEquipmentComponent* GetCharacterEquipmentComponent_Mutable() const;
	const UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const;

	UCharacterAttributesComponent* GetCharacterAttributesComponent() const;

/** IGenericTeamInterface */
	virtual FGenericTeamId GetGenericTeamId() const override;
/** ~IGenericTeamInterface */
	
protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintEnd();
	virtual void OnSprintEnd_Implementation();

	virtual void OnStartAimingInternal();
	virtual void OnStopAimingInternal();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | IK Settings")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | IK Settings")
	FName LeftFootSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceDistance = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 50.0f;

	virtual bool CanSprint();

	virtual bool CanSlide();

	bool CanMantle() const;

	bool CanWallRun() const;

	bool CanFire() const;

	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime) {};

	UGCBaseCharacterMovementComponent* GCBaseCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Movement")
	class ULedgeDetectorComponent* LedgeDetectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LowMantleMaxHeight = 125.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement | Slide", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SlideCaspsuleHalfHeight = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement | Landing", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHardLandingHeight = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Landing")
	class UAnimMontage* HardLandingMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charatcer | Components")
	UCharacterAttributesComponent* CharacterAttributesComponent;

	virtual void OnDeath();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	class UAnimMontage* OnDeathAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	class UCurveFloat* FallDamageCurve; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterEquipmentComponent* CharacterEquipmentComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Team")
	ETeams Team = ETeams::Enemy;

private:
	void TryChangeSprintState();

	bool bIsSprintRequested = false;
	
	UFUNCTION()
	void OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
		
	void UpdateIKSettings(float DeltaSeconds);

	float GetIKOffsetForASocket(const FName& SocketName);
	float CalculateIKPelvisOffset();

	float IKRightFootOffset = 0.0f;
	float IKLeftFootOffset = 0.0f;
	float IKPelvisOffset = 0.0f;

	bool bIsWallRunRequested = false;

	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;

	TArray<AInteractiveActor*> AvailableInteractiveActors; 

	FSlideSettings CurrentSlideSettings;

	float DefaultCapsuleHalfHeight = 0.f;
	float DefaultCapsuleRadius = 0.f;

	float JumpApexHeight = 0.f;
	FVector CurrentFallApex = FVector::ZeroVector;

	FTimerHandle DeathMontageTimer;
	void EnableRagdoll();

	bool bIsAiming = false;
	float CurrentAimingMovementSpeed;
};
