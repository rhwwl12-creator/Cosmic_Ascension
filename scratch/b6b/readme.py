p = 'Tools/mutations/README.md'
s = open(p, 'rb').read().decode('utf-8')
assert '\r' not in s
old = "A patch of `Tools/data` (the `b4d2_data_` ones, `b6b2a_data_laws_knob_four` and `b6b2a_data_reach_word_renamed`) needs the assets rebuilt by `Tools/make_data_tables.py` instead of a build,"
assert s.count(old) == 1
s = s.replace(old, "A patch of `Tools/data` (the `b4d2_data_` ones, `b6b2a_data_laws_knob_four`, `b6b2a_data_reach_word_renamed`, `b6b2c_data_ark_margin_half` and `b6b2c_data_binding_answers`) needs the assets rebuilt by `Tools/make_data_tables.py` instead of a build,")
old = "The `b6b2b_schema_` and `b6b2b_data_` ones are run by the Python tools alone"
assert s.count(old) == 1
s = s.replace(old, "The `b6b2b_schema_` and `b6b2b_data_` ones, the `b6b2c_schema_` ones, `b6b2c_data_binding_causality` and `b6b2c_data_bound_line_names_people` are run by the Python tools alone")
rows = [
    ("b6b2c_delay_second_rung", "the delay above the third rung also slows an AI people's construction of the second rung", "`Cosmic_Ascension.Sim.UpperRungDelay` (the second rung's row)"),
    ("b6b2c_delay_on_response", "the delay above the third rung slows the response slot too (an ark of the third rung)", "`Cosmic_Ascension.Sim.ArkAnswers` (the ark's progress, then its completion)"),
    ("b6b2c_delay_bad_knob_as_is", "`AIUpperRungProgressMultiplier` not above 0, or not a number, is read as it is (its error line kept)", "`Cosmic_Ascension.Sim.UpperRungDelay` (the knobs 0 and NaN)"),
    ("b6b2c_brake_skips_ruled", "`CosmicAscent::FourthRungOpenOf` leaves the ruled people out of the count", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (1): the next people is not held)"),
    ("b6b2c_brake_order_reversed", "the judgment walks the peoples the player does not rule from the end of `Civs`", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (1): the people first in Civs climbs)"),
    ("b6b2c_brake_cap_zero_brakes", "a cap of 0 is a brake (nothing climbs to the fourth rung)", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (1)'s free board and case (3))"),
    ("b6b2c_brake_missing_row_closes", "a galaxy row the tables do not hold closes the fourth rung", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (3), no galaxy table)"),
    ("b6b2c_brake_counts_third_rung", "the count takes the third rung too (the asking people counts itself)", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (4): the held people climbs on the tick after the count drops; case (1) too)"),
    ("b6b2c_brake_counts_landed_transits", "the count takes every crossing bound for the galaxy, a landed one too", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (5): landed, no longer counted)"),
    ("b6b2c_brake_counted_once_per_tick", "the count is read once a tick (a climb earlier in the tick is not counted)", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (1): the next is held, closed at the cap)"),
    ("b6b2c_brake_ruled_too", "the brake holds the people the player rules too", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (2): the ruled people)"),
    ("b6b2c_brake_fifth_too", "the brake is asked from the third rung up (the climb to the fifth is held)", "`Cosmic_Ascension.Sim.FourthRungBrake` (case (2): the fifth rung)"),
    ("b6b2c_rebind_moves_sealed_tick", "`CosmicSeal::BindTargetOf` takes a sealed fallen one and `Bind` writes over its seal (a second binding moves `SealedTick`)", "`Cosmic_Ascension.Sim.Binding` (cases (1), (3) - the seal never moved - and (4))"),
    ("b6b2c_bind_target_any_realm", "`BindTargetOf` takes an instance of any realm (the independent one first)", "`Cosmic_Ascension.Sim.Binding` (case (1))"),
    ("b6b2c_binding_does_not_bind", "the binding's completion seals nothing (its record and lines stay)", "`Cosmic_Ascension.Sim.Binding` (case (2): the seal, its tick, the claim after it)"),
    ("b6b2c_binding_draws", "the binding's completion draws one number of the sim stream", "`Cosmic_Ascension.Sim.Binding` (case (2): the draws of the twin tick)"),
    ("b6b2c_target_once_per_pass", "`Advance` reads the binding's target once a tick (the second binding of a pass binds a target already bound)", "`Cosmic_Ascension.Sim.Binding` (case (4): the second ends without result)"),
    ("b6b2c_bound_line_first", "the binding's line is told before the completion line", "`Cosmic_Ascension.Sim.Binding` (case (2): right after it)"),
    ("b6b2c_binding_read_as_sealed", "the evaluator reads `MegaprojectComplete project_binding` as `TranscendentSealed` (any sealed fallen one since `SinceTick`)", "`Cosmic_Ascension.Feats.Evaluate` (the binding's record: another people does not climb)"),
    ("b6b2c_nofallen_before_rowtier", "`StartBlockOf` asks `NoFallen` before `RowTier`", "`Cosmic_Ascension.Sim.StartBlockReasons` (below the row's rung it is RowTier)"),
    ("b6b2c_nofallen_after_slot", "`StartBlockOf` asks `NoFallen` after the slots (a running binding with no target reads Running)", "`Cosmic_Ascension.Sim.StartBlockReasons` (the fallen one before the slot)"),
    ("b6b2c_build_button_any_rung", "the panel's construction buttons show at any rung again (a first-rung home greys the third rung's row)", "`Cosmic_Ascension.UI.StartReasons` (the first-rung home; `UI.TopBarAndPanel` too)"),
    ("b6b2c_guard_cap_ignored", "`CosmicConflict::FleetDelayOf` leaves out the row's cap `GuardDelayCapOf`", "`Cosmic_Ascension.Sim.FleetDelay` (the share 0.25: 50, not 100)"),
    ("b6b2c_data_ark_margin_half", "`DT_Rules.AIAscensionSpeedMultiplier` 0.5 (a data patch: rebuild the assets) - the design's ark of 150 takes an AI people 300 ticks of the extinction's 250", "`Cosmic_Ascension.Data.Integrity` (the ark's margin under the game's rules)"),
    ("b6b2c_data_binding_answers", "`crisis_plague` lists `project_binding` as an answer (a data patch: rebuild the assets)", "`Cosmic_Ascension.Data.Integrity` (no hazard answers with the binding)"),
    ("b6b2c_data_binding_causality", "`project_binding` asks Causality 1 a tick (a data patch, refused before an asset is written)", "`python -I Tools/check_data.py` (rule 7: `CostPerTick.Causality` is 0)"),
    ("b6b2c_data_bound_line_names_people", "the first variant of `log_fallen_bound` names `{Civ}` (a data patch, refused before an asset is written)", "`python -I Tools/check_data.py` (rule 8: `{System}` alone)"),
    ("b6b2c_schema_causality_rule_dropped", "rule 7 of `schema.json` asks Causality `ge` 0 instead of `is` 0 (a schema patch, no build)", "`test_the_binding_rows` (`Tools/test_data_schema.py` - the breaking case Causality 1)"),
    ("b6b2c_schema_bound_line_rule_dropped", "the rule of `log_fallen_bound` takes `{Civ}`, `{Project}`, `{OtherCiv}` and `{Transcendent}` (a schema patch)", "`test_the_binding_rows` (`Tools/test_data_schema.py` - each slot)"),
    ("b6b2c_schema_pool_member_any", "the checker takes any member name of a pool (`CostPerTick.Faith`)", "`test_the_binding_rows` (`Tools/test_data_schema.py`)"),
    ("b6b2c_schema_pool_member_as_struct", "the checker reads a pool member path as a list of structs (every rule over a pool member fails every row)", "`test_the_binding_rows` and every test that checks the project's rows (`Tools/test_data_schema.py`)"),
]
add = ''.join('| `B6b/` | `%s.patch` — %s | %s |\n' % r for r in rows)
assert s.endswith('\n')
s += add
open(p, 'wb').write(s.encode('utf-8'))
print(len(rows))
