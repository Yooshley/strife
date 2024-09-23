// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "StrifePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class STRIFE_API AStrifePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	
	UFUNCTION()
	virtual void OnRep_Deaths();
	
	void AddScore(float Value);
	void AddDeaths(float Value);

private:
	UPROPERTY()
	class AStrifeCharacter* Character;
	
	UPROPERTY()
	class AStrifePlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
};
