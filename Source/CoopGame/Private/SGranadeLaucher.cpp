// Fill out your copyright notice in the Description page of Project Settings.

#include "SGranadeLaucher.h"
#include "ConstructorHelpers.h"
#include "Projectile.h"

ASGranadeLaucher::ASGranadeLaucher()
{

#pragma region ConstructorHelpers

	/** set the weapon mesh  */
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh(TEXT("/Game/Weapons/Launcher"));
	if (SkeletalMesh.Object)
	{
		GetWeaponMesh()->SetSkeletalMesh(SkeletalMesh.Object);
	}

	/** set the projectile template */
	static ConstructorHelpers::FClassFinder<AProjectile> ProjectileClass(TEXT("/Game/BP/BP_Projectile"));
	if (ProjectileClass.Succeeded())
	{
		ProjectileTemplate = ProjectileClass.Class;
	}

	/**  set the camera shake class */
	static ConstructorHelpers::FClassFinder<UCameraShake> CameraShakeBPClass(TEXT("/Game/BP/BP_GranadeLauncherCameraShake"));
	if (CameraShakeBPClass.Class != NULL)
	{
		Shake = CameraShakeBPClass.Class;
	}
	
#pragma endregion

	/** defaults  */
	FireRate = 60.f;
	TracerFX = nullptr;
	DefaultImpactFX = nullptr;
	Ammo = 30;
}

void ASGranadeLaucher::Fire()
{
	// [Client]
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (ProjectileTemplate && MyOwner && Ammo > 0)
	{
		FVector EyeLocation;
		FRotator EyeRotation;

		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = MyOwner;
			SpawnParams.Instigator = MyOwner->GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			FVector SpawnLocation = GetWeaponMesh()->GetSocketLocation(MuzzleSocketName);
						
			// [Server]
			if (Role == ROLE_Authority)
			{
				World->SpawnActor<AProjectile>(ProjectileTemplate, SpawnLocation, EyeRotation, SpawnParams);
			}
		}

		if (GetNetMode() != NM_DedicatedServer)
		{
			PlayFireFX();
		}

		LastFiredTime = GetWorld()->GetTimeSeconds();
		Ammo--;
	}
}