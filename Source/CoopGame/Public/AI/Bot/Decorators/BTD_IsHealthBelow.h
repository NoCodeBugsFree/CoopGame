// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsHealthBelow.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UBTD_IsHealthBelow : public UBTDecorator
{
	GENERATED_BODY()
	
private:

	UBTD_IsHealthBelow();
	
	/** health to hide from character to heal self */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float HealthToHide = 30.f;

	/** calculates raw, core value of decorator's condition. Should not include calling IsInversed */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
};
