// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature,USHealthComponent*,HealthComp,float,Health,
	float,HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Coop), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	//����ֵ��ͨ���ڱ����ı�ʱ���ú�����ʵ���ڿͻ�����Ҳִ�С�OnHealthChangedSignature���Ĺ���
	UPROPERTY(VisibleAnywhere,ReplicatedUsing=OnRep_Health)
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float DefaultHealth;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//���˺��󴥷����¼�,ֻ�ڷ�����ִ��
	UFUNCTION()
	void TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
		
	UPROPERTY(BlueprintAssignable,Category="Event")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable)
	float GetHealth();

};
