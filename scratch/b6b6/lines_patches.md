## B. Patches (brief 2.1, 2.5 - apply state from apply_before.txt, 1,033 patches: 892 apply, 141 fail on [H''])

- New-patch targets on [H'']: `b6b6_kills_ruler_not_extinction` -> `Source/Cosmic_Ascension/Sim/CosmicHazards.cpp:751`;
  `b6b6_data_extinction_expiry_effect` -> rule 5 at `schema.json:805-815` (keep the `rows_of` test at its end) and the new
  row's `OnExpireEffects` in DT_Hazards.json (the row goes after :261); `b6b6_brake_always_shut` -> `CosmicAscent::FourthRungOpenOf`
  in `Sim/CosmicAscent.cpp`; `b6b6_data_ark_word_in_card` -> an encounter `Body` (forbidden is the string "verdict", `schema.json:988`).
- README rows the brief names hold their lines: :5-7 (the list of asset-rebuilding data patches - `b6b6_data_extinction_expiry_effect`
  joins it), :746 `b6b2b_kills_ruler_other_tag` (Breaks `Sim.ExtinctionExpiry`; add `Data.Integrity`), :792
  `b6b2c_data_ark_margin_half` (target `Tools/data/DT_Rules.json`), :868-873 the six `b6b3b_without_b6b_*`.
- Apply state on [H''] of the patches the brief names: OK - `b6b2b_kills_ruler_other_tag`, `b6b2c_data_ark_margin_half`, the six
  `b6b3b_without_b6b_*`, the three `b6b3b_fourth_cap_*`, `b2c_helper_keeps_edict`, `b6a6_exalt_as_before_is_the_game`,
  `b5d2_without_death_keeps_expiry`, `b4d1_schema_myth_takes_baro`, `b4d1_schema_verdict_without_soul_has_soul`;
  FAIL (already dead, not this commit's) - `b4b_theodicy_one_bonus`, `b4b_theodicy_no_card_delay`.
- Likely retakes (applying patches whose context or '-' lines are lines this commit edits - apply_after decides):
  `B4/b4d1_schema_myth_takes_baro`, `B4/b4d1_schema_verdict_without_soul_has_soul` (holy_and_myth :45 - '노아');
  `B6b/b6b5_schema_event_column_without_titles`, `_event_lines_holy_off`, `_event_lines_soul_off` (DT_Events.LogTemplates :472);
  `B6b/b6b5_schema_hazard_columns_without_grasp`, `_hazard_expiry_rescue_off` (DT_Hazards template columns :738-740);
  `B6b/b6b5_schema_log_column_without_grasp` (DT_LogTemplates.Templates :345); `B6b/b6b5_data_feat_says_bound` (context at
  the end of DT_Feats.json). The b6b5_ ones are the previous commit's, same block (B6b): retake under the same name per
  docs/40:40 ([B-6b5] ㉟), not under a b6b6_ name - the brief's "b6b6_ name" applies to B4's two. No applying patch has
  `schema.json:724` (DT_Hazards.DisplayName) as context.
- Rerun-set counts per target file on [H''] (the brief's figures are cef8cc7's): schema.json **68** patches (56 apply; the
  brief's 35 + [B-6b5]'s 33), CosmicTestHelpers.h 17 (17), CosmicDataTests.cpp 15 (13), DT_Hazards.json 2 (2),
  DT_Megaprojects.json 1 (1), DT_Feats.json 2 (2). `Calling.DeathPipeline` filter: 12 patches (8 apply). The full mechanical
  set is in section D.

