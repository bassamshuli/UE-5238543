// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Test : ModuleRules
{
	public Test(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG","Paper2D"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.AddRange(new string[] {"Test/Public","Test/Obstacles/Public", "Test/TurnsPhase/Public", "Test/GameIntro/Public", "Test/Soldiers/Public", "Test/GameFeildAndPositioningPhase/Public" });

        PrivateIncludePaths.AddRange(new string[] {"Test/Private","Test/Obstacles/Private", "Test/TurnsPhase/Private", "Test/GameIntro/Private","Test/Soldiers/Private", "Test/GameFeildAndPositioningPhase/Private" });

        PublicDependencyModuleNames.AddRange(new string[] {"Core","CoreUObject","Engine","InputCore","UMG","Slate","SlateCore"});
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}

