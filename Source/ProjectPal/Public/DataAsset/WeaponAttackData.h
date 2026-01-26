// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponAttackData.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EHitDetectType : uint8
{
	None        UMETA(DisplayName="None"),
	SweepTrace  UMETA(DisplayName="Sweep Trace"),
	Overlap     UMETA(DisplayName="Overlap"),
};

USTRUCT(BlueprintType)
struct FDamageProfile
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float StaminaDamage = 0.f;

	// 필요하면 데미지 타입(물리/화염 등)이나 GameplayEffect를 붙여도 됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	TSubclassOf<UDamageType> DamageType;
};

USTRUCT(BlueprintType)
struct FHitTraceProfile
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitDetect")
	EHitDetectType DetectType = EHitDetectType::SweepTrace;

	// 무기 메시의 소켓 기반 트레이스(예: Tip -> Root)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitDetect", meta=(EditCondition="DetectType==EHitDetectType::SweepTrace"))
	FName TraceStartSocket = "TraceStart";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitDetect", meta=(EditCondition="DetectType==EHitDetectType::SweepTrace"))
	FName TraceEndSocket = "TraceEnd";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitDetect", meta=(EditCondition="DetectType==EHitDetectType::SweepTrace"))
	float TraceRadius = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitDetect")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;
};

UCLASS()
class PROJECTPAL_API UWeaponAttackData : public UDataAsset
{
	GENERATED_BODY()
	
};
