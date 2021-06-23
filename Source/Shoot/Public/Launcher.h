// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Launcher.generated.h"

/**
 * 
 */
UCLASS()
class SHOOT_API ALauncher : public AWeapon
{
	GENERATED_BODY()

public:

	ALauncher();
	virtual void StartFire() override;
	virtual void StopFire() override;


protected:

	virtual void BeginPlay() override;
	virtual void Fire() override;
	


	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;

	FTimerHandle TimerHandle_LauncherShoot;
	float LauncherLastTimeFired;


	//Ammunition system
	virtual void ServerAmmoCustomize();

	virtual void ServerShootCustomize();

	virtual void ServerReloadCustomize();

	virtual void ServerReloadDelayCustomize();

	virtual void ServerFireCustomize();
	
};
