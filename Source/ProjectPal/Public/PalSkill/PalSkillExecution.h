// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PalSkillExecution.generated.h"

// This class does not need to be modified.
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class UPalSkillExecution : public UObject
{
	GENERATED_BODY()
	
public:
	// Prepare 시작: 예) 토네이도 2개를 양옆에 미리 스폰만 해둠(Activate 전)
	virtual bool StartPrepare(class UPalSkillComponent* SkillComp, AActor* Caster, AActor* Target, const class UPalSkillDataAsset* SkillData)
	{
		return true;
	}

	// Prepare 끝난 후 실제 발동: 예) 토네이도 Activate
	virtual bool Activate(class UPalSkillComponent* SkillComp, AActor* Caster, AActor* Target, const class UPalSkillDataAsset* SkillData)
	{
		return true;
	}
};
