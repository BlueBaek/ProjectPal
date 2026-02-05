// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/WeaponType.h"
#include "CombatComponent.generated.h"

class APlayerCharacter;
class UWeaponDataAsset;
class UAnimMontage;
class UAnimInstance;

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
	
	// 사용할 무기의 데이터
	UPROPERTY(EditDefaultsOnly, Category="Combat|Data")
	TObjectPtr<UWeaponDataAsset> CurrentWeaponData;

	// 손에 붙어있는 실제 무기 메쉬(컴포넌트)
	UPROPERTY(VisibleAnywhere, Category="Combat|Weapon")
	TObjectPtr<USkeletalMeshComponent> EquippedWeaponComp;
	
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
	
	// ===== Sword 어택구현용 =====
public:
	UFUNCTION(BlueprintCallable, Category="Combat|Combo")
    void OpenComboWindow();
    
    UFUNCTION(BlueprintCallable, Category="Combat|Combo")
    void CloseComboWindow();
	
private:
	// 타이밍 클릭 콤보용
	bool bComboWindowOpen = false;     // 지금 입력 받아도 되는 구간인지

	// 다음 섹션으로 넘기는 공통 처리
	void TryAdvanceSwordCombo(UAnimInstance* AnimInst, UAnimMontage* Montage);

	// =====
	
	bool IsAttackMontagePlaying() const;	// 몽타주가 플레이 중인지
	
	// ===== 무기 붙이기용 함수 =====
	void AttachWeapon(UWeaponDataAsset* WeaponDA);
	void ClearWeapon();	// 무기 제거(visual)
	void ApplyAnimLayer(UWeaponDataAsset* WeaponDA);	// 애님레이어 적용
	void RestoreUnarmedAnimLayer();	// UnarmedLayer
	
public:
	UFUNCTION(BlueprintCallable, Category="Combat|Weapon")
	void EquipWeaponData(UWeaponDataAsset* NewWeaponData);
	
	// 현재 무기의 타입 반환
	EWeaponType GetCurrentWeaponType() const;
	
private:
	// 무기 타입별 입력 처리 함수
	void HandleUnarmedAttack(bool bPressed);
	void HandleSwordAttack(bool bPressed);
	void HandleAssaultRifleAttack(bool bPressed);
	
	// 공격 시작
	void StartAttack();

	UAnimMontage* GetCurrentAttackMontage() const;
	
	// 몽타주 종료 타이밍 콜백
	UFUNCTION()
	void OnAttackMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	
	// 섹션 유효성 체크
	bool IsComboSectionValid(UAnimMontage* Montage, const FName& SectionName) const;
};
