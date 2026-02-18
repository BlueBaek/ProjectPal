// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PalNameplateWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UPalStatComponent;
class APalCharacter;

UCLASS()
class PROJECTPAL_API UPalNameplateWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// WidgetComponent에서 생성된 뒤 Pal이 정해졌을 때 호출
	UFUNCTION(BlueprintCallable, Category="Pal|UI")
	void InitFromPal(APalCharacter* InPal);

protected:
	// WBP에서 동일한 이름으로 만들어야 자동 바인딩 됨
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TXT_Level;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TXT_Name;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> PB_HP;

	// 위젯 파괴 시 델리게이트 해제
	virtual void NativeDestruct() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<APalCharacter> Pal = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UPalStatComponent> Stat = nullptr;

	// 델리게이트 콜백
	UFUNCTION() void HandleHPChanged(float CurrentHP, float MaxHP);

	void RefreshStaticText(); // 이름/레벨 같은 “고정값” 갱신
	void UnbindDelegates();
};
