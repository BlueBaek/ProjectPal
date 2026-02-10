// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNode/BTTask_PlayMontage.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Character/Pal/PalCharacter.h"
#include "Components/SkeletalMeshComponent.h"

UBTTask_PlayMontage::UBTTask_PlayMontage()
{
	NodeName = TEXT("Play Montage");
	
	// 멤버 변수에 실행 상태를 저장하므로, 인스턴스를 공유하면 꼬일 수 있음
	bCreateNodeInstance = true;

	// OnTaskFinished를 받을 거라 켜두는 게 안전
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* Char = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;
	if (!Char)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontageAndWait] Character is null"));
		return EBTNodeResult::Failed;
	}

	UAnimMontage* Montage = ResolveMontageToPlay(Char);
	if (!Montage)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontageAndWait] No montage resolved"));
		return EBTNodeResult::Failed;
	}

	UAnimInstance* AnimInst = (Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr);
	if (!AnimInst)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontageAndWait] AnimInstance is null"));
		return EBTNodeResult::Failed;
	}

	if (bStopMovementBeforePlay && AICon)
	{
		AICon->StopMovement();
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
		}
	}

	// 이미 재생 중이면 중복 방지(정책: 성공 처리)
	if (AnimInst->Montage_IsPlaying(Montage))
	{
		return EBTNodeResult::Succeeded;
	}

	// BT 완료 콜백을 위해 캐시
	CachedOwnerComp = &OwnerComp;
	PlayingMontage = Montage;
	CachedAnimInstance = AnimInst;

	const float Len = AnimInst->Montage_Play(Montage, PlayRate);
	if (Len <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("[BTTask_PlayMontageAndWait] Montage_Play failed: %s"), *GetNameSafe(Montage));
		// 캐시 해제
		CachedOwnerComp.Reset();
		PlayingMontage.Reset();
		CachedAnimInstance.Reset();
		return EBTNodeResult::Failed;
	}
	
	// 특정 몽타주 종료 델리게이트 바인딩
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UBTTask_PlayMontage::HandleMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDelegate, Montage);
	
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayMontage::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 우리가 재생한 몽타주만 처리
	if (!PlayingMontage.IsValid() || Montage != PlayingMontage.Get())
		return;

	if (UBehaviorTreeComponent* BTC = CachedOwnerComp.Get())
	{
		const bool bSuccess = bSucceedOnInterrupted ? true : !bInterrupted;
		FinishLatentTask(*BTC, bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	}

	// 캐시 정리
	CachedOwnerComp.Reset();
	PlayingMontage.Reset();
	CachedAnimInstance.Reset();
}

UAnimMontage* UBTTask_PlayMontage::ResolveMontageToPlay(class ACharacter* Char) const
{
	if (bUsePalAggroMontage && Char)
	{
		if (APalCharacter* Pal = Cast<APalCharacter>(Char))
		{
			if (UAnimMontage* Aggro = Pal->GetAggroMontage())
			{
				return Aggro;
			}
		}
	}
	return MontageToPlay.Get();
}

