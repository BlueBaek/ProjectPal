#pragma once

#include "CoreMinimal.h"
#include "PalData.generated.h"

enum class EPalType : uint8;

USTRUCT(BlueprintType)
struct FPalData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 팰 번호
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PalNumber;
	
	// 팰 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;
	
	// 팰 이름(한글)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name_KO;
	
	// 팰 타입 (1개 또는 2개)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<EPalType> Types;
	
	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpeciesHP;

	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpeciesAttack;

	// 방어력
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpeciesDefense;
};
