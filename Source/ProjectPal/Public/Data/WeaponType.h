// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponType.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Unarmed			UMETA(DisplayName = "Unarmed"),
	Sword 			UMETA(DisplayName = "Sword"),
	AssaultRifle	UMETA(DisplayName = "AssaultRifle")
};