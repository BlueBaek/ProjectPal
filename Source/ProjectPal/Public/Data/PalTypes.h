// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PalTypes.generated.h"

UENUM(BlueprintType)
enum class EPalType : uint8
{
	None        UMETA(DisplayName="None"),
	Normal      UMETA(DisplayName="Normal"),
	Fire        UMETA(DisplayName="Fire"),
	Water       UMETA(DisplayName="Water"),
	Electric    UMETA(DisplayName="Electric"),
	Earth       UMETA(DisplayName="Earth"),
	Grass       UMETA(DisplayName="Grass"),
	Ice         UMETA(DisplayName="Ice"),
	Dark        UMETA(DisplayName="Dark"),
	Dragon		UMETA(DisplayName="Dragon"),
};