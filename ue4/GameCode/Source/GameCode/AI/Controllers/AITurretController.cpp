// Fill out your copyright notice in the Description page of Project Settings.


#include "AITurretController.h"

#include "AI/Characters/Turret.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("void AAITurretController::SetPawn(APawn* InPawn) AITurretController can be used only with ATurret"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!CachedTurret.IsValid())
	{
		return;
	}
	
	AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());	
	CachedTurret->SetCurrentTarget(ClosestActor);
}
