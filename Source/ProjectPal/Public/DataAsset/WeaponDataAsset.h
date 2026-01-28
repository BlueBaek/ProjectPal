// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

class UAnimLayerInterface;
/**
 * 
 */
UCLASS()
class PROJECTPAL_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Visuals")
	USkeletalMesh* WeaponMesh; // 맨손일 경우 NULL

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* AttackMontage; // 공격 모션이 담긴 몽타주

	UPROPERTY(EditAnywhere, Category = "Animations")
	TSubclassOf<UAnimLayerInterface> AnimLayer; // 이동/대기 애니메이션 레이어
};
