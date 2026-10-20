// Cosmic Ascension - Events layer. Design: guideline.md section 5.9, 5.10, 7.4, 9.2; docs/60_DECISIONS/headers/A2_petition_cards.md.
// The ruler's petition cards: which card the knowing stage puts before the player, and the pure rules the answer command
// (UPlayerRoleSubsystem::AnswerPetition) runs on. Functions of the state, the index and the tables; no line, no draw.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "Core/CosmicTypes.h"
#include "Data/CosmicDefinitionRows.h"
#include "Internationalization/Text.h"

struct FCosmicDataTables;
struct FCosmicStateIndex;
struct FGalaxyState;

/** A unit a Standing petition row is looked at on: the row and its subject system (section 5.10). */
struct FCosmicPetitionUnit
{
	FName Row;

	// The settled system of a SettledSystem row; the capital of a Civilization row (INDEX_NONE when the people's capital
	// is not one of its settled systems).
	int32 SystemIndex = INDEX_NONE;

	bool operator==(const FCosmicPetitionUnit& Other) const
	{
		return Row == Other.Row && SystemIndex == Other.SystemIndex;
	}
};

/** What an answer to the waiting card came to. Not state. */
enum class ECosmicPetitionOutcome : uint8
{
	// Nothing happened and nothing was drawn: no card, no ruler, a choice out of range, a cost the stockpile cannot pay,
	// or a call from a handler.
	Refused,
	// The card no longer stood (its conditions on its subject): it was taken away. One number was drawn; no cost, no
	// effect, no line.
	Withdrawn,
	Succeeded,
	Failed
};

/** The result of UPlayerRoleSubsystem::AnswerPetition, for the card that shows it. */
struct FCosmicPetitionAnswer
{
	ECosmicPetitionOutcome Outcome = ECosmicPetitionOutcome::Refused;

	// The number drawn for the answer, 0..5; INDEX_NONE when it was refused.
	int32 Roll = INDEX_NONE;

	// The outcome line as it was told (SuccessText or FailureText with the card's arguments); empty when the choice has
	// none, and when the answer was refused or withdrawn.
	FText Line;
};

namespace CosmicPetitions
{
	// How many answered rows a life remembers (FLifeRecord.LastChoiceRows): the periodic slot does not put them again.
	inline constexpr int32 RememberedChoices = 3;

	/** The index of the people the vessel rules while it is an embodied ruler; INDEX_NONE otherwise. */
	COSMIC_ASCENSION_API int32 RuledCivIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index);

	/**
	 * The subject system of a card on the people CivIndex: for SettledSystem the index of SystemID when it is a settled
	 * system of that people, for Civilization the people's capital when it is one of its settled systems; INDEX_NONE
	 * otherwise.
	 */
	COSMIC_ASCENSION_API int32 SubjectOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, ECosmicEventSubject Subject, FName SystemID);

	/** Whether Row's conditions hold on the people CivIndex and the system SystemIndex (CheckAll over the view). */
	COSMIC_ASCENSION_API bool Holds(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules,
		FName RowName, const FEncounterDef& Row, int32 CivIndex, int32 SystemIndex);

	/**
	 * The Standing rows whose conditions hold now for the people the vessel rules, unit by unit: rows in lexical order, a
	 * Civilization row once (on the capital), a SettledSystem row on each settled system of the people in OwnedSystems
	 * order. Empty while no ruler is embodied and while a card waits. SimulateTick reads it at the head of a tick.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicPetitionUnit> StandingHeld(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules);

	/**
	 * The end of the knowing stage (section 5.10): puts one card before an embodied ruler with no card waiting. The
	 * candidates are the units of StandingHeld that are not in HeldAtHead (their conditions came to hold in this tick);
	 * when there are none and Tick - Calling.BornTick is a positive multiple of DT_Rules.PetitionPeriodTicks (below 1 read
	 * as 1), the units of the Lottery rows whose conditions hold and that are not among the last choices of the life. One
	 * candidate is picked by BaseWeight with CosmicSeed::DeriveForName(Seed, ruled CivID, Purpose::Petition, Tick): no
	 * number of the sim stream. Writes Knowing.PendingPetitionRow and PendingPetitionSystemID (NAME_None for a
	 * Civilization row) and nothing else. True when a card was put.
	 */
	COSMIC_ASCENSION_API bool Present(FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules,
		TConstArrayView<FCosmicPetitionUnit> HeldAtHead);

	/**
	 * Whether the waiting card still stands: a ruler is embodied, the row exists, its subject is still there (a
	 * SettledSystem row's system is still a settled system of the ruled people) and the row's conditions hold on it.
	 */
	COSMIC_ASCENSION_API bool StillStands(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/**
	 * The value a choice is checked on (section 5.9, 16 #30 (1)): a stat of the vessel (0..10), or the ruled people's
	 * Stability as (int32)(Stability * 10 + 0.0001) (0..10 - a Stability written as 0.7 reads 7); 0 for NoCheck.
	 */
	COSMIC_ASCENSION_API int32 CheckValueOf(const FGalaxyState& State, int32 CivIndex, ECosmicCheckStat Stat);

	/** Whether the roll 0..5 passes Choice: always for NoCheck, else CheckValueOf + Roll >= Difficulty (16 never passes). */
	COSMIC_ASCENSION_API bool Passes(const FGalaxyState& State, int32 CivIndex, const FEncounterChoice& Choice, int32 Roll);

	/** How many of the six rolls pass Choice, 0..6 (6 for NoCheck): what the card shows beside the choice. */
	COSMIC_ASCENSION_API int32 PassingRollsOf(const FGalaxyState& State, int32 CivIndex, const FEncounterChoice& Choice);

	/** The cost of Choice as the answer pays it from the ruled people's stockpile: a member below 0 or not finite reads as 0. */
	COSMIC_ASCENSION_API FResourcePool CostOf(const FEncounterChoice& Choice);

	/**
	 * The arguments of a card's texts on the people CivIndex and the system SystemIndex: {Civ}, {System}, {Hazard} (the
	 * display name of the hazard the system holds), {Rebel} (the name of the people's open Notable.Rebel record, else
	 * ST_CA Role.Rebel - section 8.2's fallback of a role slot).
	 */
	COSMIC_ASCENSION_API FFormatNamedArguments ArgsOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex, int32 SystemIndex);

	/** Takes the waiting card away: both fields of Knowing. */
	COSMIC_ASCENSION_API void Clear(FGalaxyState& State);

	/** Records an answered row in the open life: the end of Calling.Lives.Last().LastChoiceRows, the last three kept. */
	COSMIC_ASCENSION_API void Remember(FGalaxyState& State, FName RowName);
}
