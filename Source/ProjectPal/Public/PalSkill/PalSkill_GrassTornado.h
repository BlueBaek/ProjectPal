// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PalSkill/PalSkillExecution.h"
#include "PalSkill_GrassTornado.generated.h"

class APJ_GrassTornado;

/**
 * Grass Tornado 실행 로직
 * - PrepareTime 동안 시전자 양 옆에 토네이도 2개를 생성(대기)
 * - PrepareTime 이후 Activate 호출 시, 발사 순간 타겟 위치를 스냅샷으로 잡고
 *   고정 방향/고정 속도로 직진 (호밍 없음)
 */
UCLASS()
class PROJECTPAL_API UPalSkill_GrassTornado : public UPalSkillExecution
{
	GENERATED_BODY()
	
public:
	virtual bool StartPrepare(class UPalSkillComponent* SkillComp, AActor* Caster, AActor* Target, const class UPalSkillDataAsset* SkillData) override;
	virtual bool Activate(class UPalSkillComponent* SkillComp, AActor* Caster, AActor* Target, const class UPalSkillDataAsset* SkillData) override;

protected:
	// ✅ GrassTornado 전용 옵션은 여기로 이동
	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	TSubclassOf<class APJ_GrassTornado> TornadoClass;

	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float SideOffset = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float MoveSpeed = 220.f;

	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float LifeTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float DamagePerTick = 6.f;

	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float DamageInterval = 0.2f;
	
private:
	UPROPERTY()
	TObjectPtr<APJ_GrassTornado> TornadoL;

	UPROPERTY()
	TObjectPtr<APJ_GrassTornado> TornadoR;
};
