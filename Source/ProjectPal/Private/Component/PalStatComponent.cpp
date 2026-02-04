// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PalStatComponent.h"

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

	// ...
	
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
	
	SHP = Row->SpeciesHP;
	SAttack = Row->SpeciesAttack;
	SDefense = Row->SpeciesDefense;
	
	IndividualHP = FMath::RandRange(1, 100);
	IndividualAttack = FMath::RandRange(1, 100);
	IndividualDefense =	FMath::RandRange(1, 100);
	
	SetStat();
}

void UPalStatComponent::SetStat()
{
	MaxHP = 500 + Level * 5 + Level * (SHP * 0.5) * (1 + IndividualHP * 0.003);
	Attack =  100 + Level * (SAttack * 0.075) * (1 + IndividualAttack * 0.003);
	Defense = 50 + Level * (SDefense * 0.075) * (1 + IndividualDefense * 0.003);
}

void UPalStatComponent::ApplyDamage(int32 Damage)
{
	
}
