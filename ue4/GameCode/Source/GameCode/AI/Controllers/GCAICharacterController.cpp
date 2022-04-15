// Fill out your copyright notice in the Description page of Project Settings.


#include "GCAICharacterController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Sight.h"
#include "GameCodeTypes.h"
#include "Perception/AISense_Damage.h"

void AGCAICharacterController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<AGCAICharacter>(), TEXT("void AGCAICharacterController::SetPawn(APawn* InPawn) GCAICherecterController can be used only with AICharacter"));
		CachedAICharacter = StaticCast<AGCAICharacter*>(InPawn);
		RunBehaviorTree(CachedAICharacter->GetBehaviorTree());
		CachedAICharacter->OnTakeAnyDamage.AddDynamic(this, &AGCAICharacterController::OnTakeAnyDamageEvent);
	}
	else
	{
		CachedAICharacter = nullptr;
	}
}

void AGCAICharacterController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!CachedAICharacter.IsValid())
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("ActorsPerceptionUpdated"));
	TryMoveToNextTarget();
}

void AGCAICharacterController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (!Result.IsSuccess())
	{
		return;
	}
	TryMoveToNextTarget();
}

void AGCAICharacterController::BeginPlay()
{
	Super::BeginPlay();
	UAIPatrollingComponent* PatrollingComponent = CachedAICharacter->GetAIPatrollingComponent();
	if (PatrollingComponent->CanPatrol())
	{
		FVector ClosestWayPoint = PatrollingComponent->SelectClosestWayPoint();
		if (IsValid(Blackboard))
		{
			Blackboard->SetValueAsVector(BB_NextLocation, ClosestWayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
		}
		bIsPatrolling = true;
	}
}

void AGCAICharacterController::OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	UAISense_Damage::ReportDamageEvent(GetWorld(), DamagedActor, DamageCauser, Damage, DamageCauser->GetActorLocation(), DamagedActor->GetActorLocation());
	//TryMoveToNextTarget();
}

void AGCAICharacterController::TryMoveToNextTarget()
{
	AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	UAIPatrollingComponent* PatrollingComponent = CachedAICharacter->GetAIPatrollingComponent();
	
	if (IsValid(ClosestActor))
	{
		if (IsValid(Blackboard))
		{
			Blackboard->SetValueAsBool(BB_bForceMove, true);
			//Blackboard->SetValueAsBool(BB_bForceMove, false);
			Blackboard->SetValueAsObject(BB_CurrentTarget, ClosestActor);
			SetFocus(ClosestActor, EAIFocusPriority::Gameplay);
		}
		bIsPatrolling = false;
	}
	else
	{
		ClosestActor = GetClosestSensedActor(UAISense_Damage::StaticClass());
		if (IsValid(ClosestActor))
		{
			if (IsValid(Blackboard))
			{
				ClearFocus(EAIFocusPriority::Gameplay);
				Blackboard->SetValueAsBool(BB_bForceMove, true);
				Blackboard->SetValueAsVector(BB_NextLocation, ClosestActor->GetActorLocation());
				Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
			}
			bIsPatrolling = false;
		}
	}
	
	if (PatrollingComponent->CanPatrol() && !IsValid(ClosestActor))
	{
		FVector NextWayPoint = bIsPatrolling ? PatrollingComponent->SelectNextWayPoint() : PatrollingComponent->SelectClosestWayPoint();
		bIsPatrolling = true;
		if (IsValid(Blackboard))
		{
			ClearFocus(EAIFocusPriority::Gameplay);
			Blackboard->SetValueAsBool(BB_bForceMove, false);
			Blackboard->SetValueAsVector(BB_NextLocation, NextWayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
		}
	}
}

bool AGCAICharacterController::IsTargetReached(FVector TargetLocation) const
{
	return (TargetLocation - CachedAICharacter->GetActorLocation()).SizeSquared() <= FMath::Square(TargetReachRadius);
}
