// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIPatrollingComponent.generated.h"

class APatrollingPath;
UENUM()
enum class EPatrolMode : uint8
{
	None = 0,
	Circle,
	PingPong
};

USTRUCT(BlueprintType)
struct FPatrolSettings
{
	GENERATED_BODY();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mode")
	EPatrolMode PatrolMode = EPatrolMode::None;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	APatrollingPath* PatrollingPath;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool CanPatrol() const;
	FVector SelectClosestWayPoint();
	FVector SelectNextWayPoint();
	
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol Settings")
	FPatrolSettings PatrolSettings;

private:
	int32 CurrentWayPointIndex = -1;
	int8 CurrentPatrolDirection = 1;
};
