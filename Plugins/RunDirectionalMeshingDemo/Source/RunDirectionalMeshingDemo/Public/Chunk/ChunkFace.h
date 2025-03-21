#pragma once
#include "CoreMinimal.h"
#include "RunDirectionalMeshingDemo/Public/Voxel/Voxel.h"

/**
 * Struct representing single voxel face. Inside helper struct not intended to be used with Unreal Engine.
 */
struct RUNDIRECTIONALMESHINGDEMO_API FChunkFace
{
	FVoxel Voxel;
	FIntVector StartVertexDown;
	FIntVector EndVertexDown;
	FIntVector EndVertexUp;
	FIntVector StartVertexUp;

	FChunkFace() : Voxel(FVoxel()), StartVertexDown(), EndVertexDown(), EndVertexUp(), StartVertexUp()
	{
	}

	FChunkFace(const FVoxel& Voxel, const FIntVector& StartVertexDown, const FIntVector& EndVertexDown,
	           const FIntVector& EndVertexUp, const FIntVector& StartVertexUp) : Voxel(Voxel),
		StartVertexDown(StartVertexDown),
		EndVertexDown(EndVertexDown), EndVertexUp(EndVertexUp), StartVertexUp(StartVertexUp)
	{
	}

	FChunkFace(const FIntVector& StartVertexDown, const FIntVector& EndVertexDown,
	           const FIntVector& EndVertexUp, const FIntVector& StartVertexUp) : StartVertexDown(StartVertexDown),
		EndVertexDown(EndVertexDown), EndVertexUp(EndVertexUp), StartVertexUp(StartVertexUp)
	{
	}

	// TODO: Switch voxel parameter
	static FChunkFace CreateFrontFace(const FIntVector& InitialPosition, const int RunLenght = 1, const FVoxel& Voxel = FVoxel());
	static FChunkFace CreateBackFace(const FIntVector& InitialPosition, const int RunLenght = 1, const FVoxel& Voxel = FVoxel());
	static FChunkFace CreateLeftFace(const FIntVector& InitialPosition, const int RunLenght = 1, const FVoxel& Voxel = FVoxel());
	static FChunkFace CreateRightFace(const FIntVector& InitialPosition, const int RunLenght = 1, const FVoxel& Voxel = FVoxel());
	static FChunkFace CreateTopFace(const FIntVector& InitialPosition, const int RunLenght = 1, const FVoxel& Voxel = FVoxel());
	static FChunkFace CreateBottomFace(const FIntVector& InitialPosition, const int RunLenght = 1, const FVoxel& Voxel = FVoxel());

	static bool MergeFaceEnd(FChunkFace& PrevFace, const FChunkFace& NewFace);
	static bool MergeFaceStart(FChunkFace& PrevFace, const FChunkFace& NewFace);
	static bool MergeFaceUp(FChunkFace& PrevFace, const FChunkFace& NewFace);

	FVector3f GetFinalStartVertexDown(const double& VoxelSize) const;
	FVector3f GetFinalStartVertexUp(const double& VoxelSize) const;
	FVector3f GetFinalEndVertexDown(const double& VoxelSize) const;
	FVector3f GetFinalEndVertexUp(const double& VoxelSize) const;
};
