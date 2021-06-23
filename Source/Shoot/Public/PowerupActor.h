// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerupActor.generated.h"

UCLASS()
class SHOOT_API APowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APowerupActor();

protected:

	//interval between ticks
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	//how many times this effect will be ticking
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	//timer for tick
	FTimerHandle TimerHandle_PowerupTick;

	//How many times this effect had ticked
	int32 TicksProcessed;

	UFUNCTION()
	void OnTickPowerup();

	//When runs OnActive() this equals to true
	UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
	bool bIsPowerupActive;

    UFUNCTION()
	void OnRep_PowerupActive();

	//visual effect that will run on both (server, client)
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupStateChanged(bool bIsNewActive);


public:	

	//this parameter is for easy replication. We can know who exactly overlapped this
	void ActivatePowerup(AActor* ActivatedFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated(AActor* ActivatedForPlayer);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();
};
