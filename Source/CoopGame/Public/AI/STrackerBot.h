// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()
		
	/* represents bots mesh  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BotMesh;

	/*  health component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USHealthComponent* HealthComp;

	/** dynamic material  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UMaterialInstanceDynamic* DynamicMaterial;
	
	/* explode FX emitter template */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ExplodeFX;
	
	/** sphere collision  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereCollision;

	/** Sound to play when attack the player character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class USoundBase* SelfDestroySound;

	/** Sound to play when explodes  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class USoundBase* ExplodeSound;

	/** audio component  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* AudioComponent;
	
protected:

	// Sets default values for this pawn's properties
	ASTrackerBot();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** calls to find next destination point  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	FVector GetNextPathPoint();

	/** calls when bot has been killed or when it explodes to damage the player  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AAA")
	void OnSelfDestruct();
	virtual void OnSelfDestruct_Implementation();

private:

	/** calls to jump at random time  */
	UFUNCTION()
	void Jump();

	/** calls to update the NextPathPoint value  */
	UFUNCTION()
	void RefreshThePath();

	/** calls per timer to check whether we have some buddies at specified radius or not  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void CheckForBuddies();

	/** calls whenever sphere collision overlaps another actors  */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/** calls by timer to damage self when it overlaps the player character  */
	UFUNCTION()
	void SelfDamage();

	/** calls whenever health in health component is changed  */
	UFUNCTION()
	void OnHealthChange(USHealthComponent* SHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	/* the min power of impulse that throw up a bot when it jumps */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float ImpulseStrengthMin = 300.f;

	/* the max power of impulse that throw up a bot when it jumps */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float ImpulseStrengthMax = 900.f;

	/** jump timer  */
	UPROPERTY()
	FTimerHandle JumpTimer;

	/** self damage timer  */
	UPROPERTY()
	FTimerHandle SelfDamageTimer;

	/** Buddies finder timer  */
	UPROPERTY()
	FTimerHandle CheckForBuddiesTimer;

	/** timer that refreshes the path (useful if we stuck) */
	UPROPERTY()
	FTimerHandle RefreshPathTimer;

	/** shows whether bot exploded  or not  */
	uint32 bExploded : 1;

	/* detection radius */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float BuddiesDetectionRadius = 1000.f;

	/* damage multiplier (when buddies is in BuddiesDetectionRadius) */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float PowerLevel = 0.f;

	/* maximum damage multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float MaxPowerLevel = 5.f;

	/* radius of the minimum damage area, from Origin */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float ExplosionRadius = 500.f;

	/* The base damage to apply, i.e. the damage at the origin. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float ExplosionDamage = 80.f;

	/* damage type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UDamageType> DamageType;

	/* how fast (in sec) self damage timer ticks */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float SelfDamageRate = 0.25f;

	/* self damage per self damage timer tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float SelfDamagePerTimerTick = 20.f;

	/** next destination point  */
	UPROPERTY()
	FVector NextPathPoint;

	/** the strength of force to apply to this actor to move */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float MovementForce = 500.f;

	/** the distance to reach to set next path point */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float RequiredDistanceToTarget = 200.f;

	/** shows whether is velocity changed or not  */ 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bUseVelocityChange : 1;

	/** the power of glowing AI Bot material  */
	UPROPERTY(ReplicatedUsing = OnRep_Alpha)
	float Alpha;
	
	/** called whenever Alpha updated  */
	UFUNCTION()
	virtual void OnRep_Alpha();
};
