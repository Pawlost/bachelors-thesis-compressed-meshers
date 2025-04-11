#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshCollision.h"
#include "RealtimeMeshConfig.h"
#include "RealtimeMeshConfig.h"
#include "RMCVoxelModelActor.generated.h"

UCLASS()
class VOXELMODELGENERATION_API ARMCVoxelModelActor : public AActor
{
	GENERATED_BODY()

public:
	ARMCVoxelModelActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="RealtimeMesh")
	TObjectPtr<URealtimeMeshComponent> RealtimeMeshComponent;

	void ClearMesh() const;
	void PrepareMesh() const;

	const FRealtimeMeshSectionGroupKey GroupKey;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FRealtimeMeshCollisionConfiguration DefaultConfig;
};
