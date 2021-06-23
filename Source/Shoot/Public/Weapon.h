// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
public:
	UPROPERTY()
    TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class SHOOT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	// Adding firing function
	virtual void StartFire();
	virtual void StopFire();
	UFUNCTION(BlueprintCallable)
	void Reload();

	//Creating Ammuntion System
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 AmmoPool;
	
	//called when player dies

protected:
	
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	virtual void Fire();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	class USkeletalMeshComponent* MeshComp;

	// Adding Damage System
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<class UDamageType> DamageType;

	// Creating Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	FName MuzzleSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	FName TracerTargetName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* MuzzleEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* DefaultImpactEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* FleshImpactEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* TracerEffect;
	void PlayFireEffects(FVector EndPoint);
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<class UCameraShake> CamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float BaseDamage;

	FTimerHandle TimerHandle_BetweenShots;
	float LastTimeFired;

	//Bullets per minute
	UPROPERTY(EditDefaultsOnly, Category = "Automatic fire options")
	float RateOfFire;
	float TimeBetweenShots;

	//Bullet spread in degrees
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpread;


	//Creating Ammuntion System
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 LoadedAmmo;
	//If yes can fire, no otherwise
	UPROPERTY(VisibleAnywhere, Replicated)
	bool bCanFire;
	void ReloadFuncForDelay();
	FTimerHandle TimerHandle_Reload;
	UPROPERTY(EditDefaultsOnly, Category = "Automatic fire options")
	float ReloadTime;
	// Gets loadedammo that was fired last time
	UPROPERTY(VisibleAnywhere, Replicated)
	int32 PreLoadedAmmo;
	//Only on client called and executed on server to avoid cheating functions
	//Added Customization funcs, so this will be easier to customize this class, when you are coding child of this class
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAmmo();
	virtual void ServerAmmoCustomize();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShoot();
	virtual void ServerShootCustomize();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();
	virtual void ServerReloadCustomize();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadDelay();
	virtual void ServerReloadDelayCustomize();
	
	//Game Networking
	UFUNCTION( Server, Reliable, WithValidation)
	void ServerFire();
	virtual void ServerFireCustomize();
	
	//Sending Trace Location to server
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();
	
	UFUNCTION()
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	//to set loaded ammo to "infinite" in InfiniteAmmo_BP
	UFUNCTION(BlueprintCallable)
	void SetLoadedAmmo(int32 NewLoad);

	
	
	

public:
	
	
	
	
	

};
