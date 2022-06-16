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

	//��̬����
	UPROPERTY(EditDefaultsOnly,Category="Component")
	class UMaterialInstanceDynamic* MaterialIns;

	//����ʱ������Ч��
	UPROPERTY(EditDefaultsOnly,Category="Component")
	class UParticleSystem* BoomEmitter;

	//����ʱ����
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USoundCue* BoomSound;

	//��ɱʱ������
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USoundCue* SelfDamageSound;

	//����������������ɫ������ײ�󣬽����Ի�
	UPROPERTY(EditDefaultsOnly, Category = "Component")
	class USphereComponent* SphereComponent;

	//��һ��·��������
	UPROPERTY(VisibleDefaultsOnly,Category="Move")
	FVector NextPathPoint;

	//ʩ�ӵ����Ĵ�С
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float CurForce;

	//������ֵ
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float AcquiredMinDistance;
		
	//�Ƿ�ı��ٶ�
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	bool bUseVelocityChange;

	//�Ƿ��ѱ�����
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	bool bExplosion;

	//��ը�����Ļ����˺�
	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float BaseDamage;

	//��ը��Χ
	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	float BoomRadius;

	//���˺���ʱ����
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

	//��ȡ��һ��·����
	UFUNCTION()
	FVector GetNextPathPoint();

	//���˺�ʱ����
	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* HealthComp, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//Ѫ���������������Ч��
	UFUNCTION()
	void SelfDestruct();

	UFUNCTION()
	void OnPawnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
