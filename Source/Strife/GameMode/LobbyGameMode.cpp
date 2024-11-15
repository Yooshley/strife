// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if(NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel("/Game/Strife/Maps/StrifeMap?listen");
		}
	}
}
