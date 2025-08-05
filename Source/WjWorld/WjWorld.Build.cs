// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WjWorld : ModuleRules
{
	public WjWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
            "Media",            // 기본 Media 모듈
            "MediaAssets",      // Media 에셋들
            "MediaUtils"        // Media 유틸리티 함수들
		});

		PublicIncludePaths.AddRange(new string[] {
            "WjWorld",
            //"WjWorld/Public",
		});

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
