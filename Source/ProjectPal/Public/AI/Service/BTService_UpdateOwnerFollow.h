// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_UpdateOwnerFollow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPAL_API UBTService_UpdateOwnerFollow : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTService_UpdateOwnerFollow();
	
protected:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector DistanceToOwnerKey; // float

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector bShouldFollowKey;   // bool (optional)

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector OwnerLocationKey;   // vector (required if MoveTo uses OwnerLocation)

	UPROPERTY(EditAnywhere, Category="Config")
	float FollowStartDistance = 500.f;

	UPROPERTY(EditAnywhere, Category="Config")
	float FollowStopDistance  = 250.f;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugLog = false;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	void SetOutputsSafe(class UBlackboardComponent* BB, float Distance, bool bFollow, const FVector& OwnerLoc) const;
};
