// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"
#include "GCAICharacter.generated.h"

/**
 * 
 */
class UAIPatrollingComponent;
class UBehaviorTree;
UCLASS(Blueprintable)
class GAMECODE_API AGCAICharacter : public AGCBaseCharacter
{
	GENERATED_BODY()
public:
	AGCAICharacter(const FObjectInitializer& ObjectInitializer);

	UAIPatrollingComponent* GetAIPatrollingComponent() const;
	UBehaviorTree* GetBehaviorTree() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAIPatrollingComponent* AIPatrollingComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviourTree;
};
