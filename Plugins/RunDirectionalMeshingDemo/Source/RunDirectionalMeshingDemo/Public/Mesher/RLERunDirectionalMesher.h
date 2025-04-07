#pragma once
#include "CoreMinimal.h"
#include "Mesher/MesherBase.h"
#include "Voxel/RLEVoxel.h"
#include "RLERunDirectionalMesher.generated.h"

struct FChunkParams;

UCLASS(ClassGroup=(Meshers), Blueprintable)
class RUNDIRECTIONALMESHINGDEMO_API URLERunDirectionalMesher : public UMesherBase
{
	GENERATED_BODY()

public:
	virtual void GenerateMesh(FMesherVariables& MeshVars, FVoxelChange* VoxelChange) override;
	virtual void CompressVoxelGrid(FChunk& Chunk, TArray<FVoxel>& VoxelGrid) override;

private:
	struct FVoxelIndexParams
	{
		bool IsBorder;
		int32 ForwardVoxelIndex;
		int32 PreviousVoxelIndex;
		int32 CurrentVoxelIndex;
		FRLEVoxel CurrentVoxel;
		EFaceDirection FaceDirection;
	};

	struct FIndexParams
	{
		TSharedPtr<TArray<FRLEVoxel>> NewVoxelGrid;
		TSharedPtr<TArray<FRLEVoxel>> VoxelGrid;
		
		int TraversedRun = 0;
		int YStart = 0;
		FVoxel EditVoxel;
		FRLEVoxel CurrentRLERun;
		FVoxel ReplacedVoxel = FVoxel();
		int32 EditAreaIndex = 0;
		int32 RunIndex = -1;
		
		FVoxelChange* VoxelChange = nullptr; 
	};
	
	static void CreateFace(FMesherVariables& MeshVars, const FStaticMergeData& StaticData, const FIntVector& InitialPosition, const FRLEVoxel& RLEVoxel, const int YEnd);

	//TODO: Types
	static bool CalculateMidRunEditIndex(FIndexParams& IndexParams, int RunEnd);

	static bool CalculateBorderRunEditIndex(FIndexParams& IndexParams);

	static void FirstRunEditIndex(FIndexParams& IndexParams);

	
	static void CalculateMidRun(const int MidRunLenght, const int EndRunLength, FIndexParams& IndexParams);
};
