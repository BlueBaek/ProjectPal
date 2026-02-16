// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PJ_GrassTornado.generated.h"

class USphereComponent;
class UParticleSystem;
class UParticleSystemComponent;
class UStaticMeshComponent;

UCLASS()
class PROJECTPAL_API APJ_GrassTornado : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APJ_GrassTornado();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// 스폰 직후 초기화(시전자/타겟/속도/수명/도트 설정)
	void InitTornado(
		AActor* InCaster,
		AActor* InTargetActor,
		float InMoveSpeed,
		float InLifeTime,
		float InDamagePerTick,
		float InDamageInterval
	);

	// Prepare 끝난 시점에 호출: "발사" (타겟 당시 위치 스냅샷 + 방향 고정)
	void Activate();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// OverlapBegin 이벤트 처리
	UFUNCTION()
	void OnDamageBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
							 bool bFromSweep, const FHitResult& SweepResult);

	// OverlapEnd 이벤트 처리
	UFUNCTION()
	void OnDamageEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 도트딜 적용
	void ApplyDotDamage();
	
	// Root
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	// 충돌 구체
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> DamageSphere;
	
	// 스킬 이펙트가 캐스케이트 파티클 시스템을 이용
	// 캐스케이드 컴포넌트
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UParticleSystemComponent> ParticleComp;

	// ✅ 에디터에서 지정할 캐스케이드 시스템(UParticleSystem)
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UParticleSystem> TornadoPS;
	
	
	// 스킬 시전자
	UPROPERTY()
	TObjectPtr<AActor> Caster;

	// 공격할 대상
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	// 스킬을 적용할 대상(닿으면 대미지)
	UPROPERTY()
	TSet<TObjectPtr<AActor>> OverlappingActors;

	
	UPROPERTY()
	TMap<TObjectPtr<AActor>, float> NextDamageTimeByActor;

	bool bActivated = false;

	// 파라미터
	float MoveSpeed = 200.f;
	float LifeTime = 5.f;
	float DamagePerTick = 5.f;
	float DamageInterval = 0.2f;

	float Elapsed = 0.f;

	// ✅ 발사 순간 고정되는 값들
	FVector FixedTargetLocation = FVector::ZeroVector;
	FVector FixedDirection = FVector::ForwardVector;
	
};
