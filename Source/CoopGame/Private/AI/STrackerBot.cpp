// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "Components/SHealthComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "Components/AudioComponent.h"
#include "ConstructorHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "Net/UnrealNetwork.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(TEXT("Coop.DebugTrackerBot"), DebugTrackerBotDrawing, TEXT("Draw Debug Lines for TrackerBot"), ECVF_Cheat);

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/** Health Comp  */
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("Health Comp"));
	HealthComp->SetTeamNumber(255);

	/** mesh  */
	BotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bot Mesh"));
	SetRootComponent(BotMesh);
	BotMesh->SetCanEverAffectNavigation(false);
	BotMesh->SetSimulatePhysics(true);

	/** sphere collision  */
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereCollision->SetupAttachment(RootComponent);
	SphereCollision->SetSphereRadius(200.f);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	/** audio component  */
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);

#pragma region ConstructorHelpers

	/**  set the default audio */
	static ConstructorHelpers::FObjectFinder<USoundBase> SoundTemplate(TEXT("/Game/TrackerBot/ball_roll_03_loop_Cue"));
	if (SoundTemplate.Succeeded())
	{
		AudioComponent->SetSound(SoundTemplate.Object);
	}

	/**  set the default static mesh */
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Game/TrackerBot/SM_TrackerBot"));
	if (Sphere.Succeeded())
	{
		BotMesh->SetStaticMesh(Sphere.Object);
	}

	/** set the explosion emitter  */
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplodeFXTemplate(TEXT("/Game/WeaponEffects/Explosion/P_Explosion"));
	if (ExplodeFXTemplate.Object)
	{
		ExplodeFX = ExplodeFXTemplate.Object;
	}

	/** set the explosion emitter  */
	static ConstructorHelpers::FObjectFinder<USoundBase> SelfDestroySoundTemplate(TEXT("/Game/TrackerBot/DroneTracker_explodewarning_Cue"));
	if (SelfDestroySoundTemplate.Object)
	{
		SelfDestroySound = SelfDestroySoundTemplate.Object;
	}

	/** set the sound asset  */
	static ConstructorHelpers::FObjectFinder<USoundBase> ExplodeSoundTemplate(TEXT("/Game/TrackerBot/Explosion01_Cue"));
	if (ExplodeSoundTemplate.Object)
	{
		ExplodeSound = ExplodeSoundTemplate.Object;
	}

#pragma endregion

	/** defaults  */
	bUseVelocityChange = true;
	bExploded = false;

	// Replication
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ASTrackerBot::OnOverlapBegin);
	
	DynamicMaterial = BotMesh->CreateDynamicMaterialInstance(0);

	if (HealthComp)
	{
		HealthComp->OnHealthChange.AddDynamic(this, &ASTrackerBot::OnHealthChange);
	}

	// [Server]
	if (Role == ROLE_Authority)
	{
		Jump();

		NextPathPoint = GetNextPathPoint();
		
		GetWorldTimerManager().SetTimer(CheckForBuddiesTimer, this, &ASTrackerBot::CheckForBuddies, 1.f, true);
	}
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// [Server]
	if (Role == ROLE_Authority)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			FVector Force = ForceDirection * MovementForce;
			BotMesh->AddForce(Force, NAME_None, bUseVelocityChange);
		}
	}
	
	if (AudioComponent)
	{
		float Volume = FMath::GetMappedRangeValueClamped(FVector2D(10.f, 1000.f), FVector2D(0.f, 2.f), GetVelocity().Size());
		AudioComponent->SetVolumeMultiplier(Volume);
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestDistance = MAX_FLT;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		if (APawn* Pawn = It->Get())
		{
			/** skip invalid pawns and friendly pawns */
			if (Pawn == nullptr || USHealthComponent::IsFriendly(Pawn, this))
			{
				continue;
			}

			USHealthComponent* HealthComp = Cast<USHealthComponent>(Pawn->GetComponentByClass(USHealthComponent::StaticClass()));

			/** alive player found  */
			if (HealthComp && HealthComp->GetHealth() > 0.f)
			{
				float CurrentDistance = (GetActorLocation() - Pawn->GetActorLocation()).Size();
				if (CurrentDistance < NearestDistance)
				{
					NearestDistance = CurrentDistance;
					BestTarget = Pawn;
				}
			}
		}
	}

	if (BestTarget)
	{
		UNavigationPath* NavigationPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);
		
		/** refresh the timer  */
		GetWorldTimerManager().ClearTimer(RefreshPathTimer);
		GetWorldTimerManager().SetTimer(RefreshPathTimer, this, &ASTrackerBot::RefreshThePath, 5.f, false);

		if (NavigationPath && NavigationPath->PathPoints.Num() > 1)
		{
			return NavigationPath->PathPoints[1];
		}
	}
	return GetActorLocation();
}

void ASTrackerBot::OnSelfDestruct_Implementation()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	/** disable actor  */
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	GetWorldTimerManager().ClearAllTimersForObject(this);

	/** play explode FX  */
	if (ExplodeFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeFX, GetActorTransform());
	}

	/** play explosion sound  */
	if (ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	}

	if (DebugTrackerBotDrawing)
	{
		/** show damage area  */
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Green, false, 5.f, 0, 1.f);
	}

	// Apply Damage!
	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Increase damage based on the power level
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);
		
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), false);
		
		/** network latency  */
		SetLifeSpan(5.0f);
	}
}

void ASTrackerBot::Jump()
{
	/**  throw up a bot  */
	FVector Impulse = BotMesh->GetMass() * FVector(FMath::RandRange(0.f, 0.2f), FMath::RandRange(0.f, 0.2f), 1.f) * FMath::RandRange(ImpulseStrengthMin, ImpulseStrengthMax);
	FVector Location = BotMesh->GetCenterOfMass();
	BotMesh->AddImpulseAtLocation(Impulse, Location);

	/** set timer to next jump  */
	GetWorldTimerManager().SetTimer(JumpTimer, this, &ASTrackerBot::Jump, FMath::RandRange(1.f, 5.f), true);
}

void ASTrackerBot::RefreshThePath()
{
	NextPathPoint = GetNextPathPoint();
}

void ASTrackerBot::CheckForBuddies()
{
	TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	TArray<AActor*> OutActors;
	if (UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), BuddiesDetectionRadius, ObjectTypes, ASTrackerBot::StaticClass(), ActorsToIgnore, OutActors))
	{
		PowerLevel = OutActors.Num();
	}
	else
	{
		PowerLevel = 0.f;
	}

	if (DebugTrackerBotDrawing)
	{
		DrawDebugString(GetWorld(), GetActorLocation(), *FString::SanitizeFloat(PowerLevel), nullptr, FColor::Red, 1.f);
	}
	
	/** calc and replicate alpha  */
	Alpha = PowerLevel / MaxPowerLevel;
	OnRep_Alpha(); /** for server  */
	
	/// ---------------- TOM ------------------------------------------------------------

	///** create collision shape  */
	//FCollisionShape CollisionShape;
	//CollisionShape.SetSphere(BuddiesDetectionRadius);

	///** only find pawns  */
	//FCollisionObjectQueryParams COQP;
	//COQP.AddObjectTypesToQuery(ECC_PhysicsBody);
	//COQP.AddObjectTypesToQuery(ECC_Pawn);

	//TArray<FOverlapResult> Overlaps;

	//const UWorld* World = GetWorld();
	//if (World)
	//{
	//	World->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, COQP, CollisionShape);
	//}

	//int32 Results = 0;
	//for (FOverlapResult Result : Overlaps)
	//{
	//	if(ASTrackerBot* STrackerBot = Cast<ASTrackerBot>(Result.GetActor()))
	//	{
	//		if (STrackerBot != this)
	//		{
	//			Results++;
	//		}
	//	}
	//}
}

void ASTrackerBot::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself. 
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		if(ASCharacter* SCharacter = Cast<ASCharacter>(OtherActor))
		{
			/** activate only near the enemy  */
			if ( ! USHealthComponent::IsFriendly(OtherActor, this) )
			{
				/** disable collision */
				SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				// [Server]
				if (Role == ROLE_Authority)
				{
					/** start destroying  */
					GetWorldTimerManager().SetTimer(SelfDamageTimer, this, &ASTrackerBot::SelfDamage, SelfDamageRate, true);
				}

				if (SelfDestroySound)
				{
					UGameplayStatics::SpawnSoundAttached(SelfDestroySound, RootComponent);
				}
			}
		}
	}
}

void ASTrackerBot::SelfDamage()
{
	UGameplayStatics::ApplyDamage(this, SelfDamagePerTimerTick, GetInstigatorController(), this, nullptr);

	if (DebugTrackerBotDrawing)
	{
		DrawDebugString(GetWorld(), GetActorLocation(), "Tick!", nullptr, FColor::Red, 1.f);
	}
}

void ASTrackerBot::OnHealthChange(USHealthComponent* SHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	/** pulse the material on hit  */
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->GetTimeSeconds());
	}

	/** destroy  */
	if (Health <= 0.f)
	{
		FTimerHandle DestroyTimer;
		GetWorldTimerManager().SetTimer(DestroyTimer, this, &ASTrackerBot::OnSelfDestruct, FMath::RandRange(0.1f, 0.4f), false);
	}
}

void ASTrackerBot::OnRep_Alpha()
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
}

/** Returns properties that are replicated for the lifetime of the actor channel */
void ASTrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASTrackerBot, Alpha);
}
