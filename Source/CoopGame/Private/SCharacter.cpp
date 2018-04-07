// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ConstructorHelpers.h"
#include "Components/SHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SPlayerState.h"
#include "SGameStateBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/** Health Comp  */
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("Health Comp"));
	HealthComp->SetTeamNumber(0);

	/*  Camera Boom */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 160.f;
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

	/* TP camera  */
	TP_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("TP_Camera"));
	TP_Camera->SetupAttachment(CameraBoom);
	TP_Camera->SetRelativeLocation(FVector(0.f, 60.f, 0.f));

	// enable crouching
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;

	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->CrouchedHalfHeight = 60.f;
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->JumpZVelocity = 500.f;

	/** capsule collision setup  */
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	
	/** mesh collision setup  */
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	
	/** set the weapon template */
	static ConstructorHelpers::FClassFinder<ASWeapon> DefaultWeaponTemplateClass(TEXT("/Game/BP/BP_Rifle"));
	if (DefaultWeaponTemplateClass.Succeeded())
	{
		DefaultWeaponTemplate = DefaultWeaponTemplateClass.Class;
	}
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority)
	{
		SpawnDefaultWeapon();

		if (HealthComp)
		{
			HealthComp->OnHealthChange.AddDynamic(this, &ASCharacter::OnHealthChange);
		}
	}

	if (ASPlayerState* SPlayerStateTest = Cast<ASPlayerState>(PlayerState))
	{
		SPlayerState = SPlayerStateTest;
	}

	if (ASGameStateBase* SGameStateBaseTest = Cast<ASGameStateBase>(GetWorld()->GetGameState()))
	{
		SGameStateBase = SGameStateBaseTest;
	}
	
	/** save the default FOV  */
	DefaultFOV = TP_Camera->FieldOfView;
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		SetFOV(DeltaTime);
	}
	
	// [Server]
	if (Role == ROLE_Authority)
	{
		/** replicated weapon pitch  */
		AimPitch = TP_Camera->GetComponentRotation().Pitch;
	}
}

void ASCharacter::SetFOV(float DeltaTime)
{
	float TargetFOV = 0.f;
	switch (FOVState)
	{
	case EFOVState::FS_None:
		TargetFOV = DefaultFOV;
		break;

	case EFOVState::FS_Simple:
		TargetFOV = SimpleFOV;
		break;

	case EFOVState::FS_Sniper:
		TargetFOV = SniperFOV;
		break;

	default:
		break;
				
	}
	float ZoomThisFrame = FMath::FInterpTo(TP_Camera->FieldOfView, TargetFOV, DeltaTime, FOVInterpSpeed);
	TP_Camera->SetFieldOfView(ZoomThisFrame);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	/** movement  */
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	/** movement  */
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);

	/** jump  */
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASCharacter::StopJumping);

	/** crouch  */
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	/** zoom  */
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::NextFOV);

	/** fire  */
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
	
}

void ASCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	bJumpedLastFrame = true;
}

void ASCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	bJumpedLastFrame = false;
}

void ASCharacter::MoveForward(float Value)
{
	if (Value != 0.f)
	{
		AddMovementInput(GetActorForwardVector() * Value);
	}
}

void ASCharacter::MoveRight(float Value)
{
	if (Value != 0.f)
	{
		AddMovementInput(GetActorRightVector() * Value);
	}
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::OnDeath_Implementation()
{
	bDead = true;
	DetachFromControllerPendingDestroy();

	GetMesh()->SetCollisionProfileName("Ragdoll");
	
	// disable movement
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);
	
	// disable collisions on the capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	SetLifeSpan(1.5f);
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		CurrentWeapon->SetLifeSpan(1.f);
	}
}

void ASCharacter::NextFOV()
{
	if (FOVState == EFOVState::FS_Sniper)
	{
		FOVState = EFOVState::FS_None;
	}
	else if (FOVState == EFOVState::FS_None)
	{
		FOVState = EFOVState::FS_Simple;
	} 
	else if(FOVState == EFOVState::FS_Simple)
	{
		FOVState = EFOVState::FS_Sniper;
	}
}

void ASCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponTemplate)
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			CurrentWeapon = World->SpawnActor<ASWeapon>(DefaultWeaponTemplate, FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
			if (CurrentWeapon)
			{
				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponAttachSocketName);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultWeaponTemplate == NULL"));
	}
}

void ASCharacter::StartFireAI(float FiringTime)
{
	StartFire();

	/** stop fire with delay  */
	FTimerDelegate StopFireDelegate;
	StopFireDelegate.BindLambda([&] { StopFire();  });
	
	FTimerHandle StopFireTimer;
	GetWorldTimerManager().SetTimer(StopFireTimer, StopFireDelegate, FiringTime, false);
}

void ASCharacter::GetHUDData(FText& AmmoText, FText& ScoreText, FText& PingText, float& HealthPercent) const
{
	int32 Ammo = CurrentWeapon ? CurrentWeapon->GetAmmo() : 0;

	AmmoText = FText::FromString(FString::Printf(TEXT("Ammo : %d"), Ammo));
	
	if (SPlayerState)
	{
		ScoreText = FText::FromString(FString::Printf(TEXT("Score : %d"), (int32)SPlayerState->GetScore()));
		PingText = FText::FromString(FString::Printf(TEXT("Ping : %d"), (int32)SPlayerState->GetExactPing()));
	}

	if (HealthComp)
	{
		HealthPercent = HealthComp->GetHealth() / HealthComp->GetDefaultHealth();
	}
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (TP_Camera)
	{
		return TP_Camera->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void ASCharacter::OnHealthChange(USHealthComponent* SHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		OnDeath();
	}
}

/** Returns properties that are replicated for the lifetime of the actor channel */
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDead);
	DOREPLIFETIME(ASCharacter, bJumpedLastFrame);
	DOREPLIFETIME(ASCharacter, AimPitch);
}