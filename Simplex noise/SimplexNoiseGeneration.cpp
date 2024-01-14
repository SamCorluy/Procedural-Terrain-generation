// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplexNoiseGeneration.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
USimplexNoiseGeneration::USimplexNoiseGeneration()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    // sets simplex constants
    m_F2 = 0.5f * (FMath::Sqrt(3.f) - 1.f);
    m_G2 = (3.f - FMath::Sqrt(3.f)) / 6.f;
}


// Called when the game starts
void USimplexNoiseGeneration::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void USimplexNoiseGeneration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

TArray<float> USimplexNoiseGeneration::GenerateSimplexNoise(int widthHeight, FVector2D offset, float scale, int octaves, float persistance, float lacunarity, UPrimitiveComponent* mesh)
{
    //Used to calculate computational time
    auto startTime = FPlatformTime::Cycles();


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
                auto simplexoise = SimplexNoise2D(FVector2D(X, Y));
                noiseHeight += simplexoise * amplitude;

                // amplitude and frequency get adjusted
                amplitude *= persistance;
                frequency *= lacunarity;
            }
            //Moves noiseHeight from -1 1 to 0 1
            noiseHeight = (noiseHeight + 1.f) / 2.f;
            noiseMap.Add(FMath::Clamp(noiseHeight, 0.f, 1.f));
        }
    }
    // computational time gets measured and logged
    auto compTime = FPlatformTime::Cycles() - startTime;
    UE_LOG(LogTemp, Warning, TEXT("CompTime simplex noise: %f"), FPlatformTime::ToMilliseconds(compTime));

    //Heightmap gets visualized on plane
    auto CustomTexture = UTexture2D::CreateTransient(widthHeight, widthHeight);
    auto MipMap = &CustomTexture->PlatformData->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);
    int ArraySize = widthHeight * widthHeight * 4;
    for (auto i = 0; i < ArraySize; i += 4)
    {
        RawImageData[i] = 255 * (noiseMap[i / 4]);
        RawImageData[i + 2] = 255 * (noiseMap[i / 4]);
        RawImageData[i + 3] = 255 * (noiseMap[i / 4]);
        RawImageData[i + 1] = 255 * (noiseMap[i / 4]);
    }
    ImageData->Unlock();
    CustomTexture->UpdateResource();

    UMaterialInstanceDynamic* DynamicMaterial = mesh->CreateDynamicMaterialInstance(0, mesh->GetMaterial(0));
    DynamicMaterial->SetTextureParameterValue("Texture", CustomTexture);
    mesh->SetMaterial(0, DynamicMaterial);

    return noiseMap;
}

//Predefined permutation list that is commonly used
static const uint8_t permutation[256] = {
    151, 160, 137, 91, 90, 15,
    131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
    190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
    77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
    135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
    5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
    223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
    251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static float grad(int32_t hash, float x, float y) {
    const int32_t h = hash & 0xF;
    const float u = h < 8 ? x : y;
    const float v = h < 8 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

float USimplexNoiseGeneration::SimplexNoise2D(const FVector2D& location)
{
    float n0, n1, n2;

    auto skewFactor = (location.X + location.Y) * m_F2;
    int i = floor(location.X + skewFactor);
    int j = floor(location.Y + skewFactor);

    auto unskewFactor = (i + j) * m_G2;
    auto x0 = location.X - (i - unskewFactor);
    auto y0 = location.Y - (j - unskewFactor);

    int i1, j1;
    if (x0 > y0)
    {
        i1 = 1;
        j1 = 0;
    }
    else
    {
        i1 = 0;
        j1 = 1;
    }

    auto x1 = x0 - i1 + m_G2;
    auto y1 = y0 - j1 + m_G2;
    auto x2 = x0 - 1.f + 2.f * m_G2;
    auto y2 = y0 - 1.f + 2.f * m_G2;

    auto ii = i & 255;
    auto jj = j & 255;

    auto gi0 = permutation[ii + permutation[jj]];
    auto gi1 = permutation[ii + i1 + permutation[jj + j1]];
    auto gi2 = permutation[ii + 1 + permutation[jj + 1]];

    auto t0 = 0.5f - x0 * x0 - y0 * y0;
    if (t0 < 0.f)
        n0 = 0.f;
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0);
    }

    auto t1 = 0.5f - x1 * x1 - y1 * y1;
    if (t1 < 0.f)
        n1 = 0.f;
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1);
    }

    float t2 = 0.5f - x2 * x2 - y2 * y2;
    if (t2 < 0.f)
        n2 = 0.f;
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2);
    }

    return 24.f * (n0 + n1 + n2);
}

