# Mutation patches

Each patch breaks one new assertion once, as `docs/40_DEV_GUIDE.md` asks of every new test: apply it (`git apply <patch>`),
build, run the named test and see it fail, then revert (`git apply -R <patch>`). The patch name says which test it breaks.
A patch of `Tools/data` (the `b4d2_data_` ones) needs the assets rebuilt by `Tools/make_data_tables.py` instead of a build,
after the apply and again after the revert.
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
| `B2/` | `b2c_no_room_made.patch` — no slot is made for a standing hazard's answer | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_vigil_kept.patch` — the policy's vigil stays after the danger passed | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_idle_answer_kept.patch` — an answer with nothing to answer stays | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_decisions_without_edicts.patch` — the decisions leave out the edict commands | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_edicts_after_end.patch` — the policy changes edicts after the provisional end | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_helper_keeps_edict.patch` — the test helper does not revoke the inherited edict | `Cosmic_Ascension.Sim.PlayerAscendsToT3` |
| `B2/` | `b2c_quiet_keeps_edict.patch` — the quiet board keeps the inherited edict | `Cosmic_Ascension.Sim.StabilityRecovery` |
| `B2/` | `b2c_sightwalk_new_world.patch` — the sight-walk proof runs on the B2 world | `Cosmic_Ascension.Sim.SightWalkMoved` |
| `B2/` | `b2c_room_takes_kept_row.patch` — room is made by giving up the kept row first | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
| `B2/` | `b2c_room_never_kept.patch` — the kept row is never given up for an answer | `Cosmic_Ascension.Sim.RulerEdictPolicy` |
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
| `B3/` | `b3b_delay_ignores_gap.patch` — a claim's delay ignores the gap to its claimant | `Cosmic_Ascension.Sim.FleetDelay` |
| `B3/` | `b3b_delay_outside_cut.patch` — the delay is added past the int32 cut | `Cosmic_Ascension.Sim.FleetDelay` |
| `B3/` | `b3b_spread_live_table.patch` — the spread reads a plague placed in the same step | `Cosmic_Ascension.Sim.PlagueSpread` |
| `B3/` | `b3b_spread_no_multiplier.patch` — the spread multiplies nothing | `Cosmic_Ascension.Sim.PlagueSpread` |
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
| `B3/` | `b3b_chronicle_keeps_stance.patch` — the chronicle admits the stance lines | `Cosmic_Ascension.Log.ConflictLines` |
| `B3/` | `b3b_effect_skips_gate.patch` — the SetStance effect skips the stance gate | `Cosmic_Ascension.Events.RoleEffectRules` |
| `B3/` | `b3b_schema_no_card_only.patch` — the schema checker lets a card-only effect stand anywhere | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py`) |
| `B3/` | `b3b_schema_log_without_holy_words.patch` — the log lines no longer refuse the holy words | `test_diplomacy_and_conflict` (`Tools/test_data_schema.py`) |
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
| `B3/` | `b3c_slots_next_strict.patch` — the next friendly slot is the next rung's, grown or not | `Cosmic_Ascension.UI.PeoplesTab` |
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
| `B4/` | `b4b_civhasedict_no_table.patch` — CivHasEdict does not ask the table | `Cosmic_Ascension.Sim.CivHasEdictCondition` |
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
| `B4/` | `b4b_theodicy_one_bonus.patch` — the theodicy's longest stand counts one edict's countdown bonus | `Cosmic_Ascension.Data.Integrity` |
| `B4/` | `b4b_theodicy_no_card_delay.patch` — the theodicy's longest stand leaves the cards' DelayHazard out | `Cosmic_Ascension.Data.Integrity` |
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
| `B4/` | `b4b_theodicy_cap_only.patch` — the theodicy's sphere bound reads the cap alone, not the one core | `Cosmic_Ascension.Data.Integrity` |
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
| `B4/` | `b4b_schema_none_always_true.patch` — the row test none always holds | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_none_any_all.patch` — the row test none asks all the entries instead of any | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_zero_inverted.patch` — the row test zero is inverted | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_contains_none_always.patch` — the row test contains_none always holds | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_struct_conditions_not_read.patch` — not_read_by does not look into the conditions of a list of structs | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_refusal_table_check_off.patch` — the schema check of not_tag_of takes a column that is no tag | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_not_read_by_tag_check_off.patch` — the schema check of not_read_by takes a condition that reads no tag | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_none_shape_check_off.patch` — the schema check of none takes tests over more than one list | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_contains_none_tag_decl_off.patch` — the tags of contains_none need not be declared | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_grace_rule_weak.patch` — the grace rule bans another condition than CivHasEdict | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_e3_list_short.patch` — the rolled-choice ban misses the Stability check | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_delay_max.patch` — DelayHazard may carry up to 1,000 ticks | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_delay_not_card_only.patch` — DelayHazard is not card_only | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_enact_not_card_only.patch` — EnactEdict is not card_only | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_civhasedict_optional.patch` — CivHasEdict's Key is optional | `test_pressure_cards` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_counterfeit_when_celestial.patch` — CounterfeitOf is required of the celestial rows instead | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_claim_pair_log_claim.patch` — Event.Crisis.Claim keeps Log.Claim among its line categories | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_drain_max_off.patch` — PressureDrainMultiplier has no upper bound | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
| `B4/` | `b4b_schema_resist_min_off.patch` — ResistFleetFactor has no lower bound | `test_voice_and_drain` (`Tools/test_data_schema.py`) |
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
| `B4/` | `b4c_drain_tip_one.patch` — the drain line shows for a factor of 1 | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_complement.patch` — the drain line tells 1 - the factor | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_below_one.patch` — the drain line shows only for a factor below 1 | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_infinite.patch` — the drain line shows for an infinite factor | `Cosmic_Ascension.UI.EdictRules` |
| `B4/` | `b4c_drain_tip_zero.patch` — the drain line shows for a factor of 0 or below | `Cosmic_Ascension.UI.EdictRules` |
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
| `B4/` | `b4d1_schema_lacks_words_list_unchecked.patch` — a lacks_words that names no word list of the schema passes | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_lacks_words_any_kind.patch` — lacks_words may be asked of a number column | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_hazard_expiry_rescue_off.patch` — the hazards' expiry lines take the rescue words | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_event_lines_holy_off.patch` — the events' lines take the holy words | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_event_lines_soul_off.patch` — the events' lines take the soul verbs | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_realm_without_most_high.patch` — realm_names misses '지극히 높은' | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_realm_with_eumbu.patch` — realm_names holds '음부' ('처음부터' is refused) | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_myth_takes_baro.patch` — holy_and_myth takes a name of the hand list ('바로') | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_petition_failure_rescue_off.patch` — a petition's failure text takes the rescue words | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_sign_slots_wide.patch` — the claim's sign may call {Hazard} | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_first_person_forms_differ.patch` — the lure's words and the claim lines' words differ | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
| `B4/` | `b4d1_schema_verdict_without_soul_has_soul.patch` — verdict_without_soul keeps '영혼' | `test_first_person_and_card_words` (`Tools/test_data_schema.py`) |
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
| `B4/` | `b4d2_data_resist_factor.patch` — (data) the fleet's time spends 0.6 of the fleet | `Cosmic_Ascension.Sim.PressureChain` |
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
| `B5/` | `b5b_tenth_outside_guard.patch` — the tenth stage runs outside the stage guard | `Cosmic_Ascension.Calling.LeapDeterminism` |
| `B5/` | `b5b_runleap_no_boundary.patch` — RunLeap sends no boundary | `Cosmic_Ascension.Calling.LeapDeterminism`, `Cosmic_Ascension.Sim.SaveLeapResume`, `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_feed_tick_rule_only.patch` — the feed rebuilds on a tick gap only, not on the leap's boundary | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_feed_no_simtick.patch` — the feed does not listen to OnSimTick | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5b_advance_no_end_cut.patch` — AdvanceLeap does not stop at the provisional end | `Cosmic_Ascension.UI.LeapBoundary` |
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
| `B5/` | `b5c_runend_ignores_pages.patch` — the chronicle's wait counts while a calling page shows | `Cosmic_Ascension.UI.Epitaph` |
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
| `B5/` | `b5c_lethal_standing_only.patch` — the lethal place is where the vessel stands (`StandingIndexOf`), not where it counts as standing (a travelling vessel is not warned of the system it left) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_follow_off.patch` — a new lethal hazard where the vessel stands does not stop the time | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_seen_kept.patch` — the lethal hazard seen is kept across a new galaxy or a load | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_no_arrival.patch` — the lethal pause compares the hazard's tag only (an arrival where the same tag stands is no news) | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_panel_no_warning.patch` — the system panel says nothing of a lethal hazard | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_travel_tip_no_warning.patch` — the travel button's tooltip says nothing of a lethal hazard | `Cosmic_Ascension.UI.LethalWarning` |
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
| `B5/` | `b5c_lethal_no_protection.patch` — the lethal place drops the shared reach (`VesselAtRiskOf`): a vessel shielded at its birth is warned and stopped for a hazard that cannot kill it | `Cosmic_Ascension.UI.LethalWarning` |
| `B5/` | `b5c_lethal_reach_after_expiry.patch` — the lethal place asks the reach one tick after the hazard's expiry (the protection's last tick no longer shields) | `Cosmic_Ascension.UI.LethalWarning` |
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
| `B5/` | `b5d1_death_line_no_hazard.patch` — the death line's {Hazard} is left empty (not the hazard row behind the death) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_death_line_no_name.patch` — the death line's {Player} is left empty (not the closed life's name - the third person, decision 12-da) | `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_death_line_any_cause.patch` — a cause with no row of its own (the measurement's Death.Age) is told with the hazard death's row | `Cosmic_Ascension.Calling.DeathPipeline` |
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
| `B5/` | `b5d1_record_no_annex.patch` — a claim that joins a sphere records nothing | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
| `B5/` | `b5d1_record_unjoined_claim.patch` — every claim's expiry records a loss, one that joins nothing too | `Cosmic_Ascension.Log.TwoBooksSameLoss`, `Cosmic_Ascension.Log.ClaimVoice` |
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
| `B5/` | `b5d1_death_line_death_place.patch` — the death line has the place of the death (the dead vessel's `CurrentSystemID`) as its focus: a narrator's line calls the claimed system after the first person when the vessel dies there on the joining tick | `Cosmic_Ascension.Log.ClaimVoice`, `Cosmic_Ascension.Calling.DeathPipeline` |
| `B5/` | `b5d1_unseen_line_shown.patch` — `CosmicChronicle::IsUnseenLifeLine` holds nothing back: the feed shows the inauguration line of the vessel the end's tick installed | `Cosmic_Ascension.UI.LeapBoundary`, `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5d1_unseen_line_while_running.patch` — `CosmicChronicle::IsUnseenLifeLine` holds the inauguration line back while the run goes on (the feed's last item after a leap is not the new vessel's line) | `Cosmic_Ascension.UI.LeapBoundary`, `Cosmic_Ascension.Log.ChronicleBuild` |
| `B5/` | `b5d1_feed_rebuild_shows_unseen.patch` — the feed's rebuild skips only untold lines (the inauguration line of the vessel the end's tick installed stands last) | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_feed_appends_unseen.patch` — a new line of the vessel the end's tick installed, heard after the end latched, is appended to the feed | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_feed_no_end_refresh.patch` — the end's latch announces nothing (`OnRunEndChanged`): after a leap of 0 on the end's tick the feed keeps the inauguration line it heard | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_feed_clear_no_refresh.patch` — forgetting the end announces nothing: the feed keeps holding the inauguration line back once the end is switched off | `Cosmic_Ascension.UI.LeapBoundary` |
| `B5/` | `b5d1_books_part_one_alone.patch` — part one is thinned first-and-last alone again, blind to part two: a shown claim can stand over a place whose record part one left out | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_books_no_item_cap.patch` — part two loses its 12-item cap (the fit test and the deal's `ItemsLeft`): only the 28-row cap stops it | `Cosmic_Ascension.Log.TwoBooksNoVessel` |
| `B5/` | `b5d1_record_any_destruction.patch` — the destruction record drops its settled-only guard (`bWasSettled`): a system under its settlement or a wild one destroyed is recorded | `Cosmic_Ascension.Log.TwoBooksSameLoss` |
| `B5/` | `b5d1_books_part_head.patch` — part one's head goes back to a part's head (grey 18): the two books' chapter has no chapter-level head | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_books_head_after_lines.patch` — a claimant's head (`Chronicle.ClaimedBy`) stands after its lines, not over them | `Cosmic_Ascension.UI.Chronicle` |
| `B5/` | `b5d1_epitaph_keeps_star_watch.patch` — the epitaph shows the trembling star's watch ('흔들리는 별 곁에서', the 'beside' title of the sacrifice reading - header 3.6-1) among a dead vessel's last answers again: CosmicRows::EpitaphLeavesOut drops EncStarWatch | `Cosmic_Ascension.UI.Epitaph` |
| `B5/` | `b5d1_epitaph_keeps_eulogy_petitions.patch` — the epitaph shows the wise heart and the feast (a virtue, a memorial day under a death) again: CosmicRows::EpitaphLeavesOut drops PetitionWiseHeart and PetitionFeast | `Cosmic_Ascension.UI.Epitaph` |
