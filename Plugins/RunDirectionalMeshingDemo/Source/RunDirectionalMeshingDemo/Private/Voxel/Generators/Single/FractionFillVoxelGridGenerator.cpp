﻿#include "Voxel/Generator/Single/FractionFillVoxelGridGenerator.h"

#include "Mesher/MesherBase.h"

void UFractionFillVoxelGridGenerator::GenerateVoxels(FChunk& Chunk)
{
	const auto VoxelFillIndex = GetSingleVoxel();
	const auto ChunkDimension = GetVoxelCountPerChunkDimension();

	TArray<FVoxel> VoxelGrid;

	for (uint32 x = 0; x < ChunkDimension / XFraction; x++)
	{
		for (uint32 y = 0; y < ChunkDimension / YFraction; y++)
		{
			for (uint32 z = 0; z < ChunkDimension / ZFraction; z++)
			{
				const auto Index = CalculateVoxelIndex(x, y, z);
				ChangeKnownVoxelAtIndex(VoxelGrid, Chunk.ChunkVoxelIdTable, Index, VoxelFillIndex);
			}
		}
	}

	Mesher->CompressVoxelGrid(Chunk, VoxelGrid);
}
