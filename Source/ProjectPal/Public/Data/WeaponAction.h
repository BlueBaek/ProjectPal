// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAction.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EWeaponAction : uint8
{
	None        UMETA(DisplayName="None"),

	// 공격
	Attack		UMETA(DisplayName="Attack"),

	// 상태
	Equip       UMETA(DisplayName="Equip"),
	Unequip     UMETA(DisplayName="Unequip"),

	// 확장용
	Reload      UMETA(DisplayName="Reload"),
};