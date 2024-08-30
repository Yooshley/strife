// Fill out your copyright notice in the Description page of Project Settings.


#include "StrifeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Strife/Components/CombatComponent.h"
#include "Strife/Weapon/Weapon.h"

AStrifeCharacter::AStrifeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 750.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>("Combat Component");
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	TurningInPlace = ETurningInPlace::ETIP_None;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void AStrifeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//only replicate to owning client
	DOREPLIFETIME_CONDITION(AStrifeCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AStrifeCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void AStrifeCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStrifeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AStrifeCharacter::InteractInput);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AStrifeCharacter::CrouchInput);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AStrifeCharacter::AimInputPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AStrifeCharacter::AimInputReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AStrifeCharacter::FireInputPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AStrifeCharacter::FireInputReleased);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AStrifeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRightward", this, &AStrifeCharacter::MoveRightward);
	PlayerInputComponent->BindAxis("Turn", this, &AStrifeCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AStrifeCharacter::LookUp);
}

void AStrifeCharacter::PlayFireMontage(bool bAiming)
{
	if(CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance =  GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AStrifeCharacter::MoveForward(float Value)
{
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AStrifeCharacter::MoveRightward(float Value)
{
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AStrifeCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AStrifeCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AStrifeCharacter::InteractInput()
{
	if(CombatComponent)
	{
		if(HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerInteract();
		}
	}
}

void AStrifeCharacter::CrouchInput()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AStrifeCharacter::AimInputPressed()
{
	if(CombatComponent)
	{
		if(CombatComponent->EquippedWeapon == nullptr)
		{
			return;
		}
		CombatComponent->SetAiming(true);
	}
}

void AStrifeCharacter::AimInputReleased()
{
	if(CombatComponent)
	{
		if(CombatComponent->EquippedWeapon == nullptr)
		{
			return;
		}
		CombatComponent->SetAiming(false);
	}
}

void AStrifeCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		//hide widget on client
		LastWeapon->ShowPickupWidget(false);
	}
}

void AStrifeCharacter::ServerInteract_Implementation()
{
	if(CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AStrifeCharacter::AimOffset(float DeltaTime)
{
	if(CombatComponent && CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}
	
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	const float Speed = Velocity.Size();
	const bool bIsFalling = GetMovementComponent()->IsFalling();

	if(Speed == 0.f && !bIsFalling)
	{
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AimOffsetYaw = -DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_None)
		{
			InterpAimOffsetYaw = AimOffsetYaw;
		}
		bUseControllerRotationYaw = false;
		TurnInPlace(DeltaTime);
	}
	if(Speed > 0.f || bIsFalling)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AimOffsetYaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_None;
	}
	
	//UE packing of FVector angles puts it in an unsigned form. Causes issues in multiplayer. Deprecated Solution.
	// if(AimOffsetPitch > 90.f && !IsLocallyControlled())
	// {
	// 	// map pitch from the range [270, 360) to the range [-90, 0)
	// 	const FVector2d InRange(270.f, 360.f);
	// 	const FVector2d OutRange(-90.f, 0.f);
	// 	AimOffsetPitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimOffsetPitch);
	// }

	//replicating entire vector to bypass UE compression
	AimOffsetPitch = GetBaseAimRotation().Vector().Rotation().Pitch;
}

void AStrifeCharacter::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AStrifeCharacter::FireInputPressed()
{
	if(CombatComponent)
	{
		CombatComponent->SetFiring(true);
	}
}

void AStrifeCharacter::FireInputReleased()
{
	if(CombatComponent)
	{
		CombatComponent->SetFiring(false);
	}
}

void AStrifeCharacter::TurnInPlace(float DeltaTime)
{
	if(AimOffsetYaw > 45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AimOffsetYaw < -45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if(TurningInPlace != ETurningInPlace::ETIP_None)
	{
		InterpAimOffsetYaw = FMath::FInterpTo(InterpAimOffsetYaw, 0.f, DeltaTime, 2.f);
		AimOffsetYaw = InterpAimOffsetYaw;
		if(FMath::Abs(AimOffsetYaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_None;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw - AimOffsetYaw, 0.f);
		}
	}
}

void AStrifeCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled())
	{
		//hide widget on server
		if (OverlappingWeapon && OverlappingWeapon != Weapon)
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
		if (Weapon)
		{
			Weapon->ShowPickupWidget(true);
		}
	}
	OverlappingWeapon = Weapon;
}

bool AStrifeCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool AStrifeCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bIsAiming);
}

AWeapon* AStrifeCharacter::GetEquippedWeapon()
{
	if(CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}

FVector AStrifeCharacter::GetHitTarget() const
{
	if(CombatComponent == nullptr) return FVector();
	return CombatComponent->TraceHitTarget;
}

void AStrifeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AimOffset(DeltaTime);
}

