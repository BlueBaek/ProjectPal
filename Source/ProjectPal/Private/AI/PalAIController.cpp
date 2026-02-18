// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/PalAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Pal/PalCharacter.h"
#include "GameFramework/FactionFunctionLibrary.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName APalAIController::KEY_PalMoveState(TEXT("PalMoveState"));
const FName APalAIController::KEY_TargetActor(TEXT("TargetActor"));
const FName APalAIController::KEY_HomeLocation(TEXT("HomeLocation"));
const FName APalAIController::KEY_bDidAggro(TEXT("bDidAggro"));
const FName APalAIController::KEY_OwnerActor(TEXT("OwnerActor"));

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

	if (!InPawn) return;

	APalCharacter* Pal = Cast<APalCharacter>(InPawn);
	const bool bOwned = (Pal && Pal->GetPalGroup() == EPalGroup::Tamed);

	UBehaviorTree* TreeToRun = (bOwned && OwnedBT) ? OwnedBT : BT;
	if (!TreeToRun)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PalAI] No BehaviorTree assigned (Owned=%d)"), bOwned);
		return;
	}

	UBlackboardComponent* BB = nullptr;
	if (!TreeToRun->BlackboardAsset || !UseBlackboard(TreeToRun->BlackboardAsset, BB) || !BB)
	{
		UE_LOG(LogTemp, Error, TEXT("[PalAI] Blackboard init failed"));
		return;
	}

	RunBehaviorTree(TreeToRun);
	
	// ✅ 소유 팰이면 OwnerActor BB 세팅
	// if (bOwned && Pal)
	// {
	// 	AActor* PalOwner = Pal->GetOwner();
	// 	if (PalOwner)
	// 	{
	// 		BB->SetValueAsObject(KEY_OwnerActor, PalOwner);
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[PalAI] Tamed Pal has no Owner (SetOwner at spawn)!"));
	// 	}
	// }

	// ✅ Follow용 주인 세팅: 엔진 Owner 말고 MasterActor 사용
	if (bOwned && Pal)
	{
		AActor* Master = Pal->GetMasterActor();
		BB->SetValueAsObject(KEY_OwnerActor, Master);

		UE_LOG(LogTemp, Warning, TEXT("[OwnedFollow] Master=%s"), *GetNameSafe(Master));
	}

	
	BB->SetValueAsVector(KEY_HomeLocation, InPawn->GetActorLocation());
	BB->SetValueAsObject(KEY_TargetActor, nullptr);

	const EPalMoveState StartState = bOwned ? EPalMoveState::Following : EPalMoveState::Wandering;
	BB->SetValueAsEnum(KEY_PalMoveState, (uint8)StartState);
	ApplyMoveStateToPawn(StartState);

	const FBlackboard::FKey KeyID = BB->GetKeyID(KEY_PalMoveState);
	if (KeyID != FBlackboard::InvalidKey)
	{
		BB->RegisterObserver(
			KeyID,
			this,
			FOnBlackboardChangeNotification::CreateUObject(this, &APalAIController::OnMoveStateChanged)
		);
	}
}

void APalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;
	
	// 자기 자신은 무시
	if (Actor == GetPawn()) return;
	
	// Stimulus.WasSuccessfullySensed() == true  => 시야로 "감지됨"
	// false => 감지 "해제"(시야에서 사라짐/기억 만료 등)
	if (Stimulus.WasSuccessfullySensed())
	{
		// 적대 진영이 아니면 무시
		APawn* MyPawn = GetPawn();
		if (!MyPawn) return;

		// ✅ 핵심: 다른 진영(적대)만 타겟으로 인정
		if (!UFactionFunctionLibrary::AreHostile(MyPawn, Actor))
		{
			return; // 같은 진영/진영판정불가(인터페이스 없음) => 무시
		}
		
		// 새 타겟인지 확인
		AActor* Current = GetTargetActor();
		if (Current != Actor)
		{
			SetTargetActor(Actor);
			UE_LOG(LogTemp, Warning, TEXT("PalAICon : Target set: %s"), *GetNameSafe(Actor));
			
			if (UBlackboardComponent* BB = GetBlackboardComponent())
			{
				BB->SetValueAsBool(KEY_bDidAggro, false);
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
				BB->SetValueAsBool(KEY_bDidAggro, false);
			}
		}
	}
}

void APalAIController::SetTargetActor(AActor* NewTarget)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(KEY_TargetActor, NewTarget);
	}
}

AActor* APalAIController::GetTargetActor() const
{
	if (const UBlackboardComponent* BB = GetBlackboardComponent())
	{
		return Cast<AActor>(BB->GetValueAsObject(KEY_TargetActor));
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
