// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Barrel.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ABarrel : public AStaticMeshActor
{
	GENERATED_BODY()

	/* explode emitter template */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ExplodeFX;

	/*  Health component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USHealthComponent* HealthComp;

	/* explode material */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* ExplodedMaterial;

	/* Radial Force Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class URadialForceComponent* ExplosionForce;
	
protected:
	
	ABarrel();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AAA")
	void OnExplode();
	virtual void OnExplode_Implementation();

private:
	
	/** Called whenever bExploded updated  */
	UFUNCTION()
	virtual void OnRep_Exploded();

	/** shows whether the barrel is exploded or not  */
	UPROPERTY(ReplicatedUsing = OnRep_Exploded, VisibleAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bExploded : 1;
	
	/** calls whenever health in health component is changed  */
	UFUNCTION()
	void OnHealthChange(USHealthComponent* SHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/* the power of impulse that throw up a barrel when it explodes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float ImpulseStrength = 300.f;

	/* explosion radius */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float ExplodeRadius = 500.f;

	/* damage to cause */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float BaseDamage = 200.f;

	/* damage type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UDamageType> DamageType;
};
