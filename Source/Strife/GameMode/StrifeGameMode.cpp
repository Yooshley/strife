// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifeGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Strife/Character/StrifeCharacter.h"
#include "Strife/PlayerController/StrifePlayerController.h"
#include "Strife/PlayerState/StrifePlayerState.h"

void AStrifeGameMode::PlayerEliminated(class AStrifeCharacter* EliminatedCharacter,
                                       class AStrifePlayerController* EliminatedController, class AStrifePlayerController* ExecutionerController)
{
	AStrifePlayerState* ExecutionerPlayerState = ExecutionerController ? Cast<AStrifePlayerState>(ExecutionerController->PlayerState) : nullptr;
	AStrifePlayerState* EliminatedPlayerState = EliminatedController ? Cast<AStrifePlayerState>(EliminatedController->PlayerState) : nullptr;

	if(ExecutionerPlayerState && ExecutionerPlayerState != EliminatedPlayerState)
	{
		ExecutionerPlayerState->AddScore(1.f);
	}

	if(EliminatedPlayerState)
	{
		EliminatedPlayerState->AddDeaths(1);
	}
	
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Death();
	}
}

void AStrifeGameMode::RequestRespawn(class ACharacter* EliminatedCharacter,
	class AController* EliminatedController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if(EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}
