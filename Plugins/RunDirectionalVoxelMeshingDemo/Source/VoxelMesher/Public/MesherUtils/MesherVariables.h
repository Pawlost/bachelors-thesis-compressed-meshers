#pragma once
#include "FaceDirection.h"
#include "VoxelMergeFace.h"
#include "Spawner/ChunkSpawnerBase.h"
#include "VoxelModel/Chunk.h"
#include "MesherVariables.generated.h"

USTRUCT()
struct VOXELMESHER_API FChunkParams
{
	GENERATED_BODY()

	TSharedPtr<FChunk> SideChunks[CHUNK_FACE_COUNT];
	TSharedPtr<FChunk> OriginalChunk;

	UPROPERTY()
	TWeakObjectPtr<AChunkSpawnerBase> SpawnerPtr = nullptr;

	bool WorldTransform = false;
	bool ShowBorders = false;
	bool ExecutedOnMainThread = false;
};

USTRUCT()
struct VOXELMESHER_API FMesherVariables
{
	GENERATED_BODY()

	TArray<TSharedPtr<TArray<FVoxelMergeFace>>> Faces[CHUNK_FACE_COUNT];
	FChunkParams ChunkParams;
	TMap<uint16, uint16> VoxelIdToLocalVoxelMap; 
};
