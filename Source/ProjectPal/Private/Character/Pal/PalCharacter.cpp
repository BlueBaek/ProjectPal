// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Pal/PalCharacter.h"

#include "Component/PalStatComponent.h"
#include "Data/PalData.h"

// Sets default values
APalCharacter::APalCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	PalStatComponent = CreateDefaultSubobject<UPalStatComponent>(TEXT("PalStatComponent"));

}

// Called when the game starts or when spawned
void APalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 최초 생성 시 스탯 부여
	PalStatComponent->SetLevel(PalLevel);
	PalStatComponent->InitializeStats(PalDT, PalRowName);
}

// Called every frame
void APalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


/*
bool APalCharacter::LoadPalData(FPalData& OutData) const
{
	if (!PalDT)
	{
		UE_LOG(LogTemp, Warning, TEXT("PalCharacter : PalDataTable is null (%s)"), *GetName());
		return false;
	}
	
	if (PalRowName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("PalCharacter : PalRowName is None (%s)"), *GetName());
		return false;
	}
	
	const FPalData* Row = PalDT->FindRow<FPalData>(PalRowName, TEXT("RowName_Pal"));
	
	if (!Row)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("PalCharacter : Row '%s' not found in PalDataTable (%s)"),
			*PalRowName.ToString(),
			*GetName()
		);
		return false;
	}
	
	OutData.PalNumber = Row->PalNumber;
	OutData.Name = Row->Name;
	OutData.Name_KO = Row->Name_KO;
	OutData.Types = Row->Types;
	OutData.SpeciesHP = Row->SpeciesHP;
	OutData.SpeciesAttack = Row->SpeciesAttack;
	OutData.SpeciesDefense = Row->SpeciesDefense;
	
	return true;
}
*/