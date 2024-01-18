// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PWLantern.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Actor.h"
#include "DayNightActor.generated.h"

UENUM()
enum WavesBetweenNightmares { FirstWave = 0, SecondWave = 1, ThirdWave = 2, LastWaveBeforeNightmare = 3, Nightmare = 4 };

UCLASS()
class PROJECTWATER_API ADayNightActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADayNightActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	
	UPROPERTY(EditAnywhere, Category="Sky actors")
	TObjectPtr<AActor> SunBP;

	UPROPERTY(EditAnywhere, Category="Sky actors")
	TObjectPtr<ADirectionalLight> DirectionalLight;
	
	UPROPERTY(EditAnywhere, Category="Sky actors")
	int32 WavesBetweenNightmares[Nightmare];
	
	//Total of seconds the timer has to move the sun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sky actors")
	float TotalOfSecondsForMovingSun = 5.0f;

	UPROPERTY()
	int SunAngle = 0;
	
	UFUNCTION(BlueprintCallable)
	void NewWaveWeather();

	UFUNCTION()
	void OnSunUpdate();

	UPROPERTY()
	int WaveEnumCounter = 0;

	UPROPERTY()
	int PreviousSunAngle = 0;

	UPROPERTY()
	FTimerHandle SunUpdateTimer;

	UPROPERTY()
	float SunRotationIncrement = 0;

	UPROPERTY()
	float ElapsedTime = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	TArray<APWLantern*> LanternArray;

};
