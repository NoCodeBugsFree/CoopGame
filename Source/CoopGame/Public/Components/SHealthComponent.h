// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChange, USHealthComponent*, SHealthComponent, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	/** on health change delegate  */
	UPROPERTY(BlueprintAssignable, Category = "EventDispatcher")
	FOnHealthChange OnHealthChange;

	/** calls to heal the owner  */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "AAA")
	void Heal(float Amount);

	/** calls to set the number of owner team (player or AI) */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "AAA")
	void SetTeamNumber(uint8 TeamNumberToSet);

	/** calls to check whether we the same team or not  */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AAA")
	static bool IsFriendly(AActor* ActorA, AActor* ActorB);

protected:

	// Sets default values for this component's properties
	USHealthComponent();

	// Called when the game starts
	virtual void BeginPlay() override;

private:

	/** calls whenever owner takes damage  */
	UFUNCTION()
	void OnOwnerTakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	/** calls whenever health value is changed  */
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	/** the current health amount  */
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float Health;

	/** the default health amount  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float DefaultHealth = 100.f;

	/** shows whether owner is dead or not  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bDead : 1;

	/** represents the team of the owner (players or AI team) */
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint8 TeamNumber = 255;

public:
	
	/** returns current health value **/
	FORCEINLINE float GetHealth() const { return Health; }
	/** returns owner is dead or not  **/
	FORCEINLINE bool IsDead() const { return bDead; }
	/** returns default health value  **/
	FORCEINLINE float GetDefaultHealth() const { return DefaultHealth; }
};
