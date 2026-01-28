// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/MyPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/Player/CombatComponent.h"
#include "Character/Player/PlayerCharacter.h"

AMyPlayerController::AMyPlayerController()
{
}

void AMyPlayerController::UpdateInputContext()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!LocalPlayer || !PlayerCharacter || !Subsystem) return;

	// 현재 적용중인 IMC를 제거 (중복 방지)
	// 개선 필요함 (다른 IMC까지 지울 수 있음)
	// Subsystem->ClearAllMappings();

	// 원하는 상태 IMC 결정
	UInputMappingContext* DesiredStateIMC = nullptr;

	// 현재 상태에 따라 적절한 IMC 할당
	if (PlayerCharacter->GetActionState() == EMyActionState::Rolling)
	{
		if (RollingIMC) { DesiredStateIMC = RollingIMC; }
	}
	/*else if (Player->GetActionState() == EMyActionState::...)*/
	else
	{
		if (RollingIMC) { DesiredStateIMC = IdleIMC; }
	}

	// 현재 IMC와 들어온 IMC 비교하여 같으면 return
	if (DesiredStateIMC == CurrentStateIMC) return;

	// 기존 상태 IMC만 제거
	if (CurrentStateIMC)
	{
		Subsystem->RemoveMappingContext(CurrentStateIMC);
		CurrentStateIMC = nullptr;
	}

	// 새 상태 IMC만 추가
	if (DesiredStateIMC)
	{
		Subsystem->AddMappingContext(DesiredStateIMC, StateIMCPriority);
		CurrentStateIMC = DesiredStateIMC;
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = ViewPitchMin;
		PlayerCameraManager->ViewPitchMax = ViewPitchMax;
	}

	// 현재 PlayerController에 연결된 Local Player 객체를 가져옴
	ULocalPlayer* player = GetLocalPlayer();
	if (!player) return;

	// Local Player에서 EnhancedInputLocalPlayerSubsystem을 얻어옴
	UEnhancedInputLocalPlayerSubsystem* Subsystem = player->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;
	if (!IdleIMC) return;

	// Subsystem을 통해 할당한 InputMappingContext를 활성화
	Subsystem->AddMappingContext(IdleIMC, 0); // Priority : 0 -> 우선순위가 가장 높음
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Move : 키를 뗄때 0이 들어올 수 있도록 Completed와 Canceled 추가
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
		                                   &AMyPlayerController::Input_Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this,
		                                   &AMyPlayerController::Input_Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this,
		                                   &AMyPlayerController::Input_Move);

		// Look
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
		                                   &AMyPlayerController::Input_Look);

		// Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyPlayerController::Input_Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
		                                   &AMyPlayerController::Input_StopJump);

		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this,
		                                   &AMyPlayerController::Input_StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this,
		                                   &AMyPlayerController::Input_StopSprint);

		// Roll
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &AMyPlayerController::Input_Roll);

		// Zoom
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this,
		                                   &AMyPlayerController::Input_StartAim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this,
		                                   &AMyPlayerController::Input_StopAim);

		// Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this,
		                                   &AMyPlayerController::Input_StartAttack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this,
		                                   &AMyPlayerController::Input_StopAttack);
	}
}

void AMyPlayerController::Input_Move(const FInputActionValue& Value)
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->Move(Value); // 캐릭터의 Move 함수 호출
	}
}

void AMyPlayerController::Input_Look(const FInputActionValue& Value)
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->Look(Value);
	}
}

void AMyPlayerController::Input_Jump()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->Jump();
	}
}

void AMyPlayerController::Input_StopJump()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->StopJumping();
	}
}

void AMyPlayerController::Input_StartSprint()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->StartSprint(FInputActionValue()); // 매개변수 필요시 전달
	}
}

void AMyPlayerController::Input_StopSprint()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->StopSprint(FInputActionValue());
	}
}

void AMyPlayerController::Input_Roll()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->Roll();
	}
}

void AMyPlayerController::Input_StartAim()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->SetAiming(true);
	}
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("StartAim"));
}

void AMyPlayerController::Input_StopAim()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		ControlledChar->SetAiming(false);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("StopAim"));
	}
}

void AMyPlayerController::Input_StartAttack()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		if (UCombatComponent* Combat = ControlledChar->GetCombatComponent())
		{
			Combat->ProcessAttack(true);
		}
	}
}

void AMyPlayerController::Input_StopAttack()
{
	if (APlayerCharacter* ControlledChar = Cast<APlayerCharacter>(GetPawn()))
	{
		if (UCombatComponent* Combat = ControlledChar->GetCombatComponent())
		{
			Combat->ProcessAttack(false);
		}
	}
}
