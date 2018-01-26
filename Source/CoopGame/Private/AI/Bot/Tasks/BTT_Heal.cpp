// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_Heal.h"
#include "AIController.h"
#include "SHealthComponent.h"

UBTT_Heal::UBTT_Heal()
{
	NodeName = FString("Heal");
}

EBTNodeResult::Type UBTT_Heal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		if(APawn* AIPawn = Cast<APawn>(AIController->GetPawn()))
		{
			USHealthComponent* HealthComp = Cast<USHealthComponent>(AIPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (HealthComp)
			{
				HealthComp->Heal(HealAmount);
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}


