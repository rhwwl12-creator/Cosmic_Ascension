// Cosmic Ascension game module. Dependencies follow guideline.md section 6.6.

using UnrealBuildTool;

public class Cosmic_Ascension : ModuleRules
{
	public Cosmic_Ascension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Headers live in sub-folders of the module root (Core/, Sim/, Events/, ...) and are included
		// relative to the module root, e.g. #include "Sim/GalaxySimulationSubsystem.h".
		// UBT (BuildSettingsVersion.V7) does not add the module root itself, so add it here.
		// Private on purpose: no other module consumes these headers, and UBT flags a module root in
		// PublicIncludePaths as "not allowed" (warning currently off by default).
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"UMG", "Slate", "SlateCore",
			"GameplayTags", "Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// CommonUI is intentionally NOT a dependency (guideline section 6.3). Revisit only if gamepad / Steam Deck support becomes a goal.
	}
}
