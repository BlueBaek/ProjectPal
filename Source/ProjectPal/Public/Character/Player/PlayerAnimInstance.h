// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation(); // AnimInstance가 처음 생성될 때 한번 호출됨
	virtual void NativeUpdateAnimation(float DeltaTime); // 매 프레임 호출

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class ACharacter* Player;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class UCharacterMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;	// 속도
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction;	// 방향
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Acceleration;	// 가속
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool IsMoving;	// 움직임 여부 검사

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool IsFalling;	// 공중 검사
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool IsRising;	// 올라가는 중인지 검사
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool IsAiming;	// 조준중인지 확인
	
	// 플레이어의 상체가 시점에 따라 꺾이는 기능 추가를 위함
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim")
	float AimPitch = 0.f;
	
	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool IsAttacking;	// 조준중인지 확인
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool IsReloading;	// 조준중인지 확인
	*/
	
	// --- 추가: 조준 상체 대기 몽타주 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim|Montage")
	TObjectPtr<UAnimMontage> AimUpperIdleMontage = nullptr;

	// 조준 해제 시 몽타주 블렌드 아웃 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim|Montage")
	float AimUpperIdleStopBlendOutTime = 0.15f;
	
private:
	// 이전 프레임 조준값(변화 감지용)
	bool bPrevIsAiming = false;

	// 조준 상태 변화에 따른 몽타주 처리
	void HandleAimUpperMontage();
};
