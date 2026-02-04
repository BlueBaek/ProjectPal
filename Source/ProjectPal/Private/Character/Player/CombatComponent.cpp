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
	if (OwnerCharacter)
	{
		// 무기 메쉬 컴포넌트 생성(한 번만)
		EquippedWeaponComp = NewObject<USkeletalMeshComponent>(OwnerCharacter, TEXT("EquippedWeapon"));
		if (EquippedWeaponComp)
		{
			EquippedWeaponComp->RegisterComponent();

			// 충돌/오버랩 OFF (무기 히트는 나중에 트레이스/콜리전으로 따로 처리)
			EquippedWeaponComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			EquippedWeaponComp->SetGenerateOverlapEvents(false);

			// 그림자
			EquippedWeaponComp->CastShadow = true;

			// 기본은 숨김
			EquippedWeaponComp->SetVisibility(false, true);

			// 캐릭터에 컴포넌트로 붙여두기(Attach는 Equip 때 소켓으로)
			EquippedWeaponComp->AttachToComponent(
				OwnerCharacter->GetMesh(),
				FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	EquipWeaponData(nullptr);
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

void UCombatComponent::OpenComboWindow()
{
	bComboWindowOpen = true;
}


void UCombatComponent::CloseComboWindow()
{
	bComboWindowOpen = false;
}

void UCombatComponent::TryAdvanceSwordCombo(UAnimInstance* AnimInst, UAnimMontage* Montage)
{
	if (!AnimInst || !Montage) return;
	
	const int32 NextIndex = ComboIndex + 1;
	
	// ComboSection이 존재하지 않으면 종료
	if (!ComboSections.IsValidIndex(NextIndex)) return;


	const FName NextSection = ComboSections[NextIndex];
	if (!IsComboSectionValid(Montage, NextSection)) return;

	// 🔥 "중간에 즉시 다음 섹션으로" 점프
	ComboIndex = NextIndex;
	AnimInst->Montage_JumpToSection(NextSection, Montage);
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

void UCombatComponent::AttachWeapon(UWeaponDataAsset* WeaponDA)
{
	if (!OwnerCharacter || !EquippedWeaponComp || !WeaponDA) return;

	EquippedWeaponComp->SetSkeletalMesh(WeaponDA->WeaponMesh);
	EquippedWeaponComp->SetVisibility(true, true);

	const FName SocketName = WeaponDA->EquipSocketName.IsNone()
		                         ? FName(TEXT("Socket_Weapon_R"))
		                         : WeaponDA->EquipSocketName;

	// 손 소켓에 스냅
	EquippedWeaponComp->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);

	// 무기마다 오프셋이 필요하면 데이터로 빼서 여기서 적용
	EquippedWeaponComp->SetRelativeLocation(FVector::ZeroVector);
	EquippedWeaponComp->SetRelativeRotation(FRotator::ZeroRotator);
	EquippedWeaponComp->SetRelativeScale3D(FVector(1.f));
}

// 무기 없애기
void UCombatComponent::ClearWeapon()
{
	if (!EquippedWeaponComp) return;

	EquippedWeaponComp->SetSkeletalMesh(nullptr);
	EquippedWeaponComp->SetVisibility(false, true);
}

// AnimLayer 교체
void UCombatComponent::ApplyAnimLayer(UWeaponDataAsset* WeaponDA)
{
	if (!OwnerCharacter || !WeaponDA) return;

	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;

	AnimInst->LinkAnimClassLayers(WeaponDA->AnimLayerClass);
}

void UCombatComponent::RestoreUnarmedAnimLayer()
{
	if (!OwnerCharacter) return;

	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;

	if (UnarmedData && UnarmedData->AnimLayerClass)
	{
		AnimInst->LinkAnimClassLayers(UnarmedData->AnimLayerClass);
	}
}

void UCombatComponent::EquipWeaponData(UWeaponDataAsset* NewWeaponData)
{
	// 공격 중이면(몽타주 재생 중) 교체 막기
	if (IsAttackMontagePlaying()) return;

	// 기존 무기 제거 (기존 EquippedWeaponComp에 적용된 SkeletalMesh를 지우고 안보이게)
	ClearWeapon();

	// 무기 데이터 갱신 (빈 슬롯이면 Unarmed)
	CurrentWeaponData = NewWeaponData;

	// 애님 레이어 적용
	if (CurrentWeaponData && CurrentWeaponData->AnimLayerClass)
	{
		ApplyAnimLayer(CurrentWeaponData);
	}
	else
	{
		// 무기 없으면 Unarmed 레이어로 복귀
		RestoreUnarmedAnimLayer();
	}

	// 5) 무기 장착
	if (CurrentWeaponData && CurrentWeaponData->WeaponMesh)
	{
		AttachWeapon(CurrentWeaponData);
	}

	// 중요: 공격 몽타주/AnimLayerClass를 CurrentWeaponData 기준으로 쓰도록 되어 있어야 함
	// GetCurrentAttackMontage()가 CurrentWeaponData 우선 반환인지 꼭 확인!
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
	if (!OwnerCharacter) return;

	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;

	UAnimMontage* Montage = GetCurrentAttackMontage();
	if (!Montage) return;

	// 1) 이미 재생 중이면: OpenComboWindow 동안만 다음 섹션으로 "즉시" 점프
	if (AnimInst->Montage_IsPlaying(Montage))
	{
		if (bComboWindowOpen)
		{
			// 윈도우에서만 점프
			TryAdvanceSwordCombo(AnimInst, Montage);
		}
		return;
	}

	// 2) 재생 중이 아니면: 1타부터 시작
	ComboIndex = 0;
	bComboWindowOpen = false;

	StartAttack(); // StartAttack 내부에서 ComboIndex(0) 섹션(Combo1)로 시작하게 되어 있음
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

	// 몽타주가 없으면 return
	if (!Montage) return;

	UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;

	if (AnimInst->Montage_IsPlaying(Montage))
	{
		return;
	}

	// 추가 : 콤보 섹션 안전 처리
	if (ComboSections.Num() <= 0)
	{
		ComboIndex = 0;
	}
	else
	{
		// 현재 ComboIndex 결정
		ComboIndex = FMath::Clamp(ComboIndex, 0, ComboSections.Num() - 1);
	}

	CurrentAttackMontage = Montage;

	// 1) 재생 시도
	const float PlayResult = AnimInst->Montage_Play(Montage);

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
