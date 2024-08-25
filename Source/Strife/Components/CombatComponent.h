// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STRIFE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class AStrifeCharacter;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	UCombatComponent();

	void EquipWeapon(class AWeapon* WeaponToEquip);
	
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bShouldAim);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bShouldAim);

	// UFUNCTION()
	// void OnRep_EquippedWeapon();

private:
	class AStrifeCharacter* Character;

	//UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	UPROPERTY(Replicated)
	class AWeapon* EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bIsAiming;

public:	
};
