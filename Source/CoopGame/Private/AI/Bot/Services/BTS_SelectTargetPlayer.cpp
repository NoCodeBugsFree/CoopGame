// Fill out your copyright notice in the Description page of Project Settings.

#include "BTS_SelectTargetPlayer.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Components/SHealthComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTS_SelectTargetPlayer::UBTS_SelectTargetPlayer()
{
	PlayerPawn.SelectedKeyName = "PlayerPawn";
	NodeName = FString("Set Target Player"); 
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	bCallTickOnSearchStart = true;
}

void UBTS_SelectTargetPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AActor* BestTargetActor = nullptr;
	float NearestTargetDistance = FLT_MAX;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		APawn* AIPawn = AIController->GetPawn();
		if (AIPawn)
		{
			UAIPerceptionComponent* AIPerceptionComponent = Cast<UAIPerceptionComponent>(AIPawn->GetComponentByClass(UAIPerceptionComponent::StaticClass()));
			if (AIPerceptionComponent)
			{
				TArray<AActor*> OutActors;
				AIPerceptionComponent->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), OutActors);

				for (AActor* TestActor : OutActors)
				{
					/** whether target actor is friendly for our AI or not ?  */
					bool bFriendly = USHealthComponent::IsFriendly(TestActor, AIPawn);

					/** whether target actor is dead or not ?  */
					bool bDead = true;
					USHealthComponent* HealthComp = Cast<USHealthComponent>(AIPawn->GetComponentByClass(USHealthComponent::StaticClass()));
					if (HealthComp)
					{
						bDead = HealthComp->IsDead();
					}

					/** if test actor is hostile and alive  */
					if ( ! bFriendly && ! bDead)
					{
						/** pick closest actor as best  */
						float Distance = (AIPawn->GetActorLocation() - TestActor->GetActorLocation()).Size();
						if (Distance < NearestTargetDistance)
						{
							NearestTargetDistance = Distance;
							BestTargetActor = TestActor;
						}
					}
				}

				if (OwnerComp.GetBlackboardComponent())
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsObject(PlayerPawn.SelectedKeyName, BestTargetActor);
				}
			}
		}
	}
}


