// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/FactionFunctionLibrary.h"

#include "GameFramework/FactionInterface.h"

bool UFactionFunctionLibrary::TryGetFaction(const AActor* Actor, ECombatFaction& OutFaction)
{
	// 1) nullptr 방어
	if (!Actor) return false;

	// 2) 인터페이스 구현 여부 확인
	//    - ImplementsInterface는 "이 클래스가 인터페이스를 구현했는지" 체크
	if (Actor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
	{
		// 3) Execute_XXX 형태로 호출해야 BP/C++ 구현 모두를 안전하게 호출 가능
		//    (Actor가 BP에서 오버라이드 했을 수도 있기 때문)
		OutFaction = IFactionInterface::Execute_GetCombatFaction(const_cast<AActor*>(Actor));
		return true;
	}

	// 4) 인터페이스 미구현 => 진영 판정 실패
	return false;
}

bool UFactionFunctionLibrary::AreHostile(const AActor* A, const AActor* B)
{
	ECombatFaction FA, FB;

	const bool bHasA = TryGetFaction(A, FA);
	const bool bHasB = TryGetFaction(B, FB);

	// ✅ 정책: 진영 판정이 안 되는 대상은 "일단 적대 아님"으로 처리
	// - 이유: 실수로 환경 오브젝트(문/벽/아이템) 같은 것에 데미지가 들어가는 걸 방지
	// - 필요하면 여기 정책을 바꿔도 됨
	if (!bHasA || !bHasB)
	{
		return false;
	}

	// 2진영 기준: 서로 다른 진영이면 적대
	return FA != FB;

}
