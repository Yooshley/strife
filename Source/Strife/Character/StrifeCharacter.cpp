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
#include "Strife/Strife.h"
#include "Strife/Components/CombatComponent.h"
#include "Strife/GameMode/StrifeGameMode.h"
#include "Strife/PlayerController/StrifePlayerController.h"
#include "Strife/PlayerState/StrifePlayerState.h"
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
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_None;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AStrifeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//only replicate to owning client
	DOREPLIFETIME_CONDITION(AStrifeCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AStrifeCharacter, CurrentHealth);
}

void AStrifeCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if(CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void AStrifeCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimulatedProxiesTurn();
	TimeSinceLastMovementReplicated = 0;
}

void AStrifeCharacter::UpdateHUDHealth()
{
	StrifePlayerController = StrifePlayerController == nullptr ? Cast<AStrifePlayerController>(Controller) : StrifePlayerController;
	if(StrifePlayerController)
	{
		StrifePlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
}

void AStrifeCharacter::Death()
{
	//server only - called from gamemode
	if(CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Drop();
	}
	MulticastDeath();
	GetWorldTimerManager().SetTimer(RespawnTimer, this, &AStrifeCharacter::RespawnTimerFinished, RespawnDelay);
}

void AStrifeCharacter::MulticastDeath_Implementation()
{
	if(StrifePlayerController)
	{
		StrifePlayerController->SetHUDAmmo(0, 0);
	}
	
	bIsDead = true;
	PlayDeathMontage();

	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 100.f);
	}
	StartDissolve();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if(StrifePlayerController)
	{
		DisableInput(StrifePlayerController);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AStrifeCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AStrifeCharacter::RecieveDamage);
	}
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
	//PlayerInputComponent->BindAxis("LookUp", this, &AStrifeCharacter::LookUp);
}

void AStrifeCharacter::PlayFireMontage(bool bAiming)
{
	if(CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance =  GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("Aim") : FName("Hip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AStrifeCharacter::PlayHitReactMontage()
{
	if(CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance =  GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AStrifeCharacter::PlayDeathMontage()
{
	UAnimInstance* AnimInstance =  GetMesh()->GetAnimInstance();
	if(AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
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

// void AStrifeCharacter::LookUp(float Value)
// {
// 	AddControllerPitchInput(Value);
// }

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
	
	const float Speed = CalculateSpeed();
	const bool bIsFalling = GetMovementComponent()->IsFalling();

	if(Speed == 0.f && !bIsFalling)
	{
		bShouldRotateBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AimOffsetYaw = -DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_None)
		{
			InterpAimOffsetYaw = AimOffsetYaw;
		}

		TurnInPlace(DeltaTime);
	}
	if(Speed > 0.f || bIsFalling)
	{
		bShouldRotateBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AimOffsetYaw = 0.f;

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

void AStrifeCharacter::SimulatedProxiesTurn()
{
	if(CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	bShouldRotateBone = false;
	const float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_None;
		return;
	}

	AimOffsetPitch = GetBaseAimRotation().Vector().Rotation().Pitch;
	ProxyRotationLastFrame = ProxyRotationThisFrame;
	ProxyRotationThisFrame = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotationThisFrame, ProxyRotationLastFrame).Yaw;

	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_None;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_None;
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

void AStrifeCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	class AController* InstigatorController, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage(); //call for server

	if(CurrentHealth == 0.f)
	{
		AStrifeGameMode* StrifeGameMode = GetWorld()->GetAuthGameMode<AStrifeGameMode>();
		if(StrifeGameMode)
		{
			StrifePlayerController = StrifePlayerController == nullptr ? Cast<AStrifePlayerController>(Controller) : StrifePlayerController;
			AStrifePlayerController* ExecutionerPlayerController = Cast<AStrifePlayerController>(InstigatorController);
			StrifeGameMode->PlayerEliminated(this, StrifePlayerController, ExecutionerPlayerController);
		}
	}
}

void AStrifeCharacter::PollInit()
{
	if(StrifePlayerState == nullptr)
	{
		StrifePlayerState = GetPlayerState<AStrifePlayerState>();
		if(StrifePlayerState)
		{
			StrifePlayerState->AddScore(0.f);
			StrifePlayerState->AddDeaths(0);
		}
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

void AStrifeCharacter::CameraCharacterCulling()
{
	//Hide Character and Weapon if too close to the Camera
	if(!IsLocallyControlled()) return;

	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CullingThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float AStrifeCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AStrifeCharacter::OnRep_CurrentHealth()
{
	UpdateHUDHealth();
	PlayHitReactMontage(); //call for clients
}

void AStrifeCharacter::RespawnTimerFinished()
{
	AStrifeGameMode* GameMode = GetWorld()->GetAuthGameMode<AStrifeGameMode>();
	if(GameMode)
	{
		GameMode->RequestRespawn(this, Controller);
	}
}

void AStrifeCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AStrifeCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AStrifeCharacter::UpdateDissolveMaterial);
	if(DissolveCurve)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
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

	if(GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled()) //either autonomous proxy or authority
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplicated += DeltaTime;
		if(TimeSinceLastMovementReplicated > 0.25f) //force call SimulatedProxiesTurn if no movement to replicate
		{
			OnRep_ReplicatedMovement();
		}
		AimOffsetPitch = GetBaseAimRotation().Vector().Rotation().Pitch;
	}
	CameraCharacterCulling();
	PollInit();
}

