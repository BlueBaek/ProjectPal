// Fill out your copyright notice in the Description page of Project Settings.


#include "PalSkill/PalSkillExecution.h"

#include "Character/Pal/PalCharacter.h"
#include "Component/PalSkillComponent.h"
#include "DataAsset/PalSkillDataAsset.h"

bool UPalSkillExecution::StartPrepare(APalCharacter* InCaster, AActor* InTarget, const UPalSkillDataAsset* InSkillData)
{
	Caster = InCaster;
	Target = InTarget;
	SkillData = InSkillData;

	if (!Caster || !SkillData)
	{
		return false;
	}

	// 1) "Start(=Prepare 전환)" 몽타주를 재생하고,
	//    끝나면 OnStartMontageEnded에서 StartLoop를 재생한다.
	UAnimMontage* StartMontage = Caster->GetSkillStartMontage();
	if (StartMontage)
	{
		FOnMontageEnded EndDel;
		EndDel.BindUObject(this, &UPalSkillExecution::OnStartMontageEnded);
		PlayWithEndDelegate(StartMontage, EndDel);
	}
	else
	{
		// StartMontage가 없다면 바로 Loop로 넘어가도 되지만
		// 지금은 안전하게 Loop를 바로 재생하도록 처리
		OnStartMontageEnded(nullptr, /*bInterrupted=*/false);
	}
	
	return true;
}

void UPalSkillExecution::Activate()
{
	if (!Caster || !SkillData)
	{
		Finish();
		return;
	}
	
	// PrepareLoop는 Action으로 넘어갈 때 끊어준다(루프라 계속 도니까)
	StopIfPlaying(Caster->GetSkillStartLoopMontage(), 0.1f);

	// SkillFire Notify가 Start(Action) 몽타주에서 발생하므로
	// Action 재생 전에 ActiveExecution 등록이 되어 있어야 함
	if (UPalSkillComponent* SkillComp = Caster->FindComponentByClass<UPalSkillComponent>())
	{
		SkillComp->SetActiveExecution(this);
	}

	// 3) "Action(=Start 전환)" 몽타주를 재생하고,
	//    끝나면 OnActionMontageEnded에서 ActionLoop를 재생한다.
	UAnimMontage* ActionMontage = Caster->GetSkillActionMontage();
	if (ActionMontage)
	{
		FOnMontageEnded EndDel;
		EndDel.BindUObject(this, &UPalSkillExecution::OnActionMontageEnded);
		PlayWithEndDelegate(ActionMontage, EndDel);
	}
	else
	{
		// Action 몽타주가 없으면 바로 루프로
		OnActionMontageEnded(nullptr, /*bInterrupted=*/false);
	}

	// 4) Duration 후 종료 (StartLoop 지속)
	const float Duration = FMath::Max(0.f, SkillData->Timing.Duration);
	if (Duration <= 0.f)
	{
		Finish();
		return;
	}

	if (UWorld* World = Caster->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DurationEndHandle,
			this,
			&UPalSkillExecution::Finish,
			Duration,
			false
		);
	}
}

void UPalSkillExecution::Finish()
{
	if (!Caster) return;

	// 지속 루프 정지
	StopIfPlaying(Caster->GetSkillActionLoopMontage(), 0.15f);

	// 타이머 정리
	if (UWorld* World = Caster->GetWorld())
	{
		World->GetTimerManager().ClearTimer(DurationEndHandle);
	}

	// ActiveExecution 해제
	if (UPalSkillComponent* SkillComp = Caster->FindComponentByClass<UPalSkillComponent>())
	{
		SkillComp->ClearActiveExecution(this);
	}
}

void UPalSkillExecution::OnStartMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Start(Prepare 전환)이 끝났으면 StartLoop(PrepareLoop)를 재생한다.
	// Interrupted여도 "루프로 들어가는 느낌"을 유지하고 싶으면 재생해도 되고,
	// 정책상 Interrupted면 종료해도 됨. (여기선 계속 진행)
	if (!Caster) return;

	UAnimMontage* StartLoop = Caster->GetSkillStartLoopMontage();
	if (StartLoop)
	{
		if (UAnimInstance* Anim = GetAnimInstance())
		{
			// 루프 몽타주는 EndDelegate가 필요 없으니 그냥 Play
			Anim->Montage_Play(StartLoop);
		}
	}
}

void UPalSkillExecution::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Action(전환)이 끝났으면 ActionLoop(지속)를 재생한다.
	if (!Caster) return;

	UAnimMontage* ActionLoop = Caster->GetSkillActionLoopMontage();
	if (ActionLoop)
	{
		if (UAnimInstance* Anim = GetAnimInstance())
		{
			Anim->Montage_Play(ActionLoop);
		}
	}
}

// ------------------------------
// Helpers
// ------------------------------
UAnimInstance* UPalSkillExecution::GetAnimInstance() const
{
	if (!Caster) return nullptr;
	if (USkeletalMeshComponent* Mesh = Caster->GetMesh())
	{
		return Mesh->GetAnimInstance();
	}
	return nullptr;
}

void UPalSkillExecution::StopIfPlaying(UAnimMontage* Montage, float BlendOutTime) const
{
	if (!Montage) return;

	if (UAnimInstance* Anim = GetAnimInstance())
	{
		if (Anim->Montage_IsPlaying(Montage))
		{
			Anim->Montage_Stop(BlendOutTime, Montage);
		}
	}
}

void UPalSkillExecution::PlayWithEndDelegate(UAnimMontage* Montage, FOnMontageEnded& InOutDelegate)
{
	if (!Montage) return;

	if (UAnimInstance* Anim = GetAnimInstance())
	{
		// 재생
		Anim->Montage_Play(Montage);

		// "이 몽타주가 끝났을 때" 호출될 EndDelegate 설정
		// (슬롯에서 다른 몽타주가 재생되면 Interrupted로 들어올 수도 있음)
		Anim->Montage_SetEndDelegate(InOutDelegate, Montage);
	}
}