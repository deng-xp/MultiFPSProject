// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SFPSCharacter.generated.h"

class ASFPSWeapon;
UCLASS()
class COOPGAME_API ASFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASFPSCharacter();

protected:
	//生命值组件
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly,Replicated, Category = "Components")
	class USHealthComponent* HealthComponent;

	UPROPERTY(Replicated)
	ASFPSWeapon* CurWeapon;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly,Replicated, Category = "Player")
	bool bDied;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComponent;

	/*开镜相关的参数*/
	float DefaultView;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zoom")
	float TargetView;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zoom", Meta = (ClampMin = 0.0, ClampMax = 100.0));
	float ZoomSpeed;

	bool IsZoom;

	//生成枪械相关
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	TSubclassOf<ASFPSWeapon> Weaponclass;

	UPROPERTY(VisibleDefaultsOnly,Category="Weapon")
	FName SocketName;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//设置要复制的变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

public:
	/** Returns	Pawn's eye location */
	virtual FVector GetPawnViewLocation() const override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	//开镜函数
	UFUNCTION(Category = "Zoom")
	void BeginZoom();

	UFUNCTION(Category = "Zoom")
	void EndZoom();

	//开枪函数
	UFUNCTION(Category="Weapon")
	void Fire();

	//生命值变化触发的函数，用于检测死亡
	UFUNCTION()
	void OnHealthChange(USHealthComponent* HealthComp, float Health,float HealthDelta, 
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};
