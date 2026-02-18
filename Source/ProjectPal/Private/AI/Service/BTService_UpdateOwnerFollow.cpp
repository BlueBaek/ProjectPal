// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_UpdateOwnerFollow.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateOwnerFollow::UBTService_UpdateOwnerFollow()
{
	NodeName = TEXT("Update Distance To Owner");

	DistanceToOwnerKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateOwnerFollow, DistanceToOwnerKey));
	bShouldFollowKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateOwnerFollow, bShouldFollowKey));
	OwnerLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateOwnerFollow, OwnerLocationKey));

	Interval = 0.1f;
	RandomDeviation = 0.02f;
}

void UBTService_UpdateOwnerFollow::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!BB || !AICon)
	{
		return;
	}

	APawn* Pawn = AICon->GetPawn();

	// 서비스 기본 BlackboardKey는 "OwnerActor"를 바라봐야 함
	if (BlackboardKey.SelectedKeyName.IsNone())
	{
		// 키가 바인딩이 안 된 상태면 MoveTo(Vector)도 못하므로, 조용히 빠지지 말고 로그로 알려줌
		if (bDebugLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UpdateOwnerFollow] Service BlackboardKey is NONE. Bind it to OwnerActor."));
		}
		return;
	}

	AActor* OwnerActor = Cast<AActor>(BB->GetValueAsObject(BlackboardKey.SelectedKeyName));

	// OwnerActor가 없으면 출력도 초기화
	if (!OwnerActor)
	{
		SetOutputsSafe(BB, 0.f, false, FVector::ZeroVector);

		if (bDebugLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UpdateOwnerFollow] OwnerActor is NULL (ServiceKey=%s)"),
				*BlackboardKey.SelectedKeyName.ToString());
		}
		return;
	}

	// ★ 핵심: OwnerLocation은 OwnerActor가 존재하는 한 무조건 세팅
	const FVector OwnerLoc = OwnerActor->GetActorLocation();

	// Pawn이 없더라도 위치는 갱신 가능
	if (!Pawn)
	{
		SetOutputsSafe(BB, 0.f, false, OwnerLoc);

		if (bDebugLog)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UpdateOwnerFollow] Pawn is NULL. Owner=%s Loc=%s"),
				*GetNameSafe(OwnerActor), *OwnerLoc.ToString());
		}
		return;
	}

	const FVector PawnLoc = Pawn->GetActorLocation();
	const float Dist = FVector::Dist(PawnLoc, OwnerLoc);

	// bShouldFollow 히스테리시스
	bool bNowFollow = false;
	if (!bShouldFollowKey.SelectedKeyName.IsNone())
	{
		const bool bWasFollow = BB->GetValueAsBool(bShouldFollowKey.SelectedKeyName);

		// Start > Stop 권장
		if (bWasFollow)
		{
			bNowFollow = !(Dist <= FollowStopDistance);
		}
		else
		{
			bNowFollow = (Dist >= FollowStartDistance);
		}
	}

	SetOutputsSafe(BB, Dist, bNowFollow, OwnerLoc);

	if (bDebugLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[UpdateOwnerFollow] ServiceKey=%s Owner=%s Dist=%.1f Follow=%s OwnerLoc=%s"),
			*BlackboardKey.SelectedKeyName.ToString(),
			*GetNameSafe(OwnerActor),
			Dist,
			bNowFollow ? TEXT("true") : TEXT("false"),
			*OwnerLoc.ToString());
	}
}

void UBTService_UpdateOwnerFollow::SetOutputsSafe(class UBlackboardComponent* BB, float Distance, bool bFollow,
	const FVector& OwnerLoc) const
{
	if (!BB) return;

	if (!DistanceToOwnerKey.SelectedKeyName.IsNone())
	{
		BB->SetValueAsFloat(DistanceToOwnerKey.SelectedKeyName, Distance);
	}

	if (!bShouldFollowKey.SelectedKeyName.IsNone())
	{
		BB->SetValueAsBool(bShouldFollowKey.SelectedKeyName, bFollow);
	}

	// ★ MoveTo(OwnerLocation)을 쓰는 경우, 이 값은 "항상" 유효해야 한다.
	if (!OwnerLocationKey.SelectedKeyName.IsNone())
	{
		BB->SetValueAsVector(OwnerLocationKey.SelectedKeyName, OwnerLoc);
	}
}
