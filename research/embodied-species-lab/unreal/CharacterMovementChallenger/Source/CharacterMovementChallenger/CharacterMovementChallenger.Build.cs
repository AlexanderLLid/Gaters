using UnrealBuildTool;

public class CharacterMovementChallenger : ModuleRules
{
    public CharacterMovementChallenger(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "Json",
            "AnimGraphRuntime", "AnimationWarpingRuntime"
        });
    }
}
