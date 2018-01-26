// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickup.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerup.h"
#include "ConstructorHelpers.h"

// Sets default values
ASPickup::ASPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	/* sphere collision  */
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetSphereRadius(75.f);

	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	/* decal component  */
	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	DecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	DecalComponent->DecalSize = FVector(64.f, 75.f, 75.f);
}

// Called when the game starts or when spawned
void ASPickup::BeginPlay()
{
	Super::BeginPlay();
	
	// [Server]
	if (Role == ROLE_Authority)
	{
		Respawn();
	}
}

void ASPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	
	if (OtherActor && SpawnedSPowerup && Role == ROLE_Authority)
	{
		SpawnedSPowerup->Activate(OtherActor);
		SpawnedSPowerup = nullptr;

		/** respawn by timer */
		FTimerHandle RespawnTimer;
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ASPickup::Respawn, CooldownDuration);
	}
}

void ASPickup::Respawn()
{
	if (PowerupClass)
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			FVector SpawnLocation = GetActorLocation();
			FRotator SpawnRotation = GetActorRotation();
			
			SpawnedSPowerup = World->SpawnActor<ASPowerup>(PowerupClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (SpawnedSPowerup)
			{

			}
		}
	}
}
