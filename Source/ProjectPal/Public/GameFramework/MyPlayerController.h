// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class PROJECTPAL_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPlayerController();

	// 상태에 따른 InputContext 교체 함수
	void UpdateInputContext();
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// 카메라 상하(Pitch) 제한
	UPROPERTY(EditDefaultsOnly, Category="Camera|Clamp", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float ViewPitchMin = -45.0f;   // 아래

	UPROPERTY(EditDefaultsOnly, Category="Camera|Clamp", meta=(ClampMin="-89.0", ClampMax="89.0"))
	float ViewPitchMax = 50.0f;    // 위
	
	// --- 입력 액션 변수들 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* IdleIMC;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* RollingIMC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RollAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* AimAction;
	
	// 장비 슬롯 변경(마우스 휠)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	UInputAction* EquipChangeAction;
	
	// 공격 (CombatComponent에서 적용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* AttackAction;
	
	// 포획
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* CatchAction;
	
	// 소환
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SpawnAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PrevPalSlotAction; // 1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* NextPalSlotAction; // 3
	
	// 현재 적용 중인 “상태 IMC”를 추적해서, 이것만 안전하게 교체
	UPROPERTY(Transient)	// 디스크 저장 제외(불필요한 데이터 낭비를 막음)
	TObjectPtr<UInputMappingContext> CurrentStateIMC = nullptr;

	// 상태 IMC 우선순위 (필요하면 조절)
	UPROPERTY(EditDefaultsOnly, Category="Input")
	int32 StateIMCPriority = 0;

	// --- 입력 처리 함수들 ---
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Jump();
	void Input_StopJump();
	void Input_StartSprint();
	void Input_StopSprint();
	void Input_Roll();
	void Input_StartAim();
	void Input_StopAim();
	void Input_StartAttack();
	void Input_StopAttack();
	void Input_EquipChange(const FInputActionValue& Value);
	void Input_PalSphereHold();
	void Input_PalSphereThrow();
	void Input_PalSphereCancel();
	void Input_TogglePalSpawn();   // E
	void Input_PrevPalSlot();    // 1
	void Input_NextPalSlot();    // 3
	
private:
	// 휠 디바운스용
	double LastEquipWheelTimeSec = -999.0;

	// 0.06~0.12 추천 (휠 한 칸이 1~2프레임 이상 들어오는 환경 대비)
	UPROPERTY(EditAnywhere, Category="Input|Wheel")
	float EquipWheelDebounceSec = 0.08f;
};