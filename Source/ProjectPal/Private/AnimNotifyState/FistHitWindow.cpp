// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/FistHitWindow.h"

#include "Character/Player/CombatComponent.h"
#include "Component/PalStatComponent.h"
#include "Component/PlayerStatComponent.h"
#include "GameFramework/FactionFunctionLibrary.h"

void UFistHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	HitActors.Reset();
}

void UFistHitWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	PerformHitCheck(MeshComp);
}

void UFistHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	HitActors.Reset();
}

void UFistHitWindow::PerformHitCheck(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UWorld* World = Owner->GetWorld();
	if (!World) return;

	// ============================
	// 1. 소켓 위치 계산
	// ============================

	const FVector Start =
		MeshComp->DoesSocketExist(HandSocketName)
		? MeshComp->GetSocketLocation(HandSocketName)
		: MeshComp->GetComponentLocation();

	const FVector End = Start + Owner->GetActorForwardVector() * Range;

	// ============================
	// 2. Trace 설정
	// ============================

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FistHitTrace), false, Owner);

	TArray<FHitResult> Hits;
	const bool bHit = World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,   // 팰이 Pawn/Character라면 OK
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	if (bDrawDebug)
	{
		const FColor Col = bHit ? FColor::Red : FColor::Green;
		DrawDebugSphere(World, Start, Radius, 16, Col, false, 0.1f);
		DrawDebugLine(World, Start, End, Col, false, 0.1f, 0, 1.5f);
	}

	if (!bHit)
		return;

	// ============================
	// 3. 플레이어 스탯 + 무기 데미지 계산
	// ============================

	UPlayerStatComponent* PlayerStat =
		Owner->FindComponentByClass<UPlayerStatComponent>();

	UCombatComponent* CombatComp =
		Owner->FindComponentByClass<UCombatComponent>();

	const float PlayerAttack =
		PlayerStat ? PlayerStat->GetAttack() : 0.f;

	const float WeaponDamage =
		CombatComp ? CombatComp->GetCurrentWeaponDamage() : 0.f;

	const float FinalDamage =
		(PlayerAttack * WeaponDamage) * DamageScale;

	// ============================
	// 4. 히트 대상 처리
	// ============================

	for (const FHitResult& HR : Hits)
	{
		AActor* Other = HR.GetActor();
		if (!Other || Other == Owner)
			continue;
		UE_LOG(LogTemp, Warning, TEXT("[FistHit] HitActor=%s"), *GetNameSafe(Other));
		
		if (HitActors.Contains(Other))
			continue;

		if (!UFactionFunctionLibrary::AreHostile(Owner, Other))
		{
			continue; // ✅ 같은 진영(아군)이면 스킵
		}
		
		if (UPalStatComponent* PalStat =
			Other->FindComponentByClass<UPalStatComponent>())
		{
			UE_LOG(LogTemp, Warning, TEXT("[FistHit] PalStat=%s"), *GetNameSafe(PalStat));
			PalStat->ApplyDamage(FinalDamage);
			UE_LOG(LogTemp, Warning, TEXT("[FistHit] ApplyDamage=%.2f"), FinalDamage);
			HitActors.Add(Other);

			if (!bAllowMultiTarget)
			{
				return;
			}
		}
	}
}
