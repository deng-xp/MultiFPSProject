// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SObstacle.generated.h"

UCLASS()
class COOPGAME_API ASObstacle : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing=OnRep_bExplode)
	bool bExplode;

	UFUNCTION()
	void OnRep_bExplode();

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere,Category="Component")
	class USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category="Component")
	class URadialForceComponent* RadialForce;

	UPROPERTY(EditDefaultsOnly, Category="Emitter")
	class UParticleSystem* BoomSystem;

	UPROPERTY(EditDefaultsOnly, Category = "Emitter")
	class UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Component")
	float ExplosionImpulse;
	
	UPROPERTY(EditInstanceOnly, Category = "Component")
	float ExplosionRadius;

public:	
	// Sets default values for this actor's properties
	ASObstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnHealthChangedCur(USHealthComponent* HealthComponents, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

};
