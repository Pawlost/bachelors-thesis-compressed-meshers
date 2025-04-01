#include "Mesher/RLERunDirectionalMesher.h"
#include "Mesher/RunDirectionalMesher.h"

#include "Mesher/MeshingUtils/MesherVariables.h"
#include "Voxel/RLEVoxel.h"
#include "Voxel/Grid/RLEVoxelGrid.h"

void URLERunDirectionalMesher::CompressVoxelGrid(FChunk& Chunk, TArray<FVoxel>& VoxelGrid)
{
	auto VoxelGridObject = NewObject<URLEVoxelGrid>();
	VoxelGridObject->VoxelGrid.Reserve(Chunk.ChunkVoxelIdTable.Num());

	VoxelGridObject->VoxelGrid.Emplace(1, VoxelGrid[0]);

	for (int32 x = 1; x < VoxelGrid.Num(); x++)
	{
		const FVoxel Voxel = VoxelGrid[x];
		if (VoxelGridObject->VoxelGrid.Last().Voxel == Voxel)
		{
			VoxelGridObject->VoxelGrid.Last().RunLenght++;
		}
		else
		{
			VoxelGridObject->VoxelGrid.Emplace(1, Voxel);
		}
	}

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

	auto& VoxelGrid = VoxelGridPtr->VoxelGrid;

	TArray<FRLEVoxel> NewVoxelGrid;
	FVoxel EditVoxel;

	//Measure---
	int voxelCount = 0;
	//---

	if (VoxelChange != nullptr)
	{
		NewVoxelGrid.Reserve(VoxelGrid.Num() + 1);
		EditVoxel = VoxelGenerator->GetVoxelByName(VoxelChange->VoxelName);


		// Measure---
		UE_LOG(LogTemp, Warning, TEXT("Edit Voxel: %d, Position: X %d,  Y %d, Z %d,"), EditVoxel.VoxelId,
		       VoxelChange->VoxelPosition.X, VoxelChange->VoxelPosition.Y, VoxelChange->VoxelPosition.Z);

		for (auto count : MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable)
		{
			voxelCount += count.Value;
		}
		//----

		if (!EditVoxel.IsEmptyVoxel())
		{
			auto& VoxelTable = MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable;
			if (!VoxelTable.Contains(EditVoxel.VoxelId))
			{
				VoxelTable.Add(EditVoxel.VoxelId, 0);
			}
		}
	}

	InitFaceContainers(MeshVars);

	bool bEdited = false;
	int32 RunIndex = -1;
	int32 EditAreaIndex = 0;
	auto CurrentRLEVoxel = VoxelGrid[0];
	auto TraversedRun = CurrentRLEVoxel.RunLenght;
	auto YEnd = 0;
	FVoxel ReplacedVoxel = FVoxel();

	const int ChunkDimension = VoxelGenerator->GetVoxelCountPerChunkDimension();

	// Traverse through voxel grid
	for (int x = 0; x < ChunkDimension; x++)
	{
		for (int z = 0; z < ChunkDimension; z++)
		{
			int YStart = 0;
			bool bInEditArea = false;

			// Calculate index
			while (YStart < ChunkDimension)
			{
				if (TraversedRun == CurrentRLEVoxel.RunLenght)
				{
					if (EditAreaIndex == 0)
					{
						bInEditArea = false;
						RunIndex++;
						CurrentRLEVoxel = VoxelGrid[RunIndex];
					}
					else
					{
						bInEditArea = true;
						CurrentRLEVoxel = NewVoxelGrid[NewVoxelGrid.Num() - EditAreaIndex];
						EditAreaIndex--;
					}

					YEnd = CurrentRLEVoxel.RunLenght;
					TraversedRun = 0;
				}
				else
				{
					YEnd = CurrentRLEVoxel.RunLenght - TraversedRun;
				}

				// No action is need when in edit area, otherwise try to apply change and add next run to a new array
				if (VoxelChange != nullptr && !bInEditArea)
				{
					// Add only full runs
					if (TraversedRun == 0)
					{
						// Add only outside edit area
						NewVoxelGrid.Add(CurrentRLEVoxel);
					}

					if (bEdited == false && x == VoxelChange->VoxelPosition.X && z == VoxelChange->VoxelPosition.Z &&
						YStart <= VoxelChange->VoxelPosition.Y && YStart + YEnd >= VoxelChange->
						VoxelPosition.Y)
					{
						//EDITED HERE
						bEdited = true;

						switch (CalculateEditIndexMidRun(TraversedRun, NewVoxelGrid, CurrentRLEVoxel, VoxelChange, YStart,
						                                 YEnd,
						                                 EditVoxel, EditAreaIndex, VoxelGrid, RunIndex, ReplacedVoxel))
						{
							case Continue:
								continue;
							case Return:
								return;
							default:
								break;
						}
					}
				}

				//MEASURE--------
				if (NewVoxelGrid.Num() > 1 && NewVoxelGrid.Last().Voxel == NewVoxelGrid[NewVoxelGrid.Num() - 2].Voxel)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE"));
				}
				//-----

				// Step to end
				if (YStart + YEnd > ChunkDimension)
				{
					YEnd = ChunkDimension - YStart;
				}

				if (!CurrentRLEVoxel.IsEmptyVoxel())
				{
					// Generate run faces
					auto InitialPosition = FIntVector(x, YStart, z);

					// Front
					CreateFace(MeshVars, FStaticMergeData::FrontFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Back
					CreateFace(MeshVars, FStaticMergeData::BackFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Top
					CreateFace(MeshVars, FStaticMergeData::TopFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Bottom
					CreateFace(MeshVars, FStaticMergeData::BottomFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Right
					CreateFace(MeshVars, FStaticMergeData::RightFaceData, InitialPosition, CurrentRLEVoxel, YEnd);

					// Left
					CreateFace(MeshVars, FStaticMergeData::LeftFaceData, InitialPosition, CurrentRLEVoxel, YEnd);
				}

				TraversedRun += YEnd;
				YStart += YEnd;
			}

			switch (CalculateEditIndexEndRun(VoxelChange, bEdited, x, z, NewVoxelGrid, TraversedRun, EditVoxel,
			                                 CurrentRLEVoxel,
			                                 ReplacedVoxel, EditAreaIndex, VoxelGrid, RunIndex))
			{
			case Return:
				return;
			default:
				break;
			}
		}
	}

	GenerateMeshFromFaces(MeshVars);

	if (VoxelChange != nullptr)
	{
		int VoxelDimension = VoxelGenerator->GetVoxelCountPerChunk();
		int TestRunIndex = 0;

		//MEASURE ----
		for (int i = NewVoxelGrid.Num() - 1; i > 0; i--)
		{
			TestRunIndex += NewVoxelGrid[i].RunLenght;

			if (NewVoxelGrid[i].RunLenght <= 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Wrong run: %d"), NewVoxelGrid[i].RunLenght);
			}

			if (NewVoxelGrid[i].Voxel == NewVoxelGrid[i - 1].Voxel)
			{
				auto firstRun = NewVoxelGrid[i];
				auto secondRun = NewVoxelGrid[i - 1];
				UE_LOG(LogTemp, Error, TEXT("Grid Index1: %d, Grid Index2: %d"), i, i-1);
				UE_LOG(LogTemp, Error, TEXT("Voxel: %d"), firstRun.Voxel.VoxelId);
				UE_LOG(LogTemp, Error, TEXT("First Run: %d"), firstRun.RunLenght);
				UE_LOG(LogTemp, Error, TEXT("Second Run: %d"), secondRun.RunLenght);
				UE_LOG(LogTemp, Error, TEXT("Reversed Run Index: %d"), TestRunIndex);
			}
		}
		//-----------

		TestRunIndex += NewVoxelGrid[0].RunLenght;

		//MEASURE ----
		if (VoxelDimension != TestRunIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("Run lenght does not match: %d; Dimension %d"), TestRunIndex, VoxelDimension);
		}
		//-----

		if (!EditVoxel.IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[EditVoxel.VoxelId]++;
		}
		else
		{
			//MEASURE ----
			voxelCount--;
		}

		if (!ReplacedVoxel.IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[ReplacedVoxel.VoxelId]--;

			if (MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable[ReplacedVoxel.VoxelId] <= 0)
			{
				MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Remove(ReplacedVoxel.VoxelId);
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
		if (NewVoxelGrid.Num() == 1 && NewVoxelGrid[0].IsEmptyVoxel())
		{
			MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
		}
		else if (NewVoxelGrid.Num() >= 1 && NewVoxelGrid.Num() <= 3)
		{
			bool empty = true;
			for (auto Run : NewVoxelGrid)
			{
				if (!Run.IsEmptyVoxel())
				{
					empty = false;
				}
			}

			if (empty)
			{
				NewVoxelGrid[0].RunLenght += NewVoxelGrid.Last().RunLenght;
				NewVoxelGrid.RemoveAt(1);
				MeshVars.ChunkParams.OriginalChunk->ChunkVoxelIdTable.Empty();
			}
		}

		VoxelGridPtr->VoxelGrid = NewVoxelGrid;
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

URLERunDirectionalMesher::EReturnFlow URLERunDirectionalMesher::CalculateEditIndexMidRun(
	int& TraversedRun, TArray<FRLEVoxel>& NewVoxelGrid, FRLEVoxel& CurrentRLEVoxel, const FVoxelChange* VoxelChange, int& YStart,
	int& YEnd, const FVoxel& EditVoxel, int& EditAreaIndex, TArray<FRLEVoxel>& VoxelGrid, int& RunIndex,
	FVoxel& ReplacedVoxel)
{
	if (YStart + YEnd == VoxelChange->VoxelPosition.Y)
	{
		auto& LastRLEVoxel = NewVoxelGrid.Last();
		
		if (CurrentRLEVoxel.Voxel == EditVoxel)
		{
			LastRLEVoxel.RunLenght++;
		}
		else
		{
			NewVoxelGrid.Emplace(1, EditVoxel);
			EditAreaIndex = 1;
		}

		if (VoxelGrid.IsValidIndex(RunIndex + 1))
		{
			auto& NextRLEVoxel = VoxelGrid[RunIndex + 1];

			if (NextRLEVoxel.Voxel == EditVoxel)
			{
				return Return;
			}

			NextRLEVoxel.RunLenght--;
			ReplacedVoxel = NextRLEVoxel.Voxel;

			if (NextRLEVoxel.RunLenght <= 0)
			{
				RunIndex++;

				if (VoxelGrid.IsValidIndex(RunIndex + 1))
				{
					NextRLEVoxel = VoxelGrid[RunIndex + 1];
					if (NextRLEVoxel.Voxel == NewVoxelGrid.Last().Voxel)
					{
						RunIndex++;
						NewVoxelGrid.Last().RunLenght += NextRLEVoxel.RunLenght;
					}
				}
			}
		}
		
		CurrentRLEVoxel = LastRLEVoxel;
		YEnd = CurrentRLEVoxel.RunLenght - TraversedRun;
	}
	else if (CurrentRLEVoxel.Voxel != EditVoxel)
	{
		UE_LOG(LogTemp, Warning, TEXT("HERE10"))
		auto& lastRLE = NewVoxelGrid.Last();
		auto newRunLenght = TraversedRun + VoxelChange->VoxelPosition.Y - YStart;

		FRLEVoxel NewRLE(1, EditVoxel);
		FRLEVoxel nextRLE(CurrentRLEVoxel.RunLenght - newRunLenght - 1, CurrentRLEVoxel.Voxel);

		UE_LOG(LogTemp, Warning, TEXT("HERE14"))
		lastRLE.RunLenght = newRunLenght;

		ReplacedVoxel = lastRLE.Voxel;

		CurrentRLEVoxel = lastRLE;

		if (lastRLE.RunLenght <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE15"))
			NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
		}

		if (!NewVoxelGrid.IsEmpty() && NewVoxelGrid.Last().Voxel == NewRLE.Voxel)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE16"))
			NewRLE.RunLenght--;
		}

		if (nextRLE.RunLenght > 0 && NewRLE.RunLenght > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE17"))
			if (NewRLE.Voxel == nextRLE.Voxel)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE18"))
				nextRLE.RunLenght++;
				EditAreaIndex = 1;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE19"))
				if (NewRLE.RunLenght > 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE20"))
					NewVoxelGrid.Add(NewRLE);
					EditAreaIndex = 2;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE21"))
					NewVoxelGrid.Last().RunLenght++;
					YStart++;
					EditAreaIndex = 1;
				}
			}
			NewVoxelGrid.Add(nextRLE);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE22"))
			bool add = true;
			if (VoxelGrid.IsValidIndex(RunIndex + 1))
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE23"))
				auto& nextRLEVoxel = VoxelGrid[RunIndex + 1];
				if (nextRLEVoxel.Voxel == NewRLE.Voxel)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE24"))
					if (NewRLE.RunLenght <= 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE25"))
						RunIndex++;
						TraversedRun = NewVoxelGrid.Last().RunLenght;
						NewVoxelGrid.Last().RunLenght += nextRLEVoxel.RunLenght + 1;
						CurrentRLEVoxel = NewVoxelGrid.Last();
						return Continue;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE26"))
						nextRLEVoxel.RunLenght++;
					}
					add = false;
				}
				else if (NewRLE.RunLenght <= 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE27"))
					NewVoxelGrid.Last().RunLenght++;
					YStart++;
					return Continue;
				}
			}

			if (add)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE28"))
				NewVoxelGrid.Add(NewRLE);
				EditAreaIndex = 1;
			}
		}


		if ((CurrentRLEVoxel.RunLenght <= 0 || newRunLenght == 0) && EditAreaIndex > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE29"))
			CurrentRLEVoxel = NewVoxelGrid[NewVoxelGrid.Num() - EditAreaIndex];
			EditAreaIndex--;
		}

		YEnd = CurrentRLEVoxel.RunLenght - TraversedRun;
	}
	else
	{
		return Return;
	}

	return MeshRun;
}

URLERunDirectionalMesher::EReturnFlow URLERunDirectionalMesher::CalculateEditIndexEndRun(
	const FVoxelChange* VoxelChange, bool& edited, int x, int z,
	TArray<FRLEVoxel>& NewVoxelGrid, int& size, FVoxel& EditVoxel,
	FRLEVoxel& RLEVoxel,
	FVoxel& ReplacedVoxel, int& editIndex,
	TArray<FRLEVoxel>& VoxelGrid, int& globalIndex)
{
	if (VoxelChange != nullptr && edited == false && (x == VoxelChange->VoxelPosition.X && z + 1 == VoxelChange->
			VoxelPosition.Z || x + 1 == VoxelChange->VoxelPosition.X && VoxelChange->VoxelPosition.Z == 0 && z == 31) &&
		VoxelChange->VoxelPosition.Y == 0)
	{
		edited = true;

		auto& lastVoxel = NewVoxelGrid.Last();

		if (size != lastVoxel.RunLenght)
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE31"))

			if (lastVoxel.Voxel == EditVoxel)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE32"))
				return EReturnFlow::Return;
			}

			auto& lastRLE = NewVoxelGrid.Last();
			int runLenght = lastRLE.RunLenght - size - 1;
			lastRLE.RunLenght = size;
			NewVoxelGrid.Emplace(1, EditVoxel);
			NewVoxelGrid.Emplace(runLenght, RLEVoxel.Voxel);
			size = 0;
			RLEVoxel.RunLenght = 0;

			ReplacedVoxel = RLEVoxel.Voxel;

			auto& lastRLE2 = NewVoxelGrid.Last();

			if (lastRLE2.RunLenght <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE33"))
				NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
				editIndex = 1;
				if (VoxelGrid.IsValidIndex(globalIndex + 1))
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE34"))
					auto& nextnextRLEVoxel = VoxelGrid[globalIndex + 1];

					if (nextnextRLEVoxel.Voxel == EditVoxel)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE35"))
						nextnextRLEVoxel.RunLenght++;
						NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
						editIndex = 0;
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE36"))
				editIndex = 2;
			}

			return Continue;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HERE55"))

			if (lastVoxel.Voxel == EditVoxel)
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE37"))
				size = lastVoxel.RunLenght;
				editIndex = 0;
				lastVoxel.RunLenght += 1;

				ReplacedVoxel = lastVoxel.Voxel;

				if (VoxelGrid.IsValidIndex(globalIndex + 1))
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE44"))
					auto& nextnextRLEVoxel = VoxelGrid[globalIndex + 1];
					nextnextRLEVoxel.RunLenght--;
					if (nextnextRLEVoxel.RunLenght <= 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE45"))
						globalIndex++;
						if (VoxelGrid.IsValidIndex(globalIndex + 1))
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE46"))
							auto& nextnextnextRLEVoxel = VoxelGrid[globalIndex + 1];

							if (nextnextnextRLEVoxel.Voxel == lastVoxel.Voxel)
							{
								UE_LOG(LogTemp, Warning, TEXT("HERE47"))
								globalIndex++;
								lastVoxel.RunLenght += nextnextnextRLEVoxel.RunLenght;
							}
						}
					}
				}

				RLEVoxel.RunLenght = lastVoxel.RunLenght;
				return Continue;
			}

			if (VoxelGrid.IsValidIndex(globalIndex + 1))
			{
				UE_LOG(LogTemp, Warning, TEXT("HERE39"))
				auto& nextRLEVoxel = VoxelGrid[globalIndex + 1];

				if (nextRLEVoxel.Voxel == EditVoxel)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE40"))
					return Return;
				}

				NewVoxelGrid.Emplace(1, EditVoxel);
				nextRLEVoxel.RunLenght--;
				editIndex = 1;

				ReplacedVoxel = nextRLEVoxel.Voxel;

				if (nextRLEVoxel.RunLenght <= 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("HERE41"))
					globalIndex++;
					if (VoxelGrid.IsValidIndex(globalIndex + 1))
					{
						UE_LOG(LogTemp, Warning, TEXT("HERE42"))
						auto& nextnextRLEVoxel = VoxelGrid[globalIndex + 1];

						if (nextnextRLEVoxel.Voxel == lastVoxel.Voxel && nextnextRLEVoxel.Voxel == EditVoxel)
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE43"))
							globalIndex++;
							lastVoxel.RunLenght += nextnextRLEVoxel.RunLenght;
						}
						else if (nextnextRLEVoxel.Voxel == EditVoxel)
						{
							UE_LOG(LogTemp, Warning, TEXT("HERE50"))
							NewVoxelGrid.RemoveAt(NewVoxelGrid.Num() - 1);
							editIndex = 0;
							nextnextRLEVoxel.RunLenght += 1;
						}
					}
				}
			}
		}
	}

	return Continue;
}
