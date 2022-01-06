// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeDetectorComponent.h"
#include "../GameCodeTypes.h"
#include <GameFramework/Character.h>
#include <Components/CapsuleComponent.h>
#include "DrawDebugHelpers.h"
#include "../Utils/GCTraceUtils.h"
#include "../GCGameInstance.h"
#include <Kismet/GameplayStatics.h>
#include "../Subsystems/DebugSubsystem.h"

// Called when the game starts
void ULedgeDetectorComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<ACharacter>(), TEXT("ULedgeDetectorComponent::BeginPlay() only a Character can use ULedgeDetectorComponent"))
	CachedCharacterOwner = StaticCast<ACharacter*>(GetOwner());
}

bool ULedgeDetectorComponent::DetectLedge(OUT FLedgeDescription& LedgeDescription)
{
	UCapsuleComponent* CapsuleComponent = CachedCharacterOwner->GetCapsuleComponent();

	ACharacter* DefaultCharacter = CachedCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	float DefaultCapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float DefaultCapsuleRadius = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());

	float BottomZOffset = 2.0f;
	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() - (CapsuleComponent->GetScaledCapsuleHalfHeight() - BottomZOffset) * FVector::UpVector;

	//Debug settings
#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryLedgeDetection);
#else
	bIsDebugEnabled = false;
#endif

	float DrawTime = 2.0f;
		
	//1. Forward check
	float ForwardCheckCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float ForwardCheckCapsuleHalfHeight = (MaxLedgeHeight - MinLedgeHeight) * 0.5f;

	FHitResult ForwardCheckHitResult;
	FVector ForwardTraceStartLocation = CharacterBottom + (MinLedgeHeight + ForwardCheckCapsuleHalfHeight) * FVector::UpVector;
	FVector ForwardTraceEndLocation = ForwardTraceStartLocation + CachedCharacterOwner->GetActorForwardVector() * ForwardCheckDistance;


	if (!GCTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), ForwardCheckHitResult, ForwardTraceStartLocation, ForwardTraceEndLocation, ForwardCheckCapsuleRadius, ForwardCheckCapsuleHalfHeight, FQuat::Identity, ECC_Climbing, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	//2. Downward check
	float DownwardCheckSphereRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float DownwardCheckDepthOffset = 10.0f;
	FHitResult DownwardCheckHitResult;
	FVector DownwardTraceStartLocation = ForwardCheckHitResult.ImpactPoint - ForwardCheckHitResult.ImpactNormal * DownwardCheckDepthOffset;
	DownwardTraceStartLocation.Z = CharacterBottom.Z + MaxLedgeHeight + DownwardCheckSphereRadius;
	FVector DownwardTraceEndLocation(DownwardTraceStartLocation.X, DownwardTraceStartLocation.Y, CharacterBottom.Z);

	if(!GCTraceUtils::SweepSphereSingleByChannel(GetWorld(), DownwardCheckHitResult, DownwardTraceStartLocation, DownwardTraceEndLocation, DownwardCheckSphereRadius, ECC_Climbing, QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	//3. Overlap check
	float OverlapCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	float OverlapCapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	float OverlapCapsuleFloorOffset = 2.0f;
	FVector OverlapLocation = DownwardCheckHitResult.ImpactPoint + (DefaultCapsuleHalfHeight + OverlapCapsuleFloorOffset) * FVector::UpVector;

	if (GCTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, OverlapCapsuleRadius, OverlapCapsuleHalfHeight, FQuat::Identity, CollisionProfilePawn, QueryParams, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	LedgeDescription.Location = OverlapLocation - DownwardCheckHitResult.Component->GetComponentLocation();
	LedgeDescription.Rotation = (ForwardCheckHitResult.ImpactNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator();
	LedgeDescription.LedgeNormal = ForwardCheckHitResult.ImpactNormal;
	LedgeDescription.LedgeImpactLocation = DownwardCheckHitResult.ImpactPoint;
	LedgeDescription.LedgePrimitiveComponent = DownwardCheckHitResult.GetComponent();

	return true;
}