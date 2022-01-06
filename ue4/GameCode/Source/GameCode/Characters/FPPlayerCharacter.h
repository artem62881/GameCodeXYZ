// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "FPPlayerCharacter.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FLadderCameraSettings
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = -89.f, UIMax = 89.f))
	float MinPitch = -60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = -89.f, UIMax = 89.f))
	float MaxPitch = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = -179.f, UIMax = 179.f))
	float MinYaw = -70.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = -179.f, UIMax = 179.f))
	float MaxYaw = 70.f;
};

USTRUCT(BlueprintType)
struct FZiplineCameraSettings
{
	GENERATED_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = -89.f, UIMax = 89.f))
	float MinPitch = -89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = -89.f, UIMax = 89.f))
	float MaxPitch = 89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinYaw = -89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxYaw = 89.f;
};

UCLASS()
class GAMECODE_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;

	virtual FRotator GetViewRotation() const override;

	virtual void HardLanding() override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

protected:
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	USkeletalMeshComponent* FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person")
	FLadderCameraSettings LadderCameraSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person")
	FZiplineCameraSettings ZiplineCameraSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Landing")
	class UAnimMontage* FPHardLandingMontage;

private:
	FTimerHandle FPMontageTimer;

	void OnFPMontageTimerElapsed();

	bool IsFPMontagePlaying() const;

	void OnLadderStart();
	void OnLadderEnd();

	void OnZiplineStart();
	void OnZiplineEnd();

	TWeakObjectPtr<class AGCPlayerController> GCPlayerController;
};
