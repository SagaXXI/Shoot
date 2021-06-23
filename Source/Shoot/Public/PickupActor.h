// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupActor.generated.h"

UCLASS()
class SHOOT_API APickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	class UDecalComponent* DecalComp;

	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* SphereComp;

	//Spawn powerup
	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<class APowerupActor> PowerupClass;

	//powerup itself
	class APowerupActor* PowerupInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CooldownDuration;

	//Timer to Respawn()
	FTimerHandle TimerHandle_Respawn;

	//This will be called when we've picked up the powerup and after cooldown we will respawn this like in overwatch first aid kit
	void Respawn();





public:	

	//this func already exists on OnActorOverlap, so we just override it to make our lives easier and to not create a new func
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
