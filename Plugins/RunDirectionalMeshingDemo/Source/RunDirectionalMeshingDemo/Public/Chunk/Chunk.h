#pragma once
#include "CoreMinimal.h"
#include "ChunkRMCActor.h"
#include "RunDirectionalMeshingDemo/Public/Voxel/Voxel.h"
#include "Voxel/Grid/VoxelData.h"
#include "Chunk.generated.h"

USTRUCT()
struct RUNDIRECTIONALMESHINGDEMO_API FChunk
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AChunkRMCActor> ChunkMeshActor = nullptr;

	// Saving voxel grids is not implemented in this demo, but a property specifier for saving has been added.

	UPROPERTY(SaveGame)
	TObjectPtr<UVoxelData> VoxelGrid;

	//Key: voxel ID; Value: voxel count
	UPROPERTY()
	TMap<int32, uint32> ChunkVoxelIdTable;

	UPROPERTY()
	FIntVector GridPosition = FIntVector(0, 0, 0);

	UPROPERTY(VisibleInstanceOnly)
	bool bHasMesh = false;

	UPROPERTY(VisibleInstanceOnly)
	bool bIsActive = false;
};
