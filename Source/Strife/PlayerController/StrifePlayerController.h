// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StrifePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STRIFE_API AStrifePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDScore(float Value);
	void SetHUDDeaths(int32 Value);
	void SetHUDAmmo(int32 Ammo, int32 MaxAmmo);
	void SetHUDCarriedAmmo(int32 Ammo);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class AStrifeHUD* StrifeHUD;
};
