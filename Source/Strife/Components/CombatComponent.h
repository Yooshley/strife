// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Strife/HUD/StrifeHUD.h"
#include "Strife/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 75000.f;

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
	void Reload();
	
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bShouldAim);
	void Fire();

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
	UPROPERTY()
	class AStrifeCharacter* Character;
	UPROPERTY()
	class AStrifePlayerController* Controller;
	UPROPERTY()
	class AStrifeHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	bool bIsFiring;

	UPROPERTY(Replicated)
	bool bIsAiming;

	//HUD and Crosshairs
	float CrosshairVelocityFactor;
	float CrosshairFallingFactor;
	float CrosshairAimFactor;
	float CrosshairShootFactor;
	FHUDPackage HUDPackage;

	FVector TraceHitTarget;
	
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;
	
	//Aiming FOV
	float DefaultFOV;
	float CurrentFOV;
    
    //void InterpFOV(float DeltaTime);

	//Automatic Fire
	FTimerHandle FireTimer;
	bool bCanFire = true;
	
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingRifleAmmo = 30;
	
	void InitialzeCarriedAmmo();

public:
};
