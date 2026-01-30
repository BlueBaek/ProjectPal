// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PlayerStatComponent.h"

// Sets default values for this component's properties
UPlayerStatComponent::UPlayerStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();

	ClampCurrents();
	// 시작 시 UI 바인딩용으로 한번 브로드캐스트
	BroadcastAll();
}

void UPlayerStatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// 스테미나 재생을 위해 Tick 사용
	if (!bCanRegenStamina) return;
	if (CurrentStamina >= MaxStamina) return;

	// 매초 MaxStamina의 StaminaRegenRate정도 회복함
	const float RegenAmount = MaxStamina * StaminaRegenRate * DeltaTime;
	RestoreStamina(RegenAmount);
	
}

// 스탯 초기화
void UPlayerStatComponent::InitializeStats(int32 InLevel, float InMaxHP, float InMaxStamina, float InAttack, float InDefense, bool bFillCurrent)
{
	const int32 OldLevel = Level;

	// 하한선 설정
	Level = FMath::Max(1, InLevel);
	MaxHP = FMath::Max(1.0f, InMaxHP);
	MaxStamina = FMath::Max(0.0f, InMaxStamina);
	Attack = FMath::Max(0.0f, InAttack);
	Defense = FMath::Max(0.0f, InDefense);

	if (bFillCurrent)
	{
		CurrentHP = MaxHP;
		CurrentStamina = MaxStamina;
	}
	else
	{
		ClampCurrents();
	}

	if (OldLevel != Level)
	{
		OnLevelChanged.Broadcast(Level, OldLevel);
	}
	BroadcastAll();
}

void UPlayerStatComponent::SetLevel(int32 NewLevel)
{
	NewLevel = FMath::Max(1, NewLevel);
	if (NewLevel == Level) return;

	const int32 OldLevel = Level;
	Level = NewLevel;
	OnLevelChanged.Broadcast(Level, OldLevel);
}

float UPlayerStatComponent::ApplyDamage(float RawDamage)
{
	if (IsDead()) return 0.0f;
	
	RawDamage = FMath::Max(0.0f, RawDamage);

	// 간단 공식: Final = max(1, Raw - Defense) 단, Raw가 0이면 0 처리
	float FinalDamage = 0.0f;
	if (RawDamage > 0.0f)
	{
		FinalDamage = FMath::Max(1.0f, RawDamage - Defense);
	}

	const float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP - FinalDamage, 0.0f, MaxHP);

	if (!FMath::IsNearlyEqual(OldHP, CurrentHP))
	{
		OnHPChanged.Broadcast(CurrentHP, MaxHP);
		
		// HP가 실제로 깎였을 때만 자동 리젠 시작
		if (FinalDamage > 0.f && CurrentHP < MaxHP && !IsDead())
		{
			StartHPRegen();
		}
	}

	return FinalDamage;
}

float UPlayerStatComponent::Heal(float Amount)
{
	if (Amount <= 0.0f || IsDead()) return 0.0f;

	const float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.0f, MaxHP);

	const float Healed = CurrentHP - OldHP;
	if (!FMath::IsNearlyZero(Healed))
	{
		OnHPChanged.Broadcast(CurrentHP, MaxHP);
	}
	
	// 외부 힐로 인해 풀피가 됐으면 타이머 종료
	if (CurrentHP >= MaxHP)
	{
		StopHPRegen();
	}
	
	return Healed;
}

bool UPlayerStatComponent::SpendStamina(float Cost)
{
	Cost = FMath::Max(0.0f, Cost);
	if (Cost <= 0.0f) return true;

	if (CurrentStamina < Cost)
	{
		return false;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - Cost, 0.0f, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	return true;
}

float UPlayerStatComponent::RestoreStamina(float Amount)
{
	if (Amount <= 0.0f) return 0.0f;

	const float Old = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);

	const float Restored = CurrentStamina - Old;
	if (!FMath::IsNearlyZero(Restored))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
	return Restored;
}

void UPlayerStatComponent::StartHPRegen()
{
	// 죽으면 동작 안함
	if (IsDead()) return;
	
	// 체력이 깎여있을 때만 실행
	if (CurrentHP >= MaxHP) return;

	// 타이머 사용을 위함
	if (!GetWorld()) return;

	// 중복 방지
	if (GetWorld()->GetTimerManager().IsTimerActive(HPRegenTimerHandle))
	{
		return;
	}

	// 1초 주기로 RegenHP 실행
	GetWorld()->GetTimerManager().SetTimer(
		HPRegenTimerHandle,
		this,
		&UPlayerStatComponent::RegenHP,
		1.0f,
		true
	);
}

void UPlayerStatComponent::StopHPRegen()
{
	if (!GetWorld()) return;

	GetWorld()->GetTimerManager().ClearTimer(HPRegenTimerHandle);
}

// ===== 디버그용 ======
void UPlayerStatComponent::DrawDebugStat()
{
	if (!GEngine) return;

	const FString DebugText = FString::Printf(
		TEXT(
			"[StatComponent]\n"
			"Level : %d\n"
			"HP : %.1f / %.1f\n"
			"Stamina : %.1f / %.1f\n"
			"Attack : %.1f\n"
			"Defense : %.1f\n"
			// "HPRegen : %s\n"
			// "StaminaRegen : %s"
		),
		Level,
		CurrentHP, MaxHP,
		CurrentStamina, MaxStamina,
		Attack,
		Defense
		// GetWorld()->GetTimerManager().IsTimerActive(HPRegenTimerHandle) ? TEXT("ON") : TEXT("OFF"),
		// bCanRegenStamina ? TEXT("ON") : TEXT("OFF")
	);

	GEngine->AddOnScreenDebugMessage(
		-1,
		0.0f,
		FColor::Green,
		DebugText
	);
}

// 최대 최소 고정
void UPlayerStatComponent::ClampCurrents()
{
	CurrentHP = FMath::Clamp(CurrentHP, 0.0f, MaxHP);
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
}

void UPlayerStatComponent::BroadcastAll()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UPlayerStatComponent::RegenHP()
{
	if (IsDead())
	{
		StopHPRegen();
		return;
	}

	if (CurrentHP >= MaxHP)
	{
		StopHPRegen();
		return;
	}

	const float RegenAmount = MaxHP * HPRegenRate;
	Heal(RegenAmount);
	
	// Heal 이후에 MaxHP 도달했으면 종료
	if (CurrentHP >= MaxHP)
	{
		StopHPRegen();
	}
}

