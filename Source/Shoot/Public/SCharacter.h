// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"


UCLASS()
class SHOOT_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();
	//For picking up ammo crate and adding ammo
	void SetAmmoPool(float Ammo);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerSetAmmoPool(float Ammo);
	//Inventory
	UPROPERTY(Replicated)
	TArray<class APickup*> Inventory;


	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void ACrouch();
	void StopCrouching();
	//avoiding player to crouch while jumping
	FTimerHandle TimerHandle_Crouch;
	void CrouchAfterJumping();

	void BeginZoom();
	void StopZoom();

	//Interaction func, Inventory and etc.
	void Interact();

	void StopInteracting();

	void ShowInventory();

	//Try to do it later
	
	UFUNCTION()
		void DropItem(APickup* Item);

	UFUNCTION()
		void DropAllItems();
	

	
	//Starter Drop for AdvancedAI
	UPROPERTY(Replicated)
	class AAmmoDrop* StarterDrop;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TSubclassOf <AAmmoDrop> DropToCreate;

	



	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArmComp;

	//Spawning Weapon
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Weapon")
		class AWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "SpawnWeapon")
		TSubclassOf<AWeapon> WeaponToSpawn;


	//Creating Weapon Zoom
	bool bWantsToZoom;
	

	UPROPERTY(EditDefaultsOnly, Category = "Zoom")
	float ZoomedFOV;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Zoom")
	float ZoomInterpSpeed;

	//Reload function
	
	void Reload();
	

	//Health system
	UPROPERTY(EditDefaultsOnly)
	class USHealthComponent* HealthComp;

	UFUNCTION()
	void WhenHealthChanging(USHealthComponent* OwnHealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;
	
	//Run/Sprint functions
	void StartRunning();

	void StopRunning();


	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float SpeedMultiplier;



public:	

	UPROPERTY(Replicated)
	bool bIsPickingUp = false;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// Setting the bullet to come from camera
	virtual FVector GetPawnViewLocation() const override;

	// Replicating CurrentWeapon variable
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void Heal(float HP);

	//Fire functions
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Character")
	void RefillSelf(float Ammo);
};
