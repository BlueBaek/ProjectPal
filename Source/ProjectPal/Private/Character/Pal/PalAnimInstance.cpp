// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Pal/PalAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPalAnimInstance::UPalAnimInstance()
{
}

void UPalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OwningPawn = TryGetPawnOwner();
	OwningCharacter = Cast<ACharacter>(OwningPawn);
	// MoveComponent = OwningCharacter->GetCharacterMovement();
	MoveComponent = OwningCharacter ? OwningCharacter->GetCharacterMovement() : nullptr;
	
}

void UPalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwningPawn)
	{
		OwningPawn = TryGetPawnOwner();
		OwningCharacter = Cast<ACharacter>(OwningPawn);
		// MoveComponent = OwningCharacter->GetCharacterMovement();
		MoveComponent = OwningCharacter ? OwningCharacter->GetCharacterMovement() : nullptr;
		
		if (!OwningPawn) return;
	}
	
	// 속도
	Speed = MoveComponent->Velocity.Size2D();
	
	// 공중/가속
	if (MoveComponent)
	{
		bIsFalling = MoveComponent->IsFalling();
		if (bIsFalling) // 공중에 떴을 때 위로 가는중인지 아래로 떨어지는중인지 확인
		{
			// 올라가는지 내려가는지 판단
			bIsRising = (OwningCharacter->GetVelocity().Z > 0);
		}
		Acceleration = MoveComponent->GetCurrentAcceleration();
		bHasAcceleration =Acceleration.SizeSquared() > 0.f;
	}
	else
	{
		bIsFalling = false;
		bHasAcceleration = false;
	}
	
	bIsMoving = Speed > 3.f;
}