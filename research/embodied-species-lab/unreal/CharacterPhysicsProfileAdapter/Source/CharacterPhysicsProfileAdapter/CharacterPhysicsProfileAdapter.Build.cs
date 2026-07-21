using UnrealBuildTool;

public class CharacterPhysicsProfileAdapter : ModuleRules
{
    public CharacterPhysicsProfileAdapter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[] { "IKRig", "Json", "PhysicsUtilities", "UnrealEd" });
    }
}
