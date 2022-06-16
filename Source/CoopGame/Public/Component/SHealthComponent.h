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
	//生命值；通过在变量改变时调用函数，实现在客户端上也执行“OnHealthChangedSignature”的功能
	UPROPERTY(VisibleAnywhere,ReplicatedUsing=OnRep_Health)
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float DefaultHealth;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//受伤害后触发的事件,只在服务器执行
	UFUNCTION()
	void TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
		
	UPROPERTY(BlueprintAssignable,Category="Event")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable)
	float GetHealth();

};
