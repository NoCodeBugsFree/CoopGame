// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameStateBase.h"
#include "SGameModeBase.generated.h"



/** broadcasts when player kill someone  */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, Victim, AActor*, Killer, AController*, KillerController);

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	/** broadcasts when player kill someone  */
	UPROPERTY(BlueprintAssignable, Category = "AAA")
	FOnActorKilled OnActorKilled;
	
protected:

	ASGameModeBase();

	/**
	* Initialize the GameState actor with default settings
	* called during PreInitializeComponents() of the GameMode after a GameState has been spawned
	* as well as during Reset()
	*/
	virtual void InitGameState() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	/** calls to spawn a single bot  */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AAA")
	void SpawnNewBot();
	
	/** calls to attempt to spawn another bot, or if all required bots was spawned - end this wave  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void SpawnBotTimerElapsed();

	/** calls to calc bots to spawn and start spawning required amount of bots */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void StartWave();

	/** calls to finish this wave */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void EndWave();

	/** calls to start new wave after NextWaveDelay secs */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void PrepareForNextWave();

	/** calls to set the new wave state in Game State */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void SetWaveState(EWaveState NewWaveState);

	/** calls to respawn all dead players before we start a new wave */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void RespawnDeadPlayers();

private:
	
	/** calls to update replicated variables in game state  */
	UFUNCTION()
	void UpdateGameState(int32 WaveCount, int32 BotsToSpawn);

	/** calls to add a score to killer's score  */
	UFUNCTION()
	void OnSomeOneWasKilled(AActor* Victim, AActor* Killer, AController* KillerController);

	/** [tick] calls to check wave state and start a new wave spawning if required  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void CheckWaveState();

	/** [tick] calls to check wave state and start a new wave spawning if required  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void CheckAnyPlayerAlive();

	/** calls to move the game to game over state  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void GameOver();

	/** timer that handle bots spawning  */
	UPROPERTY()
	FTimerHandle SpawnBotTimer;

	/** timer that handle waves spawning  */
	UPROPERTY()
	FTimerHandle NextWaveTimer;

	/** bots to spawn this wave  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	int32 BotsToSpawn = 0;

	/** current wave number  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	int32 WaveCount = 0;
	
	/** delay between spawning bots (during one wave) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float SpawnBotDelay = 1.f;

	/** delay between spawning waves  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float NextWaveDelay = 5.f;
	
	/** game state reference  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class ASGameStateBase* SGameStateBase;
	
};
