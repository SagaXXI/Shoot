// Fill out your copyright notice in the Description page of Project Settings.


#include "Launcher.h"
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>
#include <Runtime\Engine\Classes\Engine\World.h>

//try to fix bug with damage, add PlayFireEffects() (CameraShake espacially), RateOfFire, automatic fire, and fix why it is not shooting

ALauncher::ALauncher()
{
	SetReplicates(true);
}

void ALauncher::BeginPlay()
{
	TimeBetweenShots = 60 / RateOfFire;
	if (Role == ROLE_Authority)
	{
		ServerAmmoCustomize();
	}
}
void ALauncher::Fire()
{
	if (LoadedAmmo == 0)
	{
		return;
	}
	if (Role < ROLE_Authority)
	{
		ServerShoot();
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner && ProjectileClass)
	{
		//Getting Owner's eyes view
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		//Getting socket location
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		// Setting spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        // Spawning projectiles like junkrat weapon
		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation,EyeRotation, SpawnParams);

		LauncherLastTimeFired = GetWorld()->TimeSeconds;
	}
}

void ALauncher::ServerAmmoCustomize()
{
	LoadedAmmo = 6;
	AmmoPool = 18;
}

void ALauncher::ServerShootCustomize()
{
	LoadedAmmo -= 1;
	PreLoadedAmmo = LoadedAmmo;
}

void ALauncher::ServerReloadCustomize()
{
	if (LoadedAmmo == PreLoadedAmmo)
	{
		bCanFire = true;
	}                     
}

void ALauncher::ServerReloadDelayCustomize()
{
	if (AmmoPool == 0 || LoadedAmmo >= 6)
	{
		return;
	}
	if (AmmoPool < (6 - LoadedAmmo))
	{
		LoadedAmmo += AmmoPool;
		AmmoPool = 0;
	}
	else
	{
		AmmoPool = AmmoPool - (6 - LoadedAmmo);
		LoadedAmmo = 6;
	}
}

void ALauncher::ServerFireCustomize()
{
	this->Fire();
}

void ALauncher::StartFire()
{
	float FirstDelay = FMath::Max(LauncherLastTimeFired + this->TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_LauncherShoot, this, &ALauncher::Fire, this->TimeBetweenShots, true, FirstDelay);
}

void ALauncher::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_LauncherShoot);
}

