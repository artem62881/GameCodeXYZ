// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Zipline.generated.h"

/**
 * 
 */
class UStaticMeshComponent;
class UCapsuleComponent;

UCLASS(Blueprintable)
class GAMECODE_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AZipline();

	virtual void OnConstruction(const FTransform& Transform) override;

	FVector GetZiplineAttachPoint(const ACharacter* Character) const;

	FVector GetZiplineVector() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* FirstPoleStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* SecondPoleStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* CableStaticMeshComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Zipline parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float PolesHeight = 350.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Zipline parameters", meta = (MakeEditWidget))
	FVector FirstPoleLocation = FVector(0.0f, 0.0f, PolesHeight * 0.5f);
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Zipline parameters", meta = (MakeEditWidget))
	FVector SecondPoleLocation = FVector(100.0f, 0.0f, PolesHeight * 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float InteractionCapsuleRadius = 100.0f;

	UCapsuleComponent* GetZiplineInteractionCapsule() const;
	
private:
	FVector CableVector = FVector::ZeroVector;

};
