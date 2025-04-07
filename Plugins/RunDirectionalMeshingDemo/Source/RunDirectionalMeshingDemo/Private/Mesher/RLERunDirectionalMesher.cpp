#include "Mesher/RLERunDirectionalMesher.h"
#include "Mesher/RunDirectionalMesher.h"

#include "Mesher/MeshingUtils/MesherVariables.h"
#include "Voxel/RLEVoxel.h"
#include "Voxel/Grid/RLEVoxelGrid.h"

void URLERunDirectionalMesher::CompressVoxelGrid(FChunk& Chunk, TArray<FVoxel>& VoxelGrid)
{
	auto VoxelGridObject = NewObject<URLEVoxelGrid>();

	const auto RLEVoxelGrid = MakeShared<TArray<FRLEVoxel>>();

	RLEVoxelGrid->Reserve(Chunk.ChunkVoxelIdTable.Num());
	RLEVoxelGrid->Emplace(1, VoxelGrid[0]);

	for (int32 x = 1; x < VoxelGrid.Num(); x++)
	{
		const FVoxel Voxel = VoxelGrid[x];
		if (RLEVoxelGrid->Last().Voxel == Voxel)
		{
			RLEVoxelGrid->Last().RunLenght++;
		}
		else
		{
			RLEVoxelGrid->Emplace(1, Voxel);
		}
	}

	VoxelGridObject->RLEVoxelGrid = RLEVoxelGrid;
	Chunk.VoxelModel = VoxelGridObject;
}

void URLERunDirectionalMesher::GenerateMesh(FMesherVariables& MeshVars, FVoxelChange* VoxelChange)
{
	if (EmptyActor(MeshVars))
	{
		return;
	}

#if CPUPROFILERTRACE_ENABLED
	TRACE_CPUPROFILER_EVENT_SCOPE("Mesh generation")
#endif

	const auto VoxelGridPtr = Cast<URLEVoxelGrid>(MeshVars.ChunkParams.OriginalChunk->VoxelModel);

	if (VoxelGridPtr == nullptr)
	{
		return;
	}

	FIndexParams IndexParams;
	IndexParams.VoxelGrid = VoxelGridPtr->RLEVoxelGrid;

	//Measure---
	int voxelCount = 0;
	//---

	if (VoxelChange != nullptr)
	{
		IndexParams.VoxelChange = VoxelChange;
		IndexParams.NewVoxelGrid = MakeShared<TArray<FRLEVoxel>>();
		IndexParams.NewVoxelGrid->Reserve(IndexParams.VoxelGrid->Num() + 1);
		IndexParams.EditVoxel = VoxelGenerator->GetVoxelByName(VoxelChange->VoxelName);


		// Measure---
		UE_LOG(LogTemp, Warning, TEXT("Edit Voxel: %d, Position: X %d,  Y %d, Z %d,"), IndexParams.EditVoxel.VoxelId,
		       VoxelChange->VoxelPosition.X, VoxelChange->VoxelPosition.Y, VoxelChange->VoxelPosition.Z);

		for (auto count : MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable)
		{
			voxelCount += count.Value;
		}
		//----

		if (!IndexParams.EditVoxel.IsEmptyVoxel())
		{
			auto& VoxelTable = MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable;
			if (!VoxelTable.Contains(IndexParams.EditVoxel.VoxelId))
			{
				VoxelTable.Add(IndexParams.EditVoxel.VoxelId, 0);
			}
		}
	}
	else
	{
		IndexParams.NewVoxelGrid = nullptr;
	}

	InitFaceContainers(MeshVars);

	bool bEdited = false;

	IndexParams.CurrentRLERun = IndexParams.VoxelGrid->GetData()[0];
	IndexParams.TraversedRun = IndexParams.CurrentRLERun.RunLenght;

	int YEnd;

	const int ChunkDimension = VoxelGenerator->GetVoxelCountPerChunkDimension();

	// Traverse through voxel grid
	for (int x = 0; x < ChunkDimension; x++)
	{
		for (int z = 0; z < ChunkDimension; z++)
		{
			IndexParams.YStart = 0;
			bool bIsNewRun;

			// Calculate index
			while (IndexParams.YStart < ChunkDimension)
			{
				if (IndexParams.TraversedRun == IndexParams.CurrentRLERun.RunLenght)
				{
					if (IndexParams.EditAreaIndex == 0)
					{
						bIsNewRun = true;
						IndexParams.RunIndex++;
						IndexParams.CurrentRLERun = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex];
					}
					else
					{
						bIsNewRun = false;
						IndexParams.CurrentRLERun = IndexParams.NewVoxelGrid->GetData()[IndexParams.NewVoxelGrid->Num()
							- IndexParams.EditAreaIndex];
						IndexParams.EditAreaIndex--;
					}

					YEnd = IndexParams.CurrentRLERun.RunLenght;
					IndexParams.TraversedRun = 0;
				}
				else
				{
					bIsNewRun = false;
					YEnd = IndexParams.CurrentRLERun.RunLenght - IndexParams.TraversedRun;
				}

				// No action is need when in edit area, otherwise try to apply change and add next run to a new array
				if (VoxelChange != nullptr)
				{
					// Add only full runs
					if (bIsNewRun)
					{
						// Add only outside edit area
						IndexParams.NewVoxelGrid->Push(IndexParams.CurrentRLERun);
					}

					//MID RUN EDIT
					if (!bEdited && x == VoxelChange->VoxelPosition.X && z == VoxelChange->VoxelPosition.Z &&
						IndexParams.YStart <= VoxelChange->VoxelPosition.Y)
					{
						const auto RunEnd = IndexParams.YStart + YEnd;
						if (RunEnd >= VoxelChange->VoxelPosition.Y)
						{
							// Calculated once per meshing
							bEdited = true;

							if (!CalculateMidRunEditIndex(IndexParams, RunEnd))
							{
								return;
							}

							continue;
						}
					}
				}

				//MEASURE--------
				if (IndexParams.NewVoxelGrid != nullptr && IndexParams.NewVoxelGrid->Num() > 1 && IndexParams.
					NewVoxelGrid->Last().Voxel == IndexParams.NewVoxelGrid->GetData()[IndexParams.NewVoxelGrid->Num() -
						2].Voxel)
				{
					UE_LOG(LogTemp, Error, TEXT("HERE"));
				}
				//-----

				// Step to end
				if (IndexParams.YStart + YEnd > ChunkDimension)
				{
					YEnd = ChunkDimension - IndexParams.YStart;
				}

				if (!IndexParams.CurrentRLERun.IsVoxelEmpty())
				{
					// Generate run faces
					auto InitialPosition = FIntVector(x, IndexParams.YStart, z);

					// Front
					CreateFace(MeshVars, FStaticMergeData::FrontFaceData, InitialPosition, IndexParams.CurrentRLERun,
					           YEnd);

					// Back
					CreateFace(MeshVars, FStaticMergeData::BackFaceData, InitialPosition, IndexParams.CurrentRLERun,
					           YEnd);

					// Top
					CreateFace(MeshVars, FStaticMergeData::TopFaceData, InitialPosition, IndexParams.CurrentRLERun,
					           YEnd);

					// Bottom
					CreateFace(MeshVars, FStaticMergeData::BottomFaceData, InitialPosition, IndexParams.CurrentRLERun,
					           YEnd);

					// Right
					CreateFace(MeshVars, FStaticMergeData::RightFaceData, InitialPosition, IndexParams.CurrentRLERun,
					           YEnd);

					// Left
					CreateFace(MeshVars, FStaticMergeData::LeftFaceData, InitialPosition, IndexParams.CurrentRLERun,
					           YEnd);
				}

				IndexParams.TraversedRun += YEnd;
				IndexParams.YStart += YEnd;
			}

			// END RUN EDIT
			if (VoxelChange != nullptr && !bEdited && VoxelChange->VoxelPosition.Y == 0 && (x == VoxelChange->
				VoxelPosition.X && z + 1 ==
				VoxelChange->VoxelPosition.Z || x + 1 == VoxelChange->VoxelPosition.X && VoxelChange->VoxelPosition.Z ==
				0 && z == ChunkDimension - 1))
			{
				// Calculated once per meshing
				bEdited = true;

				if (!CalculateEndRunEditIndex(IndexParams))
				{
					return;
				}
			}
		}
	}

	GenerateMeshFromFaces(MeshVars);

	if (VoxelChange != nullptr)
	{
		int VoxelDimension = VoxelGenerator->GetVoxelCountPerChunk();
		int TestRunIndex = 0;

		//MEASURE ----
		for (int i = IndexParams.NewVoxelGrid->Num() - 1; i > 0; i--)
		{
			const auto RunAtIndex = IndexParams.NewVoxelGrid->GetData()[i];
			const auto RunAtPrevIndex = IndexParams.NewVoxelGrid->GetData()[i - 1];

			TestRunIndex += RunAtIndex.RunLenght;

			if (RunAtIndex.RunLenght <= 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Wrong run: %d"), RunAtIndex.RunLenght);
			}

			if (RunAtIndex.Voxel == RunAtPrevIndex.Voxel)
			{
				auto FirstRun = RunAtIndex;
				auto SecondRun = RunAtPrevIndex;
				UE_LOG(LogTemp, Error, TEXT("Grid Index1: %d, Grid Index2: %d"), i, i-1);
				UE_LOG(LogTemp, Error, TEXT("Voxel: %d"), FirstRun.Voxel.VoxelId);
				UE_LOG(LogTemp, Error, TEXT("First Run: %d"), FirstRun.RunLenght);
				UE_LOG(LogTemp, Error, TEXT("Second Run: %d"), SecondRun.RunLenght);
				UE_LOG(LogTemp, Error, TEXT("Reversed Run Index: %d"), TestRunIndex);
			}
		}
		
		TestRunIndex += IndexParams.NewVoxelGrid->GetData()[0].RunLenght;

		if (VoxelDimension != TestRunIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("Run lenght does not match: %d; Dimension %d"), TestRunIndex, VoxelDimension);
		}
		//-----

		if (!IndexParams.EditVoxel.IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[IndexParams.EditVoxel.VoxelId]++;
		}
		else
		{
			//MEASURE ----
			voxelCount--;
		}

		if (!IndexParams.ReplacedVoxel.IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[IndexParams.ReplacedVoxel.VoxelId]--;

			if (MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[IndexParams.ReplacedVoxel.VoxelId] <= 0)
			{
				MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Remove(IndexParams.ReplacedVoxel.VoxelId);
			}
		}
		else
		{
			//MEASURE ----
			voxelCount++;
		}

		//MEASURE ----
		int Count = 0;
		for (auto count : MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable)
		{
			Count += count.Value;
		}

		if (Count != voxelCount)
		{
			UE_LOG(LogTemp, Error, TEXT("Voxel count is incorrect, original count: %d; Count %d"), voxelCount, Count);
		}
		//------
		
		VoxelGridPtr->RLEVoxelGrid = IndexParams.NewVoxelGrid;
	}
}

void URLERunDirectionalMesher::CreateFace(FMesherVariables& MeshVars, const FStaticMergeData& StaticData,
                                          const FIntVector& InitialPosition, const FRLEVoxel& RLEVoxel, const int YEnd)
{
	const int LocalVoxelId = MeshVars.VoxelIdToLocalVoxelMap[RLEVoxel.Voxel.VoxelId];
	const FChunkFace NewFace = StaticData.FaceCreator(RLEVoxel.Voxel, InitialPosition, YEnd);
	const auto FaceContainerIndex = static_cast<uint8>(StaticData.FaceSide);
	MeshVars.Faces[FaceContainerIndex][LocalVoxelId]->Push(NewFace);
}

bool URLERunDirectionalMesher::CalculateMidRunEditIndex(FIndexParams& IndexParams, const int RunEnd)
{
	if (RunEnd == IndexParams.VoxelChange->VoxelPosition.Y)
	{
		auto& LastRLERun = IndexParams.NewVoxelGrid->Last();

		if (LastRLERun.Voxel == IndexParams.EditVoxel)
		{
			LastRLERun.RunLenght++;
		}
		else
		{
			IndexParams.NewVoxelGrid->Emplace(1, IndexParams.EditVoxel);
			IndexParams.EditAreaIndex = 1;
		}

		auto NextRunIndex = IndexParams.RunIndex + 1;

		if (IndexParams.VoxelGrid->IsValidIndex(NextRunIndex))
		{
			auto& NextRLERun = IndexParams.VoxelGrid->GetData()[NextRunIndex];

			if (NextRLERun.Voxel == IndexParams.EditVoxel)
			{
				return false;
			}

			FirstRunEditIndex(IndexParams);
		}

		IndexParams.CurrentRLERun = LastRLERun;
	}
	else if (IndexParams.CurrentRLERun.Voxel != IndexParams.EditVoxel)
	{
		if (IndexParams.VoxelChange->VoxelPosition.Y == 0)
		{
			//This code is activated only for first voxel in voxel model at position 0, 0, 0
			auto& LastRLERun = IndexParams.NewVoxelGrid->Last();
			auto TempRun = LastRLERun;
			LastRLERun.RunLenght = 1;
			LastRLERun.Voxel = IndexParams.EditVoxel;
			IndexParams.ReplacedVoxel = TempRun.Voxel;
			TempRun.RunLenght --;

			if (!TempRun.IsRunEmpty()){
				IndexParams.NewVoxelGrid->Push(TempRun);
				IndexParams.EditAreaIndex = 1;
			}else
			{
				auto NextRLERun = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];
				if (NextRLERun.Voxel == IndexParams.EditVoxel)
				{
					IndexParams.RunIndex++;
					LastRLERun.RunLenght += NextRLERun.RunLenght;
				}
			}
			
			IndexParams.CurrentRLERun = LastRLERun;
		}else{
			const auto MidRunLenght = IndexParams.TraversedRun + IndexParams.VoxelChange->VoxelPosition.Y - IndexParams.
				YStart;
			const int EndRunLength = IndexParams.NewVoxelGrid->Last().RunLenght - MidRunLenght - 1;

			CalculateSplitRun(MidRunLenght, EndRunLength, IndexParams);
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool URLERunDirectionalMesher::CalculateEndRunEditIndex(FIndexParams& IndexParams)
{
	auto& LastRLERun = IndexParams.NewVoxelGrid->Last();

	if (IndexParams.TraversedRun != LastRLERun.RunLenght)
	{
		if (LastRLERun.Voxel == IndexParams.EditVoxel)
		{
			return false;
		}

		CalculateSplitRun(IndexParams.TraversedRun, LastRLERun.RunLenght - IndexParams.TraversedRun - 1, IndexParams);
	}
	else
	{
		if (LastRLERun.Voxel == IndexParams.EditVoxel)
		{
			LastRLERun.RunLenght++;
			FirstRunEditIndex(IndexParams);
		}
		else if (IndexParams.VoxelGrid->IsValidIndex(IndexParams.RunIndex + 1))
		{
			auto& NextRLERun = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];
			
			if (NextRLERun.Voxel == IndexParams.EditVoxel)
			{
				return false;
			}

			IndexParams.EditAreaIndex = 1;
			IndexParams.NewVoxelGrid->Emplace(1, IndexParams.EditVoxel);

			FirstRunEditIndex(IndexParams);
		}
		
		IndexParams.CurrentRLERun = LastRLERun;
	}
	
	return true;
}

void URLERunDirectionalMesher::CalculateSplitRun(const int MidRunLenght, const int EndRunLength,
                                                 FIndexParams& IndexParams)
{
	auto& LastRLERun = IndexParams.NewVoxelGrid->Last();
	const FRLEVoxel SplitRLERun(EndRunLength, LastRLERun.Voxel);
	LastRLERun.RunLenght = MidRunLenght;
	IndexParams.ReplacedVoxel = LastRLERun.Voxel;
	IndexParams.CurrentRLERun = LastRLERun;

	if (SplitRLERun.IsRunEmpty())
	{
		const int NextIndex = IndexParams.RunIndex + 1;
		if (IndexParams.VoxelGrid->IsValidIndex(NextIndex) && IndexParams.VoxelGrid->GetData()[NextIndex].Voxel ==
			IndexParams.EditVoxel)
		{
			IndexParams.VoxelGrid->GetData()[NextIndex].RunLenght++;
		}
		else
		{
			IndexParams.NewVoxelGrid->Emplace(1, IndexParams.EditVoxel);
			IndexParams.EditAreaIndex = 1;
		}
	}
	else
	{
		IndexParams.NewVoxelGrid->Emplace(1, IndexParams.EditVoxel);
		IndexParams.NewVoxelGrid->Push(SplitRLERun);
		IndexParams.EditAreaIndex = 2;
	}
}

bool URLERunDirectionalMesher::FirstRunEditIndex(FIndexParams& IndexParams)
{
	auto& NextRLERun = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];
	auto& LastRLERun = IndexParams.NewVoxelGrid->Last();

	IndexParams.ReplacedVoxel = NextRLERun.Voxel;
	NextRLERun.RunLenght--;
	
	if (NextRLERun.IsRunEmpty())
	{
		IndexParams.RunIndex++;
		const auto NextNextIndex = IndexParams.RunIndex + 1;
		if (IndexParams.VoxelGrid->IsValidIndex(NextNextIndex) && IndexParams.VoxelGrid->GetData()[
			NextNextIndex].Voxel == LastRLERun.Voxel)
		{
			IndexParams.RunIndex++;
			LastRLERun.RunLenght += IndexParams.VoxelGrid->GetData()[NextNextIndex].RunLenght;
			return false;
		}
	}

	return true;
}
