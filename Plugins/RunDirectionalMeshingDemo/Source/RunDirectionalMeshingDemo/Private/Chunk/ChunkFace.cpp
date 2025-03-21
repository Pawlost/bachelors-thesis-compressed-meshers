#include "Chunk/ChunkFace.h"

FChunkFace FChunkFace::CreateFrontFace(const FIntVector& InitialPosition, const int RunLenght, const FVoxel& Voxel)
{
	return FChunkFace(Voxel,
		InitialPosition,
		 InitialPosition + FIntVector(0, RunLenght, 0),
		InitialPosition + FIntVector(0, RunLenght, 1),
		InitialPosition + FIntVector(0, 0, 1));
}

FChunkFace FChunkFace::CreateBackFace(const FIntVector& InitialPosition, const int RunLenght, const FVoxel& Voxel)
{
	return FChunkFace(Voxel,
		InitialPosition + FIntVector(1, RunLenght, 0),
		 InitialPosition + FIntVector(1, 0, 0),
		InitialPosition + FIntVector(1, 0, 1),
		InitialPosition + FIntVector(1, RunLenght, 1));
}

FChunkFace FChunkFace::CreateLeftFace(const FIntVector& InitialPosition, const int RunLenght, const FVoxel& Voxel)
{
	return FChunkFace(Voxel,
		InitialPosition + FIntVector(0, RunLenght, 0),
		 InitialPosition + FIntVector(1, RunLenght, 0),
		InitialPosition + FIntVector(1, RunLenght, 1),
		InitialPosition + FIntVector(0, RunLenght, 1));
}

FChunkFace FChunkFace::CreateRightFace(const FIntVector& InitialPosition, const int RunLenght, const FVoxel& Voxel)
{
	return FChunkFace(Voxel,
		InitialPosition + FIntVector(1, 0, 0),
		 InitialPosition,
		InitialPosition + FIntVector(0, 0, 1),
		InitialPosition + FIntVector(1, 0, 1));
}

FChunkFace FChunkFace::CreateTopFace(const FIntVector& InitialPosition, const int RunLenght, const FVoxel& Voxel)
{
	return FChunkFace(Voxel,
		InitialPosition + FIntVector(0, 0, 1),
		 InitialPosition + FIntVector(0, RunLenght, 1),
		InitialPosition + FIntVector(1, RunLenght, 1),
		InitialPosition + FIntVector(1, 0, 1));
}

FChunkFace FChunkFace::CreateBottomFace(const FIntVector& InitialPosition, const int RunLenght, const FVoxel& Voxel)
{
	return FChunkFace(Voxel,
		InitialPosition + FIntVector(0, RunLenght, 0),
		 InitialPosition,
		InitialPosition + FIntVector(1, 0, 0),
		InitialPosition +FIntVector(1, RunLenght, 0));
}

/**
 * Compare vertices and merge quads
 * @return true if previous face was merged
 */
bool FChunkFace::MergeFaceEnd(FChunkFace& PrevFace, const FChunkFace& NewFace)
{
	if (PrevFace.EndVertexDown == NewFace.StartVertexDown &&
		PrevFace.EndVertexUp == NewFace.StartVertexUp)
	{
		PrevFace.EndVertexDown = NewFace.EndVertexDown;
		PrevFace.EndVertexUp = NewFace.EndVertexUp;
		return true;
	}
	return false;
}

/**
 * Compare vertices and merge quads
 * @return true if previous face was merged
 */
bool FChunkFace::MergeFaceStart(FChunkFace& PrevFace, const FChunkFace& NewFace)
{
	if (PrevFace.StartVertexUp == NewFace.EndVertexUp &&
		PrevFace.StartVertexDown == NewFace.EndVertexDown)
	{
		PrevFace.StartVertexDown = NewFace.StartVertexDown;
		PrevFace.StartVertexUp = NewFace.StartVertexUp;
		return true;
	}
	return false;
}

/**
 * Compare vertices and merge quads
 * @return true if previous face was merged
 */
bool FChunkFace::MergeFaceUp(FChunkFace& PrevFace, const FChunkFace& NewFace)
{
	if (PrevFace.StartVertexUp == NewFace.StartVertexDown &&
		PrevFace.EndVertexUp == NewFace.EndVertexDown)
	{
		PrevFace.StartVertexUp = NewFace.StartVertexUp;
		PrevFace.EndVertexUp = NewFace.EndVertexUp;
		return true;
	}
	return false;
}

FVector3f FChunkFace::GetFinalStartVertexDown(const double& VoxelSize) const
{
	return static_cast<FVector3f>(StartVertexDown) * VoxelSize;
}

FVector3f FChunkFace::GetFinalStartVertexUp(const double& VoxelSize) const
{
	return static_cast<FVector3f>(StartVertexUp) * VoxelSize;
}

FVector3f FChunkFace::GetFinalEndVertexDown(const double& VoxelSize) const
{
	return static_cast<FVector3f>(EndVertexDown) * VoxelSize;
}

FVector3f FChunkFace::GetFinalEndVertexUp(const double& voxelSize) const
{
	return static_cast<FVector3f>(EndVertexUp) * voxelSize;
}
