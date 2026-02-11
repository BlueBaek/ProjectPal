// Fill out your copyright notice in the Description page of Project Settings.


#include "PalSkill/PalSkill_GrassTornado.h"

#include "Character/Pal/PalCharacter.h"
#include "DataAsset/PalSkillDataAsset.h"
#include "Projectile/PJ_GrassTornado.h"

bool UPalSkill_GrassTornado::StartPrepare(APalCharacter* InCaster, AActor* InTarget,
                                          const UPalSkillDataAsset* InSkillData)
{
	// 공통 파이프라인(Prepare/PrepareLoop 재생)은 부모가 처리
	if (!Super::StartPrepare(InCaster, InTarget, InSkillData))
	{
		return false;
	}
	
	// Prepare 단계에서 미리 스폰 (bActivated=false 상태로 회전 연출됨)
	SpawnTornadoIfNeeded();
	
	return Super::StartPrepare(InCaster, InTarget, InSkillData);
}

void UPalSkill_GrassTornado::OnSkillFire()
{
	// SkillAction 몽타주 중간(Notify)에서 호출됨
	APJ_GrassTornado* Tornado = SpawnedTornado ? SpawnedTornado.Get() : SpawnTornadoIfNeeded();
	if (!Tornado)
	{
		return;
	}

	// 발사 (타겟 위치 스냅샷 + 방향 고정)
	Tornado->Activate();
}

APJ_GrassTornado* UPalSkill_GrassTornado::SpawnTornadoIfNeeded()
{
	if (SpawnedTornado)
	{
		return SpawnedTornado.Get();
	}

	if (!Caster || !SkillData)
	{
		return nullptr;
	}

	if (!TornadoClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GrassTornado] TornadoClass is null. Set it in BP/Defaults."));
		return nullptr;
	}

	UWorld* World = Caster->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 스폰 위치: 팰 앞쪽 + 약간 위
	const FVector SpawnLoc = Caster->GetActorLocation()
		+ Caster->GetActorForwardVector() * SpawnForwardOffset
		+ FVector(0.f, 0.f, 0.f/*SpawnUpOffset*/);

	// 회전은 크게 의미 없지만, 시각적으로 전방을 보게 하고 싶으면 캐릭터 회전 사용
	const FRotator SpawnRot = Caster->GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = Caster;
	Params.Instigator = Caster;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APJ_GrassTornado* Tornado = World->SpawnActor<APJ_GrassTornado>(TornadoClass, SpawnLoc, SpawnRot, Params);
	if (!Tornado)
	{
		return nullptr;
	}

	// ✅ 투사체 파라미터를 DataAsset에서 가져온다
	// - LifeTime: 보통 스킬 지속시간과 동일하게
	const float LifeTime = FMath::Max(0.f, SkillData->Timing.Duration);

	// - DamageInterval: 0이면 PJ 쪽 로직상 "매 프레임급"으로 데미지 들어갈 수 있어서 최소값 보정
	const float Interval = (SkillData->Damage.DamageInterval <= 0.f) ? 0.2f : SkillData->Damage.DamageInterval;

	// - DamagePerTick: BaseDamage를 틱 데미지로 사용(원하면 나중에 별도 값으로 분리 가능)
	const float DamagePerTick = FMath::Max(0.f, SkillData->Damage.BaseDamage);

	Tornado->InitTornado(
		Caster,
		Target.Get(),   // 타겟이 없을 수도 있으니 Get() 사용
		MoveSpeed,
		LifeTime,
		DamagePerTick,
		Interval
	);

	SpawnedTornado = Tornado;
	return Tornado;
}
