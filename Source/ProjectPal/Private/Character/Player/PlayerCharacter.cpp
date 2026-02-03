// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Player/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "DataAsset/WeaponDataAsset.h"
#include "GameFramework/MyPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Component/PlayerStatComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

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
	// 앞
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RollMontageAsset(
		TEXT("/Game/_Pal/BluePrint/Character/Player/Montage/AM_Pal_Player_RollFwd.AM_Pal_Player_RollFwd"));
	if (RollMontageAsset.Succeeded()) { RollMontage = RollMontageAsset.Object; }
	// 뒤
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RollBwdMontageAsset(
		TEXT("/Game/_Pal/BluePrint/Character/Player/Montage/AM_Pal_Player_FlipBwd.AM_Pal_Player_FlipBwd"));
	if (RollBwdMontageAsset.Succeeded()) { RollBwdMontage = RollBwdMontageAsset.Object; }
	// 왼쪽
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RollLeftMontageAsset(
		TEXT("/Game/_Pal/BluePrint/Character/Player/Montage/AM_Pal_Player_RollLeft.AM_Pal_Player_RollLeft"));
	if (RollLeftMontageAsset.Succeeded()) { RollLeftMontage = RollLeftMontageAsset.Object; }
	// 오른쪽
	static ConstructorHelpers::FObjectFinder<UAnimMontage> RollRightMontageAsset(
		TEXT("/Game/_Pal/BluePrint/Character/Player/Montage/AM_Pal_Player_RollRight.AM_Pal_Player_RollRight"));
	if (RollRightMontageAsset.Succeeded()) { RollRightMontage = RollRightMontageAsset.Object; }

	// Component 부착
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	StatComponent = CreateDefaultSubobject<UPlayerStatComponent>(TEXT("StatComponent"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 최초 스탯 적용 (레벨, HP, Stamina, Attack, Defense, true : 최대 최력, 최대 스테미나로 적용)
	StatComponent->InitializeStats(1, 500.f, 100.f, 100.f, 100.f, true);
	// 맨손 AnimLayer가 적용되는지 확인
	ApplyUnarmedAnimLayer();
	
	// 슬롯 배열 크기 보장
	EquipWeaponSlots.SetNum(EquipSlotCount);

	// ✅ "2번째 칸" = 인덱스 1
	if (SwordData)
	{
		EquipWeaponSlots[1] = SwordData;
	}

	// 시작 시 현재 슬롯 무기 장착(원하면)
	EquipWeaponFromCurrentSlot();
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

	// ===== 스탯 컴포넌트 디버그용 =====
	if (StatComponent)
	{
		StatComponent->DrawDebugStat();
	}
}

void APlayerCharacter::SetActionState(EMyActionState NewState)
{
	if (ActionState == NewState) return;
	ActionState = NewState;

	// 필요에 따라 이곳에서 OnStateChanged 델리게이트 브로드캐스트 가능
}

void APlayerCharacter::SetEquipSlotCount(int32 NewCount)
{
	NewCount = FMath::Max(1, NewCount);
	EquipSlotCount = NewCount;

	// 슬롯 수가 바뀌면 현재 인덱스가 범위를 벗어날 수 있으니 보정
	CurrentEquipSlotIndex = FMath::Clamp(CurrentEquipSlotIndex, 0, EquipSlotCount - 1);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 아주 작은 입력은 0으로 처리(데드존)
	if (FMath::Abs(MovementVector.X) < 0.1f) MovementVector.X = 0.f;
	if (FMath::Abs(MovementVector.Y) < 0.1f) MovementVector.Y = 0.f;

	CurrentMoveInput2D = MovementVector;

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
	bSprintHeld = true;

	// 공격 중/조준 중에는 달리기 적용 금지
	if (bIsAttacking || bIsAiming) return;

	UpdateMoveSpeed();
}

void APlayerCharacter::StopSprint(const FInputActionValue& Value)
{
	bSprintHeld = false;

	UpdateMoveSpeed();
}

void APlayerCharacter::Roll()
{
	// 공중에서는 구르지 못함
	if (GetCharacterMovement()->IsFalling()) return;
	// 이미 구르는 중이면 무시
	if (ActionState == EMyActionState::Rolling) return;
	// 몽타주를 실행시킬 Anim인스턴스
	TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	// 구르기 시작 순간 조준 여부를 저장
	const bool bWasAiming = bIsAiming;

	// --- 조준 여부에 따라 사용할 몽타주 결정 ---
	TObjectPtr<UAnimMontage> MontageToPlay = RollMontage; // 실행할 몽타주 선택, 기본값 앞구르기
	if (bWasAiming) // 조준 중이면 몽타주 선택
	{
		MontageToPlay = SelectRollMontage_Aiming();
	}
	else // 그렇지 않으면 기본 구르기(비조준 상태)
	{
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
	}

	// 선택된 몽타주가 없으면 무시
	if (!MontageToPlay) return;

	// 조준 모드 비활성화
	if (bWasAiming)
	{
		SetAiming(false);
	}

	// ActionState 업데이트
	SetActionState(EMyActionState::Rolling);

	// IMC 업데이트
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
	{
		PC->UpdateInputContext();
	}

	// 몽타주 실행
	// AnimInstance->Montage_Play(MontageToPlay, RollSpeed);
	AnimInstance->Montage_Play(MontageToPlay);

	// 몽타주의 끝을 확인하는 방식을 타이머 대신 델리게이트 방식으로 수정
	FOnMontageEnded EndDelegate; // 몽타주 재생 종료 델리게이트 인스턴스
	EndDelegate.BindUObject(this, &APlayerCharacter::OnRollMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
}

// 구르기 몽타주가 끝났을 때 호출되는 함수
void APlayerCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// ActionState가 Rolling일 때만 작동
	if (ActionState != EMyActionState::Rolling) return;
	// ActionState 수정 : Idle
	SetActionState(EMyActionState::Idle);
	// MyPlayerController에서 IMC 업데이트
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
	{
		PC->UpdateInputContext();
	}
}

// 조준 모드에서 몽타주 선택 함수
TObjectPtr<UAnimMontage> APlayerCharacter::SelectRollMontage_Aiming() const
{
	// 방향키 입력 갱신용
	const FVector2D Input = CurrentMoveInput2D;

	// 방향입력 없음 : 뒷구르기
	// 몽타주가 없을 때를 대비해 Return할 때 기존에 만들어 둔 RollMontage 사용(방어적 프로그래밍)
	if (Input.IsNearlyZero())
	{
		return RollBwdMontage ? RollBwdMontage : RollMontage;
	}

	// 좌 or 좌대각선 : 왼쪽 구르기
	if (Input.X < 0.f) // Left or LeftDiagonal
	{
		return RollLeftMontage ? RollLeftMontage : RollMontage;
	}
	// 우 or 우대각선 : 오른쪽 구르기
	if (Input.X > 0.f) // Right or RightDiagonal
	{
		return RollRightMontage ? RollRightMontage : RollMontage;
	}

	// 앞
	if (Input.Y > 0.f) // Forward
	{
		return RollMontage; // 기존 앞구르기
	}

	// 뒤 구르기
	return RollBwdMontage ? RollBwdMontage : RollMontage;
}

void APlayerCharacter::SetAiming(bool isAiming)
{
	// Idle이 아니라면 활성되지 않음
	if (!(ActionState == EMyActionState::Idle)) return;

	bIsAiming = isAiming;

	// 조준 상태가 바뀌면 보간(Interpolation)을 위해 Tick 활성화
	SetActorTickEnabled(true);
	
	UpdateRotationControl(); // 시점 고정 함수
	UpdateMoveSpeed(); // 속도 변화용 함수
}

void APlayerCharacter::Attack(bool isAttacking)
{
	bIsAttacking = isAttacking;
	// 공격 중엔 시점 고정

	// bUseControllerRotationYaw = isAttacking;
	// GetCharacterMovement()->bOrientRotationToMovement = !isAttacking;
	UpdateRotationControl(); // 시점 고정 함수로 교체

	UpdateMoveSpeed();
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, "Attacking: " + bIsAttacking ? "true" : "false");
}

// State별 MovementSpeed 제어
void APlayerCharacter::UpdateMoveSpeed()
{
	if (!GetCharacterMovement()) return;

	// 우선순위: 조준 > 공격 > 스프린트 > 기본
	if (bIsAiming)
	{
		GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
	}
	else if (bIsAttacking)
	{
		GetCharacterMovement()->MaxWalkSpeed = AttackWalkSpeed;
	}
	else if (bSprintHeld)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = JogSpeed;
	}
}

void APlayerCharacter::UpdateRotationControl()
{
	if (!GetCharacterMovement()) return;

	if (bIsAiming || bIsAttacking)
	{
		// 이동 방향으로 자동회전 false
		GetCharacterMovement()->bOrientRotationToMovement = false;
		// 컨트롤러의 회전값(Yaw)을 사용
		bUseControllerRotationYaw = true;
	}
	else
	{
		// 이동 방향으로 자동회전 true
		GetCharacterMovement()->bOrientRotationToMovement = true;
		// 이동 방향으로 캐릭터가 회전하도록 복구
		bUseControllerRotationYaw = false;
	}
}

void APlayerCharacter::ChangeEquipSlotByWheel(float WheelAxisValue)
{
	// 구르는 중이면 막기
	if (ActionState == EMyActionState::Rolling) return;
	// 공격 중이면 막기
	if (bIsAttacking) return;
	
	// 마우스 휠 축 입력 (-1, 0, 1)
	if (FMath::IsNearlyZero(WheelAxisValue)) return;	// 부동소수점 오차 해결
	if (EquipSlotCount <= 1) return;
	
	const int32 OldIndex = CurrentEquipSlotIndex;
	
	// WheelAxisValue < 0 : 위로 굴림(이전 슬롯)
	// WheelAxisValue > 0 : 아래로 굴림(다음 슬롯)
	const int32 Step = (WheelAxisValue > 0.f) ? -1 : +1;
	
	//
	CurrentEquipSlotIndex = (CurrentEquipSlotIndex + Step) % EquipSlotCount;
	if (CurrentEquipSlotIndex < 0 )
	{
		CurrentEquipSlotIndex += EquipSlotCount;
	}
	
	if (OldIndex != CurrentEquipSlotIndex)
	{
		// 여기서 즉시 장착!
		EquipWeaponFromCurrentSlot();
		OnEquipSlotChanged.Broadcast(CurrentEquipSlotIndex, OldIndex);
		
		// 디버그용
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow,
				FString::Printf(TEXT("Equip Slot: %d / %d"), CurrentEquipSlotIndex + 1, EquipSlotCount)
			);
		}
	}
}

// 현재 슬롯 무기를 CombatComponent에 넘겨 장착
void APlayerCharacter::EquipWeaponFromCurrentSlot()
{
	EquipWeaponSlots.SetNum(EquipSlotCount);

	UWeaponDataAsset* WeaponDA =
		EquipWeaponSlots.IsValidIndex(CurrentEquipSlotIndex)
		? EquipWeaponSlots[CurrentEquipSlotIndex]
		: nullptr;

	if (!CombatComponent) return; // 너 프로젝트의 CombatComponent 멤버/Getter로 맞추기

	// 빈 슬롯이면 Unarmed로 폴백하고 싶다면 null 전달 허용 + Combat에서 처리
	CombatComponent->EquipWeaponData(WeaponDA);
	
	// 디버그 용
	UE_LOG(LogTemp, Warning, TEXT("Equip Slot %d: %s"), CurrentEquipSlotIndex,
	WeaponDA ? *WeaponDA->GetName() : TEXT("Unarmed"));
}

// 슬롯에 무기를 넣는 함수
void APlayerCharacter::SetWeaponToSlot(int32 SlotIndex, UWeaponDataAsset* WeaponData)
{
	EquipWeaponSlots.SetNum(EquipSlotCount);

	if (!EquipWeaponSlots.IsValidIndex(SlotIndex)) return;
	EquipWeaponSlots[SlotIndex] = WeaponData;
}

void APlayerCharacter::ApplyUnarmedAnimLayer()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	TSubclassOf<UAnimInstance> Desired = UnarmedData ? UnarmedData->AnimLayerClass : nullptr;

	if (CurrentLinkedLayerClass && CurrentLinkedLayerClass != Desired)
	{
		MeshComp->UnlinkAnimClassLayers(CurrentLinkedLayerClass);
		CurrentLinkedLayerClass = nullptr;
	}

	if (Desired && CurrentLinkedLayerClass != Desired)
	{
		MeshComp->LinkAnimClassLayers(Desired);
		CurrentLinkedLayerClass = Desired;

		UE_LOG(LogTemp, Warning, TEXT("[AnimLayer] Linked: %s"), *GetNameSafe(Desired));
	}
}