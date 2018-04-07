// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameModeBase.h"
#include "Components/SHealthComponent.h"
#include "SCharacter.h"
#include "SGameStateBase.h"
#include "SPlayerState.h"
#include "ConstructorHelpers.h"

ASGameModeBase::ASGameModeBase()
{
	/** tick per second  */
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;

	PlayerStateClass = ASPlayerState::StaticClass();
	GameStateClass = ASGameStateBase::StaticClass();

#pragma region ConstructorHelpers

	/** set pawn class */
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/BP_SCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
#pragma endregion

}

void ASGameModeBase::InitGameState()
{
	Super::InitGameState();

	/** set game state reference  */
	if(ASGameStateBase* SGameStateBaseTest = Cast<ASGameStateBase>(GameState))
	{
		SGameStateBase = SGameStateBaseTest;

		UpdateGameState(WaveCount, BotsToSpawn);
	}
}

void ASGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	OnActorKilled.AddDynamic(this, &ASGameModeBase::OnSomeOneWasKilled);
}

void ASGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();

	CheckAnyPlayerAlive();
}

void ASGameModeBase::StartPlay()
{
	Super::StartPlay();
}

void ASGameModeBase::StartWave()
{
	/** next wave begins  */
	WaveCount++;

	/** twice previous bots amount  */
	BotsToSpawn = 2 * WaveCount;

	UpdateGameState(WaveCount, BotsToSpawn);

	/** start spawning  */
	GetWorldTimerManager().SetTimer(SpawnBotTimer, this, &ASGameModeBase::SpawnBotTimerElapsed, SpawnBotDelay, true, 0.f);

	SetWaveState(EWaveState::WS_WaveInProgress);
}

void ASGameModeBase::EndWave()
{
	/** stop wave timer */
	GetWorldTimerManager().ClearTimer(NextWaveTimer);

	/** no longer spawning, waiting for players to kill remaining bots  */
	SetWaveState(EWaveState::WS_WaitingToComplete);
}

void ASGameModeBase::PrepareForNextWave()
{
	/** start wave by Next Wave Timer */
	GetWorldTimerManager().SetTimer(NextWaveTimer, this, &ASGameModeBase::StartWave, NextWaveDelay, false);
	
	SetWaveState(EWaveState::WS_WaitingToStart);

	RespawnDeadPlayers();
}

void ASGameModeBase::SetWaveState(EWaveState NewWaveState)
{
	ASGameStateBase* SGameStateBase = GetGameState<ASGameStateBase>();
	if (!ensure(SGameStateBase)) { return; }

	SGameStateBase->SetWaveState(NewWaveState);
}

void ASGameModeBase::RespawnDeadPlayers()
{
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; It++)
	{
		/** for each valid PC in the world  */
		if (APlayerController* PlayerController = Cast<APlayerController>(*It))
		{
			/** which has null pawn (dead) - respawn the pawn for this controller  */
			if (PlayerController->GetPawn() == nullptr)
			{
				/** Tries to spawn the player's pawn, at the location returned by FindPlayerStart */
				// UFUNCTION(BlueprintCallable, Category = Game)
				// virtual void RestartPlayer(AController* NewPlayer);
				RestartPlayer(PlayerController);
			}
		}
	}
}

void ASGameModeBase::OnSomeOneWasKilled(AActor* Victim, AActor* Killer, AController* KillerController)
{
	/** add reward to killers score from victim data   */
	if (KillerController)
	{
		if (APawn* KillerPawn = KillerController->GetPawn())
		{
			if(ASPlayerState* SPlayerState = Cast<ASPlayerState>(KillerController->PlayerState))
			{
				SPlayerState->AddScore(20.f); // TODO victim must have reward data
			}
		}
	}
}

void ASGameModeBase::CheckWaveState()
{
	/** does we already start a next wave timer (preparing for next wave) ?  */
	bool bPreparingForWave = GetWorldTimerManager().IsTimerActive(NextWaveTimer);

	/** we definitely has bot to spawn or preparing for next wave  */
	if (BotsToSpawn > 0 || bPreparingForWave)
	{
		return;
	}

	/** try to find any alive bot  */
	bool bAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{	
		if(APawn* Pawn = It->Get())
		{
			/** if current pawn is not AI - skip it */
			if (Pawn == nullptr || Pawn->IsPlayerControlled())
			{
				continue;
			}

			USHealthComponent* HealthComp = Cast<USHealthComponent>(Pawn->GetComponentByClass(USHealthComponent::StaticClass()));
			
			/** alive AI has been found! */
			if (HealthComp && HealthComp->GetHealth() > 0.f)
			{
				bAnyBotAlive = true;
				break;
			}
		}
	}

	/** all bots died - prepare for the next wave  */
	if (!bAnyBotAlive)
	{
		SetWaveState(EWaveState::WS_WaveComplete);
		PrepareForNextWave();
	}
}

void ASGameModeBase::CheckAnyPlayerAlive()
{
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; It++)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(*It))
		{
			if (APawn* Pawn = PlayerController->GetPawn())
			{
				if (Pawn)
				{
					if(USHealthComponent* HealthComponent = Cast<USHealthComponent>(Pawn->GetComponentByClass(USHealthComponent::StaticClass())))
					{
						if (HealthComponent && HealthComponent->GetHealth() > 0)
						{
							/** at least one alive  */
							return;
						}
					}
				}
			}
		}
	}

	/** all players are dead ! */
	GameOver();
}

void ASGameModeBase::GameOver()
{
	EndWave();

	SetWaveState(EWaveState::WS_GameOver);
}

void ASGameModeBase::UpdateGameState(int32 WaveCount, int32 BotsToSpawn)
{
	if (SGameStateBase)
	{
		// TODO
	}
}

void ASGameModeBase::SpawnBotTimerElapsed()
{
	if (BotsToSpawn > 0)
	{
		/** [BlueprintImplementableEvent] EQS query to find desired spawn location */
		SpawnNewBot();

		/** reduce remnant bots amount  */
		BotsToSpawn--;

		/** if all bots was spawn  */
		if (BotsToSpawn <= 0)
		{
			/** stop spawning and waiting until players will killed all bots or vice versa  */
			EndWave();
		}
	}
}
