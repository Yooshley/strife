// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if(!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector TargetVector = HitTarget - SocketTransform.GetLocation(); //vector b/w muzzle and crosshair trace
		FRotator TargetRotation = TargetVector.Rotation();

		if(ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
			}
		}
	}
}