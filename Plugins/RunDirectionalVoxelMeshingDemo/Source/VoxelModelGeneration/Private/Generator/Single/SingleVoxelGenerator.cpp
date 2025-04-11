#include "Generator/Single/SingleVoxelGenerator.h"
#include "VoxelMesher/Public/VoxelMesherBase.h"

void USingleVoxelGenerator::GenerateVoxels(FChunk& Chunk)
{
	const auto VoxelFillIndex = GetSingleVoxel();
	const auto VoxelGridDensity = GetVoxelCountPerChunk();

	TArray<FVoxel> VoxelGridDensityArray;
	VoxelGridDensityArray.Init(VoxelFillIndex, VoxelGridDensity);
	Chunk.ChunkVoxelIdTable.Add(VoxelFillIndex.VoxelId, VoxelGridDensity);

	Mesher->CompressVoxelGrid(Chunk, VoxelGridDensityArray);
}
