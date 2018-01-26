// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SCharacter.h"
#include "Perception/AIPerceptionTypes.h"
#include "SAICharacter.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASAICharacter : public ASCharacter
{
	GENERATED_BODY()
	
	/** AI Perception Component  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionComponent* AIPerceptionComponent;

	/** sight sense config template to our AI  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Sight* Sight;

	/** Behavior Tree  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

private:

	ASAICharacter();

	virtual void PostInitializeComponents() override;

	/** calls when AI Perception Component updated */
	UFUNCTION()
	void OnPerceptionUpdated(TArray<AActor*> UpdatedActors);
	
	/** calls when AI Perception Component updated */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);		
	
public:

	/** returns behavior tree reference **/
	FORCEINLINE class UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
};
