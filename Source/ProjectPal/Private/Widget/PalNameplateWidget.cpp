// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PalNameplateWidget.h"

#include "Character/Pal/PalCharacter.h"
#include "Component/PalStatComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPalNameplateWidget::InitFromPal(APalCharacter* InPal)
{
	// 혹시 재초기화가 들어오면 기존 바인딩 해제
	UnbindDelegates();

	Pal = InPal;
	Stat = (Pal) ? Pal->GetStatComponent() : nullptr;

	RefreshStaticText();

	// 초기 HP 반영 + 델리게이트 바인딩
	if (Stat)
	{
		HandleHPChanged(Stat->GetCurrentHP(), Stat->GetMaxHP());

		// 중복 Add 방지
		Stat->OnHPChanged.RemoveDynamic(this, &UPalNameplateWidget::HandleHPChanged);
		Stat->OnHPChanged.AddDynamic(this, &UPalNameplateWidget::HandleHPChanged);
	}
}

void UPalNameplateWidget::NativeDestruct()
{
	UnbindDelegates();
	Super::NativeDestruct();
}

void UPalNameplateWidget::HandleHPChanged(float CurrentHP, float MaxHP)
{
	if (!PB_HP) return;

	const float Pct = (MaxHP <= 0.f) ? 0.f : (CurrentHP / MaxHP);
	PB_HP->SetPercent(FMath::Clamp(Pct, 0.f, 1.f));
}

void UPalNameplateWidget::RefreshStaticText()
{
	// 이름
	if (TXT_Name && Pal)
	{
		// 네 PalCharacter에 PalDisplayName이 있다면 그걸 우선
		const FName DisplayName = Pal->GetPalDisplayName(); // 네가 추가한 getter 사용
		const FString NameStr = DisplayName.IsNone() ? Pal->GetName() : DisplayName.ToString();
		TXT_Name->SetText(FText::FromString(NameStr));
	}

	// 레벨
	if (TXT_Level && Stat)
	{
		TXT_Level->SetText(FText::FromString(FString::Printf(TEXT("%d"), Stat->GetLevel())));
	}
}

void UPalNameplateWidget::UnbindDelegates()
{
	if (Stat)
	{
		Stat->OnHPChanged.RemoveDynamic(this, &UPalNameplateWidget::HandleHPChanged);
	}
	Stat = nullptr;
}
