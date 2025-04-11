#pragma once
#include "CoreMinimal.h"
#include "Voxel/Voxel.h"

/**
 * Struct representing single voxel face. Inside helper struct not intended to be used with Unreal Engine.
 */
struct FVoxelMergeFace
{
	FVoxel Voxel;
	FIntVector StartVertexDown;
	FIntVector EndVertexDown;
	FIntVector EndVertexUp;
	FIntVector StartVertexUp;

	FVoxelMergeFace() : Voxel(FVoxel()), StartVertexDown(), EndVertexDown(), EndVertexUp(), StartVertexUp()
	{
	}

	FVoxelMergeFace(const FVoxel& Voxel, const FIntVector& StartVertexDown, const FIntVector& EndVertexDown,
	           const FIntVector& EndVertexUp, const FIntVector& StartVertexUp) : Voxel(Voxel),
		StartVertexDown(StartVertexDown),
		EndVertexDown(EndVertexDown), EndVertexUp(EndVertexUp), StartVertexUp(StartVertexUp)
	{
	}

	FVoxelMergeFace(const FIntVector& StartVertexDown, const FIntVector& EndVertexDown,
	           const FIntVector& EndVertexUp, const FIntVector& StartVertexUp) : StartVertexDown(StartVertexDown),
		EndVertexDown(EndVertexDown), EndVertexUp(EndVertexUp), StartVertexUp(StartVertexUp)
	{
	}

	// TODO: change runlenght type
	static FVoxelMergeFace CreateFrontFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght = 1);
	static FVoxelMergeFace CreateBackFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght = 1);
	static FVoxelMergeFace CreateLeftFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght = 1);
	static FVoxelMergeFace CreateRightFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght = 1);
	static FVoxelMergeFace CreateTopFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght = 1);
	static FVoxelMergeFace CreateBottomFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght = 1);

	static bool MergeFaceEnd(FVoxelMergeFace& PrevFace, const FVoxelMergeFace& NewFace);
	static bool MergeFaceStart(FVoxelMergeFace& PrevFace, const FVoxelMergeFace& NewFace);
	static bool MergeFaceUp(FVoxelMergeFace& PrevFace, const FVoxelMergeFace& NewFace);

	FVector3f GetFinalStartVertexDown(const double& VoxelSize) const;
	FVector3f GetFinalStartVertexUp(const double& VoxelSize) const;
	FVector3f GetFinalEndVertexDown(const double& VoxelSize) const;
	FVector3f GetFinalEndVertexUp(const double& VoxelSize) const;
};
