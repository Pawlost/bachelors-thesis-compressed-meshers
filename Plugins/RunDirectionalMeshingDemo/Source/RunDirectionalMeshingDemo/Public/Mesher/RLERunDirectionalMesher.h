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
	
	static void CreateFace(FMesherVariables& MeshVars, const FStaticMergeData& StaticData, const FIntVector& InitialPosition, const FRLEVoxel* RLEVoxel, const int YEnd);
	
	static bool CalculateEditIndexMidRun(const int& TraversedRun, TArray<FRLEVoxel>& NewVoxelGrid, FRLEVoxel& CurrentRLERun, const FVoxelChange* VoxelChange, int& YStart,
	                                     int RunEnd, const FVoxel& EditVoxel, int& EditAreaIndex, TArray<FRLEVoxel>& VoxelGrid, int& RunIndex, FVoxel& ReplacedVoxel);

	static bool CalculateEditIndexEndRun(const FVoxelChange* VoxelChange, bool& edited, int x, int z, TArray<FRLEVoxel>& NewVoxelGrid, int& size, FVoxel& EditVoxel, FRLEVoxel& RLEVoxel,
	                                     FVoxel& ReplacedVoxel, int& editIndex, TArray<FRLEVoxel>& VoxelGrid, int& globalIndex);
};
