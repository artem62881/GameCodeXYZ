// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlatform.h"
#include "Components/SceneComponent.h"
#include "PlatformInvocator.h"
#include "Math/UnrealMathVectorCommon.h"

ABasePlatform::ABasePlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Platform root"));
	RootComponent = DefaultPlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformMesh->SetupAttachment(DefaultPlatformRoot);
}

void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = PlatformMesh->GetRelativeLocation();
	if (IsValid(Invocator))
	{
		Invocator->OnInvocatorActivated.AddUObject(this, &ABasePlatform::OnPlatformInvoked);
	}
	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic PlatformMovementTimelineUpdate;
		PlatformMovementTimelineUpdate.BindUObject(this, &ABasePlatform::PlatformTimelineUpdate);
		PlatformTimeline.AddInterpFloat(TimelineCurve, PlatformMovementTimelineUpdate);

		FOnTimelineEventStatic PlatformTimelineFinishedCallback;
		PlatformTimelineFinishedCallback.BindUObject(this, &ABasePlatform::OnTimelineFinished);
		PlatformTimeline.SetTimelineFinishedFunc(PlatformTimelineFinishedCallback);
	}
}

void ABasePlatform::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlatformTimeline.TickTimeline(DeltaTime);
}

void ABasePlatform::PlatformTimelineUpdate(const float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	PlatformMesh->SetRelativeLocation(PlatformTargetLocation);
}
void ABasePlatform::OnTimelineFinished()
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("void ABasePlatform::OnTimelineFinished()")));
	bIsPlatformAtFinish = !bIsPlatformAtFinish;
	if (PlatformBehavior == EPlatformBehavior::Loop)
	{
		if (bIsLoopPlatformTimerOn && LoopPlatformCooldownTime != 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(LoopPlatformCooldownTimer, this, &ABasePlatform::StartPlatformMovement, LoopPlatformCooldownTime, false);
		}
		else
		{
			StartPlatformMovement();
		}
	}
}

void ABasePlatform::StartPlatformMovement()
{
	if (!bIsPlatformAtFinish)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("void ABasePlatform forward")));
		PlatformTimeline.SetPlayRate(ForwardMovingRate);
		PlatformTimeline.Play();
	}
	if (bIsPlatformAtFinish)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("void ABasePlatform backward")));
		PlatformTimeline.SetPlayRate(BackwardMovingRate);
		PlatformTimeline.Reverse();
	}
}

void ABasePlatform::OnPlatformInvoked()
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("void ABasePlatform::OnPlatformInvoked()")));
	StartPlatformMovement();
}