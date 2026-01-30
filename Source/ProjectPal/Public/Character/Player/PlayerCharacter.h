// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class UCombatComponent;
class UWeaponDataAsset;
class UAnimInstance;
class UPlayerStatComponent;

// Action 상태를 확인할 Enum class
UENUM(BlueprintType)
enum class EMyActionState : uint8
{
	Idle = 0		UMETA(DisplayName="Idle"),
	Rolling			UMETA(DisplayName="Rolling"),
	SpawningPal		UMETA(DisplayName="SpawningPal"),
	DespawningPal	UMETA(DisplayName="DespawningPal"),
	Exhaust			UMETA(DisplayName="Exhaust"),
	// Dead			UMETA(DisplayName="Dead"),
};

UCLASS()
class PROJECTPAL_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 상태 접근
	UFUNCTION(BlueprintPure, Category="State")
	FORCEINLINE EMyActionState GetActionState() const { return ActionState; }
	
	UFUNCTION(BlueprintCallable, Category="State")
	void SetActionState(EMyActionState NewState);

private:
	
	// 카메라 붐 스틱
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	USpringArmComponent* CCameraArm;
	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	UCameraComponent* CCamera;

	// --- 카메라 관련 변수 ---
	// 줌 상태 확인용 boolean, 블루프린트에서 읽을 수 있도록 UPROPERTY 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	bool bIsAiming = false;
	// 시야각
	float DefaultFOV = 90.f; // 기본 시야각
	float AimFOV = 70.f; // 줌 상태 시야각
	// 스프링 암
	float DefaultArmLength = 200.0f; // 기본 SpringArm 길이
	float AimArmLength = 180.0f; // 줌 상태 SpringArm 길이

	// 구르기 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	// UAnimMontage* RollMontage;
	TObjectPtr<UAnimMontage> RollMontage;
	
	// 구르기(뒤) 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimMontage> RollBwdMontage;
	// 구르기(왼쪽) 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimMontage> RollLeftMontage;
	// 구르기(오른쪽) 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimMontage> RollRightMontage;

	// 속도 변화를 위한 변수
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float WalkSpeed = 150.0f;
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float AimWalkSpeed = 200.0f; // 조준 시 걷는 속도
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float AttackWalkSpeed = 300.0f; // 공격 시 걷는 속도
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float JogSpeed = 400.0f;
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float SprintSpeed = 600.0f;

	// 구르기 속도 변수 : Montage Play Rate값
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float RollSpeed = 1.0f;
	// 마지막 이동 입력 : 줌모드 구르기 구현용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	FVector2D CurrentMoveInput2D = FVector2D::ZeroVector;

	// ActionState를 저장할 변수 : 초기값은 Idle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(AllowPrivateAccess="true"))
	EMyActionState ActionState = EMyActionState::Idle;
	
	// 구르기 몽타주가 끝날 때 호출할 함수
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	// 조준 모드에서 구르기 몽타주 선택 함수
	TObjectPtr<UAnimMontage> SelectRollMontage_Aiming() const;
	
	// ---- Components ----
	// StatComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stat", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlayerStatComponent> StatComponent;
	
	// CombatComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCombatComponent> CombatComponent;
	// ---- ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(AllowPrivateAccess="true"))
	bool bIsAttacking = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(AllowPrivateAccess="true"))
	bool bSprintHeld = false;
	
	// 무기 교체 및 모션 변경을 위한 변수 함수.
	UPROPERTY(EditDefaultsOnly, Category="Combat|Data")
	TObjectPtr<UWeaponDataAsset> UnarmedData; // 맨손 기본 데이터(AnimLayer 포함)

	UPROPERTY()
	TSubclassOf<UAnimInstance> CurrentLinkedLayerClass;	// 현재 링크된 LayerClass
	
	// AnimLayer적용
	void ApplyUnarmedAnimLayer();
	
public:
	// 바인딩할 함수
	void Move(const FInputActionValue& Value); // 기본 움직임(Jogging)
	void Look(const FInputActionValue& Value); // 기본 시야
	void StartSprint(const FInputActionValue& Value); // 달리기 시작
	void StopSprint(const FInputActionValue& Value); // 달리기 끝
	void Roll();
	void SetAiming(bool isAiming);
	void Attack(bool isAttacking);
	void UpdateMoveSpeed();		// 이동속도 변화를 부드럽게 하기 위한 함수
	void UpdateRotationControl();		// 이동속도 변화를 부드럽게 하기 위한 함수
	
	// Getter
	FORCEINLINE bool GetIsAiming() { return bIsAiming; }	// 조준 상태인지 확인
	FORCEINLINE bool GetIsAttacking() { return bIsAttacking; }	// 조준 상태인지 확인
	FORCEINLINE UPlayerStatComponent* GetStatComponent() const { return StatComponent; }	// StatComponent
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }	// CombatComponent
	
};
