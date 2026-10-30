// Cosmic Ascension - Data layer. Design: guideline.md section 7.3, 8.2, 10.4.
// DataTable row names that the code refers to by name. Row names are snake_case and never change once shipped:
// they land in saves (FCosmicLogEntry.SourceRow, FCivilizationState.ArchetypeRow, FLifeRecord.BirthArchetypeRow).

#pragma once

#include "CoreMinimal.h"

namespace CosmicRows
{
	// DT_Rules: the single row.
	inline const FName Rules(TEXT("rules"));

	// DT_Civs: the ordinary AI archetype. It is the fallback bank for civs without an archetype and for nameless civs,
	// and the one row whose civs draw their ethics at random (section 5.11, 7.4).
	inline const FName ArchDefault(TEXT("arch_default"));

	// DT_Civs: the first life's row, a seeking people outside the sanctuary (DT_Rules.PlayerArchetypeRow and the C++ default
	// of FCosmicRulesDef, section 5.10, 5.11). Weight 0: never drawn for AI peoples.
	inline const FName ArchSeeker(TEXT("arch_seeker"));

	// DT_Transcendents: Silence, the one fallen transcendent every run has. The roster of NewGalaxy starts from it
	// (section 4.2).
	inline const FName TransSilence(TEXT("trans_silence"));

	// DT_Megaprojects: settling. A system whose ActiveMegaproject is this row is owned but not yet settled (section 5.7).
	inline const FName ProjectSettle(TEXT("project_settle"));

	// DT_Events: the book is found. The research stage fires it at every Society breakthrough; the row's conditions
	// pick the people whose book is shut (section 5.11).
	inline const FName EventBookFound(TEXT("event_book_found"));

	// DT_Events: the grace of the remnant. The last-system gate fires it by name before a people loses its last settled
	// system; the row's own conditions keep it to once per run (section 4.5, 7.4).
	inline const FName EventGraceRemnant(TEXT("event_grace_remnant"));

	// DT_Events: the vigil's draw row (B4 header section 2.8). The balance commandlet counts the claims it eased.
	inline const FName EventClaimEased(TEXT("event_claim_eased"));

	// DT_LogTemplates: the first line of a run (section 2.4).
	inline const FName LogFirstLife(TEXT("log_first_life"));

	// DT_LogTemplates: the fallen monarch, told by the deposition pipeline (section 5.10).
	inline const FName LogFallenMonarch(TEXT("log_fallen_monarch"));

	// DT_LogTemplates: the wanderer's loop (B1, docs/60_DECISIONS/headers/B1_wanderer_loop.md section 3.3). A scan of a
	// wild system that learned something; a coronation and a return (UPlayerRoleSubsystem::BecomeRuler); an abdication
	// (the tenth stage's pipeline - also the SourceRow of its Notable.Regent record); a rebellion that expires while
	// another source has recorded the throne's end first (the throne does not go to the rebels).
	inline const FName LogScanWild(TEXT("log_scan_wild"));
	inline const FName LogCrowned(TEXT("log_crowned"));
	inline const FName LogReturned(TEXT("log_returned"));
	inline const FName LogThroneVacated(TEXT("log_throne_vacated"));
	inline const FName LogRebellionUnseated(TEXT("log_rebellion_unseated"));

	// DT_LogTemplates: the edict lines of the people the player rules (B2, docs/60_DECISIONS/headers/B2_edicts_and_research.md
	// section 3.4) - an enactment and a revocation by command, a lapse of an edict whose upkeep could not be paid. The
	// chronicle leaves them out (CosmicChronicle::Admits).
	inline const FName LogEdictEnacted(TEXT("log_edict_enacted"));
	inline const FName LogEdictRevoked(TEXT("log_edict_revoked"));
	inline const FName LogEdictLapsed(TEXT("log_edict_lapsed"));

	// DT_LogTemplates: the diplomacy and conflict lines (B3, docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section
	// 3.4) - told only when the people the player rules is a party, the other people as the argument {OtherCiv}. A stance
	// set by command and a friendly stance that lapses (the chronicle leaves both out - CosmicChronicle::Admits), a treaty
	// signed or broken, and a siege: begun by command (Severity 1 - no auto-pause), laid on the ruled people, taken, lost,
	// held, failed (Severity 2), lifted when its gate closed (Severity 1). One row per side: the attacker's and the
	// defender's lines are never one template.
	inline const FName LogStanceChanged(TEXT("log_stance_changed"));
	inline const FName LogStanceLapsed(TEXT("log_stance_lapsed"));
	inline const FName LogTreatySigned(TEXT("log_treaty_signed"));
	inline const FName LogTreatyBroken(TEXT("log_treaty_broken"));
	inline const FName LogSiegeBegun(TEXT("log_siege_begun"));
	inline const FName LogSiegeOnUs(TEXT("log_siege_on_us"));
	inline const FName LogSiegeTaken(TEXT("log_siege_taken"));
	inline const FName LogSiegeLost(TEXT("log_siege_lost"));
	inline const FName LogSiegeHeld(TEXT("log_siege_held"));
	inline const FName LogSiegeFailed(TEXT("log_siege_failed"));
	inline const FName LogSiegeLifted(TEXT("log_siege_lifted"));

	// DT_LogTemplates: B4 (header section 2.6, 2.7, 3.6). The sign of a claim that joined a sphere - told below the third
	// rung before the fallen's first-person line (UCosmicLogSubsystem::PushWithSign); the ruled people's fleet worn at its
	// target's first conflict stage.
	inline const FName LogSignClaim(TEXT("log_sign_claim"));
	inline const FName LogFleetWorn(TEXT("log_fleet_worn"));

	// DT_LogTemplates: B5 [B-5d1] (header section 3.3, decision 11). The narrator's record of a people's irreversible loss - the
	// first part of the two books, Log.Record, {System} alone: a settled system destroyed, a system joined to a fallen sphere by
	// a claim. The crisis stage tells it right after the expiry line (before a claim's sign and first person), and the log keeps
	// it untold whatever the scope (UCosmicLogSubsystem::PushVariants). Every people's loss, whatever its faith traits.
	inline const FName LogRecordDestroyed(TEXT("log_record_destroyed"));
	inline const FName LogRecordAnnexed(TEXT("log_record_annexed"));

	// DT_LogTemplates: B5 [B-5d1] (header section 2.2, 3.3, decision 12-da). The tenth stage's lines of the calling - Log.Calling,
	// Severity 2, the narrator's, in the third person by the vessel's name ({Player} is passed in: CosmicChronicle::VesselNameOf).
	// The death line of a death whose cause is Death.Hazard ({Player}, {Hazard}; no focus - UPlayerRoleSubsystem::Die, told
	// before what the death changes; a cause with no row of its own is not told), and the inauguration line of a succession
	// ({Player}, {Civ}, {System}; UPlayerRoleSubsystem::Succeed, after Install) - the line that opens the called life's chapter
	// (CosmicChronicle::OpensCalledChapter). The inauguration line's second row ({Player}, {Civ}; no {System}, no focus system):
	// told in its place when this tick's log already holds a fallen's first person (Log.Claim) about the new life's birth system
	// (UCosmicLogSubsystem::HasClaimLineThisTick) - no narrator's line names that system after it on the tick (docs/70 section 8.3
	// rule 5); the story layer's choice alone, one tell number either way, nothing of the world.
	inline const FName LogDeathHazard(TEXT("log_death_hazard"));
	inline const FName LogInauguration(TEXT("log_inauguration"));
	inline const FName LogInaugurationUnnamed(TEXT("log_inauguration_unnamed"));

	// DT_Encounters: the first contact card (B3 header section 2.9). NoteMet writes Knowing.PendingContactCivID at the
	// first meeting of the run while this row exists; the end of the knowing stage puts it after a Standing card that came to hold in the tick, before the periodic slot, while no petition waits.
	inline const FName PetitionFirstContact(TEXT("petition_first_contact"));

	// DT_Encounters: the two pressure cards (B4 header section 3.3) - the first a Standing row on a claim from tick 1,800,
	// the second put by its discernment. The balance commandlet counts them; the policy knows them by their effects.
	inline const FName PetitionPressureClaim(TEXT("petition_pressure_claim"));
	inline const FName PetitionPressureMeasured(TEXT("petition_pressure_measured"));

	// DT_Encounters: the cards whose title speaks to the reader or exalts the vessel - the throne card's question and the
	// exaltation card (B5 header section 4, decision 12-da: no 'you' across a death, no exaltation under it) - and the
	// cards whose title, right on its own card, reads as a eulogy under a dead vessel's name: a virtue (the wise heart), a
	// memorial day (the feast), and the two 'beside' titles of the sacrifice reading header section 3.6-1 bars (the plague's
	// tending, the trembling star's watch - [B-5d1] text audit). The epitaph's last answers leave them out
	// (CosmicCallingPages::LastAnswersOf); the titles stay as they are on the cards. Tools/data/schema.json lists the same
	// rows under epitaph_leaves_out, and the checker holds every card title that speaks to the reader to that list.
	inline const FName PetitionKeepThrone(TEXT("petition_keep_throne"));
	inline const FName PetitionExalt(TEXT("petition_exalt"));
	inline const FName PetitionWiseHeart(TEXT("petition_wise_heart"));
	inline const FName PetitionFeast(TEXT("petition_feast"));
	inline const FName EncPlagueTend(TEXT("enc_plague_tend"));
	inline const FName EncStarWatch(TEXT("enc_star_watch"));
	inline const FName EpitaphLeavesOut[] = { PetitionKeepThrone, PetitionExalt, PetitionWiseHeart, PetitionFeast, EncPlagueTend, EncStarWatch };

	// DT_LogTemplates: the meeting lines (section 7.4). NoteMet tells the first for the first meeting of the run, the
	// second for every later one.
	inline const FName LogFirstContact(TEXT("log_first_contact"));
	inline const FName LogContact(TEXT("log_contact"));

	// DT_LogTemplates: the far lines that stand in for an untold Severity 2 line (section 8.1): an ascent to the third
	// rung or above, told every time, and any other event, told at most once per DT_Rules.FarLineCooldownTicks.
	inline const FName LogFarAscent(TEXT("log_far_ascent"));
	inline const FName LogFarEvent(TEXT("log_far_event"));

	// DT_LogTemplates: research breakthroughs, in the order Physics, Society, Engineering, Transcendence (section 5.4).
	inline const FName LogBreakthrough[] = {
		FName(TEXT("log_breakthrough_physics")), FName(TEXT("log_breakthrough_society")),
		FName(TEXT("log_breakthrough_engineering")), FName(TEXT("log_breakthrough_transcendence"))
	};
}
