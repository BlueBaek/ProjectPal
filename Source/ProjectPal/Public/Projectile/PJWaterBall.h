// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PJWaterBall.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class PROJECTPAL_API APJWaterBall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APJWaterBall();

	// 스킬에서 스폰 직후 호출(필요 시)
	void InitProjectile(AActor* InOwnerActor, AActor* InTargetActor, float InDamage);

	void SetVelocityDirection(const FVector& Dir);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 비행 중 보여줄 캐스케이드 파티클(붙어있는 형태)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UParticleSystemComponent> FlightPSC;

	// 사라질 때(히트/제거) 재생할 캐스케이드
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UParticleSystem> ImpactVFX;

	// 비행 VFX 템플릿(원하면 BP에서 교체)
	UPROPERTY(EditDefaultsOnly, Category="VFX")
	TObjectPtr<UParticleSystem> FlightVFX;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	float LifeSeconds = 5.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	float Speed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	float SphereRadius = 20.f;

	// 고속 투사체가 얇은 벽을 뚫는 느낌이면 값을 올려주면 안정적(너무 크면 비용↑)
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	float MaxSimulationTimeStep = 0.016f;   // 60fps step

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	int32 MaxSimulationIterations = 8;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

private:
	UFUNCTION()
	void OnCollisionHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	void ExplodeAndDestroy(const FHitResult& Hit);

};
