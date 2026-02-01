// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class APlayerCharacter;
class UWeaponDataAsset;
class UAnimMontage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPAL_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UCombatComponent();
	
	// 입력 눌림/뗌 처리(컨트롤러에서 호출)
	void ProcessAttack(bool bPressed);
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<APlayerCharacter> OwnerCharacter;

	// 무기 없을 때 사용할 데이터(WeaponMesh는 NULL이어도 OK)
	UPROPERTY(EditDefaultsOnly, Category="Combat|Data")
	TObjectPtr<UWeaponDataAsset> UnarmedData;

	// 지금 재생 중인 공격 몽타주 추적
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentAttackMontage;
	
	// ===== 콤보 어택 구현용 =====
	// 입력 홀드 상태
	bool bAttackPressed = false;
	
	// 콤보 상태
	int32 ComboIndex = 0;

	// 마지막 공격 시간(콤보 리셋 판단)
	float LastAttackTime = -1000.f;
	
	// 콤보 리셋 딜레이(이 시간 넘으면 다시 1타부터)
	UPROPERTY(EditDefaultsOnly, Category="Combat|Combo")
	float ComboResetDelay = 0.9f;
	
	// 몽타주 섹션 이름들
	UPROPERTY(EditDefaultsOnly, Category="Combat|Combo")
	TArray<FName> ComboSections = { TEXT("Combo1"), TEXT("Combo2"), TEXT("Combo3") };
	
	// ===== 콤보 어택 구현용 =====
	
	void StartAttack();
	// void StopAttack();

	UAnimMontage* GetCurrentAttackMontage() const;
	
	// 몽타주 종료 타이밍 콜백
	UFUNCTION()
	void OnAttackMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	
private:
	// 섹션 유효성 체크
	bool IsComboSectionValid(UAnimMontage* Montage, const FName& SectionName) const;
};
