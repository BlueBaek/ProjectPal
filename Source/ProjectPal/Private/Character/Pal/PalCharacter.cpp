// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Pal/PalCharacter.h"

#include "EngineUtils.h"
#include "Component/PalSkillComponent.h"
#include "Component/PalStatComponent.h"
#include "Data/PalData.h"

// Sets default values
APalCharacter::APalCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	PalStatComponent = CreateDefaultSubobject<UPalStatComponent>(TEXT("PalStatComponent"));
	PalSkillComponent = CreateDefaultSubobject<UPalSkillComponent>(TEXT("PalSkillComponent"));

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
	if (PalStatComponent)	// PalStatComponent 유효성 검사
	{
		PalStatComponent->SetLevel(PalLevel);
		
		if (PalDT)
		{
			PalStatComponent->InitializeStats(PalDT, PalRowName);
		}
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
	
	UE_LOG(LogTemp, Warning, TEXT("[Pal] BeginPlay: %s  AutoTest=%d  SkillComp=%s"),
		*GetName(),
		bTestCastGrassTornadoOnBeginPlay ? 1 : 0,
		PalSkillComponent ? TEXT("Valid") : TEXT("NULL"));
	
	if (bTestCastGrassTornadoOnBeginPlay)
	{
		TestCastGrassTornadoOnce();
	}
}

// Called every frame
void APalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

// ===== 테스트용 =====
void APalCharacter::TestCastGrassTornadoOnce()
{
	if (!PalSkillComponent || !GetWorld())
		return;

	AActor* BestTarget = nullptr;
	float BestDistSq = FLT_MAX;

	const FVector MyLoc = GetActorLocation();

	// 테스트용: 가장 가까운 Pawn을 타겟으로
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* Candidate = *It;
		if (!Candidate || Candidate == this)
			continue;

		const float DistSq = FVector::DistSquared(MyLoc, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Pal] BestTarget=%s  Dist=%.1f"),
		BestTarget ? *BestTarget->GetName() : TEXT("NULL"),
		BestTarget ? FMath::Sqrt(BestDistSq) : -1.f);
	
	if (!BestTarget) return;

	// 👉 테스트 스킬 실행 (Grass Tornado)
	PalSkillComponent->Cast_Test_GrassTornado(BestTarget);

	// 안전장치: 다시는 실행 안 되게
	bTestCastGrassTornadoOnBeginPlay = false;
}
