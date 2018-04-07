// Fill out your copyright notice in the Description page of Project Settings.

#include "SAICharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SHealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "ConstructorHelpers.h"
#include "BehaviorTree/BehaviorTree.h"
#include "SAIController.h"

ASAICharacter::ASAICharacter()
{
	/*  Camera Boom */
	CameraBoom->TargetArmLength = 0.f;
	CameraBoom->SetRelativeLocation(FVector::ZeroVector);

	/* TP camera  */
	TP_Camera->SetRelativeLocation(FVector(0.f, 0.f, 70.f));

	/*  Health component */
	HealthComp->SetTeamNumber(255);

	/** AI Perception Component  */
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	
	/** sight sense config template to our AI  */
	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	Sight->SightRadius = 2000.f;
	Sight->LoseSightRadius = 2500.f;
	Sight->PeripheralVisionAngleDegrees = 60.f;
	Sight->SetMaxAge(10.f);

	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComponent->ConfigureSense(*Sight);
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	/** set the behavior tree */
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BehaviorTreeTemplate(TEXT("/Game/AdvancedAI/BT_AdvancedBotAI"));
	if (BehaviorTreeTemplate.Object)
	{
		BehaviorTree = BehaviorTreeTemplate.Object;
	}

	AIControllerClass = ASAIController::StaticClass();
}

void ASAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ASAICharacter::OnPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ASAICharacter::OnTargetPerceptionUpdated);
}

void ASAICharacter::OnPerceptionUpdated(TArray<AActor*> UpdatedActors)
{
	
}

void ASAICharacter::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	
}
