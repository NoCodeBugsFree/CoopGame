// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SAnimInstance.generated.h"

/** attaching left arm to weapon when "blendspacing"
1) create new anim BP
2) use sub anim instance and set the base anim instance as instance class
3) convert local to component - fabrik
4) if gun is a part of the pawn or a weapon class - use world space of the mesh
tip bone - hand_l
Root bone - upperarm_l
get required socket transform world space and feed it to fabrik Effector transform
*/

/**
 * base anim instance to player class 
 */
UCLASS()
class COOPGAME_API USAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	
	virtual void NativeInitializeAnimation() override;
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	/** current character's aim pitch  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float AimPitch;

	/** current character's speed  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** current character's movement direction  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float Direction;

	/** shows whether character is jumping or not  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bJump : 1;

	/** shows whether character is dead or not  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bDead : 1;
	
	/** shows whether character is crouching or not  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bCrouch : 1;

	/** the point in the 3D-space to attach the player's left arm when we are aiming  */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	/** owning pawn reference  */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class ASCharacter* SCharacter;
};
