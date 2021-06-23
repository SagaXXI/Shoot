// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoDrop.h"
#include "SCharacter.h"
//#include "Net/UnrealNetwork.h"

AAmmoDrop::AAmmoDrop()
{
}

void AAmmoDrop::Interacted()
{
	Super::Interacted();
	MyPlayerController->ServerSetAmmoPool(FMath::FRandRange(1, 15));
	Destroy(true);

}

