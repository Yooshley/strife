// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 5.f;
	ShellDeletionTime = 5.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);

	FRotator RandomRotation = FRotator(
		FMath::RandRange(-5.0f, 5.0f),
		FMath::RandRange(-5.0f, 5.0f),
		FMath::RandRange(-5.0f, 5.0f) 
	);
	FVector RandomizedDirection = RandomRotation.RotateVector(GetActorForwardVector());
	CasingMesh->AddImpulse(RandomizedDirection * ShellEjectionImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellSound && !bHasPlayedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
		bHasPlayedSound = true;
	}

	FTimerHandle DeletionTimeHandle;
	GetWorldTimerManager().SetTimer(DeletionTimeHandle, this, &ACasing::DeleteShell, ShellDeletionTime, false);
}

void ACasing::DeleteShell()
{
	Destroy();
}

