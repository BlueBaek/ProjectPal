// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "FistHitWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UFistHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:

	/** 사용할 손 소켓 이름 (1타: 오른손, 2타: 왼손) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FistHit")
	FName HandSocketName = TEXT("hand_r");

	/** 공격 판정 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FistHit", meta=(ClampMin="0.0"))
	float Range = 120.f;

	/** 공격 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FistHit", meta=(ClampMin="0.0"))
	float Radius = 35.f;

	/** 데미지 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FistHit", meta=(ClampMin="0.0"))
	float DamageScale = 1.f;

	/** 디버그 시각화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FistHit")
	bool bDrawDebug = false;

	/** 여러 대상 허용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FistHit")
	bool bAllowMultiTarget = true;

	// ======================
	// AnimNotifyState Overrides
	// ======================

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:

	/** 이미 맞춘 대상 저장 (한 윈도우당 1회 타격용) */
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> HitActors;

	void PerformHitCheck(USkeletalMeshComponent* MeshComp);
};
