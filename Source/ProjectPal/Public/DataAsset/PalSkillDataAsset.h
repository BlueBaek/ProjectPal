// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PalSkillDataAsset.generated.h"

/**
 * 
 */

enum class EPalType : uint8;
class UPalSkillExecution;

UENUM(BlueprintType)
enum class EPalSkillTargetType : uint8
{
	None        UMETA(DisplayName="None"),
	Enemy       UMETA(DisplayName="Enemy"),
	Self        UMETA(DisplayName="Self"),
	Location    UMETA(DisplayName="Location"),
};

USTRUCT(BlueprintType)
struct FPalSkillTiming
{
	GENERATED_BODY()

	// 발동 준비(캐스팅) 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0"))
	float PrepareTime = 0.0f;

	// 팰이 해당 스킬을 발동시키고 모션을 취하고 있는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0"))
	float Duration = 0.0f;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Timing", meta=(ClampMin="0.0"))
	float Cooldown = 1.0f;
};

USTRUCT(BlueprintType)
struct FPalSkillDamage
{
	GENERATED_BODY()

	// 기본 피해량(레벨 시스템 없으면 이 값만 써도 됨)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0"))
	float BaseDamage = 10.f;

	// 틱 피해형(도트)일 때의 간격. 0이면 단발로 취급 가능.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage", meta=(ClampMin="0.0"))
	float DamageInterval = 0.0f;

	// (옵션) 데미지 타입
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	// TSubclassOf<UDamageType> DamageTypeClass;
	
};

USTRUCT(BlueprintType)
struct FPalSkillActivation
{
	GENERATED_BODY()

	// “지정한 적과 거리만큼 이동해야 발동”을 위한 최소 이동 거리(0이면 제한 없음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Activation", meta=(ClampMin="0.0"))
	float RequiredMoveDistanceToCast = 0.0f;

	// 스킬 사용 가능 거리(시전자 ↔ 타겟). 0이면 제한 없음
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Activation", meta=(ClampMin="0.0"))
	float CastRange = 0.0f;
};

UCLASS()
class PROJECTPAL_API UPalSkillDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 스킬 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	FName SkillId = NAME_None;

	// 표시 이름/설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	FText DisplayName;

	// 스킬 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill", meta=(MultiLine=true))
	FText Description;

	// 스킬 속성
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill|Attribute")
	EPalType SkillType;
	
	// 발동 조건/범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	FPalSkillActivation Activation;

	// 시간(준비/지속/쿨)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	FPalSkillTiming Timing;

	// 피해 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill")
	FPalSkillDamage Damage;
	
	// ✅ 실행 클래스: 이 DataAsset을 어떻게 실행할지
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill")
	TSubclassOf<class UPalSkillExecution> ExecutionClass;
};
