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
