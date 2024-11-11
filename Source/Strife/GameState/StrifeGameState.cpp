// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifeGameState.h"

#include "Net/UnrealNetwork.h"
#include "Strife/PlayerState/StrifePlayerState.h"

void AStrifeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStrifeGameState, TopScoringPlayerStates);
}

void AStrifeGameState::UpdateTopScore(class AStrifePlayerState* ScoringPlayerState)
{
	if(TopScoringPlayerStates.Num() == 0)
	{
		TopScoringPlayerStates.Add(ScoringPlayerState);
		TopScore = ScoringPlayerState->GetScore();
	}
	else if(ScoringPlayerState->GetScore() == TopScore)
	{
		TopScoringPlayerStates.AddUnique(ScoringPlayerState);
	}
	else if(ScoringPlayerState->GetScore() > TopScore)
	{
		TopScoringPlayerStates.Empty();
		TopScoringPlayerStates.AddUnique(ScoringPlayerState);
		TopScore = ScoringPlayerState->GetScore();
	}
}
