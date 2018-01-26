// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupBase.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"

// Sets default values
APickupBase::APickupBase()
{
	/** scene  */
	ROOT = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(ROOT);

	/* sphere collision  */
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetSphereRadius(75.f);

	/** overlap only pawn  */
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	/* decal component  */
	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	DecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	DecalComponent->DecalSize = FVector(64.f, 75.f, 75.f);

	/** mesh  */
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	PickupMesh->SetupAttachment(RootComponent);
	
	PickupMesh->SetMobility(EComponentMobility::Movable);
	PickupMesh->bCastDynamicShadow = false;
	PickupMesh->SetupAttachment(RootComponent);
	PickupMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	/** point light */
	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComponent"));
	PointLightComponent->SetupAttachment(RootComponent);
	PointLightComponent->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	PointLightComponent->SetAttenuationRadius(400.f);
	PointLightComponent->CastShadows = false;
	PointLightComponent->SetIntensity(2000.f);
	PointLightComponent->SetLightColor(FLinearColor(0.f, 1.f, 0.95f));
	
	/** rotating movement  */
	RotationMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotation Movement"));
	RotationMovement->RotationRate = FRotator(0.f, 90.f, 0.f);
	RotationMovement->bUpdateOnlyIfRendered = true;

#pragma region ConstructorHelpers

	/**  set mesh asset */
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Game/Powerups/SpeedIcon"));
	if (Sphere.Succeeded())
	{
		PickupMesh->SetStaticMesh(Sphere.Object);
	}

	/** set light function  */
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Powerups/M_PowerupLightFunction"));
	if (Material.Succeeded())
	{
		PointLightComponent->SetLightFunctionMaterial(Material.Object);
	}

	/** set the PickupUsedSound sound asset  */
	static ConstructorHelpers::FObjectFinder<USoundBase> PickupUsedSoundTemplate(TEXT("/Game/Powerups/Pickup_powerup"));
	if (PickupUsedSoundTemplate.Object)
	{
		PickupUsedSound = PickupUsedSoundTemplate.Object;
	}

	/** set the PickupUsedSound sound asset  */
	static ConstructorHelpers::FObjectFinder<USoundBase> PickupRepspawnedSoundTemplate(TEXT("/Game/Powerups/respawn_01_Cue"));
	if (PickupRepspawnedSoundTemplate.Object)
	{
		PickupRespawnedSound = PickupRepspawnedSoundTemplate.Object;
	}

	/** set decal material  */
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DecalMaterial(TEXT("/Game/Powerups/M_PowerupDecal"));
	if (DecalMaterial.Succeeded())
	{
		DecalComponent->SetMaterial(0, DecalMaterial.Object);
	}
#pragma endregion

	// Replication
	SetReplicates(true);
}

void APickupBase::NotifyActorBeginOverlap(AActor* OtherActor)
{
	// [Server]
	if (Role == ROLE_Authority)
	{
		Super::NotifyActorBeginOverlap(OtherActor);

		if (OtherActor && OtherActor != this && !OtherActor->IsPendingKill() && bActivated == false && OverlappedActor == nullptr)
		{
			/** save overlapper reference (only this actor able to fire pickup respawning when he ends overlaps this actor)  */
			OverlappedActor = OtherActor;

			bActivated = true;
			OnRep_Activated(); /** server  */

			/** calls when pick up is activated  */
			OnPickupActivated(OtherActor);
		}
	}
}

void APickupBase::NotifyActorEndOverlap(AActor* OtherActor)
{
	// [Server]
	if (Role == ROLE_Authority)
	{
		Super::NotifyActorEndOverlap(OtherActor);

		if (OtherActor && OtherActor != this && OtherActor == OverlappedActor && bActivated == true)
		{
			// show and activate pickup
			FTimerDelegate DeactivateDelegate;
			DeactivateDelegate.BindLambda([&] {
				/** nullify overlapper reference  */
				OverlappedActor = nullptr;
				bActivated = false;
				OnRep_Activated(); /** server  */
			});

			FTimerHandle DeactivateTimer;
			GetWorldTimerManager().SetTimer(DeactivateTimer, DeactivateDelegate, PickupRespawnDelay, false);
		}
	}
}

void APickupBase::OnPickupActivated_Implementation(AActor* Activator)
{
	if (Activator)
	{
		
	}
}

void APickupBase::Initialize(class USoundBase* PickupUsedSoundToSet, class USoundBase* PickupRespawnedSoundToSet, float PickupRespawnDelayToSet, float PowerupDurationToSet)
{
	PickupUsedSound = PickupUsedSoundToSet;
	PickupRespawnedSound = PickupRespawnedSoundToSet;
	PickupRespawnDelay = PickupRespawnDelayToSet;
	PowerupDuration = PowerupDurationToSet;
}

void APickupBase::OnRep_Activated()
{	
	/** return to initial state  */
	if (bActivated)
	{
		/** hide mesh and point light  */
		PickupMesh->SetVisibility(false);
		PointLightComponent->SetVisibility(false);
		if (PickupUsedSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PickupUsedSound, GetActorLocation());
		}
	}
	else
	{
		/** show the mesh and point light  */
		PickupMesh->SetVisibility(true);
		PointLightComponent->SetVisibility(true);

		if (PickupRespawnedSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PickupRespawnedSound, GetActorLocation());
		}
	}
}

/** Returns properties that are replicated for the lifetime of the actor channel */
void APickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickupBase, bActivated);
}
