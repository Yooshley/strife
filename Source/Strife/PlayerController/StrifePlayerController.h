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
	void SetHUDMatchTimer(float Time);
	void SetHUDAnnouncementTimer(float Time);
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float GetServerTime(); //synced with server world clock
	virtual void ReceivedPlayer() override; //earliest possible sync with clock

	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

protected:
	virtual void BeginPlay() override;
	void PollInit();
	void SetHUDTime();

	//Sync time between client and server
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecievedClientRequest);

	float ClientServerDelta = 0.f; //different between client/server time

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float TimeOfWarmup, float TimeOfMatch, float TimeOfCooldown, float TimeOfStart);
	
private:
	UPROPERTY()
	class AStrifeHUD* StrifeHUD;

	UPROPERTY()
	class AStrifeGameMode* StrifeGameMode;

	float LevelStartTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 TimerInt = 0.f;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	float HUDDeaths;
};
