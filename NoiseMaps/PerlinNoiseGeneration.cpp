// Fill out your copyright notice in the Description page of Project Settings.


#include "PerlinNoiseGeneration.h"

#include "Logging/LogMacros.h"
#include "Engine/Texture2D.h"

#include "Math/UnrealMathUtility.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UPerlinNoiseGeneration::UPerlinNoiseGeneration()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPerlinNoiseGeneration::BeginPlay()
{
	Super::BeginPlay();
	
}

TArray<float> UPerlinNoiseGeneration::GeneratePerlinNoise(int widthHeight, FVector2D offset, float scale, int octaves, float persistance, float lacunarity, UPrimitiveComponent* mesh)
{
	//Used to calculate computational time
	float startTime = FPlatformTime::Cycles();

	TArray<float> noiseMap;
	noiseMap.Reserve(widthHeight * widthHeight);

	for (int i = 0; i < widthHeight; ++i)
	{
		for (int j = 0; j < widthHeight; ++j)
		{
			//Values used for Fractal brownian motion
			float amplitude = 1.f;
			float frequency = 1.f;
			float noiseHeight = 0.f;

			//FBM loop
			for (int k = 0; k < octaves; ++k)
			{
				// coordinates for noise function are calculated
				float X = offset.X + (j / (float)widthHeight) * scale * frequency;
				float Y = offset.Y + (i / (float)widthHeight) * scale * frequency;

				// NoiseHeight is increased
				auto perlinNoise = FMath::PerlinNoise2D(FVector2D(X, Y));
				noiseHeight += perlinNoise * amplitude * 1.2f;

				// amplitude and frequency get adjusted
				amplitude *= persistance;
				frequency *= lacunarity;
			}
			//Moves noiseHeight from -1 1 to 0 1
			noiseHeight = (noiseHeight + 1.f) / 2.f;
			// adds final noiseHeight to noisemap
			noiseMap.Add(FMath::Clamp(noiseHeight, 0.f, 1.f));
		}
		
	}
	// computational time gets measured and logged
	float compTime = FPlatformTime::Cycles() - startTime;
	UE_LOG(LogTemp, Warning, TEXT("CompTime perlin noise: %f"), FPlatformTime::ToMilliseconds(compTime));

	//Heightmap gets visualized on plane
	auto CustomTexture = UTexture2D::CreateTransient(widthHeight, widthHeight);
	auto MipMap = &CustomTexture->PlatformData->Mips[0];
	FByteBulkData* ImageData = &MipMap->BulkData;
	uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);
	int ArraySize = widthHeight * widthHeight * 4;
	for (auto i = 0; i < ArraySize; i += 4)
	{
		RawImageData[i] = 255 * (noiseMap[i / 4]);
		RawImageData[i + 1] = 255 * (noiseMap[i / 4]);
		RawImageData[i + 2] = 255 * (noiseMap[i / 4]);
		RawImageData[i + 3] = 255 * (noiseMap[i / 4]);
	}
	ImageData->Unlock();
	CustomTexture->UpdateResource();

	UMaterialInstanceDynamic* DynamicMaterial = mesh->CreateDynamicMaterialInstance(0, mesh->GetMaterial(0));
	DynamicMaterial->SetTextureParameterValue("Texture", CustomTexture);
	mesh->SetMaterial(0, DynamicMaterial);

	return noiseMap;
}


// Called every frame
void UPerlinNoiseGeneration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

