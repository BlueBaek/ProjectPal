// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectPal : ModuleRules
{
	public ProjectPal(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "AnimGraphRuntime" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
			{ "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}