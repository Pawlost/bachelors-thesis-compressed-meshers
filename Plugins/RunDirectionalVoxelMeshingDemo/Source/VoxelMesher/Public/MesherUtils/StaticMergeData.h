#pragma once
#include "FaceDirection.h"
#include "VoxelMergeFace.h"
#include "Voxel/Voxel.h"

struct FStaticMergeData
{
	const EFaceDirection FaceSide;
	const TFunctionRef<bool(FVoxelMergeFace& PrevFace, const FVoxelMergeFace& NewFace)> RunDirectionFaceMerge;
	const TFunctionRef<FVoxelMergeFace(const FVoxel& Voxel, const FIntVector& InitialPosition, const int RunLenght)> FaceCreator;
	
	static FStaticMergeData FrontFaceData;
	static FStaticMergeData BackFaceData;
	static FStaticMergeData LeftFaceData;
	static FStaticMergeData RightFaceData;
	static FStaticMergeData TopFaceData;
	static FStaticMergeData BottomFaceData;
};
