// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_EndHardLanding.h"
#include "Characters/GCBaseCharacter.h"

void UAnimNotify_EndHardLanding::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (IsValid(CharacterOwner))
	{
		CharacterOwner->EndHardLanding();
	}
}
