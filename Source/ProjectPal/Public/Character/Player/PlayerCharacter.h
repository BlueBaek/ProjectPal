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
class UOwnedPalComponent;

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

// 팰스피어 투척 상태
UENUM(BlueprintType)
enum class EPalSphereThrowState : uint8
{
	None,
	Holding,     // Q 누르는 중 (조준 강제 ON, 프리뷰/홀드몽타주)
	Throwing     // 던지는 중 (Throw 섹션 재생~Notify 발사까지)
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UOwnedPalComponent> OwnedPalComponent;
	
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
	
	// ===== 장비 교체 ===== 
	// 장비 슬롯 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Equip", meta=(AllowPrivateAccess="true", ClampMin="1"))
	int32 EquipSlotCount = 4;

	// 현재 장비 Index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Equip", meta=(AllowPrivateAccess="true"))
	int32 CurrentEquipSlotIndex = 0;
	
	// 무기 전용 슬롯 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equip|Slots", meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<UWeaponDataAsset>> EquipWeaponSlots;
	
	// 테스트용: SwordData를 BP에서 지정해두고 2번째 슬롯에 자동 배치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equip|Slots", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWeaponDataAsset> SwordData;
	
	// UI 갱신용(선택 슬롯 변경 이벤트)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipSlotChanged, int32, NewIndex, int32, OldIndex);

	UPROPERTY(BlueprintAssignable, Category="Item|Equip")
	FOnEquipSlotChanged OnEquipSlotChanged;
	
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
	FORCEINLINE UOwnedPalComponent* GetOwnedPalComponent() const { return OwnedPalComponent; }
	
	UFUNCTION(BlueprintCallable, Category="Item|Equip")
	void SetEquipSlotCount(int32 NewCount);

	UFUNCTION(BlueprintCallable, Category="Item|Equip")
	void ChangeEquipSlotByWheel(float WheelAxisValue); // 휠 입력값(+/-)
	
	UFUNCTION(BlueprintPure, Category="Item|Equip")
	int32 GetCurrentEquipSlotIndex() const { return CurrentEquipSlotIndex; }
	
	// 즉시 장착용 함수
	UFUNCTION(BlueprintCallable, Category="Equip|Slots")
	void EquipWeaponFromCurrentSlot();

	UFUNCTION(BlueprintCallable, Category="Equip|Slots")
	void SetWeaponToSlot(int32 SlotIndex, UWeaponDataAsset* WeaponData);
	
public:
	// 팰스피어 관련 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Capture", meta=(AllowPrivateAccess="true"))
	EPalSphereThrowState PalSphereState = EPalSphereThrowState::None;

	EPalSphereThrowState getPalSphereState() {return PalSphereState;}
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Capture", meta=(AllowPrivateAccess="true"))
	bool bAimWasActiveBeforePalSphere = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Capture", meta=(AllowPrivateAccess="true"))
	bool bPalSphereCancelRequested = false;
	
	// 몽타주(블루프린트에서 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Capture")
	UAnimMontage* PalSphereMontage = nullptr;
	
	// 섹션 이름
	inline static const FName SECTION_Hold = TEXT("Hold");
	inline static const FName SECTION_Throw = TEXT("Throw");
	
	// Q Started
	UFUNCTION(BlueprintCallable, Category="Capture")
	void PalSphereHold();
	// Q Completed
	UFUNCTION(BlueprintCallable, Category="Capture")
	void PalSphereThrow();
	// RMB(=AimAction Started)로 캔슬
	UFUNCTION(BlueprintCallable, Category="Capture")
	void CancelPalSphereThrow();
	// 컨트롤러가 체크할 용도
	UFUNCTION(BlueprintPure, Category="Capture")
	bool IsPalSphereHolding() const { return PalSphereState == EPalSphereThrowState::Holding; }
	
	// Notify에서 호출(던지는 타이밍)
	UFUNCTION(BlueprintCallable, Category="Capture")
	void AnimNotify_PalSphereThrow();
	
	UFUNCTION()
	void OnPalSphereMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnPalSphereMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

private:
	void EndPalSphereHold(bool bRestoreAim);
	void JumpToPalSphereSection(const FName& SectionName);
	
	// ===== PalSphere관련 변수/함수 추가 =====
protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Capture|PalSphere")
	TSubclassOf<class APJ_PalSphere> APJ_PalSphereClass;

	UPROPERTY(EditDefaultsOnly, Category="Capture|PalSphere")
	USkeletalMesh* PalSphereAsset;

	// 소켓 이름 : Socket_Weapon_R
	UPROPERTY(EditDefaultsOnly, Category="Capture|PalSphere")
	FName PalSphereHandSocketName = TEXT("Socket_Weapon_R");

	// 던진 물체 속도
	UPROPERTY(EditDefaultsOnly, Category="Capture|PalSphere")
	float PalSphereThrowSpeed = 1600.f;

	// 곡선형태로 던질 때 위로 휘는 정도
	UPROPERTY(EditDefaultsOnly, Category="Capture|PalSphere")
	float PalSphereThrowUpBoost = 250.f;

	// “손에 쥐고 있는” 프리뷰
	UPROPERTY()
	USkeletalMeshComponent* HeldPalSphereMesh = nullptr;
	
	void ShowHeldSpherePreview(bool bShow);
};
