// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "SCharacter.h"
#include "SGameModeBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicated(true);
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// [Server]
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::OnOwnerTakeDamage);
		}
	}

	Health = DefaultHealth;
}

void USHealthComponent::OnOwnerTakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// [Server]
	if (GetOwnerRole() == ROLE_Authority)
	{
		/** damage is positive and owner is alive  */
		if (Damage > 0.f && !bDead)
		{
			/** apply damage to non-friendly actors only  */
			if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser->GetOwner()))
			{
				return;
			}

			/** update Health  */
			Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);

			/** update bDead  */
			bDead = Health <= 0.f;

			OnHealthChange.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

			/** broadcast on actor killed  */
			if (Health <= 0.f)
			{
				if (ASGameModeBase* SGameModeBase = Cast<ASGameModeBase>(GetWorld()->GetAuthGameMode()))
				{
					SGameModeBase->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
				}
			}
		}
	}
}

void USHealthComponent::Heal(float Amount)
{
	/** do not heal dead owner, do not do anything with negative heal amount  */
	if (Health <= 0.f || Amount <= 0.f || GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	
	/** heal!  */
	Health = FMath::Clamp(Health + Amount, 0.f, DefaultHealth);
}

void USHealthComponent::SetTeamNumber(uint8 TeamNumberToSet)
{
	// [Server]
	if (GetOwnerRole() == ROLE_Authority)
	{
		TeamNumber = TeamNumberToSet;
	}
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		/** assume friendly */
		return true;
	}
	
	USHealthComponent* Comp_A = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* Comp_B = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if (Comp_A == nullptr || Comp_B == nullptr)
	{
		/** assume friendly  */
		return true;
	}
	
	return Comp_A->TeamNumber == Comp_B->TeamNumber;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChange.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

/** Returns properties that are replicated for the lifetime of the actor channel */
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
	DOREPLIFETIME(USHealthComponent, TeamNumber);
}