// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickup.generated.h"

UCLASS()
class COOPGAME_API ASPickup : public AActor
{
	GENERATED_BODY()
	
	/* sphere collision  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;
	
	/* decal component  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* DecalComponent;

	/* power up template  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ASPowerup> PowerupClass;
	
protected:

	// Sets default values for this actor's properties
	ASPickup();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** overlapping  */
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	/** calls to respawn the power up  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void Respawn();

private:

	/** respawn delay  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float CooldownDuration = 10.f;
	
	/** spawned power up reference  */
	UPROPERTY()
	ASPowerup* SpawnedSPowerup;
};
