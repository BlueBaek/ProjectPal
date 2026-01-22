// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/MyPlayerController.h"
#include "KismetAnimationLibrary.h" // CalculateDirection을 위해 필요

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

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

	// SpringArm 오브젝트 생성 및 초기 값 입력
	CCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CCameraArm"));
	CCameraArm->SetupAttachment(RootComponent);
	CCameraArm->TargetArmLength = DefaultArmLength;
	CCameraArm->SocketOffset = FVector(0.0f, 80.0f, 50.0f);
	CCameraArm->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 카메라 회전

	// Camera 오브젝트 생성
	CCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CCamera"));
	CCamera->SetupAttachment(CCameraArm, USpringArmComponent::SocketName);

	// 이동 설정
	bUseControllerRotationYaw = false; // 마우스 회전해도 캐릭터 회전하지않음

	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로 캐릭터 회전
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 캐릭터 회전 속도
	GetCharacterMovement()->JumpZVelocity = 500.0f; // 점프시 z축 속도
	// GetCharacterMovement()->AirControl = 0.35f;	// 공중에 떠있는 속도
	GetCharacterMovement()->MaxWalkSpeed = JogSpeed; // 최대 이동속도
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f; // 최소 아날로그 속도
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f; // 제동속도

	// 구르기 몽타주 에셋 로드
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RollMontageAsset(
		TEXT("/Game/_Pal/BluePrint/Character/Montage/AM_Pal_Player_Anim_RollFwd.AM_Pal_Player_Anim_RollFwd"));
	if (RollMontageAsset.Succeeded()) { RollMontage = RollMontageAsset.Object; }
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

	// 카메라 줌 수치 결정
	float TargetFOV = bIsAiming ? AimFOV : DefaultFOV; // 시야각
	float TargetArmLength = bIsAiming ? AimArmLength : DefaultArmLength; // SpringArm 길이

	// FInterpTo를 사용해 부드럽게 전환
	CCamera->FieldOfView = FMath::FInterpTo(CCamera->FieldOfView, TargetFOV, DeltaTime, 10.f);
	CCameraArm->TargetArmLength = FMath::FInterpTo(CCameraArm->TargetArmLength, TargetArmLength, DeltaTime, 10.f);
	
	// Tick의 On/Off를 위해 목표치에 도달했는지 체크 (오차 범위 내)
	bool bFOVReached = FMath::IsNearlyEqual(CCamera->FieldOfView, TargetFOV, 0.1f);
	bool bArmReached = FMath::IsNearlyEqual(CCameraArm->TargetArmLength, TargetArmLength, 0.1f);
	
	if (bFOVReached && bArmReached)
	{
		// 값을 목표치로 완전히 고정하고 Tick을 끕니다.
		CCamera->FieldOfView = TargetFOV;
		CCameraArm->TargetArmLength = TargetArmLength;
        
		SetActorTickEnabled(false);
	}
}

void APlayerCharacter::SetActionState(EMyActionState NewState)
{
	if (ActionState == NewState) return;
	ActionState = NewState;
	
	// 필요에 따라 이곳에서 OnStateChanged 델리게이트 브로드캐스트 가능
}

void APlayerCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (ActionState != EMyActionState::Rolling) return;
	SetActionState(EMyActionState::Idle);
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
	{
		PC->UpdateInputContext();
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

void APlayerCharacter::StartSprint(const FInputActionValue& Value)
{
	if (!bIsAiming)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; // 최대 이동속도 변경
	}
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
	if (!bIsAiming)
	{
		GetCharacterMovement()->MaxWalkSpeed = JogSpeed; // 최대 이동속도 변경
	}
}

void APlayerCharacter::Roll()
{
	// 공중에서는 구르지 못함
	if (GetCharacterMovement()->IsFalling()) return;
	// 이미 구르는 중이면 무시
	if (ActionState == EMyActionState::Rolling) return;

	// --- 즉시 회전 로직 추가 ---
	// 현재 이동 입력 벡터를 가져옵니다 (MoveAction의 현재 값)
	// Controller에서 현재 입력 중인 축 값을 확인하거나, 마지막 이동 입력을 참조합니다.
	FVector LastInput = GetLastMovementInputVector();
	// 만약 입력이 있다면 (방향키를 누르고 있다면)
	if (!LastInput.IsNearlyZero())
	{
		// 입력 방향을 바라보도록 즉시 회전
		FRotator NewRotation = LastInput.Rotation();
		NewRotation.Pitch = 0.f;
		NewRotation.Roll = 0.f;
		SetActorRotation(NewRotation);
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !RollMontage) return;
	
	// ActionState 업데이트
	SetActionState(EMyActionState::Rolling);
	// IMC 업데이트
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
	{
		PC->UpdateInputContext();
	}

	// 몽타주 실행
	AnimInstance->Montage_Play(RollMontage, RollSpeed);

	// 몽타주의 끝을 확인하는 방식을 타이머 대신 델리게이트 방식으로 수정
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &APlayerCharacter::OnRollMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);
}

void APlayerCharacter::SetAiming(bool isAiming)
{
	bIsAiming = isAiming;
	
	// 조준 상태가 바뀌면 보간(Interpolation)을 위해 Tick 활성화
	SetActorTickEnabled(true);
	
	// 조준 모드에 따라 캐릭터 움직임에 변화를 주기 위함
	if (bIsAiming)	// 조준 중
	{
		// 이동 방향으로 자동회전 false
		GetCharacterMovement()->bOrientRotationToMovement = false;
		// 컨트롤러의 회전값(Yaw)을 사용
		bUseControllerRotationYaw = true;
		// 이동 속도 감소
		GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
	}
	else	// 조준 해제
	{
		// 이동 방향으로 자동회전 true
		GetCharacterMovement()->bOrientRotationToMovement = true;
		// 이동 방향으로 캐릭터가 회전하도록 복구
		bUseControllerRotationYaw = false;
		// 이동 속도 복구
		GetCharacterMovement()->MaxWalkSpeed = JogSpeed;
	}
	
	/* 디버깅용 출력 메세지
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
	                                 FString::Printf(TEXT("Aiming : %s"), bIsAiming ? TEXT("True") : TEXT("False")));
	                                 */
}
