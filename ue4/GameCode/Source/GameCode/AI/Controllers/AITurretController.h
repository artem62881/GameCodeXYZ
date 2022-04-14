// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCAIController.h"
#include "Characters/GCBaseCharacter.h"
#include "AITurretController.generated.h"

/**
 * 
 */
class ATurret;
UCLASS()
class GAMECODE_API AAITurretController : public AGCAIController
{
	GENERATED_BODY()
public:
	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

protected:
	UFUNCTION()
	void OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
private:
	TWeakObjectPtr<ATurret> CachedTurret;
};