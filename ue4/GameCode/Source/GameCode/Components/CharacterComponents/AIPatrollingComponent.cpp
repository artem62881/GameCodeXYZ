// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPatrollingComponent.h"

#include "Actors/Navigation/PatrollingPath.h"

bool UAIPatrollingComponent::CanPatrol() const
{
	return IsValid(PatrolSettings.PatrollingPath) && PatrolSettings.PatrollingPath->GetWayPoints().Num() > 0;
}

FVector UAIPatrollingComponent::SelectClosestWayPoint()
{
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	const TArray<FVector> WayPoints = PatrolSettings.PatrollingPath->GetWayPoints();
	FTransform PathTransform = PatrolSettings.PatrollingPath->GetActorTransform();

	FVector ClosestWayPoint;
	float MinSqDistance = FLT_MAX;
	for (int32 i = 0; i < WayPoints.Num(); ++i)
	{
		FVector WayPointWorld = PathTransform.TransformPosition(WayPoints[i]);
		float CurrentSqDistance = (OwnerLocation - WayPointWorld).SizeSquared();
		if (CurrentSqDistance < MinSqDistance)
		{
			MinSqDistance = CurrentSqDistance;
			ClosestWayPoint = WayPointWorld;
			CurrentWayPointIndex = i;
		}
	}
	return  ClosestWayPoint;
}

FVector UAIPatrollingComponent::SelectNextWayPoint()
{
	const TArray<FVector> WayPoints = PatrolSettings.PatrollingPath->GetWayPoints();

	switch (PatrolSettings.PatrolMode)
	{
	case EPatrolMode::None:
		{
			break;
		}
		
	case EPatrolMode::Circle:
		{
			++CurrentWayPointIndex;
			if (CurrentWayPointIndex == WayPoints.Num())
			{
				CurrentWayPointIndex = 0;
			}
			break;
		}
		
	case EPatrolMode::PingPong:
		{
			CurrentWayPointIndex += CurrentPatrolDirection;
			if (CurrentWayPointIndex == WayPoints.Num())
			{
				CurrentPatrolDirection = -1;
				CurrentWayPointIndex = WayPoints.Num() - 2;
			}
			else if (CurrentWayPointIndex <= 0)
			{
				CurrentPatrolDirection = 1;
				CurrentWayPointIndex = 0;
			}
			break;
		}
	}
	
	FTransform PathTransform = PatrolSettings.PatrollingPath->GetActorTransform();
	FVector WayPoint = PathTransform.TransformPosition(WayPoints[CurrentWayPointIndex]);
	
	return WayPoint;
}
