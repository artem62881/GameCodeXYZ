// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBaseCharacter.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAMECODE_API APlayerCharacter : public AGCBaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void MoveForward(float Value) override;
	virtual void MoveRight(float Value) override;
	virtual void Turn(float Value) override;
	virtual void LookUp(float Value) override;
	virtual void TurnAtRate(float Value) override;
	virtual void LookUpAtRate(float Value) override;
	virtual void SwimForward(float Value) override;
	virtual void SwimRight(float Value) override;
	virtual void SwimUp(float Value) override;


	//virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	//virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	//virtual void OnStartSlide(float HalfHeightAdjust) override;
	//virtual void OnEndSlide(float HalfHeightAdjust) override;

	//virtual bool CanJumpInternal_Implementation() const override;
	//virtual void OnJumped_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Camera")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Camera")
	class USpringArmComponent* SpringArmComponent;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnStartSlide(float HalfHeightAdjust) override;
	virtual void OnEndSlide(float HalfHeightAdjust) override;

	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintEnd_Implementation() override;
	virtual void OnStartAimingInternal() override;
	virtual void OnStopAimingInternal() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Camera | Sprint")
	UCurveFloat* SprintCameraTimelineCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Camera | Sprint")
	float SprintArmLength = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Camera | Sprint", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SprintCameraMovementRate = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Camera | Aim")
	UCurveFloat* AimingCameraTimelineCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Camera | Aim", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AimingCameraMovementRate = 4.f;

public:
	float DefaultArmLength = 0.f;
	FTimeline SprintCameraTimeline;
	void SprintCameraMovementUpdate(float Alpha);
	
	FTimeline AimingCameraTimeline;
	void AimingCameraMovementUpdate(float Alpha);

};
