// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkGenerator.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChunkGenerator::AChunkGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AChunkGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	for (int x = -DrawDistance; x <= DrawDistance; ++x)
	{
		for (int y = -DrawDistance; y <= DrawDistance; ++y)
		{
			RenderChunk(FVector(x * ChunkSize.X * 100, y * ChunkSize.Y * 100, 0));
		}
	}
}

void AChunkGenerator::RenderChunk(FVector ChunkPos)
{
	if (chunks.Contains(ChunkPos))
	{
		chunks[ChunkPos]->RenderChunk();
		return;
	}

	FTransform transform = FTransform(FRotator::ZeroRotator, ChunkPos, FVector::OneVector);
	AChunk* chunk = GetWorld()->SpawnActorDeferred<AChunk>(AChunk::StaticClass(), transform, this);
	chunk->ChunkSize = ChunkSize;
	UGameplayStatics::FinishSpawningActor(chunk, transform);
	chunks.Add(ChunkPos, chunk);
}

// Called every frame


