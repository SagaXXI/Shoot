// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoCrate.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AAmmoCrate::AAmmoCrate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	count = 30;

	TouchSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollsionSphere"));
	TouchSphere->SetSphereRadius(45.f, false);
	TouchSphere->bGenerateOverlapEvents = true;
	RootComponent = TouchSphere;

	Mesh = CreateDefaultSubobject <UStaticMeshComponent> (TEXT("Mesh"));
	Mesh->SetupAttachment(TouchSphere);
}

// Called when the game starts or when spawned
void AAmmoCrate::BeginPlay()
{
	Super::BeginPlay();
	if (Role == ROLE_Authority)
	{
		TouchSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmoCrate::OnPickup);
	}
}

// Called every frame
void AAmmoCrate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AAmmoCrate::OnPickup(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ASCharacter* Character = Cast<ASCharacter>(OtherActor);
	if (Character)
	{
		Character->SetAmmoPool(count);
		this->Destroy();
	}
}



