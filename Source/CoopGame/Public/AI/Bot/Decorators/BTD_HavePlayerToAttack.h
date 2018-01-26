// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BTD_HavePlayerToAttack.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UBTD_HavePlayerToAttack : public UBTDecorator_Blackboard
{
	GENERATED_BODY()
	
private:

	UBTD_HavePlayerToAttack();

};
