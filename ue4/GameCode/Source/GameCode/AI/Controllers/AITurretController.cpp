// Fill out your copyright notice in the Description page of Project Settings.


#include "AITurretController.h"

#include "AI/Characters/Turret.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("void AAITurretController::SetPawn(APawn* InPawn) AITurretController can be used only with ATurret"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
		CachedTurret->OnTakeAnyDamage.AddDynamic(this, &AAITurretController::OnTakeAnyDamageEvent);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!CachedTurret.IsValid() || CachedTurret->IsDestroyed())
	{
		return;
	}
	
	AActor* ClosestActor = GetClosestSensedActor(UAISense_Damage::StaticClass());
	if (ClosestActor)
	{
		CachedTurret->SetCurrentTarget(ClosestActor);
		return;
	}
	
	ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());	
	if (ClosestActor)
	{
		CachedTurret->SetCurrentTarget(ClosestActor);
	}
}

void AAITurretController::OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	UAISense_Damage::ReportDamageEvent(GetWorld(), DamagedActor, DamageCauser, Damage, DamageCauser->GetActorLocation(), DamagedActor->GetActorLocation());
}