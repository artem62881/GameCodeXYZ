// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "Components/Weapon/WeaponBarellComponent.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestroyedEventSignature);

UENUM(BlueprintType)
enum class ETurretState : uint8
{
	Searching,
	Firing,
	Destroyed
};

class UWeaponBarellComponent;
UCLASS()
class GAMECODE_API ATurret : public APawn
{
	GENERATED_BODY()

public:
	ATurret();

	virtual void PossessedBy(AController* NewController) override;

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

	void SetCurrentTarget(AActor* NewTarget);

	virtual FVector GetPawnViewLocation() const override;

	virtual FRotator GetViewRotation() const override;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FOnDestroyedEventSignature OnDestroyedEvent;

	bool IsDestroyed() const { return CurrentTurretState == ETurretState::Destroyed ? true : false; };
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBaseComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBarellComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UParticleSystem* DestroyFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxHealth = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseSearchingRotationRate = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BaseFiringInterpSpeed = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float BarellPitchRotationRate = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxBarellPitchAngle = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMax = 0.f, UIMax = 0.f))
	float MinBarellPitchAngle = -30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters | Fire", meta = (ClampMax = 1.f, UIMax = 1.f))
	float RateOfFire = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters | Fire", meta = (ClampMax = 0.f, UIMax = 0.f))
	float BulletSpreadAngle = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters | Fire", meta = (ClampMax = 0.f, UIMax = 0.f))
	float FireDelayTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters | Team")
	ETeams Team = ETeams::Enemy;

	UFUNCTION()
	void OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnDestroyed();
	
private:
	void SearchingMovement(float DeltaTime);
	void FiringMovement(float DeltaTime);

	void MakeShot();
	
	ETurretState CurrentTurretState = ETurretState::Searching;
	void SetCurrentTurretState(ETurretState NewState);

	AActor* CurrentTarget = nullptr;

	float GetFireInterval() const;
	
	FTimerHandle ShotTimer;

	float Health = 100.f;
};