// OSCRoute Copyright (C) 2023 Pixsper Ltd.

using UnrealBuildTool;

public class OSCRoute : ModuleRules
{
	public OSCRoute(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] { });
		PrivateIncludePaths.AddRange(new string[] { });
		PublicDependencyModuleNames.AddRange(new[] { "Core", "OSC" });
		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"KismetCompiler",
				"BlueprintGraph",
				"GraphEditor",
				"Slate",
				"SlateCore"
			}
			);
		DynamicallyLoadedModuleNames.AddRange(new string[] { });
	}
}
