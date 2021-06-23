// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosionBarrel.generated.h"

UCLASS()
class SHOOT_API AExplosionBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosionBarrel();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class URadialForceComponent* RadialForce;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USHealthComponent* HealthComp;

	UFUNCTION()
	void WhenHealthChanged(USHealthComponent* OwnHealthComp, float Health, float HealthDelta,
			const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing = OnRep_PlayVisualEffects)
	bool bExploded;

	UFUNCTION()
	void OnRep_PlayVisualEffects();

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float ExplosionImpulse;
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UParticleSystem* ExplosionEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UMaterialInterface* ExplosionMaterial;

	UPROPERTY(EditDefaultsOnly)
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly)
	float ExplosionRadius;

public:	

};
