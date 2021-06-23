// Fill out your copyright notice in the Description page of Project Settings.
//Репликатэд это не значит, что он будет повторятся у всех, это значит, что он будет повторятся только у самого персонажа
//Возможно, когда вызывается функция на клиенте, но выполняется она на сервере, клиент отдает данные, с которыми работает сервер, а сервер все время сам все просматривает и выполняет чужие функции
#include "Weapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraShake.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Shoot.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Pickup.h"

int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw debug lines for weapons"),
	ECVF_Cheat);


// Sets default values
AWeapon::AWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 20.0f;

	RateOfFire = 600;

	ReloadTime = 1.0f;
	BulletSpread = 2.0f;
	bCanFire = true;

	SetReplicates(true);

	NetUpdateFrequency = 100.0f;
	MinNetUpdateFrequency = 60.0f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;
	if (Role < ROLE_Authority)
	{
		ServerAmmo();
	}
	//Removed because of some bags with dedicated server, remove commentary when you play in listen server
	/*if (Role == ROLE_Authority)
	{
		LoadedAmmo = 30;
		AmmoPool = 30;
	}*/
	
}

void AWeapon::Fire()
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
	//Removed because of some bags with dedicated server, remove commentary when you play in listen server
	/*if (Role == ROLE_Authority)
	{
		LoadedAmmo -= 1;
		PreLoadedAmmo = LoadedAmmo;
	}*/
	//Getting owner of the gun or actor
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
			//Getting Owner's eyes view
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
			//Shot direction
			FVector ShotDirection = EyeRotation.Vector();

			//Bullet spread
			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
			//How long the bullet will be flying
			FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);

			//Adding Collision params. Which object should this function ignore or no
			FCollisionQueryParams Query;
			Query.AddIgnoredActor(MyOwner);
			Query.AddIgnoredActor(this);
			Query.bTraceComplex = true;
			Query.bReturnPhysicalMaterial = true;

			// Bullet end point
			FVector TracerEndPoint = TraceEnd;

			EPhysicalSurface SurfaceType = SurfaceType_Default;
			// Where we hit
			FHitResult Hit;
			//Tracing the line or traectory of the bullet
			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, Query))
			{
				bCanFire = false;
				// Getting Actor that we hit
				AActor* HitActor = Hit.GetActor();

				//Adding impact effect
				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
				//Setting headshot damage multiplier
				float ActualDamage = BaseDamage;
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 4.0f;
				}
				//Applying damage	
				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

				PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

				TracerEndPoint = Hit.ImpactPoint;
		    }
			// Drawing line of the bullet traectory
			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Yellow, false, 1.0f, 0, 1.0f);
			}
			//Playing fire effects
			PlayFireEffects(TracerEndPoint);

			LastTimeFired = GetWorld()->TimeSeconds;

			if (Role == ROLE_Authority)
			{
				HitScanTrace.TraceTo = TracerEndPoint;
				HitScanTrace.SurfaceType = SurfaceType;
			}
	}
}



// Adding effects that will be playing, when we shoot
void AWeapon::PlayFireEffects(FVector EndPoint)
{
	
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, EndPoint);
		}
	}
	// Adding camera shaking when shooting
	APawn* GunOwner = Cast<APawn>(GetOwner());
	if (GunOwner)
	{
		APlayerController* PC = Cast<APlayerController>(GunOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(CamShake);
		}
	}

}

void AWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastTimeFired + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_BetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}
void AWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BetweenShots);

}
void AWeapon::Reload()
{
	if (Role < ROLE_Authority)
	{
		ServerReload();
	}
	//Removed because of some bags with dedicated server, remove commentary when you play in listen server
	
	/*if (Role == ROLE_Authority)
	{
		if (LoadedAmmo == PreLoadedAmmo)
		{
			bCanFire = true;
		}
	}*/
	GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &AWeapon::ReloadFuncForDelay, ReloadTime, false);
}
void AWeapon::ReloadFuncForDelay()
{ 
		if(bCanFire)
		{
			if (Role < ROLE_Authority)
			{
				ServerReloadDelay();
			}
			//Removed because of some bags with dedicated server, remove commentary when you play in listen server
			
			/*if (Role == ROLE_Authority)
			{
				if (AmmoPool == 0 || LoadedAmmo >= 30)
				{
					return;
				}
				if (AmmoPool < (30 - LoadedAmmo))
				{
					LoadedAmmo += AmmoPool;
					AmmoPool = 0;
				}
				else
				{
					AmmoPool = AmmoPool - (30 - LoadedAmmo);
					LoadedAmmo = 30;
				}
			}*/
		}
		GetWorldTimerManager().ClearTimer(TimerHandle_Reload);
}

void AWeapon::ServerFire_Implementation()
{
	ServerFireCustomize();
}

void AWeapon::ServerFireCustomize()
{
	Fire();
}
bool AWeapon::ServerFire_Validate()
{
	return true;
}

void AWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME(AWeapon, LoadedAmmo)
	DOREPLIFETIME(AWeapon, AmmoPool)
	DOREPLIFETIME(AWeapon, PreLoadedAmmo);
	DOREPLIFETIME(AWeapon, bCanFire);
}


void AWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT://Everything else
	case SURFACE_FLESHVULNERABLE://Head
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}
	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}
// Setting Ammo variables in beginplay()
void AWeapon::ServerAmmo_Implementation()
{
	ServerAmmoCustomize();
}

void AWeapon::ServerAmmoCustomize() 
{
	LoadedAmmo = 30;
	AmmoPool = 30;
}

bool AWeapon::ServerAmmo_Validate()
{
	return true;
}
//Setting ammo variables when shooting
void AWeapon::ServerShoot_Implementation()
{
	ServerShootCustomize();
}

void AWeapon::ServerShootCustomize()
{
	LoadedAmmo -= 1;
	PreLoadedAmmo = LoadedAmmo;
}
bool AWeapon::ServerShoot_Validate()
{
	return true;
}
//Setting ammo variables when checking in Reload() func 
void AWeapon::ServerReload_Implementation()
{
	ServerReloadCustomize();
}

void AWeapon::ServerReloadCustomize()
{
	if (LoadedAmmo == PreLoadedAmmo)
	{
		bCanFire = true;
	}
}

bool AWeapon::ServerReload_Validate()
{
	return true;
}
//ReloadFuncForDelay itself on server
void AWeapon::ServerReloadDelay_Implementation()
{
	ServerReloadDelayCustomize();
}

void AWeapon::ServerReloadDelayCustomize()
{
	if (AmmoPool == 0 || LoadedAmmo >= 30)
	{
		return;
	}
	if (AmmoPool < (30 - LoadedAmmo))
	{
		LoadedAmmo += AmmoPool;
		AmmoPool = 0;
	}
	else
	{
		AmmoPool = AmmoPool - (30 - LoadedAmmo);
		LoadedAmmo = 30;
	}
}
bool AWeapon::ServerReloadDelay_Validate()
{
	return true;
}

void AWeapon::SetLoadedAmmo(int32 NewLoad)
{
	if (Role == ROLE_Authority)
	{
		LoadedAmmo = NewLoad;
	}
}

