#include "Log/VoxelMeshingProfilingLogger.h"

DEFINE_LOG_CATEGORY(LogVoxelMeshingProfiling)

void FVoxelMeshingProfilingLogger::LogAllocatedMemory(const FString& LevelName, const SIZE_T AllocatedSize)
{
	UE_LOG(LogVoxelMeshingProfiling, Display, TEXT("Scenario name: %s; Voxel Model memory: %lld"), *LevelName,
	       AllocatedSize);
}

void FVoxelMeshingProfilingLogger::LogGeneratedVertices(const FString& LevelName, const SIZE_T VerticesCount)
{
	UE_LOG(LogVoxelMeshingProfiling, Display, TEXT("Scenario name: %s; Vertices: %lld"), *LevelName,
		   VerticesCount);
}
