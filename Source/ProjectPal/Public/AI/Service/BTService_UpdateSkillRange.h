// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_UpdateSkillRange.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTService_UpdateSkillRange : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTService_UpdateSkillRange();


protected:
	// BlackboardKeySelector
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector SkillCastRangeKey;   // float

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector DistanceToTargetKey; // float

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector bInSkillRangeKey;    // bool
	

	// 사거리 진입/이탈 여유값(히스테리시스)
	// EnterTol < ExitTol 권장 (한번 들어오면 조금 나가도 바로 false 안 됨)
	UPROPERTY(EditAnywhere, Category="Config")
	float EnterTolerance = 30.f;

	UPROPERTY(EditAnywhere, Category="Config")
	float ExitTolerance  = 80.f;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// 디버그용
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugDraw = false;
};
