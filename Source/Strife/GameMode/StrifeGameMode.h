// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "StrifeGameMode.generated.h"

namespace MatchState
{
	extern STRIFE_API const FName Cooldown; //Match duration has been reached. Display results and begin cooldown timer
}

/**
 * 
 */
UCLASS()
class STRIFE_API AStrifeGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AStrifeGameMode();

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void PlayerEliminated(class AStrifeCharacter* EliminatedCharacter, class AStrifePlayerController* EliminatedController, class AStrifePlayerController* ExecutionerController);

	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 150.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartTime = 0.f;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;
	
private:
	float CountdownTime = 0.f;
	
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
