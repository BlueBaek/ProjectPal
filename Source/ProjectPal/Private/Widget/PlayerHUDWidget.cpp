// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PlayerHUDWidget.h"

#include "Component/PlayerStatComponent.h"
#include "Components/ProgressBar.h"

void UPlayerHUDWidget::BindToPlayer(UPlayerStatComponent* StatComp)
{
	if (!StatComp) return;

	BoundStat = StatComp;

	// 초기값 반영
	HandleHPChanged(StatComp->GetCurrentHP(), StatComp->GetMaxHP());
	HandleStaminaChanged(StatComp->GetCurrentStamina(), StatComp->GetMaxStamina());

	// 이벤트 바인딩
	StatComp->OnHPChanged.AddDynamic(this, &UPlayerHUDWidget::HandleHPChanged);
	StatComp->OnStaminaChanged.AddDynamic(this, &UPlayerHUDWidget::HandleStaminaChanged);
}

void UPlayerHUDWidget::HandleHPChanged(float Cur, float Max)
{
	if (!PB_HP) return;
	const float Pct = (Max <= 0.f) ? 0.f : (Cur / Max);
	PB_HP->SetPercent(FMath::Clamp(Pct, 0.f, 1.f));
}

void UPlayerHUDWidget::HandleStaminaChanged(float Cur, float Max)
{
	if (!PB_Stamina) return;
	const float Pct = (Max <= 0.f) ? 0.f : (Cur / Max);
	PB_Stamina->SetPercent(FMath::Clamp(Pct, 0.f, 1.f));
}
