// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/PJ_PalSphere.h"

#include "Character/Pal/PalCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
APJ_PalSphere::APJ_PalSphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SphereComp->InitSphereRadius(7.2f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComp->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	
	// Hit event를 발생시킨다.
	SphereComp->SetNotifyRigidBodyCollision(true);
	RootComponent = SphereComp;

	//SphereComponent에 OnHit함수 딜리게이트 연결
	SphereComp->OnComponentHit.AddDynamic(this, &APJ_PalSphere::OnHit);
	
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereComp;
	ProjectileMovement->InitialSpeed = 1600.f;
	ProjectileMovement->MaxSpeed = 1600.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.25f;	// 탄성
	ProjectileMovement->Friction = 0.2f;             // 미끄러짐
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 10.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 1.0f;

	SphereComp->OnComponentHit.AddDynamic(this, &APJ_PalSphere::OnHit);
}

// Called when the game starts or when spawned
void APJ_PalSphere::BeginPlay()
{
	Super::BeginPlay();
	
}

void APJ_PalSphere::InitVelocity(const FVector& Velocity)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Velocity;
		ProjectileMovement->Activate(true);
	}
}

void APJ_PalSphere::SetOwnerIgnoreCollision(AActor* InOwner)
{
	if (!InOwner) return;
	SetOwner(InOwner);
	SphereComp->IgnoreActorWhenMoving(InOwner, true);
}

void APJ_PalSphere::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;

	// ✅ 팰에 맞았는지 판정
	if (OtherActor->IsA(APalCharacter::StaticClass()))
	{
		// TODO: 포획 로직 시작
		return;
	}

	// ✅ 팰이 아니면 3초 뒤 자동 소멸
	SetLifeSpan(MissDestroyDelay);
}