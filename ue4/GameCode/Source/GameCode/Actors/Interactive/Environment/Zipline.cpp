// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameCode/GameCodeTypes.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	FirstPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPole"));
	FirstPoleStaticMeshComponent->SetupAttachment(RootComponent);

	SecondPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondPole"));
	SecondPoleStaticMeshComponent->SetupAttachment(RootComponent);

	CableStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cable"));
	CableStaticMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	FirstPoleStaticMeshComponent->SetRelativeLocation(FirstPoleLocation);
	SecondPoleStaticMeshComponent->SetRelativeLocation(SecondPoleLocation);

	CableVector = SecondPoleStaticMeshComponent->GetRelativeLocation() - FirstPoleStaticMeshComponent->GetRelativeLocation();
	float CableLength = CableVector.Size();

	FVector CableMeshLocation = (FirstPoleStaticMeshComponent->GetRelativeLocation() + SecondPoleStaticMeshComponent->GetRelativeLocation() + PolesHeight * FVector::UpVector) * 0.5f;
	FRotator CableMeshRotation = FRotator(CableVector.Rotation().Pitch + 90.0f, CableVector.Rotation().Yaw, 0.0f);

	CableStaticMeshComponent->SetRelativeLocation(CableMeshLocation);
	CableStaticMeshComponent->SetRelativeRotation(CableMeshRotation);

	UStaticMesh* FirstPoleMesh = FirstPoleStaticMeshComponent->GetStaticMesh();
	if (IsValid(FirstPoleMesh))
	{
		float MeshHeight = FirstPoleMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			FirstPoleStaticMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, PolesHeight / MeshHeight));
		}
	}

	UStaticMesh* SecondPoleMesh = SecondPoleStaticMeshComponent->GetStaticMesh();
	if (IsValid(SecondPoleMesh))
	{
		float MeshHeight = SecondPoleMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			SecondPoleStaticMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, PolesHeight / MeshHeight));
		}
	}

	UStaticMesh* CableMesh = CableStaticMeshComponent->GetStaticMesh();
	if (IsValid(CableMesh))
	{
		float MeshLength = CableMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshLength))
		{
			CableStaticMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, CableLength / MeshLength));
		}
	}
	
	FVector InteractionVolumeLocation = CableMeshLocation;
	FRotator InteractionVolumeRotation = CableMeshRotation;

	InteractionVolume->SetRelativeLocation(InteractionVolumeLocation);
	InteractionVolume->SetRelativeRotation(InteractionVolumeRotation);
	GetZiplineInteractionCapsule()->SetCapsuleSize(InteractionCapsuleRadius, CableLength * 0.5f);
}

FVector AZipline::GetZiplineAttachPoint(const ACharacter* Character) const
{
	FVector Result = FVector::ZeroVector;
	FVector CharacterPoint = Character->GetActorLocation();
	float DistanceToPoint = CableStaticMeshComponent->GetClosestPointOnCollision(CharacterPoint, Result);
	
	return Result;
}

FVector AZipline::GetZiplineVector() const
{
	FVector Result = FVector::ZeroVector;

	FVector CableForwardVector = CableStaticMeshComponent->GetUpVector();
	if (CableStaticMeshComponent->GetUpVector().ToOrientationRotator().Pitch >= 0.0f)
	{
		Result = -CableForwardVector;
	}
	else
	{
		Result = CableForwardVector;
	}

	return Result;
}

class UCapsuleComponent* AZipline::GetZiplineInteractionCapsule() const
{
	return StaticCast<UCapsuleComponent*>(InteractionVolume);
}