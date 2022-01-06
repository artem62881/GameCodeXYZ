// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GCPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API AGCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	bool GetIgnoreCameraPitch() const;

	void SetIgnoreCameraPitch(bool bIgnoreCameraPitch_In);
	
protected:
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UPlayerHUDWidget> PlayerHUDWidgetClass;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void Jump();
	void PlayerStartFire();
	void PlayerStopFire();
	void StartAiming();
	void StopAiming();
	void Reload();
	void NextItem();
	void PreviousItem();
	void Mantle();
	void WallRun();
	void ChangeCrouchState();
	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);

	void ClimbLadderUp(float Value);
	void InteractWithLadder();

	void InteractWithZipline();

	void StartSlide();

	void StartSprint();
	void StopSprint();

	void CreateAndInitializeWidgets();

	TSoftObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

	class UPlayerHUDWidget* PlayerHUDWidget = nullptr;

	bool bIgnoreCameraPitch = false;
};
