// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/PlayerAnimInstance.h"

#include "Character/Player/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h" // CalculateDirection을 위해 필요

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Player = Cast<ACharacter>(TryGetPawnOwner());
	if (Player)
	{
		MovementComponent = Player->GetCharacterMovement();
	}
	// 초기값 동기화(스폰 직후 튀는 것 방지)
	bPrevIsAiming = false;
}

// 애니메이션 업데이트
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// Pawn이나 MovementComponent가 바뀌는 케이스 대비(안전)
	if (!Player)
	{
		Player = Cast<ACharacter>(TryGetPawnOwner());
	}
	if (Player && !MovementComponent)
	{
		MovementComponent = Player->GetCharacterMovement();
	}
	

	// 구현
	if (MovementComponent && Player)
	{
		// 움직임 갱신
		Speed = MovementComponent->Velocity.Size2D(); // 속도(Size2D로 상하는 포함하지 않음)
		Acceleration = MovementComponent->GetCurrentAcceleration(); // 가속도
		IsMoving = Acceleration.SizeSquared() > 0.0f; // 움직임 여부 판단(빠른 계산을 위한 SizeSquared사용)

		// 방향 갱신
		// Direction 계산: Velocity와 ActorRotation을 기반으로 각도 산출
		Direction = UKismetAnimationLibrary::CalculateDirection(MovementComponent->Velocity,
		                                                        Player->GetActorRotation());
		if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Player))
		{
			FRotator ControlRot = PlayerChar->GetControlRotation();
			FRotator ActorRot = PlayerChar->GetActorRotation();
			
			FRotator Delta = ControlRot - ActorRot;
			Delta.Normalize();
			
			AimPitch = Delta.Pitch;
			
			// PlayerCharacter에서 bIsAiming 값을 가져옴
			IsAiming = PlayerChar->GetIsAiming(); //bIsAiming의 getter
		}
		// 공중 상태 갱신
		IsFalling = MovementComponent->IsFalling(); // 공중 확인
		if (IsFalling) // 공중에 떴을 때 위로 가는중인지 아래로 떨어지는중인지 확인
		{
			// 올라가는지 내려가는지 판단
			IsRising = (Player->GetVelocity().Z > 0);
		}
		else { IsRising = false; }
		
		// 추가 조준 변화에 따른 상체 몽타주 제어
		HandleAimUpperMontage();
	}
}

void UPlayerAnimInstance::HandleAimUpperMontage()
{
	// 몽타주가 지정되지 않았으면 아무것도 하지 않음
	if (!AimUpperIdleMontage)
	{
		bPrevIsAiming = IsAiming;
		return;
	}

	// 1) false -> true : 조준 시작 시 재생(한 번만)
	if (IsAiming && !bPrevIsAiming)
	{
		// 이미 재생 중이면 중복 재생 방지
		if (!Montage_IsPlaying(AimUpperIdleMontage))
		{
			Montage_Play(AimUpperIdleMontage, 1.0f);
		}
	}

	// 2) true -> false : 조준 종료 시 정지(한 번만)
	if (!IsAiming && bPrevIsAiming)
	{
		// 재생 중일 때만 정지
		if (Montage_IsPlaying(AimUpperIdleMontage))
		{
			Montage_Stop(AimUpperIdleStopBlendOutTime, AimUpperIdleMontage);
		}
	}

	// 3) 다음 프레임 비교를 위해 저장
	bPrevIsAiming = IsAiming;
}
