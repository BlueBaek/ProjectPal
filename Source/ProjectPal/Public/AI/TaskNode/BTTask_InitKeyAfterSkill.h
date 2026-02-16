// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_InitKeyAfterSkill.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTTask_InitKeyAfterSkill : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_InitKeyAfterSkill();
	
protected:
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector SkillCastRangeKey; // float

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector bInSkillRangeKey;  // bool

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector DistanceToTargetKey; // float
	
	// SelectRandomSkill에서 저장해둔 슬롯 인덱스
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector SelectedSlotIndexKey;
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
