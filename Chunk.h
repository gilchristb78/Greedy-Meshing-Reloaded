// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.generated.h"

class FastNoiseLite;
class UProceduralMeshComponent;

UENUM(BlueprintType)
enum class EBlock
{
	Null UMETA(DisplayName = "Null"),
	Air UMETA(DisplayName = "Air"),
	Stone UMETA(DisplayName = "Stone"),
	Sand UMETA(DisplayName = "Sand"),
	Dirt UMETA(DisplayName = "Dirt"),
	Grass UMETA(DisplayName = "Grass"),
	Water UMETA(DisplayName = "Water"),
	SlopeGrass UMETA(DisplayName = "Slope Grass")
};

USTRUCT()
struct FChunkMeshData
{
	GENERATED_BODY();


public:
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> Colors;
};

struct FMask
{
	EBlock Block;
	int Normal;
};

UCLASS()
class VOXEL_API AChunk : public AActor
{
	GENERATED_BODY()
	


public:	
	// Sets default values for this actor's properties
	AChunk();

	FIntVector ChunkSize = FIntVector(2, 2, 2) * 32;

	int VoxelSize = 95;

	int SeaLevel = ChunkSize.Z / 2;

	void RenderChunk();

	EBlock GetBlock(FVector Index) const;

	class AChunkGenerator* Generator;

	TObjectPtr<UMaterialInterface> Material;
	TObjectPtr<UMaterialInterface> MaterialWater;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GenerateBlocks();

private:

	FChunkMeshData MeshData;
	FChunkMeshData MeshDataWater;
	TObjectPtr<UProceduralMeshComponent> Mesh;

	int VertexCount = 0;
	int VertexCountWater = 0;

	TObjectPtr<FastNoiseLite> Noise;
	TArray<EBlock> Blocks;

	int GetBlockIndex(int X, int Y, int Z) const;

	void ApplyMesh();

	void GenerateMesh();

	void CreateQuad(FMask Mask, FIntVector AxisMask, FIntVector V1, FIntVector V2, FIntVector V3, FIntVector V4, const int Width,
		const int Height);


	bool GetBlockOpacity(EBlock block) const;

	bool CompareMask(FMask M1, FMask M2) const;

	int GetTextureIndex(EBlock Block, const FVector Normal) const;

};
