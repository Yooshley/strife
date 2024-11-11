// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifePlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Strife/Character/StrifeCharacter.h"
#include "Strife/Components/CombatComponent.h"
#include "Strife/GameMode/StrifeGameMode.h"
#include "Strife/GameState/StrifeGameState.h"
#include "Strife/HUD/Announcement.h"
#include "Strife/HUD/CharacterOverlay.h"
#include "Strife/HUD/StrifeHUD.h"
#include "Strife/PlayerState/StrifePlayerState.h"

void AStrifePlayerController::BeginPlay()
{
	Super::BeginPlay();

	StrifeHUD = Cast<AStrifeHUD>(GetHUD());
	ServerCheckMatchState();
}

void AStrifePlayerController::PollInit()
{
	if(CharacterOverlay == nullptr)
	{
		if(StrifeHUD && StrifeHUD->CharacterOverlay)
		{
			CharacterOverlay = StrifeHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDeaths(HUDDeaths);
			}
		}
	}
}

void AStrifePlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD; //check if HUD is valid, if not Cast

	bool bIsHUDValid = StrifeHUD && StrifeHUD->CharacterOverlay && StrifeHUD->CharacterOverlay->HealthBar && StrifeHUD->CharacterOverlay->HealthText;
	if(bIsHUDValid)
	{
		const float HealthPercent = CurrentHealth / MaxHealth;
		StrifeHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
		StrifeHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = CurrentHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void AStrifePlayerController::SetHUDScore(float Value)
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD; //check if HUD is valid, if not Cast

	bool bIsHUDValid = StrifeHUD && StrifeHUD->CharacterOverlay && StrifeHUD->CharacterOverlay->ScoreText;
	if(bIsHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("Kills: %d"), FMath::FloorToInt(Value));
		StrifeHUD->CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Value;
	}
}

void AStrifePlayerController::SetHUDDeaths(int32 Value)
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD; //check if HUD is valid, if not Cast

	bool bIsHUDValid = StrifeHUD && StrifeHUD->CharacterOverlay && StrifeHUD->CharacterOverlay->DeathsText;
	if(bIsHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("Deaths: %d"), Value);
		StrifeHUD->CharacterOverlay->DeathsText->SetText(FText::FromString(DeathsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDeaths = Value;
	}
}

void AStrifePlayerController::SetHUDAmmo(int32 Ammo, int32 MaxAmmo)
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD; //check if HUD is valid, if not Cast

	bool bIsHUDValid = StrifeHUD && StrifeHUD->CharacterOverlay && StrifeHUD->CharacterOverlay->AmmoText;
	if(bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d/%d"), Ammo, MaxAmmo);
		StrifeHUD->CharacterOverlay->AmmoText->SetText(FText::FromString(AmmoText));
	}
}

void AStrifePlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD; //check if HUD is valid, if not Cast

	bool bIsHUDValid = StrifeHUD && StrifeHUD->CharacterOverlay && StrifeHUD->CharacterOverlay->CarriedAmmoText;
	if(bIsHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		StrifeHUD->CharacterOverlay->CarriedAmmoText->SetText(FText::FromString(AmmoText));
	}
}

void AStrifePlayerController::SetHUDMatchTimer(float Time)
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD; //check if HUD is valid, if not Cast

	bool bIsHUDValid = StrifeHUD && StrifeHUD->CharacterOverlay && StrifeHUD->CharacterOverlay->MatchTimerText;
	if(bIsHUDValid)
	{
		if(Time < 0.f)
		{
			StrifeHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		
		int32 Minutes = FMath::FloorToInt(Time/60.f);
		int32 Seconds = Time - Minutes * 60.f;
		
		FString TimerText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		StrifeHUD->CharacterOverlay->MatchTimerText->SetText(FText::FromString(TimerText));
	}
}

void AStrifePlayerController::SetHUDAnnouncementTimer(float Time)
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD;

	bool bIsHUDValid = StrifeHUD && StrifeHUD->Announcement && StrifeHUD->Announcement->WarmupTime;
	if(bIsHUDValid)
	{
		if(Time < 0.f)
		{
			StrifeHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		
		int32 Minutes = FMath::FloorToInt(Time/60.f);
		int32 Seconds = Time - Minutes * 60.f;
		
		FString TimerText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		StrifeHUD->Announcement->WarmupTime->SetText(FText::FromString(TimerText));
	}
}

void AStrifePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AStrifeCharacter* StrifeCharacter = Cast<AStrifeCharacter>(InPawn);
	if(StrifeCharacter)
	{
		SetHUDHealth(StrifeCharacter->GetHealth(), StrifeCharacter->GetMaxHealth());
	}
}

void AStrifePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();

	CheckTimeSync(DeltaSeconds);

	PollInit();
}

void AStrifePlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStrifePlayerController, MatchState);
}

void AStrifePlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncTime = 0.f;
	}
}

void AStrifePlayerController::ServerCheckMatchState_Implementation()
{
	AStrifeGameMode* GameMode = Cast<AStrifeGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartTime = GameMode->LevelStartTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartTime);
	}
}

void AStrifePlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float TimeOfWarmup, float TimeOfMatch, float TimeOfCooldown, float TimeOfStart)
{
	WarmupTime = TimeOfWarmup;
	MatchTime = TimeOfMatch;
	CooldownTime = TimeOfCooldown;
	LevelStartTime = TimeOfStart;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if(StrifeHUD && MatchState == MatchState::WaitingToStart)
	{
		StrifeHUD->AddAnnouncement();
	}
}

void AStrifePlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartTime;
	}
	else if(MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartTime;
	}
	else if(MatchState == MatchState::Cooldown)
	{
		TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartTime;
	}
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if(HasAuthority())
	{
		StrifeGameMode = StrifeGameMode == nullptr ? Cast<AStrifeGameMode>(UGameplayStatics::GetGameMode(this)) : StrifeGameMode;
		if(StrifeGameMode)
		{
			SecondsLeft = FMath::CeilToInt(StrifeGameMode->GetCountdownTime() + LevelStartTime);
		}
	}
	if(TimerInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementTimer(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchTimer(TimeLeft);
		}
	}
	TimerInt = SecondsLeft;
}

void AStrifePlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReciept = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReciept);
}

void AStrifePlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerRecievedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
	
}

float AStrifePlayerController::GetServerTime()
{
	if(HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

void AStrifePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if(IsLocalPlayerController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AStrifePlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AStrifePlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AStrifePlayerController::HandleMatchHasStarted()
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD;
	if(StrifeHUD)
	{
		StrifeHUD->AddCharacterOverlay();
		if(StrifeHUD->Announcement)
		{
			StrifeHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AStrifePlayerController::HandleCooldown()
{
	StrifeHUD = StrifeHUD == nullptr ? Cast<AStrifeHUD>(GetHUD()) : StrifeHUD;
	if(StrifeHUD)
	{
		StrifeHUD->CharacterOverlay->RemoveFromParent();
		if(StrifeHUD->Announcement && StrifeHUD->Announcement->AnnouncementText)
		{
			StrifeHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("NEXT MATCH STARTING IN:");
			StrifeHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AStrifeGameState* StrifeGameState =  Cast<AStrifeGameState>(UGameplayStatics::GetGameState(this));
			AStrifePlayerState* StrifePlayerState = GetPlayerState<AStrifePlayerState>();
			if(StrifeGameState && StrifePlayerState)
			{
				TArray<AStrifePlayerState*> TopPlayerStates = StrifeGameState->TopScoringPlayerStates;
				FString InfoTextString;
				if(TopPlayerStates.Num() == 0)
				{
					InfoTextString = FString("THERE IS NO WINNER");
				}
				else if(TopPlayerStates.Num() == 1 && TopPlayerStates[0] == StrifePlayerState)
				{
					InfoTextString = FString("YOU ARE THE WINNER");
				}
				else if(TopPlayerStates.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("%s IS THE WINNER"), *TopPlayerStates[0]->GetPlayerName());
				}
				else if(TopPlayerStates.Num() > 1)
				{
					for(auto TiedPlayer : TopPlayerStates)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n)"), *TiedPlayer->GetPlayerName()));
					}
					InfoTextString = FString::Printf(TEXT("\nARE THE WINNERS"));
				}
				StrifeHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AStrifeCharacter* StrifeCharacter = Cast<AStrifeCharacter>(GetPawn());
	if(StrifeCharacter && StrifeCharacter->GetCombatComponent() )
	{
		StrifeCharacter->bDisableGameplay = true;
		StrifeCharacter->GetCombatComponent()->SetFiring(false);
	}
}


