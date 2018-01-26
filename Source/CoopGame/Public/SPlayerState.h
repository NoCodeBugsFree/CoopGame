// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	/** calls to update the score */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "AAA")
	void AddScore(float Amount);
	
	/** returns current score  **/
	FORCEINLINE float GetScore() const { return Score; }
	/**  returns Exact ping as float (rounded and compressed in replicated Ping) */
	float GetExactPing() const { return ExactPing; }
};
