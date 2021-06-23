// Fill out your copyright notice in the Description page of Project Settings.


#include "MedicineChest.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AMedicineChest::AMedicineChest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthPoint = 20.0f;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetSphereRadius(45.0f, false);
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComponent);
	
}

// Called when the game starts or when spawned
void AMedicineChest::BeginPlay()
{
	Super::BeginPlay();
	
	if (Role == ROLE_Authority)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMedicineChest::OnPickup);
	}
}

// Called every frame
void AMedicineChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMedicineChest::OnPickup(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ASCharacter* Character = Cast<ASCharacter>(OtherActor);

	if (Character)
	{
		Character->Heal(HealthPoint);

		this->Destroy();
	}
}
