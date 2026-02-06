// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PalStatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPAL_API UPalStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPalStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
private:
	// ===== 종족 값 ===== (팰 종족 별 기준으로 정해지는 값)
	// 종족 값 : 체력
	int32 SHP;
	
	// 종족 값 : 공격력
	int32 SAttack;
	
	// 종족 값 : 방어력
	int32 SDefense;
	
protected:
	// ===== 개체 값 ===== (1 ~ 100까지 랜덤으로)
	// 개체 체력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	int32 IndividualHP;
	
	// 개체 공격력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	int32 IndividualAttack;
	
	// 개체 방어력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	int32 IndividualDefense;
	
	// ===== 실제 스탯 =====
	// 레벨
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	int32 Level = 1;

	// 최대 체력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	float MaxHP;

	// 공격력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	float Attack;

	// 방어력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	float Defense;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, Category="Stat|Runtime", meta=(AllowPrivateAccess="true"))
	float CurrentHP;
	
public:
	// 최초 스탯 설정
	void InitializeStats(UDataTable* DT, FName RowName);
	
	// 실제 스탯 부여(레벨업 등에서 재활용)
	void SetStat();
	
	UFUNCTION(BlueprintCallable, Category="Pal|Stat")
	int32 GetLevel() const { return Level; }
	
	UFUNCTION(BlueprintCallable, Category="Pal|Stat")
	void SetLevel(int32 lv) {Level = lv;}

	UFUNCTION(BlueprintCallable, Category="Pal|Stat")   
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintCallable, Category="Pal|Stat")
	float GetAttack() const { return Attack; }

	UFUNCTION(BlueprintCallable, Category="Pal|Stat")
	float GetDefense() const { return Defense; }

	UFUNCTION(BlueprintCallable, Category="Pal|Stat")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintCallable, Category="Pal|Stat")
	void ApplyDamage(int32 Damage);
};
