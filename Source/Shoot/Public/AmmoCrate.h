// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "AmmoCrate.generated.h"

UCLASS()
class SHOOT_API AAmmoCrate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmoCrate();
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Ammo Crate")
	class USphereComponent* TouchSphere;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Ammo Crate")
    class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Crate")
	int32 count;
    UFUNCTION()
		void OnPickup(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
