// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PalAnimInstance.generated.h"

class APawn;
class ACharacter;
class UCharacterMovementComponent;

UCLASS()
class PROJECTPAL_API UPalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPalAnimInstance();
	
protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 캐시
	UPROPERTY(Transient)
	APawn* OwningPawn = nullptr;

	UPROPERTY(Transient)
	ACharacter* OwningCharacter = nullptr;

	UPROPERTY(Transient)
	UCharacterMovementComponent* MoveComponent = nullptr;
	
	// 스킬 사용을 위함. Notify 이름은 SkillNotify
	UFUNCTION()
	void AnimNotify_SkillFire();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;	// 속도
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	// float Direction;	// 방향
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Acceleration;	// 가속
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;		// 움직임 여부 검사

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;	// 공중 검사
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsRising;		// 올라가는 중인지 검사
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bHasAcceleration;
};
