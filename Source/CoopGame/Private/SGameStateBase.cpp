// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameStateBase.h"
#include "Net/UnrealNetwork.h"

void ASGameStateBase::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChange(WaveState, OldState);
}

void ASGameStateBase::SetWaveState(EWaveState NewWaveState)
{
	if (Role == ROLE_Authority)
	{
		EWaveState OldState = WaveState;
		WaveState = NewWaveState;

		/** server  */
		OnRep_WaveState(OldState);
	}
}

void ASGameStateBase::WaveStateChange_Implementation(EWaveState NewState, EWaveState OldState)
{

}

/** Returns properties that are replicated for the lifetime of the actor channel */
void ASGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameStateBase, WaveState);
	DOREPLIFETIME(ASGameStateBase, BotsToSpawn);
	DOREPLIFETIME(ASGameStateBase, WaveCount);
}