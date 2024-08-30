// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 90000.f;

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

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void SetFiring(bool bShouldFire);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& HitTarget);

	void TraceUnderCrosshairs(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	class AStrifeCharacter* Character;
	class AStrifePlayerController* Controller;
	class AStrifeHUD* HUD;

	//UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	UPROPERTY(Replicated)
	class AWeapon* EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bIsAiming;

	bool bIsFiring;

	//HUD and Crosshairs
	float CrosshairVelocityFactor;
	float CrosshairFallingFactor;

	FVector TraceHitTarget;
	
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

public:	
};
