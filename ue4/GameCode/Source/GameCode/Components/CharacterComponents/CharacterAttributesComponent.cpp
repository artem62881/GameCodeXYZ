// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesComponent.h"
#include "Characters/GCBaseCharacter.h"
#include "Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/GCBaseCharacterMovementComponent.h"
#include "GameCodeTypes.h"
#include "Engine/EngineTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogAttributes, Display, Display)

// Sets default values for this component's properties
UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MaxHealth > 0.f, TEXT("UCharacterAttributesComponent::BeginPlay()  max health can not be equal to 0"));
	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UCharacterAttributesComponent::BeginPlay() can be used only with AGCBaseCharacter"));
	CachedBaseCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());
	CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributesComponent::OnTakeAnyDamage);
	Health = MaxHealth;
	Stamina = MaxStamina;
	Oxygen = MaxOxygen;
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UCharacterAttributesComponent::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}

	FVector HealthTextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + DebugTextOffset) * FVector::UpVector;
	DrawDebugString(GetWorld(), HealthTextLocation, FString::Printf(TEXT("Health: %.2f"), Health), nullptr, FColor::Green, 0.f, true);

	FVector StaminaTextLocation = HealthTextLocation + DebugLinesOffset * FVector::DownVector;
	DrawDebugString(GetWorld(), StaminaTextLocation, FString::Printf(TEXT("Stamina: %.2f"), Stamina), nullptr, FColor::Blue, 0.f, true);

	if (CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSwimming() || Oxygen < MaxOxygen)
	{
		FVector OxygenTextLocation = StaminaTextLocation + DebugLinesOffset * FVector::DownVector;
		DrawDebugString(GetWorld(), OxygenTextLocation, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen), nullptr, FColor::Cyan, 0.f, true);
	}
}
#endif

void UCharacterAttributesComponent::UpdateStaminaValue(float DeltaSeconds)
{
	if (CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting())
	{
		Stamina -= SprintStaminaConsumptionVelocity * DeltaSeconds;
		Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
	}
	else
	{
		Stamina += StaminaRestoreVelocity * DeltaSeconds;
		Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
	}

	if (Stamina == 0.0f)
	{
		if (OutOfStaminaEvent.IsBound())
		{
			OutOfStaminaEvent.Broadcast(true);
		}
	}
	if (Stamina == MaxStamina)
	{
		if (OutOfStaminaEvent.IsBound())
		{
			OutOfStaminaEvent.Broadcast(false);
		}
	}
}

void UCharacterAttributesComponent::UpdateOxygenValue(float DeltaTime)
{
	if (CachedBaseCharacterOwner->IsSwimmingUnderwater())
	{
		if (IsOutOfOxygen() && !GetWorld()->GetTimerManager().IsTimerActive(SwimDamageTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(SwimDamageTimer, this, &UCharacterAttributesComponent::ApplySuffocationDamage, OutOfOxygenDamageInterval, true);
		}

		Oxygen -= SwimOxygenConsumptionVelocity * DeltaTime;
		Oxygen = FMath::Clamp(Oxygen, 0.f, MaxOxygen);
	}
	else
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(SwimDamageTimer))
		{
			GetWorld()->GetTimerManager().ClearTimer(SwimDamageTimer);
		}
		Oxygen += OxygenRestoreVelocity * DeltaTime;
		Oxygen = FMath::Clamp(Oxygen, 0.f, MaxOxygen);
	}
}

void UCharacterAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}

	UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributesComponent::OnTakeAnyDamage %s recieved %.2f amount of damage from %s"), *CachedBaseCharacterOwner->GetName(), Damage, *DamageCauser->GetName());
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth); 

	if (Health <= 0.f)
	{
		UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributesComponent::OnTakeAnyDamage character %s is killed by: %s"), *CachedBaseCharacterOwner->GetName(), *DamageCauser->GetName());
		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}

void UCharacterAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateStaminaValue(DeltaTime);
	UpdateOxygenValue(DeltaTime);

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif
}

float UCharacterAttributesComponent::GetHealthPercent() const
{
	return Health/MaxHealth;
}

void UCharacterAttributesComponent::ApplySuffocationDamage()
{
	GetOwner()->TakeDamage(OutOfOxygenDamageAmount, FDamageEvent(), CachedBaseCharacterOwner->GetController(), CachedBaseCharacterOwner.Get());
}
