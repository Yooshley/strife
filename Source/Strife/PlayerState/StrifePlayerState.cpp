// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifePlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Strife/Character/StrifeCharacter.h"
#include "Strife/PlayerController/StrifePlayerController.h"

void AStrifePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStrifePlayerState, Deaths);
}

void AStrifePlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AStrifeCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AStrifePlayerState::OnRep_Deaths()
{
	Character = Character == nullptr ? Cast<AStrifeCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		}
	}
}

void AStrifePlayerState::AddScore(float Value)
{
	SetScore(GetScore() + Value);
	Character = Character == nullptr ? Cast<AStrifeCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AStrifePlayerState::AddDeaths(float Value)
{
	Deaths += Value;
	Character = Character == nullptr ? Cast<AStrifeCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->GetController()) : Controller;
		if(Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		}
	}
}
