// Cosmic Ascension - Sim layer (Core Logic). Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.5
// (decisions 12, 13, 14, 15).
// The single home of the binding's world rules (rule 13): which fallen one a binding would hold now, and the one place that
// writes FTranscendentState.bSealed and SealedTick. Pure functions of the state and the tables - no draw, no line, no knowing
// read (rule 2). The construction row project_binding (CosmicRows::ProjectBinding) asks BindTargetOf at the start
// (CosmicMegaprojects::StartBlockOf - NoFallen) and on every tick of its construction slot before it pays
// (CosmicMegaprojects::Advance), and its completion calls Bind on the target it asked for in that same call.
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"

struct FCosmicDataTables;
struct FGalaxyState;

namespace CosmicSeal
{
	/**
	 * The fallen one a binding would hold now (header section 2.5): the first instance of State.Transcendents, in array
	 * order, whose DT_Transcendents row is of Realm.Abyssal and which is not sealed; INDEX_NONE when none is left (a run
	 * holds one fallen in the vertical slice - DT_Rules.FallenTranscendentCount -, so the first binding of a run is its
	 * last). An instance whose row the tables do not hold is passed by, as the pressure passes it by. Reads only; draws
	 * nothing.
	 */
	COSMIC_ASCENSION_API int32 BindTargetOf(const FGalaxyState& State, const FCosmicDataTables& Tables);

	/**
	 * Seals the instance TranscendentIndex at Tick: bSealed = true and SealedTick = Tick, the one writer of the two fields
	 * (docs/50 - they are written in one place). Its sphere stays as it is - the binding freezes it, it does not clear it;
	 * every reader of bSealed (the pressure, the claimant, the awareness, the settlement's sphere refusal ...) treats the
	 * instance as held from the next read on. False and nothing written for an index out of range or an instance already
	 * sealed (a seal is never moved: SealedTick keeps the first binding's tick).
	 */
	COSMIC_ASCENSION_API bool Bind(FGalaxyState& State, int32 TranscendentIndex, int64 Tick);
}
