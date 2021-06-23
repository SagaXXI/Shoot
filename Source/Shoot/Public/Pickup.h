// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class SHOOT_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	//Getter for ItemName pointer
	FString GetItemName();

	UPROPERTY(EditAnywhere)
		FString ItemName = FString(TEXT(""));



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

	UPROPERTY(EditDefaultsOnly, Category = "CollisionShape")
	class UBoxComponent* Box;


	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* MeshComp;

	//Player who will be looting
	UPROPERTY(Replicated)
    class ASCharacter* MyPlayerController;

	//For casting owner to SCharacter
	ASCharacter* PlayerCharacter;

	

	//Func that will be called when player interacts with this
	virtual void Interacted();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void ServerInteracted();

	//Sets MyPlayerController
	void SetPlayer(AActor* Player);

	UPROPERTY(Replicated)
	bool bItemWithinRange = false;

	UFUNCTION()
		void TriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void TriggerExit(UPrimitiveComponent* OverlappedComponent,  AActor* OtherActor,  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);



public:

	virtual void Tick(float DeltaTime) override;

	

};
