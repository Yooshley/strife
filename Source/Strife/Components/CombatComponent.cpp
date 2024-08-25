// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Strife/Character/StrifeCharacter.h"
#include "Strife/Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::SetAiming(bool bShouldAim)
{
	if(bIsAiming == bShouldAim) return;
	bIsAiming = bShouldAim;
	if(bIsAiming)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeedCrouched;
		Character->bUseControllerRotationYaw = true;
	}
	else
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->GetCharacterMovement()->MaxWalkSpeed = 500.f; //TODO: Replace Hardcoding
		Character->bUseControllerRotationYaw = false;
	}
	ServerSetAiming(bIsAiming);
}

// void UCombatComponent::OnRep_EquippedWeapon()
// {
// 	if(EquippedWeapon && Character)
// 	{
// 		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
// 		Character->bUseControllerRotationYaw= true;
// 	}
// }

void UCombatComponent::ServerSetAiming_Implementation(bool bShouldAim)
{
	bIsAiming = bShouldAim;
	if(bIsAiming)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeedCrouched;
		Character->bUseControllerRotationYaw = true;
	}
	else
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
		Character->bUseControllerRotationYaw = false;
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	// Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	// Character->bUseControllerRotationYaw= true;
}

