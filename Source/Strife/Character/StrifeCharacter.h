// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Strife/Components/Types/TurningInPlace.h"
#include "Strife/Interfaces/InteractWithCrosshairsInterface.h"
#include "StrifeCharacter.generated.h"

UCLASS()
class STRIFE_API AStrifeCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	AStrifeCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayDeathMontage();

	virtual void OnRep_ReplicatedMovement() override;
	void UpdateHUDHealth();

	void Death();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRightward(float Value);
	void Turn(float Value);
	//void LookUp(float Value);
	void InteractInput();
	void CrouchInput();
	void AimInputPressed();
	void AimInputReleased();
	void AimOffset(float DeltaTime);
	void SimulatedProxiesTurn();
	virtual void Jump() override;
	void FireInputPressed();
	void FireInputReleased();

	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// polls for any relevant classes and initialized the HUD
	void PollInit();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;

	UFUNCTION(Server, Reliable)
	void ServerInteract();

	float AimOffsetYaw;
	float InterpAimOffsetYaw;
	float AimOffsetPitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* DeathMontage;

	void CameraCharacterCulling();
	UPROPERTY(EditAnywhere)
	float CullingThreshold = 200.f;

	bool bShouldRotateBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotationThisFrame;
	float ProxyYaw;
	float TimeSinceLastMovementReplicated;
	float CalculateSpeed();
	
	//player health
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, Category = "Player Stats")
	float CurrentHealth = MaxHealth;
	
	UFUNCTION()
	void OnRep_CurrentHealth();

	UPROPERTY()
	class AStrifePlayerController* StrifePlayerController;

	UPROPERTY()
	class AStrifePlayerState* StrifePlayerState;

	bool bIsDead = false;

	FTimerHandle RespawnTimer;

	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 5.f;

	void RespawnTimerFinished();

	//dissolve effect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(VisibleAnywhere, Category="Death")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance; //runtime instance material

	UPROPERTY(EditAnywhere, Category="Death")
	UMaterialInstance* DissolveMaterialInstance; //blueprint instance material

public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();

	bool IsAiming();

	FORCEINLINE float GetAimOffsetYaw() const { return AimOffsetYaw; }
	FORCEINLINE float GetAimOffsetPitch() const { return AimOffsetPitch; }

	AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool ShouldRotateRootBone() const { return bShouldRotateBone; }

	FORCEINLINE bool IsDead() const { return bIsDead; }

	FORCEINLINE float GetHealth() const { return CurrentHealth; }

	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
};
