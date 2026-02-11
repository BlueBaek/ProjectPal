// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PalSkillComponent.h"

#include "Character/Pal/PalCharacter.h"
#include "DataAsset/PalSkillDataAsset.h"
#include "PalSkill/PalSkillExecution.h"

// Sets default values for this component's properties
UPalSkillComponent::UPalSkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game startsd
void UPalSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// Active 슬롯 최소 3칸 확보
	if (Skills.Num() < ActiveSlotCount)
	{
		Skills.SetNum(ActiveSlotCount);
	}

	SelectedActiveSlotIndex = FMath::Clamp(SelectedActiveSlotIndex, 0, ActiveSlotCount - 1);
}


// Called every frame
void UPalSkillComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 스킬별 쿨타임 감소
	for (auto& Pair : SkillCooldownRemaining)
	{
		float& Remaining = Pair.Value;
		if (Remaining > 0.f)
		{
			Remaining = FMath::Max(0.f, Remaining - DeltaTime);
		}
	}
}

// 배열에 스킬 넣기
UPalSkillDataAsset* UPalSkillComponent::GetSkillAt(int32 Index) const
{
	// 유효한 Index가 아니면 nullptr
	if (!IsValidIndex(Index))
		return nullptr;

	// 해당 인덱스에 스킬 넣기
	return Skills[Index].Skill;
}

// SkillAsset이 이미 가지고 있는 스킬인지 검사
bool UPalSkillComponent::HasSkill(UPalSkillDataAsset* SkillAsset) const
{
	// AssetData가 유효하지 않으면 false
	if (!SkillAsset) return false;

	for (const FPalSkillSlot& Slot : Skills)
	{
		if (Slot.Skill == SkillAsset)
			return true;
	}
	return false;
}

// 스킬 배우기(배열에 스킬 추가)
bool UPalSkillComponent::LearnSkill(UPalSkillDataAsset* SkillAsset)
{
	// DataAsset이 유효한지 검사
	if (!SkillAsset)
		return false;

	// 이미 있으면 중복 학습 방지
	if (HasSkill(SkillAsset))
		return true;

	// Active(0~2) 중 빈칸이 있으면 그 칸부터 채움
	for (int32 i = 0; i < ActiveSlotCount; ++i)
	{
		if (!Skills[i].Skill)
		{
			Skills[i].Skill = SkillAsset;
			OnSkillSlotChanged.Broadcast(i, SkillAsset); // 즉시 UI 갱신
			return true; // 채웠으면 return
		}
	}

	// Active가 꽉 찼으면 learned 영역(3번 이후)에 추가
	FPalSkillSlot NewSlot;
	NewSlot.Skill = SkillAsset;
	Skills.Add(NewSlot);

	OnSkillSlotChanged.Broadcast(Skills.Num() - 1, SkillAsset);
	return true;
}

// 스킬 교체
bool UPalSkillComponent::EquipLearnedSkillToActiveSlot(int32 ActiveIndex, int32 LearnedIndex, bool bSwap)
{
	// Active 영역 인덱스 유효성 검사
	if (!IsValidActiveIndex(ActiveIndex))
		return false;

	// Learned 영역 인덱스 유효성 검사
	if (!IsValidLearnedIndex(LearnedIndex))
		return false;

	UPalSkillDataAsset* LearnedSkill = Skills[LearnedIndex].Skill;
	// DataAsset 유효성 검사
	if (!LearnedSkill)
		return false;

	// 스킬 스왑
	if (bSwap)
	{
		// Active <-> Learned 스킬 포인터 교환
		Swap(Skills[ActiveIndex].Skill, Skills[LearnedIndex].Skill);
	}
	else
	{
		// 덮어쓰기(learned는 유지) - 같은 스킬이 중복으로 존재할 수 있음
		Skills[ActiveIndex].Skill = LearnedSkill;
	}

	// UI 갱신
	OnSkillSlotChanged.Broadcast(ActiveIndex, Skills[ActiveIndex].Skill);
	return true;
}

// Active 스킬 선택 (입력 처리, AI 처리, UI 갱신 등에 필요)
// “현재 이 팰이 ‘다음에 사용할 스킬’이 무엇인지 결정하는 상태를 관리하기 위한 함수”
bool UPalSkillComponent::SelectActiveSlot(int32 NewIndex)
{
	// Active스킬 유효성 판단
	if (!IsValidActiveIndex(NewIndex))
		return false;

	// 이전에 선택한 Slot이 현재 선택한 slot과 일치
	if (SelectedActiveSlotIndex == NewIndex)
		return true;

	const int32 Old = SelectedActiveSlotIndex;
	// 선택한 슬롯 index값으로 교체
	SelectedActiveSlotIndex = NewIndex;

	// 선택 슬롯 UI 갱신
	OnSelectedSlotChanged.Broadcast(NewIndex, Old);
	return true;
}

// ===== Index기반 스킬 쿨타임 로직. 즉, Active Skill Slot 자체에 쿨타임 - 사용 안함 =====
/*
bool UPalSkillComponent::IsOnCooldown(int32 Index) const
{	
	UPalSkillDataAsset* Skill = GetSkillAt(Index);
	return IsSkillOnCooldown(Skill);
}

float UPalSkillComponent::GetCooldownRemaining(int32 Index) const
{
	UPalSkillDataAsset* Skill = GetSkillAt(Index);
	return GetSkillCooldownRemaining(Skill);
}

void UPalSkillComponent::StartCooldown(int32 Index, float CooldownSeconds)
{
	UPalSkillDataAsset* Skill = GetSkillAt(Index);
	StartSkillCooldown(Skill, CooldownSeconds);
}
*/

// ===== 개별 스킬 자체에 쿨타임 =====
// 쿨타임 중인지 여부 확인
bool UPalSkillComponent::IsSkillOnCooldown(UPalSkillDataAsset* Skill) const
{
	// DataAsset이 유효하지 않으면 false
	if (!Skill) return false;

	// float값이 0보다 크면 true
	if (const float* Rem = SkillCooldownRemaining.Find(Skill))
	{
		return (*Rem) > 0.f;
	}

	// 아니면 false
	return false;
}

// 남은 쿨타임 시간 얻기
float UPalSkillComponent::GetSkillCooldownRemaining(UPalSkillDataAsset* Skill) const
{
	if (!Skill) return 0.f;

	if (const float* Rem = SkillCooldownRemaining.Find(Skill))
	{
		return *Rem;
	}
	return 0.f;
}

// 쿨타임 적용
void UPalSkillComponent::StartSkillCooldown(UPalSkillDataAsset* Skill, float CooldownSeconds)
{
	// 스킬이 유효하지 않으면 작동X
	if (!Skill) return;

	// 쿨타임 시작 또는 갱신
	SkillCooldownRemaining.FindOrAdd(Skill) = FMath::Max(0.f, CooldownSeconds);
}

// 해당 Index의 스킬 사용 가능 여부
bool UPalSkillComponent::CanUseActiveSkill(int32 ActiveIndex) const
{
	// 유효성 판단
	if (!IsValidActiveIndex(ActiveIndex))
		return false;

	// 해당 인덱스의 Skill을 가져옴
	UPalSkillDataAsset* Skill = Skills[ActiveIndex].Skill;
	if (!Skill)
		return false;

	// 해당 스킬이 쿨타임 중이 아니라면 True
	return !IsSkillOnCooldown(Skill);
}

bool UPalSkillComponent::IsValidIndex(int32 Index) const
{
	return Skills.IsValidIndex(Index);
}

bool UPalSkillComponent::IsValidActiveIndex(int32 ActiveIndex) const
{
	// 0이상 ActiveSlotCount미만 까지만 유효
	return ActiveIndex >= 0 && ActiveIndex < ActiveSlotCount;
}

bool UPalSkillComponent::IsValidLearnedIndex(int32 LearnedIndex) const
{
	// 액티브 스킬 슬롯의 인덱스보다 크면서 유효한 Index임
	return LearnedIndex >= ActiveSlotCount && Skills.IsValidIndex(LearnedIndex);
}

APalCharacter* UPalSkillComponent::GetPalOwner() const
{
	return Cast<APalCharacter>(GetOwner());
}

bool UPalSkillComponent::TryUseSkill(const UPalSkillDataAsset* SkillData, AActor* Target)
{
	APalCharacter* Pal = GetPalOwner();
	UWorld* World = GetWorld();
	if (!Pal || !World || !SkillData)
	{
		return false;
	}

	UPalSkillDataAsset* MutableSkill = const_cast<UPalSkillDataAsset*>(SkillData);

	// 쿨타임 체크
	if (IsSkillOnCooldown(MutableSkill))
	{
		return false;
	}

	if (!SkillData->ExecutionClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[Skill] ExecutionClass is null: %s"), *GetNameSafe(SkillData));
		return false;
	}

	// 실행 객체 생성 (PrepareTime 동안 GC 방지를 위해 PendingExecutions에 보관)
	UPalSkillExecution* Exec = NewObject<UPalSkillExecution>(this, SkillData->ExecutionClass);
	if (!Exec)
	{
		return false;
	}
	
	// 타이머로 지연 실행될 수 있으므로 GC 방지
	PendingExecutions.Add(Exec);

	// Prepare 단계
	if (!Exec->StartPrepare(Pal, Target, SkillData))
	{
		PendingExecutions.Remove(Exec);
		return false;
	}

	// 쿨타임은 "시전 시작" 시점에 적용
	StartSkillCooldown(MutableSkill, FMath::Max(0.f, SkillData->Timing.Cooldown));

	const float PrepareTime = FMath::Max(0.f, SkillData->Timing.PrepareTime);
	if (PrepareTime <= 0.f)
	{
		// Start(Action) 단계에 들어갈 것이므로 Notify 전달 대상 등록
		SetActiveExecution(Exec);
		Exec->Activate();

		PendingExecutions.Remove(Exec);
		return true;
	}

	// PrepareTime 후 Activate 예약
	FTimerHandle TempHandle;
	Pal->GetWorld()->GetTimerManager().SetTimer(
		TempHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, Exec]()
		{
			// 타이머 시점엔 Component/Exec가 이미 파괴될 수도 있으니 안전 체크
			if (!IsValid(this) || !IsValid(Exec))
			{
				return;
			}

			// Start(Action) 단계 시작: Notify(SkillFire)가 들어올 Execution을 등록
			SetActiveExecution(Exec);

			Exec->Activate();

			// Finish()에서 ClearActiveExecution(this)를 해도 되지만,
			// 일단 "Activate 호출 직후 해제"하면 SkillFire가 못 올 수도 있음.
			// ✅ 그래서 Clear는 "Execution::Finish()"에서 해제하는 방식이 더 안전하다.
			//
			// 여기서는 Execution이 Finish에서 ClearActiveExecution 호출한다고 가정하고,
			// 여기서는 건드리지 않는다.

			PendingExecutions.Remove(Exec);
		}),
		PrepareTime,
		false
	);

	return true;
}

bool UPalSkillComponent::TryUseSelectedSkill(AActor* Target)
{
	UPalSkillDataAsset* Skill = GetSkillAt(SelectedActiveSlotIndex);
	return TryUseSkill(Skill, Target);
}

// 사용 가능한 스킬 랜덤으로 Pick
int32 UPalSkillComponent::PickRandomUsableActiveSlot() const
{
	// 사용 가능한 스킬을 저장할 Array
	TArray<int32> Candidates;
	// 데이터를 담을 공간을 미리 확보 (3칸)
	Candidates.Reserve(ActiveSlotCount);

	for (int32 i = 0; i < ActiveSlotCount; ++i)
	{
		// 슬롯 범위 + 스킬 존재 + 쿨타임 아님
		if (CanUseActiveSkill(i))
		{
			Candidates.Add(i);
		}
	}

	// 사용 가능한 스킬이 없음
	if (Candidates.Num() == 0)
	{
		// 유효하지 않은 Index를 반환
		return INDEX_NONE;
	}
	
	// 목록중 랜덤으로 하나 뽑아서 return
	return Candidates[FMath::RandHelper(Candidates.Num())];;
}

// 유효성 판단
bool UPalSkillComponent::TryUseRandomActiveSkill(AActor* Target, int32& OutUsedSlotIndex)
{
	// INDEX_NONE == -1 (유효하지 않은 INDEX)
	OutUsedSlotIndex = INDEX_NONE;

	// 랜덤 선택으로 선택된 슬롯
	const int32 PickedSlot = PickRandomUsableActiveSlot();
	if (PickedSlot == INDEX_NONE)
	{
		// 값이 없으면 return
		return false;
	}

	// 선택된 슬롯의 스킬을 가져옴 (SkillDataAsset을 가져옴)
	UPalSkillDataAsset* Skill = GetSkillAt(PickedSlot);
	if (!Skill)
	{
		// 비어 있으면 return
		return false;
	}

	// 원하면 "현재 선택 슬롯"도 같이 갱신해서
	// 다른 시스템(UI/디버그/BT)에서 추적 가능하게 할 수 있음
	SelectActiveSlot(PickedSlot);	// UI 갱신용
	OutUsedSlotIndex = PickedSlot;

	// 스킬 사용, 쿨타임 적용
	return TryUseSkill(Skill, Target);	
}

void UPalSkillComponent::HandleSkillFireNotify()
{
	// 실행중인 Execution이 있다면
	if (ActiveExecution)
	{
		// 스킬 발사 로직 실행
		ActiveExecution->OnSkillFire();
	}
}

void UPalSkillComponent::SetActiveExecution(class UPalSkillExecution* InExec)
{
	// Activate() 단계에서 Execution이 자기 자신을 등록함
	ActiveExecution = InExec;
}

void UPalSkillComponent::ClearActiveExecution(class UPalSkillExecution* InExec)
{
	// 스킬 종료 시 자기 자신만 해제
	if (ActiveExecution == InExec)
	{
		ActiveExecution = nullptr;
	}
}
