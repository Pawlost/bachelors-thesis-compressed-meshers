#include "Mesher/MeshingUtils/StaticMergeData.h"

FStaticMergeData FStaticMergeData:: FrontFaceData = FStaticMergeData(
	EFaceDirection::Front,
	FChunkFace::MergeFaceEnd,
	FChunkFace::CreateFrontFace
);

FStaticMergeData FStaticMergeData::BackFaceData = FStaticMergeData(
	EFaceDirection::Back,
	FChunkFace::MergeFaceStart,
	FChunkFace::CreateBackFace
);

FStaticMergeData FStaticMergeData::RightFaceData = FStaticMergeData(
	EFaceDirection::Right,
	FChunkFace::MergeFaceStart,
	FChunkFace::CreateRightFace
);

FStaticMergeData FStaticMergeData::LeftFaceData = FStaticMergeData(
	EFaceDirection::Left,
	FChunkFace::MergeFaceEnd,
	FChunkFace::CreateLeftFace
);

FStaticMergeData FStaticMergeData::TopFaceData = FStaticMergeData(
	EFaceDirection::Top,
	FChunkFace::MergeFaceEnd,
	FChunkFace::CreateTopFace
);

FStaticMergeData FStaticMergeData::BottomFaceData = FStaticMergeData(
	EFaceDirection::Bottom,
	FChunkFace::MergeFaceStart,
	FChunkFace::CreateBottomFace
);
