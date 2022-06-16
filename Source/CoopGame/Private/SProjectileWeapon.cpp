// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "SProjectile.h"

void ASProjectileWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner && ProjectileClass)
	{
		//获取当前角色眼睛的位置
		FVector ALocation;
		FRotator ARotation;
		MyOwner->GetActorEyesViewPoint(ALocation, ARotation);

		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASProjectile* Projectile = Cast<ASProjectile>(GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, ARotation, SpawnParameters));
		if (Projectile)
		{
			Projectile->FireInDirection(ARotation.Vector());
		}
	}
}
