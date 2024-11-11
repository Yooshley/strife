// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "StrifeGameState.generated.h"

/**
 * 
 */
UCLASS()
class STRIFE_API AStrifeGameState : public AGameState
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AStrifePlayerState* ScoringPlayerState);
	
	UPROPERTY(Replicated)
	TArray<AStrifePlayerState*> TopScoringPlayerStates;

private:
	float TopScore = 0.f;
};
