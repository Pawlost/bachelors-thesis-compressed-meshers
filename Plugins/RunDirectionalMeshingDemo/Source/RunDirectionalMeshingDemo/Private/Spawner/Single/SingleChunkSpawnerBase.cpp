﻿#include "Spawner/Single/SingleChunkSpawnerBase.h"

#include "Mesher/MeshingUtils/VoxelChange.h"
#include "Voxel/VoxelPosition.h"
#include "Voxel/Generator/VoxelGeneratorBase.h"
#include "Voxel/Grid/VoxelModel.h"

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

void ASingleChunkSpawnerBase::ChangeVoxelInChunk(const FVoxelPosition& VoxelPosition,
	const FName& VoxelName)
{
	if (VoxelPosition.ChunkGridPosition != SingleChunkGridPosition)
	{
		// Return if adding to single chunk border
		return;
	}
	
	// Modify voxel at hit position
	FVoxelChange Modification(VoxelName, VoxelPosition.VoxelPosition);
	StartMeshing(&Modification);
}

FName ASingleChunkSpawnerBase::GetVoxelFromChunk(const FVoxelPosition& VoxelPosition)
{
	const auto VoxelIndex = VoxelGenerator->CalculateVoxelIndex(VoxelPosition.VoxelPosition);
	const auto Voxel = SingleChunk->VoxelModel->GetVoxelAtIndex(VoxelIndex);
	const auto VoxelType = VoxelGenerator->GetVoxelTypeById(Voxel);
	return VoxelType.Key;
}
