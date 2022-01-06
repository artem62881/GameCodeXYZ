// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Ladder.generated.h"

class UAnimMontage;
class UBoxComponent;
class UStaticMeshComponent;
/**
 * 
 */
UCLASS(Blueprintable)
class GAMECODE_API ALadder : public AInteractiveActor
{
	GENERATED_BODY()
	
public:
	ALadder();

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	float GetLadderHeight() const;

	bool GetIsOnTop() const;

	UAnimMontage* GetAttachFromTopAnimMontage() const;

	FVector GetAttachFromTopAnimMontageStartingLocation() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LadderHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LadderWidth = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float StepsInterval = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float BottomStepOffset = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float BoxDepthExtent = 16.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* RightRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LeftRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* StepsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* TopInteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder parameters")
	UAnimMontage* AttachFromTopAnimMontage;

	//Offset from ladder's top
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder parameters")
	FVector AttachFromTopAnimMontageInitialOffset = FVector::ZeroVector;

	UBoxComponent* GetLadderInteractionBox() const;

	virtual void OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

private:
	bool bIsOnTop = false;
};
