// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifePlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Strife/Character/StrifeCharacter.h"
#include "Strife/HUD/CharacterOverlay.h"
#include "Strife/HUD/StrifeHUD.h"

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

void AStrifePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AStrifeCharacter* StrifeCharacter = Cast<AStrifeCharacter>(InPawn);
	if(StrifeCharacter)
	{
		SetHUDHealth(StrifeCharacter->GetHealth(), StrifeCharacter->GetMaxHealth());
	}
}

void AStrifePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	StrifeHUD = Cast<AStrifeHUD>(GetHUD());
}
