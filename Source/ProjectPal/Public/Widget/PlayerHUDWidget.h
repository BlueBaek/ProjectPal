// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"


class UPlayerStatComponent;
class UProgressBar;

UCLASS()
class PROJECTPAL_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 플레이어 스탯 컴포넌트 연결
	UFUNCTION(BlueprintCallable)
	void BindToPlayer(UPlayerStatComponent* StatComp);

protected:
	UPROPERTY(meta=(BindWidget)) UProgressBar* PB_HP;
	UPROPERTY(meta=(BindWidget)) UProgressBar* PB_Stamina;

	UFUNCTION() void HandleHPChanged(float Cur, float Max);
	UFUNCTION() void HandleStaminaChanged(float Cur, float Max);

private:
	UPROPERTY(Transient) TObjectPtr<UPlayerStatComponent> BoundStat;
};