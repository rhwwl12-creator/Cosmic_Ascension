// Cosmic Ascension - Sim layer. Design: guideline.md section 5.9, 5.10; docs/60_DECISIONS/headers/B1_wanderer_loop.md
// section 2.1. The wanderer's gates: whether a TravelTo or a coronation is taken, and the first reason it is not - the one
// answer the commands (UPlayerRoleSubsystem) and the panel read. Functions of the state; no line, no draw. The knowing a
// gate needs (whether the people is met, whether a card waits) comes in as an argument: the command fills it from
// State.Knowing (the input definition), the panel from FCosmicKnownView (rule 14 - the presentation reads knowing there only).
// RULE (section 6.1): files under Sim/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"

struct FCosmicStateIndex;

namespace CosmicWanderer
{
	/** The first reason a TravelTo is refused; None = taken. Not saved: the panel says the first reason in this order (the
	 * vessel's three states first, the sanctuary first among the reasons about the target). */
	enum class ETravelBlock : uint8
	{
		None,
		NotWanderer,
		Travelling,
		CardWaiting,
		Sanctuary,
		NotNeighbour,
		// Called from a handler of a Core Logic delegate (an error line).
		Handler
	};

	/** The first reason a coronation or a return is refused; None = taken. Not saved: an enumerator is appended at the end
	 * (FarAbove, B6b) while the gate asks it where its order says. */
	enum class ECrownBlock : uint8
	{
		None,
		NotWanderer,
		Travelling,
		CardWaiting,
		// The vessel does not stand in a system of that people.
		NotStanding,
		// It stands in one, being settled.
		NotSettled,
		Unmet,
		ThroneLocked,
		// Neither reverence nor a leader vacuum (a coronation; a return needs neither).
		Lacking,
		// Called from a handler of a Core Logic delegate (an error line).
		Handler,
		// B6b [B-6b3a] (header section 2.9, decision 41 - 16 #122): the people stands far above the calling's PersonalTier
		// (CosmicKnowing::IsFarAbove - the world's tiers; asked right after Unmet), a coronation and a return alike. The
		// panel shows no button for it (no greyed 'too high' - that would reveal a people the map hides).
		FarAbove
	};

	/** Travelling = Player.TravelTargetSystemID != NAME_None (the field comment): every gate, card and withdrawal reads this. */
	COSMIC_ASCENSION_API bool IsTravelling(const FGalaxyState& State);

	/** An embodied vessel that walks. */
	COSMIC_ASCENSION_API bool IsEmbodiedWanderer(const FGalaxyState& State);

	/** The index of the system an embodied wanderer counts as standing in - while it travels, the one it left
	 * (Player.CurrentSystemID); INDEX_NONE for a ruler, the interregnum and an ID the state does not hold. */
	COSMIC_ASCENSION_API int32 PlaceIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index);

	/** The index of the system the vessel stands in while it is an embodied wanderer that is not travelling; INDEX_NONE otherwise. */
	COSMIC_ASCENSION_API int32 StandingIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index);

	/** The ticks of one lane (DT_Rules.LaneTravelTicks, below 1 read as 1 - the two travel fields always stand and empty together). */
	COSMIC_ASCENSION_API int32 LaneTicksOf(const FCosmicRulesDef& Rules);

	/**
	 * TravelTo's gate: an embodied wanderer, not travelling, no card waiting, the target not the sanctuary, a living lane
	 * neighbour of the system the vessel stands in (in its Lanes, of its galaxy row, not a destroyed system). Reads no
	 * knowing: a wanderer's lane neighbours are always known (the new eye of BecomeWanderer, and every look reaches a lane).
	 */
	COSMIC_ASCENSION_API ETravelBlock TravelBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, FName TargetSystemID, bool bCardWaiting);

	/** Whether a coronation over CivID is the vessel's return to the people it left (Player.FormerCivID). */
	COSMIC_ASCENSION_API bool IsReturn(const FGalaxyState& State, FName CivID);

	/**
	 * BecomeRuler's gate on CivID, the owner of the system the vessel stands in: an embodied wanderer, not travelling, no
	 * card waiting, standing in a settled system of that people, bMet, the people not far above the calling's PersonalTier
	 * (CosmicKnowing::IsFarAbove - B6b, a return too: a vessel does not take the throne of a people the map hides), Tick >=
	 * the people's ThroneLockedUntilTick (written on every people when the vessel left a throne), and - unless it is a
	 * return (IsReturn: the lock alone, section 5.10) - ReverenceToPlayer >= DT_Rules.CoronationReverence or Stability <
	 * CoronationStabilityBelow (a value that is not a number passes neither). The rungs are the world's: no knowing is read.
	 */
	COSMIC_ASCENSION_API ECrownBlock CrownBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, FName CivID,
		bool bMet, bool bCardWaiting);
}
