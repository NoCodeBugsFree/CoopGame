 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameStateBase.generated.h"


/** describes wave state  */
UENUM(BlueprintType)
enum class EWaveState : uint8
{
	EW_WaitingToStart			UMETA(DisplayName = "WaitingToStart"),

	EW_WaveInProgress			UMETA(DisplayName = "WaveInProgress"),

	/** no longer spawning, waiting for players to kill remaining bots  */
	EW_WaitingToComplete		UMETA(DisplayName = "WaitingToComplete"),

	EW_GameOver					UMETA(DisplayName = "GameOver"),

	EW_WaveComplete				UMETA(DisplayName = "WaveComplete")
};

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:

	/** bots to spawn this wave  */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	int32 BotsToSpawn = 0;

	/** current wave number  */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	int32 WaveCount = 0;
	
	/** current wave state  */
	UPROPERTY(ReplicatedUsing = OnRep_WaveState, BlueprintReadOnly, Category = "Config")
	EWaveState WaveState;

	/** calls whenever WaveState updated  */
	UFUNCTION()
	virtual void OnRep_WaveState(EWaveState OldState);

	/** calls to update the wave state */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "AAA")
	void SetWaveState(EWaveState NewWaveState);

protected:
	
	/** calls to update the HUD data when wave state was changed */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AAA")
	void WaveStateChange(EWaveState NewState, EWaveState OldState);
	virtual void WaveStateChange_Implementation(EWaveState NewState, EWaveState OldState);
};
