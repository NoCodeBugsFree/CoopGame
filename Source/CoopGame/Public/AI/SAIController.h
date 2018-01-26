// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SAIController.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASAIController : public AAIController
{
	GENERATED_BODY()

	/** Blackboard Component  */
	UPROPERTY(Transient)
	class UBlackboardComponent* BlackboardComponent;

	/** BehaviorTree Component  */
	UPROPERTY(Transient)
	class UBehaviorTreeComponent* BehaviorTreeComponent;

protected:

	ASAIController();

	virtual void Possess(APawn* Pawn) override;

public:
	
	FORCEINLINE class UBlackboardComponent* GetBBComp() const { return BlackboardComponent; }

};

