// Fill out your copyright notice in the Description page of Project Settings.
//refillpointsplayer() refills to allplayers that's why ammo is doubling

#include "MyGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "MyGameState.h"
#include "MyPlayerState.h"
#include <Runtime\Engine\Classes\Engine\World.h>
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "SCharacter.h"


AMyGameMode::AMyGameMode()
{
	//set default value of TimeBetweenWaves	
	TimeBetweenWaves = 2.0f;

	//Setting GameState class that's variable is already existing
	GameStateClass = AMyGameState::StaticClass();

	//Setting PlayerState class that's variable is already existing
	PlayerStateClass = AMyPlayerState::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	//changes the tick interval
	PrimaryActorTick.TickInterval = 1.0f;
	//check the refilllllllll
	DidRefill = false;
}

void AMyGameMode::StartWave()
{
	//increases the number of waves
	WaveCount++;

	//increases the number of bots to spawn
	NrOfBotsToSpawn = 2 + WaveCount - 1;

	//Start timer to loop func, while NrOfBotsToSpawn > 0
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &AMyGameMode::SpawnBotTimerFunc, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);

}

void AMyGameMode::EndWave()
{
	//Stop spawning bots
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);

	DidRefill = false;

}

void AMyGameMode::PrepareToNextWave()
{
	//Refill and respawn players
	if (GetWaveState() == EWaveState::WaveComplete && !DidRefill)
	{
		DidRefill = true;
		//Refill players
		PlayerPointsRefill();
		//Respawn dead players
		RestartDeadPlayers();
	}
	//Start countdown to next wave
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &AMyGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);
}

void AMyGameMode::StartPlay()
{
	Super::StartPlay();

	//Starts first wave
	PrepareToNextWave();
}

void AMyGameMode::SpawnBotTimerFunc()
{
	//Spawns bot while NrOfBotsToSpawn > 0;
	SpawnNewBot();

	NrOfBotsToSpawn--;

	// if NrOfBotsToSpawn is less than 0 stops the TimerHandle_BotSpawner
	if (NrOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}

void AMyGameMode::CheckWaveState()
{

	//Check if there is no PrepareToNextWave() running
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	//If NrOfBotsToSpawn is not 0 and IsPreparingForWave = true - return
	if (NrOfBotsToSpawn > 0 && bIsPreparingForWave)
	{
		return;
	}

	//this will contain info about if there is any bot alive or not
	bool bIsAnyBotAlive = false;

	//this will check by perebor (brute force) is there any pawn alive
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		//Get the pawn
		APawn* TestPawn = It->Get();
		//check if TestPawn is not nullptr and is not controlled by player
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			//skip this pawn
			continue;
		}
		//Get HealthComp of Pawn
		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		//Check if HealthComp not nullptr and if health is higher than 0
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			//this bot is alive so we don't need to continue this func
			bIsAnyBotAlive = true;
			break;
		}
		//if there is no bots alive starts next wave
		if (!bIsAnyBotAlive && NrOfBotsToSpawn <= 0)
		{
			SetWaveState(EWaveState::WaveComplete);
			PrepareToNextWave();
		}
	}
}

void AMyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Check every sec is any bot alive
	CheckWaveState();

	//Check every sec is any player alive
	CheckAnyPlayerAlive();
}

void AMyGameMode::CheckAnyPlayerAlive()
{
	//this will check by perebor (brute force) is there any PlayerController (with that we can know if there are any Player alive) exist
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		//Get's a PlayerController
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
			{
				DeadPlayersCount++;
			}
	}

	//in future you will need to find out, how to know how many players are playing this game now (find real func)
	if (DeadPlayersCount >= 2)
	{
		GameOver();
	}
	else
	{
		return;
	}
}

void AMyGameMode::GameOver()
{
	//Stop the wave and spawning new bots
	EndWave();

	SetWaveState(EWaveState::GameOver);
	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players are died"));
}

void AMyGameMode::SetWaveState(EWaveState NewState)
{
	//Get GameState
	AMyGameState* GS = GetGameState<AMyGameState>();
	//ensureAlways() is like ensure, but it will do it always
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}

}

EWaveState AMyGameMode::GetWaveState()
{
	EWaveState CurrentWaveState;
	//Get GameState
	AMyGameState* GS = GetGameState<AMyGameState>();
	//ensureAlways() is like ensure, but it will do it always
	if (ensureAlways(GS))
	{
		 CurrentWaveState = GS->GetWaveState();
	}
	return CurrentWaveState;
}

void AMyGameMode::RestartDeadPlayers()
{
	if (Role == ROLE_Authority)
	{
		//this will check by perebor (brute force) is there any PlayerController (with that we can know is any player dead)
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			//Get's a PlayerController
			APlayerController* PC = It->Get();
			//PC->GetPawn() == nullptr, because this means that player is dead
			if (PC && PC->GetPawn() == nullptr)
			{
				DeadPlayersCount--;
				RestartPlayer(PC);
				Cast<ASCharacter>(PC->GetCharacter())->Heal(100);
			}
		}
	}
}

void AMyGameMode::PlayerPointsRefill()
{
	if (Role == ROLE_Authority)
	{
		//this will multiply the ammo that will be given to our players
		float PlusAmmo = 60 + (WaveCount * 10);

		//this will check by perebor (brute force) is there any PlayerController (with that we can get player)
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			//Get's a PlayerController
			APlayerController* PC = It->Get();
			//Check if PC is valid
			if (PC && PC->GetCharacter())
			{
				//get character and cast it to ASCharacter
				ASCharacter* Player = Cast<ASCharacter>(PC->GetCharacter());
				//Get HealthComp
				USHealthComponent* HealthComp = Cast<USHealthComponent>(Player->GetComponentByClass(USHealthComponent::StaticClass()));

				if (HealthComp && HealthComp->GetHealth() > 0.0f)
				{
					Player->RefillSelf(PlusAmmo);
				}
			}
		}
	}
}