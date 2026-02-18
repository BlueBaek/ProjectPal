// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/OwnedPalComponent.h"

#include "Character/Player/PlayerCharacter.h"
#include "Component/PalSkillComponent.h"
#include "Component/PalStatComponent.h"
#include "Kismet/GameplayStatics.h"

static AActor* ResolveMasterActor(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	// 로컬 싱글 기준: 0번 플레이어
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
	return PlayerPawn; // PlayerCharacter면 Pawn이기도 함
}

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

	// --- Stat 스냅샷 ---
	if (UPalStatComponent* Stat = PalActor->GetStatComponent())
	{
		Stat->ExportToOwned(Entry.StatData);
		Entry.Level = Entry.StatData.Level;
	}

	// --- Skill 스냅샷 ---
	if (UPalSkillComponent* Skill = PalActor->FindComponentByClass<UPalSkillComponent>())
	{
		Skill->ExportToOwned(Entry.SkillData);
	}

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

	AActor* Master = ResolveMasterActor(this);
	if (!Master) return nullptr;

	UWorld* World = Master->GetWorld();
	if (!World) return nullptr;

	// 플레이어 기준 소환 위치
	const FVector SpawnLoc = Master->GetActorLocation() + Master->GetActorRotation().RotateVector(SpawnOffset);
	const FRotator SpawnRot = Master->GetActorRotation();
	const FTransform SpawnTM(SpawnRot, SpawnLoc);
	
	// ✅ Deferred Spawn: FinishSpawning 전까지 초기값 세팅 가능
	APalCharacter* Pal = World->SpawnActorDeferred<APalCharacter>(
		Entry.PalClass,
		SpawnTM,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (!Pal)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnOwnedPalDeferred] SpawnActorDeferred failed"));
		return nullptr;
	}
	
	// ✅ OnPossess가 일어나기 전에 "소유 팰" 상태를 확정해야 BT 교체가 됨
	Pal->SetMasterActor(Master);
	Pal->SetPalGroup(EPalGroup::Tamed);
	Pal->FinishSpawning(SpawnTM);
	
	UE_LOG(LogTemp, Warning, TEXT("[Spawn] Pal=%s Master=%s Owner=%s Controller=%s"),
		*GetNameSafe(Pal),
		*GetNameSafe(Pal->GetMasterActor()),
		*GetNameSafe(Pal->GetOwner()),
		*GetNameSafe(Pal->GetController()));
	
	return Pal;
}

void UOwnedPalComponent::SyncFromSpawnedPalToEntry(APalCharacter* PalActor, FPalOwnedEntry& Entry)
{
	if (!PalActor) return;

	// --- Stat 저장 ---
	if (UPalStatComponent* Stat = PalActor->GetStatComponent())
	{
		Stat->ExportToOwned(Entry.StatData);
		Entry.Level = Entry.StatData.Level;
	}

	// --- Skill 저장 ---
	if (UPalSkillComponent* Skill = PalActor->FindComponentByClass<UPalSkillComponent>())
	{
		Skill->ExportToOwned(Entry.SkillData);
	}
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
	
	// 팰의 Owner 지정(AI용)
	Spawned->SetOwner(GetOwner());
	
	// ✅ Entry -> Component 복원
	if (UPalStatComponent* Stat = Spawned->GetStatComponent())
	{
		Stat->ImportFromOwned(Entry.StatData);
	}

	if (UPalSkillComponent* Skill = Spawned->FindComponentByClass<UPalSkillComponent>())
	{
		Skill->ImportFromOwned(Entry.SkillData);
	}

	CurrentSpawnedPal = Spawned;
	CurrentSpawnedIndex = ActiveIndex;
	return true;
}

bool UOwnedPalComponent::DespawnCurrentPal()
{
	if (!CurrentSpawnedPal)
		return false;

	// ActiveIndex가 유효할 때만 저장 갱신
	// if (OwnedPals.IsValidIndex(ActiveIndex))
	// {
	// 	SyncFromSpawnedPalToEntry(CurrentSpawnedPal, OwnedPals[ActiveIndex]);
	// }
	const int32 SaveIndex = (OwnedPals.IsValidIndex(CurrentSpawnedIndex)) ? CurrentSpawnedIndex : ActiveIndex;
	if (OwnedPals.IsValidIndex(SaveIndex))
	{
		SyncFromSpawnedPalToEntry(CurrentSpawnedPal, OwnedPals[SaveIndex]);
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

