// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxCountAlgorithm.h"
#include "Logging/LogMacros.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UBoxCountAlgorithm::UBoxCountAlgorithm()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBoxCountAlgorithm::BeginPlay()
{
	Super::BeginPlay();
	
}


void UBoxCountAlgorithm::SplitCube(int depth, float boxSize, FVector position, UBoxComponent* box)
{
	// decreases depth
	depth--;
	// if depth smaller or equal to 0, escape
	if (depth <= 0)
		return;

	// decrease boxsize
	boxSize *= 0.5f;

	// loop that splits box into 4 boxes
	for (size_t x = 0; x < 2; x++)
	{
		for (size_t y = 0; y < 2; y++)
		{
			for (size_t z = 0; z < 2; z++)
			{
				// get current box position
				auto currentBoxPosition = FVector(position.X + boxSize * x, position.Y + boxSize * y, position.Z + boxSize * z);

				// change box extent and world location
				box->SetBoxExtent(FVector(boxSize * 0.5f, boxSize * 0.5f, boxSize * 0.5f));
				box->SetWorldLocation(FVector(currentBoxPosition.X + boxSize * 0.5f, currentBoxPosition.Y + boxSize * 0.5f, currentBoxPosition.Z + boxSize * 0.5f));

				// update overlaps and check if box overlaps with terrain
				box->UpdateOverlaps();
				auto isoverlapping = box->IsOverlappingComponent(m_pProceduralMeshComponent);
				if (isoverlapping)
				{
					// if overlaps add to the collision count within this depth, split cube again
					m_Collisions[depth - 1] += 1;
					SplitCube(depth, boxSize, currentBoxPosition, box);
				}
			}
		}
	}
}

// Called every frame
void UBoxCountAlgorithm::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBoxCountAlgorithm::SetBoxes(float boxSize, int depth)
{
	if(!m_pProceduralMeshComponent)
		m_pProceduralMeshComponent = GetOwner()->FindComponentByClass<UProceduralMeshComponent>();

	// create collision box
	auto box = NewObject<UBoxComponent>(GetOwner());
	box->SetupAttachment(GetOwner()->GetRootComponent());
	box->RegisterComponent();

	// reset collision list
	m_Collisions.Empty();
	m_Collisions.SetNum(depth);
	for (auto collision : m_Collisions)
		collision = 0;

	// calculate bounds of mesh and starting position
	auto meshBox = m_pProceduralMeshComponent->CalcBounds(m_pProceduralMeshComponent->GetComponentTransform());
	auto meshBounds = meshBox.GetBox().Max - meshBox.GetBox().Min;
	auto position = meshBox.GetBox().Min;

	int dimensionsX = FMath::CeilToInt(meshBounds.X / boxSize);
	int dimensionsY = FMath::CeilToInt(meshBounds.Y / boxSize);
	int dimensionsZ = FMath::CeilToInt(meshBounds.Z / boxSize);


	// used to reset to initial position after loop
	auto defaultPosition = position;

	defaultPosition.X -= (dimensionsX * boxSize - meshBounds.X) / 2.f;
	defaultPosition.Y -= (dimensionsY * boxSize - meshBounds.Y) / 2.f;
	defaultPosition.Z -= (dimensionsZ * boxSize - meshBounds.Z) / 2.f;
	auto currentPosition = defaultPosition;

	// calculate total boxes on first depth
	int totalBoxes = dimensionsX * dimensionsY * dimensionsZ;
	for (size_t x = 0; x < dimensionsX; ++x)
	{
		for (size_t y = 0; y < dimensionsY; ++y)
		{
			for (size_t z = 0; z < dimensionsZ; ++z)
			{
				// change box extent and set world location
				box->SetBoxExtent(FVector(boxSize * 0.5f, boxSize * 0.5f, boxSize * 0.5f));
				box->SetWorldLocation(FVector(currentPosition.X + boxSize * 0.5f, currentPosition.Y + boxSize * 0.5f, currentPosition.Z + boxSize * 0.5f));

				//update and check overlaps
				box->UpdateOverlaps();
				auto isoverlapping = box->IsOverlappingComponent(m_pProceduralMeshComponent);
				if (isoverlapping)
				{
					// add to collision and split cube
					m_Collisions[depth - 1] += 1;
					SplitCube(depth, boxSize, currentPosition, box);
				}

				currentPosition.Z += boxSize;
			}
			currentPosition.Z = defaultPosition.Z;
			currentPosition.Y += boxSize;
		}
		currentPosition.Y = defaultPosition.Y;
		currentPosition.X += boxSize;
	}

	// itterate over all collisions and log data
	for (int i = 1; i <= m_Collisions.Num(); ++i)
	{
		float ratio = (float)m_Collisions[depth - i] / (float)totalBoxes;
		UE_LOG(LogTemp, Warning, TEXT("Log(Size): %f, Log(Ratio): %f"), FMath::Loge(1.f / boxSize), FMath::Loge(ratio));
		totalBoxes *= 8;
		boxSize *= 0.5;
	}
	// destroy collision box
	box->DestroyComponent();
}

void UBoxCountAlgorithm::DrawBoxes()
{
	for (auto collisionbox : m_BoxesCollided)
	{
		auto box = GetOwner()->GetActorLocation() + collisionbox;
		FColor color = FColor::Red;
		color = FColor::Green;
		float halfSize = m_Size / 2.f;
		DrawDebugBox(GetWorld(), FVector(box.X, box.Y, box.Z), FVector(halfSize, halfSize, halfSize), FQuat::Identity, color, false, 100, 0, 0.3f);
	}
}

