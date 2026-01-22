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
};