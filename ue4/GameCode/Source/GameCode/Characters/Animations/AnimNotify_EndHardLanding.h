// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EndHardLanding.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UAnimNotify_EndHardLanding : public UAnimNotify
{
	GENERATED_BODY()
private:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
};
