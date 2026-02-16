// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNode/BTTask_PickRandomSkill.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Pal/PalCharacter.h"
#include "Component/PalSkillComponent.h"
#include "DataAsset/PalSkillDataAsset.h"

UBTTask_PickRandomSkill::UBTTask_PickRandomSkill()
{
	NodeName = TEXT("Pick Random Skill");

	SelectedSlotIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PickRandomSkill, SelectedSlotIndexKey));
	SkillCastRangeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PickRandomSkill, SkillCastRangeKey));
}

EBTNodeResult::Type UBTTask_PickRandomSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] No AIController"));
		return EBTNodeResult::Failed;
	}

	APalCharacter* Pal = Cast<APalCharacter>(AICon->GetPawn());
	if (!Pal)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] Pawn is not PalCharacter"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] No Blackboard"));
		return EBTNodeResult::Failed;
	}

	UPalSkillComponent* SkillComp = Pal->FindComponentByClass<UPalSkillComponent>();
	if (!SkillComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] No PalSkillComponent"));
		return EBTNodeResult::Failed;
	}

	// 키 바인딩 체크 (여기서 자주 터짐)
	if (SelectedSlotIndexKey.SelectedKeyName.IsNone() || SkillCastRangeKey.SelectedKeyName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] Blackboard keys not assigned. SlotKey=%s RangeKey=%s"),
			*SelectedSlotIndexKey.SelectedKeyName.ToString(),
			*SkillCastRangeKey.SelectedKeyName.ToString());
		return EBTNodeResult::Failed;
	}
	
	// 1) 쓸 수 있는 Active 슬롯 중 랜덤 선택
	const int32 SlotIndex = SkillComp->PickRandomUsableActiveSlot();
	if (SlotIndex == INDEX_NONE)
	{
		return EBTNodeResult::Failed;
	}

	// 2) 슬롯에서 스킬 DataAsset을 얻고 사거리 읽기
	UPalSkillDataAsset* SkillDA = SkillComp->GetSkillAt(SlotIndex);
	if (!SkillDA)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] SkillDA is null at slot %d"), SlotIndex);
		return EBTNodeResult::Failed;
	}

	float CastRange = FMath::Max(0.f, SkillDA->Activation.CastRange);
	const float RequiredMoveDist = FMath::Max(0.f, SkillDA->Activation.RequiredMoveDistanceToCast);

	// CastRange = 0이면 “제한 없음”인데, 이동 단계에서는 의미가 없을 수 있음
	if (CastRange <= 0.f)
	{
		if (bFailIfCastRangeIsZero)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] CastRange=0 and bFailIfCastRangeIsZero=true (slot %d)"), SlotIndex);
			return EBTNodeResult::Failed;
		}

		CastRange = FMath::Max(0.f, FallbackCastRange);
	}
	
	// 3) BB에 저장
	// if (!SelectedSlotIndexKey.SelectedKeyType || !SkillCastRangeKey.SelectedKeyType)
	// {
	// 	return EBTNodeResult::Failed;
	// }
	
	BB->SetValueAsInt(SelectedSlotIndexKey.SelectedKeyName, SlotIndex);
	BB->SetValueAsFloat(SkillCastRangeKey.SelectedKeyName, CastRange);
	
	UE_LOG(LogTemp, Warning, TEXT("[PickRandomSkill] Picked slot=%d CastRange=%.1f"), SlotIndex, CastRange);
	return EBTNodeResult::Succeeded;
}
