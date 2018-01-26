// Fill out your copyright notice in the Description page of Project Settings.

#include "EQC_HumanPlayers.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"

void UEQC_HumanPlayers::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	
	if (AActor* Actor = Cast<AActor>((QueryInstance.Owner).Get()))
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(Actor, ASCharacter::StaticClass(), OutActors);
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, OutActors);
	}
}