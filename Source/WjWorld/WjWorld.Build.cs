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
            "SlateCore",        // UI 관련 (FSlateBrush 포함)
            "EnhancedInput",    // 입력 시스템 확장
            "GameplayAbilities",  // GAS 핵심
            "GameplayTags",       // 태그 시스템
            "GameplayTasks",      // 태스크 시스템
            "CommonUI",         // 공통 UI 컴포넌트
            "CoreOnline",       // FUniqueNetIdWrapper
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "Niagara",
            "GameplayCameras",  // 카메라 관련 기능
            "GeometryCollectionEngine"  // Chaos Destruction
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
            "Media",            // 기본 Media 모듈
            "MediaAssets",      // Media 에셋들
            "MediaUtils",        // Media 유틸리티 함수들
		});

		PublicIncludePaths.AddRange(new string[] {
            "WjWorld",
            //"WjWorld/Public",
		});

        // Steam 조건부 빌드
        bool bWithSteam = (Target.Platform == UnrealTargetPlatform.Win64);
        //bool bWithSteam = false;
        if (bWithSteam)
		{
			PublicDependencyModuleNames.AddRange(new string[] {
				"Steamworks",
				"OnlineSubsystemSteam",
			});
			PublicDefinitions.Add("WITH_STEAM=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_STEAM=0");
		}
    }
}
