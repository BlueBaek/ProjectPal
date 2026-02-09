// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PalAIController.generated.h"

/**
 * 
 */
class UBehaviorTree;

UCLASS()
class PROJECTPAL_API APalAIController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
public:
	UPROPERTY(EditDefaultsOnly, Category="AI")
	UBehaviorTree* BT;
	
};
