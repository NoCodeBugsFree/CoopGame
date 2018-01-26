// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SWeapon.h"
#include "SCharacter.generated.h"


/** FOV state  */
UENUM(BlueprintType)
enum class EFOVState : uint8
{
	FS_None			UMETA(DisplayName = "None"),
	FS_Simple		UMETA(DisplayName = "Simple"),
	FS_Sniper		UMETA(DisplayName = "Sniper")
};

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	/* TP camera  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TP_Camera;
	
	/*  Camera Boom */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/*  Health component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USHealthComponent* HealthComp;
	
public:
	
	/** @return	Pawn's eye location (in our case TP_Camera component location )*/
	virtual FVector GetPawnViewLocation() const override;

	/** calls when character is dead  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AAA")
	void OnDeath();
	virtual void OnDeath_Implementation();

	/** calls to start fire  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void StartFire();

	/** calls to stop fire */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void StopFire();

	/** calls to perform AI fire with delay */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StartFireAI(float FiringTime);
	
	/** calls to get all necessary data to represent it on HUD  */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void GetHUDData(FText& AmmoText, FText& ScoreText, FText& PingText, float& HealthPercent) const;

	/** calls to get the current weapon  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	class ASWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	/** calls to get health component */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	class USHealthComponent* GetHealthComp() const { return HealthComp; }

protected:

	/** calls whenever health in health component is changed  */
	UFUNCTION()
	void OnHealthChange(USHealthComponent* SHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// Sets default values for this character's properties
	ASCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** [tick] calls to adjust FOV  */
	void SetFOV(float DeltaTime);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** BlueprintNativeEvent fired when the character has just started jumping */
	virtual void OnJumped_Implementation() override;

	/**
	* Called upon landing when falling, to perform actions based on the Hit result. Triggers the OnLanded event.
	* Note that movement mode is still "Falling" during this event. Current Velocity value is the velocity at the time of landing.
	* Consider OnMovementModeChanged() as well, as that can be used once the movement mode changes to the new mode (most likely Walking).
	*
	* @param Hit Result describing the landing that resulted in a valid landing spot.
	* @see OnMovementModeChanged()
	*/
	virtual void Landed(const FHitResult& Hit) override;
	
	// -----------------------------------------------------------------------------------

	/** calls to move forward/backward  */
	void MoveForward(float Value);

	/** calls to move right/left  */
	void MoveRight(float Value);

	/** calls to start crouch  */
	void BeginCrouch();

	/** calls to end crouch  */
	void EndCrouch();

private:

	/** calls to set new FOV state  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void NextFOV();

	/** calls to get character the initial weapon  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void SpawnDefaultWeapon();

	/** current aim pitch to update animation  */
	UPROPERTY(Replicated)
	float AimPitch;

	/** current FOV state  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	EFOVState FOVState = EFOVState::FS_None;

	/** default field of view value  */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float DefaultFOV = 90.f;

	/** simple (for each weapon) field of view value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float SimpleFOV = 30.f;

	/** sniper (sniper rifles only) field of view value */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float SniperFOV = 7.f;

	/** default weapon template  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ASWeapon> DefaultWeaponTemplate;
	
	/** current weapon reference  */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class ASWeapon* CurrentWeapon;

	/** shows whether character is dead or alive */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bDead : 1;

	/** shows whether character was jumped last frame or not */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bJumpedLastFrame : 1;
	
	/** field of view interpolation speed  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float FOVInterpSpeed = 15.f;
	
	/** the name of the socket to attach weapon  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	FName WeaponAttachSocketName = FName("Weapon");

	/** player state reference */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class ASPlayerState* SPlayerState;

	/** game state reference */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class ASGameStateBase* SGameStateBase;

public:
	
	/** returns whether character is dead or not **/
	FORCEINLINE bool IsDead() const { return bDead; }
	/** returns whether character was jumped last frame or not **/
	FORCEINLINE bool IsJumpedLastFrame() const { return bJumpedLastFrame; }
	/** returns current aim pitch **/
	FORCEINLINE float GetAimPitch() const { return AimPitch; }
	/** retunrs current transform of the left arm socket **/
	FORCEINLINE FTransform GetLeftArmSocketTransform() const { return CurrentWeapon ? CurrentWeapon->GetLeftArmSocketTransform() : FTransform(); }
};
