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
            "UMG",              // UI 위젯용
            "Slate",            // UI 관련
            "SlateCore",        // UI 관련
            "ToolWidgets",      // Slate Brush 관련
            "EnhancedInput",    // 입력 시스템 확장
            "GameplayAbilities",  // GAS 핵심
            "GameplayTags",       // 태그 시스템
            "GameplayTasks",      // 태스크 시스템
            "CommonUI",         // 공통 UI 컴포넌트
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
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
