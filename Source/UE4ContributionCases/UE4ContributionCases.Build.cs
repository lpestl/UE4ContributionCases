// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE4ContributionCases : ModuleRules
{
    public UE4ContributionCases(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", 
            "CoreUObject", 
            "Engine", 
            "UnrealEd",
            "InputCore",
            "ClassViewer",
            "DesktopPlatform"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        PrivateDependencyModuleNames.AddRange(new string[] {"Json", "JsonUtilities",});

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}