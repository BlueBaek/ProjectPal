// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNode/BTTask_GetRandomPointInRadius.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_GetRandomPointInRadius::UBTTask_GetRandomPointInRadius()
{
	// BT 에디터에 표시될 노드 이름을 지정 (이름은 직관적으로 할것) 
	NodeName = TEXT("Get Random Point In Radius");
}

EBTNodeResult::Type UBTTask_GetRandomPointInRadius::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Blackboard 가져오기
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		// Blackboard가 없다면 Failed. 작업 불가
		UE_LOG(LogTemp, Error, TEXT("Blackboard component is nullptr"));
		return EBTNodeResult::Failed;
	}

	// 중심 위치 가져오기
	const FVector CenterLocation = BlackboardComp->GetValueAsVector(CenterKey.SelectedKeyName);

	// Navigation System 가져오기 : NavMesh 기반 경로 탐색/위치 계산 담당
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogTemp, Error, TEXT("NavSys is nullptr"));
		return EBTNodeResult::Failed;
	}

	// 결과로 받을 NavMesh 위치
	FNavLocation RandomNavLocation;

	// NavMesh 위에서 CenterLocation 기준 WanderingRadius 반경 내 실제로 이동 가능한 랜덤 위치 탐색
	const bool bFound = NavSys->GetRandomReachablePointInRadius(CenterLocation,			// 기준 위치
	                                                            WanderingRadius,		// 탐색 반경
	                                                            RandomNavLocation	// 결과
	                                                            );
	if (!bFound)
	{
		// 반경 내 이동 가능 Location이 없다면 Failed.
		UE_LOG(LogTemp, Error, TEXT("No Location to Move"));
		return EBTNodeResult::Failed;
	}
	
	// 결과 위치를 Blackboard에 저장 (ResultKey에 저장)
	BlackboardComp->SetValueAsVector(ResultKey.SelectedKeyName, RandomNavLocation);

	// Task 성공
	return EBTNodeResult::Succeeded;
}
