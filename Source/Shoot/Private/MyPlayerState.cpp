// Fill out your copyright notice in the Description page of Project Settings.

//this thing is already existing on every player, but to share the score, we need this, it is like a gamestate, and it's replicating to others by gamemode
#include "MyPlayerState.h"


void AMyPlayerState::AddScore(float ScoreDelta)
{
	Score += ScoreDelta;
}
