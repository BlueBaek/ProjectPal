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

EWeaponType UCombatComponent::GetCurrentWeaponType() const
{
	// 무기 장착 시스템이 이미 있으면 CurrentWeaponData 사용
	if (CurrentWeaponData)
	{
		return CurrentWeaponData->WeaponType;
	}

	// 무기 없으면 맨손
	if (UnarmedData)
	{
		return UnarmedData->WeaponType;
	}

	return EWeaponType::Unarmed;
}

/*
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
*/

// CurrentWeaponData에 따라 무기 공격 타입 설정
void UCombatComponent::ProcessAttack(bool bPressed)
{
	switch (GetCurrentWeaponType())
	{
	case EWeaponType::Unarmed:
		HandleUnarmedAttack(bPressed);
		break;

	case EWeaponType::Sword:
		HandleSwordAttack(bPressed);
		break;

	case EWeaponType::AssaultRifle:
		HandleAssaultRifleAttack(bPressed);
		break;

	default:
		break;
	}
}

bool UCombatComponent::IsAttackMontagePlaying() const
{
	if (!OwnerCharacter) return false;

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp) return false;

	UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
	if (!AnimInst) return false;

	// 현재 공격 몽타주가 유효하지 않으면 공격 중 아님
	if (!CurrentAttackMontage) return false;

	// 해당 몽타주가 실제로 재생 중인지 확인
	return AnimInst->Montage_IsPlaying(CurrentAttackMontage);
}

void UCombatComponent::OpenComboWindow()
{
	bComboWindowOpen = true;

	// 이미 클릭을 버퍼해둔 상태면 즉시 다음 타로 진행
	if (bBufferedNextAttack)
	{
		bBufferedNextAttack = false;
		bComboWindowOpen = false;

		// 다음 섹션으로
		if (ComboSections.Num() > 0)
		{
			ComboIndex = FMath::Clamp(ComboIndex + 1, 0, ComboSections.Num() - 1);
		}
		StartAttack(); // Combo2/Combo3
	}
}

void UCombatComponent::CloseComboWindow()
{
	bComboWindowOpen = false;
	bBufferedNextAttack = false;

	// 윈도우가 닫혔는데 다음 입력이 없었다면, 콤보를 끊어주고
	// 다음 클릭은 다시 1타부터 시작하게 만들기
	ComboIndex = 0;
}

void UCombatComponent::HandleUnarmedAttack(bool bPressed)
{
	bAttackPressed = bPressed;

	if (bPressed)
	{
		StartAttack(); // 1 → 2 → 3 자동 콤보
	}
}

void UCombatComponent::HandleSwordAttack(bool bPressed)
{
	if (!bPressed) return;

	// 공격 중이 아니면 1타
	if (!IsAttackMontagePlaying())
	{
		ComboIndex = 0;
		StartAttack();
		return;
	}

	// 공격 중이면 콤보 윈도우에서만 예약
	if (bComboWindowOpen)
	{
		bBufferedNextAttack = true;
	}
}


void UCombatComponent::HandleAssaultRifleAttack(bool bPressed)
{
	/*
	bAttackPressed = bPressed;

	if (bPressed)
	{
		StartAutoFire();
	}
	else
	{
		StopAutoFire();
	}
	*/
}


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

// 몽타주 가져오기
UAnimMontage* UCombatComponent::GetCurrentAttackMontage() const
{
	if (CurrentWeaponData && CurrentWeaponData->AttackMontage)
	{
		return CurrentWeaponData->AttackMontage;
	}
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
	
	switch (GetCurrentWeaponType())
	{
	case EWeaponType::Unarmed:
		if (bAttackPressed)
		{
			ComboIndex = (ComboIndex + 1) % ComboSections.Num();
			StartAttack();
		}
		else
		{
			ComboIndex = 0;
		}
		break;

	case EWeaponType::Sword:
		// Sword는 “타이밍 클릭”으로만 진행: 여기서 자동 다음타 금지
		// 콤보 인덱스 리셋은 상황에 맞게(보통 0으로)
		ComboIndex = 0;
		bComboWindowOpen = false;
		bBufferedNextAttack = false;
		break;

	default:
		ComboIndex = 0;
		break;
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


