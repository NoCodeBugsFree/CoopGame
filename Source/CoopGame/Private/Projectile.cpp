// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "ConstructorHelpers.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	/** Projectile Collision  */
	ProjectileCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
	SetRootComponent(ProjectileCollision);
	ProjectileCollision->SetSphereRadius(20.f);
	ProjectileCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProjectileCollision->SetCollisionProfileName("BlockAll");

	/** projectile mesh  */
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetRelativeScale3D(FVector(0.2f, 0.1f, 0.1f));
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**  set the default static mesh */
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (Sphere.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(Sphere.Object);
	}
	
	/** Projectile Movement Component  */
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->MaxSpeed = 2000.f;
	ProjectileMovementComponent->bShouldBounce = true;

	/** set the explosion emitter  */
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleSystem(TEXT("/Game/WeaponEffects/Explosion/P_Explosion"));
	if (ParticleSystem.Object)
	{
		ExplodeFX = ParticleSystem.Object;
	}

	/**  Radial Force Component */
	ExplosionForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("ExplosionForce"));
	ExplosionForce->SetupAttachment(RootComponent);
	ExplosionForce->Radius = ExplodeRadius;
	ExplosionForce->bIgnoreOwningActor = true;
	ExplosionForce->ForceStrength = 10000.f;
	ExplosionForce->bAutoActivate = false;
	ExplosionForce->ImpulseStrength = 1000.f;
	ExplosionForce->ForceStrength = 1000000.f;
	ExplosionForce->DestructibleDamage = 0.f;
	
	// Replication
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	/** explode  */
	FTimerDelegate ExplodeDelegate;
	ExplodeDelegate.BindLambda( [&] { Explode();  });
	FTimerHandle ExplodeTimer;
	GetWorldTimerManager().SetTimer(ExplodeTimer, ExplodeDelegate, ExlodeDelay, false);
}

void AProjectile::Explode()
{
	if (ExplodeFX)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeFX, GetActorTransform());
		if (PSC)
		{
			PSC->SetWorldScale3D(FVector(3.f, 3.f, 3.f));
		}
	}

	// [Server]
	if (Role == ROLE_Authority)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplodeRadius, 12, FColor::Green, false, 5.f, 0, 1.f);

		AController* InstigatorController = nullptr;
		if (GetOwner())
		{
			InstigatorController = GetOwner()->GetInstigatorController();
		}

		/** note to apply radial damage victim must block visibility channel  */
		UGameplayStatics::ApplyRadialDamage(this, BaseDamage, GetActorLocation(), ExplodeRadius, DamageType, TArray<AActor*>(), this, InstigatorController);

		/** activate explosion force  */
		ExplosionForce->Activate(true);
		ExplosionForce->FireImpulse();

		/** Hide actor and disable collisions  */
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);

		/** deactivate explosion force  */
		FTimerDelegate DisableExplosionForceDelegate;
		DisableExplosionForceDelegate.BindLambda([&] { ExplosionForce->Deactivate(); Destroy();  });

		FTimerHandle DisableTimer;
		GetWorldTimerManager().SetTimer(DisableTimer, DisableExplosionForceDelegate, ExplosionForceDuration, false);
	}

}

