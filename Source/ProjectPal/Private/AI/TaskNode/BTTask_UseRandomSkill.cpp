// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNode/BTTask_UseRandomSkill.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Pal/PalCharacter.h"
#include "Component/PalSkillComponent.h"

UBTTask_UseRandomSkill::UBTTask_UseRandomSkill()
{
	NodeName = TEXT("Use Random Skill");

	// Blackboard 키 선택 허용 타입(Actor)
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_UseRandomSkill, TargetActorKey), AActor::StaticClass());

	// UsedSlotIndex는 int
	UsedSlotIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_UseRandomSkill, UsedSlotIndexKey));
}

EBTNodeResult::Type UBTTask_UseRandomSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AICon->GetPawn();
	APalCharacter* Pal = Cast<APalCharacter>(Pawn);
	if (!Pal)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	// (선택) 사거리 체크
	if (bCheckRange && !IsInRange(Pal, TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	UPalSkillComponent* SkillComp = Pal->FindComponentByClass<UPalSkillComponent>();
	if (!SkillComp)
	{
		return EBTNodeResult::Failed;
	}

	int32 UsedSlotIndex = INDEX_NONE;
	const bool bStarted = SkillComp->TryUseRandomActiveSkill(TargetActor, UsedSlotIndex);

	if (!bStarted)
	{
		// 지금 쓸 수 있는 스킬이 없거나(쿨타임), 실행 시작 실패
		return EBTNodeResult::Failed;
	}

	// (선택) 디버그로 "이번에 어떤 슬롯 사용했는지" BB에 기록
	if (bWriteUsedSlotToBlackboard && UsedSlotIndexKey.SelectedKeyType)
	{
		BB->SetValueAsInt(UsedSlotIndexKey.SelectedKeyName, UsedSlotIndex);
	}

	// 이 Task는 "시전 시작"만 성공으로 보고 끝냄.
	// 실제 발사는 Start 몽타주의 SkillFire Notify에서 일어나며,
	// 스킬 종료(지속 끝)는 Execution::Finish에서 처리됨.
	return EBTNodeResult::Succeeded;
}

bool UBTTask_UseRandomSkill::IsInRange(const AActor* SelfActor, const AActor* TargetActor) const
{
	if (!SelfActor || !TargetActor) return false;

	const float Dist = FVector::Dist(SelfActor->GetActorLocation(), TargetActor->GetActorLocation());

	// MinRange = 0 이면 하한 없음
	if (MinRange > 0.f && Dist < MinRange) return false;

	// MaxRange = 0 이면 상한 없음으로 볼 수도 있지만 여기선 "0이면 항상 실패"가 되니 보통 MaxRange는 >0로 씀
	if (MaxRange > 0.f && Dist > MaxRange) return false;

	return true;
}
