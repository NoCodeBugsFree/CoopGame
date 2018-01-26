// Fill out your copyright notice in the Description page of Project Settings.

#include "BTD_IsHealthBelow.h"
#include "Components/SHealthComponent.h"
#include "AIController.h"

UBTD_IsHealthBelow::UBTD_IsHealthBelow()
{
	NodeName = FString("Should I hide to heal ?");
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;
}

bool UBTD_IsHealthBelow::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		APawn* AIPawn = AIController->GetPawn();
		if (AIPawn)
		{
			USHealthComponent* HealthComp = Cast<USHealthComponent>(AIPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (HealthComp)
			{
				return HealthComp->GetHealth() <= HealthToHide;
			}
		}
	}
	return false;
}
