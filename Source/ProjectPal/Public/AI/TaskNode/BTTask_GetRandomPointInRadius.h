// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetRandomPointInRadius.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTTask_GetRandomPointInRadius : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_GetRandomPointInRadius();

	// 계산할 랜덤 위치의 기준점. ex) HomeLocation
	UPROPERTY(EditAnywhere, Category="AI")
	struct FBlackboardKeySelector CenterKey;

	// 계산된 랜덤 위치를 저장할 Blackboard 키. ex) MoveLocation
	UPROPERTY(EditAnywhere, Category="AI")
	struct FBlackboardKeySelector ResultKey;

	// CenterKey를 기준으로 탐색할 반경
	UPROPERTY(EditAnywhere, Category="AI")
	float WanderingRadius = 800.f;

protected:
	// Behavior Tree에서 해당 노드에 도달했을 때 호출됨.
	// 반환 값
	// - Succeeded : 위치를 정상적으로 찾음
	// - Failed : 위치를 찾지 못함 (다음 로직으로 분기 가능)
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
