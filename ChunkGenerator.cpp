// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkGenerator.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChunkGenerator::AChunkGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

EBlock AChunkGenerator::GetBlockFrom(FVector ChunkPos, FVector BlockIndex)
{
	if (chunks.Contains(ChunkPos))
	{
		return chunks[ChunkPos]->GetBlock(BlockIndex);
	}
	return EBlock::Air;
}

// Called when the game starts or when spawned
void AChunkGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	for (int x = -DrawDistance; x <= DrawDistance; ++x)
	{
		for (int y = -DrawDistance; y <= DrawDistance; ++y)
		{
			CreateChunk(FVector(x * ChunkSize.X * 100, y * ChunkSize.Y * 100, 0));
		}
	}

	for(TPair<FVector, AChunk*> chunk : chunks)
	{
		chunk.Value->RenderChunk();
	}
}

void AChunkGenerator::CreateChunk(FVector ChunkPos)
{
	if (chunks.Contains(ChunkPos))
	{
		chunks[ChunkPos]->RenderChunk();
		return;
	}

	FTransform transform = FTransform(FRotator::ZeroRotator, ChunkPos, FVector::OneVector);
	AChunk* chunk = GetWorld()->SpawnActorDeferred<AChunk>(AChunk::StaticClass(), transform, this);
	chunk->ChunkSize = ChunkSize;
	chunk->Generator = this;
	UGameplayStatics::FinishSpawningActor(chunk, transform);
	chunks.Add(ChunkPos, chunk);
}

// Called every frame


