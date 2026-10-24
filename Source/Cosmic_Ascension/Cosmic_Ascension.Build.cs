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

		// Same seed, same hash in every binary (guideline section 9.2): the tests and the balance commandlet run the editor
		// module, the player runs the game module. V7 compiles only the editor target with precise floating point, so ask
		// for it everywhere else too. Left alone for the editor target so it keeps the shared engine PCH.
		if (Target.Type != TargetType.Editor)
		{
			FPSemantics = FPSemanticsMode.Precise;
		}

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"UMG", "Slate", "SlateCore",
			"GameplayTags", "Niagara"
		});

		// EngineSettings: UGameMapsSettings, read by the balance commandlet to create its GameInstance (guideline section 9.3),
		// and UGeneralProjectSettings, whose ProjectVersion the CosmicBuild start line prints (World/CosmicGameMode.cpp, MVP
		// item 9).
		// Json: the automation test Data.Integrity reads Tools/data/schema.json (guideline section 7.3, 9.3). Tests only:
		// no Core Logic code parses JSON.
		PrivateDependencyModuleNames.AddRange(new string[] { "EngineSettings", "Json" });

		// CommonUI is intentionally NOT a dependency (guideline section 6.3). Revisit only if gamepad / Steam Deck support becomes a goal.
	}
}
