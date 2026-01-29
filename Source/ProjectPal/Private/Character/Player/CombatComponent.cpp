// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CombatComponent.h"

#include "Character/Player/PlayerCharacter.h"
#include "DataAsset/WeaponDataAsset.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;	
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
	UE_LOG(LogTemp, Warning, TEXT("Combat Owner: %s"), *GetNameSafe(GetOwner()));
	UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter: %s"), *GetNameSafe(OwnerCharacter));
}

void UCombatComponent::ProcessAttack(bool bPressed)
{
	// bAttackPressed = bPressed;
	//
	// if (bPressed)	StartAttack();
	// else			StopAttack();
	
	if (!bPressed)
	{
		// 버튼 뗐을 때는 상태를 풀지 않음.
		// 상태 해제는 몽타주 끝 타이밍에서만 처리.
		return;
	}

	StartAttack();
}

void UCombatComponent::StartAttack()
{
	if (!OwnerCharacter) return;

	UAnimMontage* Montage = GetCurrentAttackMontage();
	if (!Montage) return;

	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;

	// 이미 재생 중이면 아무것도 하지 않음
	if (AnimInst->Montage_IsPlaying(Montage)) return;

	CurrentAttackMontage = Montage;

	// 1) 먼저 재생 시도
	const float PlayResult = AnimInst->Montage_Play(Montage);
	UE_LOG(LogTemp, Warning, TEXT("Montage_Play result=%.3f montage=%s"),
		PlayResult, *GetNameSafe(Montage));

	// PlayResult가 0이면 재생 실패
	if (PlayResult <= 0.f)
	{
		CurrentAttackMontage = nullptr;
		return;
	}

	// 2) 재생 성공 후 델리게이트 연결
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &UCombatComponent::OnAttackMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, Montage);

	// ✅ 3) 상태 ON (재생이 확정된 뒤에만)
	OwnerCharacter->Attack(true);
}

// 몽타주 재생시간을 기준으로 Stop하는 방식으로 변경
// void UCombatComponent::StopAttack()
// {
// 	if (!OwnerCharacter) return;
//
// 	// 캐릭터 측(이동속도 복구 등)
// 	OwnerCharacter->Attack(false);
//
// 	// 지금은 "버튼 뗐다고 몽타주를 강제로 끊지" 않는 쪽이 자연스러움
// 	// (차후 차지/사격/콤보 등에서 분기 가능)
// 	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("StopAttack"));
// }

UAnimMontage* UCombatComponent::GetCurrentAttackMontage() const
{
	// 우선은 무기 없을 때만
	return UnarmedData ? UnarmedData->AttackMontage : nullptr;
}

void UCombatComponent::OnAttackMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// 디버그용
	// UE_LOG(LogTemp, Warning, TEXT("Attack BlendOut: %s Interrupted=%d"),
	// *GetNameSafe(Montage), bInterrupted ? 1 : 0);
	//
	// 내가 관리하는 공격 몽타주가 아닐 수도 있으니 가드
	if (!OwnerCharacter) return;
	if (!CurrentAttackMontage) return;
	if (Montage != CurrentAttackMontage) return;

	// 공격 종료: 상태 OFF
	OwnerCharacter->Attack(false);
	
	CurrentAttackMontage = nullptr;
}


