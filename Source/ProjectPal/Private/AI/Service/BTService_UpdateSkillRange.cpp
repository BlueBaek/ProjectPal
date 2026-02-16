// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_UpdateSkillRange.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateSkillRange::UBTService_UpdateSkillRange()
{
	NodeName = TEXT("Update Distance Between Target");

	SkillCastRangeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateSkillRange, SkillCastRangeKey));
	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateSkillRange, DistanceToTargetKey));
	bInSkillRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateSkillRange, bInSkillRangeKey));

	Interval = 0.1f;
	RandomDeviation = 0.02f;
}


void UBTService_UpdateSkillRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (SkillCastRangeKey.SelectedKeyName.IsNone() ||
		DistanceToTargetKey.SelectedKeyName.IsNone() ||
		bInSkillRangeKey.SelectedKeyName.IsNone())
	{
		return;
	}

	if (BlackboardKey.SelectedKeyName.IsNone())
	{
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!BB || !AICon)
	{
		return;
	}

	APawn* Pawn = AICon->GetPawn();
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(BlackboardKey.SelectedKeyName));

	if (!Pawn || !Target || Target == Pawn)
	{
		BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.f);
		BB->SetValueAsBool(bInSkillRangeKey.SelectedKeyName, false);
		return;
	}

	const float CastRange = BB->GetValueAsFloat(SkillCastRangeKey.SelectedKeyName);
	const float Dist = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());

	BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Dist);

	bool bWasInRange = BB->GetValueAsBool(bInSkillRangeKey.SelectedKeyName);
	bool bNowInRange = bWasInRange
		                   ? !(Dist >= (CastRange + ExitTolerance))
		                   : (Dist <= (CastRange + EnterTolerance));

	BB->SetValueAsBool(bInSkillRangeKey.SelectedKeyName, (CastRange > 0.f) && bNowInRange);
}
