// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PalAIController.generated.h"

class UBehaviorTree;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
enum class EPalMoveState : uint8;

UCLASS()
class PROJECTPAL_API APalAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	APalAIController();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	// 타겟 지정
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// Blackboard helper
	void SetTargetActor(AActor* NewTarget);
	AActor* GetTargetActor() const;
	
public:
	UPROPERTY(EditDefaultsOnly, Category="AI")
	UBehaviorTree* BT;
	
private:
	// Blackboard 키 이름(통일)
	static const FName KEY_PalMoveState;

	// Blackboard 변경 콜백
	EBlackboardNotificationResult OnMoveStateChanged(const UBlackboardComponent& BBComp, FBlackboard::FKey ChangedKeyID);

	void ApplyMoveStateToPawn(EPalMoveState NewState) const;
};
