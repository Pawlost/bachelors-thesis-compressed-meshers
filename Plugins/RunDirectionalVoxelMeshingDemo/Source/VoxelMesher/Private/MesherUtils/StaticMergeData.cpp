#include "MesherUtils/StaticMergeData.h"

#include "MesherUtils/VoxelMergeFace.h"

FStaticMergeData FStaticMergeData:: FrontFaceData = FStaticMergeData(
	EFaceDirection::Front,
	FVoxelMergeFace::MergeFaceEnd,
	FVoxelMergeFace::CreateFrontFace
);

FStaticMergeData FStaticMergeData::BackFaceData = FStaticMergeData(
	EFaceDirection::Back,
	FVoxelMergeFace::MergeFaceStart,
	FVoxelMergeFace::CreateBackFace
);

FStaticMergeData FStaticMergeData::RightFaceData = FStaticMergeData(
	EFaceDirection::Right,
	FVoxelMergeFace::MergeFaceStart,
	FVoxelMergeFace::CreateRightFace
);

FStaticMergeData FStaticMergeData::LeftFaceData = FStaticMergeData(
	EFaceDirection::Left,
	FVoxelMergeFace::MergeFaceEnd,
	FVoxelMergeFace::CreateLeftFace
);

FStaticMergeData FStaticMergeData::TopFaceData = FStaticMergeData(
	EFaceDirection::Top,
	FVoxelMergeFace::MergeFaceEnd,
	FVoxelMergeFace::CreateTopFace
);

FStaticMergeData FStaticMergeData::BottomFaceData = FStaticMergeData(
	EFaceDirection::Bottom,
	FVoxelMergeFace::MergeFaceStart,
	FVoxelMergeFace::CreateBottomFace
);
