#pragma once
#include "FaceDirection.h"
#include "Chunk/ChunkFace.h"

struct FStaticMergeData
{
	const EFaceDirection FaceSide;
	const TFunctionRef<bool(FChunkFace& PrevFace, const FChunkFace& NewFace)> RunDirectionFaceMerge;
	const TFunctionRef<FChunkFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght)> FaceCreator;
	
	static FStaticMergeData FrontFaceData;
	static FStaticMergeData BackFaceData;
	static FStaticMergeData LeftFaceData;
	static FStaticMergeData RightFaceData;
	static FStaticMergeData TopFaceData;
	static FStaticMergeData BottomFaceData;
};
