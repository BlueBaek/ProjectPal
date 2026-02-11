// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/PJ_GrassTornado.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
APJ_GrassTornado::APJ_GrassTornado()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(Root);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.5f));
	VisualMesh->SetHiddenInGame(false);
	
	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(Root);
	DamageSphere->SetSphereRadius(120.f);
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetCollisionObjectType(ECC_WorldDynamic);
	DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	
	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->SetupAttachment(Root);
	ParticleComp->bAutoActivate = true;               // 자동 재생
	ParticleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ParticleComp->SetHiddenInGame(false);
	
	DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &APJ_GrassTornado::OnDamageBeginOverlap);
	DamageSphere->OnComponentEndOverlap.AddDynamic(this, &APJ_GrassTornado::OnDamageEndOverlap);
}

// Called when the game starts or when spawned
void APJ_GrassTornado::BeginPlay()
{
	Super::BeginPlay();
	
	// ✅ 시스템 에셋 연결(필수)
	if (ParticleComp && TornadoPS)
	{
		ParticleComp->SetTemplate(TornadoPS);
		ParticleComp->ActivateSystem(true);
	}
}

// Called every frame
void APJ_GrassTornado::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Elapsed += DeltaTime;
	if (Elapsed >= LifeTime)
	{
		Destroy();
		return;
	}

	if (!bActivated)
	{
		// Prepare 동안은 이동하지 않고 연출만 (회전)
		AddActorWorldRotation(FRotator(0.f, 180.f * DeltaTime, 0.f));
		return;
	}

	// ✅ 호밍 없음: 방향/속도 고정 직진
	SetActorLocation(GetActorLocation() + FixedDirection * MoveSpeed * DeltaTime, true);

	ApplyDotDamage();
}

void APJ_GrassTornado::InitTornado(AActor* InCaster, AActor* InTargetActor, float InMoveSpeed, float InLifeTime,
	float InDamagePerTick, float InDamageInterval)
{
	Caster = InCaster;
	TargetActor = InTargetActor;

	MoveSpeed = InMoveSpeed;
	LifeTime = 5; //
	DamagePerTick = InDamagePerTick;
	DamageInterval = InDamageInterval;
}

void APJ_GrassTornado::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("[Tornado] Activate | %s"), *GetName());
	bActivated = true;

	// ✅ 발사 순간의 타겟 위치를 스냅샷
	FixedTargetLocation = TargetActor ? TargetActor->GetActorLocation() : GetActorLocation();

	// ✅ 현재 위치에서 "그 당시 타겟 위치"로 향하는 방향을 고정
	const FVector From = GetActorLocation();
	FVector Dir = (FixedTargetLocation - From);
	if (!Dir.Normalize())
	{
		// 타겟이 바로 같은 위치면 전방으로라도 진행
		Dir = GetActorForwardVector();
	}
	FixedDirection = Dir;
}

void APJ_GrassTornado::OnDamageBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == Caster)
		return;

	OverlappingActors.Add(OtherActor);
	NextDamageTimeByActor.FindOrAdd(OtherActor) = GetWorld()->GetTimeSeconds(); // 즉시 1틱
}

void APJ_GrassTornado::OnDamageEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	OverlappingActors.Remove(OtherActor);
	NextDamageTimeByActor.Remove(OtherActor);
}

void APJ_GrassTornado::ApplyDotDamage()
{
	if (!GetWorld()) return;
	const float Now = GetWorld()->GetTimeSeconds();

	for (AActor* Victim : OverlappingActors)
	{
		if (!Victim) continue;

		float& NextTime = NextDamageTimeByActor.FindOrAdd(Victim);
		if (Now >= NextTime)
		{
			UGameplayStatics::ApplyDamage(
				Victim,
				DamagePerTick,
				Caster ? Caster->GetInstigatorController() : nullptr,
				Caster,
				nullptr
			);

			NextTime = Now + DamageInterval;
		}
	}
}
