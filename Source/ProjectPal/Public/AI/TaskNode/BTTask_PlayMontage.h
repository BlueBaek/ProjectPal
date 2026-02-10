// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayMontage.generated.h"

class UAnimMontage;
class ACharacter;
class UAnimInstance;

UCLASS()
class PROJECTPAL_API UBTTask_PlayMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PlayMontage();

	// 재생할 몽타주(Aggro 몽타주)
	UPROPERTY(EditAnywhere, Category="Montage")
	TObjectPtr<UAnimMontage> MontageToPlay;

	// true면 PalCharacter의 AggroMontage를 우선 사용 
	UPROPERTY(EditAnywhere, Category="Montage")
	bool bUsePalAggroMontage = true;

	UPROPERTY(EditAnywhere, Category="Montage")
	float PlayRate = 1.f;

	// 몽타주 시작 전에 이동을 멈출지(연출용)
	UPROPERTY(EditAnywhere, Category="Montage")
	bool bStopMovementBeforePlay = true;

	// 몽타주가 끊겨도 성공 처리할지(상황에 따라 선택)
	UPROPERTY(EditAnywhere, Category="Montage")
	bool bSucceedOnInterrupted = false;

	// Behavior Tree에서 해당 노드에 도달했을 때 호출됨.
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// BT가 중간에 Abort(중단)될 때 몽타주 정리용(선택이지만 추천)
	// virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	//                             EBTNodeResult::Type TaskResult) override;

private:
	// 몽타주가 끝나면 BT에 알려주기 위해 OwnerComp를 저장
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	// 이번에 실제로 재생한 몽타주(콜백에서 어떤 몽타주가 끝났는지 판별)
	TWeakObjectPtr<UAnimMontage> PlayingMontage;

	// 우리가 바인딩한 AnimInstance(유효성 체크용)
	TWeakObjectPtr<UAnimInstance> CachedAnimInstance;

	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 내부: 어떤 몽타주를 재생할지 결정
	UAnimMontage* ResolveMontageToPlay(ACharacter* Char) const;
};
