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
	
	// 간단한 연타/중복 재생 방지용
	bool bAttackPressed = false;

	void StartAttack();
	// void StopAttack();

	UAnimMontage* GetCurrentAttackMontage() const;
	
	// 몽타주 종료 타이밍 콜백
	UFUNCTION()
	void OnAttackMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
};
