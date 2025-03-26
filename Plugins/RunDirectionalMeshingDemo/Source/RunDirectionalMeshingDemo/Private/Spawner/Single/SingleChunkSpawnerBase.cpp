#include "Spawner/Single/SingleChunkSpawnerBase.h"

#include "Mesher/MeshingUtils/VoxelChange.h"
#include "Voxel/Generator/VoxelGeneratorBase.h"

void ASingleChunkSpawnerBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (AlignGridPositionWithSpawner)
	{
		SingleChunkGridPosition = WorldPositionToChunkGridPosition(GetActorLocation());;
	}

	// Initialize single chunk
	SingleChunk = MakeShared<FChunk>();

	AsyncTask(ENamedThreads::BackgroundThreadPriority, [this]()
	{
		AddChunkToGrid(SingleChunk, SingleChunkGridPosition);
		StartMeshing();
	});
}

void ASingleChunkSpawnerBase::ChangeVoxelInChunk(const FIntVector& ChunkGridPosition, const FIntVector& VoxelPosition,
	const FName& VoxelName)
{
	if (ChunkGridPosition != SingleChunkGridPosition)
	{
		// Return if adding to single chunk border
		return;
	}
	
	// Modify voxel at hit position
	FVoxelChange Modification(VoxelName, VoxelPosition);
	StartMeshing(&Modification);
}