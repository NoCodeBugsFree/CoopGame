// Fill out your copyright notice in the Description page of Project Settings.

#include "Barrel.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "ConstructorHelpers.h"
#include "Components/SHealthComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

ABarrel::ABarrel()
{
	/** Health Comp  */
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("Health Comp"));
	HealthComp->SetTeamNumber(255);

	/** mesh  */
	GetStaticMeshComponent()->SetSimulatePhysics(true);
	GetStaticMeshComponent()->SetCanEverAffectNavigation(false);
	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->bGenerateOverlapEvents = true;
	GetStaticMeshComponent()->SetCollisionProfileName("BlockAllDynamic");
	
	/**  Radial Force Component */
	ExplosionForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("ExplosionForce"));
	ExplosionForce->SetupAttachment(RootComponent);
	ExplosionForce->Radius = ExplodeRadius;
	ExplosionForce->bIgnoreOwningActor = true;
	ExplosionForce->bImpulseVelChange = true;
	ExplosionForce->ForceStrength = 10000.f;
	ExplosionForce->bAutoActivate = false;
	ExplosionForce->ImpulseStrength = 1000.f;
	ExplosionForce->ForceStrength = 1000000.f;
	ExplosionForce->DestructibleDamage = 0.f;
	ExplosionForce->AddCollisionChannelToAffect(ECC_WorldDynamic);
	
#pragma region ConstructorHelpers

	/** explode fx  */
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleSystem(TEXT("/Game/WeaponEffects/Explosion/P_Explosion"));
	if (ParticleSystem.Object)
	{
		ExplodeFX = ParticleSystem.Object;
	}

	/** set mesh */
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Game/Barrel/SM_ExplosiveBarrel"));
	if (Sphere.Succeeded())
	{
		GetStaticMeshComponent()->SetStaticMesh(Sphere.Object);
	}

	/**  set material */
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ExplodedMaterialTemplate(TEXT("/Game/Barrel/M_ExplosiveBarrelExploded"));
	if (ExplodedMaterialTemplate.Succeeded())
	{
		ExplodedMaterial = ExplodedMaterialTemplate.Object;
	}

#pragma endregion

	bExploded = false;

	// Replication
	SetReplicates(true);
	SetReplicateMovement(true);
}

void ABarrel::BeginPlay()
{
	Super::BeginPlay();

	// [Server]
	if (Role == ROLE_Authority)
	{
		if (HealthComp)
		{
			HealthComp->OnHealthChange.AddDynamic(this, &ABarrel::OnHealthChange);
		}
	}
}

void ABarrel::OnRep_Exploded()
{
	/** change material  */
	if (ExplodedMaterial)
	{
		GetStaticMeshComponent()->SetMaterial(0, ExplodedMaterial);
	}

	/** explode particles  */
	if (ExplodeFX)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeFX, GetActorTransform());
		if (PSC)
		{
			PSC->SetWorldScale3D(GetStaticMeshComponent()->GetComponentScale() * 3.f);
		}
	}
}

void ABarrel::OnHealthChange(USHealthComponent* SHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.f && !bExploded)
	{
		bExploded = true;

		OnExplode();

		OnRep_Exploded();
	}
}

void ABarrel::OnExplode_Implementation()
{
	if (Role == ROLE_Authority)
	{
		/**  throw up a barrel  */
		FVector Impulse = GetStaticMeshComponent()->GetMass() * FVector(FMath::RandRange(0.f, 0.2f), FMath::RandRange(0.f, 0.2f), 1.f) * ImpulseStrength;
		FVector Location = GetStaticMeshComponent()->GetCenterOfMass();
		GetStaticMeshComponent()->AddImpulseAtLocation(Impulse, Location);

		/** note to apply radial damage victim must block visibility channel  */
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, BaseDamage, GetActorLocation(), ExplodeRadius, DamageType, IgnoredActors, this, GetInstigatorController(), false, ECC_GameTraceChannel1);
		
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplodeRadius, 12, FColor::Green, false, 5.f, 0, 1.f);
		
		/** activate explosion force  */
		ExplosionForce->FireImpulse();
	}
}

/** Returns properties that are replicated for the lifetime of the actor channel */
void ABarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABarrel, bExploded);
}