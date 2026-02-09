// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PalSkillComponent.generated.h"

class UPalSkillDataAsset;
class UPalSkillExecution;

// Delegate : UI 갱신용
// Active스킬 슬롯(index 0 ~ 2) 변경
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPalSkillSlotChanged, int32, SlotIndex, UPalSkillDataAsset*, NewSkill);
// Active기준 스킬 변경했을 때
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPalSkillSelectedChanged, int32, NewIndex, int32, OldIndex);

/*
 * 단일 배열 구조:
 * Skills[0..2]  : 현재 사용 가능한 스킬 3칸 (Active)
 * Skills[3..]   : 배워서 기억하고 있는 스킬들 (Learned)
 *
 * 쿨타임은 "스킬(=DataAsset)" 단위로 TMap에 저장한다.
 */
USTRUCT(BlueprintType)
struct FPalSkillSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UPalSkillDataAsset> Skill = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPAL_API UPalSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPalSkillComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ------------------------
	// 슬롯 접근
	// ------------------------
	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	UPalSkillDataAsset* GetSkillAt(int32 Index) const;

	// ------------------------
	// 배우기(기억하기)
	// ------------------------
	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	bool LearnSkill(UPalSkillDataAsset* SkillAsset);

	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	bool HasSkill(UPalSkillDataAsset* SkillAsset) const;

	// ------------------------
	// 장착(Active 0~2 교체)
	// ------------------------
	// 스킬 교체
	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	bool EquipLearnedSkillToActiveSlot(int32 ActiveIndex, int32 LearnedIndex, bool bSwap = true);

	// ------------------------
	// 선택
	// ------------------------
	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	bool SelectActiveSlot(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	int32 GetSelectedActiveSlotIndex() const { return SelectedActiveSlotIndex; }
	
	// ------------------------
	// 쿨다운 (스킬 기반 API) - 현재 채용
	// ------------------------
	// 쿨타임 중인지 여부 확인
	UFUNCTION(BlueprintCallable, Category="Pal|Skill|Cooldown")
	bool IsSkillOnCooldown(UPalSkillDataAsset* Skill) const;

	// 남은 쿨타임
	UFUNCTION(BlueprintCallable, Category="Pal|Skill|Cooldown")
	float GetSkillCooldownRemaining(UPalSkillDataAsset* Skill) const;

	// 쿨타임 적용
	UFUNCTION(BlueprintCallable, Category="Pal|Skill|Cooldown")
	void StartSkillCooldown(UPalSkillDataAsset* Skill, float CooldownSeconds);

	// Active 슬롯 사용 가능 여부 (쿨 포함)
	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	bool CanUseActiveSkill(int32 ActiveIndex) const;

	// ------------------------
	// 실제 스킬 사용 (DataAsset 기반)
	// ------------------------
	// SkillDataAsset만 있으면 해당 스킬을 실행할 수 있다.
	// (실제 실행 내용은 SkillDataAsset.ExecutionClass(UPalSkillExecution 전략)에 의해 결정)
	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	bool TryUseSkill(const UPalSkillDataAsset* SkillData, AActor* Target);

	// 현재 선택된 슬롯의 스킬을 사용
	UFUNCTION(BlueprintCallable, Category="Pal|Skill")
	bool TryUseSelectedSkill(AActor* Target);
	
	// ------------------------
	// Delegate
	// ------------------------
	UPROPERTY(BlueprintAssignable, Category="Pal|Skill")
	FOnPalSkillSlotChanged OnSkillSlotChanged;

	UPROPERTY(BlueprintAssignable, Category="Pal|Skill")
	FOnPalSkillSelectedChanged OnSelectedSlotChanged;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	// Index의 유효성 여부
	bool IsValidIndex(int32 Index) const;
	bool IsValidActiveIndex(int32 ActiveIndex) const;
	bool IsValidLearnedIndex(int32 LearnedIndex) const;
	
	UPROPERTY(EditDefaultsOnly, Category="Pal|Skill")
	int32 ActiveSlotCount = 3;

	// 스킬들이 담길 PalSkillSlot 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TArray<FPalSkillSlot> Skills;

	// 스킬별 남은 쿨타임 (팰 개체별로 별도 적용됨: 컴포넌트 인스턴스마다 맵이 따로 있음)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TMap<TObjectPtr<UPalSkillDataAsset>, float> SkillCooldownRemaining;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	int32 SelectedActiveSlotIndex = 0;
	
	// PrepareTime 이후 Activate까지 기다리는 Execution들이 GC로 사라지지 않도록 보관
	UPROPERTY()
	TArray<TObjectPtr<UPalSkillExecution>> PendingExecutions;
};
