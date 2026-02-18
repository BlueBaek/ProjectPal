// Fill out your copyright notice in the Description page of Project Settings.


#include "PalSkill/PalSkill_WaterBall.h"

#include "Character/Pal/PalCharacter.h"
#include "Projectile/PJWaterBall.h"

bool UPalSkill_WaterBall::StartPrepare(APalCharacter* InCaster, AActor* InTarget, const UPalSkillDataAsset* InSkillData)
{
	if (!Super::StartPrepare(InCaster, InTarget, InSkillData))
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[WaterBall] StartPrepare Caster=%s Target=%s"),
		*GetNameSafe(Caster), *GetNameSafe(InTarget));
	
	// ✅ “발사될 때 대상이 있던 장소” = Prepare 시작 시점의 위치 스냅샷
	if (Target)
	{
		CachedAimLocation = Target->GetActorLocation();
	}
	else
	{
		// 타겟이 없으면 정면으로 (혹은 Caster 전방 일정 거리)
		CachedAimLocation = Caster->GetActorLocation() + Caster->GetActorForwardVector() * 1000.f;
	}

	// 데미지 계산(너 기존 GrassTornado처럼 쓰던 방식 유지)
	CachedDamage = DamageCompute(InSkillData);

	return true;
}

void UPalSkill_WaterBall::OnSkillFire()
{
	if (!Caster || !SkillData)
	{
		return;
	}

	// Notify 시점에는 타겟이 이미 없을 수도 있으니, 스냅샷만 사용
	SpawnProjectile(CachedAimLocation);
}

APJWaterBall* UPalSkill_WaterBall::SpawnProjectile(const FVector& AimLocation) const
{
	UE_LOG(LogTemp, Warning, TEXT("[WaterBall] OnSkillFire Caster=%s ProjectileClass=%s Aim=%s"),
		*GetNameSafe(Caster),
		*GetNameSafe(ProjectileClass ? ProjectileClass.Get() : nullptr),
		*CachedAimLocation.ToString());
	
	if (!Caster || !SkillData)
	{
		return nullptr;
	}

	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaterBall] ProjectileClass is null. Set it in BP/Defaults."));
		return nullptr;
	}

	UWorld* World = Caster->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector SpawnLoc =
		Caster->GetActorLocation()
		+ Caster->GetActorForwardVector() * SpawnForwardOffset
		+ FVector(0.f, 0.f, SpawnUpOffset);

	const FVector Dir = (AimLocation - SpawnLoc).GetSafeNormal();
	const FRotator SpawnRot = Dir.Rotation();

	FActorSpawnParameters Params;
	Params.Owner = Caster;
	Params.Instigator = Caster;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APJWaterBall* Proj = World->SpawnActor<APJWaterBall>(ProjectileClass, SpawnLoc, SpawnRot, Params);
	if (!Proj)
	{
		return nullptr;
	}
	
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaterBall] ProjectileClass is null. Set it in BP/Defaults."));
		return nullptr;
	}
	
	
	// ✅ 투사체에 오너/타겟(선택) / 데미지 세팅
	// 타겟은 “스냅샷 위치로 쏘는” 요구사항이라 필수는 아님(호밍X).
	Proj->InitProjectile(Caster, /*InTarget*/ nullptr, CachedDamage);

	// ✅ 호밍 없이: 발사 방향으로 속도만 설정
	// PJWaterBall 내부가 UProjectileMovementComponent를 가지고 있다고 가정.
	// (이전 투사체 구현처럼)
	Proj->SetVelocityDirection(Dir);

	return Proj;
}
