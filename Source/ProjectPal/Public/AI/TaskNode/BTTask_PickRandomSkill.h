// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PickRandomSkill.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTTask_PickRandomSkill : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_PickRandomSkill();
	
protected:
	// 저장할 슬롯 인덱스 (int)
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector SelectedSlotIndexKey;

	// 저장할 스킬 사거리 (float) : SkillDataAsset.Activation.CastRange
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector SkillCastRangeKey;

	// CastRange가 0(제한 없음)인 스킬을 어떻게 처리할지
	UPROPERTY(EditAnywhere, Category="Config")
	bool bFailIfCastRangeIsZero = false;

	// CastRange가 0인데 실패시키지 않을 경우 대체값
	UPROPERTY(EditAnywhere, Category="Config", meta=(EditCondition="!bFailIfCastRangeIsZero", ClampMin="0.0"))
	float FallbackCastRange = 600.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
