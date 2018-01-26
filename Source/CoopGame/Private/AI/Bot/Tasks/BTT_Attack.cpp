// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_Attack.h"
#include "SCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_Attack::UBTT_Attack()
{
	NodeName = FString("Attack");
	LastFireTime.SelectedKeyName = "LastFireTime";
}

EBTNodeResult::Type UBTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		if(ASCharacter* SCharacter = Cast<ASCharacter>(AIController->GetPawn()))
		{
			float FiringTime = FMath::RandRange(AIFireDelayMin, AIFireDelayMax);
			SCharacter->StartFireAI(FiringTime);

			if (OwnerComp.GetBlackboardComponent())
			{
				OwnerComp.GetBlackboardComponent()->SetValueAsFloat(LastFireTime.SelectedKeyName, FiringTime);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}


