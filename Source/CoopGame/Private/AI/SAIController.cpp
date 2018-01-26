// Fill out your copyright notice in the Description page of Project Settings.

#include "SAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "SAICharacter.h"

ASAIController::ASAIController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ASAIController::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);
	
	if(ASAICharacter* SAICharacter = Cast<ASAICharacter>(Pawn))
	{
		UBehaviorTree* BehaviorTree = SAICharacter->GetBehaviorTree();
		if (Pawn && BehaviorTree && BehaviorTree->BlackboardAsset)
		{
			BlackboardComponent->InitializeBlackboard(*(BehaviorTree->BlackboardAsset));
			BehaviorTreeComponent->StartTree(*BehaviorTree);
		}
	}
}
