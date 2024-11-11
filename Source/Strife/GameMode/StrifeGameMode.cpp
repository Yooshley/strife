// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifeGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Strife/Character/StrifeCharacter.h"
#include "Strife/GameState/StrifeGameState.h"
#include "Strife/PlayerController/StrifePlayerController.h"
#include "Strife/PlayerState/StrifePlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AStrifeGameMode::AStrifeGameMode()
{
	bDelayedStart = true;
}

void AStrifeGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartTime = GetWorld()->GetTimeSeconds();
}

void AStrifeGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AStrifePlayerController* StrifePlayerController = Cast<AStrifePlayerController>(*It);
		if(StrifePlayerController)
		{
			StrifePlayerController->OnMatchStateSet(MatchState);
		}
	}
}

void AStrifeGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if(CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AStrifeGameMode::PlayerEliminated(class AStrifeCharacter* EliminatedCharacter,
                                       class AStrifePlayerController* EliminatedController, class AStrifePlayerController* ExecutionerController)
{
	AStrifePlayerState* ExecutionerPlayerState = ExecutionerController ? Cast<AStrifePlayerState>(ExecutionerController->PlayerState) : nullptr;
	AStrifePlayerState* EliminatedPlayerState = EliminatedController ? Cast<AStrifePlayerState>(EliminatedController->PlayerState) : nullptr;

	AStrifeGameState* StrifeGameState = GetGameState<AStrifeGameState>();
	if(ExecutionerPlayerState && ExecutionerPlayerState != EliminatedPlayerState && StrifeGameState)
	{
		ExecutionerPlayerState->AddScore(1.f);
		StrifeGameState->UpdateTopScore(ExecutionerPlayerState);
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
