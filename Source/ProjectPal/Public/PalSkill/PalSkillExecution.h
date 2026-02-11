// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PalSkillExecution.generated.h"

class APalCharacter;
class UAnimInstance;
class UAnimMontage;
class UPalSkillComponent;
class UPalSkillDataAsset;

// This class does not need to be modified.
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class UPalSkillExecution : public UObject
{
	GENERATED_BODY()
	
public:

	// 스킬 준비
	virtual bool StartPrepare(APalCharacter* InCaster, AActor* InTarget, const UPalSkillDataAsset* InSkillData);

	// 스킬 시전
	virtual void Activate();

	// Start 몽타주에서 SkillFire Notify가 오면 호출 (기본 Execution은 로직이 없음)
	virtual void OnSkillFire() {}

protected:

	// 캐시
	UPROPERTY()
	TObjectPtr<APalCharacter> Caster = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> Target = nullptr;

	UPROPERTY()
	TObjectPtr<const UPalSkillDataAsset> SkillData = nullptr;

	// 타이머
	FTimerHandle DurationEndHandle;

	// ----- EndDelegate 체인 콜백 -----
	UFUNCTION()
	void OnStartMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	// --------------------------------------------
	// 내부 헬퍼
	// --------------------------------------------
	UAnimInstance* GetAnimInstance() const;
	void PlayWithEndDelegate(UAnimMontage* Montage, FOnMontageEnded& InOutDelegate);
	void StopIfPlaying(UAnimMontage* Montage, float BlendOutTime = 0.1f) const;

	void Finish();           // 종료 처리(Execution 해제 등)
};
