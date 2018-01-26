// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "Net/UnrealNetwork.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("Coop.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for weapons"), ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	/** weapon mesh  */
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	/** defaults  */

	/** first shot has no bullet spread   */
	bFirstShot = true;
	
#pragma region ConstructorHelpers

	/** set the MuzzleFX emitter  */
	static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleFXParticleSystem(TEXT("/Game/WeaponEffects/Muzzle/P_Muzzle_Large"));
	if (MuzzleFXParticleSystem.Object)
	{
		MuzzleFX = MuzzleFXParticleSystem.Object;
	}

	/** set the Default ImpactFX emitter  */
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactFXParticleSystem(TEXT("/Game/WeaponEffects/GenericImpact/P_RifleImpact"));
	if (ImpactFXParticleSystem.Object)
	{
		DefaultImpactFX = ImpactFXParticleSystem.Object;
	}

	/** set the Flesh ImpactFX emitter  */
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FleshFXParticleSystem(TEXT("/Game/WeaponEffects/BloodImpact/P_blood_splash_02"));
	if (FleshFXParticleSystem.Object)
	{
		FleshImpactFX = FleshFXParticleSystem.Object;
	}

	/** set the TracerFX emitter  */
	static ConstructorHelpers::FObjectFinder<UParticleSystem> TracerFXParticleSystem(TEXT("/Game/WeaponEffects/BasicTracer/P_SmokeTrail"));
	if (TracerFXParticleSystem.Object)
	{
		TracerFX = TracerFXParticleSystem.Object;
	}

	/** set the weapon mesh  */
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh(TEXT("/Game/Weapons/SK_Rifle"));
	if (SkeletalMesh.Object)
	{
		WeaponMesh->SetSkeletalMesh(SkeletalMesh.Object);
	}

	/**  set the camera shake class */
	static ConstructorHelpers::FClassFinder<UCameraShake> CameraShakeBPClass(TEXT("/Game/BP/BP_RifleCameraShake"));
	if (CameraShakeBPClass.Class != NULL)
	{
		Shake = CameraShakeBPClass.Class;
	}

#pragma endregion

	TimeBetweenShots = 60.f / FireRate;
	SetReplicates(true);
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60.f / FireRate;

	/** I want to fire at begin play  */
	LastFiredTime = -TimeBetweenShots;
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

void ASWeapon::Fire()
{
	// [Client]
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner && Ammo > 0)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();

		/** bullet spread not applied to first shot  */
		if (!bFirstShot)
		{
			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		}
		
		FVector TraceEnd = EyeLocation + ShotDirection * TraceDistance;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		/** particle "Target" parameter  */
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			bFirstShot = false;
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				// add impulse to physics actors
				UPrimitiveComponent* DamagedComponent = Hit.GetComponent();
				if (DamagedComponent && DamagedComponent->IsSimulatingPhysics())
				{
					DamagedComponent->AddImpulseAtLocation(ShotDirection * DamagedComponent->GetMass() * 100.f, Hit.Location);
				}

				float ActualDamage = Damage;
				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
				if (SurfaceType == SURFACE_FLESH_VULNERABLE)
				{
					ActualDamage *= 4.f;
				}

				// [Server]
				if (Role == ROLE_Authority)
				{
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
				}

				if (GetNetMode() != NM_DedicatedServer)
				{
					PlayImpactEffect(SurfaceType, Hit.ImpactPoint);
				}

				PlayImpactEffect(SurfaceType, Hit.ImpactPoint);

				/** update tracer end point  */
				TracerEndPoint = Hit.ImpactPoint;
			}
		}

		/** debug line  */
		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation,TraceEnd, FColor::Red, false, 1.f, 0, 1.f);
		}
		
		if (GetNetMode() != NM_DedicatedServer)
		{
			PlayFireFX(TracerEndPoint);
		}
		PlayFireFX(TracerEndPoint);

		// [Server]
		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
		
			HitScanTrace.SurfaceType = SurfaceType;

			OnRep_HitScanTrace();
		}

		LastFiredTime = GetWorld()->GetTimeSeconds();
		Ammo--;
	}
}

void ASWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	/** impact particles FX  */
	UParticleSystem* SelectedFX = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESH_DEFAULT:
	case SURFACE_FLESH_VULNERABLE:
		SelectedFX = FleshImpactFX;
		break;

	default:
		SelectedFX = DefaultImpactFX;
		break;
	}

	if (SelectedFX)
	{
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedFX, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	/** play cosmetic FX  */
	PlayFireFX(HitScanTrace.TraceTo);
	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->GetTimeSeconds(), 0.f);
	GetWorldTimerManager().SetTimer(FireTimer, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	bFirstShot = true;
	GetWorldTimerManager().ClearTimer(FireTimer);
}

void ASWeapon::PlayFireFX(FVector TracerEndPoint)
{
	/** muzzle flash  */
	if (MuzzleFX)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeaponMesh, MuzzleSocketName);
	}

	/** smoke tracer  */
	if (TracerFX)
	{
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* SmokeTrail = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerFX, MuzzleLocation);
		if (SmokeTrail)
		{
			SmokeTrail->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
	
	/** play camera shake  */
	AActor* Owner = GetOwner();
	if (Owner)
	{
		if(APawn* PlayerPawn = Cast<APawn>(Owner))
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(PlayerPawn->GetController()))
			{
				PlayerController->ClientPlayCameraShake(Shake);
			}
		}
	}
}

#if WITH_EDITOR
void ASWeapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	TimeBetweenShots = 60.f / FireRate;

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

/** Returns properties that are replicated for the lifetime of the actor channel */
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASWeapon, Ammo);
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}