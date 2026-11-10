EDITS = [(
    "\tint32 Third = 0;\n"
    "\tfor (const FCivilizationState& Civ : State.Civs)\n"
    "\t{\n"
    "\t\tThird += (!Civ.bIsPlayerRuled && static_cast<int32>(Civ.Tier) >= static_cast<int32>(EAscensionTier::Tier3_Causal)) ? 1 : 0;\n"
    "\t}\n"
    "\tTestTrue(TEXT(\"An AI people reaches the third rung in 20,000 ticks of the game's data\"), Third >= 1);\n",
    "\t// B6b [B-6b6] (the notes' table of the tests whose meaning moves): with the passage by the ark in the data, an AI\n"
    "\t// people climbs past the third rung - the extinction, its own ark and the treaty step (mutation: the brake always shut).\n"
    "\tint32 Fourth = 0;\n"
    "\tfor (const FCivilizationState& Civ : State.Civs)\n"
    "\t{\n"
    "\t\tFourth += (!Civ.bIsPlayerRuled && static_cast<int32>(Civ.Tier) >= static_cast<int32>(EAscensionTier::Tier4_Transcendent)) ? 1 : 0;\n"
    "\t}\n"
    "\tTestTrue(TEXT(\"An AI people reaches the fourth rung in 20,000 ticks of the game's data\"), Fourth >= 1);\n",
)]
