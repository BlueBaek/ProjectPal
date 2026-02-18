// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/PJWaterBall.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
APJWaterBall::APJWaterBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(SphereRadius);

	// 프로젝트에서 쓰는 Collision Preset이 있으면 그걸로 교체 추천
	// 예: Projectile / OverlapAllDynamic 등
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->SetNotifyRigidBodyCollision(true); // Hit 이벤트
	Collision->SetGenerateOverlapEvents(false);

	Collision->OnComponentHit.AddDynamic(this, &APJWaterBall::OnCollisionHit);

	FlightPSC = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FlightPSC"));
	FlightPSC->SetupAttachment(RootComponent);
	FlightPSC->bAutoActivate = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	// 고속 탄환 안정성(프레임당 이동 거리가 커도 Sweep/Hit 잘 나게)
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->MaxSimulationTimeStep = MaxSimulationTimeStep;
	ProjectileMovement->MaxSimulationIterations = MaxSimulationIterations;

	// 기본 수명
	InitialLifeSpan = 0.f; // BeginPlay에서 SetLifeSpan 사용
}

// Called when the game starts or when spawned
void APJWaterBall::BeginPlay()
{
	Super::BeginPlay();
	
	// BP에서 VFX 지정하면 자동 적용
	if (FlightVFX)
	{
		FlightPSC->SetTemplate(FlightVFX);
		FlightPSC->Activate(true);
	}

	// 반경 런타임 반영
	Collision->SetSphereRadius(SphereRadius, true);

	// Speed 런타임 반영
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;

	SetLifeSpan(LifeSeconds);
}

void APJWaterBall::InitProjectile(AActor* InOwnerActor, AActor* InTargetActor, float InDamage)
{
	OwnerActor = InOwnerActor;
	TargetActor = InTargetActor;
	Damage = InDamage;

	// Instigator 설정(데미지/팀판정에 유용)
	if (APawn* PawnOwner = Cast<APawn>(InOwnerActor))
	{
		SetInstigator(PawnOwner);
	}
	SetOwner(InOwnerActor);
	
	// ✅ 오너는 이동/스윕 충돌에서 무시 (중요)
	if (Collision && InOwnerActor)
	{
		Collision->IgnoreActorWhenMoving(InOwnerActor, true);
		Collision->MoveIgnoreActors.AddUnique(InOwnerActor);
	}
}

void APJWaterBall::SetVelocityDirection(const FVector& Dir)
{
	if (ProjectileMovement)
	{
		const FVector N = Dir.GetSafeNormal();
		ProjectileMovement->Velocity = N * ProjectileMovement->InitialSpeed;
	}
}

void APJWaterBall::OnCollisionHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                  FVector NormalImpulse, const FHitResult& Hit)
{
	// 자기 자신/오너는 무시
	if (!OtherActor || OtherActor == this || OtherActor == OwnerActor)
	{
		return;
	}

	// 여기서 데미지 처리까지 할 수도 있고(스킬 설계에 따라), 지금은 “히트하면 사라짐”만
	ExplodeAndDestroy(Hit);
}

void APJWaterBall::ExplodeAndDestroy(const FHitResult& Hit)
{
	// 1) 임팩트 VFX(캐스케이드)
	if (ImpactVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactVFX,
			Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation()
		);
	}

	// 2) 비행 파티클은 즉시 끄기(끊김 방지)
	if (FlightPSC)
	{
		FlightPSC->Deactivate();
	}

	// 3) 충돌 끄고 바로 제거
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Destroy();
}

