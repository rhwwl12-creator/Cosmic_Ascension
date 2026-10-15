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

	// DT_Transcendents: Silence, the one fallen transcendent every run has. The roster of NewGalaxy starts from it
	// (section 4.2).
	inline const FName TransSilence(TEXT("trans_silence"));

	// DT_Megaprojects: settling. A system whose ActiveMegaproject is this row is owned but not yet settled (section 5.7).
	inline const FName ProjectSettle(TEXT("project_settle"));

	// DT_Events: the book is found. The research stage fires it at every Society breakthrough; the row's conditions
	// pick the people whose book is shut (section 5.11).
	inline const FName EventBookFound(TEXT("event_book_found"));

	// DT_LogTemplates: the first line of a run (section 2.4).
	inline const FName LogFirstLife(TEXT("log_first_life"));

	// DT_LogTemplates: research breakthroughs, in the order Physics, Society, Engineering, Transcendence (section 5.4).
	inline const FName LogBreakthrough[] = {
		FName(TEXT("log_breakthrough_physics")), FName(TEXT("log_breakthrough_society")),
		FName(TEXT("log_breakthrough_engineering")), FName(TEXT("log_breakthrough_transcendence"))
	};
}
