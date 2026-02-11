// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PalSkill/PalSkillExecution.h"
#include "PalSkill_GrassTornado.generated.h"

class APJ_GrassTornado;

UCLASS()
class PROJECTPAL_API UPalSkill_GrassTornado : public UPalSkillExecution
{
	GENERATED_BODY()
	
public:
	// Prepare 시작 시: Super로 공통 몽타주(Prepare/Loop) 처리 + 토네이도 스폰
	virtual bool StartPrepare(APalCharacter* InCaster, AActor* InTarget, const UPalSkillDataAsset* InSkillData) override;

	// SkillAction 몽타주에서 SkillFire Notify가 오면 호출 → 실제 발사
	virtual void OnSkillFire() override;

protected:
	// 어떤 투사체를 스폰할지 (BP에서 지정)
	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	TSubclassOf<APJ_GrassTornado> TornadoClass;

	// 스폰 위치 오프셋(캐릭터 앞/위)
	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float SpawnForwardOffset = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float SpawnUpOffset = 20.f;

	// 투사체 이동 속도(고정값. 필요하면 DataAsset로 빼도 됨)
	UPROPERTY(EditDefaultsOnly, Category="GrassTornado")
	float MoveSpeed = 600.f;

	// Prepare 중에 스폰해둔 투사체를 OnSkillFire 때 Activate하기 위해 캐시
	UPROPERTY()
	TObjectPtr<APJ_GrassTornado> SpawnedTornado = nullptr;

private:
	APJ_GrassTornado* SpawnTornadoIfNeeded();
};
