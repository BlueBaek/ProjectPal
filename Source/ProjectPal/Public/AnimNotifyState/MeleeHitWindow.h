// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MeleeHitWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UMeleeHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	/** 선분 시작(예: 손잡이/가드 근처) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SwordHit")
	FName StartSocketName = TEXT("LeftHandIKSocket");

	/** 선분 끝(예: 칼끝) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SwordHit")
	FName EndSocketName= TEXT("MeleeEffect");
	
	// 반경(검 두께/관대함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SwordHit", meta=(ClampMin="0.0"))
	float TraceRadius = 10.f;

	// 데미지 배율 (블루프린트에서 3타에 1.5배 적용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SwordHit", meta=(ClampMin="0.0"))
	float DamageScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MeleeHit")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MeleeHit")
	bool bAllowMultiTarget = true;

	// 충돌 채널(프로젝트에 맞게 바꿀 수 있게)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MeleeHit")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
							const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
						   const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;

	// 이전 프레임 선분
	FVector PrevStart = FVector::ZeroVector;
	FVector PrevEnd   = FVector::ZeroVector;
	bool bHasPrev = false;

	USkeletalMeshComponent* FindMeshWithSockets(AActor* Owner) const;

	void PerformHitCheck(USkeletalMeshComponent* AnyMeshComp);
	void SweepAndApply(UWorld* World, AActor* Owner, const FVector& From, const FVector& To, float FinalDamage);
};
