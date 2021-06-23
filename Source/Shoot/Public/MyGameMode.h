// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameMode.generated.h"


enum class EWaveState : uint8;

//This event will run when someone got killed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, KilledActor, AActor*, KillerActor, AController*, KillerController);


UCLASS()
class SHOOT_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	//timer for spawn bots in one wave 
	FTimerHandle TimerHandle_BotSpawner;
	//Countdown for StartWave()
	FTimerHandle TimerHandle_NextWaveStart;

	//Count of dead players
	int32 DeadPlayersCount;
	//number of bots to spawn
	int32 NrOfBotsToSpawn;
	
	//count of waves that gone
	int32 WaveCount;

	//Time between waves
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	//BP Event for Spawn
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();
	
	//Func that will be checking for NrOfBotsToSpawn and decreasing it's value  and running SpawnNewBot()
	void SpawnBotTimerFunc();

	//Starts wave and starts timer TimerHandle_BotSpawner
	void StartWave();

	//Stops TimerHandle_BotSpawner and stops spawning new bots
	void EndWave();

	//Starts countdown for next wave
	void PrepareToNextWave();

	//Checks if there any bot alive
	void CheckWaveState();

	//Checks if there any players alive
	void CheckAnyPlayerAlive();

	//func which will be called when game overs
	void GameOver();

	//Setter WaveState
	void SetWaveState(EWaveState NewState);

	EWaveState GetWaveState();

	bool DidRefill;

	//respawn dead players
	void RestartDeadPlayers();

	//func for refilling HP and Ammo of Character
	void PlayerPointsRefill();

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	TSubclassOf <AActor> ClassToFind;
public:

	AMyGameMode();

	//Analog of BeginPlay() for GameMode
	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;
};
