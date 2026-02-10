// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/PalAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Pal/PalCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName APalAIController::KEY_PalMoveState(TEXT("PalMoveState"));

APalAIController::APalAIController()
{
	// AI Perception
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp")); 
	SetPerceptionComponent(*PerceptionComp);
	
	// AISenseConfig_Sight
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1800.f;	// 시야 거리
	SightConfig->LoseSightRadius = 2200.f;	// 감지 유지 거리. 실제로는 팰이 사라질 때 까지 Target을 잃지 않는 듯 하다...
	SightConfig->PeripheralVisionAngleDegrees = 60.f;	// 시야 각(90도하면 양쪽해서 180도)
	SightConfig->SetMaxAge(5.f);	// 대상 기억(초)
	// 마지막으로 목격된 지점 주변 어디까지를 여전히 발견한 상태로 간주할 것인가
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.f; 
	
	// 감지 진영 설정 : 현재는 모든 진영 감시
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	// 컴포넌트에 감각 추가
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	// ✅ 이벤트 바인딩
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &APalAIController::OnTargetPerceptionUpdated);
}

void APalAIController::BeginPlay()
{
	Super::BeginPlay();
}

void APalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (!BT || !InPawn) return;
	
	UBlackboardComponent* BB = nullptr;
	if (!BT->BlackboardAsset || !UseBlackboard(BT->BlackboardAsset, BB) || !BB)
	{
		UE_LOG(LogTemp, Error, TEXT("[PalAI] Blackboard init failed"));
		return;
	}
	RunBehaviorTree(BT);
	
	// HomeLocation을 현재 팰 위치(스폰 위치)로 설정
	BB->SetValueAsVector(TEXT("HomeLocation"), InPawn->GetActorLocation());
	// Target을 비움
	BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
	// PalMoveState 초기값 세팅(예: 배회로 시작)
	BB->SetValueAsEnum(KEY_PalMoveState, (uint8)EPalMoveState::Wandering);
	ApplyMoveStateToPawn(EPalMoveState::Wandering);
	
	// PalMoveState 키 변화 감지 등록(이게 핵심)
	const FBlackboard::FKey KeyID = BB->GetKeyID(KEY_PalMoveState);
	if (KeyID != FBlackboard::InvalidKey)
	{
		BB->RegisterObserver(
			KeyID,
			this,
			FOnBlackboardChangeNotification::CreateUObject(this, &APalAIController::OnMoveStateChanged)
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PalAI] Blackboard has no key named MoveState"));
	}
}

void APalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;
	
	// Stimulus.WasSuccessfullySensed() == true  => 시야로 "감지됨"
	// false => 감지 "해제"(시야에서 사라짐/기억 만료 등)
	if (Stimulus.WasSuccessfullySensed())
	{
		// 새 타겟인지 확인
		AActor* Current = GetTargetActor();
		if (Current != Actor)
		{
			SetTargetActor(Actor);
			UE_LOG(LogTemp, Warning, TEXT("PalAICon : Target set: %s"), *GetNameSafe(Actor));
			if (UBlackboardComponent* BB = GetBlackboardComponent())
			{
				BB->SetValueAsBool(TEXT("bDidAggro"), false);
			}
		}
	}
	else
	{
		// 현재 Target이 Actor였을 때만 해제
		if (GetTargetActor() == Actor)
		{
			SetTargetActor(nullptr);
			UE_LOG(LogTemp, Warning, TEXT("PalAICon : Target cleared"));
			
			// 타겟 잃었을 때 현재 타겟이면 해제 + 플래그 리셋
			if (UBlackboardComponent* BB = GetBlackboardComponent())
			{
				BB->SetValueAsBool(TEXT("bDidAggro"), false);
			}
		}
	}
}

void APalAIController::SetTargetActor(AActor* NewTarget)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(TEXT("TargetActor"), NewTarget);
	}
}

AActor* APalAIController::GetTargetActor() const
{
	if (const UBlackboardComponent* BB = GetBlackboardComponent())
	{
		return Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	}
	return nullptr;
}

EBlackboardNotificationResult APalAIController::OnMoveStateChanged(const UBlackboardComponent& BBComp,
	FBlackboard::FKey ChangedKeyID)
{
	const uint8 Raw = BBComp.GetValueAsEnum(KEY_PalMoveState);
	const EPalMoveState NewState = static_cast<EPalMoveState>(Raw);

	ApplyMoveStateToPawn(NewState);

	// 계속 관찰
	return EBlackboardNotificationResult::ContinueObserving;
}

void APalAIController::ApplyMoveStateToPawn(EPalMoveState NewState) const
{
	APalCharacter* Pal = Cast<APalCharacter>(GetPawn());
	if (!Pal) return;

	Pal->SetMoveState(NewState); // 여기서 네가 만든 속도 적용 로직이 실행됨
}
