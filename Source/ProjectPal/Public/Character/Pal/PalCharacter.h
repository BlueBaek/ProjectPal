// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PalCharacter.generated.h"

class UPalStatComponent;
class UPalSkillComponent;
enum class EPalType : uint8;
class UDataTable;
class UAnimMontage;

// 포획 가능 구분용
UENUM(BlueprintType)
enum class EPalGroup : uint8
{
	Wild UMETA(DisplayName="Wild"),
	Tamed UMETA(DisplayName="Tamed"),
	Boss UMETA(DisplayName="Boss")
};

// 상태에 따른 움직임 속도 제한을 위한 Enum class
UENUM(BlueprintType)
enum class EPalMoveState : uint8
{
	// 비전투 상태(야생 팰). 단순 배회중
	Wandering UMETA(DisplayName="Wandering"),
	// 비전투 상태(야생 팰). 비선공 팰이 적 발견시 도주할 때
	RunningAway UMETA(DisplayName="RunningAway"),
	// 비전투 상태(소유 팰). 주인을 쫓아갈 때.
	Following UMETA(DisplayName="Following"),
	// 전투 상태(공용). 상대를 쫓을 때. ex) 스킬 사용을 위해 상대에게 접근
	Chasing UMETA(DisplayName="Chasing"),
	// 전투 상태(공용). 대치 상태. 공격 후 다음 공격까지 약간의 딜레이 시간(약 3초). 쳐다보면서 천천히 우측으로 원을 그리며 움직임
	StandOff UMETA(DisplayName="StandOff")
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

	// 팰 그룹
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pal")
	EPalGroup PalGroup = EPalGroup::Wild;

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
	EPalMoveState MoveState = EPalMoveState::Wandering;

	// 팰 타입 출력용
	FString PalTypeString;

	// State에 따라 MoveSpeed 적용
	void ApplyMoveSpeed();

	// 현재 타겟 변수
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> CurrentTargetActor = nullptr;
	
public:
	// 이동 상태 변경 인터페이스
	void SetMoveState(EPalMoveState NewState);

	// 타겟을 발견했을 때 재생할 몽타주(팰마다 다르게 BP에서 지정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Montage")
	TObjectPtr<UAnimMontage> Encount;
	UFUNCTION(BlueprintCallable, Category="Animation|Montage")
	UAnimMontage* GetAggroMontage() const { return Encount; }

	// Enum을 Blueprint에서 사용하기 위함
	UPROPERTY(EditAnywhere)
	EPalMoveState PalMoveState;

	// 스킬 초기화
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pal|Skill")
	TArray<TObjectPtr<class UPalSkillDataAsset>> InitialSkills;
	
	// 스킬 사용 시 사용할 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Montage")
	TObjectPtr<UAnimMontage> SkillStart;
	UFUNCTION(BlueprintCallable, Category="Animation|Montage")
	UAnimMontage* GetSkillStartMontage() const { return SkillStart; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Montage")
	TObjectPtr<UAnimMontage> SkillStartLoop;
	UFUNCTION(BlueprintCallable, Category="Animation|Montage")
	UAnimMontage* GetSkillStartLoopMontage() const { return SkillStartLoop; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Montage")
	TObjectPtr<UAnimMontage> SkillAction;
	UFUNCTION(BlueprintCallable, Category="Animation|Montage")
	UAnimMontage* GetSkillActionMontage() const { return SkillAction; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Montage")
	TObjectPtr<UAnimMontage> SkillActionLoop;
	UFUNCTION(BlueprintCallable, Category="Animation|Montage")
	UAnimMontage* GetSkillActionLoopMontage() const { return SkillActionLoop; }
	
	// 현재 타겟 Setter
	UFUNCTION(BlueprintCallable, Category="Combat")
	void SetCurrentTarget(AActor* NewTarget) { CurrentTargetActor = NewTarget; }

	// 현재 타겟 Getter
	UFUNCTION(BlueprintCallable, Category="Combat")
	AActor* GetCurrentTarget() const { return CurrentTargetActor; }
	
	
private:
	// 데이터 테이블로부터 팰 정보 Load
	bool LoadPalData();
};
