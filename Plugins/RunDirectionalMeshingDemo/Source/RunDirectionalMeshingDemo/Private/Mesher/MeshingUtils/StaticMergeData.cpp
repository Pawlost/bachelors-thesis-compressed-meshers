#include "Mesher/MeshingUtils/StaticMergeData.h"

FStaticMergeData FStaticMergeData:: FrontFaceData = FStaticMergeData(
	EFaceDirection::Front,
	FChunkFace::MergeFaceEnd
);

FStaticMergeData FStaticMergeData::BackFaceData = FStaticMergeData(
	EFaceDirection::Back,
	FChunkFace::MergeFaceStart
);

FStaticMergeData FStaticMergeData::RightFaceData = FStaticMergeData(
	EFaceDirection::Right,
	FChunkFace::MergeFaceStart
);

FStaticMergeData FStaticMergeData::LeftFaceData = FStaticMergeData(
	EFaceDirection::Left,
	FChunkFace::MergeFaceEnd
);

FStaticMergeData FStaticMergeData::TopFaceData = FStaticMergeData(
	EFaceDirection::Top,
	FChunkFace::MergeFaceEnd
);

FStaticMergeData FStaticMergeData::BottomFaceData = FStaticMergeData(
	EFaceDirection::Bottom,
	FChunkFace::MergeFaceStart
);
