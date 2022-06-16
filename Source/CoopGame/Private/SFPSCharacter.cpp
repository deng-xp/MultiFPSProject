// Fill out your copyright notice in the Description page of Project Settings.

#include "SFPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SFPSWeapon.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame/CoopGame.h"
#include "Component/SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASFPSCharacter::ASFPSCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	//使用角色控制器的面向
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON,ECR_Ignore);

	HealthComponent=CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	//开启蹲伏
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	IsZoom = false;
	TargetView = 45.0f;
	ZoomSpeed = 20.0f;
	bDied=false;
	SocketName="Hold_Rifle";
}

// Called when the game starts or when spawned
void ASFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultView=CameraComponent->FieldOfView;
	//绑定生命值变化事件
	//此处逻辑为：actor受伤害，触发其OnTakeAnyDamage事件，该事件进一步调用Health组件中的TakeDamage函数，触发Health组件中的OnHealthChanged事件，
	//进而触发此处的OnHealthChange
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASFPSCharacter::OnHealthChange);

	if (HasAuthority())
	{
		//生成初始化枪械
		if (Weaponclass)
		{
			FActorSpawnParameters Parameters;
			Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			CurWeapon = GetWorld()->SpawnActor<ASFPSWeapon>(Weaponclass, FVector(0, 0, 0), FRotator(0, 0, 0), Parameters);

			if (CurWeapon)
			{
				CurWeapon->SetOwner(this);
				CurWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			}
		}
	}
}

void ASFPSCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ASFPSCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ASFPSCharacter::BeginCrouch()
{
	Crouch();
}

void ASFPSCharacter::EndCrouch()
{
	UnCrouch();
}

// Called every frame
void ASFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float FinalView=IsZoom?TargetView:DefaultView;
	float CurView=FMath::FInterpTo(CameraComponent->FieldOfView,FinalView,DeltaTime,ZoomSpeed);
	CameraComponent->SetFieldOfView(CurView);
}

// Called to bind functionality to input
void ASFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//移动相关
	PlayerInputComponent->BindAxis("MoveForward", this, &ASFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASFPSCharacter::MoveRight);
	//视角相关
	PlayerInputComponent->BindAxis("LookUp", this, &ASFPSCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASFPSCharacter::AddControllerYawInput);
	//下蹲
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASFPSCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASFPSCharacter::EndCrouch);
	//跳跃
	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//开镜
	PlayerInputComponent->BindAction("Zoom",IE_Pressed,this,&ASFPSCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASFPSCharacter::EndZoom);
	//开枪
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&ASFPSCharacter::Fire);
}

void ASFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASFPSCharacter,CurWeapon);
	DOREPLIFETIME(ASFPSCharacter, bDied);
}

FVector ASFPSCharacter::GetPawnViewLocation() const
{
	if (CameraComponent)
	{
		FVector Location=CameraComponent->GetComponentLocation();
		return Location;
	}
	return Super::GetPawnViewLocation();
}

void ASFPSCharacter::BeginZoom()
{
	IsZoom=true;
}

void ASFPSCharacter::EndZoom()
{
	IsZoom=false;
}

void ASFPSCharacter::Fire()
{
	CurWeapon->Fire();
}

void ASFPSCharacter::OnHealthChange(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0 && !bDied)
	{
		//HealthComponent中的健康改变发生在服务器，间接保证了bDied的变化只发生在服务器
		bDied=true;

		//停止移动
		GetMovementComponent()->StopMovementImmediately();
		//禁用碰撞
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//解除控制
		DetachFromControllerPendingDestroy();
		//一定时间后销毁
		SetLifeSpan(10.0f);
	}
}

