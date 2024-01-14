// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerlinNoiseGeneration.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALTERRAIN_API UPerlinNoiseGeneration : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPerlinNoiseGeneration();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Function used in blueprint to generate noisemap
	UFUNCTION(BlueprintCallable)
	TArray<float> GeneratePerlinNoise(int widthHeight, FVector2D offset, float scale, int octaves, float persistance, float lacunarity, UPrimitiveComponent* mesh);
};
