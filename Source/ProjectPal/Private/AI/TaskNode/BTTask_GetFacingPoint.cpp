// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNode/BTTask_GetFacingPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_GetFacingPoint::UBTTask_GetFacingPoint()
{
	NodeName = TEXT("Face Target");
	// Tick 기능을 활성화하여 부드러운 회전을 구현할 수 있게 합니다.
	bNotifyTick = true;

	// 블랙보드에서 'Object' 또는 'Vector' 타입만 선택할 수 있게 필터링
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_GetFacingPoint, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_GetFacingPoint, TargetKey));
}

EBTNodeResult::Type UBTTask_GetFacingPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress; // 성공할 때까지 계속 실행
}

void UBTTask_GetFacingPoint::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	APawn* ControlledPawn = AIController->GetPawn();

	// 1. 타겟 위치 가져오기 (액터 혹은 좌표)
	FVector TargetLocation = FVector::ZeroVector;
	AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TargetKey.SelectedKeyName));
	
	if (TargetActor)
	{
		TargetLocation = TargetActor->GetActorLocation();
	}
	else
	{
		TargetLocation = BBComp->GetValueAsVector(TargetKey.SelectedKeyName);
	}

	// if (TargetLocation.IsZero())
	// {
	// 	FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	// 	return;
	// }

	// 2. 방향 계산 및 회전값 도출
	FVector CurrentLocation = ControlledPawn->GetActorLocation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
	
	// 캐릭터가 위아래로 꺾이지 않게 Yaw값만 사용
	FRotator TargetRotation = FRotator(0.f, LookAtRotation.Yaw, 0.f);

	// 3. 부드러운 회전 적용 (RInterpTo)
	// CharacterMovement의 RotationRate를 참고하거나 고정된 속도를 사용합니다.
	FRotator NewRotation = FMath::RInterpTo(ControlledPawn->GetActorRotation(), TargetRotation, DeltaSeconds, 5.0f);
	ControlledPawn->SetActorRotation(NewRotation);

	// 4. 완료 조건 체크
	// float AngleRemaining = FMath::Abs(NewRotation.Yaw - TargetRotation.Yaw);
	float AngleRemaining = FMath::Abs(FMath::FindDeltaAngleDegrees(NewRotation.Yaw, TargetRotation.Yaw));
	if (AngleRemaining <= Precision)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}