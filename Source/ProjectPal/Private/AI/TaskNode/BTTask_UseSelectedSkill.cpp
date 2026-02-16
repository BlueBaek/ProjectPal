// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNode/BTTask_UseSelectedSkill.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Pal/PalCharacter.h"
#include "Component/PalSkillComponent.h"

UBTTask_UseSelectedSkill::UBTTask_UseSelectedSkill()
{
	NodeName = TEXT("Use Selected Skill");

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_UseSelectedSkill, TargetActorKey), AActor::StaticClass());
	SelectedSlotIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_UseSelectedSkill, SelectedSlotIndexKey));
}

EBTNodeResult::Type UBTTask_UseSelectedSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 유효성 검사
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	APalCharacter* Pal = Cast<APalCharacter>(AICon->GetPawn());
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
		return EBTNodeResult::Failed;

	UPalSkillComponent* SkillComp = Pal->FindComponentByClass<UPalSkillComponent>();
	if (!SkillComp)
	{
		return EBTNodeResult::Failed;
	}
	
	const int32 SlotIndex = BB->GetValueAsInt(SelectedSlotIndexKey.SelectedKeyName);
	if (SlotIndex == INDEX_NONE)
	{
		return EBTNodeResult::Failed;
	}
	
	
	// 슬롯 유효성/쿨 체크는 컴포넌트가 담당하는 구조라
	// 여기서는 "선택 후 사용 시도"만 한다.
	if (!SkillComp->SelectActiveSlot(SlotIndex))
	{
		if (bClearSlotIndexOnFail)
			BB->SetValueAsInt(SelectedSlotIndexKey.SelectedKeyName, INDEX_NONE);

		return EBTNodeResult::Failed;
	}

	const bool bStarted = SkillComp->TryUseSelectedSkill(TargetActor);
	if (!bStarted)
	{
		if (bClearSlotIndexOnFail)
			BB->SetValueAsInt(SelectedSlotIndexKey.SelectedKeyName, INDEX_NONE);

		return EBTNodeResult::Failed;
	}

	// RandomSkill Task와 동일: "시전 시작" 성공이면 종료.
	// 실제 발사는 Start 몽타주의 SkillFire Notify에서 진행.
	UE_LOG(LogTemp, Warning, TEXT("[UseSelectedSkill] Task Succeeded!"));
	
	// ✅ 여기서 초기화
	BB->SetValueAsInt(SelectedSlotIndexKey.SelectedKeyName, INDEX_NONE);

	if (SkillCastRangeKey.SelectedKeyType)
		BB->SetValueAsFloat(SkillCastRangeKey.SelectedKeyName, 0.f);

	if (bInSkillRangeKey.SelectedKeyType)
		BB->SetValueAsBool(bInSkillRangeKey.SelectedKeyName, false);
	
	return EBTNodeResult::Succeeded;
}
