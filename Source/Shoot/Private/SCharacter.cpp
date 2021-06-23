// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Weapon.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Shoot.h"
#include "SHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Pickup.h"
//this might be useful when you will decide to add some animation or wtf that girl from tutorial said to me
#include "Animation/SkeletalMeshActor.h"
#include "GameFramework/Actor.h"
#include "Engine.h"
#include "AmmoDrop.h"
#include "Pickup.h"

//try to fix Launcher_BP



// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	
	ZoomedFOV = 65.0f;

	ZoomInterpSpeed = 20.0f;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	NetUpdateFrequency = 100.0f;
	MinNetUpdateFrequency = 60.0f;

	SpeedMultiplier = 2.0f;

	
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	//Setting the default FOV
	DefaultFOV = CameraComp->FieldOfView;

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::WhenHealthChanging);

	if (Role == ROLE_Authority)
	{
		// Spawning Weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "GunSocket");
		}
	}

	//Try to do it later
	//Spawn and disable drop
	/*APawn* PlayerPawn = Cast<APawn>(this);
	if (Role == ROLE_Authority && PlayerPawn && !PlayerPawn->IsControlled())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		StarterDrop = GetWorld()->SpawnActor<AAmmoDrop>(DropToCreate, GetActorLocation(), GetActorRotation(), SpawnParams);

		if (StarterDrop)
		{
			//StarterDrop->SetActorHiddenInGame(true);
			StarterDrop->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "BauchSocket");


			Inventory.Add(StarterDrop);
		}
	}*/
	
}
// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Creating Zoom and UnZoom system
	float CurrentFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, CurrentFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);
	
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}
//Crouch func
void ASCharacter::ACrouch()
{
	if (!GetMovementComponent()->IsFalling() && !GetMovementComponent()->IsFlying())
	{
		Crouch();
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerHandle_Crouch, this, &ASCharacter::CrouchAfterJumping, 0.2, true);
	}
	
	
}

void ASCharacter::StopCrouching()
{
	UnCrouch();
}
//Zooming func
void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::StopZoom()
{
	bWantsToZoom = false;
}
// Fire function
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
//Reload function
void ASCharacter::Reload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

// Setting the bullet to come from camera
FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}
void ASCharacter::SetAmmoPool(float Ammo)
{
	if (Role == ROLE_Authority)
	{
		CurrentWeapon->AmmoPool += Ammo;
	}
}

void ASCharacter::ServerSetAmmoPool_Implementation(float Ammo)
{
	SetAmmoPool(Ammo);
}

bool ASCharacter::ServerSetAmmoPool_Validate(float Ammo)
{
	return true;
}
void ASCharacter::WhenHealthChanging(USHealthComponent* OwnHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		//Die!
		bDied = true;

		//Try to do it later
		/*if (StarterDrop)
		{
			StarterDrop->SetActorHiddenInGame(false);
			DropAllItems();
		}*/
		

		GetCharacterMovement()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		APawn* PlayerPawn = Cast<APawn>(this);
		if (PlayerPawn && !PlayerPawn->IsControlled())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			StarterDrop = GetWorld()->SpawnActor<AAmmoDrop>(DropToCreate, CurrentWeapon->GetActorLocation(),
				CurrentWeapon->GetActorRotation(), SpawnParams);

			if (StarterDrop)
			{
				CurrentWeapon->Destroy();

				DropAllItems();
				DetachFromControllerPendingDestroy();
				SetLifeSpan(3.0f);

			}
		}
		else
		{
			CurrentWeapon->Destroy();
			DropAllItems();

			DetachFromControllerPendingDestroy();
			SetLifeSpan(3.0f);
		}
	}
}


// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//Binding movement
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::ACrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::StopCrouching);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::StopZoom);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::Reload);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::StartRunning);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::StopRunning);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASCharacter::Interact);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ASCharacter::StopInteracting);
	PlayerInputComponent->BindAction("ShowInventory", IE_Pressed, this, &ASCharacter::ShowInventory);

}

void ASCharacter::Heal(float HP)
{
	if (Role == ROLE_Authority)
	{
		HealthComp->Heal(HP);
	}
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
	DOREPLIFETIME(ASCharacter, Inventory);
	DOREPLIFETIME(ASCharacter, bIsPickingUp);
	//Try to do it later
	DOREPLIFETIME(ASCharacter, StarterDrop);
}

void ASCharacter::StartRunning()
{
	
		GetCharacterMovement()->MaxWalkSpeed *= SpeedMultiplier;
	
}

void ASCharacter::StopRunning()
{
	
		GetCharacterMovement()->MaxWalkSpeed /= SpeedMultiplier;
}

//Player can't jump while crouching, this func will do crouch after jumping
void ASCharacter::CrouchAfterJumping()
{
	if (!GetMovementComponent()->IsFlying() && !GetMovementComponent()->IsFalling())
	{
		Crouch();
		GetWorldTimerManager().ClearTimer(TimerHandle_Crouch);
	}
}

void ASCharacter::RefillSelf(float Ammo)
{
	Heal(100.0f);
	SetAmmoPool(Ammo);
}

void ASCharacter::Interact()
{
		bIsPickingUp = true;
}

void ASCharacter::StopInteracting()
{
		bIsPickingUp = false;
}

void ASCharacter::ShowInventory()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("This is your inventory:  "));
	for (auto& Item : Inventory)
	{
		FString ItemN = Item->ItemName;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Item: %s"), *ItemN));
	}
	
}

//Try to do it later

void ASCharacter::DropItem(APickup* Item)
{
	if (Role == ROLE_Authority)
	{
		FVector Location = GetActorLocation();
		//Randomize loot location
		Location.X += FMath::RandRange(-50.f, 100.0f);
		Location.Y += FMath::RandRange(-50.f, 100.0f);

		this->SetActorLocation(Location);
		Inventory.RemoveSingle(Item);

	}

}

void ASCharacter::DropAllItems()
{
	if (Role == ROLE_Authority)
	{
		for (APickup* Item : Inventory)
		{
			DropItem(Item);
		}
	}
}

