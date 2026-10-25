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
