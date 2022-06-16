// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectile.h"
#include "GameFramework\ProjectileMovementComponent.h"
#include "Kismet\GameplayStatics.h"
#include "DrawDebugHelpers.h"

//用于从控制台调试弹药爆炸范围的显示
static int32 DebugProjectileDrawing=0;
FAutoConsoleVariableRef CVARDebugProjectileDrawing(
	TEXT("COOP.DebugProjectile"),
	DebugProjectileDrawing,
	TEXT("Draw Debug Line for Projectile"),
	ECVF_Cheat);

// Sets default values
ASProjectile::ASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent=MeshComponent;

	if (!MovementComponent)
	{
		MovementComponent=CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
		MovementComponent->SetUpdatedComponent(MeshComponent);
		MovementComponent->InitialSpeed=500.0f;
		MovementComponent->MaxSpeed=500.0f;
		MovementComponent->bRotationFollowsVelocity=true;
		MovementComponent->bShouldBounce=false;
		MovementComponent->ProjectileGravityScale=0.0f;
	}

	BoomRadius=50.0f;
	BoomDamage=200.0f;
	BoomTime=1.0f;
}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	//定时销毁
	if (HasAuthority())
	{
		FTimerHandle CountTimerHandle;
		GetWorldTimerManager().SetTimer(CountTimerHandle, this, &ASProjectile::Boom, BoomTime, false);
	}
}

void ASProjectile::Boom()
{
	if (BlastEmitter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),BlastEmitter, GetActorLocation(), GetActorRotation());
	}

	if (BoomSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BoomSound, GetActorLocation());
	}
	
	//应用范围伤害
	const TArray<AActor*> IgnoreActor;
	UGameplayStatics::ApplyRadialDamage(GetWorld(), BoomDamage,GetActorLocation(),BoomRadius,DamageType, IgnoreActor);
	if (DebugProjectileDrawing > 0)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 100.0f, 10.0f, FColor::Red, false, 2);
	}
	Destroy();
}

// Called every frame
void ASProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASProjectile::FireInDirection(const FVector& ShootDirection)
{
	MovementComponent->Velocity=(MovementComponent->InitialSpeed)*ShootDirection;
}

