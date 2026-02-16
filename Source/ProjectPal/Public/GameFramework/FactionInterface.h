// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CombatFactionTypes.h"
#include "UObject/Interface.h"
#include "FactionInterface.generated.h"


/**
 * 전투 대상(플레이어/팰/적 NPC 등)이 "나는 어떤 진영인가?"를 공통적으로 제공하기 위한 인터페이스
 *
 * ✅ 왜 인터페이스?
 * - 데미지 처리/스킬/AI 등 여러 시스템에서
 *   "이 액터의 진영이 뭐야?"를 공통 방식으로 묻기 위함
 * - Actor 종류가 달라도(플레이어든 팰이든) 동일 API로 처리 가능
 */
UINTERFACE(MinimalAPI)
class UFactionInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTPAL_API IFactionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * 이 Actor의 전투 진영을 반환한다.
	 *
	 * - PlayerCharacter: Player 반환
	 * - PalCharacter: PalGroup이 Wild면 Wild, 아니면 Player(테이밍된 팰은 플레이어 편)
	 *
	 * BlueprintNativeEvent로 만든 이유:
	 * - C++ 기본 구현도 가능하고
	 * - 필요하면 BP에서 오버라이드도 가능하게(확장성)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Faction")
	ECombatFaction GetCombatFaction() const;
};
