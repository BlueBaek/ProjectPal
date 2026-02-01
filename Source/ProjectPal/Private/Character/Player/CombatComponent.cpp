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

/*
void UCombatComponent::ProcessAttack(bool bPressed)
{
	bAttackPressed = bPressed;
	
	if (!bPressed)
	{
		StartAttack();
	}
}
*/

// 콤보어택 기반으로 수정
void UCombatComponent::ProcessAttack(bool bPressed)
{
	bAttackPressed = bPressed;
	UE_LOG(LogTemp, Warning, TEXT("bAttackPressed = %s"),bAttackPressed ? TEXT("True") : TEXT("False"));
	// 버튼을 뗐을 때: 현재 재생 중인 몽타주는 끝까지 가게 두고,
	// 다음 콤보가 자동으로 이어지지 않도록만 한다.
	if (!bPressed)
	{
		return;
	}

	// 눌렀을 때: 콤보 리셋 시간 넘었으면 1타부터
	if (GetWorld())
	{
		const float Now = GetWorld()->GetTimeSeconds();
		if ((Now - LastAttackTime) > ComboResetDelay)
		{
			ComboIndex = 0;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("StartAttack"));
	StartAttack();
}


/*
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
*/

void UCombatComponent::StartAttack()
{
	if (!OwnerCharacter) return;
	UE_LOG(LogTemp, Warning, TEXT("OwnCharacter: %s"), *OwnerCharacter->GetName());
	
	UAnimMontage* Montage = GetCurrentAttackMontage();
	UE_LOG(LogTemp, Warning, TEXT("Montage=%s"), *GetNameSafe(Montage));
	
	if (!Montage) return;

	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;

	// 이미 재생 중이면(아직 1타/2타가 끝나기 전이면) 새로 시작하지 않음
	if (AnimInst->Montage_IsPlaying(Montage)) return;

	
	// 추가 : 콤보 섹션 안전 처리
	if (ComboSections.Num() <= 0)
	{
		ComboIndex = 0;
	}
	else
	{
		ComboIndex = FMath::Clamp(ComboIndex, 0, ComboSections.Num() - 1);
	}
	
	CurrentAttackMontage = Montage;

	// 1) 재생 시도
	const float PlayResult = AnimInst->Montage_Play(Montage);
	
	// 디버그용
	
	
	// PlayResult가 0이면 재생 실패
	if (PlayResult <= 0.f)
	{
		CurrentAttackMontage = nullptr;
		return;
	}
	
	// 추가 : 해당 콤보 섹션으로 시작
	if (ComboSections.IsValidIndex(ComboIndex))
	{
		const FName SectionName = ComboSections[ComboIndex];
		if (IsComboSectionValid(Montage, SectionName))
		{
			AnimInst->Montage_JumpToSection(SectionName, Montage);
		}
	}
	
	// 2) 재생 성공 후 델리게이트 연결
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &UCombatComponent::OnAttackMontageBlendingOut);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, Montage);

	// 3) 상태 ON (재생이 확정된 뒤에만)
	OwnerCharacter->Attack(true);
	
	// 추가 : 마지막 공격 시간 갱신
	if (GetWorld())
	{
		LastAttackTime = GetWorld()->GetTimeSeconds();
	}
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
	
	// 내가 관리하는 공격 몽타주가 아닐 수도 있으니 조건 추가
	if (!OwnerCharacter) return;
	if (!CurrentAttackMontage) return;
	if (Montage != CurrentAttackMontage) return;

	// 공격 종료: 상태 OFF
	OwnerCharacter->Attack(false);
	
	CurrentAttackMontage = nullptr;
	
	/*	콤보 어택으로 교체
	// 마우스가 아직 눌려있다면 다음 공격 이어서 실행
	if (bAttackPressed && !bInterrupted)
	{
		StartAttack();
	}
	*/
	
	// 추가 : 홀드 중이면 다음 콤보로 진행
	if (bAttackPressed)
	{
		// 다음 타로 증가 (1->2->3)
		if (ComboSections.Num() > 0)
		{
			ComboIndex = (ComboIndex + 1) % ComboSections.Num();
		}
		else
		{
			ComboIndex = 0;
		}

		StartAttack();
	}
	else
	{
		// 버튼 뗐으면 콤보 리셋(다음 공격은 1타부터)
		ComboIndex = 0;
	}
}


bool UCombatComponent::IsComboSectionValid(UAnimMontage* Montage, const FName& SectionName) const
{
	if (!Montage) return false;
	if (SectionName.IsNone()) return false;

	// 섹션이 없으면 INDEX_NONE
	const int32 SectionIndex = Montage->GetSectionIndex(SectionName);
	return SectionIndex != INDEX_NONE;
}


