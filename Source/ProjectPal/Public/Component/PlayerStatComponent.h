// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32, NewLevel, int32, OldLevel);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTPAL_API UPlayerStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
private:
	// 레벨
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	int32 Level = 1;

	// 최대 체력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	float MaxHP = 500.0f;

	// 최대 스테미나
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	float MaxStamina = 100.0f;

	// 공격력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	float Attack = 100.0f;

	// 방어력
	UPROPERTY(EditAnywhere, Category="Stat|Base", meta=(AllowPrivateAccess="true"))
	float Defense = 100.0f;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, Category="Stat|Runtime", meta=(AllowPrivateAccess="true"))
	float CurrentHP = 500.0f;

	// 현재 스테미나
	UPROPERTY(VisibleAnywhere, Category="Stat|Runtime", meta=(AllowPrivateAccess="true"))
	float CurrentStamina = 100.0f;
	
public:
	// Getter
	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE int32 GetLevel() const { return Level; }

	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE float GetAttack() const { return Attack; }

	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE float GetDefense() const { return Defense; }
	
	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category="Stat")
	FORCEINLINE bool IsDead() const { return CurrentHP <= 0.0f; }

	// ===== Initialize / Setters =====
	// bFillCurrent=true면 CurrentHP/CurrentStamina를 Max로 채움
	UFUNCTION(BlueprintCallable, Category="Stat")
	void InitializeStats(int32 InLevel, float InMaxHP, float InMaxStamina, float InAttack, float InDefense, bool bFillCurrent = true);

	UFUNCTION(BlueprintCallable, Category="Stat")
	void SetLevel(int32 NewLevel);

	// ===== HP =====
	// RawDamage에 Defense를 적용해 최종 데미지 계산 후 HP 감소
	UFUNCTION(BlueprintCallable, Category="Stat")
	float ApplyDamage(float RawDamage);

	UFUNCTION(BlueprintCallable, Category="Stat")
	float Heal(float Amount);

	// ===== Stamina =====
	// 성공 시 true, 부족하면 false
	UFUNCTION(BlueprintCallable, Category="Stat")
	bool SpendStamina(float Cost);

	UFUNCTION(BlueprintCallable, Category="Stat")
	float RestoreStamina(float Amount);

	// ===== Events =====
	UPROPERTY(BlueprintAssignable, Category="Stat|Event")
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category="Stat|Event")
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category="Stat|Event")
	FOnLevelChanged OnLevelChanged;
	
	// ===== Regen =====
	// HP : 초당 MaxHP의 0.3%
	UPROPERTY(EditAnywhere, Category="Stat|Regen", meta=(ClampMin="0.0"))
	float HPRegenRate = 0.003f;

	// Stamina : 초당 MaxStamina의 20%
	UPROPERTY(EditAnywhere, Category="Stat|Regen", meta=(ClampMin="0.0"))
	float StaminaRegenRate = 0.2f;

	// 외부에서 제어 (특정 행동 중이면 false)
	UPROPERTY(BlueprintReadWrite, Category="Stat|Regen")
	bool bCanRegenStamina = true;

	// 타이머 Start
	UFUNCTION(BlueprintCallable, Category="Stat|Regen")
	void StartHPRegen();

	// 타이머 Stop
	UFUNCTION(BlueprintCallable, Category="Stat|Regen")
	void StopHPRegen();
	
	// ===== 디버그용 ======
	UFUNCTION()
	void DrawDebugStat();
	
private:
	void ClampCurrents();
	void BroadcastAll();
	
	FTimerHandle HPRegenTimerHandle;	// 체력 재생용 타이머
	void RegenHP();	// 체력 재생
};
