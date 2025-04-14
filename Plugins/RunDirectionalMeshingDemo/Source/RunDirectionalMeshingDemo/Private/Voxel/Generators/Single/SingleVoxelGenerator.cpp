#include "Voxel/Generator/Single/SingleVoxelGenerator.h"

#include "Chunk/Chunk.h"
#include "Mesher/MesherBase.h"

void USingleVoxelGenerator::GenerateVoxels(FChunk& Chunk)
{
	const auto VoxelFillIndex = GetSingleVoxel();
	const auto VoxelGridDensity = GetVoxelCountPerChunk();

	TArray<FVoxel> VoxelGridDensityArray;
	VoxelGridDensityArray.Init(VoxelFillIndex, VoxelGridDensity);
	Chunk.ChunkVoxelIdTable.Add(VoxelFillIndex.VoxelId, VoxelGridDensity);

	VoxelMesher->CompressVoxelGrid(Chunk, VoxelGridDensityArray);
}
