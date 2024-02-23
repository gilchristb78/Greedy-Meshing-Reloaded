// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "ChunkGenerator.h"
#include "ProceduralMeshComponent.h"
#include "FastNoiseLite.h"

// Sets default values
AChunk::AChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");
	Noise = new FastNoiseLite();

	Mesh->SetCastShadow(false);
	SetRootComponent(Mesh);

}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
	Super::BeginPlay();

	Noise->SetFrequency(0.005f);
	Noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	Noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	Blocks.SetNum(ChunkSize.X * ChunkSize.Y * ChunkSize.Z); 

	GenerateBlocks();
	
}

void AChunk::GenerateBlocks()
{
	const auto Location = GetActorLocation();

	for (int x = 0; x < ChunkSize.X; ++x)
	{
		for (int y = 0; y < ChunkSize.Y; ++y)
		{
			const float XPos = (x * VoxelSize + Location.X) / VoxelSize;
			const float YPos = (y * VoxelSize + Location.Y) / VoxelSize;

			const int Height = FMath::Clamp(FMath::RoundToInt((Noise->GetNoise(XPos, YPos) + 1) * ChunkSize.Z / 2), 0, ChunkSize.Z); //get noise is -1 -> 1 , + 1 = 0 -> 2 * size = 0->2size / 2

			for (int z = 0; z < Height - 3; ++z)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Stone;
			}

			if (Height < SeaLevel + 1)
			{
				for (int z = Height - 3; z < Height; ++z)
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Sand;
				}
			}
			else
			{
				for (int z = Height - 3; z < Height - 1; ++z)
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Dirt;
				}

				Blocks[GetBlockIndex(x, y, Height - 1)] = EBlock::Grass;
			}

			for (int z = Height; z < SeaLevel; z++)
			{
				Blocks[GetBlockIndex(x, y, z)] = EBlock::Water;
			}
			

			for (int z = Height; z < ChunkSize.Z; ++z)
			{
				if (z > SeaLevel)
				{
					Blocks[GetBlockIndex(x, y, z)] = EBlock::Air;
				}
					
			}

		}

	}
}

int AChunk::GetBlockIndex(int X, int Y, int Z) const
{
	return X * ChunkSize.Y * ChunkSize.Z + Y * ChunkSize.Z + Z;
}

void AChunk::ApplyMesh()
{
	Mesh->SetMaterial(0, Material);
	Mesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, TArray<FVector>(), MeshData.UV0, MeshData.Colors, TArray<FProcMeshTangent>(), true);

	Mesh->SetMaterial(1, MaterialWater);
	Mesh->CreateMeshSection(1, MeshDataWater.Vertices, MeshDataWater.Triangles, TArray<FVector>(), MeshDataWater.UV0, MeshDataWater.Colors, TArray<FProcMeshTangent>(), false);

}

void AChunk::GenerateMesh()
{
	for (int Axis = 0; Axis < 3; Axis++) //take a slice equal to a plane in a perpindicular axis
	{
		const int Axis1 = (Axis + 1) % 3;
		const int Axis2 = (Axis + 2) % 3;

		const int MainAxisLimit = ChunkSize[Axis]; //0 = X, 1 = Y, 2 = Z
		int Axis1Limit = ChunkSize[Axis1];
		int Axis2Limit = ChunkSize[Axis2];

		auto DeltaAxis1 = FIntVector::ZeroValue; //growth of greedy mesh (width x Height)
		auto DeltaAxis2 = FIntVector::ZeroValue;

		auto ChunkItr = FIntVector::ZeroValue; //the blocks that we are itterating over
		auto AxisMask = FIntVector::ZeroValue; //which axis we are itterating over

		AxisMask[Axis] = 1; // [1,0,0] for x axis

		TArray<FMask> Mask;
		//TArray<FMask> MaskWater;
		Mask.SetNum(Axis1Limit * Axis2Limit); //flattened 2d array
		//MaskWater.SetNum(Axis1Limit * Axis2Limit);

		// check each slice

		for (ChunkItr[Axis] = -1; ChunkItr[Axis] < MainAxisLimit; ) //on the main axis go from -1 to the limit, slice before block 0, 1 , 2 and after 2
		{	//this "looks" in the x axis for meshes 
			int N = 0;
			//int NWater = 0;

			for (ChunkItr[Axis2] = 0; ChunkItr[Axis2] < Axis1Limit; ++ChunkItr[Axis2])
			{	//from the 2d meshes in x axis look at ones going in the y
				for (ChunkItr[Axis1] = 0; ChunkItr[Axis1] < Axis2Limit; ++ChunkItr[Axis1])
				{	//and z
					const auto CurrentBlock = GetBlock(FVector(ChunkItr)); //current
					const auto CompareBlock = GetBlock(FVector(ChunkItr + AxisMask)); //get neighbor along current iteration direction //the other side of the mask we are looking at

					const bool CurrentBlockOpaque = GetBlockOpacity(CurrentBlock);
					const bool CompareBlockOpaque = GetBlockOpacity(CompareBlock); 

					if (CurrentBlockOpaque == CompareBlockOpaque)
					{
						if (CurrentBlock == EBlock::Water && !(CompareBlock == EBlock::Water) && !CompareBlockOpaque)
						{
							Mask[N++] = FMask{ EBlock::Water, 1 };
						}
						else if (CompareBlock == EBlock::Water && !(CurrentBlock == EBlock::Water) && !CurrentBlockOpaque)
						{
							Mask[N++] = FMask{ EBlock::Water, -1 };
						}
						else
						{
							Mask[N++] = FMask{ EBlock::Null, 0 };
						}
						//Mask[N++] = FMask{ EBlock::Null, 0 }; //both blocks are air or stone, we dont need the mesh
					}
					else if (CurrentBlockOpaque)
					{
						Mask[N++] = FMask{ CurrentBlock, 1 };
					}
					else
					{
						Mask[N++] = FMask{ CompareBlock, -1 };
					}
				}
			}

			++ChunkItr[Axis];
			N = 0;

			// Generate Mesh from the mask
			for (int j = 0; j < Axis2Limit; j++) //iterate over slice again
			{
				for (int i = 0; i < Axis1Limit; )
				{
					if (Mask[N].Normal != 0) //current mask pos needs a face
					{
						const auto CurrentMask = Mask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;

						int width;

						for (width = 1; i + width < Axis1Limit && CompareMask(Mask[N + width], CurrentMask); ++width) //loop through until current width is equal to the max width
						{
						}

						int height;
						bool done = false;
						for (height = 1; j + height < Axis2Limit; ++height)
						{
							for (int k = 0; k < width; ++k)
							{
								if (CompareMask(Mask[N + k + height * Axis1Limit], CurrentMask)) continue;

								done = true;
								break;
							}
							if (done) break;
						}

						DeltaAxis1[Axis1] = width;
						DeltaAxis2[Axis2] = height;

						CreateQuad(CurrentMask, AxisMask,
							FVector(ChunkItr),//original vertex
							FVector(ChunkItr + DeltaAxis1), //+ width
							FVector(ChunkItr + DeltaAxis2), //+ height
							FVector(ChunkItr + DeltaAxis1 + DeltaAxis2), //both
							width, height);

						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for (int l = 0; l < height; l++)
						{
							for (int k = 0; k < width; ++k)
							{
								Mask[N + k + l * Axis1Limit] = FMask{ EBlock::Null, 0 };
							}
						}

						i += width;
						N += width;
					}
					else
					{
						i++;
						N++;
					}
				}
			}
		}
	}
}

void AChunk::CreateQuad(FMask Mask, FIntVector AxisMask, FVector V1, FVector V2, FVector V3, FVector V4, const int Width,
	const int Height)
{
	FChunkMeshData* data = &MeshData;
	int* vertex = &VertexCount;
	
	if (Mask.Block == EBlock::Water) //lower waterline and set data and vertece variables
	{
		V1.Z -= .2;
		V2.Z -= .2;
		V3.Z -= .2;
		V4.Z -= .2;
		data = &MeshDataWater;
		vertex = &VertexCountWater;
	}

	data->Vertices.Append({ //4 vertices (corners of square)
		(V1 * VoxelSize),
		(V2 * VoxelSize),
		(V3 * VoxelSize),
		(V4 * VoxelSize)
		});

	data->Triangles.Append({ //2 triangles (3 points each) split up square
		(*vertex),
		(*vertex + 2 + Mask.Normal),
		(*vertex + 2 - Mask.Normal),
		(*vertex + 3),
		(*vertex + 1 - Mask.Normal),
		(*vertex + 1 + Mask.Normal)
		});

	FVector Normal = FVector(AxisMask * Mask.Normal);
	data->Normals.Append({ Normal, Normal, Normal, Normal });

	const FColor Color = FColor(0, 0, 0, GetTextureIndex(Mask.Block, Normal));
	data->Colors.Append({ Color,Color,Color,Color });

	//uv data to be used by texures (could be changed if I made triangles in same order every time
	if (Normal.X == 1 || Normal.X == -1) 
	{
		data->UV0.Append({
			FVector2D(Width, Height),
			FVector2D(0, Height),
			FVector2D(Width, 0),
			FVector2D(0, 0),
			});
	}
	else // Z == 1 || Z == -1
	{
		data->UV0.Append({
			FVector2D(Height, Width),
			FVector2D(Height, 0),
			FVector2D(0, Width),
			FVector2D(0, 0),
			});
	} //y axis (top faces) right now rotations dont matter so may be wrong.

	*vertex += 4;
}

bool AChunk::GetBlockOpacity(EBlock block) const
{
	switch (block)
	{
	case EBlock::Null:
		false;
		break;
	case EBlock::Air:
		false;
		break;
	case EBlock::Water:
		false;
		break;
	default:
		return true;
		break;
	}
	return false; //should never get here
}

EBlock AChunk::GetBlock(FVector Index) const
{
	if (Index.Z >= ChunkSize.Z || Index.Z < 0) //above or below chunk
		return EBlock::Air;
	if (Index.X >= ChunkSize.X || Index.Y >= ChunkSize.Y || Index.X < 0 || Index.Y < 0)
	{
		if (Generator)
		{
			FVector ChunkPos = GetActorLocation();
			FVector NewIndex = Index;

			if (NewIndex.X == -1)
			{
				ChunkPos.X -= (VoxelSize * ChunkSize.X);
				NewIndex.X = ChunkSize.X - 1;
			}
			if (NewIndex.Y == -1)
			{
				ChunkPos.Y -= (VoxelSize * ChunkSize.Y);
				NewIndex.Y = ChunkSize.Y - 1;
			}

			if (NewIndex.X == ChunkSize.X)
			{
				ChunkPos.X += (VoxelSize * ChunkSize.X);
				NewIndex.X = 0;
			}
			if (NewIndex.Y == ChunkSize.Y)
			{
				ChunkPos.Y += (VoxelSize * ChunkSize.Y);
				NewIndex.Y = 0;
			}
			

			return Generator->GetBlockFrom(ChunkPos, NewIndex);
		}
		return EBlock::Null;
	}
	return Blocks[GetBlockIndex(Index.X, Index.Y, Index.Z)];
}

bool AChunk::CompareMask(FMask M1, FMask M2) const
{
	return M1.Block == M2.Block && M1.Normal == M2.Normal;
}

int AChunk::GetTextureIndex(EBlock Block, const FVector Normal) const
{
	switch (Block)
	{
	case EBlock::Stone:
		return 3;
	case EBlock::Dirt:
		return 2;
	case EBlock::Grass:
		if (Normal == FVector::UpVector) return 0;
		if (Normal == FVector::DownVector) return 2;
		return 1;
	case EBlock::Sand:
		return 4;
	default:
		return 255;
	}
	return 255;
}

void AChunk::RenderChunk()
{
	GenerateMesh();

	ApplyMesh();
}





