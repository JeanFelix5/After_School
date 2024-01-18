// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactor/Interactable.h"

#include "Engine/StaticMeshActor.h"
#include "Characters/Player/WaterSystem/PWFountain.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"


// Sets default values
AInteractable::AInteractable()
{
	InteractableHelpText=FString("Press E to interact with item");
}

int32 AInteractable::GetInteractableId()
{
	return InteractableId;
}

int32 AInteractable::GetAmount()
{
	return InteractableAmount;
}

void AInteractable::Interact_Implementation()
{
	this->Destroy();
}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	Super::BeginPlay();

	if(bHasSpawnAnimation == true)
	{
		StartLocation = GetActorLocation();
		CurrentLocation = StartLocation;

		//Calculate random destination around the spawned location
		CalculateRandomDestination();

		//Verify if the end destination isn't in the fountain and is valid
		IsEndLocationInFountain();

		//Get the ground Z location with a sphere trace
		GetGroundPosition();
	}
}

void AInteractable::CalculateRandomDestination()
{
	// Calculate a random angle in radians
	const float RandomAngle = FMath::RandRange(0.0f, 1.0f) * 2.0f * PI;
	
	// Create a new vector based on the angle and fixed distance
	const FVector RandomVector = FVector(MaxDistanceToTravel * FMath::Cos(RandomAngle), MaxDistanceToTravel * FMath::Sin(RandomAngle), GetActorLocation().Z);
	
	// Define the new random location where the candy need to go
	EndLocation = StartLocation + RandomVector;
}

void AInteractable::IsEndLocationInFountain()
{
	// create tarray for hit results
	TArray<FHitResult> OutHits;
	
	// start and end locations
	const FVector SweepStart = StartTraceDetectionOffset;
	const FVector SweepEnd = FVector(EndLocation.X, EndLocation.Y, -100.0f);

	// create a collision sphere
	const FCollisionShape ColSphere = FCollisionShape::MakeBox(FVector(50.f,50.f,100.f));

	// ignoring self for the collision detection
	const FCollisionQueryParams Params = FCollisionQueryParams();
	
	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, SweepStart, SweepEnd, FQuat::Identity, ECC_Destructible, ColSphere,Params);

	if (isHit)
	{
		for(auto& Hit : OutHits)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				//Check if the HitActor is a fountain
				if(APWFountain* Fountain = Cast<APWFountain>(HitActor))
				{
					//The candy end location is in the fountain. That is why I am giving it a new location to prevent this
					//GEngine->AddOnScreenDebugMessage(-1,10.0f, FColor::Orange,"Fountain found with candy!" + HitActor->GetName());

					// Calculate a random angle in radians
					const float RandomAngle = FMath::RandRange(0.0f, 1.0f) * 2.0f * PI;
	
					// Create a new vector based on the angle and fixed distance
					const FVector RandomVector = FVector(330 * FMath::Cos(RandomAngle), 330 * FMath::Sin(RandomAngle), GetActorLocation().Z);
					const FVector RandomPositionAroundFountain = Fountain->GetActorLocation() + RandomVector;

					//new X and Y end location
					EndLocation.X = RandomPositionAroundFountain.X;
					EndLocation.Y = RandomPositionAroundFountain.Y;
					return;
				}
			}
		}
	}
	
	IsEndLocationReachable();
}

void AInteractable::IsEndLocationReachable()
{
	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// start and end locations
	const FVector SweepStart = StartLocation;
	const FVector SweepEnd = EndLocation;

	// create a collision sphere
	const FCollisionShape ColSphere = FCollisionShape::MakeSphere(15.0f);

	// ignoring self for the collision detection
	const FCollisionQueryParams Params = FCollisionQueryParams();
	
	FHitResult HitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("SphereTrace")), false, nullptr);

	// draw collision sphere
	//DrawDebugSphere(GetWorld(), EndLocation, ColSphere.GetSphereRadius(), 50, FColor::Red, true);

	bool bHitStaticActor = GetWorld()->SweepSingleByObjectType(
		HitResult,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllStaticObjects,
		ColSphere,
		TraceParams
	);

	if (bHitStaticActor)
	{
		if(AActor* HitActor = HitResult.GetActor())
		{
			if(AStaticMeshActor* HitActorProp = Cast<AStaticMeshActor>(HitActor))
			{
				//GEngine->AddOnScreenDebugMessage(-1,15.0f, FColor::Yellow,"static mesh actor: " + HitActor->GetName());

				// Calculate a random angle in radians
				const float RandomAngle = FMath::RandRange(0.0f, 1.0f) * 2.0f * PI;
	
				// Create a new vector based on the angle and fixed distance
				const FVector RandomVector = FVector(70 * FMath::Cos(RandomAngle), 70 * FMath::Sin(RandomAngle), GetActorLocation().Z);
	
				// Define the new random location where the candy need to go
				EndLocation = StartLocation + RandomVector;

				//EndLocation.X = StartLocation.X + 3.0f;
				//EndLocation.Y = StartLocation.Y + 3.0f;
				GroundLocation = GetActorLocation().Z + 10.0f;
			}
			else
			{
				//verify if is not outside the map
				UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
				APlayerController* PC = GetWorld()->GetFirstPlayerController();
				if (!NavSys || !PC)
				{
					return;
				}

				const ANavigationData* NavData = NavSys->GetNavDataForProps(PC->GetNavAgentPropertiesRef());
				if (!NavData)
				{
					return;
				}
	
				FPathFindingQuery Query(PC, *NavData, PC->GetNavAgentLocation(), EndLocation);
				if (NavSys->TestPathSync(Query))
				{
					//on nav mesh
					//GEngine->AddOnScreenDebugMessage(-1,15.0f, FColor::Green,"location on nav mesh");
				}
				else
				{
					//not on nav mesh
					//GEngine->AddOnScreenDebugMessage(-1,15.0f, FColor::Red,"location not on nav mesh");
					
					// Calculate a random angle in radians
					const float RandomAngle = FMath::RandRange(0.0f, 1.0f) * 2.0f * PI;
	
					// Create a new vector based on the angle and fixed distance
					const FVector RandomVector = FVector(70 * FMath::Cos(RandomAngle), 70 * FMath::Sin(RandomAngle), GetActorLocation().Z);
	
					// Define the new random location where the candy need to go
					EndLocation = StartLocation + RandomVector;
					
					//EndLocation.X = StartLocation.X + 3.0f;
					//EndLocation.Y = StartLocation.Y + 3.0f;
					GroundLocation = GetActorLocation().Z + 10.0f;
				}
			}
		}
	}
}

void AInteractable::GetGroundPosition()
{
	/* Check for ground z position */
	
	// create tarray for hit results
	TArray<FHitResult> OutHits;
	
	// start and end locations
	const FVector SweepStart = FVector(EndLocation.X, EndLocation.Y, 100.0f);
	const FVector SweepEnd = FVector(EndLocation.X, EndLocation.Y, -100.0f);

	// create a collision sphere
	const FCollisionShape ColSphere = FCollisionShape::MakeBox(FVector(1.f,1.f,1.f));
	

	// ignoring self for the collision detection
	const FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	
	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, ColSphere,Params);

	if (isHit)
	{
		for(auto& Hit : OutHits)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				if(AStaticMeshActor* ground = Cast<AStaticMeshActor>(HitActor))
				{
					GroundLocation = Hit.ImpactPoint.Z + 65.0f;
					
					//set the height of the ground
					EndLocation.Z = GroundLocation;
					
					//GEngine->AddOnScreenDebugMessage(-1,10.0f, FColor::Green,FString::Printf(TEXT("static mesh ground actor found! to %f"), GroundLocation));
					return;
				}
			}
		}
	}
	
	EndLocation.Z = GroundLocation + 15.0;
	//GEngine->AddOnScreenDebugMessage(-1,10.0f, FColor::Orange,FString::Printf(TEXT("static mesh ground actor found! to %f"), GroundLocation));
}