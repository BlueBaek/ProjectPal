// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/WeaponAction.h"
#include "Engine/DataAsset.h"
#include "WeaponAttackData.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct FWeaponActionMontage
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EWeaponAction Action = EWeaponAction::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName StartSection = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PlayRate = 1.0f;
};

UCLASS()
class PROJECTPAL_API UWeaponAttackData : public UDataAsset
{
	GENERATED_BODY()

};
