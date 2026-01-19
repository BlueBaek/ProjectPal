// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 스켈레탈메시 로드
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/_Pal/Import/Player/Pal_Player.Pal_Player"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}
	// Mesh의 Location, Rotation 설정
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -85.0f), FRotator(0.0f, -90.0f, 0.f));
	
	//캡슐 콜리전 크기 조절
	GetCapsuleComponent()->InitCapsuleSize(34.0f, 85.0f);

	// SpringArm 오브젝트 생성
	CCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CCameraArm"));
	CCameraArm->SetupAttachment(RootComponent);
	CCameraArm->TargetArmLength = 400.0f;
	CCameraArm->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 카메라 회전

	// Camera 오브젝트 생성
	CCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CCamera"));
	CCamera->SetupAttachment(CCameraArm, USpringArmComponent::SocketName);

	// 이동 설정
	bUseControllerRotationYaw = false; // 마우스 회전해도 캐릭터 회전하지않음
	
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로 캐릭터 회전
	GetCharacterMovement()->JumpZVelocity = 700.0f;	// 점프시 z축 속도
	GetCharacterMovement()->AirControl = 0.35f;	// 공중에 떠있는 속도
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;	// 최대 이동속도
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;	// 최소 아날로그 속도
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;	// 제동속도
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 입력 바인딩
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			SubSystem->AddMappingContext(CMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jump (Action)
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Move (Axis)
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		// Look (Axis)
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// 카메라 방향을 기준으로 앞, 뒤, 왼쪽, 오른쪽 계산
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 앞 구하기
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// 오른쪽 구하기
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 움직임 추가
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// 컨트롤러의 축 입력 추가
		AddControllerYawInput(LookAxisVector.X); // X축 : Yaw
		AddControllerPitchInput(LookAxisVector.Y); // Y축 : Pitch
	}
}
