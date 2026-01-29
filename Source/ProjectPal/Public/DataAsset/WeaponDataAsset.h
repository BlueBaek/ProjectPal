// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	UPROPERTY(EditAnywhere, Category = "Visuals")
	USkeletalMesh* WeaponMesh; // 맨손일 경우 NULL

	// UPROPERTY(EditAnywhere, Category="Animations")
	// TObjectPtr<UAnimSequenceBase> AimUpperPose; // 무기별 줌(조준) 상체 포즈(가능하면 Additive 권장)
	
	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* AttackMontage; // 공격 모션이 담긴 몽타주

	// 무기별 Anim Layer Class
	UPROPERTY(EditAnywhere, Category = "Animations")
	TSubclassOf<UAnimInstance> AnimLayerClass; // 이동/대기 애니메이션 레이어
	// TSubclassOf<UAnimLayerInterface> AnimLayer;
};
