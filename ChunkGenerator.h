// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.h"
#include "ChunkGenerator.generated.h"

UCLASS()
class VOXEL_API AChunkGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChunkGenerator();

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	int DrawDistance = 5;

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	FIntVector ChunkSize = FIntVector(32,32,32);

	TMap<FVector, AChunk*> chunks;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RenderChunk(FVector ChunkPos);
};
