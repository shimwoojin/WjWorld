// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WjWorldEditor : ModuleRules
{
	public WjWorldEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"WjWorld",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"UnrealEd",
			"EditorSubsystem",
			"Slate",
			"SlateCore",
			"InputCore",
			"AssetRegistry",
		});

		PublicIncludePaths.AddRange(new string[] {
			"WjWorldEditor",
		});
	}
}
