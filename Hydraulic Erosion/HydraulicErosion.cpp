// Fill out your copyright notice in the Description page of Project Settings.


#include "HydraulicErosion.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UHydraulicErosion::UHydraulicErosion()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UHydraulicErosion::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UHydraulicErosion::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FHeightGradient UHydraulicErosion::CalcHeightGradient(TArray<float> map, int dimensions, float posX, float posY)
{
	// Get current position in grid
	FHeightGradient heightGradient;
	int coordX = (int)posX;
	int coordY = (int)posY;

	// get offset within grid
	float x = posX - coordX;
	float y = posY - coordY;

	// get north western corner index
	int nodeindexNW = coordX + dimensions * coordY;

	// get corner gray values
	float heightNW = map[nodeindexNW];
	float heightNE = map[nodeindexNW + 1];
	float heightSW = map[nodeindexNW + dimensions];
	float heightSE = map[nodeindexNW + dimensions + 1];

	// calculate gradient vars based on offset within grid
	heightGradient.gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
	heightGradient.gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNW) * x;
	heightGradient.height = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;
	
	return heightGradient;
}

TArray<float> UHydraulicErosion::ErodeTerrain(TArray<float> HeightmapData)
{
	// calculate width/height of map
	int heightmapDimension = FMath::Sqrt(static_cast<float>(HeightmapData.Num()));

	// Initialize the brushes
	InitializeBrushIndices(heightmapDimension, m_Radius);

	// Used to calculate computational time
	auto startTime = FPlatformTime::Cycles();
	for (int a = 0; a < m_IterateAmount; ++a)
	{
		// Create drop and spawn within grid
		FRainDrop drop;
		drop.Location.X = FMath::FRandRange(0.f, heightmapDimension - 2.f);
		drop.Location.Y = FMath::FRandRange(0.f, heightmapDimension - 2.f);
		drop.Direction = FVector2d(0.f, 0.f);

		// loop over its max path
		for (int i = 0; i < m_MaxPath; ++i)
		{
			// get current location in grid
			int currentX = (int)drop.Location.X;
			int currentY = (int)drop.Location.Y;
			int mapIndex = XYToPos(FVector2D(currentX, currentY), heightmapDimension);

			// calculate offset within that cell
			float currentOffsetX = drop.Location.X - currentX;
			float currentOffsetY = drop.Location.Y - currentY;

			// get heightgradient
			auto heightGradient = CalcHeightGradient(HeightmapData, heightmapDimension, drop.Location.X, drop.Location.Y);

			// set direction based on heightgradient, current direction and inertia
			drop.Direction.X = (drop.Direction.X * m_Inertia - heightGradient.gradientX * (1 - m_Inertia));
			drop.Direction.Y = (drop.Direction.Y * m_Inertia - heightGradient.gradientY * (1 - m_Inertia));
			drop.Direction.Normalize();

			// Change location based on direction
			drop.Location.X += drop.Direction.X;
			drop.Location.Y += drop.Direction.Y;

			// escape if drop left map
			if ((drop.Direction.X == 0.f && drop.Direction.Y == 0.f) || drop.Location.X < 0.f || drop.Location.X >= heightmapDimension - 1 || drop.Location.Y < 0.f || drop.Location.Y >= heightmapDimension - 1)
				break;

			// get height in new cell
			float newHeight = CalcHeightGradient(HeightmapData, heightmapDimension, drop.Location.X, drop.Location.Y).height;

			// calculate heightdifference
			float heightDifference = newHeight - heightGradient.height;

			// calculate capacity
			float capacity = FMath::Max(-heightDifference, m_MinSlope) * drop.Velocity * drop.Water * m_Capacity;

			if (drop.Sediment > capacity || heightDifference > 0.f)
			{
				// calculate sediment to drop based on heightdifference
				auto sedimentTodrop = (heightDifference > 0.f) ? FMath::Min(heightDifference, drop.Sediment) : (drop.Sediment - capacity) * m_Deposition;
				drop.Sediment -= sedimentTodrop;

				// spread sediment drop over corners of cell
				HeightmapData[mapIndex] += sedimentTodrop * (1 - currentOffsetX) * (1 - currentOffsetY);
				HeightmapData[mapIndex + 1] += sedimentTodrop * currentOffsetX * (1 - currentOffsetY);
				HeightmapData[mapIndex + heightmapDimension] += sedimentTodrop * (1 - currentOffsetX) * currentOffsetY;
				HeightmapData[mapIndex + heightmapDimension + 1] += sedimentTodrop * currentOffsetX * currentOffsetY;
			}
			else
			{
				// calculate amount to erode
				auto erode = FMath::Min(m_Erosion * (capacity - drop.Sediment), -heightDifference);

				// loop over all brushes
				for (int brushIdx = 0; brushIdx < erosionBrushIndices[mapIndex].size(); ++brushIdx)
				{
					// get neighbor information
					int nodeIdx = erosionBrushIndices[mapIndex][brushIdx];
					float weightErode = erode * erosionBrushWeights[mapIndex][brushIdx];

					// calculate sediment to take from terrain and add sediment to drop
					auto deltaSediment = (HeightmapData[nodeIdx] < weightErode) ? HeightmapData[nodeIdx] : weightErode;
					HeightmapData[nodeIdx] -= deltaSediment;
					drop.Sediment += deltaSediment;
				}
			}

			// decrease water capacity and change velocity
			drop.Water *= (1 - m_Evaporation);
			drop.Velocity = FMath::Sqrt(drop.Velocity * drop.Velocity + FMath::Abs(heightDifference) * m_Gravity);
		}
	}
	// computational time gets measured and logged
	auto compTime = FPlatformTime::Cycles() - startTime;
	UE_LOG(LogTemp, Warning, TEXT("CompTime Hydraulic erosion: %f"), FPlatformTime::ToMilliseconds(compTime));

	return HeightmapData;
}

FVector2D UHydraulicErosion::posToXY(int position, int arraySize)
{
	FVector2D pos;
	pos.X = position % arraySize;
	pos.Y = (position - pos.X) / arraySize;
	return pos;
}

int UHydraulicErosion::XYToPos(FVector2D position, int arraySize)
{
	return position.X + arraySize * position.Y;
}

void UHydraulicErosion::InitializeBrushIndices(int mapSize, int radius)
{
	// clears and resizes lists
	erosionBrushIndices.clear();
	erosionBrushWeights.clear();
	erosionBrushIndices.resize(mapSize * mapSize);
	erosionBrushWeights.resize(mapSize * mapSize);

	// initialize lists used to store neighbor offsets and weight
	std::vector<int> xOffsets(radius * radius * 4);
	std::vector<int> yOffsets(radius * radius * 4);
	std::vector<float> weights(radius * radius * 4);
	float weightSum = 0;
	int addIndex = 0;

	for (int i = 0; i < erosionBrushIndices.size(); i++) {
		// gets current cell index
		int centreX = i % mapSize;
		int centreY = i / mapSize;

		// checks if current cell is within the edge of the map(reduces amount of calculations)
		if (centreY <= radius || centreY >= mapSize - radius || centreX <= radius || centreX >= mapSize - radius) {
			weightSum = 0;
			addIndex = 0;
			//Loops over the square grid the circle fits in
			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					// calculates distance from center and sees if it is within the radius
					float sqrDst = x * x + y * y;
					if (sqrDst < radius * radius) {
						// neighbourindex gets calculated
						int coordX = centreX + x;
						int coordY = centreY + y;

						// checks if coordinates are within the map size
						if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize) {
							// calculates weigh and adds it to the weight list
							float weight = 1 - FMath::Sqrt(sqrDst) / radius;
							weightSum += weight;
							weights[addIndex] = weight;

							// adds offset to offset list
							xOffsets[addIndex] = x;
							yOffsets[addIndex] = y;
							addIndex++;
						}
					}
				}
			}
		}

		// resizes list on current index to the appropriate size
		int numEntries = addIndex;
		erosionBrushIndices[i].resize(numEntries);
		erosionBrushWeights[i].resize(numEntries);

		// loops over the amount of entries and sets variables
		for (int j = 0; j < numEntries; j++) {
			erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
			erosionBrushWeights[i][j] = weights[j] / weightSum;
		}
	}
}

