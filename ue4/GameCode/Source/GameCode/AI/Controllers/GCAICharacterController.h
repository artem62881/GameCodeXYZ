// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Characters/GCAICharacter.h"
#include "AI/Controllers/GCAIController.h"
#include "GCAICharacterController.generated.h"

/**
 * 
 */
class AGCAICharacter;
UCLASS()
class GAMECODE_API AGCAICharacterController : public AGCAIController
{
	GENERATED_BODY()
public:
	virtual void SetPawn(APawn* InPawn) override;
	
	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category= "Movement")
	float TargetReachRadius = 100.f;

	UFUNCTION()
	void OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

private:
	void TryMoveToNextTarget();
	bool IsTargetReached(FVector TargetLocation) const;
	
	TWeakObjectPtr<AGCAICharacter> CachedAICharacter;
	
	bool bIsPatrolling = false;
};
