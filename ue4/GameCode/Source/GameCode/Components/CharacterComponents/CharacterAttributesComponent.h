// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOutOfStaminaEventSignature, bool);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterAttributesComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FOnDeathEventSignature OnDeathEvent;

	FOutOfStaminaEventSignature OutOfStaminaEvent;

	bool IsAlive() { return Health > 0.f; }

	bool IsOutOfOxygen() { return Oxygen == 0.f; }

	float GetHealthPercent() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug draw", meta = (UIMin = 0.f))
	float DebugTextOffset = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug draw", meta = (UIMin = 0.f))
	float DebugLinesOffset = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.f))
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (UIMin = 0.f))
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (UIMin = 0.f))
	float StaminaRestoreVelocity = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (UIMin = 0.f))
	float SprintStaminaConsumptionVelocity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float MaxOxygen = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float OxygenRestoreVelocity = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float SwimOxygenConsumptionVelocity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float OutOfOxygenDamageAmount = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.f))
	float OutOfOxygenDamageInterval = 1.0f;

private: 
	float Health = 0.f;
	float Stamina = 0.0f;
	float Oxygen = 0.f;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();
#endif

	void UpdateStaminaValue(float DeltaSeconds);

	void UpdateOxygenValue(float DeltaTime);

	void ApplySuffocationDamage();

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacterOwner;
	TWeakObjectPtr<class UGCBaseCharacterMovementComponent> CachedBaseCharacterMovement;

	FTimerHandle SwimDamageTimer;
};
