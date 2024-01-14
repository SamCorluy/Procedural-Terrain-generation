// Fill out your copyright notice in the Description page of Project Settings.


#include "ThermalErosion.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UThermalErosion::UThermalErosion()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UThermalErosion::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UThermalErosion::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

TArray<float> UThermalErosion::ErodeTerrain(TArray<float> HeightmapData)
{
	// calculate width/height of map
	int heightmapDimension = FMath::Sqrt(static_cast<float>(HeightmapData.Num()));

	// Used to calculate computational time
	auto startTime = FPlatformTime::Cycles();

	for (int a = 0; a < m_IterateAmount; ++a)
	{
		// updates heightmapdata variable
		m_HeightmapData = HeightmapData;

		// sorts terrain by height
		auto sortedTerrain = terrainByHeight();

		// loops through sorted terrain
		for (int i = 0; i < sortedTerrain.Num(); ++i)
		{
			// gets index of current cell and gets lowest neighbor
			int adjustedIdx = sortedTerrain[i];
			int lowestNeighbor = getLowestNeighbor(adjustedIdx, heightmapDimension);

			// escapes if lowestneighbor doesn't exist
			if (lowestNeighbor == -1)
				continue;

			// calculates deltaheight
			float heightDif = m_HeightmapData[adjustedIdx] - m_HeightmapData[lowestNeighbor];

			// if the height difference is bigger than the max angle it erodes terrain
			if (heightDif > m_MaxAngle)
			{
				float sedimentToMove = heightDif * 0.1f;
				HeightmapData[adjustedIdx] -= sedimentToMove;
				if (HeightmapData[adjustedIdx] < 0.f)
					HeightmapData[adjustedIdx] = 0.f;
				HeightmapData[lowestNeighbor] += sedimentToMove;
				if (HeightmapData[lowestNeighbor] > 1.f)
					HeightmapData[lowestNeighbor] = 1.f;
			}
		}
	}
	// computational time gets measured and logged
	auto compTime = FPlatformTime::Cycles() - startTime;
	UE_LOG(LogTemp, Warning, TEXT("CompTime simplex noise: %f"), FPlatformTime::ToMilliseconds(compTime));

	return HeightmapData;
}

int UThermalErosion::getLowestNeighbor(int currentIndex, int heightMapDimension)
{
	// sets default idx
	int idx = -1;
	float LowestPoint = m_HeightmapData[currentIndex];
	int idxToCheck = currentIndex + heightMapDimension - 1;

	// initializes list with all neighboring indexes
	TArray<int> indexesToCheck;
	indexesToCheck.Add(idxToCheck + 1);
	idxToCheck -= heightMapDimension;
	indexesToCheck.Add(idxToCheck);
	indexesToCheck.Add(idxToCheck + 2);
	idxToCheck -= heightMapDimension;
	indexesToCheck.Add(idxToCheck + 1);
	for (auto currIdx : indexesToCheck)
	{
		if (currIdx < 0 || currIdx >= m_HeightmapData.Num())
			continue;
		if (m_HeightmapData[currIdx] < LowestPoint)
		{
			idx = currIdx;
			LowestPoint = m_HeightmapData[currIdx];
		}
	}

	return idx;
}

bool UThermalErosion::HeightComparison(const int32& a, const int32& b) const {
	return m_HeightmapData[a] < m_HeightmapData[b];
}

TArray<int> UThermalErosion::terrainByHeight()
{
	TArray<int> heightmapIndex;
	heightmapIndex.Reserve(m_HeightmapData.Num());
	for (size_t i = 0; i < m_HeightmapData.Num(); i++)
		heightmapIndex.Add(i);
	heightmapIndex.Sort([this](const int32& A, const int32& B) {
		return HeightComparison(A, B);
	});
	return heightmapIndex;
}

