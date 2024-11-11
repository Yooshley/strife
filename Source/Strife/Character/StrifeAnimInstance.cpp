// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifeAnimInstance.h"

#include "StrifeCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Strife/Weapon/Weapon.h"

void UStrifeAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	StrifeCharacter = Cast<AStrifeCharacter>(TryGetPawnOwner());
}

void UStrifeAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(StrifeCharacter == nullptr)
	{
		StrifeCharacter = Cast<AStrifeCharacter>(TryGetPawnOwner());
	}

	if(StrifeCharacter == nullptr)
	{
		return;
	}

	FVector Velocity = StrifeCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsFalling = StrifeCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = StrifeCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bIsWeaponEquipped = StrifeCharacter->IsWeaponEquipped();
	EquippedWeapon = StrifeCharacter->GetEquippedWeapon();
	bIsCrouched = StrifeCharacter->bIsCrouched;
	bIsAiming = StrifeCharacter->IsAiming();
	TurningInPlace = StrifeCharacter->GetTurningInPlace();
	bShouldRotateRootBone = StrifeCharacter->ShouldRotateRootBone();
	bIsDead = StrifeCharacter->IsDead();
	bIsReloading = StrifeCharacter->GetCombatState() == ECombatState::ECS_Reloading;
	
	// //Strafing
	// FRotator AimRotation = StrifeCharacter->GetBaseAimRotation();
	// FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(StrifeCharacter->GetVelocity());
	// FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	// DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 15.f);
	// YawOffset = DeltaRotation.Yaw;
	//
	// //Leaning
	// CharacterRotationLastFrame = CharacterRotationThisFrame;
	// CharacterRotationThisFrame = StrifeCharacter->GetActorRotation();
	// const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationThisFrame, CharacterRotationLastFrame);
	// const float Target = Delta.Yaw / DeltaSeconds;
	// const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	// Lean = FMath::Clamp(Interp, -90.f, 90.f);

	//8 directional animations w/o leaning
	const FVector WorldVelocity = StrifeCharacter->GetVelocity();
	const FRotator ActorRotation = StrifeCharacter->GetActorRotation();
	const FVector LocalVelocity = ActorRotation.UnrotateVector(WorldVelocity);
	
	HValue = LocalVelocity.Y;
	VValue = LocalVelocity.X;

	AimOffsetYaw = StrifeCharacter->GetAimOffsetYaw();
	//AimOffsetPitch = StrifeCharacter->GetAimOffsetPitch();

	//FABRIK IK for left hand placement relative to right hand bone
	if(bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && StrifeCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("GripSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		StrifeCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if(StrifeCharacter->IsLocallyControlled()) //accurate orientation of weapon based on crosshair
		{
			bIsLocallyControlled = true;
			FTransform RightHandTransform = StrifeCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - StrifeCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 25.f);
			
			//UE_LOG(LogTemp, Warning, TEXT("RightHandRotation: Pitch=%f, Yaw=%f, Roll=%f"), RightHandRotation.Pitch, RightHandRotation.Yaw, RightHandRotation.Roll);
			// FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), RTS_World);
			// FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
			// DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
			// DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), StrifeCharacter->GetHitTarget(), FColor::Yellow);
		}
	}
}
