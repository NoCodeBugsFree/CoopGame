// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SPowerup.generated.h"

UCLASS()
class COOPGAME_API ASPowerup : public AStaticMeshActor
{
	GENERATED_BODY()
	
	/* scene component  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* ROOT;

	/* point light  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UPointLightComponent* PointLightComponent;

	/* rotating movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class URotatingMovementComponent* RotationMovement;

public:	

	/** [Begin Play] calls to activate power up and start ticking  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void Activate(AActor* OtherActor);
	
protected:

	// Sets default values for this actor's properties
	ASPowerup();
	
	/** calls when power up is activated  to enable special ability */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AAA")
	void OnActivated(AActor* OtherActor);

	/** calls per power up tick  */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AAA")
	void OnPowerupTicked();

	/**  calls to disable power up effect */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AAA")
	void OnExpired();

	/**  calls whenever power up state is changed */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AAA")
	void OnPowerUpStateChanged(bool bActive);

private:
	
	/** called whenever bPowerUpActive updated  */
	UFUNCTION()
	virtual void OnRep_PowerUpActive();
	
	/** shows whether the power up is active or not  */
	UPROPERTY(ReplicatedUsing = OnRep_PowerUpActive, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bPowerUpActive : 1;

	/** power up tick  */
	UFUNCTION()
	void OnTickPowerup();

	int32 TickProcessed = 0;

	/** time between power up ticks  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float PowerupInterval = 5.f;

	/** total times we apply power up effect  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	int32 TotalNumberOfTicks = 1;

};
