#include "DatasetGenerators/SpecificVoxelGenerator.h"

#include "Mesher/MesherBase.h"

void USpecificVoxelGenerator::GenerateVoxels(FChunk& Chunk)
{
	const auto VoxelFillIndex = GetSingleVoxel();
	
	TArray<FVoxel> VoxelGrid;
	VoxelGrid.SetNum(GetVoxelCountPerChunk());

	for (auto Position : SpecificPositions)
	{
		const auto Index = CalculateVoxelIndex(Position);
		ChangeKnownVoxelAtIndex(VoxelGrid, Chunk.ChunkVoxelIdTable, Index, VoxelFillIndex);
	}
	
	VoxelMesher->CompressVoxelGrid(Chunk, VoxelGrid);
}
