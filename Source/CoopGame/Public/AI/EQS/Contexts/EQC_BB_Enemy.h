// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EQC_BB_Enemy.generated.h"

/**
 * returns the enemy pawn from blackboard
 */
UCLASS()
class COOPGAME_API UEQC_BB_Enemy : public UEnvQueryContext
{
	GENERATED_BODY()

protected:
	
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

};
