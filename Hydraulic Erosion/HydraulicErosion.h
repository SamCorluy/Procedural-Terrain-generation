// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HydraulicErosion.generated.h"

//Structure used for raindrops
USTRUCT(BlueprintType)
struct FRainDrop
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Velocity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Water = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Sediment = 0.f;
};

//structure used for heightgradient
USTRUCT(BlueprintType)
struct FHeightGradient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float gradientX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float gradientY;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALTERRAIN_API UHydraulicErosion : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHydraulicErosion();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//variables that influence hydraulic erosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_Inertia{ .05f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_Capacity{ 4.f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_MinCapacity{ .01f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_Deposition{ .3f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_Erosion{ .3f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_Evaporation{ .01f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	int m_MaxPath{ 30 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_Gravity{ 4.f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_Radius{ 3.f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_MinSlope{ 0.01f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	int m_IterateAmount{ 7000 };
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// function called in blueprint that returns eroded terrain heightmap
	UFUNCTION(BlueprintCallable, Category = "Procedural Mesh")
	TArray<float> ErodeTerrain(TArray<float> HeightmapData);

	// Helperfunctions that convert x y coordinates to list index and vise versa
	FVector2D posToXY(int position, int arraySize);
	int XYToPos(FVector2D position, int arraySize);

	// Helper function that gets heightgradient
	FHeightGradient CalcHeightGradient(TArray<float> map, int dimensions, float posX, float posY);

	// Helper variables, these are lists that contain all neighboring cells within a radius per index
	std::vector<std::vector<int>> erosionBrushIndices;
	std::vector<std::vector<float>> erosionBrushWeights;

	// Helper function that fills these helper lists
	void InitializeBrushIndices(int mapSize, int radius);
};
