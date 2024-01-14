// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/UnrealMathUtility.h"
#include "SimplexNoiseGeneration.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALTERRAIN_API USimplexNoiseGeneration : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USimplexNoiseGeneration();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Function used in blueprint to generate noisemap
	UFUNCTION(BlueprintCallable)
	TArray<float> GenerateSimplexNoise(int widthHeight, FVector2D offset, float scale, int octaves, float persistance, float lacunarity, UPrimitiveComponent* mesh);

private:
	float SimplexNoise2D(const FVector2D& location);
	//Simplex constants
	float m_F2;
	float m_G2;
};
