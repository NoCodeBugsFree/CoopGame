// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

UCLASS()
class COOPGAME_API APickupBase : public AActor
{
	GENERATED_BODY()

	/* scene component  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* ROOT;

	/* sphere collision  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;

	/* decal component  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* DecalComponent;
	
	/* point light  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UPointLightComponent* PointLightComponent;

	/* rotating movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class URotatingMovementComponent* RotationMovement;

	/* represents the mesh of the pick up */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* PickupMesh;

	/** sound to play when we pick up the pickup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class USoundBase* PickupUsedSound;

	/** sound to play when we pick up is respawned */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class USoundBase* PickupRespawnedSound;
	
protected:

	// Sets default values for this actor's properties
	APickupBase();

	/** calls when other actor begin overlaps this one  */
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	/** calls when other actor end overlaps this one  */
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	
	/** calls when pick up is activated for particular character */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AAA")
	void OnPickupActivated(AActor* Activator);
	virtual void  OnPickupActivated_Implementation(AActor* Activator);
	
private:

	/** calls to init all config variables */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void Initialize(class USoundBase* PickupUsedSoundToSet, class USoundBase* PickupRespawnedSoundToSet, float PickupRespawnDelayToSet, float PowerupDuration);

	/** actor that firstly overlaps this actor  */
	UPROPERTY()
	class AActor* OverlappedActor;
	
	/** calls whenever bActive updated  */
	UFUNCTION()
	virtual void OnRep_Activated();

	/** shows whether this pick up is activated (picked up actually)  or not  */
	UPROPERTY(ReplicatedUsing = OnRep_Activated, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bActivated : 1;

	/** delay to respawn the pickup since the time it was pickuped  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float PickupRespawnDelay = 10.f;

	/** shows the duration of power up (if this pickup intended to be a power up pickup)  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true", EditCondition = "bPowerUp"))
	float PowerupDuration = 5.f;
};
