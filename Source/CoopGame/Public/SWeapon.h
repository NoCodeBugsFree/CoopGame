// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

/** contains info of a single weapon line trace  */
USTRUCT()
struct FHitScanTrace
{
	GENERATED_USTRUCT_BODY()

	/** the surface of actor we hit  */
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	/**  the impact point in 3D space */
	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()

protected:

	/* represents weapon mesh  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* WeaponMesh;

	/* MuzzleFX Emitter Template */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFX;
	
	/* Default ImpactFX Emitter Template */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* DefaultImpactFX;

	/* Flesh ImpactFX Emitter Template */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* FleshImpactFX;
	
	/* TracerFX Emitter Template */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* TracerFX;

	/** camera shake when fires a weapon  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UCameraShake> Shake;

	/** damage type for this kind of weapon  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UDamageType> DamageType;

public:
	
	/** calls to fire a singe shot  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void StartFire();

	/** calls to stop fire  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void StopFire();

	/** calls to add some ammo  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void AddAmmo(int32 AmmoToAdd) { Ammo += AmmoToAdd; }
	
protected:

	// Sets default values for this actor's properties
	ASWeapon();

	virtual void BeginPlay() override;

	/** calls to play Muzzle and tracer FX */
	void  PlayFireFX(FVector TracerEndPoint = FVector(0.f, 0.f, 0.f));

	/** server fire  */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();
	bool ServerFire_Validate() { return true; }
	void ServerFire_Implementation();


	/** the name of the muzzle flash socket */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	FName MuzzleSocketName = FName("MuzzleFlashSocket");

	/** RPM - bullets per minute fired by weapon  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float FireRate = 600.f;

	/** the amount of ammo  */
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	int32 Ammo = 120;

	/** delay between each shot in seconds */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float TimeBetweenShots = 0.f;

	/** the time of last shot */
	UPROPERTY(BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float LastFiredTime = 0.f;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	
	/** calls to fire a singe shot  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	virtual void Fire();

	/** calls to play impact FX according to surface type we hit  */
	UFUNCTION(BlueprintCallable, Category = "AAA")
	void PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	/** timer that triggers the fire event  */
	UPROPERTY()
	FTimerHandle FireTimer;

	/** shows whether this fire shot is first after we stop firing or not*/
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint32 bFirstShot : 1;

	/** (in UU) weapon range (1km default) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float TraceDistance = 100000.f;

	/** damage to cause per hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float Damage = 33.f;

	/** bullet spread per degrees */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	float BulletSpread = 1.f;

	/** the name of the param  in SmokeTrail emitter to attach the smoke trail to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	FName TracerTargetName = FName("BeamEnd");

	/** the name of the socket on weapon mesh to attach the player's left arm when we aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	FName LeftArmSocketName = FName("LeftArm");

	/** info of a single weapon line trace  */
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;
	
	/**  called whenever HitScanTrace updated */
	UFUNCTION()
	virtual void OnRep_HitScanTrace();
	
public:

	/** retunrs weapon mesh comp **/
	FORCEINLINE class USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	/** retunrs current ammo amount **/
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	/** retunrs current transform of the left arm socket **/
	FORCEINLINE FTransform GetLeftArmSocketTransform() const { return WeaponMesh->GetSocketTransform(LeftArmSocketName); }
};