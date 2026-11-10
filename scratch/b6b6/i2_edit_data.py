import sys
p = sys.argv[1]
s = open(p, 'rb').read().decode('ascii')


def rep(old, new):
    global s
    assert s.count(old) == 1, (old[:80], s.count(old))
    s = s.replace(old, new)


# include (direct, the rule against transitive includes)
rep('#include "Sim/GalaxySimulationSubsystem.h"\n#include "Tests/CosmicTestHelpers.h"\n',
    '#include "Sim/GalaxySimulationSubsystem.h"\n#include "Tests/CosmicB6bRowNames.h"\n#include "Tests/CosmicTestHelpers.h"\n')

# death comment
rep("// (CosmicHazards::KillsTheRulerOf, Sim.ExtinctionExpiry), so this pin of the data stands as it is; [B-6b6] pins that row.\n",
    "// (CosmicHazards::KillsTheRulerOf, Sim.ExtinctionExpiry), so this pin of the data stands as it is; from [B-6b6] the next pin\n"
    "\t// holds that code death to the one row.\n")

# code-death pin
old = "\t\t\tDeaths == TArray<FString>{ TEXT(\"DT_Hazards.crisis_ai_awakening.OnExpireEffects Death.Hazard\") });\n"
new = old + (
    "\t\t// B6b [B-6b6] (decision 5, DECISIONS_LOG B6b [B-6b2b]): the code's one death - the ruler killed at an extinction's expiry\n"
    "\t\t// (CosmicHazards::KillsTheRulerOf, read by the tag) - comes from one row of the game's hazards, crisis_species_extinction,\n"
    "\t\t// and that row's expiry carries nothing of the data's own: no effect on any side and no branch (the expiry is the code's\n"
    "\t\t// alone - LetPeopleGo, the death, DeposeRuler; the schema's rule 5 holds the same on the sources).\n"
    "\t\tTArray<FName> CodeDeaths;\n"
    "\t\tfor (const FName HazardRow : Game.GetHazardRowNames())\n"
    "\t\t{\n"
    "\t\t\tconst FHazardDef* Hazard = Game.FindHazard(HazardRow);\n"
    "\t\t\tif (Hazard && CosmicHazards::KillsTheRulerOf(Hazard->HazardTag))\n"
    "\t\t\t{\n"
    "\t\t\t\tCodeDeaths.Add(HazardRow);\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t\tTestTrue(*FString::Printf(TEXT(\"The code's one death is the extinction's expiry (decision 5, DECISIONS_LOG B6b [B-6b2b]): exactly crisis_species_extinction kills the ruler at its expiry - another source of death is a new decision (found: %s)\"),\n"
    "\t\t\t*FString::JoinBy(CodeDeaths, TEXT(\", \"), [](FName Row) { return Row.ToString(); })),\n"
    "\t\t\tCodeDeaths == TArray<FName>{ CosmicB6bRows::CrisisSpeciesExtinction });\n"
    "\t\tconst FHazardDef* Extinction = Game.FindHazard(CosmicB6bRows::CrisisSpeciesExtinction);\n"
    "\t\tTestTrue(TEXT(\"and its expiry carries no effect of the data - none on expiry, on the branch or on the neighbours, and a BranchChance of 0\"),\n"
    "\t\t\tExtinction && Extinction->OnExpireEffects.IsEmpty() && Extinction->OnExpireBranchEffects.IsEmpty() && Extinction->OnExpireNeighbourEffects.IsEmpty()\n"
    "\t\t\t&& Extinction->BranchChance == 0.f);\n")
rep(old, new)

# tripwire
rep("\t// (-WithoutB6bData=1) and, with WithoutDeath, a test's world without death. (1) Today's data holds one of B6b's ten rows,\n"
    "\t// project_binding: the helper takes exactly it (a tripwire - [B-6b6] brings rows and updates it). (2) On a full copy - a\n",
    "\t// (-WithoutB6bData=1) and, with WithoutDeath, a test's world without death. (1) Today's data holds four of B6b's ten rows,\n"
    "\t// the passage by the ark, the ark, project_binding and the extinction ([B-6b6]): the helper takes exactly them, in its\n"
    "\t// order (a tripwire - [B-6b7] and [B-6b8] bring rows and update it). (2) On a full copy - a\n")
rep("\t\tTestTrue(*FString::Printf(TEXT(\"Today's data: WithoutB6bData takes out project_binding alone (taken: %s) - [B-6b6] updates this pin with its rows\"), *Names(Today)),\n"
    "\t\t\tToday == TArray<FName>{ CosmicRows::ProjectBinding });\n",
    "\t\tTestTrue(*FString::Printf(TEXT(\"Today's data: WithoutB6bData takes out [B-6b6]'s three rows and project_binding (taken: %s) - [B-6b7] and [B-6b8] update this pin with their rows\"), *Names(Today)),\n"
    "\t\t\tToday == TArray<FName>{ CosmicB6bRows::FeatPassageByArk, CosmicB6bRows::ProjectArk, CosmicRows::ProjectBinding, CosmicB6bRows::CrisisSpeciesExtinction });\n")

# ark margin comment
rep("\t// construction slot), so the feat that reads its record would come without the binding. The game holds no extinction row\n"
    "\t// before [B-6b6]: the margin is held on a copy with a test extinction and a test ark too, under the game's rules - the\n"
    "\t// check bites now (a multiplier of 0.5 there leaves the ark 300 ticks of 250).\n",
    "\t// construction slot), so the feat that reads its record would come without the binding. The game's extinction and ark are\n"
    "\t// held by the first assertion (from [B-6b6]); the copies below hold the edges - a test extinction and a test ark under the\n"
    "\t// game's rules (a multiplier of 0.5 there leaves the ark 300 ticks of 250).\n")

open(p, 'wb').write(s.encode('ascii'))
print("ok")
