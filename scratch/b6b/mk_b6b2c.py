"""Write the [B-6b2c] mutation patches (project root): each is a list of (file, before, after) hunks against the working tree."""
import difflib
import os
import sys

sys.stdout.reconfigure(encoding="utf-8")
OUT = "Tools/mutations/B6b"
MEGA = "Source/Cosmic_Ascension/Sim/CosmicMegaprojects.cpp"
ASCENT = "Source/Cosmic_Ascension/Sim/CosmicAscent.cpp"
ASC = "Source/Cosmic_Ascension/Ascension/AscensionSubsystem.cpp"
SEAL = "Source/Cosmic_Ascension/Sim/CosmicSeal.cpp"
SIM = "Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp"
EVENTS = "Source/Cosmic_Ascension/Events/CosmicEventSubsystem.cpp"
PANEL = "Source/Cosmic_Ascension/UI/CosmicSystemPanelWidget.cpp"
CONFLICT = "Source/Cosmic_Ascension/Sim/CosmicConflict.cpp"

M = {}

M["b6b2c_delay_second_rung"] = [(MEGA,
    "const bool bUpper = !State.Civs[OwnerIndex].bIsPlayerRuled && static_cast<int32>(Row->RequiredTier) >= static_cast<int32>(EAscensionTier::Tier3_Causal);",
    "const bool bUpper = !State.Civs[OwnerIndex].bIsPlayerRuled && static_cast<int32>(Row->RequiredTier) >= static_cast<int32>(EAscensionTier::Tier2_Stellar);")]

M["b6b2c_delay_on_response"] = [(MEGA,
    "\t\t\t\tSystem.ResponseProgress += Rate;\n",
    "\t\t\t\tSystem.ResponseProgress += (!State.Civs[OwnerIndex].bIsPlayerRuled && static_cast<int32>(Row->RequiredTier) >= static_cast<int32>(EAscensionTier::Tier3_Causal)) ? Rate * UpperRungMultiplierOf(Ctx.Rules) : Rate;\n")]

M["b6b2c_delay_bad_knob_as_is"] = [(MEGA,
    "\t\t\tUE_LOG(LogCosmic, Error, TEXT(\"DT_Rules.AIUpperRungProgressMultiplier %f is not above 0: read as 1\"), Multiplier);\n\t\t\treturn 1.f;\n",
    "\t\t\tUE_LOG(LogCosmic, Error, TEXT(\"DT_Rules.AIUpperRungProgressMultiplier %f is not above 0: read as 1\"), Multiplier);\n\t\t\treturn Multiplier;\n")]

M["b6b2c_brake_skips_ruled"] = [(ASCENT,
    "Count += (Civ.GalaxyRow == GalaxyRow && static_cast<int32>(Civ.Tier)",
    "Count += (!Civ.bIsPlayerRuled && Civ.GalaxyRow == GalaxyRow && static_cast<int32>(Civ.Tier)")]

M["b6b2c_brake_order_reversed"] = [(ASC,
    "\tfor (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)\n",
    "\tfor (int32 CivIndex = State.Civs.Num() - 1; CivIndex >= 0; --CivIndex)\n")]

M["b6b2c_brake_cap_zero_brakes"] = [(ASCENT,
    "\tif (!Galaxy || Galaxy->AIFourthRungCap <= 0)\n",
    "\tif (!Galaxy)\n")]

M["b6b2c_brake_missing_row_closes"] = [(ASCENT,
    "\tif (!Galaxy || Galaxy->AIFourthRungCap <= 0)\n\t{\n\t\treturn true;\n\t}\n",
    "\tif (!Galaxy)\n\t{\n\t\treturn false;\n\t}\n\tif (Galaxy->AIFourthRungCap <= 0)\n\t{\n\t\treturn true;\n\t}\n")]

M["b6b2c_brake_counts_third_rung"] = [(ASCENT,
    ">= static_cast<int32>(EAscensionTier::Tier4_Transcendent)) ? 1 : 0;",
    ">= static_cast<int32>(EAscensionTier::Tier3_Causal)) ? 1 : 0;")]

M["b6b2c_brake_counts_landed_transits"] = [(ASCENT,
    "Count += (Transit.ToGalaxyRow == GalaxyRow && Transit.ArriveTick >= State.Tick) ? 1 : 0;",
    "Count += (Transit.ToGalaxyRow == GalaxyRow) ? 1 : 0;")]

M["b6b2c_brake_counted_once_per_tick"] = [(ASCENT,
    "\tint32 Count = 0;\n\tfor (const FCivilizationState& Civ : State.Civs)\n",
    "\tint32 Count = 0;\n\tstatic int64 OnceTick = -1;\n\tstatic int32 OnceCount = 0;\n\tif (OnceTick == State.Tick)\n\t{\n\t\treturn OnceCount < Galaxy->AIFourthRungCap;\n\t}\n\tfor (const FCivilizationState& Civ : State.Civs)\n"),
    (ASCENT,
    "\treturn Count < Galaxy->AIFourthRungCap;\n",
    "\tOnceTick = State.Tick;\n\tOnceCount = Count;\n\treturn Count < Galaxy->AIFourthRungCap;\n")]

M["b6b2c_brake_ruled_too"] = [(ASC,
    "if (!bPlayer && State.Civs[CivIndex].Tier == EAscensionTier::Tier3_Causal && ",
    "if (State.Civs[CivIndex].Tier == EAscensionTier::Tier3_Causal && ")]

M["b6b2c_brake_fifth_too"] = [(ASC,
    "if (!bPlayer && State.Civs[CivIndex].Tier == EAscensionTier::Tier3_Causal && ",
    "if (!bPlayer && static_cast<int32>(State.Civs[CivIndex].Tier) >= static_cast<int32>(EAscensionTier::Tier3_Causal) && ")]

M["b6b2c_rebind_moves_sealed_tick"] = [(SEAL,
    "\t\tconst FTranscendentDef* Def = Transcendent.bSealed ? nullptr : Tables.FindTranscendent(Transcendent.TranscendentID);\n",
    "\t\tconst FTranscendentDef* Def = Tables.FindTranscendent(Transcendent.TranscendentID);\n"),
    (SEAL,
    "\tif (!State.Transcendents.IsValidIndex(TranscendentIndex) || State.Transcendents[TranscendentIndex].bSealed)\n",
    "\tif (!State.Transcendents.IsValidIndex(TranscendentIndex))\n")]

M["b6b2c_bind_target_any_realm"] = [(SEAL,
    "\t\tif (Def && Def->Allegiance == AbyssalTag)\n",
    "\t\tif (Def)\n")]

M["b6b2c_binding_does_not_bind"] = [(MEGA,
    "\t\t\t\t\t\tCosmicSeal::Bind(State, BindTarget, State.Tick);\n",
    "")]

M["b6b2c_binding_draws"] = [(MEGA,
    "\t\t\t\t\t\tCosmicSeal::Bind(State, BindTarget, State.Tick);\n",
    "\t\t\t\t\t\tCosmicSeal::Bind(State, BindTarget, State.Tick);\n\t\t\t\t\t\tCtx.Stream.GetFraction();\n")]

M["b6b2c_target_once_per_pass"] = [(MEGA,
    "\t\t\tconst int32 BindTarget = (Row && bBinding) ? CosmicSeal::BindTargetOf(State, Ctx.Tables) : INDEX_NONE;\n",
    "\t\t\tstatic int64 PassTick = -1;\n\t\t\tstatic int32 PassTarget = INDEX_NONE;\n\t\t\tif (PassTick != State.Tick)\n\t\t\t{\n\t\t\t\tPassTick = State.Tick;\n\t\t\t\tPassTarget = CosmicSeal::BindTargetOf(State, Ctx.Tables);\n\t\t\t}\n\t\t\tconst int32 BindTarget = (Row && bBinding) ? PassTarget : INDEX_NONE;\n")]

M["b6b2c_bound_line_first"] = [(SIM,
    "\t\t\t\tLog->PushVariants(Line, Row->CompleteTemplates, Args);\n\t\t\t\t// The binding's line right after it",
    "\t\t\t\t// The binding's line right after it"),
    (SIM,
    "\t\t\t\t\tLog->PushRow(CosmicRows::LogFallenBound, Line.Severity, BoundArgs, Line.FocusSystem, Line.FocusCiv);\n\t\t\t\t}\n",
    "\t\t\t\t\tLog->PushRow(CosmicRows::LogFallenBound, Line.Severity, BoundArgs, Line.FocusSystem, Line.FocusCiv);\n\t\t\t\t}\n\t\t\t\tLog->PushVariants(Line, Row->CompleteTemplates, Args);\n")]

M["b6b2c_binding_read_as_sealed"] = [(EVENTS,
    "\tcase EConditionType::MegaprojectComplete:\n",
    "\tcase EConditionType::MegaprojectComplete:\n\t\tif (Condition.Key == CosmicRows::ProjectBinding)\n\t\t{\n\t\t\treturn State.Transcendents.ContainsByPredicate([&View](const FTranscendentState& Each) { return Each.bSealed && Each.SealedTick >= View.SinceTick; });\n\t\t}\n")]

M["b6b2c_nofallen_before_rowtier"] = [(MEGA,
    "\tif (static_cast<int32>(State.Civs[OwnerIndex].Tier) < static_cast<int32>(Row->RequiredTier))\n\t{\n\t\treturn ECosmicStartBlock::RowTier;\n\t}\n\t// The binding after the rung, before the slots (B6b header 2.5): no fallen one left, nothing to start.\n\tif (RowName == CosmicRows::ProjectBinding && CosmicSeal::BindTargetOf(State, Tables) == INDEX_NONE)\n\t{\n\t\treturn ECosmicStartBlock::NoFallen;\n\t}\n",
    "\tif (RowName == CosmicRows::ProjectBinding && CosmicSeal::BindTargetOf(State, Tables) == INDEX_NONE)\n\t{\n\t\treturn ECosmicStartBlock::NoFallen;\n\t}\n\tif (static_cast<int32>(State.Civs[OwnerIndex].Tier) < static_cast<int32>(Row->RequiredTier))\n\t{\n\t\treturn ECosmicStartBlock::RowTier;\n\t}\n")]

M["b6b2c_nofallen_after_slot"] = [(MEGA,
    "\t// The binding after the rung, before the slots (B6b header 2.5): no fallen one left, nothing to start.\n\tif (RowName == CosmicRows::ProjectBinding && CosmicSeal::BindTargetOf(State, Tables) == INDEX_NONE)\n\t{\n\t\treturn ECosmicStartBlock::NoFallen;\n\t}\n",
    ""),
    (MEGA,
    "\treturn System.ActiveMegaproject == RowName ? ECosmicStartBlock::Running : ECosmicStartBlock::None;\n",
    "\tif (System.ActiveMegaproject == RowName)\n\t{\n\t\treturn ECosmicStartBlock::Running;\n\t}\n\treturn (RowName == CosmicRows::ProjectBinding && CosmicSeal::BindTargetOf(State, Tables) == INDEX_NONE) ? ECosmicStartBlock::NoFallen : ECosmicStartBlock::None;\n")]

M["b6b2c_build_button_any_rung"] = [(PANEL,
    "\t\t\t\tif (!Tables.IsHazardAnswer(Row) && RuledRung + 1 < static_cast<int32>(Def->RequiredTier))\n",
    "\t\t\t\tif (!Tables.IsHazardAnswer(Row) && RuledRung + 1 < static_cast<int32>(Def->RequiredTier) && false)\n")]

M["b6b2c_guard_cap_ignored"] = [(CONFLICT,
    "\treturn Delay < RowCap ? Delay : RowCap;\n",
    "\treturn Delay;\n")]

# Data patches (Tools/data): the runner rebuilds the assets.
M["b6b2c_data_ark_margin_half"] = [("Tools/data/DT_Rules.json",
    "\t\t\"AIAscensionSpeedMultiplier\": 1.0,\n",
    "\t\t\"AIAscensionSpeedMultiplier\": 0.5,\n")]

M["b6b2c_data_binding_answers"] = [("Tools/data/DT_Hazards.json", '"Name": "crisis_plague",\n\t\t"HazardTag": "Hazard.Plague",\n\t\t"SourceDoctrine": "",\n\t\t"DisplayName": "성간 역병",\n\t\t"CountdownTicks": 200,\n\t\t"SpawnRule": "Draw",\n\t\t"SpawnChancePerTick": 0.0001,\n\t\t"SpawnConditions": [],\n\t\t"OnExpireEffects": [\n\t\t\t{\n\t\t\t\t"Type": "ChangePopulation",\n\t\t\t\t"Tag": "",\n\t\t\t\t"Key": "None",\n\t\t\t\t"Value": -0.4,\n\t\t\t\t"DurationTicks": 0\n\t\t\t}\n\t\t],\n\t\t"BranchChance": 0.0,\n\t\t"OnExpireBranchEffects": [],\n\t\t"OnExpireNeighbourEffects": [],\n\t\t"OnResolveEffects": [],\n\t\t"RulerResolutions": ["project_relief"],', '"Name": "crisis_plague",\n\t\t"HazardTag": "Hazard.Plague",\n\t\t"SourceDoctrine": "",\n\t\t"DisplayName": "성간 역병",\n\t\t"CountdownTicks": 200,\n\t\t"SpawnRule": "Draw",\n\t\t"SpawnChancePerTick": 0.0001,\n\t\t"SpawnConditions": [],\n\t\t"OnExpireEffects": [\n\t\t\t{\n\t\t\t\t"Type": "ChangePopulation",\n\t\t\t\t"Tag": "",\n\t\t\t\t"Key": "None",\n\t\t\t\t"Value": -0.4,\n\t\t\t\t"DurationTicks": 0\n\t\t\t}\n\t\t],\n\t\t"BranchChance": 0.0,\n\t\t"OnExpireBranchEffects": [],\n\t\t"OnExpireNeighbourEffects": [],\n\t\t"OnResolveEffects": [],\n\t\t"RulerResolutions": ["project_relief", "project_binding"],')]

# Checker patches: refused by python -I Tools/check_data.py before an asset is written.
M["b6b2c_data_binding_causality"] = [("Tools/data/DT_Megaprojects.json",
    "\t\t\"CostPerTick\": {\"Energy\": 4.0, \"Matter\": 4.0, \"Insight\": 0.0, \"Influence\": 0.0, \"Causality\": 0.0},\n",
    "\t\t\"CostPerTick\": {\"Energy\": 4.0, \"Matter\": 4.0, \"Insight\": 0.0, \"Influence\": 0.0, \"Causality\": 1.0},\n")]
M["b6b2c_data_bound_line_names_people"] = [("Tools/data/DT_LogTemplates.json",
    "\t\t\t\"{System}의 하늘에 드리우던 먼 그늘이 묶였다.\",\n",
    "\t\t\t\"{System}의 하늘에 드리우던 먼 그늘이 {Civ}의 일로 묶였다.\",\n")]

# Schema patches: python -I Tools/test_data_schema.py, no build.
M["b6b2c_schema_causality_rule_dropped"] = [("Tools/data/schema.json",
    "\t\t\t\t\t\"then\": [{\"col\": \"CostPerTick.Causality\", \"is\": 0}]\n",
    "\t\t\t\t\t\"then\": [{\"col\": \"CostPerTick.Causality\", \"ge\": 0}]\n")]
M["b6b2c_schema_bound_line_rule_dropped"] = [("Tools/data/schema.json",
    "\t\t\t\t\t\"then\": [{\"col\": \"Templates\", \"slots_in\": [\"System\"]}]\n",
    "\t\t\t\t\t\"then\": [{\"col\": \"Templates\", \"slots_in\": [\"System\", \"Civ\", \"Project\", \"OtherCiv\", \"Transcendent\"]}]\n")]
M["b6b2c_schema_pool_member_any"] = [("Tools/cosmic_data_schema.py",
    "            if sub not in members:\n",
    "            if False:\n")]
M["b6b2c_schema_pool_member_as_struct"] = [("Tools/cosmic_data_schema.py",
    "        if mark and isinstance(row.get(head), dict):\n",
    "        if mark and isinstance(row.get(head), dict) and False:\n")]


def read(path):
    with open(path, "rb") as f:
        return f.read().decode("utf-8").replace("\r\n", "\n")


def write(name, hunks):
    by_file = {}
    for path, before, after in hunks:
        by_file.setdefault(path, []).append((before, after))
    body = []
    for path, changes in by_file.items():
        text = read(path)
        mutated = text
        for before, after in changes:
            if mutated.count(before) != 1:
                raise SystemExit("%s: before text found %d times in %s: %r" % (name, mutated.count(before), path, before[:80]))
            mutated = mutated.replace(before, after)
        diff = list(difflib.unified_diff(text.splitlines(keepends=True), mutated.splitlines(keepends=True), "a/" + path, "b/" + path, n=3))
        body += ["diff --git a/%s b/%s\n" % (path, path)] + [l.rstrip(" \t\n") + "\n" if l.startswith(("---", "+++")) else l for l in diff]
    out = os.path.join(OUT, name + ".patch")
    with open(out, "wb") as f:
        f.write("".join(body).encode("utf-8"))
    print(out, len([l for l in body if l.startswith("@@")]), "hunk(s)")


for name in (sys.argv[1:] or M):
    write(name, M[name])
