// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerupActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APowerupActor::APowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;

	bIsPowerupActive = false;

	SetReplicates(true);
}


void APowerupActor::OnTickPowerup()
{
	//schetchik
	TicksProcessed++;

	//effect that will be called, when this is like long effect (health regeneration for example)
	OnPowerupTicked();

	//last tick
	if (TicksProcessed >= TotalNrOfTicks)
	{
		//this called when powerup's effect ends
		OnExpired();
		//to set visibility true again
		bIsPowerupActive = false;
		OnRep_PowerupActive();

		//Stop ticking
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void APowerupActor::ActivatePowerup(AActor* ActivatedFor)
{
	OnActivated(ActivatedFor);

	//to remove static mesh's visibility
	//here we call it on server, because it calls on clients but to call it on server, we need to do it by ourselves
	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		//Start ticking
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &APowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		//if this should run only once
		OnTickPowerup();
	}
}

void APowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void APowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerupActor, bIsPowerupActive);
	

}