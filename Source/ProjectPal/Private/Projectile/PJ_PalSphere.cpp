// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/PJ_PalSphere.h"

#include "Character/Pal/PalCharacter.h"
#include "Character/Player/PlayerCharacter.h"
#include "Component/OwnedPalComponent.h"
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
	ProjectileMovement->Bounciness = 0.25f; // 탄성
	ProjectileMovement->Friction = 0.2f; // 미끄러짐
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 10.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
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
	if (APalCharacter* HitPal = Cast<APalCharacter>(OtherActor))
	{
		// ✅ Wild만 포획 가능
		if (HitPal->GetPalGroup() != EPalGroup::Wild)
		{
			SetLifeSpan(MissDestroyDelay);
			return;
		}

		// ✅ 소유자(던진 플레이어) 찾기
		APlayerCharacter* OwnerPlayer = Cast<APlayerCharacter>(GetOwner());
		if (!OwnerPlayer || !OwnerPlayer->GetOwnedPalComponent())
		{
			SetLifeSpan(MissDestroyDelay);
			return;
		}

		// ✅ 슬롯 여유가 있으면 저장(포획 성공)
		const bool bCaptured = OwnerPlayer->GetOwnedPalComponent()->AddPalFromActor(HitPal);
		if (bCaptured)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Capture] Captured Pal: %s"), *HitPal->GetName());
			UE_LOG(LogTemp, Warning, TEXT("[Capture] Total Owned Pals: %d"),
			       OwnerPlayer->GetOwnedPalComponent()->GetOwnedPals().Num());

			// 그룹 변경: Tamed
			HitPal->SetPalGroup(EPalGroup::Tamed);

			// 월드에서 제거(포획됐다고 가정)
			HitPal->Destroy();

			// 스피어도 즉시 제거
			Destroy();
			return;
		}

		// ✅ 슬롯이 꽉 찼으면 실패 처리
		SetLifeSpan(MissDestroyDelay);
		return;
	}

	// ✅ 팰이 아니면 3초 뒤 자동 소멸
	SetLifeSpan(MissDestroyDelay);
}
