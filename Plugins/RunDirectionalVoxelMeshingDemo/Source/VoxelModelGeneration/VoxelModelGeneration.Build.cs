using UnrealBuildTool;

public class VoxelModelGeneration : ModuleRules
{
    public VoxelModelGeneration(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "RealtimeMeshComponent", 
                "FastNoise",
                "FastNoiseGenerator", "VoxelMesher",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
    }
}