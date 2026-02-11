// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseRandomSkill.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTTask_UseRandomSkill : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_UseRandomSkill();

protected:
	// 대상(보통 Blackboard의 TargetActor)
	UPROPERTY(EditAnywhere, Category="Blackboard")
	struct FBlackboardKeySelector TargetActorKey;

	// 사거리 체크 옵션
	UPROPERTY(EditAnywhere, Category="Range")
	bool bCheckRange = false;

	UPROPERTY(EditAnywhere, Category="Range", meta=(EditCondition="bCheckRange", ClampMin="0.0"))
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, Category="Range", meta=(EditCondition="bCheckRange", ClampMin="0.0"))
	float MaxRange = 500.f;

	// 디버그용: 어떤 슬롯을 썼는지 BB에 기록하고 싶으면 설정
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bWriteUsedSlotToBlackboard = false;

	UPROPERTY(EditAnywhere, Category="Debug", meta=(EditCondition="bWriteUsedSlotToBlackboard"))
	FBlackboardKeySelector UsedSlotIndexKey;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	bool IsInRange(const AActor* SelfActor, const AActor* TargetActor) const;
};
