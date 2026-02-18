// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PalSkill/PalSkillExecution.h"
#include "PalSkill_WaterBall.generated.h"

class APJWaterBall;

UCLASS()
class PROJECTPAL_API UPalSkill_WaterBall : public UPalSkillExecution
{
	GENERATED_BODY()
	
public:
	virtual bool StartPrepare(APalCharacter* InCaster, AActor* InTarget, const UPalSkillDataAsset* InSkillData) override;
	virtual void OnSkillFire() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="WaterBall")
	TSubclassOf<APJWaterBall> ProjectileClass;

	// 스폰 위치 튜닝 (팰 앞/위)
	UPROPERTY(EditDefaultsOnly, Category="WaterBall")
	float SpawnForwardOffset = 80.f;

	UPROPERTY(EditDefaultsOnly, Category="WaterBall")
	float SpawnUpOffset = 40.f;

	// 타겟 위치 대신 “타겟의 당시 위치”로 쏘기 위한 스냅샷
	UPROPERTY(VisibleAnywhere, Category="WaterBall")
	FVector CachedAimLocation = FVector::ZeroVector;

private:
	APJWaterBall* SpawnProjectile(const FVector& AimLocation) const;
};
