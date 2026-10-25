# Mutation patches

Each patch breaks one new assertion once, as `docs/40_DEV_GUIDE.md` asks of every new test: apply it (`git apply <patch>`),
build, run the named test and see it fail, then revert (`git apply -R <patch>`). The patch name says which test it breaks.
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
