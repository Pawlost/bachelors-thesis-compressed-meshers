﻿#include "Mesher/RLERunDirectionalMesher.h"
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
	}else
	{
		IndexParams.NewVoxelGrid = nullptr;
	}

	InitFaceContainers(MeshVars);

	bool bEdited = false;
	
	IndexParams.RunIndex = -1;
	IndexParams.EditAreaIndex = 0;
	IndexParams.ReplacedVoxel = FVoxel();
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
						IndexParams.CurrentRLERun = IndexParams.NewVoxelGrid->GetData()[IndexParams.NewVoxelGrid->Num() - IndexParams.EditAreaIndex];
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

					if (!bEdited && x == VoxelChange->VoxelPosition.X && z == VoxelChange->VoxelPosition.Z &&
						IndexParams.YStart <= VoxelChange->VoxelPosition.Y)
					{

						const auto RunEnd = IndexParams.YStart + YEnd;
						if (RunEnd >= VoxelChange->VoxelPosition.Y)
						{
							//MID RUN EDIT
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
				if (IndexParams.NewVoxelGrid != nullptr && IndexParams.NewVoxelGrid->Num() > 1 && IndexParams.NewVoxelGrid->Last().Voxel == IndexParams.NewVoxelGrid->GetData()[IndexParams.NewVoxelGrid->Num() - 2].Voxel)
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
					CreateFace(MeshVars, FStaticMergeData::FrontFaceData, InitialPosition, IndexParams.CurrentRLERun, YEnd);

					// Back
					CreateFace(MeshVars, FStaticMergeData::BackFaceData, InitialPosition, IndexParams.CurrentRLERun, YEnd);

					// Top
					CreateFace(MeshVars, FStaticMergeData::TopFaceData, InitialPosition, IndexParams.CurrentRLERun, YEnd);

					// Bottom
					CreateFace(MeshVars, FStaticMergeData::BottomFaceData, InitialPosition, IndexParams.CurrentRLERun, YEnd);

					// Right
					CreateFace(MeshVars, FStaticMergeData::RightFaceData, InitialPosition, IndexParams.CurrentRLERun, YEnd);

					// Left
					CreateFace(MeshVars, FStaticMergeData::LeftFaceData, InitialPosition, IndexParams.CurrentRLERun, YEnd);
				}

				IndexParams.TraversedRun += YEnd;
				IndexParams.YStart += YEnd;
			}

			if (VoxelChange != nullptr && !bEdited && VoxelChange->VoxelPosition.Y == 0 && (x == VoxelChange->VoxelPosition.X && z + 1 ==
				VoxelChange->VoxelPosition.Z || x + 1 == VoxelChange->VoxelPosition.X && VoxelChange->VoxelPosition.Z == 0 && z == ChunkDimension))
			{
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
			const auto RunAtPrevIndex = IndexParams.NewVoxelGrid->GetData()[i-1];
			
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
		//-----------

		TestRunIndex += IndexParams.NewVoxelGrid->GetData()[0].RunLenght;

		//MEASURE ----
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

		// TODO: remove this
		if (IndexParams.NewVoxelGrid->Num() == 1 && IndexParams.NewVoxelGrid->GetData()[0].IsVoxelEmpty())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
		}
		else if (IndexParams.NewVoxelGrid->Num() >= 1 && IndexParams.NewVoxelGrid->Num() <= 3)
		{
			bool bEmpty = true;
			for (auto Run : *IndexParams.NewVoxelGrid)
			{
				if (!Run.IsVoxelEmpty())
				{
					bEmpty = false;
				}
			}

			if (bEmpty)
			{
				IndexParams.NewVoxelGrid->GetData()[0].RunLenght += IndexParams.NewVoxelGrid->Last().RunLenght;
				IndexParams.NewVoxelGrid->RemoveAt(1);
				MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
			}
		}

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

			NextRLERun.RunLenght--;
			IndexParams.ReplacedVoxel = NextRLERun.Voxel;

			if (NextRLERun.IsRunEmpty())
			{
				IndexParams.RunIndex++;
				NextRunIndex++;

				if (IndexParams.VoxelGrid->IsValidIndex(NextRunIndex))
				{
					NextRLERun = IndexParams.VoxelGrid->GetData()[NextRunIndex];
					if (NextRLERun.Voxel == LastRLERun.Voxel)
					{
						IndexParams.RunIndex++;
						LastRLERun.RunLenght += NextRLERun.RunLenght;
					}
				}
			}
		}

		IndexParams.CurrentRLERun = LastRLERun;
	}
	else if (IndexParams.CurrentRLERun.Voxel != IndexParams.EditVoxel)
	{
		auto& LastRLERun = IndexParams.NewVoxelGrid->Last();
		const auto MidRunLenght = IndexParams.TraversedRun + IndexParams.VoxelChange->VoxelPosition.Y - IndexParams.YStart;

		const FRLEVoxel SplitRLERun(LastRLERun.RunLenght - MidRunLenght - 1, LastRLERun.Voxel);

		LastRLERun.RunLenght = MidRunLenght;
		IndexParams.ReplacedVoxel = LastRLERun.Voxel;
		IndexParams.CurrentRLERun = LastRLERun;

		CalculateSplitRun(SplitRLERun, IndexParams);
	}
	else
	{
		return false;
	}

	return true;
}

void  URLERunDirectionalMesher::CalculateSplitRun(const FRLEVoxel& SplitRLERun, FIndexParams& IndexParams)
{
	if (SplitRLERun.IsRunEmpty())
	{
		const int NextIndex = IndexParams.RunIndex + 1;
		if (IndexParams.VoxelGrid->IsValidIndex(NextIndex) && IndexParams.VoxelGrid->GetData()[NextIndex].Voxel == IndexParams.EditVoxel)
		{
			IndexParams.VoxelGrid->GetData()[NextIndex].RunLenght++;
		}else
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

bool URLERunDirectionalMesher::CalculateEndRunEditIndex(FIndexParams& IndexParams)
{
	auto& LastRLERun = IndexParams.NewVoxelGrid->Last();

	if (IndexParams.TraversedRun != LastRLERun.RunLenght)
	{
		if (LastRLERun.Voxel == IndexParams.EditVoxel)
		{
			return false;
		}
		
		const FRLEVoxel SplitRLERun(LastRLERun.RunLenght - IndexParams.TraversedRun - 1, LastRLERun.Voxel);
		LastRLERun.RunLenght = IndexParams.TraversedRun;
		IndexParams.ReplacedVoxel = LastRLERun.Voxel;

		LastRLERun.RunLenght = IndexParams.TraversedRun;
		IndexParams.ReplacedVoxel = LastRLERun.Voxel;

		CalculateSplitRun(SplitRLERun, IndexParams);
		
		auto& lastRLE2 = IndexParams.NewVoxelGrid->Last();

		if (lastRLE2.RunLenght <= 0)
		{
			IndexParams.NewVoxelGrid->RemoveAt(IndexParams.NewVoxelGrid->Num() - 1);
			IndexParams.EditAreaIndex = 1;
			if (IndexParams.VoxelGrid->IsValidIndex(IndexParams.RunIndex + 1))
			{
				auto& nextnextRLEVoxel = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];

				if (nextnextRLEVoxel.Voxel == IndexParams.EditVoxel)
				{
					nextnextRLEVoxel.RunLenght++;
					IndexParams.NewVoxelGrid->RemoveAt(IndexParams.NewVoxelGrid->Num() - 1);
					IndexParams.EditAreaIndex = 0;
				}
			}
		}
		else
		{
			IndexParams.EditAreaIndex = 2;
		}
	}
	else
	{
		if (LastRLERun.Voxel == IndexParams.EditVoxel)
		{
			IndexParams.TraversedRun = LastRLERun.RunLenght;
			IndexParams.EditAreaIndex = 0;
			LastRLERun.RunLenght += 1;

			IndexParams.ReplacedVoxel = LastRLERun.Voxel;

			if (IndexParams.VoxelGrid->IsValidIndex(IndexParams.RunIndex + 1))
			{
				auto& nextnextRLEVoxel = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];
				nextnextRLEVoxel.RunLenght--;
				if (nextnextRLEVoxel.RunLenght <= 0)
				{
					IndexParams.RunIndex++;
					if (IndexParams.VoxelGrid->IsValidIndex(IndexParams.RunIndex + 1))
					{
						auto& nextnextnextRLEVoxel = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];

						if (nextnextnextRLEVoxel.Voxel == LastRLERun.Voxel)
						{
							IndexParams.RunIndex++;
							LastRLERun.RunLenght += nextnextnextRLEVoxel.RunLenght;
						}
					}
				}
			}

			IndexParams.RLEVoxel.RunLenght = LastRLERun.RunLenght;
			return true;
		}

		if (IndexParams.VoxelGrid->IsValidIndex(IndexParams.RunIndex + 1))
		{
			auto& nextRLEVoxel = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];

			if (nextRLEVoxel.Voxel == IndexParams.EditVoxel)
			{
				return false;
			}

			IndexParams.NewVoxelGrid->Emplace(1, IndexParams.EditVoxel);
			nextRLEVoxel.RunLenght--;
			IndexParams.EditAreaIndex = 1;

			IndexParams.ReplacedVoxel = nextRLEVoxel.Voxel;

			if (nextRLEVoxel.RunLenght <= 0)
			{
				IndexParams.RunIndex++;
				if (IndexParams.VoxelGrid->IsValidIndex(IndexParams.RunIndex + 1))
				{
					auto& nextnextRLEVoxel = IndexParams.VoxelGrid->GetData()[IndexParams.RunIndex + 1];

					if (nextnextRLEVoxel.Voxel == LastRLERun.Voxel && nextnextRLEVoxel.Voxel == IndexParams.EditVoxel)
					{
						IndexParams.RunIndex++;
						LastRLERun.RunLenght += nextnextRLEVoxel.RunLenght;
					}
					else if (nextnextRLEVoxel.Voxel == IndexParams.EditVoxel)
					{
						IndexParams.NewVoxelGrid->RemoveAt(IndexParams.NewVoxelGrid->Num() - 1);
						IndexParams.EditAreaIndex = 0;
						nextnextRLEVoxel.RunLenght += 1;
					}
				}
			}
		}
	}

	return true;
}
