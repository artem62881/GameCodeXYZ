// Fill out your copyright notice in the Description page of Project Settings.


#include "GCAICharacter.h"

AGCAICharacter::AGCAICharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIPatrollingComponent = CreateDefaultSubobject<UAIPatrollingComponent>(TEXT("AI Patrolling Component"));
}

UAIPatrollingComponent* AGCAICharacter::GetAIPatrollingComponent() const
{
	return AIPatrollingComponent;
}

UBehaviorTree* AGCAICharacter::GetBehaviorTree() const
{
	return BehaviourTree;
}
