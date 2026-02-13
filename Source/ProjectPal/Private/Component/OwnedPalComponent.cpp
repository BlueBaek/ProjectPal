// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/OwnedPalComponent.h"

#include "Component/PalStatComponent.h"

// Sets default values for this component's properties
UOwnedPalComponent::UOwnedPalComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
}


// Called when the game starts
void UOwnedPalComponent::BeginPlay()
{
	Super::BeginPlay();
	
	FixActiveIndexAfterChange();
}

// 팰 추가 가능한가?
bool UOwnedPalComponent::CanAddPal() const
{
	return OwnedPals.Num() < MaxOwnedCount;
}

// 팰 추가
bool UOwnedPalComponent::AddPal(const FPalOwnedEntry& Entry)
{
	if (!Entry.IsValid()) return false;
	if (!CanAddPal()) return false;

	OwnedPals.Add(Entry);

	// 첫 팰 보유 시 자동 활성화
	if (ActiveIndex == INDEX_NONE)
	{
		ActiveIndex = 0;
		OnActivePalChanged.Broadcast(ActiveIndex);
	}

	OnOwnedPalsChanged.Broadcast();
	return true;
}

// 해당 인덱스의 Pal제거
bool UOwnedPalComponent::RemovePalByIndex(int32 Index)
{
	// 팰이 없으면 return
	if (!OwnedPals.IsValidIndex(Index)) return false;
	
	OwnedPals.RemoveAt(Index);
	FixActiveIndexAfterChange();

	OnOwnedPalsChanged.Broadcast();
	return true;
}

bool UOwnedPalComponent::SetActiveIndex(int32 NewIndex)
{
	if (!OwnedPals.IsValidIndex(NewIndex)) return false;
	if (ActiveIndex == NewIndex) return true;

	const bool bWasSpawned = (CurrentSpawnedPal != nullptr);
	
	ActiveIndex = NewIndex;
	OnActivePalChanged.Broadcast(ActiveIndex);
	
	if (bWasSpawned)
	{
		DespawnCurrentPal();
		SpawnActivePal();
	}
	
	return true;
}

bool UOwnedPalComponent::GetPalByIndex(int32 Index, FPalOwnedEntry& OutEntry) const
{
	if (!OwnedPals.IsValidIndex(Index))
	{
		return false;
	}

	OutEntry = OwnedPals[Index];
	return true;
}

// 현재 선택된 팰의 정보를 반환
bool UOwnedPalComponent::GetActivePal(FPalOwnedEntry& OutEntry) const
{
	return GetPalByIndex(ActiveIndex, OutEntry);
}

bool UOwnedPalComponent::AddPalFromActor(APalCharacter* PalActor)
{
	if (!PalActor) return false;
	if (!CanAddPal()) return false;

	FPalOwnedEntry Entry;
	Entry.PalClass = PalActor->GetClass();

	// 레벨은 StatComponent에서
	if (UPalStatComponent* Stat = PalActor->GetStatComponent())
	{
		Entry.Level = Stat->GetLevel();
	}

	// 닉네임은 일단 비워두거나, 필요하면 PalCharacter에 DisplayName Getter 추가해서 넣어도 됨
	// Entry.Nickname = ...

	return AddPal(Entry);
}

bool UOwnedPalComponent::SelectPrevPal()
{
	const int32 Count = OwnedPals.Num();
	if (Count <= 0) return false;

	int32 NewIndex = (ActiveIndex == INDEX_NONE) ? 0 : (ActiveIndex - 1);
	if (NewIndex < 0) NewIndex = Count - 1;

	return SetActiveIndex(NewIndex);
}

bool UOwnedPalComponent::SelectNextPal()
{
	const int32 Count = OwnedPals.Num();
	if (Count <= 0) return false;

	int32 NewIndex = (ActiveIndex == INDEX_NONE) ? 0 : (ActiveIndex + 1);
	if (NewIndex >= Count) NewIndex = 0;

	return SetActiveIndex(NewIndex);
}

// 변화가 있으면 BroadCast
void UOwnedPalComponent::FixActiveIndexAfterChange()
{
	if (OwnedPals.Num() == 0)
	{
		if (ActiveIndex != INDEX_NONE)
		{
			ActiveIndex = INDEX_NONE;
			OnActivePalChanged.Broadcast(ActiveIndex);
		}
		return;
	}

	const int32 Clamped = FMath::Clamp(ActiveIndex, 0, OwnedPals.Num() - 1);
	if (ActiveIndex != Clamped)
	{
		ActiveIndex = Clamped;
		OnActivePalChanged.Broadcast(ActiveIndex);
	}
}

APalCharacter* UOwnedPalComponent::SpawnPalFromEntry(const FPalOwnedEntry& Entry)
{
	if (!Entry.PalClass) return nullptr;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return nullptr;

	UWorld* World = OwnerActor->GetWorld();
	if (!World) return nullptr;

	// 플레이어 기준 소환 위치
	const FVector SpawnLoc = OwnerActor->GetActorLocation() + OwnerActor->GetActorRotation().RotateVector(SpawnOffset);
	const FRotator SpawnRot = OwnerActor->GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = OwnerActor;
	Params.Instigator = Cast<APawn>(OwnerActor);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APalCharacter* Spawned = World->SpawnActor<APalCharacter>(Entry.PalClass, SpawnLoc, SpawnRot, Params);
	return Spawned;
}

void UOwnedPalComponent::SyncFromSpawnedPalToEntry(APalCharacter* PalActor, FPalOwnedEntry& Entry)
{
	if (!PalActor) return;

	// 예시: 레벨 동기화
	if (UPalStatComponent* Stat = PalActor->GetStatComponent())
	{
		Entry.Level = Stat->GetLevel();
	}

	// TODO: HP/스킬/상태이상/경험치 등도 여기서 Entry에 저장
}

bool UOwnedPalComponent::SpawnActivePal()
{
	if (CurrentSpawnedPal)  // 이미 소환되어 있으면 실패
		return false;

	if (!OwnedPals.IsValidIndex(ActiveIndex))
		return false;

	const FPalOwnedEntry& Entry = OwnedPals[ActiveIndex];
	if (!Entry.IsValid())
		return false;

	APalCharacter* Spawned = SpawnPalFromEntry(Entry);
	if (!Spawned)
		return false;

	// 소유/그룹 처리(프로젝트 규칙에 맞게)
	Spawned->SetPalGroup(EPalGroup::Tamed);

	// TODO: Entry(영속 데이터) -> PalStatComponent 초기화(레벨/HP/스킬) 필요하면 여기서
	// if (UPalStatComponent* Stat = Spawned->GetStatComponent()) { Stat->InitFromOwned(Entry); }

	CurrentSpawnedPal = Spawned;
	CurrentSpawnedIndex = ActiveIndex;
	return true;
}

bool UOwnedPalComponent::DespawnCurrentPal()
{
	if (!CurrentSpawnedPal)
		return false;

	// ActiveIndex가 유효할 때만 저장 갱신
	if (OwnedPals.IsValidIndex(ActiveIndex))
	{
		SyncFromSpawnedPalToEntry(CurrentSpawnedPal, OwnedPals[ActiveIndex]);
	}

	// 월드에서 제거(회수)
	CurrentSpawnedPal->Destroy();
	CurrentSpawnedPal = nullptr;
	CurrentSpawnedIndex = INDEX_NONE;
	return true;
}

bool UOwnedPalComponent::ToggleSpawn()
{
	// 소환된 팰이 없으면 → 현재 선택 팰 소환
	if (!CurrentSpawnedPal)
	{
		return SpawnActivePal();
	}

	// 소환된 팰이 있는데, 현재 슬롯이 유효하지 않으면 → 일단 회수만
	if (!OwnedPals.IsValidIndex(ActiveIndex))
	{
		return DespawnCurrentPal();
	}

	// ✅ 같은 슬롯이면: 회수만(토글 off)
	if (CurrentSpawnedIndex == ActiveIndex)
	{
		return DespawnCurrentPal();
	}

	// ✅ 다른 슬롯이면: 회수 후 소환(교체)
	const bool bDespawnOk = DespawnCurrentPal();
	if (!bDespawnOk) return false;

	return SpawnActivePal();
}

