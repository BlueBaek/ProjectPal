// Fill out your copyright notice in the Description page of Project Settings.


#include "PalSkill/PalSkill_GrassTornados.h"

#include "Character/Pal/PalCharacter.h"
#include "DataAsset/PalSkillDataAsset.h"
#include "Projectile/PJ_GrassTornado.h"

bool UPalSkill_GrassTornados::StartPrepare(APalCharacter* InCaster, AActor* InTarget,
                                           const UPalSkillDataAsset* InSkillData)
{
	// 공통 파이프라인(Prepare/PrepareLoop 재생)은 부모가 처리
	if (!Super::StartPrepare(InCaster, InTarget, InSkillData))
	{
		return false;
	}

	CachedDamage = DamageCompute(InSkillData);

	// Prepare 단계에서 미리 2개 스폰(아직 bActivated=false → 회전 연출)
	SpawnTornado(-1.f); // Left
	SpawnTornado(+1.f); // Right

	return true;
}

void UPalSkill_GrassTornados::OnSkillFire()
{
	// SkillAction 몽타주 중간(Notify)에서 호출됨
	APJ_GrassTornado* Left = SpawnedLeftTornado ? SpawnedLeftTornado.Get() : SpawnTornado(-1.f);
	APJ_GrassTornado* Right = SpawnedRightTornado ? SpawnedRightTornado.Get() : SpawnTornado(+1.f);

	if (Left)
	{
		Left->Activate();
	}
	if (Right)
	{
		Right->Activate();
	}
}

APJ_GrassTornado* UPalSkill_GrassTornados::SpawnTornado(float SideSign)
{
	// 캐시가 있으면 반환
	if (SideSign < 0.f)
	{
		if (SpawnedLeftTornado) return SpawnedLeftTornado.Get();
	}
	else
	{
		if (SpawnedRightTornado) return SpawnedRightTornado.Get();
	}

	if (!Caster || !SkillData)
	{
		return nullptr;
	}

	if (!TornadoClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GrassTornados] TornadoClass is null. Set it in BP/Defaults."));
		return nullptr;
	}

	UWorld* World = Caster->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// ✅ 스폰 위치: 팰 앞 + 위 + 좌/우(오른쪽 벡터 기준)
	const FVector SpawnLoc =
		Caster->GetActorLocation()
		+ Caster->GetActorForwardVector() * SpawnForwardOffset
		+ Caster->GetActorRightVector() * (SpawnSideOffset * SideSign)
		+ FVector(0.f, 0.f, SpawnUpOffset);

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

	// ✅ 투사체 파라미터를 DataAsset에서 가져오기
	const float LifeTime = FMath::Max(0.f, SkillData->Timing.Duration);
	const float Interval = (SkillData->Damage.DamageInterval <= 0.f) ? 0.2f : SkillData->Damage.DamageInterval;
	const float DamagePerTick = FMath::Max(0.f, CachedDamage);

	Tornado->InitTornado(
		Caster,
		Target.Get(),
		MoveSpeed,
		LifeTime,
		DamagePerTick,
		Interval
	);

	// 캐시 저장
	if (SideSign < 0.f)
	{
		SpawnedLeftTornado = Tornado;
	}
	else
	{
		SpawnedRightTornado = Tornado;
	}

	return Tornado;
}
