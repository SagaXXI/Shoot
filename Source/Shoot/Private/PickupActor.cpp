// Fill out your copyright notice in the Description page of Project Settings.
//APickupActor is responsible for the overlaping/picking up and spawning/respawning APowerup. And APowerupActor is responsible for effects

#include "PickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "PowerupActor.h"


// Sets default values
APickupActor::APickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>("DecalComp");
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetupAttachment(RootComponent);

	CooldownDuration = 10.0f;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void APickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		Respawn();
	}
}


void APickupActor::Respawn()
{
	if (PowerupClass == nullptr)
	{
		// this is only for designer, but i'm solo dev :P
		UE_LOG(LogTemp, Warning, TEXT("PowerupClass is nullptr in %s. Please update your blueprint"), *GetName());

		return;
	}
	//Spawn params
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//Spawn itself
	PowerupInstance = GetWorld()->SpawnActor<APowerupActor>(PowerupClass, GetTransform(), SpawnParams);
}

void APickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	//In the way to not lose some BP functionality
	Super::NotifyActorBeginOverlap(OtherActor);
	//check if player
	APawn* PlayerPawn = Cast<APawn>(OtherActor);

	if (Role == ROLE_Authority && PowerupInstance && PlayerPawn->IsPlayerControlled())
	{
		PowerupInstance->ActivatePowerup(OtherActor);
		PowerupInstance = nullptr;

		//Start delay for respawning
		GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &APickupActor::Respawn, CooldownDuration);
	}
}

