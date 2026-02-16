// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseSelectedSkill.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTTask_UseSelectedSkill : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_UseSelectedSkill();

protected:
	// 비우기 위함
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector SkillCastRangeKey; // float

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector bInSkillRangeKey;  // bool
	
	// 대상(보통 Blackboard의 TargetActor)
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// SelectRandomSkill에서 저장해둔 슬롯 인덱스
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector SelectedSlotIndexKey;

	// (선택) 실패 시 슬롯을 다시 뽑게 하고 싶으면, 실패 시 SlotIndex를 초기화할지
	UPROPERTY(EditAnywhere, Category="Config")
	bool bClearSlotIndexOnFail = false;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
