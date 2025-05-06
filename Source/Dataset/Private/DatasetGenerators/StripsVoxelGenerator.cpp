#include "DatasetGenerators/StripsVoxelGenerator.h"

#include "Mesher/MesherBase.h"

bool UStripsVoxelGenerator::IsInGap(const int Coordinate, const int StripDimension)
{
	return (Coordinate / StripDimension) % 2 != 0 ;
}

void UStripsVoxelGenerator::GenerateVoxels(FChunk& Chunk)
{
	const auto VoxelFillIndex = GetSingleVoxel();
	const auto ChunkDimension = GetVoxelCountPerChunkDimension();

	TArray<FVoxel> VoxelGrid;
	VoxelGrid.SetNum(GetVoxelCountPerChunk());

	const int StripDimensionX = ChunkDimension/(XGap + XGap + 1);
	const int StripDimensionY = ChunkDimension/(YGap + YGap + 1);
	const int StripDimensionZ = ChunkDimension/(ZGap + ZGap + 1);

	for (uint32 x = 0; x < ChunkDimension; x++)
	{
		for (uint32 y = 0; y < ChunkDimension; y++)
		{
			for (uint32 z = 0; z < ChunkDimension; z++)
			{
				FIntVector VoxelPosition(x, y, z);
				if (IsInGap(x, StripDimensionX) || IsInGap(y, StripDimensionY) || IsInGap(z, StripDimensionZ))
				{
					continue;
				}
				
				const auto Index = CalculateVoxelIndex(VoxelPosition);
				ChangeKnownVoxelAtIndex(VoxelGrid, Chunk.ChunkVoxelIdTable, Index, VoxelFillIndex);
			}
		}
	}

	VoxelMesher->CompressVoxelGrid(Chunk, VoxelGrid);
}
