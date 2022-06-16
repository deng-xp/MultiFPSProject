// Fill out your copyright notice in the Description page of Project Settings.


#include "SFPSWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame/CoopGame.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASFPSWeapon::ASFPSWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TraceTargetName = "Target";

	BaseDamage=20.0f;
	HeadShotDamage=80.0f;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ASFPSWeapon::BeginPlay()
{
	Super::BeginPlay();
	Emmo = 30;
}

// Called every frame
void ASFPSWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASFPSWeapon::Fire()
{
	if (!HasAuthority())
	{
		ServerFire();
	}
	if (Emmo <= 0)
	{
		UIEmmoIsZero();
		return;
	}
	if (HasAuthority())
	{
		Emmo--;
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		//获取当前角色眼睛的位置
		FVector ALocation;
		FRotator ARotation;
		MyOwner->GetActorEyesViewPoint(ALocation, ARotation);

		//设置碰撞参数，在发生碰撞后返回碰撞相关的信息
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bReturnPhysicalMaterial = true;
		QueryParams.bTraceComplex = true;

		//碰撞追踪设置
		FVector HitDirection = ARotation.Vector();
		FHitResult Hit;
		FVector EndStation = ALocation + HitDirection * 10000;
		FVector TraceEndStation = EndStation;

		//击中后播放的粒子效果
		UParticleSystem* CurEmitter = nullptr;
		//击中
		//"ECC_Visibility"为射线所属的碰撞通道，自主设定
		if (GetWorld()->LineTraceSingleByChannel(Hit, ALocation, EndStation, COLLISION_WEAPON, QueryParams))
		{
			//发生碰撞时，触发伤害
			AActor* HitActor = Hit.GetActor();
			TraceEndStation = Hit.ImpactPoint;

			//判定击中的是否为肉体
			float CurDamage=BaseDamage;		
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
				CurDamage= BaseDamage;
				CurEmitter = BloodEffect;
				break;
			case SURFACE_FLESHVULNERABLE:
				CurDamage=HeadShotDamage;
				CurEmitter = BloodEffect;
				break;
			default:
				CurEmitter = DefaultEffect;
				break;
			}

			//仅在服务器施加伤害
			if (HasAuthority())
			{
				UGameplayStatics::ApplyPointDamage(HitActor, CurDamage, HitDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			}
		}
		
		//播放各种粒子效果
		if (HasAuthority())
		{
			HitScanTrace.TraceEnd =TraceEndStation;
			OnRep_HitScanTrace();
			if (CurEmitter) 
			{
				PlayerEmitterA(Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), CurEmitter);
			}
		}
	}
}

void ASFPSWeapon::PlayEmitterAndSound(const FVector& TraceEndStation)
{
	//发射弹药时特效
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}

	//弹药轨迹特效
	FVector TraceStartStation = MeshComponent->GetSocketLocation(MuzzleSocketName);
	if (TraceEmitter)
	{
		UParticleSystemComponent* TraceComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceEmitter, TraceStartStation);
		if (TraceComponent)
		{
			TraceComponent->SetVectorParameter(TraceTargetName, TraceEndStation);
		}
	}

	//开火声音
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	//后坐力：角色摄像机震动
	APawn* CurPawn = Cast<APawn>(GetOwner());
	if (CurPawn)
	{
		APlayerController* CurController = Cast<APlayerController>(CurPawn->GetController());
		if (CurController)
		{
			CurController->ClientStartCameraShake(CameraShake);
		}
	}
}

/***********************网络相关的*************************/
void ASFPSWeapon::OnRep_HitScanTrace()
{
	PlayEmitterAndSound(HitScanTrace.TraceEnd);
}

void ASFPSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME_CONDITION(ASFPSWeapon, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME(ASFPSWeapon, HitScanTrace);
	DOREPLIFETIME(ASFPSWeapon, Emmo);
}

void ASFPSWeapon::ServerFire_Implementation()
{
	Fire();
}

//播放粒子效果，击中后的
void ASFPSWeapon::PlayerEmitterA_Implementation(const FVector& Locat, const FRotator& Rotat, UParticleSystem* CUREmitter)
{
	if (CUREmitter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CUREmitter, Locat, Rotat, false);
	}
}