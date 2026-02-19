// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Pal/PalCharacter.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Component/PalSkillComponent.h"
#include "Component/PalStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/PalData.h"
#include "DataAsset/PalSkillDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/PalNameplateWidget.h"

// Sets default values
APalCharacter::APalCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PalStatComponent = CreateDefaultSubobject<UPalStatComponent>(TEXT("PalStatComponent"));
	PalSkillComponent = CreateDefaultSubobject<UPalSkillComponent>(TEXT("PalSkillComponent"));
	NameplateWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateWidgetComponent"));

	// 자연스러운 회전
	// 컨트롤러의 회전값이 폰에 즉각 반영되지 않도록 꺼줍니다.
	bUseControllerRotationYaw = false;
	// 가속 방향(이동 방향)으로 몸을 돌리도록 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 회전 속도를 제한
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 200.f);

	// 팰의 기본 움직임 속도 제한
	MoveState = EPalMoveState::Wandering;
	ApplyMoveSpeed();

	// 위젯
	NameplateWidgetComp->SetupAttachment(GetCapsuleComponent());
	NameplateWidgetComp->SetDrawAtDesiredSize(true);
	NameplateWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	// 위치는 대략 머리 위로 (팰마다 다르면 BP에서 조정 가능)
	NameplateWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 160.f));
	// 성능: 너무 멀면 숨기기
	NameplateWidgetComp->SetCullDistance(5000.f);
}

// Called when the game starts or when spawned
void APalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// PalDT를 기준으로 데이터 로딩
	const bool bLoaded = LoadPalData();
	if (!bLoaded)
	{
		UE_LOG(LogTemp, Warning, TEXT("PalCharacter : LoadPalData failed. Stat init skipped. (%s)"), *GetName());
		return;
	}

	// 최초 생성 시 스탯 부여
	if (PalStatComponent) // PalStatComponent 유효성 검사
	{
		PalStatComponent->SetLevel(PalLevel);

		if (PalDT)
		{
			PalStatComponent->InitializeStats(PalDT, PalRowName);
		}

		// 죽었을 때 구독
		PalStatComponent->OnDeath.AddUObject(this, &APalCharacter::HandleDeath);
	}

	// 팰 타입 출력용
	for (int32 i = 0; i < PalTypes.Num(); ++i)
	{
		const EPalType Type = PalTypes[i];

		const UEnum* EnumPtr = StaticEnum<EPalType>();
		if (EnumPtr)
		{
			PalTypeString += EnumPtr->GetNameStringByValue((int64)Type);
		}

		if (i < PalTypes.Num() - 1)
		{
			PalTypeString += TEXT(", ");
		}
	}

	// === 팰 정보 종합 로그 ===
	UE_LOG(LogTemp, Warning,
	       TEXT("[Pal] %s (%s) | Type:%s | HP:%d ATK:%d DEF:%d"),
	       *PalDisplayName.ToString(),
	       *PalName.ToString(),
	       *PalTypeString,
	       PalStatComponent ? FMath::FloorToInt(PalStatComponent->GetMaxHP()) : -1,
	       PalStatComponent ? FMath::FloorToInt(PalStatComponent->GetAttack()) : -1,
	       PalStatComponent ? FMath::FloorToInt(PalStatComponent->GetDefense()) : -1
	);

	// 보유 스킬 추가
	if (UPalSkillComponent* SkillComp = FindComponentByClass<UPalSkillComponent>())
	{
		// InitialSkills의 스킬들을 보유 목록에 추가
		for (UPalSkillDataAsset* SkillDA : InitialSkills)
		{
			if (IsValid(SkillDA))
			{
				SkillComp->LearnSkill(SkillDA);
			}
		}
	}

	// 팰 위젯 추가
	if (NameplateWidgetComp)
	{
		if (UUserWidget* Widget = NameplateWidgetComp->GetUserWidgetObject())
		{
			if (UPalNameplateWidget* Nameplate = Cast<UPalNameplateWidget>(Widget))
			{
				Nameplate->InitFromPal(this);
			}
		}
	}
}

// Called every frame
void APalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APalCharacter::ApplyMoveSpeed()
{
	if (!GetCharacterMovement())
		return;

	switch (MoveState)
	{
	case EPalMoveState::Wandering:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;

	case EPalMoveState::Following:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		break;

	case EPalMoveState::RunningAway:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		break;

	case EPalMoveState::Chasing:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		break;

	case EPalMoveState::StandOff:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;
	}
}

// 상태 변화
void APalCharacter::SetMoveState(EPalMoveState NewState)
{
	if (MoveState == NewState)
		return;

	MoveState = NewState;
	ApplyMoveSpeed();
}

float APalCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                class AController* EventInstigator, AActor* DamageCauser)
{
	if (!PalStatComponent) return 0.f;

	const float FinalDamage = PalStatComponent->ApplyDamage(DamageAmount);

	// 사망 처리/AI 상태 변경 등
	// if (PalStatComponent->GetCurrentHP() <= 0.f) { ... }

	return FinalDamage;
}

void APalCharacter::HandleDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	// 1) AI 멈추기 (BT/BehaviorTree/Move)
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
		AIC->StopMovement();
		AIC->UnPossess(); // 선택: 시체에 AI가 붙어있지 않게
	}

	// 2) 캐릭터 이동/충돌/입력 차단
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
		MoveComp->SetComponentTickEnabled(false);
	}

	// 캡슐은 보통 ragdoll과 충돌/밀림 문제를 일으켜서 "죽으면 캡슐 충돌 끄고" 메시에 물리로 넘김
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3) 애님 끊기 + 래그돌 시작
	StartRagdoll();

	// 4) (선택) 일정 시간 뒤 제거
	SetLifeSpan(10.f);
}

void APalCharacter::StartRagdoll()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	// 애님이 계속 뼈를 제어하면 물리랑 싸움 -> 애님을 물리 우선으로
	MeshComp->bPauseAnims = true; // 간단 차단
	MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint); // 유지해도 되지만
	MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));

	// 물리 시뮬 활성화
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetAllBodiesSimulatePhysics(true);
	MeshComp->WakeAllRigidBodies();
	MeshComp->bBlendPhysics = true;

	// 캐릭터가 회전/이동 제어하던 것들 무력화
	MeshComp->SetEnableGravity(true);

	// (선택) 충돌 반응 튜닝
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APalCharacter::PlayHitSound()
{
	if (!HitSound) return;

	// 거리 감쇠가 적용된 3D 사운드 재생
	UGameplayStatics::SpawnSoundAtLocation(
		this,
		HitSound,
		GetActorLocation(),
		FRotator::ZeroRotator,
		1.f,
		1.f,
		0.f,
		HitSoundAttenuation
	);
}

bool APalCharacter::LoadPalData()
{
	// PalDT 유/무 검사
	if (!PalDT)
	{
		UE_LOG(LogTemp, Warning, TEXT("PalCharacter : PalDT is null. (%s)"), *GetName());
		return false;
	}

	// PalRowName 유/무 검사
	if (PalRowName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("PalCharacter : PalRowName is None. (%s)"), *GetName());
		return false;
	}

	// PalRowName 유효성 검사
	const FPalData* Row = PalDT->FindRow<FPalData>(PalRowName, TEXT("LoadPalData"));
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("PalCharacter : Row '%s' not found in PalDT. (%s)"),
		       *PalRowName.ToString(), *GetName());
		return false;
	}

	// 타입은 항상 종족값으로 동기화
	PalTypes = Row->Types;

	// 종족 고정 이름(PalName): 비어있을 때만 최초 1회 설정
	//    FText -> FName 변환
	if (PalName.IsNone())
	{
		// 우선 한글 이름이 있으면 그걸 사용, 없으면 영문 이름
		const FString SpeciesName =
			!Row->Name_KO.IsEmpty() ? Row->Name_KO.ToString() : Row->Name.ToString();

		PalName = FName(*SpeciesName);
	}

	// UI 표시용 이름(PalDisplayName): 유저가 바꿀 수 있으므로 "비어있을 때만" 기본값 세팅
	if (PalDisplayName.IsNone())
	{
		const FString DefaultDisplay =
			!Row->Name_KO.IsEmpty() ? Row->Name_KO.ToString() : Row->Name.ToString();

		PalDisplayName = FName(*DefaultDisplay);
	}

	return true;
}
