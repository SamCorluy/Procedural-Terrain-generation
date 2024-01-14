// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"
#include "BoxCountAlgorithm.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROCEDURALTERRAIN_API UBoxCountAlgorithm : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBoxCountAlgorithm();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
	UProceduralMeshComponent* m_pProceduralMeshComponent;

	// Used for visual representation(doesn't get used in current code)
	TArray<FVector> m_BoxesCollided;
	float m_Size;

	// list that keeps track of collision count
	TArray<int> m_Collisions;

	// helper function that splits cube into 8 cubes(recursive)
	void SplitCube(int depth, float boxSize, FVector position, UBoxComponent* box);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// function called in blueprint that generates data for fractal dimension plotting
	UFUNCTION(BlueprintCallable, Category = "BoxCounting")
	void SetBoxes(float boxSize, int depth);

	// helper function that draws debugboxes
	UFUNCTION(BlueprintCallable, Category = "BoxCounting")
	void DrawBoxes();
};
