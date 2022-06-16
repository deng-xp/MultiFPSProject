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
		//��ȡ��ǰ��ɫ�۾���λ��
		FVector ALocation;
		FRotator ARotation;
		MyOwner->GetActorEyesViewPoint(ALocation, ARotation);

		//������ײ�������ڷ�����ײ�󷵻���ײ��ص���Ϣ
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bReturnPhysicalMaterial = true;
		QueryParams.bTraceComplex = true;

		//��ײ׷������
		FVector HitDirection = ARotation.Vector();
		FHitResult Hit;
		FVector EndStation = ALocation + HitDirection * 10000;
		FVector TraceEndStation = EndStation;

		//���к󲥷ŵ�����Ч��
		UParticleSystem* CurEmitter = nullptr;
		//����
		//"ECC_Visibility"Ϊ������������ײͨ���������趨
		if (GetWorld()->LineTraceSingleByChannel(Hit, ALocation, EndStation, COLLISION_WEAPON, QueryParams))
		{
			//������ײʱ�������˺�
			AActor* HitActor = Hit.GetActor();
			TraceEndStation = Hit.ImpactPoint;

			//�ж����е��Ƿ�Ϊ����
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

			//���ڷ�����ʩ���˺�
			if (HasAuthority())
			{
				UGameplayStatics::ApplyPointDamage(HitActor, CurDamage, HitDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			}
		}
		
		//���Ÿ�������Ч��
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
	//���䵯ҩʱ��Ч
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}

	//��ҩ�켣��Ч
	FVector TraceStartStation = MeshComponent->GetSocketLocation(MuzzleSocketName);
	if (TraceEmitter)
	{
		UParticleSystemComponent* TraceComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceEmitter, TraceStartStation);
		if (TraceComponent)
		{
			TraceComponent->SetVectorParameter(TraceTargetName, TraceEndStation);
		}
	}

	//��������
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	//����������ɫ�������
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

/***********************������ص�*************************/
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

//��������Ч�������к��
void ASFPSWeapon::PlayerEmitterA_Implementation(const FVector& Locat, const FRotator& Rotat, UParticleSystem* CUREmitter)
{
	if (CUREmitter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CUREmitter, Locat, Rotat, false);
	}
}