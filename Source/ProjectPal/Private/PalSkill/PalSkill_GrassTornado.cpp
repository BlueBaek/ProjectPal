// Fill out your copyright notice in the Description page of Project Settings.


#include "PalSkill/PalSkill_GrassTornado.h"

#include "Component/PalSkillComponent.h"
#include "Projectile/PJ_GrassTornado.h"

bool UPalSkill_GrassTornado::StartPrepare(class UPalSkillComponent* SkillComp, AActor* Caster, AActor* Target,
                                          const class UPalSkillDataAsset* SkillData)
{
	if (!SkillComp || !Caster) return false;

	UWorld* World = Caster->GetWorld();
	if (!World) return false;

	if (!TornadoClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[GrassTornado] TornadoClass is null. Set it in BP_PalSkill_GrassTornado."));
		return false;
	}

	const FVector CasterLoc = Caster->GetActorLocation();
	const FRotator CasterRot = Caster->GetActorRotation();
	const FVector Right = CasterRot.RotateVector(FVector::RightVector);

	const FVector SpawnL = CasterLoc - Right * SideOffset;
	const FVector SpawnR = CasterLoc + Right * SideOffset;

	FActorSpawnParameters Params;
	Params.Owner = Caster;
	Params.Instigator = Cast<APawn>(Caster);
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TornadoL = World->SpawnActor<APJ_GrassTornado>(TornadoClass, SpawnL, CasterRot, Params);
	TornadoR = World->SpawnActor<APJ_GrassTornado>(TornadoClass, SpawnR, CasterRot, Params);
	if (!TornadoL || !TornadoR) return false;

	TornadoL->InitTornado(Caster, Target, MoveSpeed, LifeTime, DamagePerTick, DamageInterval);
	TornadoR->InitTornado(Caster, Target, MoveSpeed, LifeTime, DamagePerTick, DamageInterval);
	return true;
}

bool UPalSkill_GrassTornado::Activate(class UPalSkillComponent* SkillComp, AActor* Caster, AActor* Target,
	const class UPalSkillDataAsset* SkillData)
{
	if (IsValid(TornadoL)) TornadoL->Activate();
	if (IsValid(TornadoR)) TornadoR->Activate();
	return Super::Activate(SkillComp, Caster, Target, SkillData);
}
