// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Creator/CreationItem.h"
#include "Components/SphereComponent.h"
#include "Characters/Enemies/PWEnemyCharacter.h"
#include "Characters/Enemies/Controllers/PWEnemyController.h"
#include "PW_RocketCreation.generated.h"

/**
 * Rocket zone that has an influence over gravity.
 */
UCLASS()
class PROJECTWATER_API APW_RocketCreation : public ACreationItem
{
	GENERATED_BODY()

	APW_RocketCreation();

public:
	virtual void BeginPlay() override;

	//Sphere collision that activate the new gravity
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> CollisionBox;
	
	//Total of seconds before the rocket is destroy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket")
	float SecondsBeforeRocketLaunch = 30.0f;

	//The speed of the enemies when they are floating in the air
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket|Enemies")
	float EnemyGravityZoneSpeed = 150.0f;

	//gravity scale to make the enemies smoothly float in the air
	UPROPERTY(VisibleAnywhere, Category="Rocket|Enemies")
	float EnemyGravityScale = -0.002f;
	
	//Player can moon jump as long as one rocket is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket")
	bool bCanPlayerMoonJump = false;

	UFUNCTION(BlueprintCallable)
	void PlayerMoonJump();

	UFUNCTION(BlueprintCallable)
	void PlayerNormalJump();

	//Player can float in the rocket gravity zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket")
	bool bCanPlayerFloatInRocketZone = true;

	UFUNCTION()
	void AddRocketToActiveRocketCounter();

	UFUNCTION()
	void RemoveRocketToActiveRocketCounter();

	//Gravity scale for the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket|Player")
	float GravityScaleForPlayer = 0.001f;

	//Player air control when moving in the air in the gravity zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket|Player")
	float AirControlPlayer = 1.0f;

	//The velocity applied when the player hold the space bar or left shift to move up or down (A or B for controller) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket|Player")
	float ZVelocityFloatingPlayer = 12.0f;

	//The breaking deceleration of the player while moving horizontally with the rocket gravity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket|Player")
	float RocketMovementFriction = 400.0f;

	//Z velocity for the moon jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket|Player", meta=(EditCondition="bCanPlayerMoonJump==true", EditConditionHides))
	float NewPlayerMoonJumpZVelocity = 300.0f;

	//Gravity scale for the player during his moon jump
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rocket|Player", meta=(EditCondition="bCanPlayerMoonJump==true", EditConditionHides))
	float NewPlayerMoonJumpGravityScale = 0.3f;
	
private:

	//delegates
	//On begin overlap with the collision sphere
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//On end overlap with the collision sphere
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Function to get all the enemies inside the gravity zone when the rocket is spawned
	UFUNCTION()
	void VerifyEnemyAlreadyInside();

	//Timer to add a delay to the overlapping actor detection when the rocket is spawned so that we can detect correctly all the overlapping actors on spawn
	UPROPERTY()
	FTimerHandle VerificationTimer;

	//Called at the end of the timer
	UFUNCTION()
	void LaunchRocket();

	//Bool to indicate the end of the initial delay timer for overlapping actors on spawn
	UPROPERTY()
	bool IsOverlappingInitialDelayOver = false;

	//Indicate if the rocket is in the process of being destroyed
	UPROPERTY()
	bool bIsRocketDestroyed = false;

	//Countdown timer before the rocket is destroyed 
	UPROPERTY()
	FTimerHandle RocketTimer;
};