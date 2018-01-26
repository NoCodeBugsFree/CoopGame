// Fill out your copyright notice in the Description page of Project Settings.

#include "BTD_HavePlayerToAttack.h"

UBTD_HavePlayerToAttack::UBTD_HavePlayerToAttack()
{	
	BlackboardKey.SelectedKeyName = "PlayerPawn";
	NodeName = FString("I have a player to attack!");
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;
}
