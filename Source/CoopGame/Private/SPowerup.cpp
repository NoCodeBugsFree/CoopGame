// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerup.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerup::ASPowerup()
{
	/** scene  */
	ROOT = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(ROOT);
	
	/** mesh  */
	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->bCastDynamicShadow = false;
	GetStaticMeshComponent()->SetupAttachment(RootComponent);
	GetStaticMeshComponent()->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**  set mesh asset */
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Game/Powerups/SpeedIcon"));
	if (Sphere.Succeeded())
	{
		GetStaticMeshComponent()->SetStaticMesh(Sphere.Object);
	}

	/** point light */
	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComponent"));
	PointLightComponent->SetupAttachment(RootComponent);
	PointLightComponent->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	PointLightComponent->SetAttenuationRadius(400.f);
	PointLightComponent->CastShadows = false;
	PointLightComponent->SetIntensity(2000.f);

	/** set light function  */
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Powerups/M_PowerupLightFunction"));
	if (Material.Succeeded())
	{
		PointLightComponent->SetLightFunctionMaterial(Material.Object);
	}

	/** rotating movement  */
	RotationMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotation Movement"));
	RotationMovement->RotationRate = FRotator(0.f, 90.f, 0.f);

	// Replication
	SetReplicates(true);
	
	/** defaults  */
	bPowerUpActive = false;
}

void ASPowerup::Activate(AActor* OtherActor)
{
	if (OtherActor)
	{
		OnActivated(OtherActor);

		bPowerUpActive = true;
		OnRep_PowerUpActive(); /** server  */

		if (PowerupInterval > 0.f)
		{
			FTimerHandle Timer;
			GetWorldTimerManager().SetTimer(Timer, this, &ASPowerup::OnTickPowerup, PowerupInterval, true);
		}
		else
		{
			OnTickPowerup();
		}
	}
}

void ASPowerup::OnRep_PowerUpActive()
{
	OnPowerUpStateChanged(bPowerUpActive);
}

void ASPowerup::OnTickPowerup()
{
	TickProcessed++;
	
	OnPowerupTicked();

	if (TotalNumberOfTicks >= TickProcessed)
	{
		OnExpired();

		bPowerUpActive = false;
		OnRep_PowerUpActive(); /** server  */
	}
}

/** Returns properties that are replicated for the lifetime of the actor channel */
void ASPowerup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerup, bPowerUpActive);
}
