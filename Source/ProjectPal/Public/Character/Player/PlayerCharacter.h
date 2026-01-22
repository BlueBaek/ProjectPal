// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;

// Action 상태를 확인할 Enum class
UENUM(BlueprintType)
enum class EMyActionState : uint8
{
	Idle = 0 UMETA(DisplayName="Idle"),
	Rolling UMETA(DisplayName="Rolling"),
	SpawningPal UMETA(DisplayName="SpawningPal"),
	DespawningPal UMETA(DisplayName="DespawningPal"),
	Exhaust UMETA(DisplayName="Exhaust"),
	Dead UMETA(DisplayName="Dead"),
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
	UAnimMontage* RollMontage;
	// 구르기(뒤) 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	UAnimMontage* RollBwdMontage;
	// 구르기(왼쪽) 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	UAnimMontage* RollLeftMontage;
	// 구르기(오른쪽) 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	UAnimMontage* RollRightMontage;

	// 속도 변화를 위한 변수
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float WalkSpeed = 150.0f;
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float AimWalkSpeed = 200.0f; // 조준 시 걷는 속도
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float JogSpeed = 400.0f;
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float SprintSpeed = 600.0f;

	// 구르기 속도 함수 : Montage Play Rate값
	UPROPERTY(EditAnywhere, Category="Movement", meta=(AllowPrivateAccess="true"))
	float RollSpeed = 1.0f;

	// ActionState를 저장할 변수 : 초기값은 Idle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(AllowPrivateAccess="true"))
	EMyActionState ActionState = EMyActionState::Idle;
	
	// Roll montage end callback
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	// 바인딩할 함수
	void Move(const FInputActionValue& Value); // 기본 움직임(Jogging)
	void Look(const FInputActionValue& Value); // 기본 시야
	void StartSprint(const FInputActionValue& Value); // 달리기 시작
	void StopSprint(const FInputActionValue& Value); // 달리기 끝
	void Roll();
	void SetAiming(bool isAiming);

	// Getter
	FORCEINLINE bool GetIsAiming() { return bIsAiming; }
};
