// Fill out your copyright notice in the Description page of Project Settings.

#include "SAnimInstance.h"
#include "SCharacter.h"

void USAnimInstance::NativeInitializeAnimation()
{
	if(ASCharacter* TestSCharacter = Cast<ASCharacter>(TryGetPawnOwner()))
	{
		SCharacter = TestSCharacter;
	}
}

void USAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (SCharacter)
	{
		FVector CharacterVelocity = SCharacter->GetVelocity();
		FRotator CharacterRotation = SCharacter->GetActorRotation();

		/** speed  */
		Speed = CharacterVelocity.Size();

		/** direction  */
		Direction = CalculateDirection(CharacterVelocity, CharacterRotation);

		/** crouch  */
		bCrouch = SCharacter->bIsCrouched;

		/** jumping  */
		bJump = SCharacter->IsJumpedLastFrame();

		/** dead  */
		bDead = SCharacter->IsDead();

		/** aim pitch  */
		AimPitch = SCharacter->GetAimPitch();

		/** left arm transform  */
		LeftHandTransform = SCharacter->GetLeftArmSocketTransform();
		
	}
}
