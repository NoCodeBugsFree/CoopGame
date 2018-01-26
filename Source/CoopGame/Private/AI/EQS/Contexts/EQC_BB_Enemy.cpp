// Fill out your copyright notice in the Description page of Project Settings.

#include "EQC_BB_Enemy.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SAIController.h"


void UEQC_BB_Enemy::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	if(ASAIController* SAIController = Cast<ASAIController>((Cast<AActor>((QueryInstance.Owner).Get())->GetInstigatorController())))
	{
		UBlackboardComponent* BB = SAIController->GetBBComp();
		if (BB)
		{
			AActor* PlayerPawnFromBB = Cast<AActor>(BB->GetValueAsObject(TEXT("PlayerPawn")));
			if (PlayerPawnFromBB)
			{
				UEnvQueryItemType_Actor::SetContextHelper(ContextData, PlayerPawnFromBB);
			}
		}
	}
}
