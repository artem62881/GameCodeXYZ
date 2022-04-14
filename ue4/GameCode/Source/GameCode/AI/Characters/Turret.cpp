// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"

#include "AIController.h"
#include "AI/Controllers/AITurretController.h"
#include "Components/Weapon/WeaponBarellComponent.h"
#include "Kismet/GameplayStatics.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
	SetRootComponent(TurretRoot);

	TurretBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBase"));
	TurretBaseComponent->SetupAttachment(TurretRoot);

	TurretBarellComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBarell"));
	TurretBarellComponent->SetupAttachment(TurretBaseComponent);

	WeaponBarell = CreateDefaultSubobject<UWeaponBarellComponent>(TEXT("WeaponBarell"));
	WeaponBarell->SetupAttachment(TurretBarellComponent);	
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();
	OnTakeAnyDamage.AddDynamic(this, &ATurret::OnTakeAnyDamageEvent);
	OnDestroyedEvent.AddDynamic(this, &ATurret::OnDestroyed);
	Health = MaxHealth;
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	switch (CurrentTurretState)
	{
		case ETurretState::Searching:
			{
				SearchingMovement(DeltaTime);
				break;
			}
		case ETurretState::Firing:
			{
				FiringMovement(DeltaTime);
				break;
			}
		case ETurretState::Destroyed:
			{
				break;
			}
		default:
			break;
	}
}

void ATurret::SetCurrentTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
	ETurretState NewState = IsValid(CurrentTarget) ? ETurretState::Firing : ETurretState::Searching;
	SetCurrentTurretState(NewState);
}

FVector ATurret::GetPawnViewLocation() const
{
	return WeaponBarell->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{
	return WeaponBarell->GetComponentRotation();
}

void ATurret::OnTakeAnyDamageEvent(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (IsDestroyed())
	{
		return;
	}

	UE_LOG(LogDamage, Warning, TEXT("ATurret::OnTakeAnyDamage %s recieved %.2f amount of damage from %s"), *GetName(), Damage, *DamageCauser->GetName());
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth); 
	
	if (Health <= 0.f)
	{
		UE_LOG(LogDamage, Warning, TEXT("ATurret::OnTakeAnyDamage character %s is killed by: %s"), *GetName(), *DamageCauser->GetName());
		if (OnDestroyedEvent.IsBound())
		{
			OnDestroyedEvent.Broadcast();
		}
	}
	//AGCAIController* Controller = StaticCast<AGCAIController*>(GetController());
	//TakeDamage(Damage, FDamageEvent(), GetController(), DamageCauser);
}

void ATurret::SearchingMovement(float DeltaTime)
{
	FRotator TurretBaseRotation = TurretBaseComponent->GetRelativeRotation();
	TurretBaseRotation.Yaw += BaseSearchingRotationRate * DeltaTime;
	TurretBaseComponent->SetRelativeRotation(TurretBaseRotation);

	FRotator TurretBarellRotation = TurretBarellComponent->GetRelativeRotation();
	TurretBarellRotation.Pitch = FMath::FInterpTo(TurretBarellRotation.Pitch, 0.f, DeltaTime, BarellPitchRotationRate);
	TurretBarellComponent->SetRelativeRotation(TurretBarellRotation);
}

void ATurret::FiringMovement(float DeltaTime)
{
	FVector BaseLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBaseComponent->GetComponentLocation()).GetSafeNormal2D();
	FQuat LookAtQuat = BaseLookAtDirection.ToOrientationQuat();
	FQuat TargetQuat = FMath::QInterpTo(TurretBaseComponent->GetComponentQuat(), LookAtQuat, DeltaTime, BaseFiringInterpSpeed);
	TurretBaseComponent->SetWorldRotation(TargetQuat);

	FVector BarellLookAtDirection = (CurrentTarget->GetActorLocation() - TurretBarellComponent->GetComponentLocation()).GetSafeNormal();
	float BarellLookAtPitchAngle = BarellLookAtDirection.ToOrientationRotator().Pitch;
	
	FRotator BarellLocalRotation = TurretBarellComponent->GetRelativeRotation();
	BarellLocalRotation.Pitch = FMath::FInterpTo(BarellLocalRotation.Pitch, BarellLookAtPitchAngle, DeltaTime, BarellPitchRotationRate);
	TurretBarellComponent->SetRelativeRotation(BarellLocalRotation);
}

void ATurret::MakeShot()
{
	FVector ShotLocation = WeaponBarell->GetComponentLocation();
	FVector ShotDirection = WeaponBarell->GetComponentRotation().RotateVector(FVector::ForwardVector);
	WeaponBarell->Shot(ShotLocation, ShotDirection, GetController());
}

void ATurret::SetCurrentTurretState(ETurretState NewState)
{
	bool bIsStateChanged = NewState != CurrentTurretState ;
	CurrentTurretState = NewState;

	if (!bIsStateChanged)
	{
		return;
	}

	switch (CurrentTurretState)
	{
	case ETurretState::Searching:
		{
			GetWorld()->GetTimerManager().ClearTimer(ShotTimer);
			break;
		}
	case ETurretState::Firing:
		{
			GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ATurret::MakeShot, GetFireInterval(), true, FireDelayTime);
			break;
		}
	case ETurretState::Destroyed:
		{
			GetWorld()->GetTimerManager().ClearTimer(ShotTimer);
			break;
		}
	}
}

float ATurret::GetFireInterval() const
{
	return  60.f / RateOfFire;
}

void ATurret::OnDestroyed()
{
	//SpawnEmitter
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyFX, GetActorTransform());
	SetCurrentTurretState(ETurretState::Destroyed);
	GetController()->Destroy();
	UE_LOG(LogDamage, Warning, TEXT("ATurret::OnTakeAnyDamage character %s is killed"), *GetName());
}


