// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "AmmoDrop.generated.h"

/**
 * 
 */
UCLASS()
class SHOOT_API AAmmoDrop : public APickup
{
	GENERATED_BODY()
		
public:

	AAmmoDrop();

protected:

	virtual void Interacted() override;

	
};
