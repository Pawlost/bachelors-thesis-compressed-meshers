#pragma once
#include "CoreMinimal.h"
#include "Mesh/RealtimeMeshDataStream.h"
#include "Mesher/MesherBase.h"
#include "MeshingUtils/VoxelChange.h"
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

	void InitFaceContainers(FMesherVariables& MeshVars) const;
	void GenerateMeshFromFaces(const FMesherVariables& MeshVars) const;
	
	void GenerateActorMesh(const TMap<uint32, uint16>& LocalVoxelTable,
	                       const RealtimeMesh::FRealtimeMeshStreamSet& StreamSet,
	                       const TSharedPtr<FChunkParams>& ChunkParams) const;
	
	FCriticalSection Mutex;
};
