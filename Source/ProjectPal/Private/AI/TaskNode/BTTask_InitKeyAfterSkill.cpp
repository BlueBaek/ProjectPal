// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNode/BTTask_InitKeyAfterSkill.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_InitKeyAfterSkill::UBTTask_InitKeyAfterSkill()
{
	NodeName = TEXT("Init Keys After Skill");

	SelectedSlotIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InitKeyAfterSkill, SelectedSlotIndexKey));
	SkillCastRangeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InitKeyAfterSkill, SkillCastRangeKey));
	bInSkillRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InitKeyAfterSkill, bInSkillRangeKey));
	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_InitKeyAfterSkill, DistanceToTargetKey));
}

EBTNodeResult::Type UBTTask_InitKeyAfterSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 유효성 검사
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}
	
	// 여기서 초기화
	if (SelectedSlotIndexKey.SelectedKeyType)
		BB->SetValueAsInt(SelectedSlotIndexKey.SelectedKeyName, INDEX_NONE);

	if (SkillCastRangeKey.SelectedKeyType)
		BB->SetValueAsFloat(SkillCastRangeKey.SelectedKeyName, 0.f);
	
	if (bInSkillRangeKey.SelectedKeyType)
		BB->SetValueAsBool(bInSkillRangeKey.SelectedKeyName, false);
	
	if (DistanceToTargetKey.SelectedKeyType)
		BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.f);
	
	return EBTNodeResult::Succeeded;
}
