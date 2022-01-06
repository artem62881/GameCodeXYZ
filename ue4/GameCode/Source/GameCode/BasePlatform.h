// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Components/TimelineComponent.h"
#include "BasePlatform.generated.h"

UENUM(BlueprintType)
enum class EPlatformBehavior : uint8
{
	OnDemand = 0,
	Loop
};

UCLASS()
class GAMECODE_API ABasePlatform : public AActor
{
	GENERATED_BODY()

public:
	ABasePlatform();

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlatformMovement")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "PlatformMovement", meta = (MakeEditWidget))
	FVector EndLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlatformMovement", Transient)
	FVector StartLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* TimelineCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlatformMovement")
	float ForwardMovingRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlatformMovement")
	float BackwardMovingRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlatformMovement")
	EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlatformMovement")
	float LoopPlatformCooldownTime = 0.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	class APlatformInvocator* Invocator;

	UFUNCTION(BlueprintCallable)
	void StartPlatformMovement();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlatformMovement")
	bool bIsLoopPlatformTimerOn = false;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	FTimerHandle LoopPlatformCooldownTimer;

	FTimeline PlatformTimeline;
	void PlatformTimelineUpdate(float Alpha);
	void OnTimelineFinished();	

	bool bIsPlatformAtFinish = false;

	void OnPlatformInvoked();
};
