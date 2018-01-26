// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_SelectTargetPlayer.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API UBTS_SelectTargetPlayer : public UBTService
{
	GENERATED_BODY()
	
private:

	UBTS_SelectTargetPlayer();
	
	/** update next tick interval this function should be considered as const (don't modify state of object) if node is not instanced! */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** player pawn to set BB value  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA", meta = (AllowPrivateAccess = "true"))
	struct FBlackboardKeySelector PlayerPawn;
		
};