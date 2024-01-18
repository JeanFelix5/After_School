// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemies/AITasks/BTTask_MoveToward_FloatingChase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Characters/Enemies/Controllers/BlackboardKeys.h"
#include "Characters/Enemies/Controllers/PWEnemyController.h"
#include "Kismet/GameplayStatics.h"

UBTTask_MoveToward_FloatingChase::UBTTask_MoveToward_FloatingChase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = TEXT("Chase Player while floating");
}

EBTNodeResult::Type UBTTask_MoveToward_FloatingChase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const APWEnemyController* AIController = Cast<APWEnemyController>(OwnerComp.GetAIOwner());
	checkf(AIController, TEXT("AIController is invalid"));

	const AActor* PlayerActor = Cast<AActor>(AIController->GetBlackboard()->GetValueAsObject(BBKeys::TargetActor));
	//checkf(PlayerActor, TEXT("Failed to cast PlayerObject"));

	if(!PlayerActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Player actor in rocket null");	
		return EBTNodeResult::Failed;
	}
	
	const FVector PlayerLocation = PlayerActor->GetActorLocation();

	APWEnemyCharacter* EnemyCharacter = Cast<APWEnemyCharacter>(AIController->GetPawn());
	if (EnemyCharacter == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	const FVector MyLocation = EnemyCharacter->GetActorLocation();
	FVector Direction = PlayerLocation - MyLocation;
	Direction.Normalize();
	
	// Move towards player
	EnemyCharacter->AddMovementInput(Direction, 1.0f);

	return EBTNodeResult::Succeeded;
}
