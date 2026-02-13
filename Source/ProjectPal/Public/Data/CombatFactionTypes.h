// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "CoreMinimal.h"
#include "CombatFactionTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatFaction : uint8
{
	Player UMETA(DisplayName="Player"),
	Wild   UMETA(DisplayName="Wild"),
};
