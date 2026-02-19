// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectPal : ModuleRules
{
	public ProjectPal(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime", "Niagara" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule", "GameplayTasks",
			"NavigationSystem", "UMG", "Slate", "SlateCore"
		});
		
		// 핵심 수정 부분: 에디터일 때만 AnimationModifiers 관련 참조 허용
		if (Target.Type == TargetRules.TargetType.Editor)
		{
			// 로그에서 언급된 AnimationModifiers를 에디터 전용으로 격리
			PublicDependencyModuleNames.Add("AnimationModifiers");
		}
	}
}