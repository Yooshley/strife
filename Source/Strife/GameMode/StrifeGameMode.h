// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "StrifeGameMode.generated.h"

/**
 * 
 */
UCLASS()
class STRIFE_API AStrifeGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class AStrifeCharacter* EliminatedCharacter, class AStrifePlayerController* EliminatedController, class AStrifePlayerController* ExecutionerController);

	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController);
};
