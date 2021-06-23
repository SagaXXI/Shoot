// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TrackerBot.generated.h"

UCLASS()
class SHOOT_API ATrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATrackerBot();

protected:

	//Starter Drop for AdvancedAI
	UPROPERTY(Replicated)
		class AAmmoDrop* StarterDrop;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
		TSubclassOf <AAmmoDrop> DropToCreate;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly)
	class USHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class URadialForceComponent* RadialForce;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
			const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	FVector GetPathNextPoint();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	FVector NextPathPoint;

	UPROPERTY(Replicated)
	UMaterialInstanceDynamic* MatInst;
	//Func which runs when we get shooted
	UFUNCTION()
	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UParticleSystem* ExplosionEffect;

	bool bExploded;

	bool bStartedToBoom;

	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float SelfDamageInterval;

	//Adding Sounds
	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	class USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	class USoundCue* SelfDamageSound;
	//Boom timer
	FTimerHandle TimerHandle_SelfDamage;
	
	//Kamikadze function
	UFUNCTION()
	void SelfDamage();
	//func that checks to nearby bots
	void OnCheckNearbyBots();
	//damage multiplier
	int32 PowerLevel;
	UPROPERTY(Replicated)
	float Alpha;
	//Replicate LastTimeDamageTaken param
	UFUNCTION(NetMulticast, Reliable)
	void ChangeLastTimeDamageTaken();
	//Replicate ParamLevelAlpha param
	UFUNCTION(NetMulticast, Reliable)
	void ChangeParamLevelAlpha();

	//Refresh PathPoint
	FTimerHandle TimerHandle_RefreshPath;

	//void func for TimerHandle_RefreshPath
	void RefreshPath();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//this func already exists on OnActorOverlap, so we just override it to make our lives easier and to not create a new func
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	

};
