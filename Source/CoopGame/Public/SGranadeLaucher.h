// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGranadeLaucher.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGranadeLaucher : public ASWeapon
{
	GENERATED_BODY()
	
public:

	ASGranadeLaucher();

protected:

	/** calls to fire a singe shot  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	virtual void Fire() override;

private:
	
	/** projectile template  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AProjectile> ProjectileTemplate;
};
