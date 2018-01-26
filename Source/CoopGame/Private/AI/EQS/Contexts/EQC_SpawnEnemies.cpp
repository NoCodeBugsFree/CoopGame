// Fill out your copyright notice in the Description page of Project Settings.

#include "EQC_SpawnEnemies.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"

void UEQC_SpawnEnemies::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	if (AActor* Actor = Cast<AActor>((QueryInstance.Owner).Get()))
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(Actor, ATargetPoint::StaticClass(), OutActors);
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, OutActors);
	}
}
