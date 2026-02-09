// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/PalAIController.h"

#include "BehaviorTree/BlackboardComponent.h"

void APalAIController::BeginPlay()
{
	Super::BeginPlay();
}

void APalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (!BT || !InPawn)
	{
		return;
	}
	
	if (BT)
	{
		RunBehaviorTree(BT);
	}
	
	// Blackboard 가져와서 HomeLocation 세팅
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(TEXT("HomeLocation"), InPawn->GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PalAIController : BlackboardComponent is null"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[PalAI] Set HomeLocation = %s"), *InPawn->GetActorLocation().ToString());
}
