// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackBot.generated.h"

UCLASS()
class COOPGAME_API ASTrackBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackBot();

protected:
	UPROPERTY(VisibleDefaultsOnly,Category="Component")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly,Category="Health")
	class USHealthComponent* BotHealthComp;

	//动态材质
	UPROPERTY(EditDefaultsOnly,Category="Component")
	class UMaterialInstanceDynamic* MaterialIns;

	//销毁时的粒子效果
	UPROPERTY(EditDefaultsOnly,Category="Component")
	class UParticleSystem* BoomEmitter;

	//销毁时声音
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USoundCue* BoomSound;

	//自杀时的声音
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USoundCue* SelfDamageSound;

	//球体组件，用于与角色发生碰撞后，进行自毁
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USphereComponent* SphereComponent;

	//下一个路径点坐标
	UPROPERTY(VisibleDefaultsOnly,Category="Move")
	FVector NextPathPoint;

	//施加的力的大小
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float CurForce;

	//距离阈值
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float AcquiredMinDistance;
		
	//是否改变速度
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	bool bUseVelocityChange;

	//是否已被销毁
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	bool bExplosion;

	//爆炸产生的基础伤害
	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float BaseDamage;

	//爆炸范围
	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float BoomRadius;

	//受伤害的时间间隔
	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;

	bool bSelfDied;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void DamageSelf();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//获取下一个路径点
	UFUNCTION()
	FVector GetNextPathPoint();

	//受伤害时触发
	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* HealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//血量归零后生成粒子效果
	UFUNCTION()
	void SelfDestruct();

	UFUNCTION()
	void OnPawnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
