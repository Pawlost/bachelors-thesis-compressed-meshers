#include "VoxelModel/RMCVoxelModelActor.h"

#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSimple.h"

ARMCVoxelModelActor::ARMCVoxelModelActor() : GroupKey(FRealtimeMeshSectionGroupKey::Create(0, FName("Chunk Mesh")))
{
	// Register and set RealTimeMeshComponent which will render and store generated mesh.
	RealtimeMeshComponent = CreateDefaultSubobject<URealtimeMeshComponent>(TEXT("RealtimeMeshComponent"));
	RealtimeMeshComponent->SetMobility(EComponentMobility::Movable);
	SetRootComponent(RealtimeMeshComponent);
}

void ARMCVoxelModelActor::ClearMesh() const
{
	if (!IsValid(this))
	{
		return;
	}

	const auto RealTimeMesh =
		RealtimeMeshComponent->GetRealtimeMeshAs<
			URealtimeMeshSimple>();
	/*
	* This is a workaround to error caused by RealTimeMeshComponent library.
	* Error seem to be copied by the library, for detail see: https://forums.unrealengine.com/t/collision-data-isnt-removed-after-clearing-a-section-if-buseasynccooking-true/268178
	* Should be removed after a fix.
	*/
	FRealtimeMeshCollisionConfiguration Config;
	Config.bUseAsyncCook = false;
	RealTimeMesh->SetCollisionConfig(Config);

	RealtimeMeshComponent->GetRealtimeMeshAs<
		URealtimeMeshSimple>()->RemoveSectionGroup(GroupKey);
}

void ARMCVoxelModelActor::PrepareMesh() const
{
	if (!IsValid(RealtimeMeshComponent))
	{
		return;
	}

	/*
	* Return default values.
	* Should be removed after bUseAsyncCook doesn't cause bugs.
	*/
	const auto RealTimeMesh =
		RealtimeMeshComponent->GetRealtimeMeshAs<
			URealtimeMeshSimple>();
	RealTimeMesh->SetCollisionConfig(DefaultConfig);
}

void ARMCVoxelModelActor::BeginPlay()
{
	// Initialize RealTimeMesh and get default configuration.
	RealtimeMeshComponent->InitializeRealtimeMesh<URealtimeMeshSimple>();
	DefaultConfig = RealtimeMeshComponent->GetRealtimeMeshAs<
		URealtimeMeshSimple>()->GetCollisionConfig();

	Super::BeginPlay();
}

void ARMCVoxelModelActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearMesh();
	Super::EndPlay(EndPlayReason);
}
