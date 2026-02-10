// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetFacingPoint.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTTask_GetFacingPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_GetFacingPoint();

protected:
	// 태스크 실행 시 호출되는 함수
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 태스크가 진행 중일 때 매 프레임 호출 (필요시 회전 보간을 위해 사용)
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	/** 바라볼 타겟이 담긴 블랙보드 키 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** 회전이 완료되었다고 판단할 각도 오차 범위 */
	UPROPERTY(EditAnywhere, Category = "Node")
	float Precision = 10.0f;
};
