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
