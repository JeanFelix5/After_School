// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

UCLASS()
class AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractable();

	int32 GetInteractableId();
	int32 GetAmount();

	UFUNCTION(BlueprintNativeEvent)
	void Interact();
	
	void Use();
	
	/*Every interactable must have a mesh*/
	UPROPERTY(EditAnywhere,Category="Interactable Properties")
	class UStaticMeshComponent* InteractableMesh;

	/*Every interactable must have a help text for the player*/
	UPROPERTY(EditAnywhere,Category="Interactable Properties")
	FString InteractableHelpText;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	class USoundBase* TakeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interactable Properties|Spawn Animation")
	bool bHasSpawnAnimation = false;

	// Animation start location
	UPROPERTY(BlueprintReadWrite)
	FVector  StartLocation;

	// Animation end location
	UPROPERTY(BlueprintReadWrite)
	FVector  EndLocation;

	// Animation current location
	UPROPERTY(BlueprintReadWrite)
	FVector  CurrentLocation;

	//Max distance the collectable can travel to when moving to a random position after being spawned into the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interactable Properties|Spawn Animation")
	float MaxDistanceToTravel = 150.0f;

	// Sphere trace vectors
	UPROPERTY(EditDefaultsOnly,Category="Placement")
	FVector StartTraceDetectionOffset = FVector(GetActorLocation().X, GetActorLocation().Y, 100.f);

	UPROPERTY(EditDefaultsOnly,Category="Placement")
	FVector EndTraceDetectionOffset = FVector(GetActorLocation().X, GetActorLocation().Y,-100.f);

	UPROPERTY(EditDefaultsOnly,Category="Placement")
	FVector TraceDetectionWidth = FVector(100.f,100.f,100.f);

	//Calculate random destination around the spawned location
	UFUNCTION(BlueprintCallable)
	void CalculateRandomDestination();

	//Verify if the end destination isn't in the fountain
	UFUNCTION(BlueprintCallable)
	void IsEndLocationInFountain();

	//Verify if the end destination of the animation is valid
	UFUNCTION()
	void IsEndLocationReachable();

	//Get the ground Z location with a sphere trace
	UFUNCTION(BlueprintCallable)
	void GetGroundPosition();
	
	UPROPERTY()
	float GroundLocation = 60.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,Category="Interactable Properties")
	int32 InteractableId=0;

	UPROPERTY(EditAnywhere, Category="Interactable Properties")
	int32 InteractableAmount=1;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Interactable Properties")
	float TimeBeforeDestroy=10.f;

};
