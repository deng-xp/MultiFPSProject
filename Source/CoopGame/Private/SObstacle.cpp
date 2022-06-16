// Fill out your copyright notice in the Description page of Project Settings.


#include "SObstacle.h"
#include "Component/SHealthComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASObstacle::ASObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));
	
	//模拟物体效果的网格体
	MeshComponent=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent=MeshComponent;

	ExplosionRadius=70.0f;

	RadialForce=CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForce"));
	RadialForce->SetupAttachment(RootComponent);
	RadialForce->bImpulseVelChange=true;	//若为真，则脉冲将忽略物体本身质量
	RadialForce->bIgnoreOwningActor=true;	//脉冲不施加于属于这个Actor的部分
	RadialForce->bAutoActivate=false;		//组件是在创建时激活还是必须显示激活

	ExplosionImpulse = 400;
	bExplode=false;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASObstacle::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnHealthChanged.AddDynamic(this, &ASObstacle::OnHealthChangedCur);
	RadialForce->Radius = ExplosionRadius;
}

// Called every frame
void ASObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASObstacle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASObstacle, bExplode);
}

void ASObstacle::OnHealthChangedCur(USHealthComponent* HealthComponents, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExplode)
	{
		return;
	}
	if (Health <= 0.0f)
	{
		bExplode=true;
		
		//显示视觉效果
		OnRep_bExplode();

		//施加力等仅在服务器完成，通过复制移动同步至客户端
		MeshComponent->AddImpulse(FVector::UpVector * ExplosionImpulse, NAME_None, true);
		//触发冲击力
		RadialForce->FireImpulse();
	}
}

void ASObstacle::OnRep_bExplode()
{
	if (BoomSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BoomSystem, GetActorLocation(), GetActorRotation());
	}
	if (DefaultMaterial)
	{
		MeshComponent->SetMaterial(0, DefaultMaterial);
	}
	
}
