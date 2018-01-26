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

void ASGameModeBase::StartWave()
{
	/** next wave begins  */
	WaveCount++;
	BotsToSpawn = 2 * WaveCount;

	UpdateGameState(WaveCount, BotsToSpawn);

	/** start spawning  */
	GetWorldTimerManager().SetTimer(SpawnBotTimer, this, &ASGameModeBase::SpawnBotTimerElapsed, SpawnBotDelay, true, 0.f);

	SetWaveState(EWaveState::EW_WaveInProgress);
}

void ASGameModeBase::EndWave()
{
	/** stop wave timer */
	GetWorldTimerManager().ClearTimer(NextWaveTimer);

	SetWaveState(EWaveState::EW_WaitingToComplete);
}

void ASGameModeBase::PrepareForNextWave()
{
	/** start wave */
	GetWorldTimerManager().SetTimer(NextWaveTimer, this, &ASGameModeBase::StartWave, NextWaveDelay, false);

	SetWaveState(EWaveState::EW_WaitingToStart);

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
	// ControllerIterator
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; It++)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(*It))
		{
			if (PlayerController->GetPawn() == nullptr)
			{
				///** Tries to spawn the player's pawn, at the location returned by FindPlayerStart */
				//UFUNCTION(BlueprintCallable, Category = Game)
				//virtual void RestartPlayer(AController* NewPlayer);
				RestartPlayer(PlayerController);
			}
		}
	}
}

void ASGameModeBase::OnSomeOneWasKilled(AActor* Victim, AActor* Killer, AController* KillerController)
{
	if (KillerController)
	{
		if (APawn* KillerPawn = KillerController->GetPawn())
		{
			if(ASPlayerState* SPlayerState = Cast<ASPlayerState>(KillerController->PlayerState))
			{
				SPlayerState->AddScore(20.f);
			}
		}
	}
}

void ASGameModeBase::CheckWaveState()
{
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
			
			/** AI alive found! */
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
		SetWaveState(EWaveState::EW_WaveComplete);
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

	SetWaveState(EWaveState::EW_GameOver);

	/** TODO  */
	UE_LOG(LogTemp, Error, TEXT("GameOver!"));
}

void ASGameModeBase::UpdateGameState(int32 WaveCount, int32 BotsToSpawn)
{
	if (SGameStateBase)
	{

	}
}

void ASGameModeBase::SpawnBotTimerElapsed()
{
	if (BotsToSpawn > 0)
	{
		/** [BlueprintImplementableEvent]  */
		SpawnNewBot();

		BotsToSpawn--;

		if (BotsToSpawn <= 0)
		{
			EndWave();
		}
	}
}
