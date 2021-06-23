// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyGameState.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
//uint8 is because it is blueprint supported, when there is no uint8 it will not compile this
enum class EWaveState : uint8
{
	WaitingToStart,

	WaveInProgress,

	//No longer spawning bots, waiting players kill remaining bots
	WaitingToComplete,

	WaveComplete,

	GameOver
};


UCLASS()
class SHOOT_API AMyGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	//this will rep to others this gamestate
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	//this will be spawning effects and do other things in OnRep_WaveState()
	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	//variable that will contain info about the situation in MyGameMode
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;
public:
	void SetWaveState(EWaveState NewState);
	
	EWaveState GetWaveState();
	
};
