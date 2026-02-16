// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FactionFunctionLibrary.generated.h"

enum class ECombatFaction : uint8;
/**
 * 진영 판정 관련 공용 유틸 함수 모음
 * - 어디서든(투사체 OnHit, 근접 타격, 스킬 데미지, AI 타겟팅 등)
 *   "서로 적인가?"를 한 줄로 판단하기 위해 만든 라이브러리
 */
UCLASS()
class PROJECTPAL_API UFactionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Actor에서 진영을 얻어온다.
	 *
	 * @param Actor      진영을 알고 싶은 대상
	 * @param OutFaction 성공 시 해당 Actor의 진영이 채워짐
	 * @return           성공(true) / 실패(false)
	 *
	 * 실패 케이스:
	 * - Actor가 nullptr
	 * - Actor가 UFactionInterface를 구현하지 않음
	 *
	 * ✅ 왜 TryGetFaction을 따로 두나?
	 * - AreHostile 같은 판정 함수에서 중복 코드를 줄이기 위해
	 * - "인터페이스 없는 액터"를 어떻게 취급할지 정책을 한 곳에서 관리하기 위해
	 */
	UFUNCTION(BlueprintCallable, Category="Faction")
	static bool TryGetFaction(const AActor* Actor, ECombatFaction& OutFaction);

	/**
	 * 두 Actor가 서로 적대 관계인지 판단한다.
	 *
	 * @return true면 적(서로 다른 진영), false면 적대 아님(같은 진영 or 판정 불가)
	 *
	 * ✅ 지금(2진영) 기준 규칙:
	 * - Player vs Wild => Hostile(true)
	 * - Player vs Player => Friendly(false)
	 * - Wild vs Wild => Friendly/중립 취급(false)  (야생끼리 서로 싸우게 하려면 정책 변경 가능)
	 *
	 * ⚠️ 인터페이스 없는 액터 처리 정책:
	 * - 현재 구현은 "판정 불가면 false(적대 아님)"로 안전하게 둠
	 *   (원하면 '판정 불가면 true'로 바꿔서 무엇이든 공격 가능하게도 가능)
	 */
	UFUNCTION(BlueprintCallable, Category="Faction")
	static bool AreHostile(const AActor* A, const AActor* B);
};
