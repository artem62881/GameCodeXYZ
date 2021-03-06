// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "AnimNotify_EndSlide.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UAnimNotify_EndSlide : public UAnimNotify
{
	GENERATED_BODY()
private:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
};
