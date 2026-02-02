// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/WeaponType.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

class USkeletalMesh;
class UAnimMontage;
class UAnimInstance;

UCLASS()
class PROJECTPAL_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 무기 공격 입력 방식
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	EWeaponType WeaponType = EWeaponType::Unarmed;
	
	UPROPERTY(EditAnywhere, Category = "Visuals")
	USkeletalMesh* WeaponMesh; // 맨손일 경우 NULL
	
	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* AttackMontage; // 공격 모션이 담긴 몽타주

	// 무기별 Anim Layer Class
	UPROPERTY(EditAnywhere, Category = "Animations")
	TSubclassOf<UAnimInstance> AnimLayerClass; // 이동/대기 애니메이션 레이어
	// TSubclassOf<UAnimLayerInterface> AnimLayer;
};
