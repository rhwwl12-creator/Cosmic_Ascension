# Mutation patches

Each patch breaks one new assertion once, as `docs/40_DEV_GUIDE.md` asks of every new test: apply it (`git apply <patch>`),
build, run the named test and see it fail, then revert (`git apply -R <patch>`). The patch name says which test it breaks.
A patch of `Tools/data` (the `b4d2_data_` ones, `b6b2a_data_laws_knob_four`, `b6b2a_data_reach_word_renamed`, `b6b2c_data_ark_margin_half`, `b6b2c_data_binding_answers`, `b6b3a_data_far_line_shares_text`, `b6b4_data_bound_words_seal`, `b6b6_data_extinction_expiry_effect`, `b6b7_data_laws_two_rows`, `b6b7_data_feat_without_binding`, `b6b7_data_upper_rung_not_binary`, `b6b7_data_law_answers`, `b6b7_data_ark_margin_half`, `b6b8_data_era_borders_0_3`, `b6b8_data_witness_reverence_0_3`, `b6b8_data_extinction_expiry_effect`, `b6b8_data_era_no_ruled_past_checker`, `b6b8b_data_share_off`, `b6b8b_data_share_third`, `b6b10_data_clock_six_thousand` and `b6b12_data_resist_factor`) needs the assets rebuilt by `Tools/make_data_tables.py` instead of a build,
after the apply and again after the revert. The `b6b2b_schema_` and `b6b2b_data_` ones, the `b6b2c_schema_` ones, `b6b2c_data_binding_causality`, `b6b2c_data_bound_line_names_people`, `b6b3a_data_far_line_names_people`, `b6b3a_schema_far_rule_takes_other`, the `b6b5_schema_` and `b6b5_data_` ones and the `b6b6_schema_` ones, `b6b6_data_extinction_says_ark`, `b6b6_data_ark_rescues`, `b6b6_data_ark_word_in_card`, the `b6b8_schema_` ones, `b6b8_data_witness_says_jeungeon`, `b6b8_data_era_no_ruled` and the `b6b12_schema_` ones are run by the Python tools alone (`python -I Tools/test_data_schema.py`,
`python -I Tools/check_data.py`): the checker refuses them before an asset is written, so no asset is rebuilt (`b6b6_data_ark_word_in_card` passes the checker and fails `Tools/test_data_schema.py` alone - the runner then writes the assets, records the failure and exits 1; rebuild them after the revert).
The `b6b7_probe_` ones are commandlet probes in the form of `b6a3a_probe_no_awareness_flag` (B6a), not mutations that break a test: applied, built, run on the seed and tick their row names, then reverted and both targets rebuilt clean.
Kept from milestone B on (B1); the earlier milestones ran their mutations from scripts and recorded them in the commit
messages and in `docs/60_DECISIONS/DECISIONS_LOG.md`.

| Folder | Patch | Breaks |
|---|---|---|
| `B1/` | `b1a_walk_from_one_place.patch` — the look starts from the first place of the eye only | `Cosmic_Ascension.Sim.SightWalkMoved` (both ruler cases) |
| `B2/` | `b2a_enact_free.patch` — an enactment pays nothing | `Cosmic_Ascension.Sim.EdictCommand` |
| `B2/` | `b2a_revoke_swap.patch` — a revocation swaps the last edict into the gap | `Cosmic_Ascension.Sim.EdictCommand` |
| `B2/` | `b2a_enact_in_handler.patch` — EnactEdict runs inside a handler | `Cosmic_Ascension.Sim.EdictCommand` |
| `B2/` | `b2a_enact_no_line.patch` — an enactment tells no line | `Cosmic_Ascension.Sim.EdictCommand` |
| `B2/` | `b2a_no_tier_check.patch` — the gate skips the rung | `Cosmic_Ascension.Sim.EdictCommand` |
| `B2/` | `b2a_no_condition_check.patch` — the gate skips the conditions | `Cosmic_Ascension.Sim.EdictCommand` |
| `B2/` | `b2a_no_slot_check.patch` — the gate skips the slots | `Cosmic_Ascension.Sim.Edict` |
| `B2/` | `b2a_no_cost_check.patch` — the gate skips the cost | `Cosmic_Ascension.Sim.EdictCommand` |
| `B2/` | `b2a_effects_before_income.patch` — the edicts' effects come before the income is set | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_upkeep_never_lapses.patch` — an unpaid upkeep never lapses | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_upkeep_from_old_stock.patch` — the upkeep is paid from the stockpile before this tick's income | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_otherid_kept.patch` — an edict's effect keeps the OtherID of the people before | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_lapse_tells_ai.patch` — an AI people's lapse is told | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_no_spawn_factor.patch` — the spawn share leaves out the edicts' factor | `Cosmic_Ascension.Sim.EdictHazards` |
| `B2/` | `b2a_no_countdown_bonus.patch` — a placed hazard takes no countdown bonus | `Cosmic_Ascension.Sim.EdictHazards` |
| `B2/` | `b2a_expiry_ignores_answer.patch` — the expiry never looks for a held answer | `Cosmic_Ascension.Sim.EdictHazards` |
| `B2/` | `b2a_answer_empties_slot_only.patch` — a held answer only empties the slot | `Cosmic_Ascension.Sim.EdictHazards` |
| `B2/` | `b2a_ai_revoke_at_threshold.patch` — the AI revokes at the threshold too | `Cosmic_Ascension.Sim.EdictAI` |
| `B2/` | `b2a_ai_keeps_other_edicts.patch` — the AI keeps the edicts without a Stability cost | `Cosmic_Ascension.Sim.EdictAI` |
| `B2/` | `b2a_slots_strict.patch` — a slot opens only above its mark | `Cosmic_Ascension.Sim.EdictSlots` |
| `B2/` | `b2a_heldcount_counts_gone.patch` — a name the table does not hold takes a slot | `Cosmic_Ascension.Sim.EdictSlots` |
| `B2/` | `b2a_starting_first_only.patch` — only the first people inherits the edict | `Cosmic_Ascension.Sim.StartingEdict` |
| `B2/` | `b2a_society_ignored.patch` — the recovery bonus ignores Society | `Cosmic_Ascension.Sim.SocietyRecovery` |
| `B2/` | `b2a_society_infinite.patch` — an infinite recovery bonus is taken | `Cosmic_Ascension.Sim.SocietyRecovery` |
| `B2/` | `b2a_invariant_ignores_bonus.patch` — the invariant leaves out the recovery bonus | `Cosmic_Ascension.Sim.SocietyRecovery` |
| `B2/` | `b2a_shares_by_max.patch` — the shares divide by the largest weight | `Cosmic_Ascension.Sim.ResearchShares` |
| `B2/` | `b2a_shares_take_transcendence.patch` — the shares take a transcendence weight | `Cosmic_Ascension.Sim.ResearchShares` |
| `B2/` | `b2a_chronicle_takes_edicts.patch` — the chronicle takes the edict lines | `Cosmic_Ascension.Log.ChronicleEdicts` |
| `B2/` | `b2a_integrity_duration.patch` — the integrity check lets a short duration through | `Cosmic_Ascension.Data.Integrity` |
| `B2/` | `b2a_resolution_runs_claim_rule.patch` — a held answer runs the claim rule after the resolution | `Cosmic_Ascension.Sim.EdictHazards` |
| `B2/` | `b2a_resolution_no_line.patch` — a held answer tells no resolution line | `Cosmic_Ascension.Sim.EdictHazards` |
| `B2/` | `b2a_upkeep_partial_pay.patch` — an unpaid upkeep pays part of it | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_upkeep_reverse_order.patch` — the upkeep walks the edicts backwards | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_effects_reverse_order.patch` — the effects walk the edicts backwards | `Cosmic_Ascension.Sim.EdictEconomy` |
| `B2/` | `b2a_invariant_negative_only.patch` — the invariant counts only the Stability costs | `Cosmic_Ascension.Data.Integrity` |
| `B2/` | `b2b_reason_other_gate.patch` — the window greys a row by slots alone, not by the command's gate | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_command_every_drag.patch` — every slider move commands the shares | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_shape_greys.patch` — a focus button greys by the map's shape, not by its preset | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_line_any_people.patch` — the panel's edict line stands on another people's system | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_open_keeps_hint.patch` — opening the window does not end the edict hint | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_mark_without_holding.patch` — the inherited mark stands on a row not held | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_slider_overwritten.patch` — a refresh sets a slider a person moved | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_apply_without_move.patch` — [Share it so] is enabled before a person's move | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_all_rows.patch` — rows above the people's rung stand in the window | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_hint_never_ends.patch` — the hint does not end when the edict is held no more | `Cosmic_Ascension.UI.EdictRules` |
| `B2/` | `b2b_hint_at_threshold.patch` — the hint starts at Stability 0.5 itself | `Cosmic_Ascension.UI.EdictRules` |
| `B2/` | `b2b_focusword_shape.patch` — the focus word follows the map's shape | `Cosmic_Ascension.UI.EdictRules` |
| `B2/` | `b2b_next_slot_strict.patch` — a mark reached exactly is not reached | `Cosmic_Ascension.UI.EdictRules` |
| `B2/` | `b2b_society_tip_missing.patch` — [Society]'s tooltip says nothing of what it opens | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_bar_for_wanderer.patch` — a wanderer's bar shows the edict button | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_window_stays_for_wanderer.patch` — the window stays open for a wanderer | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_hint_line_any_panel.patch` — the hint line stands on any panel | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_tip_no_answers.patch` — an edict's tooltip leaves out the hazards it answers | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_slots_reason_one_form.patch` — the slots' reason sends the player to Society research when every slot is open | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_reset_after_apply.patch` — the sliders jump after [Share it so] | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_open_resets_moves.patch` — [Share by hand] on an open window drops a person's moves | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_hint_kept_on_new_galaxy.patch` — an unanswered hint carries into the next run | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_mark_collapsed.patch` — the bar's mark collapses and moves the research block | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_can_after_end.patch` — the 'can answer' line stands after the end | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_zero_shows_current.patch` — three sliders at 0 promise the current shares | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_would_before_move.patch` — the 'would' line shows before a move | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_bar_after_end.patch` — the edict button stays after the end | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_direct_after_end.patch` — [Share by hand] stays enabled after the end | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_window_after_end.patch` — the window stays open after the end | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_shares_after_end.patch` — the controller takes shares after the end | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_recovery_line_always.patch` — [Society] promises a faster recovery with no bonus | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_toggle_always_opens.patch` — a second press of the bar's button does not close the window | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_new_galaxy_keeps_window.patch` — the window stays open over a new galaxy | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_open_ends_started_only.patch` — an open before the fall does not stop the hint | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2b_row_unbound.patch` — a row's button commands nothing | `Cosmic_Ascension.UI.EdictWindow` |
| `B2/` | `b2c_vigil_with_rebellion.patch` — the policy enacts the vigil with a rebellion already standing | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_kept_without_rest.patch` — the kept Stability edict comes back below the resting level | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_kept_revoked_as_idle.patch` — the kept row is revoked as an idle answer | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_no_room_made.patch` — no slot is made for a standing hazard's answer (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_no_room_made`) | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_vigil_kept.patch` — the policy's vigil stays after the danger passed | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_idle_answer_kept.patch` — an answer with nothing to answer stays | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_decisions_without_edicts.patch` — the decisions leave out the edict commands | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_edicts_after_end.patch` — the policy changes edicts after the provisional end | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_helper_keeps_edict.patch` — the test helper does not revoke the inherited edict | `Cosmic_Ascension.Sim.PlayerAscendsToT3` |
| `B2/` | `b2c_quiet_keeps_edict.patch` — the quiet board keeps the inherited edict | `Cosmic_Ascension.Sim.StabilityRecovery` |
| `B2/` | `b2c_sightwalk_new_world.patch` — the sight-walk proof runs on the B2 world (no longer applies since [B-3d1] - taken again at [B-6b12] as `b6b12_sightwalk_new_world`) | `Cosmic_Ascension.Sim.SightWalkMoved` |
| `B2/` | `b2c_room_takes_kept_row.patch` — room is made by giving up the kept row first (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_room_takes_kept_row`) | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_room_never_kept.patch` — the kept row is never given up for an answer (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_room_never_kept`) | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_room_ignores_gate.patch` — an edict is given up for an answer that would be refused | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_hashrun_no_edict.patch` — the hash run's policy holds no edicts | `Cosmic_Ascension.Sim.HashRun` |
| `B2/` | `b2c_ship_hint_under_rebellion.patch` — the first ship's line stands beside a rebellion | `Cosmic_Ascension.UI.FirstHints` |
| `B2/` | `b2c_panel_ignores_rebellion.patch` — the panel does not tell the ship's line of the rebellion | `Cosmic_Ascension.UI.FirstHints` |
| `B2/` | `b2c_bar_never_opens.patch` — the bar's edict button opens nothing | `Cosmic_Ascension.UI.MouseOnlyAscent` |
| `B2/` | `b2c_inert_keeps_edict.patch` — the knowing-inert ruler keeps the inherited edict and falls | `Cosmic_Ascension.Sim.KnowingInert` |
| `B3/` | `b3a_recompletion_keeps_completer.patch` — a re-completion keeps the old completer | `Cosmic_Ascension.Sim.MegaprojectProgress` |
| `B3/` | `b3a_new_record_no_completer.patch` — a new completion record names no completer | `Cosmic_Ascension.Sim.MegaprojectProgress` |
| `B3/` | `b3a_fleet_tick_not_saved.patch` — the fleet target tick is not a saved property | `Cosmic_Ascension.Sim.SaveRoundTrip` |
| `B3/` | `b3a_tags_hashed_in_stored_order.patch` — a tag container is hashed in its stored order | `Cosmic_Ascension.Sim.Determinism` |
| `B3/` | `b3b_sign_one_side.patch` — a treaty is signed on one side only | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_neutral_keeps_entry.patch` — turning neutral leaves an empty pair record | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_stance_same_before_met.patch` — the same stance is asked before the meeting | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_view_stored_everywhere.patch` — a pair that does not border is seen through its stored relation | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_hostility_keeps_trade.patch` — turning hostile keeps the trade treaty | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_siege_gate_order.patch` — the realm cap is asked before the lane | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_tie_to_attacker.patch` — a tie of the siege number goes to the attacker | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_defence_share_ignored.patch` — the defender always defends with half | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_cost_before_refused.patch` — the cost of a treaty is asked before its refusal | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_stance_treaty_before_tier.patch` — a non-aggression treaty is asked before the rung | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_fleet_before_cap.patch` — the fleet is asked before the realm cap | `Cosmic_Ascension.Sim.DiplomacyRules` |
| `B3/` | `b3b_drift_unbordered.patch` — every pair drifts, bordering or not | `Cosmic_Ascension.Sim.DiplomacyStage` |
| `B3/` | `b3b_drift_writes_at_zero.patch` — a rate of 0 still writes relation entries | `Cosmic_Ascension.Sim.DiplomacyStage` |
| `B3/` | `b3b_upkeep_never_lapses.patch` — a friendly stance never lapses for want of Influence | `Cosmic_Ascension.Sim.DiplomacyStage` |
| `B3/` | `b3b_cleanup_keeps_ties.patch` — the crossing cleanup keeps the ties of a people gone | `Cosmic_Ascension.Sim.DiplomacyStage` |
| `B3/` | `b3b_no_trade_income.patch` — a trade treaty brings no Influence | `Cosmic_Ascension.Sim.DiplomacyStage` |
| `B3/` | `b3b_drift_draws.patch` — the drift draws one number a bordering pair | `Cosmic_Ascension.Sim.DiplomacyStage` |
| `B3/` | `b3b_commands_ignore_met.patch` — the stance command does not ask whether the people is met | `Cosmic_Ascension.Sim.DiplomacyCommands` |
| `B3/` | `b3b_accept_without_bonus.patch` — the acceptance leaves out the edicts' bonus | `Cosmic_Ascension.Sim.DiplomacyCommands` |
| `B3/` | `b3b_break_no_penalty.patch` — breaking a treaty costs no view | `Cosmic_Ascension.Sim.DiplomacyCommands` |
| `B3/` | `b3b_same_target_rewinds.patch` — the same target again winds the siege back | `Cosmic_Ascension.Sim.DiplomacyCommands` |
| `B3/` | `b3b_handler_break_runs.patch` — BreakTreaty goes on from a handler after its error line | `Cosmic_Ascension.Sim.DiplomacyCommands` |
| `B3/` | `b3b_cap_by_tier_only.patch` — the fleet cap ignores the settled systems | `Cosmic_Ascension.Sim.Fleet` |
| `B3/` | `b3b_attrition_counts_owner.patch` — the attrition counts the owner's rung (the full pressure) | `Cosmic_Ascension.Sim.Fleet` |
| `B3/` | `b3b_attrition_counts_wanderer.patch` — the attrition counts a wanderer standing there | `Cosmic_Ascension.Sim.Fleet` |
| `B3/` | `b3b_fleet_below_zero.patch` — the attrition takes the fleet below 0 | `Cosmic_Ascension.Sim.Fleet` |
| `B3/` | `b3b_unpaid_upkeep_paid.patch` — an upkeep that cannot be paid is paid anyway | `Cosmic_Ascension.Sim.Fleet` |
| `B3/` | `b3b_gate_lost_no_draw.patch` — a siege that lost its gate on its last tick draws nothing | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_one_draw_per_tick.patch` — every siege at its end shares one number | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_no_recheck.patch` — the gate is not asked again before an end | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_transfer_keeps_targets.patch` — a hand-over keeps the targets on the system | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_destroy_keeps_targets.patch` — a destruction keeps the targets on the system | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_due_exact.patch` — a siege ends only on the exact tick | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_held_keeps_fleet.patch` — a siege that fails costs the attacker nothing | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_one_row_both_sides.patch` — the defender is told the attacker's row | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_siege_line_unmet.patch` — the siege line is told before the other people is met | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_taking_unknown.patch` — a taking does not follow the knowing of the system received | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_conquest_keeps_vessel.patch` — the ruler's vessel stays on the system its people lost | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_siege_knob_no_floor.patch` — a SiegeTicks of 0 is not read as 1 | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_loser_not_counted.patch` — the loser cut off from the conqueror does not think less of it | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_guard_never_checked.patch` — a guard is never asked its gate | `Cosmic_Ascension.Sim.Siege` |
| `B3/` | `b3b_delay_on_rebellion.patch` — a rebellion in a guarded capital is put off | `Cosmic_Ascension.Sim.FleetDelay` |
| `B3/` | `b3b_delay_unguarded.patch` — an unguarded system gains the delay | `Cosmic_Ascension.Sim.FleetDelay` |
| `B3/` | `b3b_delay_ignores_gap.patch` — a claim's delay ignores the gap to its claimant (no longer applies since [B-3c] - taken again at [B-6b12] as `b6b12_delay_ignores_gap`) | `Cosmic_Ascension.Sim.FleetDelay` |
| `B3/` | `b3b_delay_outside_cut.patch` — the delay is added past the int32 cut (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_delay_outside_cut`) | `Cosmic_Ascension.Sim.FleetDelay` |
| `B3/` | `b3b_spread_live_table.patch` — the spread reads a plague placed in the same step (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_spread_live_table`) | `Cosmic_Ascension.Sim.PlagueSpread` |
| `B3/` | `b3b_spread_no_multiplier.patch` — the spread multiplies nothing (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_spread_no_multiplier`) | `Cosmic_Ascension.Sim.PlagueSpread` |
| `B3/` | `b3b_ai_pileon.patch` — an AI opens hostility on a people another AI is hostile to | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_ai_no_aim_needed.patch` — an AI turns hostile with no system to aim at | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_ai_keeps_hostility.patch` — an AI keeps a hostility with no siege | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_ai_no_rest.patch` — an AI does not rest after a siege | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_ai_keeps_guard.patch` — an AI keeps a guard | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_coronation_keeps_siege.patch` — the coronation keeps the new ruler's siege | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_ai_line_unmet.patch` — the AI's siege line is told before the besieger is met | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_ai_keeps_trade.patch` — the AI's opening keeps the trade treaty | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_ai_reads_knowing.patch` — the AI opens only toward a ruled people the calling has met | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3b_completion_any_people.patch` — a completion counts for any people | `Cosmic_Ascension.Sim.OwnCompletions` |
| `B3/` | `b3b_transfer_no_stamp.patch` — a hand-over does not write the old owner into a record of no people | `Cosmic_Ascension.Sim.OwnCompletions` |
| `B3/` | `b3b_chronicle_keeps_stance.patch` — the chronicle admits the stance lines (no longer applies since [B-6b3a] - taken again as `b6b3a_chronicle_keeps_stance`) | `Cosmic_Ascension.Log.ConflictLines` |
| `B3/` | `b3b_effect_skips_gate.patch` — the SetStance effect skips the stance gate | `Cosmic_Ascension.Events.RoleEffectRules` |
| `B3/` | `b3b_schema_no_card_only.patch` — the schema checker lets a card-only effect stand anywhere | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py`) |
| `B3/` | `b3b_schema_log_without_holy_words.patch` — the log lines no longer refuse the holy words (does not apply since [B-4d1] `605a050`, where `rescue_words` joined the column - stopped before B6b) | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py`) |
| `B3/` | `b3c_time_left_twice.patch` — the window's siege lines say the time left twice (TimeLeftLabel in a line that says it) | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_panel_time_left_twice.patch` — the panel's siege lines say the time left twice | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_row_flip_missing.patch` — their siege on us is told with their band, not ours | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_hostility_in_head.patch` — their hostility stands on the name's line | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_held_unmarked.patch` — the stance held is not marked apart from a refused one | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_rows_any_met.patch` — a people met but not shown has a row | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_rows_any_shown.patch` — a people shown but not met has a row (seeing all) | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_peoples_tab_to_shares.patch` — the peoples' tab turns to the shares' page | `Cosmic_Ascension.UI.PeoplesTab`, `Cosmic_Ascension.UI.EdictWindow` |
| `B3/` | `b3c_minus_zero.patch` — a term of 0 reads '-0' | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_signed_no_plus.patch` — a term above 0 is told without its '+' | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_view_bar_toward_zero.patch` — the view beside the bar is cut toward 0 (a refused view reads at the bar) | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_view_below_reached_bar.patch` — a view that reaches a bar of no tenth reads below it | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_bar_without_bonus.patch` — the window tells the bar without the edicts' bonus | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_tip_their_stance.patch` — the stance term reads their stance toward us, not ours toward them | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_tip_target_swapped.patch` — the target told is ours of them, not theirs of us | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_view_words_swapped.patch` — two band words change places | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_band_even_as_behind.patch` — an even siege is told as behind | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_ticks_left_below_zero.patch` — a siege past its end has time left below 0 | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_nofleet_head_always_fleet.patch` — the head tells a fleet of 0 with a cap of 0 | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_slots_all_missing.patch` — the slots' tooltip says nothing once every slot is open | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_slots_next_strict.patch` — the next friendly slot is the next rung's, grown or not (no longer applies since [B-6b4] - taken again as `b6b4_slots_next_strict`) | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_no_betrayal.patch` — [Hostile] over a trade treaty says nothing of the betrayal | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_break_far_as_near.patch` — breaking with a far people tells the bordering fall | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_lifts_missing.patch` — [Neutral] and [Friendly] do not say our siege of them lifts | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_friendly_held_bare.patch` — the friendly stance held drops its upkeep from the tooltip | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_withdraw_tip_fixed.patch` — [Withdraw the fleet] says a guard ends while it besieges | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_edict_line_always.patch` — an edict of factor 1 tells the fleet growth | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_fleet_in_projects.patch` — [Guard here] joins the project buttons | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_send_any_owner.patch` — [Send the fleet] stands on a system whose owner is not shown | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_send_if_met.patch` — [Send the fleet] stands on the system of a people met but not shown | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_besieger_named_if_met.patch` — a besieger met but not shown is named | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_panel_siege_reads_ruler.patch` — the panel tells a siege on us with our own fleet's count | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_above_as_nofleet.patch` — an owner too high is told as no fleet | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_odds_on_greyed.patch` — a greyed [Send the fleet] tells our side's band | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_guard_retarget_silent.patch` — [Guard here] does not say the siege it would lift | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_guard_suffix_missing.patch` — a greyed [Guard here] has no short reason | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_delay_always.patch` — the delay's line stands for any fleet | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_guard_name_wrong.patch` — a guard's NoFleet is named NoSystem | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_fleet_after_end.patch` — [Guard here] keeps its own words after the end | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_commands_after_end.patch` — the controller takes a fleet command after the end | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3c_double_click_kept.patch` — a double-click's second press presses the button | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_attack_offers_treaties.patch` — the attack policy offers treaties (a non-aggression closes its own siege gate, a trade makes the siege a betrayal) | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy`, `Cosmic_Ascension.Sim.HashRun` |
| `B3/` | `b3d_attack_drops_guard.patch` — the attack policy opens a hostility while a siege on its system stands | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_guard_after_friendship.patch` — a friendship goes before the guard at one boundary | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_gone_needs_id.patch` — a first contact card whose slot the cleanup cleared names no one | `Cosmic_Ascension.Sim.FirstContactCard`, `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_card_gone_greyed.patch` — a card whose people is gone greys [Reach out] (any answer withdraws it) | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_card_gone_unnamed.patch` — the card on screen names a people gone by empty words | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_schema_chain_into_contact.patch` — a chain may lead to the first contact card | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py`) |
| `B3/` | `b3d_notemet_any_role.patch` — the first meeting writes the first contact slot for a wanderer too | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_notemet_without_row.patch` — the slot is written although the table lacks the card's row | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_install_keeps_contact.patch` — a new life keeps the first contact card waiting | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_clear_clears_contact.patch` — answering another petition clears the waiting first contact slot | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_answer_keeps_contact.patch` — the first contact card's answer leaves its slot | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_present_before_standing.patch` — the first contact card goes before a Standing card that came to hold in the tick | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_present_ignores_row.patch` — the first contact card is put although its row does not hold | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_present_gone_people.patch` — the first contact card is put for a people gone | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_stands_gone_people.patch` — a card about a people gone still stands | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_contact_other_empty.patch` — the answer gives the effects no other people (OtherID empty) | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_answer_draws_refused.patch` — a choice the stance gate refuses is answered (a draw, nothing changed) | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_contact_effect_skips_gate.patch` — the SetStance effect skips the stance gate | `Cosmic_Ascension.Sim.FirstContactCard`, `Cosmic_Ascension.Events.RoleEffectRules` |
| `B3/` | `b3d_other_gone_empty.patch` — a people gone is named by empty words | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_contact_friendly_line_missing.patch` — the card's friendly choice does not say what the friendly stance holds | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_card_upkeep_literal.patch` — the card's friendly line tells 0.2 whatever the rule | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_card_upkeep_negative.patch` — the card's friendly line tells an upkeep below 0 | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_card_not_greyed.patch` — a choice the stance gate refuses is not greyed | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_screen_passes_contact_always.patch` — the screen names no people on the first contact card | `Cosmic_Ascension.UI.PeoplesTab` |
| `B3/` | `b3d_policy_contact_round.patch` — the policy answers the first contact card in its round | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_policy_contact_ignores_gate.patch` — the policy takes a choice the stance gate refuses | `Cosmic_Ascension.Sim.FirstContactCard` |
| `B3/` | `b3d_policy_two_a_boundary.patch` — a friendship does not end the boundary (two commands in one) | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_policy_turns_hostile.patch` — the peace policy turns hostile and besieges | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_policy_befriends_card_people.patch` — the policy befriends the people whose first contact card waits | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_policy_friend_unpaid.patch` — the policy befriends without the Influence income to keep it | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_policy_keeps_idle_hostility.patch` — the attack policy keeps a hostility with no siege behind it | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_guard_while_besieging.patch` — the policy guards while its fleet besieges | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d_hashrun_policy_without_diplomacy.patch` — the hash run plays the policy without diplomacy | `Cosmic_Ascension.Sim.HashRun` |
| `B3/` | `b3d_hashrun_attack_policy.patch` — the hash run plays the attack policy | `Cosmic_Ascension.Sim.HashRun` |
| `B3/` | `b3d_schema_row_not_always.patch` — the row test row_not holds for the named row too | `test_row_prefix` (`Tools/test_data_schema.py`) |
| `B3/` | `b3d_schema_row_not_unknown_row.patch` — a row_not naming a row the table lacks is not reported | `test_row_prefix` (`Tools/test_data_schema.py`) |
| `B3/` | `b3d_schema_stance_enc_only.patch` — only the wanderer's cards are kept from SetStance and {OtherCiv} | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py`) |
| `B3/` | `b3d2_ruled_relation_general.patch` — the AI gate toward the ruled people reads the general relation | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3d2_ruled_ratio_general.patch` — the AI gate toward the ruled people reads the general power ratio | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3d2_ruled_gate_for_all.patch` — the toward-the-ruled relation gates every people | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3d2_no_counter_siege.patch` — a besieged AI never opens on its besieger | `Cosmic_Ascension.Sim.DiplomacyAI` |
| `B3/` | `b3d2_attack_fills_last_slot.patch` — the attack policy settles into its last realm slot | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B3/` | `b3d2_schema_ruled_ratio_unbounded.patch` — the power ratio toward the ruled people may be below 0 | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py`) |
| `B4/` | `b4m0_drain_reads_attrition.patch` — the public drain reading reads the attrition's terms (inside a higher sphere only) | `Cosmic_Ascension.Sim.PressureDrainEdict` |
| `B4/` | `b4m0_drain_sums_systems.patch` — the public drain reading sums the systems' P instead of taking the largest | `Cosmic_Ascension.Sim.PressureDrainEdict` |
| `B4/` | `b4m0_drain_counts_settlements.patch` — the public drain reading counts a settlement under way | `Cosmic_Ascension.Sim.PressureDrainEdict` |
| `B4/` | `b4m0_drain_any_dominant_tier.patch` — the public drain reading takes any dominant source, not one above the people | `Cosmic_Ascension.Sim.PressureDrainEdict` |
| `B4/` | `b4m0_drain_both_sum.patch` — the economy stage and the public reading both sum the systems' P | `Cosmic_Ascension.Sim.PressureDrainEdict` (the boards' expected values) |
| `B4/` | `b4b_enact_skips_gate.patch` — EnactEdict skips EnactBlockOf | `Cosmic_Ascension.Sim.EnactEdictEffect` |
| `B4/` | `b4b_enact_pays_twice.patch` — EnactEdict pays the EnactCost twice | `Cosmic_Ascension.Sim.EnactEdictEffect` |
| `B4/` | `b4b_enact_per_tick.patch` — EnactEdict joins the per-tick effects | `Cosmic_Ascension.Sim.EnactEdictEffect` |
| `B4/` | `b4b_enact_accepts_norow.patch` — EnactEdict appends a Key the table does not hold | `Cosmic_Ascension.Sim.EnactEdictEffect` |
| `B4/` | `b4b_civhasedict_no_table.patch` — CivHasEdict does not ask the table (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_civhasedict_no_table`) | `Cosmic_Ascension.Sim.CivHasEdictCondition` |
| `B4/` | `b4b_gate_draw_first.patch` — the answer draws before the choice's gate | `Cosmic_Ascension.Sim.PetitionChoiceGate` |
| `B4/` | `b4b_policy_stance_only.patch` — the policy reads the stance gate alone, with the tables too | `Cosmic_Ascension.Sim.PetitionChoiceGate` |
| `B4/` | `b4b_gate_held_open.patch` — the choice's gate takes an edict held | `Cosmic_Ascension.Sim.PetitionChoiceGate` |
| `B4/` | `b4b_resist_no_fleet_check.patch` — ResistBlockOf does not ask the fleet | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4b_resist_by_gap.patch` — DelayHazard scales its ticks by the tier gap | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4b_resist_rebellion.patch` — ResistBlockOf takes a Capital row's hazard | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4b_resist_apply_own_gate.patch` — DelayHazard reads a gate of its own (the hazard only) | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4b_resist_no_cut.patch` — DelayHazard does not cut the countdown at MAX_int32 | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4b_resist_resolves.patch` — DelayHazard empties the hazard's slot | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4b_drain_factor_per_system.patch` — the edicts' drain factor multiplies once per system | `Cosmic_Ascension.Sim.PressureDrainEdict` |
| `B4/` | `b4b_drain_factor_unguarded.patch` — the edicts' drain factor reads 0, below 0 and NaN as they are | `Cosmic_Ascension.Sim.PressureDrainEdict` |
| `B4/` | `b4b_sign_no_max.patch` — the sign line has no upper window | `Cosmic_Ascension.Log.SignHelper` |
| `B4/` | `b4b_sign_draws_empty.patch` — PushWithSign tells the sign with no variants of the line | `Cosmic_Ascension.Log.SignHelper` |
| `B4/` | `b4b_sign_after_line.patch` — PushWithSign tells the sign after the line | `Cosmic_Ascension.Log.SignHelper` |
| `B4/` | `b4b_sign_silent_missing.patch` — a sign row the table lacks passes silently | `Cosmic_Ascension.Log.SignHelper` |
| `B4/` | `b4b_theodicy_one_bonus.patch` — the theodicy's longest stand counts one edict's countdown bonus (no longer applies since [B-4d2] - taken again at [B-6b12] as `b6b12_theodicy_one_bonus`) | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_theodicy_no_card_delay.patch` — the theodicy's longest stand leaves the cards' DelayHazard out (no longer applies since [B-4d2] - taken again at [B-6b12] as `b6b12_theodicy_no_card_delay`) | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_theodicy_factor_one.patch` — the theodicy counts the sphere factor as 1 | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_theodicy_lower_bar.patch` — the theodicy keeps a fifth instead of a quarter | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_card_edict_name_unchecked.patch` — a choice that enacts an edict need not carry its name | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_policy_revokes_card_edict.patch` — MakeRoomFor revokes a card's edict while a claim stands | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_vigil_revoked_under_claim.patch` — step (4) revokes the vigil while a claim stands under a pressure policy | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_guard_after_withdraw.patch` — -Guard=1 takes a standing guard back before it guards | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_guard_needs_diplomacy.patch` — -Guard=1 does not guard with -Diplomacy=0 | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_guard0_after_end.patch` — -Guard=0 runs step (1) after the end | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_builds_relief.patch` — a pressure policy builds the claim's relief | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_takes_tribute.patch` — -PlayerPolicy reads Tribute | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_leave_under_round.patch` — -LeaveNearSphere=1 leaves crises alone under Round too | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_faithful_discerns_first.patch` — Faithful discerns before the vigil | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_hashrun_guards.patch` — the hash run's policy guards | `Cosmic_Ascension.Sim.HashRun` |
| `B4/` | `b4b_theodicy_cap_only.patch` — the theodicy's sphere bound reads the cap alone, not the one core | `Cosmic_Ascension.Data.Integrity` (the core-only case's weight 1.85 since [B-6b7] - the plague's longest stand is 350 ticks with the galaxy warning) |
| `B4/` | `b4b_gate_effect_anywhere.patch` — a gated effect may stand behind another effect | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_delay_shape_lottery_ok.patch` — a Lottery card may buy a hazard time | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_delay_shape_cycle_ok.patch` — a card that buys a hazard time may chain to itself | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_delay_shape_roots_ok.patch` — two Standing cards may lead to a card that buys a hazard time | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_policy_room_with_own_vigil.patch` — MakeRoomFor revokes the script's own vigil while a claim stands under a pressure policy | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_round_revokes_card_vigil.patch` — step (4) revokes a card's vigil under Round while a claim stands | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_leave_ignores_switch.patch` — a pressure policy leaves the near-sphere crises alone without -LeaveNearSphere | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_act_drops_pressure.patch` — Act does not hand the pressure policy to ChoiceOf | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_policy_no_settle_fallback.patch` — a pressure policy starts no settlement when its list is empty | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_guard_no_inside.patch` — GuardTargetOf never keeps a system inside a sphere | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_guard_inside_first.patch` — GuardTargetOf takes inside before beside | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_guard_never_moves.patch` — a standing guard never moves to a new system to keep | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_guard_skips_siege.patch` — -Guard=1 passes a besieged system of the people by | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_leave_faithful_only.patch` — -LeaveNearSphere=1 acts under Faithful alone | `Cosmic_Ascension.Sim.RulerPressurePolicy` |
| `B4/` | `b4b_resist_per_tick.patch` — DelayHazard joins the per-tick effects | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4b_sign_window_off_by_one.patch` — the sign's window ends a rung too low | `Cosmic_Ascension.Log.SignHelper` |
| `B4/` | `b4b_drain_loss_no_factor.patch` — DrainLossOf leaves the held edicts' factor out | `Cosmic_Ascension.Sim.PressureDrainEdict` |
| `B4/` | `b4b_schema_not_tag_of_skipped.patch` — the checker skips the tag option not_tag_of | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_not_read_by_skipped.patch` — the checker skips the tag option not_read_by | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_none_always_true.patch` — the row test none always holds (no longer applies since [B-6b2c] - taken again at [B-6b12] as `b6b12_schema_none_always_true`) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_none_any_all.patch` — the row test none asks all the entries instead of any (no longer applies since [B-6b2c] - taken again at [B-6b12] as `b6b12_schema_none_any_all`) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_zero_inverted.patch` — the row test zero is inverted | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_contains_none_always.patch` — the row test contains_none always holds | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_struct_conditions_not_read.patch` — not_read_by does not look into the conditions of a list of structs | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_refusal_table_check_off.patch` — the schema check of not_tag_of takes a column that is no tag | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_not_read_by_tag_check_off.patch` — the schema check of not_read_by takes a condition that reads no tag | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_none_shape_check_off.patch` — the schema check of none takes tests over more than one list | `test_voice_and_drain` (`Tools/test_data_schema.py` - corrected at [B-6b6]: the row named `test_pressure_cards`, which this patch never broke) |
| `B4/` | `b4b_schema_contains_none_tag_decl_off.patch` — the tags of contains_none need not be declared (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_schema_contains_none_tag_decl_off`) | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_grace_rule_weak.patch` — the grace rule bans another condition than CivHasEdict | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_e3_list_short.patch` — the rolled-choice ban misses the Stability check | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_delay_max.patch` — DelayHazard may carry up to 1,000 ticks | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_delay_not_card_only.patch` — DelayHazard is not card_only | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_enact_not_card_only.patch` — EnactEdict is not card_only (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_schema_enact_not_card_only`) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_civhasedict_optional.patch` — CivHasEdict's Key is optional (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_schema_civhasedict_optional`) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_counterfeit_when_celestial.patch` — CounterfeitOf is required of the celestial rows instead | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_claim_pair_log_claim.patch` — Event.Crisis.Claim keeps Log.Claim among its line categories | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_drain_max_off.patch` — PressureDrainMultiplier has no upper bound (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_schema_drain_max_off`) | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_resist_min_off.patch` — ResistFleetFactor has no lower bound (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_schema_resist_min_off`) | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_e1_prefix_petition.patch` — the ruler-only rule bans the petition rows instead of the wanderer's | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_e5_when_free_only.patch` — a free choice with a stance counts as the card's unblocked choice | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_v2_ge_2.patch` — a Log.Transcendent row may be revealed from the second rung | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_capital_when_draw.patch` — the DelayHazard ban names the Draw rows instead of the Capital rows | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4c_card_no_system.patch` — the card asks the gates with no system (not the answer's subject) | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_card_slots_form_fixed.patch` — the card tells the slots taken in the form of slots research can grow, always | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_gate_effect_at_unset.patch` — ChoiceGateOf leaves EffectAt unset (the card cannot name the refused edict) | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_resist_reason_swapped.patch` — a fleet with no cap gets the words of a spent fleet | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_resist_notown_words.patch` — NotOwn, which no card shows, gets the words of NoHazard | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_resist_line_raw_factor.patch` — the fleet's line reads the knob raw, not as ResistShareOf reads it | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_resist_line_spent_share.patch` — the fleet's line tells the share spent, not the share kept | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_apply_spent_share.patch` — DelayHazard keeps the share spent | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_edict_line_slots_plus.patch` — the edict's line tells one slot more | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_edict_cost_none_dropped.patch` — a free edict's line leaves its cost blank | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_tip_gate_before_short.patch` — the gate's reason comes before the stockpile's | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_tip_no_edict_words.patch` — an edict's choice does not tell the edict's words | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_no_suffix.patch` — a greyed choice has no short words after its label | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_suffix_on_unpaid.patch` — a choice the stockpile cannot pay gets the gate's short words | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_no_fleet_suffix.patch` — a fleet with no cap has no short words after its label | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_resist_hazard_unnamed.patch` — the fleet's reasons do not name the refused hazard | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_panel_not_redrawn.patch` — an answer does not redraw the panel (the crisis line waits for a tick) | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4c_drain_tip_one.patch` — the drain line shows for a factor of 1 (no longer applies since [B-6b4] - taken again at [B-6b12] as `b6b12_drain_tip_one`) | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_complement.patch` — the drain line tells 1 - the factor | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_below_one.patch` — the drain line shows only for a factor below 1 (no longer applies since [B-6b4] - taken again at [B-6b12] as `b6b12_drain_tip_below_one`) | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_infinite.patch` — the drain line shows for an infinite factor (no longer applies since [B-6b4] - taken again at [B-6b12] as `b6b12_drain_tip_infinite`) | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_zero.patch` — the drain line shows for a factor of 0 or below (no longer applies since [B-6b4] - taken again at [B-6b12] as `b6b12_drain_tip_zero`) | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_ticks_round.patch` — ResistTicksOf rounds instead of taking the whole part | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4c_ticks_from_above_one.patch` — ResistTicksOf takes a Value of 1 as none | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4c_ticks_no_cut.patch` — ResistTicksOf does not cut at 2^31 | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4c_apply_takes_zero.patch` — DelayHazard takes a Value below 1 (adds no tick, spends the fleet) | `Cosmic_Ascension.Sim.ResistDelay` |
| `B4/` | `b4d1_voice_window_two.patch` — the fallen's first-person line opens at the second rung | `Cosmic_Ascension.Log.ClaimVoice` |
| `B4/` | `b4d1_voice_severity_two.patch` — the first-person line is Severity 2 | `Cosmic_Ascension.Log.ClaimVoice` |
| `B4/` | `b4d1_voice_no_owner.patch` — the first-person line names no owner | `Cosmic_Ascension.Log.ClaimVoice` |
| `B4/` | `b4d1_voice_first_instance.patch` — the first instance speaks, not the one that joined | `Cosmic_Ascension.Log.ClaimVoice` |
| `B4/` | `b4d1_voice_no_sign.patch` — the claim's voice comes with no sign | `Cosmic_Ascension.Log.ClaimVoice` |
| `B4/` | `b4d1_voice_writes_world.patch` — the voice writes a key of the world map | `Cosmic_Ascension.Log.ClaimVoice` |
| `B4/` | `b4d1_worn_every_tick.patch` — the fleet worn line is told on every tick at the target | `Cosmic_Ascension.Log.AttritionLine` |
| `B4/` | `b4d1_worn_ai_too.patch` — an AI people's worn fleet is told | `Cosmic_Ascension.Log.AttritionLine` |
| `B4/` | `b4d1_worn_severity_two.patch` — the fleet worn line is Severity 2 | `Cosmic_Ascension.Log.AttritionLine` |
| `B4/` | `b4d1_worn_no_focus.patch` — the fleet worn line names no system | `Cosmic_Ascension.Log.AttritionLine` |
| `B4/` | `b4d1_schema_lacks_words_always.patch` — the row test lacks_words always holds | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_lacks_words_first_text.patch` — lacks_words reads the first text of a list alone | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_lacks_words_list_unchecked.patch` — a lacks_words that names no word list of the schema passes (no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_schema_lacks_words_list_unchecked`) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_lacks_words_any_kind.patch` — lacks_words may be asked of a number column | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_hazard_expiry_rescue_off.patch` — the hazards' expiry lines take the rescue words (no longer applies since [B-6b5] - taken again as `b6b5_schema_hazard_expiry_rescue_off`) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_event_lines_holy_off.patch` — the events' lines take the holy words (no longer applies since [B-6b5] - taken again as `b6b5_schema_event_lines_holy_off`) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_event_lines_soul_off.patch` — the events' lines take the soul verbs (no longer applies since [B-6b5] - taken again as `b6b5_schema_event_lines_soul_off`) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_realm_without_most_high.patch` — realm_names misses '지극히 높은' | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_realm_with_eumbu.patch` — realm_names holds '음부' ('처음부터' is refused) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_myth_takes_baro.patch` — holy_and_myth takes a name of the hand list ('바로') (no longer applies since [B-6b6] - taken again as `b6b6_schema_myth_takes_baro`) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_petition_failure_rescue_off.patch` — a petition's failure text takes the rescue words | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_sign_slots_wide.patch` — the claim's sign may call {Hazard} | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_first_person_forms_differ.patch` — the lure's words and the claim lines' words differ | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_verdict_without_soul_has_soul.patch` — verdict_without_soul keeps '영혼' (no longer applies since [B-6b6] - taken again as `b6b6_schema_verdict_without_soul_has_soul`) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_singing_star_by_the_hedge.patch` — a text holds the singing star and the sanctuary's hedge | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d2_unpressed_keeps_ease.patch` — CosmicTests::WithoutPressure keeps the vigil's draw row | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_unpressed_keeps_cards.patch` — CosmicTests::WithoutPressure keeps the measured card | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_unpressed_keeps_edict.patch` — CosmicTests::WithoutPressure keeps the seventh edict | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_unpressed_keeps_voice.patch` — CosmicTests::WithoutPressure keeps the claim's first-person lines | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_unpressed_drops_more.patch` — CosmicTests::WithoutPressure takes the plague's repentance row away too | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_chain_skips_conditions.patch` — a chain puts its next card without asking the card's conditions | `Cosmic_Ascension.Sim.PressureChain` |
| `B4/` | `b4d2_data_card_before_1800.patch` — (data) the claim card comes from tick 1,000 | `Cosmic_Ascension.Sim.PressureChain` |
| `B4/` | `b4d2_data_discern_cheaper.patch` — (data) the discernment costs 150 Insight | `Cosmic_Ascension.Sim.PressureChain` |
| `B4/` | `b4d2_data_delay_40.patch` — (data) the fleet buys the claim 40 ticks | `Cosmic_Ascension.Sim.PressureChain` |
| `B4/` | `b4d2_data_discern_no_chain.patch` — (data) the discernment puts no second card | `Cosmic_Ascension.Sim.PressureChain` |
| `B4/` | `b4d2_data_resist_factor.patch` — (data) the fleet's time spends 0.6 of the fleet (no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_data_resist_factor`) | `Cosmic_Ascension.Sim.PressureChain` |
| `B4/` | `b4d2_data_silence_first_rung.patch` — (data) the seventh edict stands from the first rung | `Cosmic_Ascension.UI.EdictWindow` |
| `B4/` | `b4d2_data_silence_third_rung.patch` — (data) the seventh edict stands from the third rung | `Cosmic_Ascension.UI.EdictWindow` |
| `B4/` | `b4d2_data_ease_no_vigil.patch` — (data) the vigil's draw row eases a claim with no vigil held | `Cosmic_Ascension.Sim.VigilEase` |
| `B4/` | `b4d2_data_ease_plague.patch` — (data) the vigil's draw row resolves a plague | `Cosmic_Ascension.Sim.VigilEase` |
| `B4/` | `b4d2_data_ease_heavier.patch` — (data) the vigil's draw row weighs 1.3 (over the quarter of the theodicy) | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_fleet_suffix_index.patch` — the fleet's suffix names the rung's index, not its number | `Cosmic_Ascension.UI.PeoplesTab` |
| `B4/` | `b4d2_fleet_suffix_first_entry.patch` — a rung gives a fleet whenever its value is not 0 (a negative one too) | `Cosmic_Ascension.UI.PeoplesTab` |
| `B4/` | `b4d2_fleet_suffix_always.patch` — the fleet's suffix names the second rung when no rung has a fleet | `Cosmic_Ascension.UI.PeoplesTab` |
| `B4/` | `b4d2_fleet_rung_from_first.patch` — the fleet's first rung is looked for from the first rung, not from the people's own | `Cosmic_Ascension.UI.PeoplesTab` |
| `B4/` | `b4d2_theodicy_standing_ignored.patch` — the theodicy's count leaves out a Standing row that resolves the hazard | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_theodicy_no_lock.patch` — the theodicy never asks the throne lock to outlast a delayed hazard | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_theodicy_lock_strict.patch` — a throne lock as long as the longest stand is refused | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4d2_card_held_no_mark.patch` — a held edict's choice on a card carries no held mark | `Cosmic_Ascension.UI.PetitionCard` |
| `B4/` | `b4d2_card_mark_any_edict_block.patch` — every edict-gated choice on a card carries the held mark | `Cosmic_Ascension.UI.PetitionCard` |
| `B5/` | `b5a_risk_no_protection.patch` — the reach skips the birth protection | `Cosmic_Ascension.Sim.VesselAtRisk` |
| `B5/` | `b5a_risk_protection_le.patch` — the birth protection shields its last tick too (<= for <) | `Cosmic_Ascension.Sim.VesselAtRisk` |
| `B5/` | `b5a_risk_ruler_by_system.patch` — a system subject reaches whoever stands in it, a ruler too | `Cosmic_Ascension.Sim.VesselAtRisk` |
| `B5/` | `b5a_risk_any_people.patch` — a people subject reaches the ruler of any people | `Cosmic_Ascension.Sim.VesselAtRisk` |
| `B5/` | `b5a_risk_in_interregnum.patch` — the reach does not ask that the calling is embodied | `Cosmic_Ascension.Sim.VesselAtRisk` |
| `B5/` | `b5a_aim_lift_ignored.patch` — the lift opens neither Capital nor LastSettled | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5a_aim_lift_capital_only.patch` — the lift opens Capital only | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5a_aim_lift_last_only.patch` — the lift opens LastSettled only | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5a_aim_lift_opens_adjacency.patch` — the lift opens NotAdjacent too | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5a_aim_lift_opens_besieged.patch` — the lift opens Besieged too | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5a_aim_lift_opens_nofleet.patch` — the lift opens NoFleet too | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5a_aim_lift_opens_notsettled.patch` — the lift opens NotSettled too | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5a_ai_stage_lifts.patch` — the AI stage asks its aim with the lift | `Cosmic_Ascension.Sim.AIAimLift` |
| `B5/` | `b5b_death_keeps_reverence.patch` — a death keeps every people's reverence | `Cosmic_Ascension.Calling.DeathPipeline`, `Cosmic_Ascension.Calling.RulerDeath` |
| `B5/` | `b5b_ruler_death_no_lock.patch` — a ruler's death locks no throne | `Cosmic_Ascension.Calling.RulerDeath`, `Cosmic_Ascension.Sim.ThroneLockEveryPath` |
| `B5/` | `b5b_death_no_regent.patch` — a ruler's death leaves no regent (the hook takes the abdication only) | `Cosmic_Ascension.Calling.RulerDeath` |
| `B5/` | `b5b_death_new_eye.patch` — a ruler's death looks around as a deposition does | `Cosmic_Ascension.Calling.RulerDeath` |
| `B5/` | `b5b_ruler_death_name_first.patch` — a ruler's death draws the regent's name before its profile | `Cosmic_Ascension.Calling.RulerDeath` |
| `B5/` | `b5b_leap_span.patch` — the leap maps the number over its span, not its span + 1 (the longest is never drawn) | `Cosmic_Ascension.Sim.LeapDraw` |
| `B5/` | `b5b_leap_no_cap.patch` — the leap ignores LeapTotalCap | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5b_countdown_never_succeeds.patch` — the countdown reaches 0 and never succeeds | `Cosmic_Ascension.Calling.FirstTickOfLife`, `Cosmic_Ascension.Calling.HardReset`, `Cosmic_Ascension.Calling.LeapDeterminism`, `Cosmic_Ascension.Sim.NoDanglingRefsAfterLeap`, `Cosmic_Ascension.Sim.SaveLeapResume`, `Cosmic_Ascension.Sim.SuccessionPossible`, `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_countdown_two_a_tick.patch` — the countdown takes two ticks off each tick | `Cosmic_Ascension.Calling.DeathPipeline`, `Cosmic_Ascension.Calling.NoPlayerDuringLeap` and six more |
| `B5/` | `b5b_silence_one_draw.patch` — the silence draws one number, not two | `Cosmic_Ascension.Sim.SuccessionPossible` |
| `B5/` | `b5b_candidates_death_place.patch` — the candidates keep the death place | `Cosmic_Ascension.Sim.SuccessionCandidates` and five more |
| `B5/` | `b5b_candidates_lethal_relaxed.patch` — a lethal hazard's system stays a candidate | `Cosmic_Ascension.Sim.SuccessionCandidates` |
| `B5/` | `b5b_candidates_sphere_first.patch` — the sphere filter is let go of before the short-hazard filter | `Cosmic_Ascension.Sim.SuccessionCandidates` |
| `B5/` | `b5b_succeed_keeps_low_tier.patch` — a succession keeps a personal tier below the third | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5b_succeed_drops_way.patch` — a succession drops the way (Way.*) | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5b_name_compare_off.patch` — a new vessel's name is never compared with the previous life's | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5b_tenth_outside_guard.patch` — the tenth stage runs outside the stage guard (no longer applies since [B-6a3b] - taken again at [B-6b12] as `b6b12_tenth_outside_guard`) | `Cosmic_Ascension.Calling.LeapDeterminism` |
| `B5/` | `b5b_runleap_no_boundary.patch` — RunLeap sends no boundary | `Cosmic_Ascension.Calling.LeapDeterminism`, `Cosmic_Ascension.Sim.SaveLeapResume`, `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_feed_tick_rule_only.patch` — the feed rebuilds on a tick gap only, not on the leap's boundary | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_feed_no_simtick.patch` — the feed does not listen to OnSimTick | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_advance_no_end_cut.patch` — AdvanceLeap does not stop at the provisional end (no longer applies since [B-6a3c] - `RunLeap` stops at the ending record, covered by `b6a3c_runleap_never_stops` and `b6a3c_advanceleap_no_second_call`) | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_death_stop_after_end.patch` — the boundary looks for a death only after the end's early return | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_save_role_before_death.patch` — the save causes put Role before Death | `Cosmic_Ascension.Sim.SaveLeapResume`, `Cosmic_Ascension.Sim.SaveTriggers`, `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_first_tick_of_life_off.patch` — a succeeded life's first tick compares with the head | `Cosmic_Ascension.Calling.FirstTickOfLife` |
| `B5/` | `b5b_dying_vessel_moves.patch` — a vessel whose death is recorded still ends its journey | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5b_crossing_error_on_death.patch` — the crossing stage's error line ignores a recorded death | `Cosmic_Ascension.Calling.RulerDeath` |
| `B5/` | `b5b_pick_unclamped.patch` — PickOf reads a number past 1 unclamped | `Cosmic_Ascension.Sim.SuccessionCandidates` |
| `B5/` | `b5c_digest_reads_knowing.patch` — the digest's rung item reads `Knowing.MetCivs`, not the known view | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_reads_known_systems.patch` — a digest line's system is looked up in `Knowing.KnownSystems` (breaks the CosmicSeeAll case) | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_reads_claim.patch` — the digest reads `Log.Claim` lines | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_other_purpose.patch` — the digest's variants are picked with `Purpose::Encounter` | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_noleap_head.patch` — a leap of 0 shows the head too | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_far_seen.patch` — a far line does not bring `Digest.Elsewhere` | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_thrones_unbounded.patch` — every changed seat becomes an item | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_reign_end_opens_epitaph.patch` — an abdication or a deposition opens the epitaph, not the reign end's page | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_next_without_leap.patch` — the epitaph's 'next' opens the digest without running the leap | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_runend_ignores_pages.patch` — the chronicle's wait counts while a calling page shows (no longer applies since [B-6b4] - taken again as `b6b4_runend_ignores_pages`) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_speeds_not_grey.patch` — the speed buttons stay enabled while a calling page shows | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_apart_never.patch` — the answer on the killing hazard's card never stands apart | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_answer_any_slot.patch` — an answer title keeps a slot its line does not fill (empty) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_first_death_skips.patch` — the first death's epitaph offers 'skip' | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_inauguration_gives_speed.patch` — closing the inauguration gives the speed back | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_no_reopen_on_load.patch` — a load in the interregnum does not open the epitaph again | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_span_not_gathered.patch` — the span with no vessel is not gathered | `Cosmic_Ascension.Log.ChronicleBuild`, `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5c_end_tick_life_shown.patch` — a life born on the end's tick gets a chapter | `Cosmic_Ascension.Log.ChronicleBuild`, `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5c_leap0_tick_twice.patch` — a leap of 0's shared tick is shown in both chapters | `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5c_span_unshown_people.patch` — the span gathers the peoples of an unshown life too | `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5c_span_uncapped.patch` — the span ignores `MaxNoVesselLines` | `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5c_born_among_off.patch` — a life that never ruled shows its birth people's name alone | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5c_died_walks.patch` — a ruler who died on the throne reads as having wandered | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5c_no_vessel_head_off.patch` — the span's lines stand without the `Chronicle.NoVessel` head | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5c_until_deposed_only.patch` — the top bar's clock-end line shows only after a reign ended (not for a called vessel that never ruled) | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5c_bar_people_in_interregnum.patch` — the top bar shows the ruled people in the interregnum | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5c_lethal_standing_only.patch` — the lethal place is where the vessel stands (`StandingIndexOf`, regenerated at [B-5r]), not where it counts as standing (a travelling vessel is not warned of the system it left) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_follow_off.patch` — a new lethal hazard where the vessel stands does not stop the time | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_seen_kept.patch` — the lethal hazard seen is kept across a new galaxy or a load | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_no_arrival.patch` — the lethal pause compares the hazard's tag only (an arrival where the same tag stands is no news) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_panel_no_warning.patch` — the system panel says nothing of a lethal hazard (no longer applies since [B-6a4] - taken again at [B-6b4] as `b6b4_panel_no_warning`) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_travel_tip_no_warning.patch` — the travel button's tooltip says nothing of the lethal hazard where the vessel stands (the destination's own [B-5r] words stay; regenerated at [B-5r]) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lock_reason_old.patch` — a new vessel gets the old lock reason ('left not long ago') | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_digest_quiet_then_elsewhere.patch` — a digest with no item closes with `Digest.Elsewhere` after the quiet line | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_throne_converted_off.patch` — the throne item never reads a rebel a rebellion made the ruler: the first rebel to take a throne with no record is not told | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_throne_rebel_counts.patch` — a record of the rebellion's outbreak row counts whatever its role: a resolved rebellion's rebel becomes a throne item | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_throne_outbreak_gone.patch` — the taking is searched from the log's first line, not from the outbreak's own told line: a later resolution proves a taking the log no longer holds | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_rose_first.patch` — a people that rose twice is told at its first rise | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_no_knowledge_cut.patch` — the digest reads the knowledge of now: what the succession's `Install` made known becomes items | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_known_cut_inclusive.patch` — `KnownViewOf`'s cut counts a system marked on the cut's tick | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_known_cut_people_off.patch` — `KnownViewOf`'s cut counts every people met | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_owner_unshown.patch` — an owner change to a people the view does not show becomes an `Owner` item | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_owner_from_unshown.patch` — an owner change from a people the view does not show becomes an `Owner` item | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_opens_at_first_line.patch` — `CosmicChronicle::OpensCalledChapter` opens a called life's chapter at the first line of its BornTick tick (the chronicle's span and the digest both lose that tick) | `Cosmic_Ascension.UI.Digest`, `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5c_span_birth_tick_in_body.patch` — the chronicle opens a called life at its BornTick tick's first entry again (the span with no vessel loses that tick) | `Cosmic_Ascension.UI.Digest`, `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5c_digest_span_birth_tick_cut.patch` — the digest's span stops before the next life's BornTick tick | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_page_takes_speed.patch` — a speed set while a calling page shows runs the time (the controller's speed path keeps no page guard: the presentation's, the card's and the silence's give-back would run it under the page) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_death_wears_presentation.patch` — a death's boundary leaves a showing ascension presentation up (no silent fold: its timed close would give its speed back under the epitaph) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_reign_end_wears_presentation.patch` — an abdication's or a deposition's boundary leaves a showing ascension presentation up under the reign end's page | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_silence_stops_short.patch` — a leap that ends in the silence (no candidate) stops there - it does not run on to the end that is ahead (switch on) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_silence_keeps_time.patch` — the digest's 'next' in the silence with no end ahead folds the pages but leaves the time stopped forever with no vessel (switch off) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_reopen_keeps_running.patch` — a load in the interregnum over running time opens the epitaph and leaves the time running (the HUD's and the controller's BeginPlay calls of FollowInterregnumOnOpen are left to the hand check - no patch) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_no_reopen_after_succession.patch` — a load (a continue) on a succession's tick opens no page: no epitaph, digest or inauguration for the vessel the player was not shown | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_wheel_under_pages.patch` — the mouse wheel zooms the camera under a calling page | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_epitaph_keeps_exalt.patch` — the epitaph's last answers show the titles that speak to the reader or exalt (`CosmicRows::EpitaphLeavesOut` not read) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_cause_card_title.patch` — a death by a card (Death.Encounter) puts the card's title on the cause line instead of its hazard's name or no source | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_inauguration_no_people.patch` — the inauguration's facts leave out the people the new vessel was born among (`Chronicle.BornAmong`) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_end_tick_inaugurates.patch` — a succession on the end's tick opens the inauguration from the digest instead of the chronicle (the end does not win) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_skip_end_inaugurates.patch` — 'skip' to a succession on the end's tick closes as the inauguration does - no chronicle | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_death_card_stays.patch` — the death does not fold the card (an outcome shown on it stays up over the epitaph) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_leap0_no_digest.patch` — a leap of 0's epitaph 'next' folds the pages (AdvanceLeap false) - no digest line, no inauguration | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_apart_any_last.patch` — the newest answer of any card is set apart from the cause line, whatever the cause or the place | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_apart_any_card.patch` — the newest answer is set apart whether or not its card is the killing hazard's | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_apart_any_place.patch` — the killing hazard's card answered on another system than the death's is set apart too | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_apart_any_cause.patch` — the answer is set apart for a life that a hazard did not close (the source row alone read) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_digest_page_no_seeall.patch` — the digest page ignores the controller's see-all (`CosmicSeeAll`) | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_crown_not_noted.patch` — `TakeThrone` does not note the new reign: one taken and left between two boundaries opens no reign end's page | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_galaxy_reign_not_noted.patch` — a new galaxy or a load does not note the role (`HandleGalaxyGenerated`'s FollowReign): the next abdication opens no reign end's page | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_inauguration_no_camera.patch` — closing the inauguration does not put the camera on the new vessel's system | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_reign_seen_always.patch` — `FollowReign` compares with no role it saw: every boundary of the walking vessel opens the reign end's page again | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_lethal_no_protection.patch` — the lethal place drops the shared reach (`VesselAtRiskOf` — standing through `LethalStayOf`'s placed vessel and under way, regenerated at [B-5r]): a vessel shielded at its birth is warned and stopped for a hazard that cannot kill it | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_reach_after_expiry.patch` — the lethal place asks the reach one tick after the hazard's expiry (in `LethalStayOf` and under way, regenerated at [B-5r]) (the protection's last tick no longer shields) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_map_dead_vessel.patch` — the map's mark (`VesselLookOf`) asks the role alone: the dead ruler, a wanderer in the interregnum, is drawn | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_vessel_shown_under_page.patch` — `IsVesselShown` forgets the calling pages: the digest, the inauguration and the reign end show the vessel under the veil | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_vessel_shown_end_tick.patch` — `IsVesselShown` forgets the end's tick: the vessel an ended run's last tick installed is drawn after 'look at the map' | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_fold_no_map_redraw.patch` — the pages' fold does not ask the map to redraw (a stopped simulation sends no tick): the new vessel's mark does not come | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_bar_vessel_under_page.patch` — the top bar's wanderer line ignores `IsVesselShownFor` | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_bar_lethal_under_page.patch` — the top bar's lethal warning ignores `IsVesselShownFor` | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_map_vessel_under_page.patch` — the map's mark and lane ignore `IsVesselShown` | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_panel_wanderer_under_page.patch` — the system panel's wanderer lines, commands and lethal warnings ignore `IsVesselShownFor` | `Cosmic_Ascension.UI.VesselShown` |
| `B5/` | `b5c_page_stop_drops_reason.patch` — a plain stop (the pause key, `SetSpeed(0)`) under a calling page replaces the page's reason with none | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_reopen_stopped_silent.patch` — a load on a succession's tick over a time already stopped with no reason opens the epitaph with no reason line | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5c_digest_reads_record.patch` — the digest reads `Log.Record` lines | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5c_digest_reads_calling.patch` — the digest reads `Log.Calling` lines | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5d1_death_line_after_release.patch` — a ruler's death line is told after the throne is left (the regent named, the vessel no longer ruling, the regent's numbers drawn) instead of before what the death changes | `Cosmic_Ascension.Calling.RulerDeath` |
| `B5/` | `b5d1_death_line_after_leap.patch` — a wanderer's death line is told after the interregnum and its leap are set (the reverence cleared, the leap's number drawn) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_no_death_line.patch` — the death tells no line | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_death_line_draws_sim.patch` — the death line draws a number of the sim stream (rule 3: its variant is the tell stream's alone) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_death_line_focus.patch` — the death line has a focus system (a narrator's line about the place of the death could follow a first-person line about it) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_death_line_no_hazard.patch` — the death line's {Hazard} is left empty (not the hazard row behind the death; no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_death_line_no_hazard`) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_death_line_no_name.patch` — the death line's {Player} is left empty (not the closed life's name - the third person, decision 12-da; no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_death_line_no_name`) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_death_line_any_cause.patch` — a cause with no row of its own (the measurement's Death.Age) is told with the hazard death's row (no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_death_line_any_cause`) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_no_inauguration_line.patch` — the succession tells no inauguration line | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5d1_inauguration_focus.patch` — the inauguration line has no focus (not the new life's birth system and people) | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5d1_inauguration_before_install.patch` — the inauguration line is told before Install (it names the life that closed, not the new vessel) | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5d1_inauguration_in_silence.patch` — the silence (no candidate) tells an inauguration line and draws a number of the tell stream | `Cosmic_Ascension.Calling.HardReset` |
| `B5/` | `b5d1_feed_without_inauguration.patch` — a succession after a leap tells no inauguration line (the feed's last item after the leap is a world line) | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_opens_never.patch` — no line opens a called life's chapter (CosmicChronicle::OpensCalledChapter false for the inauguration line too) | `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5d1_opens_any_tick.patch` — an inauguration line of another tick opens the called life's chapter (the rule does not ask the BornTick) | `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5d1_leap_zero_keeps_tick.patch` — with a leap of 0 the dead life's chapter keeps its whole tick (the inauguration line and what follows it), and the called life's chapter starts after it | `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5d1_after_span_whole_end_tick.patch` — with a life born on the end's tick, the last chapter's After span keeps the whole end tick: the inauguration line stands under Chronicle.NoVessel (digest-5) | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5d1_digest_past_inauguration.patch` — the digest reads the succession's tick past the inauguration line (what is told after it becomes an item) | `Cosmic_Ascension.UI.Digest` |
| `B5/` | `b5d1_record_told.patch` — a record line is judged by the scope like any other: told in the tell-all mode and in scope | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_record_no_annex.patch` — a claim that joins a sphere records nothing (no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_record_no_annex`) | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_record_unjoined_claim.patch` — every claim's expiry records a loss, one that joins nothing too (no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_record_unjoined_claim`) | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_record_no_destroy.patch` — a settled system destroyed records nothing | `Cosmic_Ascension.Log.TwoBooksSameLoss` |
| `B5/` | `b5d1_record_after_voice.patch` — the record line comes after the claim's sign and first person | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_record_before_expiry.patch` — the record line comes before the expiry line | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_record_no_focus.patch` — the record line names no system to the books (FocusSystemID None) | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_record_draws_sim.patch` — the record line draws a number of the sim stream | `Cosmic_Ascension.Log.TwoBooksSameLoss` |
| `B5/` | `b5d1_record_faith_only.patch` — only a people with a faith trait has its loss recorded (the old rule) | `Cosmic_Ascension.Log.RecordEveryPeople`, `Cosmic_Ascension.Log.TwoBooksSameLoss` |
| `B5/` | `b5d1_cleanup_records.patch` — the crossing stage's cleanup records the people it takes away | `Cosmic_Ascension.Log.TwoBooksSameLoss` |
| `B5/` | `b5d1_books_admit_sign.patch` — the two books take a `Log.Sign` line (the claim's sign) into part one | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_span_keeps_claims.patch` — the chapters and the span with no vessel keep `Log.Claim` and `Log.Record` lines | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_books_one_group.patch` — part two puts every claim under one group, not one group a claimant | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_books_bare_part_one.patch` — a part two over an empty part one leaves part one's head bare (no 'more') | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_books_no_line_cap.patch` — the two books' chapter drops its 28-row cap | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_books_claims_purple.patch` — the page draws the first-person lines of part two in the feed's purple | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_books_always.patch` — the page stands the two books' heads with no `Log.Record` and no `Log.Claim` | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_books_empty_part_two_head.patch` — the page stands part two's head over no claim | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_books_no_claimant_head.patch` — the page leaves out the claimant's head (`Chronicle.ClaimedBy`) | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_inauguration_always_named.patch` — the inauguration line always tells the named row, also after a fallen's first person about the birth system on the same tick (a narrator's line calls the system the claim just joined - docs/70 section 8.3 rule 5) | `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_unnamed_calls_system.patch` — the unnamed inauguration row is told with the birth system as its focus (it calls the system the first person just claimed) | `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_claim_line_told_only.patch` — the log's question of a same-tick first person passes the first person kept untold (below the third rung the named row follows it) | `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_claim_line_any_tick.patch` — the log's question of a same-tick first person reads a first person of any tick | `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_opens_named_only.patch` — `CosmicChronicle::OpensCalledChapter` opens the called life's chapter at the named inauguration row only (the unnamed row opens nothing) | `Cosmic_Ascension.Log.ChronicleBuild`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_death_line_death_place.patch` — the death line has the place of the death (the dead vessel's `CurrentSystemID`) as its focus: a narrator's line calls the claimed system after the first person when the vessel dies there on the joining tick (no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_death_line_death_place`) | `Cosmic_Ascension.Log.ClaimVoice`, `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_unseen_line_shown.patch` — `CosmicChronicle::IsUnseenLifeLine` holds nothing back: the feed shows the inauguration line of the vessel the end's tick installed | `Cosmic_Ascension.UI.LeapBoundary`, `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5d1_unseen_line_while_running.patch` — `CosmicChronicle::IsUnseenLifeLine` holds the inauguration line back while the run goes on (the feed's last item after a leap is not the new vessel's line) | `Cosmic_Ascension.UI.LeapBoundary`, `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5d1_feed_rebuild_shows_unseen.patch` — the feed's rebuild skips only untold lines (the inauguration line of the vessel the end's tick installed stands last) | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_feed_appends_unseen.patch` — a new line of the vessel the end's tick installed, heard after the end latched, is appended to the feed | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_feed_no_end_refresh.patch` — the end's latch announces nothing (`OnRunEndChanged`): after a leap of 0 on the end's tick the feed keeps the inauguration line it heard (no longer applies since [B-6a3c] - taken again at [B-6b12] as `b6b12_feed_no_end_refresh`) | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_feed_clear_no_refresh.patch` — forgetting the end announces nothing: the feed keeps holding the inauguration line back once the end is switched off | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_books_part_one_alone.patch` — part one is thinned first-and-last alone again, blind to part two: a shown claim can stand over a place whose record part one left out | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_books_no_item_cap.patch` — part two loses its 12-item cap (the fit test and the deal's `ItemsLeft`): only the 28-row cap stops it | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_record_any_destruction.patch` — the destruction record drops its settled-only guard (`bWasSettled`): a system under its settlement or a wild one destroyed is recorded | `Cosmic_Ascension.Log.TwoBooksSameLoss` |
| `B5/` | `b5d1_books_part_head.patch` — part one's head goes back to a part's head (grey 18): the two books' chapter has no chapter-level head | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_books_head_after_lines.patch` — a claimant's head (`Chronicle.ClaimedBy`) stands after its lines, not over them | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_epitaph_keeps_star_watch.patch` — the epitaph shows the trembling star's watch ('흔들리는 별 곁에서', the 'beside' title of the sacrifice reading - header 3.6-1) among a dead vessel's last answers again: CosmicRows::EpitaphLeavesOut drops EncStarWatch | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5d1_epitaph_keeps_eulogy_petitions.patch` — the epitaph shows the wise heart and the feast (a virtue, a memorial day under a death) again: CosmicRows::EpitaphLeavesOut drops PetitionWiseHeart and PetitionFeast | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5d2_risk_reaches_ruler.patch` — a KillPlayer on a system reaches whoever stands there, a ruler too (CosmicHazards::VesselAtRiskOf drops its wanderer guard): the ruler standing where the AI awakening expires dies | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_risk_any_system.patch` — a KillPlayer on a system reaches a wanderer anywhere (the reach no longer asks where it stands): a wanderer one lane from the expiry dies | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_risk_spares_travellers.patch` — a vessel under way is out of reach (the reach asks for no journey): the vessel that set out from the awakening's system lives (header 2.8: it stands where it left until it arrives) | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_risk_no_protection.patch` — the reach drops the birth protection: a vessel shielded on the expiry's tick dies | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_kills_branch_only.patch` — CosmicHazards::KillsAVesselOf reads the branch effects alone: the game's AI awakening (own side lethal) reads as harmless, and the succession offers its system (no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_kills_branch_only`) | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_kills_branch_only_ui.patch` — the same patch against the screens: no warning where the game's AI awakening stands (and none for the test row; no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_kills_branch_only`) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5d2_expiry_own_side_always.patch` — an expiry runs its own effects whichever side its number takes: the AI awakening's branch side kills | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_kill_no_source_row.patch` — a recorded role effect keeps no source row: the death's CauseSourceRow is not crisis_ai_awakening and its line names no hazard | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_card_resolves_nothing.patch` — ResolveHazard reports success and leaves the slot: the awakening the card's success ended still expires | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_candidates_keep_lethal.patch` — CosmicSuccession::CandidatesOf drops its lethal filter: a system holding a live AI awakening is offered, and so is the one place left | `Cosmic_Ascension.Calling.LethalSource` |
| `B5/` | `b5d2_without_death_keeps_expiry.patch` — CosmicTests::WithoutDeath leaves the hazards' own expiry effects alone: the game's one death survives the data-off side of a pair | `Cosmic_Ascension.Data.Integrity` |
| `B5/` | `b5d2_first_crisis_edge_open.patch` — the first-crisis cross-check accepts a birth protection that ends on the first crisis's expiry tick (`<` for `<=`) | `Cosmic_Ascension.Data.Integrity` |
| `B5/` | `b5r_warn_stay_expiry_on_arrival.patch` — a lethal hazard that expires on the tick the vessel comes to stand in its system warns (no way out) although the crisis stage runs before the move stage | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_stay_trap_narrow.patch` — an expiry exactly one lane's time after the vessel comes to stand there reads as 'can leave' (a vessel that sets out then still stands there at the expiry) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_stay_trap_wide.patch` — 'no way out' reaches two lanes' time after the vessel comes to stand there (an expiry it could leave before reads as a trap) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_stay_no_trap.patch` — the 'no way out' band is never answered (every warned stay reads as 'can leave') | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_stay_unshielded.patch` — where the vessel would come to stand, the reach is not asked at the expiry (the birth protection does not shield it) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_stay_not_placed.patch` — the reach is asked of the vessel where it stands now, not placed in the system it would come to stand in (no destination is ever warned) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_stay_ruler_not_placed.patch` — the placed vessel keeps the ruler's role (an abdication onto a lethal hazard is never warned: what happens to a system never reaches a ruler) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_underway_origin_always.patch` — under way the system the vessel left stays the lethal place whatever the expiry (the [B-5c] rule: a hazard that expires after the arrival still warns and pauses) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_underway_lane_not_journey.patch` — under way the system it left is weighed against a whole lane's time, not the journey's own time left | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_travel_ahead_asks_here.patch` — [Travel] asks of the system the vessel stands in, not of the destination (the destination's lethal hazard is never told) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_travel_ahead_now.patch` — [Travel] weighs the destination from now, not from the arrival (a hazard that expires before the vessel would arrive warns) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_travel_no_mark.patch` — [Travel] to a lethal destination carries no short words after its label | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_travel_never_late.patch` — [Travel] away never says that leaving now is too late (a lethal hazard where the vessel stands that expires within a lane's time) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_travel_late_any_band.patch` — [Travel] away says 'too late' for every lethal hazard where the vessel stands, also one it can still leave | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_abdicate_hidden.patch` — [Abdicate] warns only while the vessel is not shown (never in play) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_abdicate_now.patch` — [Abdicate] weighs the walk from now, not from the tenth stage of the next tick (a hazard that expires while the vessel still rules warns) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_abdicate_tip_dropped.patch` — [Abdicate]'s tooltip leaves out the lethal hazard where the vessel would walk from | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_abdicate_no_mark.patch` — [Abdicate] over a lethal hazard carries no short words after its label | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_bar_ahead_asks_origin.patch` — the top bar under way asks of the system the vessel left, not of the one it goes to (the destination's lethal hazard is never told) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_bar_ahead_lane.patch` — the top bar under way weighs the destination from a whole lane's time, not the journey's own time left | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_bar_one_band.patch` — the top bar under way says 'can leave' words for the 'no way out' band too | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_card_no_line.patch` — a wanderer's card at the lethal place has no lethal line | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_card_any_system.patch` — the card's lethal line is asked of wherever the vessel stands, not of the card's own system | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_card_outcome_stale.patch` — under an outcome a refresh leaves the card's lethal line as it was (a crisis gone keeps its line) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_card_close_keeps.patch` — closing the card keeps its lethal line | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_fallen_status_one_tick.patch` — the tenth stage clears the vessel's status on every tick with no role effect (the fallen monarch's status lasts only the tick of the deposition) | `Cosmic_Ascension.Sim.PhaseTwoExit` |
| `B5/` | `b5r_deposed_no_status.patch` — a deposition leaves no fallen monarch's status (read at the boundary of the deposition) | `Cosmic_Ascension.Sim.PhaseTwoExit` |
| `B5/` | `b5r_death_draws_when_stepped.patch` — a death draws one more number of the sim stream while the broadcast gate is open, so stepping one tick at a time is no longer the same world as the whole run at once (lived while the test ran WithoutDeath) | `Cosmic_Ascension.Sim.PhaseTwoExit` |
| `B5/` | `b5r_phase_two_no_death.patch` — the tenth stage never carries a death out (the whole run of seed 1 - seed 18 until [B-6b11] - crosses no death and no succession) | `Cosmic_Ascension.Sim.PhaseTwoExit` |
| `B5/` | `b5r_warn_card_trap_never.patch` — a wanderer's card at a lethal hazard that expires within one lane's time never says that leaving after the answer is too late (the plain words in the trap band too) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_card_one_band.patch` — the card window's two bands swapped: the trap's words for a lethal hazard the vessel can still leave, the plain words where leaving after the answer is too late | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5r_warn_ship_hint_ignores_lethal.patch` — the capital's first-ship line waits for a rebellion only, so it invites the abdication while a lethal hazard stands where the vessel off the throne would walk from | `Cosmic_Ascension.UI.LethalWarning` |
| `B6a/` | `b6a1_notables_allow_end_peoples.patch` — the allowance reads `PeoplesOfEveryLife` at the end of the run instead of cutting at the `BornTick` of the first life the people belongs to (a rebellion line that left the log before a succession later called a life among that people is demanded in the chronicle) | `Cosmic_Ascension.Sim.NotablesFromEventsOnly` |
| `B6a/` | `b6a1_notables_no_allowance.patch` — the test drops its `#118` allowance (`bMayHaveLeft` false): a Severity 1 outbreak line of an AI people that left the log is demanded in the log or chronicle | `Cosmic_Ascension.Sim.NotablesFromEventsOnly` |
| `B6a/` | `b6a2_ending_tick_default.patch` — `FCosmicEndingRecord::Tick` starts at -1 instead of the frozen 0 (a changed initializer of a saved field reads every old save's record as that value) | `Cosmic_Ascension.Sim.SaveEndingRecord` |
| `B6a/` | `b6a2_ending_novessel_unsaved.patch` — `bNoVessel` loses its `UPROPERTY()`: the field is neither saved nor hashed | `Cosmic_Ascension.Sim.SaveEndingRecord`, `Cosmic_Ascension.Sim.HashCoversEnding` |
| `B6a/` | `b6a2_ending_out_of_world_hash.patch` — the hash walk leaves `Ending` out of the world hash (the record is world state — B6a decision 3) | `Cosmic_Ascension.Sim.HashCoversEnding`, `Cosmic_Ascension.Sim.TellStreamSeparate` |
| `B6a/` | `b6a3a_aware_no_rung_gate.patch` — the awareness flag drops its third-rung gate: a claim on a settled system of the ruled people at the second rung makes the claimant aware | `Cosmic_Ascension.Sim.AwarenessOnce` |
| `B6a/` | `b6a3a_aware_no_pending_gate.patch` — the awareness flag drops its `Calling.PendingEffect` gate: a vessel whose deposition or death this tick already recorded is noticed | `Cosmic_Ascension.Sim.AwarenessOnce` |
| `B6a/` | `b6a3a_aware_draws.patch` — writing the awareness flag draws one number from the sim stream | `Cosmic_Ascension.Sim.AwarenessOnce` |
| `B6a/` | `b6a3a_aware_rung_exact.patch` — the awareness flag's rung gate reads 'exactly the third rung' instead of 'the third rung or above': a claim on a ruled people on the fourth rung makes nobody aware | `Cosmic_Ascension.Sim.AwarenessOnce` |
| `B6a/` | `b6a3a_aware_no_owner_gate.patch` — the awareness flag drops its owner gate (`Player.RuledCivID != OwnerID`): the rung gate then reads the claimed system's owner, so a claim on another people's third-rung system makes its claimant aware | `Cosmic_Ascension.Sim.AwarenessOnce` |
| `B6a/` | `b6a3a_rung_end_back.patch` — `ProvisionalEndOf` answers `Rung` again for a life's `PeakTier` at the third rung (the end that B6a decision 2-ga removed; no longer applies since [B-6a3c] - `ProvisionalEndOf` deleted; not retaken at [B-6b12]: no live patch puts the third rung's end back into `EndingOf` - the `Sim.EndingOf` case 'A third peak, its people at the third rung: none' has no mutation, so a retake is owed) | `Cosmic_Ascension.Sim.EndingOf`, `Cosmic_Ascension.Sim.PlayerAscendsToT3`, `Cosmic_Ascension.UI.RunEnd`, `Cosmic_Ascension.UI.MouseOnlyAscent` |
| `B6a/` | `b6a3a_probe_no_awareness_flag.patch` — **a probe, not a mutation** (header B6a section 5.3, check d of `[B-6a3a]`): the build without the flag write gives, on the five runs with `-PastEnd=1`, both hashes of the `[B-6a2]` build's `-PastEnd=1` runs (`Saved\Balance\b6a2_pastend\`). At `[B-6a3a]` the five `-PastEnd=1` runs of the real build already gave those hashes (no claim stands on the ruled people while the policy plays), and the probe build was run on the one run where a claim does - the ruler's 20,000 ticks without the switch, two claims on the third-rung ruled people after the input stops at the clock: 1072463830 / 4098835384 without the flag against 1219932898 / 330974340 with it (notes B6a section 5) | (probe — commandlet runs, no test) |
| `B6a/` | `b6a3b_other_people_first.patch` — `EndingOf` skips the ruled people's end whenever any people stands on the fifth rung, so another people's end comes first (B6a decision 5 reversed) | `Cosmic_Ascension.Sim.EndingOf` |
| `B6a/` | `b6a3b_other_people_reads_knowing.patch` — the other people's end counts only a people the calling has met (`Knowing.MetCivs` — the world read through the knowing, CLAUDE.md rule 2) | `Cosmic_Ascension.Sim.EndingOf` |
| `B6a/` | `b6a3b_no_vessel_exception_dropped.patch` — `bNoVessel` loses its exception: a fifth rung reached in the tick the vessel died reads as an end met with no vessel | `Cosmic_Ascension.Sim.EndingOf`, `Cosmic_Ascension.Sim.EndingRecordOnce` |
| `B6a/` | `b6a3b_silent_ignores_leap.patch` — `CosmicSuccession::IsSilent` drops the leap: every interregnum tick is the silence | `Cosmic_Ascension.Sim.EndingOf`, `Cosmic_Ascension.Sim.SilenceEnds` |
| `B6a/` | `b6a3b_record_before_tenth_stage.patch` — the record is written before `ResolvePendingRoleEffect` (the tenth stage), so the tick's death is not yet carried out | `Cosmic_Ascension.Sim.EndingRecordOnce` |
| `B6a/` | `b6a3b_record_overwritten.patch` — the write loses its 'only while empty' guard: a later end overwrites the first | `Cosmic_Ascension.Sim.EndingRecordOnce` |
| `B6a/` | `b6a3b_record_draws.patch` — writing the record draws one number from the sim stream | `Cosmic_Ascension.Sim.EndingRecordOnce` |
| `B6a/` | `b6a3b_stage_reads_record.patch` — the research stage reads the record and skips itself once an end stands | `Cosmic_Ascension.Sim.EndingInert` |
| `B6a/` | `b6a3c_runleap_never_stops.patch` — `RunLeap` never stops at the tick that wrote the ending record | `Cosmic_Ascension.Sim.RunLeapStopsAtEnding`, `Cosmic_Ascension.UI.LeapBoundary` |
| `B6a/` | `b6a3c_runleap_stops_past_the_record.patch` — `RunLeap` stops at the record even when the record already stood at the call (a leap past the end stops at once) | `Cosmic_Ascension.Sim.RunLeapStopsAtEnding` |
| `B6a/` | `b6a3c_runheadless_stops_at_the_record.patch` — `RunHeadless` refuses to run once a record stands (the sim stops at the end - B6a decision 4 reversed) | `Cosmic_Ascension.Sim.RunLeapStopsAtEnding` |
| `B6a/` | `b6a3c_advanceleap_no_second_call.patch` — `AdvanceLeap` with the end switched off no longer runs the rest of the leap after `RunLeap` stopped at the record | `Cosmic_Ascension.Calling.LeapDeterminism`, `Cosmic_Ascension.UI.LeapBoundary` |
| `B6a/` | `b6a3c_advanceleap_second_call_with_the_end_on.patch` — `AdvanceLeap` runs the rest of the leap even with the end switched on (past the latched end) | `Cosmic_Ascension.UI.LeapBoundary` |
| `B6a/` | `b6a3c_latch_reads_the_predicate.patch` — `FollowEnd` latches `CosmicEndings::EndingOf` afresh instead of the record the tick wrote | `Cosmic_Ascension.Sim.SaveOldEndFlows` |
| `B6a/` | `b6a3c_latched_record_empty.patch` — the pages and the chronicle get an empty record from `LatchedRecordOf` (no note, no notice) | `Cosmic_Ascension.UI.Epitaph` |
| `B6a/` | `b6a3c_cosmicend_state_tick.patch` — the `CosmicEnd:` line prints the state's tick instead of the record's | `Cosmic_Ascension.Sim.SaveOldEndFlows` |
| `B6a/` | `b6a3c_markof_reads_clock.patch` — `UCosmicSaveSubsystem::MarkOf` marks the run ended from the clock instead of the record | `Cosmic_Ascension.Sim.SaveTriggers`, `Cosmic_Ascension.Sim.SaveOldEndFlows` |
| `B6a/` | `b6a3c_wanderer_policy_reads_clock.patch` — the wanderer policy's end reads the clock instead of the record | `Cosmic_Ascension.Sim.WandererPolicy` |
| `B6a/` | `b6a3c_ruler_policy_reads_clock.patch` — the ruler policy's end reads the clock instead of the record | `Cosmic_Ascension.Sim.RulerDiplomacyPolicy` |
| `B6a/` | `b6a4_claim_rule_no_last_unclaimed.patch` — `CosmicHazards::ClaimRuleHoldsOf` drops its last-unclaimed check (Law.Remnant): the screen and the sim move together, the fixed answer catches it | `Cosmic_Ascension.UI.ClaimMayFollow` |
| `B6a/` | `b6a4_claim_may_follow_no_edict_gate.patch` — `ClaimMayFollowOf` drops the gate of the edict that answers the hazard (the expiry resolves it without the claim rule) | `Cosmic_Ascension.UI.ClaimMayFollow` |
| `B6a/` | `b6a4_rebellion_watch_written_count.patch` — `RebellionWatchOf` writes 100 ticks instead of reading the rebellion row's count | `Cosmic_Ascension.UI.RebellionWatch` |
| `B6a/` | `b6a4_chronicle_names_unshown_people.patch` — the chronicle's cause line names another people at the fifth rung even when the viewer is not shown it (rule 14; no longer applies since [B-6b3a] - taken again as `b6b3a_chronicle_names_unshown_people`) | `Cosmic_Ascension.UI.Chronicle` |
| `B6a/` | `b6a4_provisional_notice_every_end.patch` — the provisional notice stands under every end, not only the clock's (decision 13) | `Cosmic_Ascension.UI.Chronicle` |
| `B6a/` | `b6a4_no_vessel_line_dropped.patch` — the line of an end met with no vessel never stands | `Cosmic_Ascension.UI.Chronicle`, `Cosmic_Ascension.UI.Epitaph` |
| `B6a/` | `b6a4_clock_tip_reads_rung_rule.patch` — the clock tip shows the rung's rule line (`Chronicle.RuleRung`) when the knob is 0 or below (no longer applies since [B-6b4] - taken again as `b6b4_clock_tip_rung_rule_without_row`) | `Cosmic_Ascension.UI.Chronicle` |
| `B6a/` | `b6a4_nocandidate_says_rung.patch` — the chronicle's cause line loses its no-candidate branch: the silence reads as a people's fifth rung | `Cosmic_Ascension.UI.Chronicle`, `Cosmic_Ascension.UI.Epitaph` |
| `B6a/` | `b6a4_claim_may_follow_no_nova_claim_gate.patch` — `ClaimMayFollowOf` drops the gate of `Hazard.PreSupernova` and `Hazard.Claim` (their expiries never run the claim rule on that system; no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_claim_may_follow_no_nova_claim_gate`) | `Cosmic_Ascension.UI.ClaimMayFollow` |
| `B6a/` | `b6a4_rebellion_watch_under_standing.patch` — the rebellion watch shows while a rebellion already stands on the capital | `Cosmic_Ascension.UI.RebellionWatch` |
| `B6a/` | `b6a4_rebellion_watch_zero_count.patch` — the rebellion watch shows below the threshold with a count of 0 | `Cosmic_Ascension.UI.RebellionWatch` |
| `B6a/` | `b6a4_rebellion_watch_no_floor.patch` — the rebellion watch's wait ignores the row's first tick (`GalaxyTickAtLeast`) | `Cosmic_Ascension.UI.RebellionWatch` |
| `B6a/` | `b6a4_ruled_people_reads_view.patch` — the ruled people's end is named only when the knowing shows it (no longer applies since [B-6b3a] - taken again as `b6b3a_ruled_people_reads_view`) | `Cosmic_Ascension.UI.Chronicle` |
| `B6a/` | `b6a4_claim_line_any_owner.patch` — the system panel's claim warning stands on any owner's settled system, not only the ruled people's | `Cosmic_Ascension.UI.ClaimMayFollow` |
| `B6a/` | `b6a5_awareness_category_claim.patch` — the awareness line is told as `Log.Claim`: the two books gather it | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_awareness_severity_2.patch` — the awareness line is Severity 2: the automatic pause stops for it | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_awareness_gate_own_flag.patch` — the line's gate is the claimant's own flag, not 'no transcendent aware before': a second fallen's first claim tells a second line | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_awareness_with_sign.patch` — the awareness line is told with a sign (`PushWithSign`): two numbers of the tell stream | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_awareness_before_outbreak.patch` — the awareness line is told before the new claim's outbreak line (it is no longer the expiry's last line; no longer applies since [B-6b2b] - taken again at [B-6b12] as `b6b12_awareness_before_outbreak`) | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_life_chapter_admits_awareness.patch` — both chronicle gates of `Log.Transcendent` go (the life chapter's gather and `Admits`): the awareness line moves into the chronicle and a chapter shows it | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_admits_awareness.patch` — `CosmicChronicle::Admits` takes the awareness line when it leaves the log (a saved entry nothing can show, holding a slot of the chronicle's cap) | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_succession_clears_flags.patch` — a succession clears every `bAwareOfPlayer` (the flag must survive it - B6a decision 7-na) | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_awareness_reads_voice.patch` — the awareness line reads `VoiceTemplates` (the hedge's testimony, empty in the game) instead of `AwarenessTemplates` | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a5_awareness_writes_knowing.patch` — the awareness line's block writes the knowing (`FarLineUntilTick`) | `Cosmic_Ascension.Log.AwarenessVoice` |
| `B6a/` | `b6a6_exalt_as_before_is_the_game.patch` — `CosmicTests::WithExaltAsBefore` hands back the game's tables (the pair's off side would be the on side; the data's own revert is caught by the schema test - `Tools/test_data_schema.py`) | `Cosmic_Ascension.Sim.ExaltArrives` |
| `B6b/` | `b6b1_laws_held_initializer_one.patch` — `LawsHeldTicks` starts at 1 (the frozen initializer is 0 - CLAUDE.md rule 7) | `Cosmic_Ascension.Sim.SaveLawsHeld` |
| `B6b/` | `b6b1_hasher_skips_counters.patch` — the state hasher skips `LawsHeldTicks` and `PressureEnduredTicks` (the two counters are world state) | `Cosmic_Ascension.Sim.HashCoversNewCounters` |
| `B6b/` | `b6b1_move_capital_keeps_endured.patch` — `CosmicHazards::MoveCapital` leaves `PressureEnduredTicks` alone (the count runs on through the loss of the capital) | `Cosmic_Ascension.Sim.PressureEndured` |
| `B6b/` | `b6b1_endured_capital_inside.patch` — a drained tick counts as endured even with the capital inside a fallen sphere (subjected is not enduring) | `Cosmic_Ascension.Sim.PressureEndured` |
| `B6b/` | `b6b1_endured_any_sphere.patch` — any transcendent's sphere over the capital stops the count, not only an unsealed fallen one (an independent sphere is no subjection) | `Cosmic_Ascension.Sim.PressureEndured` |
| `B6b/` | `b6b1_endured_ruled_only.patch` — only the ruled people counts its endured pressure (the count is kept for every people) | `Cosmic_Ascension.Sim.PressureEndured` |
| `B6b/` | `b6b1_endured_fallen_drain_only.patch` — a drained tick counts only when a settled system lies inside a fallen sphere (the drain of any presser is a drain) | `Cosmic_Ascension.Sim.PressureEndured` |
| `B6b/` | `b6b2a_treaty_partners_any_tag.patch` — `CosmicDiplomacy::TreatyPartnersOf` counts a partner of any treaty whatever the Tag asks (a non-aggression is not the trade treaty) | `Cosmic_Ascension.Events.CheckIsPure` |
| `B6b/` | `b6b2a_treaty_strictly_more.patch` — `CivHasTreaty` asks more partners than its Value (the bound itself holds) | `Cosmic_Ascension.Feats.Evaluate` (and `Events.CheckIsPure`) |
| `B6b/` | `b6b2a_laws_held_no_min.patch` — `CivLawsHeldTicksAtLeast` reads `LawsHeldTicks` without the `Min` with `Tick - SinceTick` (the stretch before the vessel's birth counts) | `Cosmic_Ascension.Feats.Evaluate` (and `Events.CheckIsPure`) |
| `B6b/` | `b6b2a_endured_window_inclusive.patch` — the window of `CivPressureEnduredTicksReached` is one tick wider (the next slot finds it too) | `Cosmic_Ascension.Sim.EraSelector` (and `Events.CheckIsPure`) |
| `B6b/` | `b6b2a_ruled_is_context.patch` — the selector word `Ruled` names the context people (an AI people's capital answers for it) | `Cosmic_Ascension.Sim.EraSelector` (and `Events.CheckIsPure`) |
| `B6b/` | `b6b2a_borders_twice.patch` — `ChangeReverence Borders` adds twice to each bordering people | `Cosmic_Ascension.Sim.EraSelector` |
| `B6b/` | `b6b2a_laws_counted_before_upkeep.patch` — `LawsHeldTicks` is counted before the upkeep (a law that lapses in the tick is held at the count) | `Cosmic_Ascension.Sim.LawsHeld` |
| `B6b/` | `b6b2a_laws_held_any_rung.patch` — `CosmicEdicts::HoldsGalaxyLawsOf` without its fourth-rung test (a people of the third rung holding the laws counts) | `Cosmic_Ascension.Sim.LawsHeld` (the third rung) |
| `B6b/` | `b6b2a_hooks_overlap.patch` — the galaxy walk adds a law once for every people that holds it (the union counts each row once) | `Cosmic_Ascension.Sim.GalaxyLaws` |
| `B6b/` | `b6b2a_hooks_owner_only.patch` — the two hooks read the owner's edicts alone (a galaxy law reaches no other people's system) | `Cosmic_Ascension.Sim.GalaxyLaws` |
| `B6b/` | `b6b2a_hooks_any_galaxy.patch` — `CosmicEdicts::HookEdictsOf` without its `GalaxyRow` test (a galaxy law reaches a system of another galaxy) | `Cosmic_Ascension.Sim.GalaxyLaws` (the other galaxy) |
| `B6b/` | `b6b2a_ai_treaty_to_ruled.patch` — the AI's treaty step may take the ruled people as its partner (its consent is the ruler's command) | `Cosmic_Ascension.Sim.AIRowSteps` |
| `B6b/` | `b6b2a_ai_treaty_draws.patch` — the AI's treaty step draws one number from the sim stream | `Cosmic_Ascension.Sim.AIRowSteps` |
| `B6b/` | `b6b2a_ai_law_step_skips_gate.patch` — the AI's law step enacts every law it lacks without the gate `EnactBlockOf` (a refused condition, full slots) | `Cosmic_Ascension.Sim.AIRowSteps` |
| `B6b/` | `b6b2a_ai_steps_unasked.patch` — the AI's treaty step runs with no row that asks (`AsksForTreatyOf` in `TickCivAI` reads true) | `Cosmic_Ascension.Sim.AIRowSteps` (without a row that asks; the plain board's treaty also spoils the asked board's comparisons) |
| `B6b/` | `b6b2a_ai_treaty_skips_gate.patch` — `CosmicDiplomacy::AITreatyPartnerOf` without the gate `TreatyBlockOf` (a hostile partner is not passed by) | `Cosmic_Ascension.Sim.AIRowSteps` (the hostile case; the second treaty of the next tick too - the partner the first tick took is taken again) |
| `B6b/` | `b6b2a_ai_revokes_laws.patch` — `CosmicEdicts::AIRevokesOf` walks the galaxy laws too | `Cosmic_Ascension.Sim.EdictAI` |
| `B6b/` | `b6b2a_policy_step1_drops_law.patch` — the policy's step (1) revokes a galaxy law with a Stability cost below 0.5 | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B6b/` | `b6b2a_policy_law_first.patch` — the policy's room making gives up a galaxy law first (laws come last) | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B6b/` | `b6b2a_policy_law_before_edict_row.patch` — the policy's room making gives up a galaxy law before `EdictRow` (the ranks of the two swapped) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (the kept row's case) |
| `B6b/` | `b6b2a_policy_law_for_law.patch` — the policy's law step gives up a law for a law (the laws turn over) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (the first law case, its second boundary) |
| `B6b/` | `b6b2a_policy_law_step_ignores_answer.patch` — the policy's law step no longer stands still while an answer edict holds a slot (a law takes the free slot) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (the free slot's case) |
| `B6b/` | `b6b2a_policy_vigil_for_law.patch` — the policy's law step takes the vigil's slot at any boundary, while step (3) would enact it again too (the vigil and a law swap every two boundaries) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (the vigil's case, its two boundaries) |
| `B6b/` | `b6b2a_policy_vigil_band.patch` — the policy's law step keeps off the vigil's slot only while step (3) would enact it again, not below Stability 0.5 (under a steady drain the vigil and a law swap each time Stability climbs back to 0.3) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (the vigil's case, the boundary at 0.45) |
| `B6b/` | `b6b2a_policy_vigil_waits_for_rest.patch` — the policy's law step keeps off the vigil's slot below `RestAt` (0.6) instead of 0.5, so the law waits for step (4) to let the vigil go (the stall the verdict measured) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (the vigil's case, the boundary at 0.55) |
| `B6b/` | `b6b2a_policy_law_step_ignores_slots.patch` — the policy's law step enacts while the slots are fewer than `GalaxyLawsToHold` | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B6b/` | `b6b2a_policy_law_step_free_slot_only.patch` — the policy's room making gives a law no room once the slots are full (a law takes free slots only) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (the first law case) |
| `B6b/` | `b6b2a_policy_law_step_unasked.patch` — the policy's law step (4') runs with no row that asks for laws | `Cosmic_Ascension.Sim.RulerEdictPolicy` (without a row that reads the laws; the third rung's case too) |
| `B6b/` | `b6b2a_asks_for_laws_any_rung.patch` — `CosmicEdicts::AsksForLawsOf` without its rung check (a row of the third rung that reads the laws asks for them) | `Cosmic_Ascension.Sim.RulerEdictPolicy` (below the fourth rung - run over the whole suite, the one test of the 214 it breaks: no other board puts a row that reads the laws below the fourth rung) |
| `B6b/` | `b6b2a_data_laws_knob_four.patch` — `DT_Rules.GalaxyLawsToHold` 4, above the edict slots' end (a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` |
| `B6b/` | `b6b2a_data_reach_word_renamed.patch` — the schema's word list `reverence_reach` says `Border` (a data patch of `schema.json`: `make_data_tables.py` stops at the checker's own tests, `Tools/test_data_schema.py` - caught there as well - having taken the stamp `generated.sha1` away, so run it again after the revert; `Data.Integrity` reads the file itself) | `Cosmic_Ascension.Data.Integrity` |
| `B6b/` | `b6b2b_gate_after_loss.patch` — the extinction no longer asks the last-system gate before its loss; the grace is asked after `LetPeopleGo` (where no people stands under the system any more, so it never keeps anything) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the grace case) |
| `B6b/` | `b6b2b_gate_last_system_only.patch` — the extinction asks the gate of its people's last settled system only (a people of two settled systems is not kept) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the grace case) |
| `B6b/` | `b6b2b_kill_subject_system.patch` — the extinction's `KillPlayer` carries the system as its subject (it reaches a wanderer standing there, not the ruler elsewhere) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_no_kill.patch` — the extinction's code records no `KillPlayer` (the ruler is deposed, not dead) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_depose_before_death.patch` — the extinction records `DeposeRuler` before `KillPlayer` (first recorded wins: the death is lost) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_no_depose.patch` — the extinction records no `DeposeRuler` after the death (a death the birth protection hid leaves the ruler on an empty throne) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the birth protection) |
| `B6b/` | `b6b2b_extra_draw.patch` — the extinction's code draws one number of the sim stream | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the AI people's two draws) |
| `B6b/` | `b6b2b_record_before_expiry.patch` — the record line `log_record_extinct` is told inside the code of the tag, before the expiry line | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss; the AI people) |
| `B6b/` | `b6b2b_stage_clock_system_subject.patch` — the stage clock asks `VesselAtRiskOf` with the system for the extinction too (the ruler elsewhere reads as out of reach) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_lets_go_drops_records.patch` — `CosmicHazards::LetPeopleGo` empties the completion records of the systems it lets go | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_lets_go_keeps_project.patch` — `LetPeopleGo` leaves the construction slot (`ActiveMegaproject`, its progress) as it was | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_lets_go_keeps_hazards.patch` — `LetPeopleGo` leaves the hazard and response slots (the later plague's expiry runs on a system no one owns) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_lets_go_keeps_targets.patch` — `LetPeopleGo` lifts no fleet target | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the window called alone, case (2'): within a tick the AI stage lets go of a target on a system no one owns, so the first run of this patch survived the tick's case and the test gained the direct call) |
| `B6b/` | `b6b2b_lets_go_no_stamp.patch` — `LetPeopleGo` leaves a completion record of no people unstamped (#73) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the loss) |
| `B6b/` | `b6b2b_claim_may_follow_asks_rule.patch` — `CosmicHazards::ClaimMayFollowOf` asks the claim rule of the slot for the extinction too | `Cosmic_Ascension.UI.ClaimMayFollow` (case (h)) |
| `B6b/` | `b6b2b_kills_ruler_other_tag.patch` — `CosmicHazards::KillsTheRulerOf` is true for `Hazard.Plague` as well | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the one answer); from [B-6b6] `Cosmic_Ascension.Data.Integrity` too (the code's one death: two rows kill the ruler) |
| `B6b/` | `b6b2b_death_row_hazard.patch` — `DeathRowOf` tells every `Death.Hazard` by `log_death_hazard` (the extinction's own line is never chosen) | `Cosmic_Ascension.Calling.DeathPipeline` (the extinction's case) |
| `B6b/` | `b6b2b_grace_keeps_one_slot.patch` — a grace that keeps a people from its extinction empties that one slot only (a second extinction of the people expiring in the same pass takes it right after the grace line) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the two extinctions, case (1'); the AI people's grace, case (4')) |
| `B6b/` | `b6b2b_gate_ruled_only.patch` — only the extinction of the people the vessel rules asks the last-system gate (the alternative decision 5 did not take: an AI people is not kept) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the AI people's grace, case (4')) |
| `B6b/` | `b6b2b_kill_wanderer_too.patch` — the extinction's code also applies `KillPlayer` on the system before the systemless one (a wanderer standing there dies) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the wanderer, case (4'')) |
| `B6b/` | `b6b2b_lets_go_keeps_rebel.patch` — `LetPeopleGo` leaves a rebellion's leader record open and the count of discontent as it was | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the window called alone, case (2')) |
| `B6b/` | `b6b2b_lets_go_settled_only.patch` — `LetPeopleGo` lets only the settled systems go (a settlement under way stays: the shielded ruler is not deposed and keeps an empty people) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the birth protection, case (3)) |
| `B6b/` | `b6b2b_grace_ends_every_extinction.patch` — a grace that keeps a people from its extinction ends every extinction of the run, not its people's alone (the ruled people's extinction standing on another system ends without result) | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the AI people's grace, case (4')) |
| `B6b/` | `b6b2b_schema_rows_of_always.patch` — the schema test `rows_of` holds for any list (a schema patch: no build, run `python -I Tools/test_data_schema.py`) | `test_rows_of_and_the_extinction_row` and `test_every_rule_of_the_schema_can_fail` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b2b_schema_rows_of_any_table.patch` — the checker no longer reports a `rows_of` that names a table its column does not name (a schema patch) | `test_rows_of_and_the_extinction_row` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b2b_schema_extinction_rule_dropped.patch` — the extinction's rule of `schema.json` loses its `rows_of` test (an edict may answer it - a data patch of `schema.json`, no build) | `test_every_rule_of_the_schema_can_fail` and `test_rows_of_and_the_extinction_row` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b2b_schema_death_extinction_rule_dropped.patch` — the rule of `log_death_extinction` loses its `death_frame_words` test (a data patch of `schema.json`, no build; regenerated at [B-6b2c] - the rule of `log_fallen_bound` follows it; regenerated at [B-6b5] - the rule's new stay and desolation tests follow it) | `test_every_rule_of_the_schema_can_fail` and `test_death_and_record_words` - the death rows' loop (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b2b_data_death_line_frame.patch` — the first variant of `log_death_extinction` says '그들을 위해' (a data patch: `Tools/check_data.py` refuses it before an asset is written; regenerated at [B-6b5] on the rewritten first variant) | `python -I Tools/check_data.py` (the death line's rule of `log_death_extinction`) |
| `B6b/` | `b6b2b_data_record_category.patch` — `log_record_extinct` in `Log.Civ` (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the record row's rule) |
| `B6b/` | `b6b2b_data_plague_as_extinction.patch` — `crisis_plague` tagged `Hazard.Extinction`, keeping its expiry effects (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the extinction's rule: `OnExpireEffects` `max_len` 0) |
| `B6b/` | `b6b2c_delay_second_rung.patch` — the delay above the third rung also slows an AI people's construction of the second rung | `Cosmic_Ascension.Sim.UpperRungDelay` (the second rung's row) |
| `B6b/` | `b6b2c_delay_on_response.patch` — the delay above the third rung slows the response slot too (an ark of the third rung) | `Cosmic_Ascension.Sim.ArkAnswers` (the ark's progress, then its completion) |
| `B6b/` | `b6b2c_delay_bad_knob_as_is.patch` — `AIUpperRungProgressMultiplier` not above 0, or not a number, is read as it is (its error line kept) | `Cosmic_Ascension.Sim.UpperRungDelay` (the knobs 0 and NaN) |
| `B6b/` | `b6b2c_brake_skips_ruled.patch` — `CosmicAscent::FourthRungOpenOf` leaves the ruled people out of the count | `Cosmic_Ascension.Sim.FourthRungBrake` (case (1): the next people is not held) |
| `B6b/` | `b6b2c_brake_order_reversed.patch` — the judgment walks the peoples the player does not rule from the end of `Civs` | `Cosmic_Ascension.Sim.FourthRungBrake` (case (1): the people first in Civs climbs) |
| `B6b/` | `b6b2c_brake_cap_zero_brakes.patch` — a cap of 0 is a brake (nothing climbs to the fourth rung) | `Cosmic_Ascension.Sim.FourthRungBrake` (case (1)'s free board and case (3)) |
| `B6b/` | `b6b2c_brake_missing_row_closes.patch` — a galaxy row the tables do not hold closes the fourth rung | `Cosmic_Ascension.Sim.FourthRungBrake` (case (3), no galaxy table) |
| `B6b/` | `b6b2c_brake_counts_third_rung.patch` — the count takes the third rung too (the asking people counts itself) | `Cosmic_Ascension.Sim.FourthRungBrake` (case (4): the held people climbs on the tick after the count drops; case (1) too) |
| `B6b/` | `b6b2c_brake_counts_landed_transits.patch` — the count takes every crossing bound for the galaxy, a landed one too | `Cosmic_Ascension.Sim.FourthRungBrake` (case (5): landed, no longer counted) |
| `B6b/` | `b6b2c_brake_counts_every_galaxy.patch` — the count takes the peoples of every galaxy on the fourth rung (not only its galaxy's) | `Cosmic_Ascension.Sim.FourthRungBrake` (case (6): another galaxy's people is not counted) |
| `B6b/` | `b6b2c_brake_counts_every_crossing.patch` — the count takes the crossings bound for any galaxy (not only for its galaxy) | `Cosmic_Ascension.Sim.FourthRungBrake` (case (6): a crossing bound elsewhere is not counted) |
| `B6b/` | `b6b2c_brake_counted_once_per_tick.patch` — the count is read once a tick (a climb earlier in the tick is not counted) | `Cosmic_Ascension.Sim.FourthRungBrake` (case (1): the next is held, closed at the cap) |
| `B6b/` | `b6b2c_brake_ruled_too.patch` — the brake holds the people the player rules too | `Cosmic_Ascension.Sim.FourthRungBrake` (case (2): the ruled people) |
| `B6b/` | `b6b2c_brake_fifth_too.patch` — the brake is asked from the third rung up (the climb to the fifth is held) | `Cosmic_Ascension.Sim.FourthRungBrake` (case (2): the fifth rung) |
| `B6b/` | `b6b2c_rebind_moves_sealed_tick.patch` — `CosmicSeal::BindTargetOf` takes a sealed fallen one and `Bind` writes over its seal (a second binding moves `SealedTick`) | `Cosmic_Ascension.Sim.Binding` (cases (1), (3) - the seal never moved - and (4)) |
| `B6b/` | `b6b2c_bind_target_any_realm.patch` — `BindTargetOf` takes an instance of any realm (the independent one first) | `Cosmic_Ascension.Sim.Binding` (case (1)) |
| `B6b/` | `b6b2c_bind_target_last_first.patch` — `CosmicSeal::BindTargetOf` walks `Transcendents` from the end (the last unsealed fallen one) | `Cosmic_Ascension.Sim.Binding` (case (1): of two unsealed fallen ones, the first) |
| `B6b/` | `b6b2c_binding_does_not_bind.patch` — the binding's completion seals nothing (its record and lines stay) | `Cosmic_Ascension.Sim.Binding` (case (2): the seal, its tick, the claim after it) |
| `B6b/` | `b6b2c_binding_draws.patch` — the binding's completion draws one number of the sim stream | `Cosmic_Ascension.Sim.Binding` (case (2): the draws of the twin tick) |
| `B6b/` | `b6b2c_target_once_per_pass.patch` — `Advance` reads the binding's target once a tick (the second binding of a pass binds a target already bound) | `Cosmic_Ascension.Sim.Binding` (case (4): the second ends without result) |
| `B6b/` | `b6b2c_bound_line_first.patch` — the binding's line is told before the completion line | `Cosmic_Ascension.Sim.Binding` (case (2): right after it) |
| `B6b/` | `b6b2c_binding_draws_tell.patch` — the binding's line draws a second number of the tell stream | `Cosmic_Ascension.Sim.Binding` (case (2): one number more than the twin's completion) |
| `B6b/` | `b6b2c_bound_line_severity_zero.patch` — the binding's line is told with Severity 0 (not the completion line's) | `Cosmic_Ascension.Sim.Binding` (case (2): the ruled people's binding, Severity 1) |
| `B6b/` | `b6b2c_ended_binding_draws_tell.patch` — a binding that ends without result draws a number of the tell stream (no line told) | `Cosmic_Ascension.Sim.Binding` (case (3): as the same tick without the binding) |
| `B6b/` | `b6b2c_binding_read_as_sealed.patch` — the evaluator reads `MegaprojectComplete project_binding` as `TranscendentSealed` (any sealed fallen one since `SinceTick`) | `Cosmic_Ascension.Feats.Evaluate` (the binding's record: another people does not climb) |
| `B6b/` | `b6b2c_nofallen_before_rowtier.patch` — `StartBlockOf` asks `NoFallen` before `RowTier` | `Cosmic_Ascension.Sim.StartBlockReasons` (below the row's rung it is RowTier) |
| `B6b/` | `b6b2c_nofallen_after_slot.patch` — `StartBlockOf` asks `NoFallen` after the slots (a running binding with no target reads Running) | `Cosmic_Ascension.Sim.StartBlockReasons` (the fallen one before the slot) |
| `B6b/` | `b6b2c_build_button_any_rung.patch` — the panel's construction buttons show at any rung again (a first-rung home greys the third rung's row) | `Cosmic_Ascension.UI.StartReasons` (the first-rung home; `UI.TopBarAndPanel` too) |
| `B6b/` | `b6b2c_build_button_filters_answers.patch` — the panel's rung filter takes the answers too (the third rung's answer is gone from a first-rung home under its hazard) | `Cosmic_Ascension.UI.StartReasons` (section 6: the answer two rungs up, greyed for the rung) |
| `B6b/` | `b6b2c_guard_cap_ignored.patch` — `CosmicConflict::FleetDelayOf` leaves out the row's cap `GuardDelayCapOf` | `Cosmic_Ascension.Sim.FleetDelay` (the share 0.25: 50, not 100) |
| `B6b/` | `b6b2c_guard_cap_no_fleet_cap.patch` — `CosmicConflict::GuardDelayCapOf` leaves out FleetDelayCap: the share's product alone | `Cosmic_Ascension.Sim.FleetDelay` (the share 0.5 of 300: 100, not 150; the share 1); from [B-6b8b] on also `Cosmic_Ascension.Data.Integrity` (game theodicy: under the game's 0.5 the claim's cap is 150, its longest stand 600) |
| `B6b/` | `b6b2c_data_ark_margin_half.patch` — `DT_Rules.AIAscensionSpeedMultiplier` 0.5 (a data patch: rebuild the assets; no longer applies since [B-6b7] - taken again as `b6b7_data_ark_margin_half`) - the design's ark of 150 takes an AI people 300 ticks of the extinction's 250 | `Cosmic_Ascension.Data.Integrity` (the ark's margin under the game's rules; from [B-6b6] on the game's own extinction and ark as well) |
| `B6b/` | `b6b2c_data_binding_answers.patch` — `crisis_plague` lists `project_binding` as an answer (a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` (no hazard answers with the binding) |
| `B6b/` | `b6b2c_data_binding_causality.patch` — `project_binding` asks Causality 1 a tick (a data patch, refused before an asset is written; regenerated at [B-6b5] - its context is the renamed `DisplayName` '항성 기관') | `python -I Tools/check_data.py` (rule 7: `CostPerTick.Causality` is 0) |
| `B6b/` | `b6b2c_data_bound_line_names_people.patch` — the first variant of `log_fallen_bound` names `{Civ}` (a data patch, refused before an asset is written; regenerated at [B-6b5] on the rewritten first variant) | `python -I Tools/check_data.py` (rule 8: `{System}` alone) |
| `B6b/` | `b6b2c_schema_causality_rule_dropped.patch` — rule 7 of `schema.json` asks Causality `ge` 0 instead of `is` 0 (a schema patch, no build; regenerated at [B-6b5] - the binding's text rule follows it) | `test_the_binding_rows` (`Tools/test_data_schema.py` - the breaking case Causality 1) |
| `B6b/` | `b6b2c_schema_bound_line_rule_dropped.patch` — the rule of `log_fallen_bound` takes `{Civ}`, `{Project}`, `{OtherCiv}` and `{Transcendent}` (a schema patch; regenerated at [B-6b3a] - the nameless lines' rules follow it; regenerated at [B-6b5] - the rule's triumph and dawn tests follow it) | `test_the_binding_rows` (`Tools/test_data_schema.py` - each slot) |
| `B6b/` | `b6b2c_schema_pool_member_any.patch` — the checker takes any member name of a pool (`CostPerTick.Faith`) | `test_the_binding_rows` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b2c_schema_pool_member_as_struct.patch` — the checker reads a pool member path as a list of structs (every rule over a pool member fails every row) | `test_the_binding_rows` and every test that checks the project's rows (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b2c_schema_share_range_dropped.patch` — `FleetDelayCountdownShare` has no range in `schema.json` (a schema patch, no build) | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py` - below 0 and above 1) |
| `B6b/` | `b6b3a_far_above_at_two.patch` — `CosmicKnowing::FarAboveRungs` 2 - the gap rule's one predicate hides a people two rungs up | `Cosmic_Ascension.Knowing.FarAbove` (the copies' answer on every pair; the readers at gap 2) |
| `B6b/` | `b6b3a_tell_no_gap.patch` — `UCosmicLogSubsystem::Tell` drops the gap: a met people's line is stored however far above the viewer | `Cosmic_Ascension.Knowing.FarAbove` (Tell at gap 3 and 4) |
| `B6b/` | `b6b3a_notemet_no_gap.patch` — `NoteMet` drops the gap: a people far above the viewer is met | `Cosmic_Ascension.Knowing.FarAbove` (NoteMet and Knowing at gap 3 and 4) |
| `B6b/` | `b6b3a_shown_no_gap.patch` — `CosmicMapLook::IsShownPeople` drops the gap: every met people is shown | `Cosmic_Ascension.Knowing.FarAbove` (IsShownPeople at gap 3 and 4) |
| `B6b/` | `b6b3a_wanderer_gap_two.patch` — `CosmicPetitions::WithinWandererGap` lets a wanderer's card come two rungs up (the same mutation as `B1/b1b_present_without_gap`, which no longer applies since [B-6b3a]) | `Cosmic_Ascension.Knowing.FarAbove` (the card's own threshold) |
| `B6b/` | `b6b3a_crown_gate_removed.patch` — `CosmicWanderer::CrownBlockOf` asks no gap: a vessel is crowned by a people far above it | `Cosmic_Ascension.Sim.CrownFarAbove` (three and four rungs up; the command; the return) |
| `B6b/` | `b6b3a_crown_return_exempt.patch` — the gap is asked of a coronation only: the vessel returns to a people far above it | `Cosmic_Ascension.Sim.CrownFarAbove` (the return three rungs up, by the gate and by the command) |
| `B6b/` | `b6b3a_crown_from_first_rung.patch` — the gap is counted from the first rung, not from the calling's PersonalTier | `Cosmic_Ascension.Sim.CrownFarAbove` (from the second rung the fourth is not far) |
| `B6b/` | `b6b3a_crown_far_before_unmet.patch` — `FarAbove` is asked before `Unmet` | `Cosmic_Ascension.Sim.CrownFarAbove` (a people far above and not met says Unmet first) |
| `B6b/` | `b6b3a_siege_start_named.patch` — the AI's siege on the ruled people is told by `log_siege_on_us` however far above the besieger stands | `Cosmic_Ascension.Log.FarNameless` (the siege's opening, met or not) |
| `B6b/` | `b6b3a_treaty_broken_named.patch` — the trade treaty the AI's opening broke is told by `log_treaty_broken` however far above | `Cosmic_Ascension.Log.FarNameless` (the treaty's line of the opening) |
| `B6b/` | `b6b3a_siege_end_named.patch` — `TellSiege` tells the named end rows however far above the other people stands | `Cosmic_Ascension.Log.FarNameless` (taken, held and lifted on the ruled side) |
| `B6b/` | `b6b3a_lapse_named.patch` — a friendship of the ruled people that lapses is told by `log_stance_lapsed` however far above the friend stands | `Cosmic_Ascension.Log.FarNameless` (the lapse by its upkeep) |
| `B6b/` | `b6b3a_far_reads_notemet.patch` — the AI's opening chooses the nameless rows by NoteMet's answer (a people already met two rungs up is told nameless) | `Cosmic_Ascension.Log.FarNameless` (two rungs up, already met: the named rows) |
| `B6b/` | `b6b3a_far_any_gap.patch` — `ToldRowOf` takes the nameless row whatever the gap | `Cosmic_Ascension.Log.FarNameless` (two rungs up: the named rows) |
| `B6b/` | `b6b3a_lapse_far_in_chronicle.patch` — `CosmicChronicle::Admits` takes the nameless lapse `log_stance_lapsed_far` into the chronicle | `Cosmic_Ascension.Log.FarNameless` (out of the chronicle as the named one) |
| `B6b/` | `b6b3a_chronicle_ruled_only.patch` — the cause line's 'always named' is the ruled people's end alone, not `CosmicChronicle::PeoplesOfEveryLife` | `Cosmic_Ascension.UI.Chronicle` (a people the vessel ruled, at the fifth rung far above the viewer) |
| `B6b/` | `b6b3a_chronicle_names_unshown_people.patch` — the cause line names another people at the fifth rung even when the viewer is not shown it (rule 14 - `b6a4_chronicle_names_unshown_people` taken again on the widened line) | `Cosmic_Ascension.UI.Chronicle` (another people far above; never met) |
| `B6b/` | `b6b3a_panel_far_as_unmet.patch` — the panel's filter of a met people far above is dropped and `CrownKeyOf` gives `FarAbove` the words of `Unmet`: a greyed [Crown] stands for a people the map hides (regenerated by the review - with the filter, `CrownKeyOf` alone is never asked `FarAbove`) | `Cosmic_Ascension.UI.WandererPanel` (a people far above: [Scan] alone) |
| `B6b/` | `b6b3a_data_far_line_names_people.patch` — the first variant of `log_siege_on_us_far` names `{OtherCiv}` (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (rule 8: no `{OtherCiv}` in a nameless variant) |
| `B6b/` | `b6b3a_schema_far_rule_takes_other.patch` — the rule of `log_stance_lapsed_far` takes `{OtherCiv}` (a schema patch, no build) | `test_the_nameless_lines` (`Tools/test_data_schema.py` - the row given `{OtherCiv}`) |
| `B6b/` | `b6b3a_panel_far_under_way.patch` — the panel drops its filter of a met people far above: under way from it the gate's `Travelling` greys a [Return] | `Cosmic_Ascension.UI.WandererPanel` (under way and with a card waiting: [Scan] alone) |
| `B6b/` | `b6b3a_far_choice_draws_sim.patch` — the AI's opening draws one sim number when the besieger stands far above | `Cosmic_Ascension.Log.FarNameless` (as many sim numbers as the same tick two rungs up) |
| `B6b/` | `b6b3a_far_choice_draws_tell.patch` — the AI's opening draws one more tell number when the besieger stands far above | `Cosmic_Ascension.Log.FarNameless` (as many tell numbers as the same tick two rungs up) |
| `B6b/` | `b6b3a_far_reads_metcivs.patch` — the AI's opening chooses the nameless rows by Knowing (a people met before it rose is told by name) | `Cosmic_Ascension.Log.FarNameless` (met before it rose: the nameless rows) |
| `B6b/` | `b6b3a_treaty_broken_far_severity.patch` — the nameless broken treaty's line is told at Severity 2 | `Cosmic_Ascension.Log.FarNameless` (each line keeps the named row's Severity) |
| `B6b/` | `b6b3a_data_far_line_shares_text.patch` — the second variant of `log_siege_on_us_far` is the first of `log_siege_failed` (a data patch; the assets are rebuilt) | `Cosmic_Ascension.Log.FarNameless` (no template shared with the eleven named lines) |
| `B6b/` | `b6b3a_chronicle_this_life_only.patch` — the cause line's 'always named' is the present life's ruled peoples alone | `Cosmic_Ascension.UI.Chronicle` (an earlier life's people under the next life; a birth people never ruled) |
| `B6b/` | `b6b3a_ruled_people_reads_view.patch` — the ruled people's end is named only when the knowing shows it (`b6a4_ruled_people_reads_view` taken again on the widened line - both exceptions dropped; the ruled people's end alone dropped is the same answer, every ruled people is a people of a life) | `Cosmic_Ascension.UI.Chronicle` (the ruled people far above the vessel gone from its throne) |
| `B6b/` | `b6b3a_chronicle_keeps_stance.patch` — the chronicle admits the stance lines, the nameless lapse too (`b3b_chronicle_keeps_stance` taken again on the widened line) | `Cosmic_Ascension.Log.ConflictLines` |
| `B6b/` | `b6b3a_panel_far_skips_meeting.patch` — the panel's filter asks the rung without the meeting: a people far above and not met loses its greyed [Crown], and the missing button tells a rung the map hides | `Cosmic_Ascension.UI.WandererPanel` (far above, not met: the greyed Unmet, and Travelling under way, of any people not met) |
| `B6b/` | `b6b3a_far_choice_draws_sim_siege_end.patch` — `TellSiege` draws one sim number when the other people stands far above the ruled one | `Cosmic_Ascension.Log.FarNameless` (the loss, the hold and the lifting: as many sim numbers as the same tick two rungs up) |
| `B6b/` | `b6b3a_far_choice_draws_sim_lapse.patch` — a lapsing friendship of the ruled people draws one sim number when the friend stands far above | `Cosmic_Ascension.Log.FarNameless` (the lapse: as many sim numbers as the same tick two rungs up) |
| `B6b/` | `b6b3b_race_check_ruled_not_counted.patch` — the race check (`CosmicRaceMeasure::FourthRungCheckOf`) leaves the ruled people's climb out of the climbed count | `Cosmic_Ascension.Sim.RaceMeasure` (the ruled people's climb is counted: 1 + 2 > 2 fails) |
| `B6b/` | `b6b3b_race_check_asked_on_ruled.patch` — the race check is asked on the ruled people's climb too | `Cosmic_Ascension.Sim.RaceMeasure` (only the ruled people climbed past the cap: not asked) |
| `B6b/` | `b6b3b_race_check_strict.patch` — the race check holds only strictly below the cap | `Cosmic_Ascension.Sim.RaceMeasure` (one on the fourth rung and one AI climb at cap 2 holds) |
| `B6b/` | `b6b3b_race_check_cap_zero_asked.patch` — the race check is asked under a cap of 0 | `Cosmic_Ascension.Sim.RaceMeasure` (a cap of 0 is no brake) |
| `B6b/` | `b6b3b_race_check_before_ignored.patch` — the race check drops the count before the tick | `Cosmic_Ascension.Sim.RaceMeasure` (two on the fourth rung or above and one AI climb fail) |
| `B6b/` | `b6b3b_race_check_before_skips_ruled.patch` — the race check leaves the ruled people standing on the fourth rung out of the count before the tick | `Cosmic_Ascension.Sim.RaceMeasure` (the ruled people standing there is counted: 2 + 1 > 2 fails) |
| `B6b/` | `b6b3b_race_check_asked_on_fifth_climb.patch` — the race check is asked on an AI climb from the fourth rung to the fifth | `Cosmic_Ascension.Sim.RaceMeasure` (a climb from the fourth to the fifth is not asked) |
| `B6b/` | `b6b3b_fourth_count_skips_ruled.patch` — `FourthRungCountOf` skips the people the player rules | `Cosmic_Ascension.Sim.RaceMeasure` (it counts the ruled people; the pin to `FourthRungOpenOf` at 2) |
| `B6b/` | `b6b3b_fifth_rung_keeps_clock.patch` — `FifthRungEndOf` passes the clock's end | `Cosmic_Ascension.Sim.RaceMeasure` (the clock alone is no fifth-rung end) |
| `B6b/` | `b6b3b_fifth_rung_takes_silence.patch` — `FifthRungEndOf` passes the silence | `Cosmic_Ascension.Sim.RaceMeasure` (the silence alone is no fifth-rung end) |
| `B6b/` | `b6b3b_fifth_rung_drops_other.patch` — `FifthRungEndOf` drops another people's end | `Cosmic_Ascension.Sim.RaceMeasure` (an AI people on the fifth rung; a fifth rung in the silence) |
| `B6b/` | `b6b3b_fifth_rung_drops_ruled.patch` — `FifthRungEndOf` drops the ruled people's end | `Cosmic_Ascension.Sim.RaceMeasure` (a life's peak on the fifth rung) |
| `B6b/` | `b6b3b_claim_end_eased_first.patch` — `ClaimEndOf` puts eased before joined | `Cosmic_Ascension.Sim.RaceMeasure` (joined and eased: joined) |
| `B6b/` | `b6b3b_claim_end_answered_before_eased.patch` — `ClaimEndOf` puts the answer (relief) before eased | `Cosmic_Ascension.Sim.RaceMeasure` (eased and answered: eased) |
| `B6b/` | `b6b3b_claim_end_expired_before_relief.patch` — `ClaimEndOf` puts expired before relief | `Cosmic_Ascension.Sim.RaceMeasure` (answered and expired: relief) |
| `B6b/` | `b6b3b_claim_end_drops_expired.patch` — `ClaimEndOf` never answers expired | `Cosmic_Ascension.Sim.RaceMeasure` (expired alone: expired) |
| `B6b/` | `b6b3b_claim_end_default_expired.patch` — `ClaimEndOf` answers expired when none of the four holds | `Cosmic_Ascension.Sim.RaceMeasure` (none of the four: another end) |
| `B6b/` | `b6b3b_claim_end_names_swapped.patch` — `ClaimEndNameOf` swaps 'input' and 'standing' | `Cosmic_Ascension.Sim.RaceMeasure` (the words of the ends) |
| `B6b/` | `b6b3b_laws_as_placed_keeps_new.patch` — `LawsAsPlacedOf` keeps a law a people enacted in the tick | `Cosmic_Ascension.Sim.RaceMeasure` (A: own, x; the bonus before the tick on every system) |
| `B6b/` | `b6b3b_laws_as_placed_drops_gone.patch` — `LawsAsPlacedOf` does not bring back a people gone in the tick | `Cosmic_Ascension.Sim.RaceMeasure` (every people after the tick, and the one gone appended; the bonus) |
| `B6b/` | `b6b3b_laws_as_placed_keeps_arrival.patch` — `LawsAsPlacedOf` keeps an arrival's laws | `Cosmic_Ascension.Sim.RaceMeasure` (an arrival's laws are dropped) |
| `B6b/` | `b6b3b_laws_as_placed_true_unchanged.patch` — `LawsAsPlacedOf` answers true and writes Out for a tick that changed no law | `Cosmic_Ascension.Sim.RaceMeasure` (a tick that changed no law answers false) |
| `B6b/` | `b6b3b_fourth_cap_every_row.patch` — `CosmicTests::GalaxiesWithCap` sets the cap of every galaxy row | `Cosmic_Ascension.Sim.RaceMeasure` (every other row is equal) |
| `B6b/` | `b6b3b_fourth_cap_home_row_only.patch` — `CosmicTests::GalaxiesWithCap` copies the home row alone | `Cosmic_Ascension.Sim.RaceMeasure` (no row is left out) |
| `B6b/` | `b6b3b_fourth_cap_home_row_blank.patch` — `CosmicTests::GalaxiesWithCap` blanks the home row but its cap | `Cosmic_Ascension.Sim.RaceMeasure` (every other field of the home row is the game's) |
| `B6b/` | `b6b3b_brake_held_ignores_brake.patch` — `CosmicRaceMeasure::BrakeHeldOf` does not ask the brake's window (`FourthRungOpenOf`) | `Cosmic_Ascension.Sim.FourthRungBrake` (under a cap of 0 a people of the third with its deed met is not held) |
| `B6b/` | `b6b3b_brake_held_ignores_deed.patch` — `BrakeHeldOf` does not ask the deed (every judged row holds) | `Cosmic_Ascension.Sim.FourthRungBrake` (a people the brake holds whose deed is unmet is not counted) |
| `B6b/` | `b6b3b_brake_held_counts_ruled.patch` — `BrakeHeldOf` counts the people the player rules | `Cosmic_Ascension.Sim.FourthRungBrake` (the people the player rules is never held - case (2)'s board before its tick) |
| `B6b/` | `b6b3b_brake_held_any_row.patch` — `BrakeHeldOf` asks every feat row, not only the rows judged at the third rung | `Cosmic_Ascension.Sim.FourthRungBrake` (a row of another rung whose deed is met is not asked) |
| `B6b/` | `b6b3b_brake_held_unsettled.patch` — `BrakeHeldOf` asks the systems being settled too (no `CosmicState::IsSettled`) | `Cosmic_Ascension.Sim.FourthRungBrake` (a people whose every system is being settled is not held) |
| `B6b/` | `b6b3b_pressure_tiers_three.patch` — the ruler policy's pressure cards by rung clamped back to B4's three bins (the third and up) | `Cosmic_Ascension.Sim.RulerPressurePolicy` (a card at the fourth rung in the fourth bin, not the third) |
| `B6b/` | `b6b3b_wanderer_rows_count_withdrawn.patch` — the wanderer policy counts a withdrawn answer in `AnswersByRow` too | `Cosmic_Ascension.Sim.WandererPolicy` (a withdrawn answer is counted by its row apart from the answers) |
| `B6b/` | `b6b3b_wanderer_cards_by_row_off.patch` — the wanderer policy counts no card by its row (both places) | `Cosmic_Ascension.Sim.WandererPolicy` (a card that came is counted by its row) |
| `B6b/` | `b6b3b_wanderer_chain_card_not_by_row.patch` — the wanderer policy counts the card a chain opens (`NextEncounter`) in `Cards` but not by its row | `Cosmic_Ascension.Sim.WandererPolicy` (a card a chain opens is counted by its row) |
| `B6b/` | `b6b3b_without_b6b_keeps_binding.patch` — `CosmicTests::WithoutB6bData` keeps `project_binding` | `Cosmic_Ascension.Data.Integrity` (today's data - from [B-6b6] its four rows; the ten on the full copy) |
| `B6b/` | `b6b3b_without_b6b_keeps_extinction.patch` — `WithoutB6bData` keeps the extinction row | `Cosmic_Ascension.Data.Integrity` (the ten taken out; no hazard kills the ruler, either order; from [B-6b6] today's four rows too) |
| `B6b/` | `b6b3b_without_b6b_keeps_knobs.patch` — `WithoutB6bData` leaves B6b's two knobs as given | `Cosmic_Ascension.Data.Integrity` (its knobs are off) |
| `B6b/` | `b6b3b_without_b6b_drops_log_rows.patch` — `WithoutB6bData` takes B6b's nine log rows out too | `Cosmic_Ascension.Data.Integrity` (B6b's log rows stay; the other tables left alone) |
| `B6b/` | `b6b3b_without_b6b_default_rules.patch` — `WithoutB6bData` starts its rules from the C++ defaults | `Cosmic_Ascension.Data.Integrity` (every other field of the rules is the copy's) |
| `B6b/` | `b6b3b_without_b6b_drops_other_rows.patch` — `WithoutB6bData` takes `petition_exalt` out with the witness | `Cosmic_Ascension.Data.Integrity` (every other row of the six tables kept; the ten, in order) |
| `B6b/` | `b6b4_fifth_rung_gold.patch` — `CosmicAscensionOverlay::LookOf` gives the fifth rung the gold title | `Cosmic_Ascension.UI.AscensionOverlay`, `Cosmic_Ascension.UI.RunEnd` (the fifth's white title) |
| `B6b/` | `b6b4_fifth_rung_glow.patch` — `LookOf` gives the fifth rung the glow | `Cosmic_Ascension.UI.AscensionOverlay`, `Cosmic_Ascension.UI.RunEnd` (the fifth wears no glow) |
| `B6b/` | `b6b4_fifth_rung_chime.patch` — `Show` plays the chime on the fifth rung too | `Cosmic_Ascension.UI.AscensionOverlay`, `Cosmic_Ascension.UI.RunEnd` (no chime counted on the fifth) |
| `B6b/` | `b6b4_fourth_rung_plain.patch` — `LookOf` treats the fourth rung as the fifth | `Cosmic_Ascension.UI.AscensionOverlay` (the second to fourth rungs wear the gold, the glow and the chime) |
| `B6b/` | `b6b4_look_not_restored.patch` — `Show` paints the title's colour and the glow only on the fifth rung | `Cosmic_Ascension.UI.AscensionOverlay` (a second rung after the fifth has the gold and the glow again) |
| `B6b/` | `b6b4_fifth_rung_opens_sight.patch` — `LookOf` lets the fifth rung open the sight | `Cosmic_Ascension.UI.AscensionOverlay`, `Cosmic_Ascension.UI.RunEnd`, `Cosmic_Ascension.UI.ThirdRungSpeed` (no reveal after the fifth rung's presentation) |
| `B6b/` | `b6b4_x25_hidden_any_presentation.patch` — the top bar hides x25 while any presentation shows | `Cosmic_Ascension.UI.ThirdRungSpeed` (x25 stays while the fourth and the fifth rung's presentation show) |
| `B6b/` | `b6b4_sight_opening_ignores_showing.patch` — `IsShowingSightOpening` does not ask whether a presentation shows (reads the closed one's rung) | `Cosmic_Ascension.UI.ThirdRungSpeed` (x25 shows when the presentation closes) |
| `B6b/` | `b6b4_clock_tip_rung_rule_without_row.patch` — `ClockTipOf` states the fifth rung's rule without a judged row when there is no clock end (`b6a4_clock_tip_reads_rung_rule` taken again) | `Cosmic_Ascension.UI.Chronicle` (no clock end and no judged row: no rule) |
| `B6b/` | `b6b4_clock_tip_rung_rule_end_off.patch` — `ClockTipOf` states the fifth rung's rule with the end switched off | `Cosmic_Ascension.UI.Chronicle` (with the end off the fifth rung's rule is not stated) |
| `B6b/` | `b6b4_clock_tip_rung_rule_needs_clock.patch` — `ClockTipOf` states the fifth rung's rule only with a clock end | `Cosmic_Ascension.UI.Chronicle` (without a clock end the fifth rung's rule still stands) |
| `B6b/` | `b6b4_clock_tip_rule_order.patch` — `ClockTipOf` puts the clock's rule before the fifth rung's | `Cosmic_Ascension.UI.Chronicle` (the fifth rung's rule first, the clock's last) |
| `B6b/` | `b6b4_clock_tip_caller_always_rung.patch` — the top bar's clock passes the rung rule whatever the tables hold | `Cosmic_Ascension.UI.Chronicle` (the bar's clock carries that tooltip; since [B-6b7], whose game tables hold the judged fourth-to-fifth row, a bar built without B6b's data states the clock's rule alone) |
| `B6b/` | `b6b4_judged_row_any_path.patch` — `CosmicNextRung::HasJudgedRowAt` takes a row of any path | `Cosmic_Ascension.UI.Chronicle` (a wanderer's row is not one) |
| `B6b/` | `b6b4_judged_row_never.patch` — `HasJudgedRowAt` answers false | `Cosmic_Ascension.UI.Chronicle` (a Ruler row from the fourth rung with a condition is one) |
| `B6b/` | `b6b4_run_end_skips_presentation.patch` — the run end's wait counts while a presentation other than the sight's shows | `Cosmic_Ascension.UI.RunEnd` (while the fifth rung's presentation shows the page waits) |
| `B6b/` | `b6b4_rung_treaty_as_other.patch` — `AllOf` reads `CivHasTreaty` as an other condition | `Cosmic_Ascension.UI.NextRung` (the treaty is a part of its own) |
| `B6b/` | `b6b4_rung_treaty_counts_treaties.patch` — the treaty part counts treaties, not peoples | `Cosmic_Ascension.UI.NextRung` (trade and non-aggression with one people count one people) |
| `B6b/` | `b6b4_rung_laws_as_other.patch` — `AllOf` reads `CivLawsHeldTicksAtLeast` as an other condition | `Cosmic_Ascension.UI.NextRung` (the laws are a part of their own) |
| `B6b/` | `b6b4_rung_laws_left_ignores_life.patch` — the laws' ticks left count the held ticks before this life | `Cosmic_Ascension.UI.NextRung` (the count less the stretch within this life) |
| `B6b/` | `b6b4_rung_laws_left_ignores_held.patch` — the laws' ticks left count the whole life even when the laws were held for less of it | `Cosmic_Ascension.UI.NextRung` (laws held less than this life leave the count less the ticks held) |
| `B6b/` | `b6b4_rung_laws_reset_missing.patch` — `TextOf` drops `Rung.LawsReset` | `Cosmic_Ascension.UI.NextRung` (a laws part ends the tooltip with what resets the count) |
| `B6b/` | `b6b4_rung_laws_reset_per_row.patch` — `TextOfAll` adds `Rung.LawsReset` after every row with laws | `Cosmic_Ascension.UI.NextRung` (over two ways up it is said once) |
| `B6b/` | `b6b4_rung_nofallen_ignores_record.patch` — the binding closes with a completion of this life on a lost system | `Cosmic_Ascension.UI.NextRung` (a completion on a lost system keeps the binding open) |
| `B6b/` | `b6b4_rung_nofallen_with_target.patch` — the binding closes with a fallen one left to bind | `Cosmic_Ascension.UI.NextRung` (with a fallen one left the binding is not closed) |
| `B6b/` | `b6b4_rung_lost_reads_notstarted.patch` — this life's completion on a lost system reads 'not started' (`bLost` never set) | `Cosmic_Ascension.UI.NextRung` (it is lost: done, on a system to take back) |
| `B6b/` | `b6b4_rung_ark_not_started.patch` — an answer no slot holds reads 'not started', not the hazard it answers | `Cosmic_Ascension.UI.NextRung` (the ark's words name the extinction it answers) |
| `B6b/` | `b6b4_slots_count_fifth.patch` — `NextFriendlySlotRungOf` tells the fifth rung's slot again | `Cosmic_Ascension.UI.PeoplesTab` (from the third rung no next slot is told) |
| `B6b/` | `b6b4_slots_next_strict.patch` — the next friendly slot is the next rung's, grown or not (`b3c_slots_next_strict` taken again) | `Cosmic_Ascension.UI.PeoplesTab` (the rung that opens the next slot) |
| `B6b/` | `b6b4_rung_words_unmet.patch` — `UCosmicEdictWidget::RungWordsOf` tells the rung of a people not met | `Cosmic_Ascension.UI.PeoplesTab` (seeing all, a people not met has no rung words) |
| `B6b/` | `b6b4_rung_words_ignore_gap.patch` — `RungWordsOf` tells the rung of a people three rungs above | `Cosmic_Ascension.UI.PeoplesTab` (a people three rungs above has no rung words) |
| `B6b/` | `b6b4_rung_words_off_by_one.patch` — `RungWordsOf` says the rung's index, not its number | `Cosmic_Ascension.UI.PeoplesTab` (people 1's row says its rung) |
| `B6b/` | `b6b4_rung_set_at_build.patch` — the peoples tab sets the rung words once, when the row is built | `Cosmic_Ascension.UI.PeoplesTab` (the row follows its climb) |
| `B6b/` | `b6b4_treaty_tip_ungated.patch` — the treaty buttons carry `Diplomacy.TreatyRungTip` with no judged row asking a treaty | `Cosmic_Ascension.UI.PeoplesTab` (no judged row: no tip) |
| `B6b/` | `b6b4_treaty_tip_proposal_only.patch` — the held treaty's tooltip drops `Diplomacy.TreatyRungTip` | `Cosmic_Ascension.UI.PeoplesTab` (the held treaty's tooltip ends with the tip) |
| `B6b/` | `b6b4_treaty_tip_any_condition.patch` — the treaty buttons carry `Diplomacy.TreatyRungTip` for a judged third-rung row that asks no treaty | `Cosmic_Ascension.UI.PeoplesTab` (a judged third-rung row that asks no treaty adds no tip) |
| `B6b/` | `b6b4_bind_hint_without_target.patch` — the binding's hint names the first instance when none is left to bind | `Cosmic_Ascension.UI.StartReasons` (with every fallen one bound no one is named) |
| `B6b/` | `b6b4_bind_hint_missing.patch` — the binding's tooltip drops `Start.BindHint` | `Cosmic_Ascension.UI.StartReasons` (its tooltip names the fallen one it would hold) |
| `B6b/` | `b6b4_bind_hint_every_row.patch` — every construction's tooltip carries the binding's hint | `Cosmic_Ascension.UI.StartReasons` (another construction's does not) |
| `B6b/` | `b6b4_bind_hint_physical_view.patch` — the binding's hint stands in the physical view too | `Cosmic_Ascension.UI.StartReasons` (in the physical view its tooltip names no one) |
| `B6b/` | `b6b4_epitaph_peak_first_rung.patch` — the epitaph keeps the rung line on the first rung | `Cosmic_Ascension.UI.Epitaph` (a life whose people stayed on the first rung has no rung line) |
| `B6b/` | `b6b4_epitaph_peak_from_third.patch` — the epitaph's rung line from the third rung (an off-by-one between the enum and the shown rung) | `Cosmic_Ascension.UI.Epitaph` (a people that reached the second rung: its rung after the cycles lived) |
| `B6b/` | `b6b4_runend_ignores_pages.patch` — the chronicle's wait counts while a calling page shows (`b5c_runend_ignores_pages` taken again) | `Cosmic_Ascension.UI.Epitaph` |
| `B6b/` | `b6b4_extinction_lines_any_hazard.patch` — the panel's extinction lines stand under any hazard | `Cosmic_Ascension.UI.ClaimMayFollow`, `Cosmic_Ascension.UI.TopBarAndPanel` (the AI awakening says nothing of the extinction; the plague's panel and details checks of `UI.ClaimMayFollow`) |
| `B6b/` | `b6b4_extinction_reads_vessel_kill.patch` — `CosmicAutoPause::RulerDiesAtExpiryOf` does not ask `VesselAtRiskOf` (the birth protection) | `Cosmic_Ascension.UI.TopBarAndPanel` (an expiry inside the birth protection ends no ruler's life) |
| `B6b/` | `b6b4_ruler_lethal_ungated.patch` — the panel adds `Panel.RulerLethal` whatever `RulerDiesAtExpiryOf` answers | `Cosmic_Ascension.UI.TopBarAndPanel` (an expiry inside the birth protection ends no ruler's life) |
| `B6b/` | `b6b4_extinction_tip_dropped.patch` — the details' tooltip keeps the claim's warning alone | `Cosmic_Ascension.UI.TopBarAndPanel` (the details' tooltip is that line) |
| `B6b/` | `b6b4_extinction_any_owner.patch` — the panel's extinction line stands on any owner's settled system | `Cosmic_Ascension.UI.ClaimMayFollow` (on another people's settled system no extinction line) |
| `B6b/` | `b6b4_extinction_in_interregnum.patch` — the panel's extinction line stands without an embodied ruler (`bRules` dropped) | `Cosmic_Ascension.UI.ClaimMayFollow` (in the interregnum no extinction line on the dead ruler's people) |
| `B6b/` | `b6b4_extinction_in_ended_run.patch` — the panel's extinction line stands in an ended run | `Cosmic_Ascension.UI.ClaimMayFollow` (in an ended run no extinction line) |
| `B6b/` | `b6b4_extinction_while_settling.patch` — the panel's extinction line stands on a system still being settled | `Cosmic_Ascension.UI.ClaimMayFollow` (on a system still being settled no extinction line) |
| `B6b/` | `b6b4_realm_cap_last_fifth_only.patch` — `ReasonOf` gives `Settle.Block.RealmCapLast` only from the fifth rung | `Cosmic_Ascension.UI.TopBarAndPanel` (on the fourth rung the settle tooltip starts with the cap's last words) |
| `B6b/` | `b6b4_realm_cap_last_from_third.patch` — `ReasonOf` gives `Settle.Block.RealmCapLast` from the third rung | `Cosmic_Ascension.UI.TopBarAndPanel` (below the fourth rung the cap's reason keeps its words) |
| `B6b/` | `b6b4_realm_cap_last_line_dropped.patch` — the reason line under the settle button reads the cap without the rung | `Cosmic_Ascension.UI.TopBarAndPanel` (and so does the line under it) |
| `B6b/` | `b6b4_panel_no_warning.patch` — the system panel says nothing of a lethal hazard (`b5c_panel_no_warning` taken again) | `Cosmic_Ascension.UI.LethalWarning` |
| `B6b/` | `b6b4_sealed_ring_gold.patch` — the seal ring is drawn gold | `Cosmic_Ascension.UI.SpiritualSight` (the seal ring's segments are grey) |
| `B6b/` | `b6b4_sealed_ring_dashed.patch` — the seal ring is drawn dashed | `Cosmic_Ascension.UI.SpiritualSight` (the seal ring's 24 segments) |
| `B6b/` | `b6b4_data_bound_words_seal.patch` — `ST_CA.csv` `Sight.Bound` says '봉인' (a data patch) | `Cosmic_Ascension.UI.SpiritualSight` (no word of a seal on the panel) |
| `B6b/` | `b6b4_wide_tip_any_edict.patch` — `Edict.TipWide` follows the hazard lines of any edict | `Cosmic_Ascension.UI.EdictWindow` (nor a people's own edict with them) |
| `B6b/` | `b6b4_wide_tip_without_hazard_lines.patch` — `Edict.TipWide` stands on a galaxy law without hazard lines | `Cosmic_Ascension.UI.EdictWindow` (a galaxy law without hazard lines does not) |
| `B6b/` | `b6b4_wide_tip_spawn_only.patch` — `Edict.TipWide` only under a spawn line (the countdown half of the gate dropped) | `Cosmic_Ascension.UI.EdictWindow` (a galaxy law with only a countdown line says it too) |
| `B6b/` | `b6b4_wide_tip_first.patch` — `Edict.TipWide` goes first in the tooltip | `Cosmic_Ascension.UI.EdictWindow` (not first) |
| `B6b/` | `b6b4_wide_mark_every_row.patch` — every edict row carries the galaxy law's mark | `Cosmic_Ascension.UI.EdictWindow` (and no other row does) |
| `B6b/` | `b6b5_schema_rung_prefix_without_binding.patch` — the next-rung keys `Rung.` no longer refuse `binding_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_sight_prefix_dropped.patch` — the sight keys `Sight.` are not checked | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_bind_hint_unchecked.patch` — `Start.BindHint` is not checked | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_treaty_tip_unchecked.patch` — `Diplomacy.TreatyRungTip` loses `binding_words` (only the lists of `Diplomacy.` stay) | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_ruler_lethal_without_death_frame.patch` — `Panel.RulerLethal` no longer refuses `death_frame_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_title_without_messiah.patch` — '메시야' leaves `title_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_feats_take_no_afterlife.patch` — the feat `LogTemplates` column no longer refuses `afterlife_words` | `test_death_and_record_words`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_feats_take_no_triumph.patch` — the feat `LogTemplates` column no longer refuses `triumph_words` (a deed may buy the rung: '칙령을 지켰기에') | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_chronicle_without_titles.patch` — `Chronicle.` no longer refuses `title_words` | `test_string_table_prefixes`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_digest_without_grasp.patch` — `Digest.` no longer refuses `rung_grasp_words` ('…칸에 올라섰다') | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_note_without_binding.patch` — `Chronicle.Note.` no longer refuses `binding_words` | `test_awareness_and_end_words`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_epitaph_without_titles.patch` — `Epitaph.` no longer refuses `title_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_no_fallen_block_unchecked.patch` — `Start.Block.NoFallen` loses `binding_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_no_fallen_suffix_unchecked.patch` — `Start.Suffix.NoFallen` loses `rung_grasp_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_extinction_follows_without_stay.patch` — `Panel.ExtinctionFollows` no longer refuses `stay_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_realm_cap_last_unchecked.patch` — `Settle.Block.RealmCapLast` loses `title_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_log_column_without_grasp.patch` — `DT_LogTemplates.Templates` no longer refuses `rung_grasp_words` (regenerated at [B-6b6] - the column's list ends in `ark_words`) | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_event_column_without_titles.patch` — `DT_Events.LogTemplates` no longer refuses `title_words` (regenerated at [B-6b6] - the column's list ends in `ark_words`) | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_hazard_columns_without_grasp.patch` — `DT_Hazards.ExpireTemplates` no longer refuses `rung_grasp_words` (regenerated at [B-6b6] - the columns' lists end in `ark_words`) | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_project_column_without_titles.patch` — `DT_Megaprojects.CompleteTemplates` no longer refuses `title_words` | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_holy_passive_unextended.patch` — '종말' leaves `holy_passive` | `test_awareness_and_end_words`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_desolation_without_lamp.patch` — '등불' leaves `desolation_words` (regenerated at [B-6b6] - `ark_words` follows the list; no longer applies since [B-6b8] - taken again as `b6b8_schema_desolation_without_lamp`) | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_fifth_feat_rule_dropped.patch` — the fifth-rung feat rule is gone ('때가 찼다' on a T5 row passes; regenerated at [B-6b6] - the feats' `row_not` rule of '방주' follows the rule) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_extinction_text_rule_dropped.patch` — the extinction's text rule is gone ('…끝까지 곁에 남았다' on its expiry passes) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_era_rule_dropped.patch` — the era rule is gone ('새벽의 시대' passes) (no longer applies since [B-6b8] - taken again as `b6b8_schema_era_rule_dropped`) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_binding_name_dropped.patch` — the binding's name no longer refuses `binding_name_words` ('둘레 탑', '공명 기관' pass; regenerated at [B-6b6] - the ark's two rules follow the rule) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_bound_line_triumph_dropped.patch` — `log_fallen_bound` no longer refuses `triumph_words` ('마침내 그늘이 걷혔다' passes `triumph_words`) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_far_ascent_rule_unextended.patch` — `log_far_ascent` loses its `holy_passive` and `binding_words` tests ('…허락되었다는 소식' passes) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_record_extinct_rule_unextended.patch` — `log_record_extinct` loses its stay, frame and desolation tests ('{System}의 등불이 꺼진 일이 기록되었다.' passes) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_death_extinction_rule_unextended.patch` — `log_death_extinction` loses its stay and desolation tests ('…백성을 위해 쓰러졌다' passes `stay_words`) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_event_lines_holy_off.patch` — the events' lines take the holy words (`b4d1_schema_event_lines_holy_off` taken again at [B-6b5]; regenerated at [B-6b6] - the column's list ends in `ark_words`) | `test_first_person_and_card_words`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_event_lines_soul_off.patch` — the events' lines take the soul verbs (`b4d1_schema_event_lines_soul_off` taken again at [B-6b5]; regenerated at [B-6b6] - the column's list ends in `ark_words`) | `test_first_person_and_card_words`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_schema_hazard_expiry_rescue_off.patch` — the hazards' expiry lines take the rescue words (`b4d1_schema_hazard_expiry_rescue_off` taken again at [B-6b5]; regenerated at [B-6b6] - the columns' lists end in `ark_words`) | `test_first_person_and_card_words`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b5_data_bound_line_at_last.patch` — the first variant of `log_fallen_bound` says '마침내' (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the rule of `log_fallen_bound`: `triumph_words`) |
| `B6b/` | `b6b5_data_bound_line_active.patch` — the first variant of `log_fallen_bound` is the active '{System}에서 그늘을 묶었다.' (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the rule of `log_fallen_bound`: `triumph_words` and `{System}` alone) |
| `B6b/` | `b6b5_data_feat_title_eternal.patch` — the dyson feat's title is '영겁의 별' (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the feat column: `rung_grasp_words`) |
| `B6b/` | `b6b5_data_rung_names_god.patch` — `Rung.Done` names the holy order '하나님' (a data patch of `ST_CA.csv`, refused before an asset is written) | `python -I Tools/check_data.py` (the prefix `Rung.`: `realm_names`) |
| `B6b/` | `b6b5_data_record_stayed.patch` — the first variant of `log_record_extinct` says '끝까지' (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the rule of `log_record_extinct`: `stay_words`) |
| `B6b/` | `b6b5_data_feat_says_bound.patch` — a line of `feat_three_worlds` says '…묶였다' (a data patch, refused before an asset is written; regenerated at [B-6b6] - `feat_passage_by_ark` follows the row) | `python -I Tools/check_data.py` (the feat column: `binding_words`) |
| `B6b/` | `b6b6_kills_ruler_not_extinction.patch` — `CosmicHazards::KillsTheRulerOf` is never true (no row's expiry kills the ruler) | `Cosmic_Ascension.Data.Integrity` (the code's one death: exactly `crisis_species_extinction` kills the ruler); `Cosmic_Ascension.Sim.ExtinctionExpiry` |
| `B6b/` | `b6b6_data_extinction_expiry_effect.patch` — the schema's extinction rule (rule 5) keeps `rows_of` alone, the checker's unit tests lose rule 5's three breaking cases and count 312 then-tests, and `crisis_species_extinction` carries a `ChangeStability` on its expiry (a data patch: rebuild the assets - `check_data.py` and `test_data_schema.py`, which `make_data_tables.py` runs first, let it through; no longer applies since [B-6b8] - taken again as `b6b8_data_extinction_expiry_effect`) | `Cosmic_Ascension.Data.Integrity` (the extinction's expiry carries no effect of the data) |
| `B6b/` | `b6b6_schema_ark_rule_dropped.patch` — the ark's row rule is gone ('…방주로 백성을 건져 냈다' on `project_ark`'s completion lines and '노아의 배' as its name pass) | `test_every_rule_of_the_schema_can_fail` (the count), `test_the_ark_and_its_word` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_schema_ark_guard_projects_dropped.patch` — the projects' `row_not` rule of '방주' is gone ('방주' on `project_test_build`'s completion lines passes) | `test_every_rule_of_the_schema_can_fail` (the count), `test_the_ark_and_its_word` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_schema_ark_guard_feats_dropped.patch` — the feats' `row_not` rule of '방주' is gone ('방주' on `feat_test_climb`'s lines passes) | `test_every_rule_of_the_schema_can_fail` (the count), `test_the_ark_and_its_word` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_schema_ark_words_off_hazards.patch` — `DT_Hazards`' name and three template columns no longer refuse `ark_words` | `test_the_ark_and_its_word` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_schema_ark_words_off_lines.patch` — `DT_Events.LogTemplates` and `DT_LogTemplates.Templates` no longer refuse `ark_words` | `test_the_ark_and_its_word`, `test_upper_rung_text_words` (the exact lists) (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_schema_myth_without_noah.patch` — '노아' leaves `holy_and_myth` | `test_the_ark_and_its_word` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_schema_myth_takes_baro.patch` — holy_and_myth takes a name of the hand list ('바로') (`b4d1_schema_myth_takes_baro` taken again at [B-6b6]) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_schema_verdict_without_soul_has_soul.patch` — verdict_without_soul keeps '영혼' (`b4d1_schema_verdict_without_soul_has_soul` taken again at [B-6b6]) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b6_data_extinction_says_ark.patch` — the first resolution line of `crisis_species_extinction` says '방주' (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (`DT_Hazards.ResolveTemplates`: `ark_words`) |
| `B6b/` | `b6b6_data_ark_rescues.patch` — the first completion line of `project_ark` says '…구했다' (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the ark's row rule: `self_rescue_words`) |
| `B6b/` | `b6b6_data_ark_word_in_card.patch` — the body of `petition_wise_heart` says '방주' (a data patch that `check_data.py` lets through - `DT_Encounters.Body` refuses the verdict alone) | `test_the_ark_and_its_word` (`Tools/test_data_schema.py` - the whole project outside the two rows) |
| `B6b/` | `b6b6_brake_always_shut.patch` — `CosmicAscent::FourthRungOpenOf` is never open (no AI people climbs to the fourth rung) | `Cosmic_Ascension.Feats.Reachable` (an AI people reaches the fourth rung in 20,000 ticks; the fifth since [B-6b7]) |
| `B6b/` | `b6b7_data_laws_two_rows.patch` — `edict_wide_warning` leaves `DT_Edicts` (two HomeGalaxy rows under `GalaxyLawsToHold` 3; a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` (the game's galaxy laws: `feat_laws_and_binding` reads the held laws above the 2 HomeGalaxy rows; the `WithoutB6bData` tripwire) |
| `B6b/` | `b6b7_data_feat_without_binding.patch` — `feat_laws_and_binding` drops its `MegaprojectComplete project_binding` (a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` (the fourth-to-fifth ruler row reads the binding and the held laws) |
| `B6b/` | `b6b7_data_upper_rung_not_binary.patch` — `DT_Rules.AIUpperRungProgressMultiplier` 0.4 (a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` (the delay above the third rung is a sixteenth step in (0, 1]) |
| `B6b/` | `b6b7_data_law_answers.patch` — `crisis_plague` lists `edict_wide_quarantine` among its `RulerResolutions` (a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` (the game's galaxy laws: no law is a hazard's answer - on a game row) |
| `B6b/` | `b6b7_data_ark_margin_half.patch` — `DT_Rules.AIAscensionSpeedMultiplier` 0.5 (`b6b2c_data_ark_margin_half` taken again at [B-6b7] - the next line is r; a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` (the ark's margin under the game's rules, on the game's own extinction and ark) |
| `B6b/` | `b6b7_without_b6b_keeps_laws.patch` — `CosmicTests::WithoutB6bData` takes out no galaxy law (its `Edicts` list empty) | `Cosmic_Ascension.Data.Integrity` (the tripwire on today's eight rows and the full copy's ten) |
| `B6b/` | `b6b7_without_b6b_keeps_fifth_feat.patch` — `CosmicTests::WithoutB6bData` keeps `feat_laws_and_binding` | `Cosmic_Ascension.UI.Chronicle` (without B6b's data the tables judge no row up to the fifth rung; a wanderer's row is not one; nor a row without a condition); `Cosmic_Ascension.Data.Integrity` (the tripwire) |
| `B6b/` | `b6b7_binding_never_completes.patch` — `CosmicSeal::BindTargetOf` finds no fallen one (always `INDEX_NONE`: no binding starts, none completes) | `Cosmic_Ascension.Feats.Reachable` (an AI people reaches the fifth rung in 20,000 ticks) |
| `B6b/` | `b6b7_phase_two_exit_full_data.patch` — `Sim.PhaseTwoExit` runs on the game's data whole instead of `CosmicTests::WithoutB6bData` (the deposed vessel of seed 18 then lives past 20,000 ticks; no longer applies since [B-6b12] - `Sim.PhaseTwoExit` returned to the game's data whole on seed 1) | `Cosmic_Ascension.Sim.PhaseTwoExit` (the whole run: the deposed life closes after the deposition and a succession follows before the end) |
| `B6b/` | `b6b7_probe_race_cap_minus_one.patch` — **a probe, not a mutation** (B6b header section 5.1, notes section 5 [B-6b7]): the race check reads the home cap less one, so an AI people that climbs to the fourth rung while another people already stands there is a miss - one `CosmicBalance race check: at <tick> ... past the home cap <cap-1> ...` line naming an `:a` climber and exit code 1 (no `the home cap was passed at N boundaries` line, which prints only from a second miss, `RaceCheckMisses > 1`) on the seed and tick of notes section 5 (`-Ticks` = that climb + 100; at [B-6b7]: seed 12, `-Ticks=3030`, `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0 -Rules=ProvisionalEndTick:0` - the ruled civ_0 on T4 at 2,806, civ_6 climbs at 2,930; notes section 3, bundle A'); never `-FourthRungCap`, which lowers the brake too | (probe — commandlet runs, no test) |
| `B6b/` | `b6b7_probe_race_ruled_unjudged.patch` — **a probe, not a mutation** (B6b header section 5.1, notes section 5 [B-6b7]): the cap less one and `RaceRuledJudged` never matching, so the ruled people's own climb to the fourth rung beside one AI people is asked as an AI climb - a miss at the ruled climb's tick naming the ruled `CivID` as `:a` (the broken flag feeds the climbers' tags too), where `b6b7_probe_race_cap_minus_one` alone gives none at that tick (at [B-6b7]: seed 6, `-Ticks=3177`, the same arguments - civ_2 alone on T4 from 2,779, the ruled civ_0 climbs at 3,077) | (probe — commandlet runs, no test) |
| `B6b/` | `b6b8_schema_witness_rule_dropped.patch` — the witness rule of `DT_Encounters` is gone ('증언' in a label, '목소리' in the body, '기록' outside a success line pass on a wanderer's card on the claim) | `test_every_rule_of_the_schema_can_fail` (the count), `test_the_witness_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b8_schema_witness_words_off_body.patch` — the witness rule hangs `sign_voice_words` on the title instead of the body (a voice of the sign in a claim card's body passes; the count stays) | `test_the_witness_words` (the body's voice case) (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b8_schema_witness_past_forms_dropped.patch` — '기록했' and '떠났' leave `witness_words` (the contracted past forms: '기록했다' and '떠났다' pass, since '기록하' and '떠나' are not in them) | `test_the_witness_words` (the '기록했다' and '떠났다' cases, then the exact list) (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b8_schema_witness_adnominal_forms_dropped.patch` — the forms ending in -ㄴ/-ㄹ and -ㅁ ('말한', '기록한', '떠난', '떠남' and the rest, the fix stage's review L1-2) leave `witness_words` ('떠난 뒤', '기록한 것', '말한다', '떠남' pass, since '떠나', '기록하' and '말하' are not in them) | `test_the_witness_words` (the -ㄴ/-ㄹ/-ㅁ cases, then the exact list) (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b8_data_witness_says_jeungeon.patch` — the first label of `enc_witness` says '증언' (a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the witness rule: `witness_words` on `Choices.Label`) |
| `B6b/` | `b6b8_schema_era_shape_dropped.patch` — the era's shape rule of `DT_Events` is gone (a row on `CivPressureEnduredTicksReached` without `SystemOwnedBy Ruled`, or with the context people's reverence, passes) | `test_every_rule_of_the_schema_can_fail` (the count), `test_the_witness_words` (the era's cases) (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b8_data_era_no_ruled.patch` — `event_era_endured` drops its `SystemOwnedBy Ruled` (every AI people's endurance would lift its neighbours; a data patch, refused before an asset is written) | `python -I Tools/check_data.py` (the era's shape rule: decision 22) |
| `B6b/` | `b6b8_data_era_no_ruled_past_checker.patch` — the era's shape rule keeps its `Borders` test alone, the checker's unit tests lose the two `Ruled` expects and count 329 then-tests, and `event_era_endured` drops its `SystemOwnedBy Ruled` (review L3-2: `b6b8_data_era_no_ruled` never gets past `check_data.py`; a data patch: rebuild the assets - `check_data.py` and `test_data_schema.py`, which `make_data_tables.py` runs first, let it through) | `Cosmic_Ascension.Sim.EraEndured` (an AI people under the same drain gets no era line; its neighbours gain nothing) |
| `B6b/` | `b6b8_data_era_borders_0_3.patch` — `event_era_endured` raises the neighbours' reverence by 0.3 (a data patch that the checker and its unit tests let through: rebuild the assets) | `Cosmic_Ascension.Sim.EraEndured` (each bordering people gains 0.2) |
| `B6b/` | `b6b8_data_witness_reverence_0_3.patch` — the first choice of `enc_witness` raises the owner's reverence by 0.3 (a data patch that the checker and its unit tests let through: rebuild the assets) | `Cosmic_Ascension.Sim.WitnessCard` (a passed check gives the owner 0.1) |
| `B6b/` | `b6b8_wanderer_nocheck_not_by_row.patch` — the wanderer policy counts every accepted answer in `NoCheckByRow` (the `CheckStat` of the choice taken not read: a passed or failed check counts as a step back) | `Cosmic_Ascension.Sim.WitnessCard` (the policy answering two witness cards in turn - the check, then the step back - counts one answer without a check) |
| `B6b/` | `b6b8_wanderer_nocheck_reads_next_choice.patch` — the wanderer policy reads the `CheckStat` of the choice after the count of answers moved on (the check counted as a step back and the step back as a check: the total stays one; review L3-1) | `Cosmic_Ascension.Sim.WitnessCard` (per boundary: no answer without a check after the first, one after the second) |
| `B6b/` | `b6b8_wanderer_nocheck_inverted.patch` — the wanderer policy counts the answers with a check in `NoCheckByRow` (`!=` for `==`: the total stays one; review L3-1) | `Cosmic_Ascension.Sim.WitnessCard` (per boundary: no answer without a check after the first) |
| `B6b/` | `b6b8_cmdlet_witness_first_any_card.patch` — the race line's witness `first at` is set at the first boundary any card of the wanderer policy came (not a rise of `CardsByRow[enc_witness]`; measurement only - no test reads the line) | the `bundleB.py --max-seed 2` smoke on the wanderer cell (its W controls: `first at` >= 0 if and only if witness cards > 0, W not before the wanderer left nor before the first claim placed) - a harness check, not an automation test |
| `B6b/` | `b6b8_schema_desolation_without_lamp.patch` — '등불' leaves `desolation_words` (`b6b5_schema_desolation_without_lamp` taken again at [B-6b8] - the witness lists follow `ark_words`) | `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b8_schema_era_rule_dropped.patch` — the era words rule is gone ('새벽의 시대' passes; the era's shape rule stays) (`b6b5_schema_era_rule_dropped` taken again at [B-6b8] - the shape rule follows it) | `test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b8_data_extinction_expiry_effect.patch` — the schema's extinction rule (rule 5) keeps `rows_of` alone, the checker's unit tests lose rule 5's three breaking cases and count 326 then-tests, and `crisis_species_extinction` carries a `ChangeStability` on its expiry (`b6b6_data_extinction_expiry_effect` taken again at [B-6b8] - the count; a data patch: rebuild the assets - `check_data.py` and `test_data_schema.py`, which `make_data_tables.py` runs first, let it through) | `Cosmic_Ascension.Data.Integrity` (the extinction's expiry carries no effect of the data) |
| `B6b/` | `b6b8b_data_share_off.patch` — `DT_Rules.FleetDelayCountdownShare` 0, the value before [B-6b8b] (a data patch: rebuild the assets) | `Cosmic_Ascension.Sim.FleetDelay` (the game's share is 0.5; fleet 3 adds the game's awakening 75: 225, not 250) |
| `B6b/` | `b6b8b_data_share_third.patch` — `DT_Rules.FleetDelayCountdownShare` 0.3 (a data patch: rebuild the assets) | `Cosmic_Ascension.Data.Integrity` (game theodicy: the claim's longest stand becomes 540, so a throne lock of 549 reports nothing - the W pin reads the game's share) and `Cosmic_Ascension.Sim.FleetDelay` (the 0.5 pin; the plague's cap 60) |
| `B6b/` | `b6b8b_share_zero_is_a_share.patch` — `CosmicConflict::GuardDelayCapOf` takes a share of 0 as a share (`Share < 0` for `!(Share > 0)`: the row's cap 0, while NaN and a share below 0 still give FleetDelayCap) | `Cosmic_Ascension.Sim.FleetDelay` (at a share of 0 the row's cap is FleetDelayCap's 100: the awakening 250, not 150) |
| `B6b/` | `b6br_memo_unsorted.patch` — `SortedRowNamesOf` (`Data/CosmicDataTables.cpp`) keeps the names in the table's own order (`Memo.Sorted.Sort(FNameLexicalLess())` dropped) | `Cosmic_Ascension.Data.SortedNamesFollowTable` (i-1/i-2 for `GetCivRowNames`, `GetEventRowNames`, `GetFeatRowNames`, `GetEncounterRowNames`, `GetEdictRowNames` and `GetGalaxyRowNames` on the fresh and the live tables - `DT_Hazards` and `DT_Transcendents` are lexical in `Tools/data`; ii-1..ii-4, ii'-1..ii'-4; iii for `GetFeatRowNames`, `GetEdictRowNames`, `GetEventRowNames`, `GetEncounterRowNames`, `GetGalaxyRowNames`) |
| `B6b/` | `b6br_memo_never_refreshes.patch` — the memo is checked against the table only while it is empty (`Memo.Keys.IsEmpty() && !SameKeySequence(...)`: once made, never rebuilt) | `Cosmic_Ascension.Data.SortedNamesFollowTable` (ii-3, ii-4, ii-6, ii'-2, ii'-3, ii'-4; iii for all eight accessors) |
| `B6b/` | `b6br_memo_count_only.patch` — `SameKeySequence` compares the counts only (the key loop removed) | `Cosmic_Ascension.Data.SortedNamesFollowTable` (ii-4: a row taken out and another put in; ii'-2, ii'-3, ii'-4) |
| `B6b/` | `b6br_memo_case_blind.patch` — `SameKeySequence` compares the keys with `ENameCase::IgnoreCase` | `Cosmic_Ascension.Data.SortedNamesFollowTable` (ii'-2: a row renamed in another case keeps its old name) |
| `B6b/` | `b6br_memo_number_blind.patch` — `SameKeySequence` compares the keys without their numbers (`IsEqual(..., false)`) | `Cosmic_Ascension.Data.SortedNamesFollowTable` (ii'-4: `feat_n_2` named `feat_n_1`) |
| `B6b/` | `b6br_memo_null_reads_memo.patch` — a null table answers the memo's names instead of none | `Cosmic_Ascension.Data.SortedNamesFollowTable` (ii-5, right after a non-empty memo) |
| `B6b/` | `b6br_memo_wrong_table.patch` — `GetEncounterRowNames` sorts `Events` (`SortedRowNamesOf(Events, EncounterNamesMemo)`) | `Cosmic_Ascension.Data.SortedNamesFollowTable` (i-1/i-2 for `GetEncounterRowNames` on both sets; iii for `GetEncounterRowNames`) |
| `B6b/` | `b6br_memo_fast_less.patch` — `SortedRowNamesOf` sorts with `FNameFastLess` (the order of creation in the name pool, which differs between processes - docs/45 forbids it) | `Cosmic_Ascension.Data.SortedNamesFollowTable` (ii-1, ii-2, ii-3: `feat_b` is made before `feat_a`; not ii' - its names were made in the lexical order b, c, d) |
| `B6b/` | `b6br_answer_first_row_only.patch` — `IsHazardAnswer` reads only the first row of the map (the loop body returns at once) | `Cosmic_Ascension.Data.HazardAnswerAnyRow` (b-1; b-3 for `project_ark`, `project_relief`, `project_stabilize` and `project_reform`) |
| `B6b/` | `b6br_answer_miss_true.patch` — `IsHazardAnswer` answers true when no row lists the name (its last `return false` for `return true`) | `Cosmic_Ascension.Data.HazardAnswerAnyRow` (b-2; b-3 for every `DT_Megaprojects` and `DT_Edicts` row no hazard lists) |
| `B6b/` | `b6b10_latch_reads_the_knob.patch` — the latch (`ACosmicPlayerController::FollowEnd`, `World/CosmicGameMode.cpp`) compares the record with the knob: a record `Ending.Clock` whose tick lies under `ProvisionalEndTick` is taken for no record | `Cosmic_Ascension.Sim.SaveOldEndFlows` (case (5): a B6a save ended by the clock at 6,000, opened under a clock of 7,000, latches nothing; cases (1), (2) and (4) have a record tick at or past the knob) |
| `B6b/` | `b6b10_markof_reads_the_knob.patch` — `UCosmicSaveSubsystem::MarkOf` compares the record with the knob: a record `Ending.Clock` whose tick lies under `Rules.ProvisionalEndTick` marks the disk copy not ended (the patch adds `#include "Core/CosmicTags.h"`) | `Cosmic_Ascension.Sim.SaveOldEndFlows` (case (5): the disk copy of the 6,000 save under the 7,000 clock is a run that goes on) |
| `B6b/` | `b6b10_data_clock_six_thousand.patch` — `DT_Rules.ProvisionalEndTick` 6000, the B6a start value before [B-6b10] (a data patch: rebuild the assets) | `Cosmic_Ascension.Sim.EndingOf` (the game's clock end is tick 9,000) and `Cosmic_Ascension.UI.RunEnd` (the game's clock end is tick 9,000) |
| `B6b/` | `b6b12_chronicle_rung_at_first.patch` — the chronicle chapter's facts keep the rung on the first rung (`Life.PeakTier == Tier1_Planetary`) | `Cosmic_Ascension.UI.Chronicle` (a life whose peak stayed on the first rung has no rung fact) |
| `B6b/` | `b6b12_chronicle_rung_never.patch` — the chronicle chapter's facts never show the rung (`Rung` always empty) | `Cosmic_Ascension.UI.Chronicle` (the chapter of a life whose peak is the third rung carries its rung fact) |
| `B6b/` | `b6b12_data_resist_factor.patch` — `DT_Rules.ResistFleetFactor` 0.6 (`b4d2_data_resist_factor` taken again at [B-6b12]; a data patch: rebuild the assets) | `Cosmic_Ascension.Sim.PressureChain` |
| `B6b/` | `b6b12_no_room_made.patch` — no slot is made for a standing hazard's answer (`b2c_no_room_made` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B6b/` | `b6b12_sightwalk_new_world.patch` — the sight-walk proof runs on the current world (`b2c_sightwalk_new_world` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.SightWalkMoved` |
| `B6b/` | `b6b12_room_takes_kept_row.patch` — room is made by giving up the kept row first (`b2c_room_takes_kept_row` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B6b/` | `b6b12_room_never_kept.patch` — the kept row is never given up for an answer (`b2c_room_never_kept` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B6b/` | `b6b12_delay_ignores_gap.patch` — a claim's delay ignores the gap to its claimant (`b3b_delay_ignores_gap` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.FleetDelay` |
| `B6b/` | `b6b12_delay_outside_cut.patch` — the delay is added past the int32 cut (`b3b_delay_outside_cut` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.FleetDelay` |
| `B6b/` | `b6b12_spread_live_table.patch` — the spread reads a plague placed in the same step (`b3b_spread_live_table` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.PlagueSpread` |
| `B6b/` | `b6b12_spread_no_multiplier.patch` — the spread multiplies nothing (`b3b_spread_no_multiplier` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.PlagueSpread` |
| `B6b/` | `b6b12_civhasedict_no_table.patch` — CivHasEdict does not ask the table (`b4b_civhasedict_no_table` taken again at [B-6b12]) | `Cosmic_Ascension.Sim.CivHasEdictCondition` |
| `B6b/` | `b6b12_theodicy_one_bonus.patch` — the theodicy's longest stand counts one edict's countdown bonus (`b4b_theodicy_one_bonus` taken again at [B-6b12]) | `Cosmic_Ascension.Data.Integrity` |
| `B6b/` | `b6b12_theodicy_no_card_delay.patch` — the theodicy's longest stand leaves the cards' DelayHazard out (`b4b_theodicy_no_card_delay` taken again at [B-6b12]) | `Cosmic_Ascension.Data.Integrity` |
| `B6b/` | `b6b12_schema_none_always_true.patch` — the row test none always holds (`b4b_schema_none_always_true` taken again at [B-6b12]) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_schema_none_any_all.patch` — the row test none asks all the entries instead of any (`b4b_schema_none_any_all` taken again at [B-6b12]) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_schema_contains_none_tag_decl_off.patch` — the tags of contains_none need not be declared (`b4b_schema_contains_none_tag_decl_off` taken again at [B-6b12]) | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_schema_enact_not_card_only.patch` — EnactEdict is not card_only (`b4b_schema_enact_not_card_only` taken again at [B-6b12]) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_schema_civhasedict_optional.patch` — CivHasEdict's Key is optional (`b4b_schema_civhasedict_optional` taken again at [B-6b12]) | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_schema_drain_max_off.patch` — PressureDrainMultiplier has no upper bound (`b4b_schema_drain_max_off` taken again at [B-6b12]) | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_schema_resist_min_off.patch` — ResistFleetFactor has no lower bound (`b4b_schema_resist_min_off` taken again at [B-6b12]) | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_drain_tip_one.patch` — the drain line shows for a factor of 1 (`b4c_drain_tip_one` taken again at [B-6b12]) | `Cosmic_Ascension.UI.EdictRules` |
| `B6b/` | `b6b12_drain_tip_below_one.patch` — the drain line shows only for a factor below 1 (`b4c_drain_tip_below_one` taken again at [B-6b12]) | `Cosmic_Ascension.UI.EdictRules` |
| `B6b/` | `b6b12_drain_tip_infinite.patch` — the drain line shows for an infinite factor (`b4c_drain_tip_infinite` taken again at [B-6b12]) | `Cosmic_Ascension.UI.EdictRules` |
| `B6b/` | `b6b12_drain_tip_zero.patch` — the drain line shows for a factor of 0 or below (`b4c_drain_tip_zero` taken again at [B-6b12]) | `Cosmic_Ascension.UI.EdictRules` |
| `B6b/` | `b6b12_schema_lacks_words_list_unchecked.patch` — a lacks_words that names no word list of the schema passes (`b4d1_schema_lacks_words_list_unchecked` taken again at [B-6b12]) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B6b/` | `b6b12_tenth_outside_guard.patch` — the tenth stage runs outside the stage guard (`b5b_tenth_outside_guard` taken again at [B-6b12]) | `Cosmic_Ascension.Calling.LeapDeterminism` |
| `B6b/` | `b6b12_death_line_no_hazard.patch` — the death line's {Hazard} is left empty (`b5d1_death_line_no_hazard` taken again at [B-6b12]) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B6b/` | `b6b12_death_line_no_name.patch` — the death line's {Player} is left empty (`b5d1_death_line_no_name` taken again at [B-6b12]) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B6b/` | `b6b12_death_line_any_cause.patch` — a cause with no row of its own is told with the hazard death's row (`b5d1_death_line_any_cause` taken again at [B-6b12]) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B6b/` | `b6b12_record_no_annex.patch` — a claim that joins a sphere records nothing (`b5d1_record_no_annex` taken again at [B-6b12]) | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B6b/` | `b6b12_record_unjoined_claim.patch` — every claim's expiry records a loss, one that joins nothing too (`b5d1_record_unjoined_claim` taken again at [B-6b12]) | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B6b/` | `b6b12_death_line_death_place.patch` — the death line has the place of the death as its focus (`b5d1_death_line_death_place` taken again at [B-6b12]) | `Cosmic_Ascension.Log.ClaimVoice`, `Cosmic_Ascension.Calling.DeathPipeline` |
| `B6b/` | `b6b12_feed_no_end_refresh.patch` — the end's latch announces nothing (`b5d1_feed_no_end_refresh` taken again at [B-6b12]) | `Cosmic_Ascension.UI.LeapBoundary` |
| `B6b/` | `b6b12_kills_branch_only.patch` — CosmicHazards::KillsAVesselOf reads the branch effects alone (`b5d2_kills_branch_only` and `b5d2_kills_branch_only_ui` taken again at [B-6b12]) | `Cosmic_Ascension.Calling.LethalSource`, `Cosmic_Ascension.UI.LethalWarning` |
| `B6b/` | `b6b12_claim_may_follow_no_nova_claim_gate.patch` — `ClaimMayFollowOf` drops the gate of `Hazard.PreSupernova` and `Hazard.Claim` (`b6a4_claim_may_follow_no_nova_claim_gate` taken again at [B-6b12]) | `Cosmic_Ascension.UI.ClaimMayFollow` |
| `B6b/` | `b6b12_awareness_before_outbreak.patch` — the awareness line is told before the new claim's outbreak line (`b6a5_awareness_before_outbreak` taken again at [B-6b12]) | `Cosmic_Ascension.Log.AwarenessVoice` |
