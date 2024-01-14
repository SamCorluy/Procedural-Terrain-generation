// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThermalErosion.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALTERRAIN_API UThermalErosion : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UThermalErosion();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	float m_MaxAngle{ .1f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion settings")
	int m_IterateAmount{ 500 };
	TArray<float> m_HeightmapData;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// function used in blueprint to erode terrain
	UFUNCTION(BlueprintCallable, Category = "Procedural Mesh")
	TArray<float> ErodeTerrain(TArray<float> HeightmapData);

	// helper function that gets lowest neighbor
	int getLowestNeighbor(int currentIndex, int heightMapDimention);
	// helper function that provides terrain sorted by height
	TArray<int> terrainByHeight();
	// helper function used by the sort function
	bool HeightComparison(const int32& A, const int32& B) const;
};
