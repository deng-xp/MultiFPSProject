// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STrackBot.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Component/SHealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "SFPSCharacter.h"

// Sets default values
ASTrackBot::ASTrackBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you   don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent=MeshComp;
	MeshComp->SetSimulatePhysics(true);
	//网格体不会影响AI寻路
	MeshComp->SetCanEverAffectNavigation(false);

	//碰撞体组件
	SphereComponent= CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(200.0f);
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);

	//OnHealthChanged仅在服务器执行（UHealthComponent组件中设计的）
	BotHealthComp= CreateDefaultSubobject<USHealthComponent>(TEXT("BotHealthComp"));
	BotHealthComp->OnHealthChanged.AddDynamic(this, &ASTrackBot::HandleTakeDamage);

	CurForce=1000;
	AcquiredMinDistance=10;
	bUseVelocityChange=false;

	BaseDamage=25;
	BoomRadius=100;
	bExplosion = false;

	SelfDamageInterval=0.25f;
	bSelfDied=false;
}

// Called when the game starts or when spawned
void ASTrackBot::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		NextPathPoint = GetNextPathPoint();
	}
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ASTrackBot::OnPawnBeginOverlap);
}

void ASTrackBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this,20,GetInstigatorController(),this,nullptr);
}

// Called every frame
void ASTrackBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority()&&!bExplosion)
	{
		float CurDistance = (NextPathPoint - GetActorLocation()).Size();
		if (CurDistance > AcquiredMinDistance)
		{
			FVector Direction = NextPathPoint - GetActorLocation();
			Direction.Normalize();
			FVector WillAddForce = CurForce * Direction;
			MeshComp->AddForce(WillAddForce, NAME_None, bUseVelocityChange);
		}
		NextPathPoint = GetNextPathPoint();
	}
}

//寻路相关仅在服务器运行
FVector ASTrackBot::GetNextPathPoint()
{
	AActor*	CurCharacter=Cast<AActor>(UGameplayStatics::GetPlayerCharacter(this, 0));
	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), CurCharacter);
	if (NavPath && NavPath->PathPoints.Num() > 1)
	{
		return NavPath->PathPoints[1];
	}
	return GetActorLocation();
}

void ASTrackBot::HandleTakeDamage(USHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogTemp,Warning,TEXT("BotHealth is %f"),HealthComp->GetHealth());
	
	//受到伤害后材质闪烁
	if (!MaterialIns)
	{
		MaterialIns = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MaterialIns)
	{
		MaterialIns->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	//生命值归零后，启动销毁程序
	if (BotHealthComp->GetHealth() <= 0)
	{
		SelfDestruct();
	}
}

void ASTrackBot::SelfDestruct()
{
	if (bExplosion)
	{
		return;
	}

	bExplosion =true;

	//爆发爆炸时的粒子效果和声音
	if (BoomEmitter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, BoomEmitter, GetActorLocation(), GetActorRotation());
	}
	if (BoomSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BoomSound, GetActorLocation());
	}

	//先隐藏，后销毁（逻辑上是：此Actor已经被销毁了）
	MeshComp->SetVisibility(false,true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//仅在服务器进行伤害
	if (HasAuthority())
	{
		//在爆炸位置产生范围伤害
		TArray<AActor*> IgnoreActor;
		IgnoreActor.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, BaseDamage, GetActorLocation(), BoomRadius, nullptr, IgnoreActor, this, GetInstigatorController(), true);
		
		//立即销毁可能导致客户端难以及时同步粒子效果和声音，因此用设置生命周期替代立即销毁
		//Destroy();
		SetLifeSpan(2.0f);
	}
}

//与角色发生重叠后，开启自毁
void ASTrackBot::OnPawnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bSelfDied|| bExplosion)
	{
		return;
	}
	ASFPSCharacter* PlayerPawn=Cast<ASFPSCharacter>(OtherActor);
	if (PlayerPawn)
	{
		//通过定时器对自身施加持续伤害
		GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackBot::DamageSelf, SelfDamageInterval, true, 0.0f);
		bSelfDied = true;
	}

	//生成自毁时的声音效果
	UGameplayStatics::SpawnSoundAttached(SelfDamageSound,RootComponent);
}

