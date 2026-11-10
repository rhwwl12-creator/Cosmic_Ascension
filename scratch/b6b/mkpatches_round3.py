"""The new [B-6b2a] mutation patches of the last review round (run from the project root)."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkpatch import write_patch  # noqa: E402

D = "Tools/mutations/B6b/"
POLICY = "Source/Cosmic_Ascension/World/CosmicRulerPolicy.cpp"
EDICTS = "Source/Cosmic_Ascension/Sim/CosmicEdicts.cpp"
SIM = "Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp"
DIPLO = "Source/Cosmic_Ascension/Sim/CosmicDiplomacy.cpp"

SKIP = (
    "\t\t\tif (bForLaw && Held == Vigil\n"
    "\t\t\t\t&& ((State.Civs[CivIndex].LowStabilityTicks > 0 && !RebellionStands()) || State.Civs[CivIndex].Stability < RevokeBelow))\n"
    "\t\t\t{\n"
    "\t\t\t\tcontinue;\n"
    "\t\t\t}\n"
)
SPECS = [
    # The law step takes the vigil's slot while (3) would enact it again (the whole skip gone).
    ("b6b2a_policy_vigil_for_law.patch", POLICY,
     "\t\t\t\tcontinue;\n\t\t\t}\n" + SKIP + "\t\t\tconst int32 Rank",
     "\t\t\t\tcontinue;\n\t\t\t}\n" + "\t\t\tconst int32 Rank"),
    # The band: only the clause 'Stability < RevokeBelow' dropped.
    ("b6b2a_policy_vigil_band.patch", POLICY,
     "\t\t\t\t&& ((State.Civs[CivIndex].LowStabilityTicks > 0 && !RebellionStands()) || State.Civs[CivIndex].Stability < RevokeBelow))\n",
     "\t\t\t\t&& State.Civs[CivIndex].LowStabilityTicks > 0 && !RebellionStands())\n"),
    # MakeRoomFor gives a law no room when the slots are full: free slots only.
    ("b6b2a_policy_law_step_free_slot_only.patch", POLICY,
     "\t\t\treturn true;\n\t\t}\n\t\tconst FEdictDef* Wanted = Tables.FindEdict(Row);\n",
     "\t\t\treturn true;\n\t\t}\n\t\tif (bForLaw)\n\t\t{\n\t\t\treturn false;\n\t\t}\n\t\tconst FEdictDef* Wanted = Tables.FindEdict(Row);\n"),
    # The gate of (4') reads true: the law step runs unasked.
    ("b6b2a_policy_law_step_unasked.patch", POLICY,
     "\tif (CosmicEdicts::AsksForLawsOf(State.Civs[CivIndex], Tables))\n",
     "\tif (CosmicEdicts::AsksForLawsOf(State.Civs[CivIndex], Tables) || CivIndex >= 0)\n"),
    # AsksForLawsOf without its rung check.
    ("b6b2a_asks_for_laws_any_rung.patch", EDICTS,
     "bool CosmicEdicts::AsksForLawsOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)\n{\n"
     "\tif (static_cast<uint8>(Civ.Tier) < static_cast<uint8>(EAscensionTier::Tier4_Transcendent))\n\t{\n\t\treturn false;\n\t}\n",
     "bool CosmicEdicts::AsksForLawsOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)\n{\n"),
    # The AI's treaty step runs unasked.
    ("b6b2a_ai_steps_unasked.patch", SIM,
     "\t\tif (CosmicDiplomacy::AsksForTreatyOf(State, StateIndex, Tables, Rules, CivIndex))\n",
     "\t\tif (CosmicDiplomacy::AsksForTreatyOf(State, StateIndex, Tables, Rules, CivIndex) || CivIndex >= 0)\n"),
    # AITreatyPartnerOf without the gate TreatyBlockOf.
    ("b6b2a_ai_treaty_skips_gate.patch", DIPLO,
     "\t\tif (Bordering[Other] && !State.Civs[Other].bIsPlayerRuled\n"
     "\t\t\t&& TreatyBlockOf(State, Index, Tables, Rules, CivIndex, State.Civs[Other].CivID, CosmicTags::TreatyNonAggression(), true) == ECosmicTreatyBlock::None)\n",
     "\t\tif (Bordering[Other] && !State.Civs[Other].bIsPlayerRuled)\n"),
    # HookEdictsOf without the GalaxyRow test: a law reaches every galaxy.
    ("b6b2a_hooks_any_galaxy.patch", EDICTS,
     "\t\tif (CivIndex == OwnerIndex || Civ.ActiveEdicts.IsEmpty() || Civ.GalaxyRow != System.GalaxyRow)\n",
     "\t\tif (CivIndex == OwnerIndex || Civ.ActiveEdicts.IsEmpty())\n"),
    # HoldsGalaxyLawsOf without the fourth-rung test.
    ("b6b2a_laws_held_any_rung.patch", EDICTS,
     "\treturn static_cast<uint8>(Civ.Tier) >= static_cast<uint8>(EAscensionTier::Tier4_Transcendent) && GalaxyLawCountOf(Civ, Tables) >= GalaxyLawsToHoldOf(Rules);\n",
     "\treturn GalaxyLawCountOf(Civ, Tables) >= GalaxyLawsToHoldOf(Rules);\n"),
]

for name, path, before, after in SPECS:
    write_patch(D + name, path, before, after)
