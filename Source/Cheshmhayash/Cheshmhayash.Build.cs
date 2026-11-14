// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Cheshmhayash : ModuleRules
{
	public Cheshmhayash(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
			"Json",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Cheshmhayash",
			"Cheshmhayash/Variant_Platforming",
			"Cheshmhayash/Variant_Platforming/Animation",
			"Cheshmhayash/Variant_Combat",
			"Cheshmhayash/Variant_Combat/AI",
			"Cheshmhayash/Variant_Combat/Animation",
			"Cheshmhayash/Variant_Combat/Gameplay",
			"Cheshmhayash/Variant_Combat/Interfaces",
			"Cheshmhayash/Variant_Combat/UI",
			"Cheshmhayash/Variant_SideScrolling",
			"Cheshmhayash/Variant_SideScrolling/AI",
			"Cheshmhayash/Variant_SideScrolling/Gameplay",
			"Cheshmhayash/Variant_SideScrolling/Interfaces",
			"Cheshmhayash/Variant_SideScrolling/UI"
		});
	}
}
