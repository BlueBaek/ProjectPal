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
}

// 애니메이션 업데이트
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
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
			// PlayerCharacter에서 bIsAiming 값을 가져옴
			IsAiming = PlayerChar->GetIsAiming();	//bIsAiming의 getter
		}
		// 공중 상태 갱신
		IsFalling = MovementComponent->IsFalling(); // 공중 확인
		if (IsFalling) // 공중에 떴을 때 위로 가는중인지 아래로 떨어지는중인지 확인
		{
			// 올라가는지 내려가는지 판단
			if (Player->GetVelocity().Z > 0) { IsRising = true; }
			else if (Player->GetVelocity().Z <= 0) { IsRising = false; }
		}
		else { IsRising = false; }
		
	}
}
