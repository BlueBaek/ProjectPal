// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PalCharacter.generated.h"

class UPalStatComponent;
class UPalSkillComponent;
enum class EPalType : uint8;
class UDataTable;

UENUM(BlueprintType)
enum class EPalMoveState : uint8
{
	Idle UMETA(DisplayName="Idle"),
	Walk UMETA(DisplayName="Walk"),
	Run UMETA(DisplayName="Run"),
};

UCLASS()
class PROJECTPAL_API APalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APalCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Pal")
	UPalStatComponent* GetStatComponent() const { return PalStatComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 데이터 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	UDataTable* PalDT = nullptr;

	// 팰 확정을 위해 PalRowName 부여
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pal")
	FName PalRowName;

	// 팰 이름 (수정 불가)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pal")
	FName PalName;

	// UI표시용 이름(수정 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pal")
	FName PalDisplayName;

	// 팰 타입(수정 불가)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pal")
	TArray<EPalType> PalTypes;

	// 팰 최초 레벨 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pal")
	int32 PalLevel = 1;

	// 스탯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pal|Components")
	UPalStatComponent* PalStatComponent = nullptr;

	// 스킬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pal|Components")
	UPalSkillComponent* PalSkillComponent = nullptr;

	UPROPERTY(EditAnywhere, Category="Pal|Movement")
	float WalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, Category="Pal|Movement", meta=(AllowPrivateAccess="true"))
	float RunSpeed = 600.0f;

	// 팰의 움직임 속도를 조절하기 위한 Enum class 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Move")
	EPalMoveState MoveState = EPalMoveState::Idle;

	// 팰 타입 출력용
	FString PalTypeString;

	void ApplyMoveSpeed();

public:
	// 이동 상태 변경 인터페이스
	void SetMoveState(EPalMoveState NewState);

private:
	// 데이터 테이블로부터 팰 정보 Load
	bool LoadPalData();
};
