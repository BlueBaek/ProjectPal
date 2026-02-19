// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PalStatComponent.h"

#include "Component/OwnedPalComponent.h"
#include "Data/PalData.h"

// Sets default values for this component's properties
UPalStatComponent::UPalStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UPalStatComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UPalStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// 최초 한번 실행되어야 함
// 데이터 테이블에서 읽어온 값을 기준으로 설정
void UPalStatComponent::InitializeStats(UDataTable* DT, FName RowName)
{
	const FPalData* Row = DT ? DT->FindRow<FPalData>(RowName, TEXT("PalData")) : nullptr;
	if (!Row) return;
	
	// 종족값 설정
	SHP = Row->SpeciesHP;
	SAttack = Row->SpeciesAttack;
	SDefense = Row->SpeciesDefense;
	
	// 개체치 설정
	IndividualHP = FMath::RandRange(1, 100);
	IndividualAttack = FMath::RandRange(1, 100);
	IndividualDefense =	FMath::RandRange(1, 100);
	
	SetStat();
}

// 팰월드 팰의 기본 능력치 공식을 따름
void UPalStatComponent::SetStat()
{
	// HP 공식
	MaxHP = 500 + Level * 5 + Level * (SHP * 0.5) * (1 + IndividualHP * 0.003);
	
	// ATK 공식
	Attack =  100 + Level * (SAttack * 0.075) * (1 + IndividualAttack * 0.003);
	
	// DEF 공식
	Defense = 50 + Level * (SDefense * 0.075) * (1 + IndividualDefense * 0.003);
	
	// 최대 체력 적용
	CurrentHP = MaxHP;
	
	// 체력 변경 알림
	BroadcastHPChanged();
}

float UPalStatComponent::ApplyDamage(float RawDamage)
{
	if ((RawDamage <= 0.f) || IsDead()) return 0.f;
	
	RawDamage = FMath::Max(0.0f, RawDamage);
	
	const float Def = FMath::Max(1.f, Defense);
	const float FinalDamage = (RawDamage > 0.f) ? (RawDamage / Def) : 0.f;
	
	CurrentHP = FMath::Clamp(CurrentHP - FinalDamage, 0.0f, MaxHP);

	if (APalCharacter* Pal = Cast<APalCharacter>(GetOwner()))
	{
		Pal->PlayHitSound();
	}
	
	if (CurrentHP <= 0.f)
	{
		OnDeath.Broadcast();
	}
	
	BroadcastHPChanged();
	return FinalDamage;
}

void UPalStatComponent::ExportToOwned(FPalStatSaveData& Out) const
{
	Out.Level = Level;
	Out.IndividualHP = IndividualHP;
	Out.IndividualAttack = IndividualAttack;
	Out.IndividualDefense = IndividualDefense;
	Out.CurrentHP = CurrentHP;
}

void UPalStatComponent::ImportFromOwned(const FPalStatSaveData& In)
{
	Level = In.Level;

	// 개체값 복원
	IndividualHP = In.IndividualHP;
	IndividualAttack = In.IndividualAttack;
	IndividualDefense = In.IndividualDefense;

	// 레벨/개체값 기반으로 스탯 재계산
	SetStat();

	// 현재 HP 복원 (MaxHP 고려)
	if (In.CurrentHP <= 0.f)
	{
		CurrentHP = MaxHP;
	}
	else
	{
		CurrentHP = FMath::Clamp(In.CurrentHP, 1.f, MaxHP);
	}
}

void UPalStatComponent::BroadcastHPChanged()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}
