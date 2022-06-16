// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	//��������
	SetIsReplicated(true);

	DefaultHealth=100.0f;
}

// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = DefaultHealth;
	//ֻ�ڷ������ϰ�ִ���˺��¼�
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::TakeDamage);
		}
	}
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	//����TakeDamage�е�.Broadcast���ڷ��������У�Ϊʹ��������ڿͻ��ˣ����½���.Broadcast
	float DamageValue=Health-OldHealth;
	OnHealthChanged.Broadcast(this, Health, DamageValue, nullptr, nullptr, nullptr);
}

//���Ʊ���
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USHealthComponent,Health);
}
 
// Called every frame
void USHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage > 0.0f && Health>0.0f)
	{
		Health=FMath::Clamp(Health-Damage,0.0f,DefaultHealth);
		//UE_LOG(LogTemp, Warning, TEXT("Current Health is %f"), Health);
	}

	OnHealthChanged.Broadcast(this,Health,Damage,DamageType,InstigatedBy,DamageCauser);
}
	
float USHealthComponent::GetHealth()
{
	return Health;
}

