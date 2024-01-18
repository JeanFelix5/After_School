// Fill out your copyright notice in the Description page of Project Settings.


#include "Creator/Items/CreationItems/PW_RocketCreation.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/PWEnemyMovementComponent.h"
#include "Characters/Enemies/Controllers/BlackboardKeys.h"
#include "Characters/Enemies/Controllers/PWEnemyController.h"
#include "Kismet/GameplayStatics.h"

APW_RocketCreation::APW_RocketCreation()
{
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Box"));
	CollisionSphere->SetupAttachment(Mesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box verification"));
	CollisionBox->SetupAttachment(CollisionSphere);
}

void APW_RocketCreation::BeginPlay()
{
	Super::BeginPlay();

	//Set a countdown timer with the specified time, will destroy the rocket at the end and remove the current behaviors 
	GetWorld()->GetTimerManager().SetTimer(RocketTimer, this, &APW_RocketCreation::LaunchRocket, SecondsBeforeRocketLaunch, false);

	//Call this function with a little delay to prevent an error where the detection of the overlapping actors would always fail on spawn
	GetWorld()->GetTimerManager().SetTimer(VerificationTimer, this, &APW_RocketCreation::VerifyEnemyAlreadyInside, 0.1f, false);
	
	//delegates functions
	CollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &APW_RocketCreation::OnBeginOverlap);
	CollisionSphere->OnComponentEndOverlap.AddUniqueDynamic(this, &APW_RocketCreation::OnEndOverlap);

	//add a rocket to the total for moon jump
	AddRocketToActiveRocketCounter();
	
	if(bCanPlayerMoonJump == true)
	{
		//call the moon jump function
		PlayerMoonJump();
	}
}

void APW_RocketCreation::VerifyEnemyAlreadyInside()
{
	//Verify if there is enemies that are already inside the rocket collision sphere when the rocket is crafted/spawned
	
	//Get all the actors of the enemy character class that are overlapping the sphere collision
	TArray<AActor*> EnemyAlreadyInsideArray;
	this->CollisionSphere->GetOverlappingActors(EnemyAlreadyInsideArray, APWEnemyCharacter::StaticClass());
	
	//For each actors in the array, verify if it can be cast to enemy character and that it doesn't return nullptr
	for(int i = 0; i < EnemyAlreadyInsideArray.Num(); i++)
	{
		if(APWEnemyCharacter* EnemyCharacter = Cast<APWEnemyCharacter>(EnemyAlreadyInsideArray[i]); EnemyCharacter != nullptr) 
		{
			if(EnemyCharacter->bEnemyAlreadyInsideOnCraft == false){	//if the enemy wasn't overlapping on spawn
				
				EnemyCharacter->SetIfEnemyAlreadyInsideRocketZone(true);	//set it to true and add a new rocket to the counter or else the counter won't increment
				EnemyCharacter->setNumberOfOverlappingRocket(++EnemyCharacter->NbRocketOverlappingCounter);
			}
			else
			{
				//if the enemy was already in the zone on spawn and have spawned a new rocket on top of it, I just increment the counter
				EnemyCharacter->setNumberOfOverlappingRocket(++EnemyCharacter->NbRocketOverlappingCounter);	//add a new overlapping rocket to the counter
			}
		}
	}

	EnemyAlreadyInsideArray.Empty();

	//clear the little delay timer
	GetWorld()->GetTimerManager().ClearTimer(VerificationTimer);
	
	//indicate that the delay for detection is over
	IsOverlappingInitialDelayOver = true;
}

void APW_RocketCreation::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(APWEnemyCharacter* EnemyCharacter = Cast<APWEnemyCharacter>(OtherActor))	//collision with an enemy
	{
		//Verify if the enemy is valid, and the begin overlap is not being called while the rocket is in the process of being destroyed
		if (OtherActor && OtherActor->IsValidLowLevel() && !this->IsPendingKillPending())
		{
			if(bIsRocketDestroyed == true)
			{
				//return here or will get a weird bug where a lot of begin overlap would be called after the rocket has been destroyed
				//if the enemy is on the edge of the bubble
				return;
			}
			
			//if the enemy is already in the gravity zone and have entered a new gravity zone, I add 1 to the total of overlapping rockets
			if(EnemyCharacter->bIsInGravityZone == true) 
			{
				//If the delay is not over, return or else there will a weird bug
				if(IsOverlappingInitialDelayOver == false)
				{
					return;
				}
			
				EnemyCharacter->setNumberOfOverlappingRocket(++EnemyCharacter->NbRocketOverlappingCounter);	//add a new overlapping rocket to the counter
				//GEngine->AddOnScreenDebugMessage(0,2.0f, FColor::Red,FString::Printf(TEXT("Added a new rocket %d"), EnemyCharacter->NbRocketOverlappingCounter));
				return;
			}
		
			if(IsOverlappingInitialDelayOver == false) //If the enemy is already in the rocket and not in the air and the timer isn't finished.
			{
				//Set it to 0 to prevent an off-by-one error when I increment it for the first time in the verification function
				EnemyCharacter->setNumberOfOverlappingRocket(0);
			}
			else
			{
				//else set it to 1 because the actor is in the first rocket in the counter
				EnemyCharacter->setNumberOfOverlappingRocket(1);
			}
		
			//make the enemy start to float
			UPWEnemyMovementComponent* MovementComponent = Cast<UPWEnemyMovementComponent>(EnemyCharacter->GetCharacterMovement());
			checkf(MovementComponent, TEXT("Failed to cast enemy movement component"));

			MovementComponent->Velocity = FVector::ZeroVector;
			MovementComponent->SetUseAccelerationForPaths(false);
			MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);		//Change the movement mode
			MovementComponent->MaxWalkSpeed = EnemyGravityZoneSpeed;			//Set the new speed in the zone
			MovementComponent->GravityScale = EnemyGravityScale;				//Set the new gravity scale
			MovementComponent->AirControl = 1.0f;
			EnemyCharacter->bIsInGravityZone = true;

			//Launch the character in the air or he won't move up.
			EnemyCharacter->LaunchCharacter(FVector(0,0, 10), false, true);

			if (APWEnemyController* AIController = Cast<APWEnemyController>(EnemyCharacter->GetController()); AIController != nullptr)
			{
				//Activate the new state of the AI with the blackboard key
				AIController->GetBlackboard()->SetValueAsBool(BBKeys::GravityEnabled, true);
			}
		}
	}
	else //if collision event isn't an enemy
	{
		//player enter collision
		if(APWPlayerCharacter* Player = Cast<APWPlayerCharacter>(OtherActor))
		{
			//Make the player float 
			if(bCanPlayerFloatInRocketZone == true)
			{
				//If the player is already flying in the gravity zone, add a new overlapping rocket to the counter
				if(Player->bIsPlayerFlyingInGravityZone == true) 
				{
					Player->setNumberOfOverlappingRocketForPlayer(++Player->NbRocketOverlappingCounter);	//add a new overlapping rocket to the counter
					return;
				}

				//If the player is currently only in one gravity zone, set the counter to 1
				Player->setNumberOfOverlappingRocketForPlayer(1);

				//Change the gravity
				Player->GetCharacterMovement()->Velocity = FVector::ZeroVector;
				Player->GetCharacterMovement()->GravityScale = GravityScaleForPlayer;
				Player->GetCharacterMovement()->AirControl = AirControlPlayer;
				Player->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

				//Launch him in the air to give him an initial push to make him leave the ground
				Player->LaunchCharacter(FVector(0,0, 5), false, true);

				//If the player is flying
				if(Player->GetCharacterMovement()->IsFlying() == true)
				{
					//The player is currently flying
					Player->bIsPlayerFlyingInGravityZone = true;

					//set the velocity to the floating player when he moves up (same when he moves down)
					Player->GravityZoneZVelocity = ZVelocityFloatingPlayer;

					//add more deceleration while is floating in the air to make the movement easier. (Adding "air" friction when moving in the zone)
					Player->GetCharacterMovement()->BrakingDecelerationFalling = RocketMovementFriction;
				}
			}
		}
	}
}

void APW_RocketCreation::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//if the other actor is an enemy
	if(APWEnemyCharacter* EnemyCharacter = Cast<APWEnemyCharacter>(OtherActor))
	{
		//Verify if the enemy is valid, and the end overlap is not being called while the rocket is in the process of being destroyed
		if (OtherActor && OtherActor->IsValidLowLevel() && !this->IsPendingKillPending())
		{
			if(bIsRocketDestroyed == true)
			{
				//return here or will get a weird bug where a lot of end overlap would be called after the rocket has been destroyed
				return;
			}
			
			//if the number of overlapping rockets is higher than 1, it means that the character is currently in multiple gravity zones
			if(EnemyCharacter->NbRocketOverlappingCounter > 1)
			{
				if(IsOverlappingInitialDelayOver == false)	//If the initial delay isn't over return (just to be sure)
				{
					return;
				}
				
				//Since he is exiting a zone, I remove a rocket from the total counter of overlapping rockets
				EnemyCharacter->setNumberOfOverlappingRocket(--EnemyCharacter->NbRocketOverlappingCounter);
				//GEngine->AddOnScreenDebugMessage(0,2.0f, FColor::Green,FString::Printf(TEXT("Enemy is still in the bounds of another rocket %d"), EnemyCharacter->NbRocketOverlappingCounter));
				return;
			}
			
			//If the total of overlapping rocket is set to 0, it means the character have exited all the gravity zones, so the normal behavior can execute
			//If it is set to 1 and is doing an end overlap, it means the enemy is leaving the zone of his last active rocket

			EnemyCharacter->setNumberOfOverlappingRocket(0); //set it to 0 because the enemy return to his normal behavior
			
			//Make the enemy stop floating
			UPWEnemyMovementComponent* MovementComponent = Cast<UPWEnemyMovementComponent>(EnemyCharacter->GetCharacterMovement());
			checkf(MovementComponent, TEXT("Failed to cast enemy movement component"));

			//Launch the character in the air or he won't exit the zone properly if he is on top of the collision sphere
			EnemyCharacter->LaunchCharacter(FVector(170.0 * EnemyCharacter->GetActorForwardVector().X ,
				170.0 * EnemyCharacter->GetActorForwardVector().Y, 10), false, true);

			//Reset the parameters to the ones by default
			MovementComponent->SetUseAccelerationForPaths(true);
			MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
			MovementComponent->MaxWalkSpeed = EnemyCharacter->GetMovementSpeed();
			MovementComponent->GravityScale = 1.0f;
			MovementComponent->AirControl = 0.2f;
			EnemyCharacter->bIsInGravityZone = false;

			if (const APWEnemyController* AIController = Cast<APWEnemyController>(EnemyCharacter->GetController()); AIController != nullptr)
			{
				//Deactivate the new state of the AI with the blackboard key
				AIController->GetBlackboard()->SetValueAsBool(BBKeys::GravityEnabled, false);
			}

			//if the enemy was already in the zone when the rocket spawned, set the boolean to false because the enemy is now out of the zone
			if(EnemyCharacter->bEnemyAlreadyInsideOnCraft == true)	
			{
				EnemyCharacter->SetIfEnemyAlreadyInsideRocketZone(false);
			}
		}
	}
	else //if collision event isn't an enemy
	{
		//player exit collision
		if(APWPlayerCharacter* Player = Cast<APWPlayerCharacter>(OtherActor))
		{
			//Make the player stop floating 
			if(bCanPlayerFloatInRocketZone == true)
			{
				//Prevent the case where this would be executed if the player is inside the sphere when the rocket is destroyed
				if(bIsRocketDestroyed == true)
				{
					//return here or will get a weird bug where a lot of end overlap would be called after the rocket has been destroyed
					return;
				}

				//If the player rocket counter is greater than one, I remove one rocket from the counter and return
				if(Player->NbRocketOverlappingCounter > 1)
				{
					//Since he is exiting a zone, I remove a rocket from the total counter
					Player->setNumberOfOverlappingRocketForPlayer(--Player->NbRocketOverlappingCounter);
					return;
				}

				//If the player was in only 1 rocket (according to the counter) and is currently leaving the zone. I set the counter to 0, because
				//I am now setting the player back to his normal behavior.
				Player->setNumberOfOverlappingRocketForPlayer(0);

				//Put back the normal gravity and jump velocity 
				Player->GetCharacterMovement()->AirControl = 0.2f;
				Player->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
				Player->GetCharacterMovement()->BrakingDecelerationFalling = 0.0f;

				if(Player->NumberOfActiveRocket > 0 && bCanPlayerMoonJump == true)
				{
					Player->GetCharacterMovement()->JumpZVelocity = NewPlayerMoonJumpZVelocity;
					Player->GetCharacterMovement()->GravityScale = NewPlayerMoonJumpGravityScale;
				}
				else
				{
					Player->GetCharacterMovement()->JumpZVelocity = 420.0f;
					Player->GetCharacterMovement()->GravityScale = 1.0f;
				}

				//Launch the character so that he leaves the zone effectively
				Player->LaunchCharacter(FVector(10.0 * Player->GetActorForwardVector().X,
				10.0 * Player->GetActorForwardVector().Y, 10), false, true);

				//Set the is flying condition to false, because the player is back on foot
				if(Player->GetCharacterMovement()->IsFlying() == false)
				{
					Player->bIsPlayerFlyingInGravityZone = false;
				}
			}
		}
	}
}

void APW_RocketCreation::LaunchRocket()
{
	//reset the boolean of the first timer and indicate that the rocket is in the process of being destroyed
	IsOverlappingInitialDelayOver = false;	
	bIsRocketDestroyed = true;
	bool bLastRocket = false;
	
	//Get all the actors of the enemy character class that are overlapping the sphere collision
	TArray<AActor*> EnemyArray;
	this->CollisionSphere->UpdateOverlaps();	//Update the overlaps
	this->CollisionSphere->GetOverlappingActors(EnemyArray, APWEnemyCharacter::StaticClass());
	
	//For each actors in the array, verify if it can be cast to enemy character and that it doesn't return nullptr
	for(int i = 0; i < EnemyArray.Num(); i++)
	{
		if(APWEnemyCharacter* EnemyCharacter = Cast<APWEnemyCharacter>(EnemyArray[i]); EnemyCharacter != nullptr) 
		{
			//If the timer is finished and the enemy is still in the gravity zone
 			if(EnemyCharacter->bIsInGravityZone == true && EnemyCharacter->NbRocketOverlappingCounter > 0)	
 			{
 				//if the enemy is in his last gravity zone, reset his behavior to normal
 				if(EnemyCharacter->NbRocketOverlappingCounter == 1)
 				{
 					EnemyCharacter->setNumberOfOverlappingRocket(0); //to indicate he is back on the ground
				
 					//Make the enemy stop floating
 					UPWEnemyMovementComponent* MovementComponent = Cast<UPWEnemyMovementComponent>(EnemyCharacter->GetCharacterMovement());
 					checkf(MovementComponent, TEXT("Failed to cast enemy movement component"));

 					//Reset the parameters to the ones by default
 					MovementComponent->SetUseAccelerationForPaths(true);
 					MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
 					MovementComponent->MaxWalkSpeed = EnemyCharacter->GetMovementSpeed();
 					MovementComponent->GravityScale = 1.0f;
 					MovementComponent->AirControl = 0.2f;
 					EnemyCharacter->bIsInGravityZone = false;								//reset the boolean of the enemy to false

 					if (const APWEnemyController* AIController = Cast<APWEnemyController>(EnemyCharacter->GetController()); AIController != nullptr)
 					{
 						//Deactivate the new state of the AI with the blackboard key
 						AIController->GetBlackboard()->SetValueAsBool(BBKeys::GravityEnabled, false);
 					}
 				}
 				else
 				{
 					//If the enemy is inside more than one overlapping rocket and the current one is being destroyed
 					//Remove one from the rocket counter or else the enemy won't be up to date with the counter
 					EnemyCharacter->setNumberOfOverlappingRocket(--EnemyCharacter->NbRocketOverlappingCounter);
 				}
 			}
		}
	}

	//Remove the rocket from the total
	RemoveRocketToActiveRocketCounter();

	//Get all the actors of the player character class that are overlapping the sphere collision
	TArray<AActor*> PlayerArray;
	this->CollisionSphere->UpdateOverlaps();	//Update the overlaps
	this->CollisionSphere->GetOverlappingActors(PlayerArray, APWPlayerCharacter::StaticClass());
	
	//For each actors in the array, verify if it can be cast to player character and that it doesn't return nullptr
	for(int i = 0; i < PlayerArray.Num(); i++)
	{
		if(APWPlayerCharacter* PlayerCharacter = Cast<APWPlayerCharacter>(PlayerArray[i]); PlayerCharacter != nullptr) 
		{
			if(PlayerCharacter->bIsPlayerFlyingInGravityZone == true)	//If the timer is finished and the player is still in the gravity zone
			{
				//If the player is in his last gravity zone, reset his behavior to normal
				if(PlayerCharacter->NbRocketOverlappingCounter == 1)
				{
					PlayerCharacter->setNumberOfOverlappingRocketForPlayer(0); //to indicate he is back on the ground
					
					//reset default parameters for the player (like in the OnEndOverlap)
					PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = 650.0f;
					PlayerCharacter->GetCharacterMovement()->JumpZVelocity = 420.0f;
					PlayerCharacter->GetCharacterMovement()->GravityScale = 1.0f;
					PlayerCharacter->GetCharacterMovement()->AirControl = 0.2f;
					PlayerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
					PlayerCharacter->GetCharacterMovement()->BrakingDecelerationFalling = 0.0f;

					if(PlayerCharacter->NumberOfActiveRocket > 0 && bCanPlayerMoonJump == true)
					{
						PlayerCharacter->GetCharacterMovement()->JumpZVelocity = NewPlayerMoonJumpZVelocity;
						PlayerCharacter->GetCharacterMovement()->GravityScale = NewPlayerMoonJumpGravityScale;
					}
					else
					{
						PlayerCharacter->GetCharacterMovement()->JumpZVelocity = 420.0f;
						PlayerCharacter->GetCharacterMovement()->GravityScale = 1.0f;
					}
					
					if(PlayerCharacter->GetCharacterMovement()->IsFlying() == false)
					{
						PlayerCharacter->bIsPlayerFlyingInGravityZone = false;
					}
				}
				else
				{
					//If the player is inside more than one overlapping rocket and the current one is being destroyed
					//Remove one from the rocket counter or else the player won't be up to date with the counter
					PlayerCharacter->setNumberOfOverlappingRocketForPlayer(--PlayerCharacter->NbRocketOverlappingCounter);
				}
			}

			if(PlayerCharacter->NumberOfActiveRocket == 0)
			{
				bLastRocket = true;
			}
		}
	}
	
	// Clear the timer
	GetWorld()->GetTimerManager().ClearTimer(RocketTimer);
	
	//Inform the player that the rocket has been deleted
	//GEngine->AddOnScreenDebugMessage(0,3.0f, FColor::Black,"Rocket launched in the sky");
	
	//When the rocket is launched, reset the normal jump to the player
	if(bCanPlayerMoonJump == true)
	{
		PlayerNormalJump();
	}

	if(bLastRocket == true)
	{
		//Ensure that all the enemies are back to normal gravity with the last rocket being destroyed
		
		//Get all the actors of the enemy character class that are overlapping the box collision
		TArray<AActor*> LastRocketEnemyArray;
		this->CollisionBox->UpdateOverlaps();
		this->CollisionBox->GetOverlappingActors(EnemyArray, APWEnemyCharacter::StaticClass());
		
		//For each actors in the array, verify if it can be cast to enemy character and that it doesn't return nullptr
		for(int i = 0; i < EnemyArray.Num(); i++)
		{
			if(APWEnemyCharacter* EnemyCharacter = Cast<APWEnemyCharacter>(EnemyArray[i]); EnemyCharacter != nullptr) 
			{
				//If the timer is finished and the enemy is still in the gravity zone
 				if(EnemyCharacter->bIsInGravityZone == true)	
 				{
 					EnemyCharacter->setNumberOfOverlappingRocket(0); //to indicate he is back on the ground
				
 					//Make the enemy stop floating
 					UPWEnemyMovementComponent* MovementComponent = Cast<UPWEnemyMovementComponent>(EnemyCharacter->GetCharacterMovement());
 					checkf(MovementComponent, TEXT("Failed to cast enemy movement component"));

 					//Reset the parameters to the ones by default
 					MovementComponent->SetUseAccelerationForPaths(true);
 					MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
 					MovementComponent->MaxWalkSpeed = EnemyCharacter->GetMovementSpeed();
 					MovementComponent->GravityScale = 1.0f;
 					MovementComponent->AirControl = 0.2f;
 					EnemyCharacter->bIsInGravityZone = false;								//reset the boolean of the enemy to false

 					if (const APWEnemyController* AIController = Cast<APWEnemyController>(EnemyCharacter->GetController()); AIController != nullptr)
 					{
 						//Deactivate the new state of the AI with the blackboard key
 						AIController->GetBlackboard()->SetValueAsBool(BBKeys::GravityEnabled, false);
 					}
 				}
			}
		}
	}
	
	
	//Destroy the rocket actor
	this->Destroy();
}

void APW_RocketCreation::AddRocketToActiveRocketCounter()
{
	auto* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if(APWPlayerCharacter* Player = Cast<APWPlayerCharacter>(PlayerCharacter))
	{
		Player->setNumberOfRocket(++Player->NumberOfActiveRocket);	//add a rocket to the total of active rocket for the player
	}
}

void APW_RocketCreation::RemoveRocketToActiveRocketCounter()
{
	auto* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if(APWPlayerCharacter* Player = Cast<APWPlayerCharacter>(PlayerCharacter))
	{
		Player->setNumberOfRocket(--Player->NumberOfActiveRocket);	//remove a rocket to the total of active rocket for the player
	}
}

void APW_RocketCreation::PlayerMoonJump()
{
	auto* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if(APWPlayerCharacter* Player = Cast<APWPlayerCharacter>(PlayerCharacter))
	{
		Player->GetCharacterMovement()->JumpZVelocity = NewPlayerMoonJumpZVelocity;
		Player->GetCharacterMovement()->GravityScale = NewPlayerMoonJumpGravityScale;
		//GEngine->AddOnScreenDebugMessage(0,2.0f, FColor::Blue,"Player moon jump activated");
	}
}

void APW_RocketCreation::PlayerNormalJump()
{
	auto* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if(APWPlayerCharacter* Player = Cast<APWPlayerCharacter>(PlayerCharacter))
	{
		if(Player->NumberOfActiveRocket == 0)
		{
			Player->GetCharacterMovement()->JumpZVelocity = 420.0f;
			Player->GetCharacterMovement()->GravityScale = 1.0f;
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(0,2.0f, FColor::Green,"A rocket is still active right now");
		}
	}
}
