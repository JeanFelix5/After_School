// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveDirectlyToward.h"
#include "BTTask_MoveToward_FloatingChase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTWATER_API UBTTask_MoveToward_FloatingChase : public UBTTask_MoveDirectlyToward
{
	GENERATED_BODY()

public:
	UBTTask_MoveToward_FloatingChase(const FObjectInitializer& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
