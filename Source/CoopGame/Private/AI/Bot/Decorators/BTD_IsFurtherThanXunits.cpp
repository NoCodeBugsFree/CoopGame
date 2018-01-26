// Fill out your copyright notice in the Description page of Project Settings.

#include "BTD_IsFurtherThanXunits.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTD_IsFurtherThanXunits::UBTD_IsFurtherThanXunits()
{
	PlayerPawn.SelectedKeyName = "PlayerPawn";
	NodeName = FString("Is Further Than X units ???");
	FlowAbortMode = EBTFlowAbortMode::Self;
	SetIsInversed(false);
}

bool UBTD_IsFurtherThanXunits::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		APawn* AIPawn = AIController->GetPawn();
		if (AIPawn)
		{
			UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
			if (BB)
			{
				if (AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(PlayerPawn.SelectedKeyName)))
				{
					FVector AIPawnLocation = AIPawn->GetActorLocation();
					FVector TargetActorLocation = TargetActor->GetActorLocation();
					return (AIPawnLocation - TargetActorLocation).Size() > MaxDistance;
				}
			}
		}
	}
	return false;
}
