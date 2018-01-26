// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsFurtherThanXunits.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UBTD_IsFurtherThanXunits : public UBTDecorator
{
	GENERATED_BODY()

private:

	UBTD_IsFurtherThanXunits();

	/** player pawn blackboard variable  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	struct FBlackboardKeySelector PlayerPawn;

	/** distance to check from character to enemy  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float MaxDistance = 2000.f;

	/** calculates raw, core value of decorator's condition. Should not include calling IsInversed */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
};
