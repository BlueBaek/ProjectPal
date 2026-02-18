// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/MeleeHitWindow.h"

#include "Character/Player/CombatComponent.h"
#include "Component/PalStatComponent.h"
#include "Component/PlayerStatComponent.h"
#include "GameFramework/FactionFunctionLibrary.h"

void UMeleeHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	HitActors.Reset();
	bHasPrev = false;

	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	USkeletalMeshComponent* TraceMesh = FindMeshWithSockets(Owner);
	if (!TraceMesh) return;

	PrevStart = TraceMesh->GetSocketLocation(StartSocketName);
	PrevEnd = TraceMesh->GetSocketLocation(EndSocketName);
	bHasPrev = true;
}

void UMeleeHitWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	PerformHitCheck(MeshComp);
}

void UMeleeHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Reset();
	bHasPrev = false;
}

USkeletalMeshComponent* UMeleeHitWindow::FindMeshWithSockets(AActor* Owner) const
{
	if (!Owner) return nullptr;

	TArray<USkeletalMeshComponent*> SkelComps;
	Owner->GetComponents<USkeletalMeshComponent>(SkelComps);

	for (USkeletalMeshComponent* C : SkelComps)
	{
		if (!C) continue;
		if (C->DoesSocketExist(StartSocketName) && C->DoesSocketExist(EndSocketName))
		{
			return C;
		}
	}
	return nullptr;
}

void UMeleeHitWindow::PerformHitCheck(USkeletalMeshComponent* AnyMeshComp)
{
	if (!AnyMeshComp) return;

	AActor* Owner = AnyMeshComp->GetOwner();
	if (!Owner) return;

	UWorld* World = Owner->GetWorld();
	if (!World) return;

	USkeletalMeshComponent* TraceMesh = FindMeshWithSockets(Owner);
	if (!TraceMesh) return;

	const FVector CurStart = TraceMesh->GetSocketLocation(StartSocketName);
	const FVector CurEnd = TraceMesh->GetSocketLocation(EndSocketName);

	// 첫 Tick이면 기준만 세팅
	if (!bHasPrev)
	{
		PrevStart = CurStart;
		PrevEnd = CurEnd;
		bHasPrev = true;
		return;
	}

	// 데미지 계산: Attack * WeaponDamage
	const UPlayerStatComponent* PlayerStat = Owner->FindComponentByClass<UPlayerStatComponent>();
	const UCombatComponent* CombatComp = Owner->FindComponentByClass<UCombatComponent>();

	const float PlayerAttack = PlayerStat ? PlayerStat->GetAttack() : 0.f;
	const float WeaponDamage = CombatComp ? CombatComp->GetCurrentWeaponDamage() : 0.f;
	const float FinalDamage = (PlayerAttack * WeaponDamage) * DamageScale;

	// 스윕 3종: Start 궤적 / End 궤적 / Mid 궤적(빠른 휘두름 보완)
	const FVector PrevMid = (PrevStart + PrevEnd) * 0.5f;
	const FVector CurMid = (CurStart + CurEnd) * 0.5f;

	SweepAndApply(World, Owner, PrevStart, CurStart, FinalDamage);
	SweepAndApply(World, Owner, PrevEnd, CurEnd, FinalDamage);
	SweepAndApply(World, Owner, PrevMid, CurMid, FinalDamage);

	if (bDrawDebug)
	{
		// 현재 선분 표시(검 위치)
		DrawDebugLine(World, CurStart, CurEnd, FColor::Cyan, false, 0.1f, 0, 1.5f);
		DrawDebugSphere(World, CurStart, TraceRadius, 12, FColor::Cyan, false, 0.1f);
		DrawDebugSphere(World, CurEnd, TraceRadius, 12, FColor::Cyan, false, 0.1f);
	}

	// 다음 Tick을 위해 갱신
	PrevStart = CurStart;
	PrevEnd = CurEnd;
}

void UMeleeHitWindow::SweepAndApply(UWorld* World, AActor* Owner, const FVector& From, const FVector& To,
                                    float FinalDamage)
{
	if (!World || !Owner) return;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SwordSegmentHitTrace), false, Owner);

	TArray<FHitResult> Hits;
	const bool bHit = World->SweepMultiByChannel(
		Hits,
		From,
		To,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	if (bDrawDebug)
	{
		const FColor Col = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(World, From, To, Col, false, 0.1f, 0, 1.0f);
	}

	if (!bHit) return;
	
	for (const FHitResult& HR : Hits)
	{
		AActor* Other = HR.GetActor();
		if (!Other || Other == Owner) continue;

		if (HitActors.Contains(Other)) continue;

		if (!UFactionFunctionLibrary::AreHostile(Owner, Other))
		{
			continue; // ✅ 같은 진영(아군)이면 스킵
		}
		
		if (UPalStatComponent* PalStat = Other->FindComponentByClass<UPalStatComponent>())
		{
			PalStat->ApplyDamage(FinalDamage);
			HitActors.Add(Other);

			if (!bAllowMultiTarget)
				return;
		}
	}
}
