// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PalSkillComponent.h"

#include "Projectile/PJ_GrassTornado.h"

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
void UPalSkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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
			OnSkillSlotChanged.Broadcast(i, SkillAsset);	// 즉시 UI 갱신
			return true;	// 채웠으면 return
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

void UPalSkillComponent::Cast_Test_GrassTornado(AActor* Target)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[Skill] Cast_Test_GrassTornado ENTER | Owner=%s Target=%s"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"),
		Target ? *Target->GetName() : TEXT("NULL"));
	
	AActor* Caster = GetOwner();
	if (!Caster || !GetWorld()) return;

	// 테스트 파라미터
	const float PrepareTime = 1.0f;
	const float MoveSpeed   = 220.f;
	const float LifeTime    = 3.0f;   // ✅ 3초 후 자동 소멸
	const float DamagePerTick   = 6.f;
	const float DamageInterval  = 0.2f;

	const FVector CasterLoc = Caster->GetActorLocation();
	const FRotator CasterRot = Caster->GetActorRotation();

	const FVector Right = CasterRot.RotateVector(FVector::RightVector);
	const float SideOffset = 120.f;

	const FVector SpawnL = CasterLoc - Right * SideOffset;
	const FVector SpawnR = CasterLoc + Right * SideOffset;

	FActorSpawnParameters Params;
	Params.Owner = Caster;
	Params.Instigator = Cast<APawn>(Caster);

	APJ_GrassTornado* TornadoL = GetWorld()->SpawnActor<APJ_GrassTornado>(
		APJ_GrassTornado::StaticClass(), SpawnL, CasterRot, Params);

	APJ_GrassTornado* TornadoR = GetWorld()->SpawnActor<APJ_GrassTornado>(
		APJ_GrassTornado::StaticClass(), SpawnR, CasterRot, Params);

	if (!TornadoL || !TornadoR) return;

	TornadoL->InitTornado(Caster, Target, MoveSpeed, LifeTime, DamagePerTick, DamageInterval);
	TornadoR->InitTornado(Caster, Target, MoveSpeed, LifeTime, DamagePerTick, DamageInterval);

	UE_LOG(LogTemp, Warning,
	TEXT("[Skill] Spawn Tornado | L=%s R=%s"),
	TornadoL ? *TornadoL->GetName() : TEXT("NULL"),
	TornadoR ? *TornadoR->GetName() : TEXT("NULL"));
	
	// PrepareTime 후 "발사"(방향 고정, 호밍 없음)
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateWeakLambda(this, [TornadoL, TornadoR]()
		{
			if (IsValid(TornadoL)) TornadoL->Activate();
			if (IsValid(TornadoR)) TornadoR->Activate();
		}),
		PrepareTime,
		false
	);
}
