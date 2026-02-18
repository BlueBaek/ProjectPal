// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Pal/PalCharacter.h"
#include "Components/ActorComponent.h"
#include "OwnedPalComponent.generated.h"

class APalCharacter;

// PalStat
USTRUCT(BlueprintType)
struct FPalStatSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Level = 1;

	// 개체값(랜덤) - 개체 고유성 유지용
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 IndividualHP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 IndividualAttack = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 IndividualDefense = 0;

	// 런타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CurrentHP = 0.f;
};

// PalSkill
USTRUCT(BlueprintType)
struct FPalSkillSaveData
{
	GENERATED_BODY()

	// Skills 배열 전체(Active 0~2 + Learned 3~)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<class UPalSkillDataAsset>> Skills;

	// 쿨타임 맵 저장용 (Asset -> Remaining)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<class UPalSkillDataAsset>> CooldownAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> CooldownRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SelectedActiveSlotIndex = 0;
};

// 팰 복원을 위해 필요한 팰의 정보
USTRUCT(BlueprintType)
struct FPalOwnedEntry
{
	GENERATED_BODY()

public:
	// 어떤 팰인지: 가장 단순하게는 클래스 참조(나중에 DataAsset/ID로 바꿔도 됨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APalCharacter> PalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InstanceId = FGuid::NewGuid();

	// 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;
	
	// 표시용 이름(선택)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Nickname;

	// ✅ 추가: 포획/회수 시점 스냅샷
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPalStatSaveData StatData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPalSkillSaveData SkillData;
	
	bool IsValid() const
	{
		return PalClass != nullptr;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOwnedPalsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActivePalChanged, int32, NewIndex);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPAL_API UOwnedPalComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOwnedPalComponent();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// 최대 보유 수 (요구사항: 5)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OwnedPal")
	int32 MaxOwnedCount = 5;

	// 현재 보유 팰 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OwnedPal")
	TArray<FPalOwnedEntry> OwnedPals;

	// 현재 활성 슬롯(소환할 팰 선택용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OwnedPal")
	int32 ActiveIndex = INDEX_NONE;

	// 현재 소환된 팰이 어떤 슬롯에서 나온 건지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OwnedPal")
	int32 CurrentSpawnedIndex = INDEX_NONE;
	
	// Delegate
	UPROPERTY(BlueprintAssignable, Category="OwnedPal")
	FOnOwnedPalsChanged OnOwnedPalsChanged;

	UPROPERTY(BlueprintAssignable, Category="OwnedPal")
	FOnActivePalChanged OnActivePalChanged;

	// ---- API ----
	// 팰 추가 가능 여부
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool CanAddPal() const;

	// 팰 추가
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool AddPal(const FPalOwnedEntry& Entry);

	// 해당 Index의 팰 제거
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool RemovePalByIndex(int32 Index);

	// 팰 슬롯 변경 함수
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool SetActiveIndex(int32 NewIndex);
	
	// Index로 팰 Get
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool GetPalByIndex(int32 Index, FPalOwnedEntry& OutEntry) const;

	// 선택한 팰의 정보를 반환
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool GetActivePal(FPalOwnedEntry& OutEntry) const;

	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool AddPalFromActor(APalCharacter* PalActor);
	
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool SelectPrevPal();

	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool SelectNextPal();
	
public:
	FORCEINLINE TArray<FPalOwnedEntry> GetOwnedPals() const {return OwnedPals;};
	
private:
	void FixActiveIndexAfterChange();
	
	// ===== 포획한 팰을 소환하기 위한 부분 추가 =====
	APalCharacter* SpawnPalFromEntry(const FPalOwnedEntry& Entry);
	void SyncFromSpawnedPalToEntry(APalCharacter* PalActor, FPalOwnedEntry& Entry);
	
public:
	// 현재 소환된 팰(1마리만 운영)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OwnedPal")
	TObjectPtr<APalCharacter> CurrentSpawnedPal = nullptr;

	// 소환 위치 오프셋(플레이어 기준)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OwnedPal")
	FVector SpawnOffset = FVector(150.f, 0.f, 0.f);

	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool IsPalSpawned() const { return CurrentSpawnedPal != nullptr; }

	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	APalCharacter* GetCurrentSpawnedPal() const { return CurrentSpawnedPal; }

	// 활성 팰 소환
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool SpawnActivePal();

	// 현재 팰 회수(저장 갱신 후 제거)
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool DespawnCurrentPal();

	// 토글
	UFUNCTION(BlueprintCallable, Category="OwnedPal")
	bool ToggleSpawn();
};
