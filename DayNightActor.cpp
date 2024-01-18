// Fill out your copyright notice in the Description page of Project Settings.


#include "DayNight/DayNightActor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"

// Sets default values
ADayNightActor::ADayNightActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ADayNightActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADayNightActor::NewWaveWeather()
{
	//if the wave counter is greater than the size of the array, I reset the counter to 0.
	if(WaveEnumCounter > Nightmare)
	{
		WaveEnumCounter = 0;
	}
	
	PreviousSunAngle = SunAngle;
	
	//set the new sun angle depending of the wave from the enum array
	switch (WaveEnumCounter)
	{
		case FirstWave:
			SunAngle = WavesBetweenNightmares[FirstWave];
			break;
		case SecondWave:
			SunAngle = WavesBetweenNightmares[SecondWave];
			if(!LanternArray.IsEmpty())
			{
				for (int i = 0; i < LanternArray.Num(); i++)
				{
					LanternArray[i]->OnLanternNeeded();
				}
			}
			break;
		case ThirdWave:
			SunAngle = WavesBetweenNightmares[ThirdWave];
			break;
		case LastWaveBeforeNightmare:
			SunAngle = WavesBetweenNightmares[LastWaveBeforeNightmare];
			break;
		case Nightmare:
			//Nightmare wave
			if(!LanternArray.IsEmpty()){
				for (int i = 0; i < LanternArray.Num(); i++) { LanternArray[i]->OnLanternDisapear(); }
			}
			SunAngle = 0;
			++WaveEnumCounter;

			if(DirectionalLight)
			{
				//set the local rotation of the sun
				//I subtract negative sun angle to put the rotation back to 0, before adding an other rotation,
				//or else the new rotation value will be added to the current position and the sun angle will be wrong
				DirectionalLight->AddActorLocalRotation(FRotator(-PreviousSunAngle, 0, 0));	 
			}

			if(SunBP)
			{
				//update the sun direction and position in the sky by calling the sky BP function
				FOutputDeviceNull AR;
				SunBP->CallFunctionByNameWithArguments(TEXT("UpdateSunDirection"), AR, NULL, true);
			}
			return;
		
		default:
			return;
	}

	//Calculate the increment from the sun current position to the end position. I multiply the total of time by 10, because I increment every 0,1 seconds.
	SunRotationIncrement = (SunAngle - PreviousSunAngle) / (TotalOfSecondsForMovingSun * 10.0f);
	
	GetWorld()->GetTimerManager().SetTimer(SunUpdateTimer, this, &ADayNightActor::OnSunUpdate, 0.1f,true);

	//increment the wave counter 
	++WaveEnumCounter;
	
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Blue, FString::Printf(TEXT("Current wave number is : %d and the sun angle is %d"), WaveEnumCounter, SunAngle));
}

void ADayNightActor::OnSunUpdate()
{
	ElapsedTime += 0.1f;
	if(ElapsedTime >= TotalOfSecondsForMovingSun)
	{
		GetWorld()->GetTimerManager().ClearTimer(SunUpdateTimer);
		ElapsedTime = 0.0f;
		return;
	}
	
	//Add the new sun rotation from the angle it is currently
	if(DirectionalLight)
	{
		DirectionalLight->AddActorLocalRotation(FRotator(SunRotationIncrement, 0, 0));
	}

	if(SunBP)
	{
		FOutputDeviceNull AR;
		SunBP->CallFunctionByNameWithArguments(TEXT("UpdateSunDirection"), AR, NULL, true);
	}
}



