// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Components/StaticMeshComponent.h"
#include "SCharacter.h"
#include "Components/BoxComponent.h"
#include "Engine.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

//Just idea:
//Try to replace Weapon from SCharacter to AmmoDrop or do something same

int32 DebugPickupDrawing = 0;
FAutoConsoleVariableRef CVARDebugPickupDrawing(TEXT("COOP.DebugPickup"),
	DebugPickupDrawing,
	TEXT("Draw debug lines for Pickup"),
	ECVF_Cheat);
	

// Sets default values
APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;

	//it is UBoxComponent here and UShapeComponent in header file, because we can choose which shape it will be in CreateDefaultSubobject
	Box = CreateDefaultSubobject<UBoxComponent>("BoxComp");
	Box->SetGenerateOverlapEvents(true);
	RootComponent = Box;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);

	PlayerCharacter = Cast<ASCharacter>(GetOwner());

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	if (Role == ROLE_Authority)
	{
		Box->OnComponentBeginOverlap.AddDynamic(this, &APickup::TriggerEnter);
		Box->OnComponentEndOverlap.AddDynamic(this, &APickup::TriggerExit);
	}
}

void APickup::Interacted()
{
	MyPlayerController->Inventory.Add(this);
	if (DebugPickupDrawing)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("You picked the item up!"));
	}
}

void APickup::TriggerEnter( UPrimitiveComponent* OverlappedComponent,  AActor* OtherActor,  UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	bItemWithinRange = true;
	if (DebugPickupDrawing)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, FString::Printf(TEXT("Press E to pickup the item %s"), *ItemName));
	}
	SetPlayer(OtherActor);
}

void APickup::TriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	bItemWithinRange = false;
}


void APickup::SetPlayer(AActor* Player)
{
	MyPlayerController = Cast<ASCharacter>(Player);
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MyPlayerController != nullptr)
	{
		if (MyPlayerController->bIsPickingUp && bItemWithinRange)
		{
			if (Role < ROLE_Authority)
			{
				ServerInteracted();
			}
		}
	}
}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, MyPlayerController);
	DOREPLIFETIME(APickup, bItemWithinRange);
}

void APickup::ServerInteracted_Implementation()
{
	Interacted();
}

bool APickup::ServerInteracted_Validate()
{
	return true;
}

FString APickup::GetItemName()
{
	return ItemName;
}


