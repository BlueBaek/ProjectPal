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
}

void UCombatComponent::ProcessAttack(bool bPressed)
{
	bAttackPressed = bPressed;

	if (bPressed)	StartAttack();
	else			StopAttack();
}

void UCombatComponent::StartAttack()
{
	if (!OwnerCharacter) return;

	// 캐릭터 측(이동속도 변경 등)
	OwnerCharacter->Attack(true);

	UAnimMontage* Montage = GetCurrentAttackMontage();
	if (!Montage) return;

	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;

	// 이미 재생 중이면 중복 재생 방지
	if (AnimInst->Montage_IsPlaying(Montage)) return;

	AnimInst->Montage_Play(Montage);
}

void UCombatComponent::StopAttack()
{
	if (!OwnerCharacter) return;

	// 캐릭터 측(이동속도 복구 등)
	OwnerCharacter->Attack(false);

	// 지금은 "버튼 뗐다고 몽타주를 강제로 끊지" 않는 쪽이 자연스러움
	// (차후 차지/사격/콤보 등에서 분기 가능)
}

UAnimMontage* UCombatComponent::GetCurrentAttackMontage() const
{
	// 우선은 무기 없을 때만
	return UnarmedData ? UnarmedData->AttackMontage : nullptr;
}


