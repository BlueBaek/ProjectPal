// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PJ_PalSphere.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTPAL_API APJ_PalSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APJ_PalSphere();
	
	void InitVelocity(const FVector& Velocity);
	void SetOwnerIgnoreCollision(AActor* InOwner);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereComp;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	// UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* SkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileMovement;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	// 적절한 대상이 아니면 Hit 후 3초 후 삭제시킴
	UPROPERTY(EditDefaultsOnly, Category="PalSphere")
	float MissDestroyDelay = 3.0f;
};
