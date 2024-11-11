// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Strife/Character/StrifeCharacter.h"
#include "Strife/PlayerController/StrifePlayerController.h"
#include "Strife/Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	//if(Character)
	//{
		// if(Character->GetFollowCamera())
		// {
		// 	DefaultFOV = Character->GetFollowCamera()->FieldOfView;
		// 	CurrentFOV = DefaultFOV;
		// }
	//}

	if(Character->HasAuthority())
	{
		InitialzeCarriedAmmo();
	}
}

void UCombatComponent::SetAiming(bool bShouldAim)
{
	bIsAiming = bShouldAim;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = !bShouldAim;
		Character->bUseControllerRotationYaw= bShouldAim;
	}
	//bShouldAim ? StartCameraInterp() : StopCameraInterp();
	ServerSetAiming(bIsAiming);
}

void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		ServerFire(TraceHitTarget);
		StartFireTimer();
		if(EquippedWeapon)
		{
			CrosshairShootFactor = 0.75f; //TODO: Get recoil from weapon
		}
	}
}

void UCombatComponent::SetFiring(bool bShouldFire)
{
	bIsFiring = bShouldFire;

	if (bShouldFire && EquippedWeapon)
	{
		Fire();
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2d ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2d CrosshairLocation(ViewportSize.X / 2, ViewportSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
		);

	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if(Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f); //added a reasonable amount to move the start ahead of the character
			//DrawDebugSphere(GetWorld(), Start, 15.f, 10, FColor::Red, false);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

		if (!HitResult.bBlockingHit) HitResult.ImpactPoint = End;
	}

	if(HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()) //checking if actor implements the interface
	{
		HUDPackage.CrosshairColor = FLinearColor::Red;
	}
	else
	{
		HUDPackage.CrosshairColor = FLinearColor::White;
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->Controller) : Controller;

	if(Controller)
	{
		HUD = HUD == nullptr ? Cast<AStrifeHUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
			}
			//calculate spread for crosshair
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			if(EquippedWeapon != nullptr) //crosshaircalculations only happen if weapon equipped
			{
				CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

				if(Character->GetCharacterMovement()->IsFalling())
				{
					CrosshairFallingFactor = FMath::FInterpTo(CrosshairFallingFactor, 2.25f, DeltaTime, 2.25f);
				}
				else
				{
					CrosshairFallingFactor = FMath::FInterpTo(CrosshairFallingFactor, 0.f, DeltaTime, 22.5f);
				}
			
				if(bIsAiming)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, EquippedWeapon->ZoomInterpSpeed);
				}
				else
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, EquippedWeapon->ZoomInterpSpeed);
				}

				CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, DeltaTime, EquippedWeapon->ZoomInterpSpeed); //shooting factor always interping to zero
			
				HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairFallingFactor -  CrosshairAimFactor + CrosshairShootFactor; //hard coded baseline spread
			}
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

// void UCombatComponent::InterpFOV(float DeltaTime)
// {
// 	if(EquippedWeapon == nullptr) return;
//
// 	if(bIsAiming)
// 	{
// 		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
// 	}
// 	else
// 	{
// 		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
// 	}
//
// 	if(Character && Character->GetFollowCamera())
// 	{
// 		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
// 	}
// }

// void UCombatComponent::StartCameraInterp()
// {
// 	if (Character && Character->GetFollowCamera())
// 	{
// 		DefaultCameraLocation = Character->GetFollowCamera()->GetComponentLocation();
// 		FVector ForwardDirection = Character->GetActorForwardVector();
// 		TargetCameraLocation = DefaultCameraLocation + (ForwardDirection * AimDistance);
// 		bInterpCamera = true;
// 	}
// }
//
// void UCombatComponent::StopCameraInterp()
// {
// 	TargetCameraLocation = DefaultCameraLocation;
// 	bInterpCamera = true;
// }
//
// void UCombatComponent::InterpAim(float DeltaTime)
// {
// 	if (bInterpCamera && Character && Character->GetFollowCamera())
// 	{
// 		FVector CurrentCameraLocation = Character->GetFollowCamera()->GetComponentLocation();
// 		FVector NewCameraLocation = FMath::VInterpTo(CurrentCameraLocation, TargetCameraLocation, DeltaTime, AimInterpSpeed);
// 		
// 		Character->GetFollowCamera()->SetWorldLocation(NewCameraLocation);
// 		
// 		if (FVector::Dist(CurrentCameraLocation, TargetCameraLocation) <= 1.f)
// 		{
// 			bInterpCamera = false;
// 		}
// 	}
// }

void UCombatComponent::AdjustCameraForAiming(float DeltaTime)
{
	if(Character == nullptr) return;

	USpringArmComponent* CharacterCameraBoom = Character->GetCameraBoom();
	if(CharacterCameraBoom)
	{
		if (bIsAiming)
		{
			const FVector AimDirection = Character->GetFacingDirection();
			const FVector NewTargetOffset = AimDirection * AimOffsetDistance;
			CharacterCameraBoom->TargetOffset = FMath::VInterpTo(CharacterCameraBoom->TargetOffset, NewTargetOffset, DeltaTime, AimOffsetSpeed);
		}
		else
		{
			CharacterCameraBoom->TargetOffset = FMath::VInterpTo(CharacterCameraBoom->TargetOffset, FVector::ZeroVector, DeltaTime, AimOffsetSpeed);
		}
	}
}

void UCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if(EquippedWeapon == nullptr) return;
	bCanFire = true;
	if(bIsFiring && EquippedWeapon->bIsAutomatic)
	{
		Fire();
	}
	if(EquippedWeapon->IsEmpty())
	{
		ReloadWeapon();
	}
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr)
	{
		return false;
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitialzeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingRifleAmmo);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& HitTarget)
{
	Multicast_Fire(HitTarget);
}

void UCombatComponent::Multicast_Fire_Implementation(const FVector_NetQuantize& HitTarget)
{
	if(EquippedWeapon == nullptr) return;

	if(Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(HitTarget);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RifleSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		// Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		// Character->bUseControllerRotationYaw= true;
			if(EquippedWeapon->EquipSound)
        {
        	UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound,Character->GetActorLocation());
        }
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bShouldAim)
{
	bIsAiming = bShouldAim;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = !bShouldAim;
		Character->bUseControllerRotationYaw= bShouldAim;
	}
	//bShouldAim ? StartCameraInterp() : StopCameraInterp();
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		TraceHitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		//InterpAim(DeltaTime);
		//InterpFOV(DeltaTime);
		AdjustCameraForAiming(DeltaTime);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	if(EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RifleSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	if(EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound,Character->GetActorLocation());
	}

	if(EquippedWeapon->IsEmpty())
	{
		ReloadWeapon();
	}
}

void UCombatComponent::ReloadWeapon()
{
	if(CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::EndReload()
{
	if(Character == nullptr) return;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if(bIsFiring)
	{
		Fire();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon)
	{
		int32 RoomInMag = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
		if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
			int32 Least = FMath::Min(RoomInMag, AmountCarried);
			return FMath::Clamp(RoomInMag, 0, Least);
		}
	}
	return 0;
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bIsFiring)
		{
			Fire();
		}
		break;
	default:
		break;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	int32 ReloadAmount = AmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AStrifePlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}
