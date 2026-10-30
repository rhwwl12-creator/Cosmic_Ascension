// Cosmic Ascension - Events layer. Design: guideline.md section 5.9, 5.10, 7.4, 9.2; docs/60_DECISIONS/headers/A2_petition_cards.md,
// B1_wanderer_loop.md section 2.5, 2.6.
// The waiting card: the ruler's petitions and the wanderer's cards (B1) - which card the knowing stage puts before the
// player, and the pure rules the answer command (UPlayerRoleSubsystem::AnswerPetition) runs on. Functions of the state,
// the index and the tables; no line, no draw.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "Core/CosmicTypes.h"
#include "Data/CosmicDefinitionRows.h"
#include "Internationalization/Text.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"

struct FCosmicDataTables;
struct FCosmicStateIndex;
struct FGalaxyState;

/** A unit a Standing petition row is looked at on: the row and its subject system (section 5.10). */
struct FCosmicPetitionUnit
{
	FName Row;

	// The settled system of a SettledSystem row; the capital of a Civilization row (INDEX_NONE when the people's capital
	// is not one of its settled systems); the system the vessel stands in for a wanderer's card (VesselSystem).
	int32 SystemIndex = INDEX_NONE;

	bool operator==(const FCosmicPetitionUnit& Other) const
	{
		return Row == Other.Row && SystemIndex == Other.SystemIndex;
	}
};

/** The subject of a wanderer's card (B1): the system the vessel stands in, and its owner - INDEX_NONE for a wild system. */
struct FCosmicVesselSubject
{
	int32 SystemIndex = INDEX_NONE;
	int32 CivIndex = INDEX_NONE;
};

/** The kind of gate that refuses a choice of a card (B4 section 2.4): a stance (B3 - the first contact card), an edict
 * (EnactEdict), the fleet's time (DelayHazard). Not saved. */
enum class ECosmicChoiceGateKind : uint8
{
	None,
	Stance,
	Edict,
	Resist,
};

/** The first refusal of a choice (CosmicPetitions::ChoiceGateOf): its kind, that kind's reason and the refused effect's
 * place in the choice's OnSuccess (the card window names its edict); None and INDEX_NONE when it is taken. */
struct FCosmicChoiceGate
{
	ECosmicChoiceGateKind Kind = ECosmicChoiceGateKind::None;
	CosmicDiplomacy::ECosmicStanceBlock Stance = CosmicDiplomacy::ECosmicStanceBlock::None;
	CosmicEdicts::ECosmicEdictBlock Edict = CosmicEdicts::ECosmicEdictBlock::None;
	CosmicConflict::ECosmicResistBlock Resist = CosmicConflict::ECosmicResistBlock::None;
	int32 EffectAt = INDEX_NONE;

	bool IsOpen() const
	{
		return Kind == ECosmicChoiceGateKind::None;
	}
};

/** What an answer to the waiting card came to. Not state. */
enum class ECosmicPetitionOutcome : uint8
{
	// Nothing happened and nothing was drawn: no card, a card of the other role, a choice out of range, a cost the
	// stockpile cannot pay, or a call from a handler.
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
	// How many answered rows a life remembers (FLifeRecord.LastChoiceRows): the periodic slot passes them over first, and
	// when that leaves no candidate only the most recent Lottery row among them (Present, MVP item 8).
	inline constexpr int32 RememberedChoices = 3;

	/** The index of the people the vessel rules while it is an embodied ruler; INDEX_NONE otherwise. */
	COSMIC_ASCENSION_API int32 RuledCivIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index);

	/** Whether Row is a wanderer's card (Subject VesselSystem - the schema binds the enc_ rows to it, B1). */
	inline bool IsWandererCard(const FEncounterDef& Row)
	{
		return Row.Subject == ECosmicEventSubject::VesselSystem;
	}

	/** Whether Row is about a place (SettledSystem or VesselSystem): its line is about that system and the waiting card keeps it. */
	inline bool IsAboutPlace(const FEncounterDef& Row)
	{
		return Row.Subject == ECosmicEventSubject::SettledSystem || Row.Subject == ECosmicEventSubject::VesselSystem;
	}

	/**
	 * The subject of a wanderer's card (B1 header section 2.5): the system an embodied wanderer that is not travelling
	 * stands in, and that system's owner when the state holds one (INDEX_NONE otherwise). Both INDEX_NONE for a ruler, a
	 * travelling vessel and the interregnum - except with bTravelAtOrigin, where a travelling vessel counts as standing in
	 * the system it left (CosmicWanderer::PlaceIndexOf: the head of a tick, section 2.4).
	 */
	COSMIC_ASCENSION_API FCosmicVesselSubject VesselSubjectOf(const FGalaxyState& State, const FCosmicStateIndex& Index, bool bTravelAtOrigin = false);

	/**
	 * Whether a wanderer's card may come on the people CivIndex: no people (a wild system), or a people at most one rung
	 * above the calling's PersonalTier - the gap-2 variants of awe and fear (pillars.md) come later. The world's tiers
	 * only: no knowing is read.
	 */
	COSMIC_ASCENSION_API bool WithinWandererGap(const FGalaxyState& State, int32 CivIndex);

	/**
	 * The first tick of a reign (B1 header section 2.7): an embodied ruler whose open reign has not counted a tick yet
	 * (Lives.Last().ReignTicks.Last() == 0 - the eighth stage counts it). The knowing stage then compares the petitions
	 * with an empty head: a petition that already stood when the vessel was crowned comes as one that just came to hold.
	 */
	COSMIC_ASCENSION_API bool IsFirstTickOfReign(const FGalaxyState& State);

	/**
	 * The first tick of a life a succession began (B5 header section 2.2): at the head of a tick (before State.Tick moves on),
	 * an embodied wanderer whose life was installed on that tick (Tick == Calling.BornTick) and is not the first. The knowing
	 * stage then compares the wanderer's cards with an empty head, as for the first tick of a reign: a hazard that already
	 * stood where the new vessel was born comes as one that just came to hold.
	 */
	COSMIC_ASCENSION_API bool IsFirstTickOfLife(const FGalaxyState& State);

	/**
	 * The subject system of a card on the people CivIndex: for SettledSystem the index of SystemID when it is a settled
	 * system of that people, for Civilization the people's capital when it is one of its settled systems; INDEX_NONE
	 * otherwise.
	 */
	COSMIC_ASCENSION_API int32 SubjectOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, ECosmicEventSubject Subject, FName SystemID);

	/** Whether Row's conditions hold on the people CivIndex and the system SystemIndex (CheckAll over the view). CivIndex
	 * may be INDEX_NONE for a wanderer's card on a wild system: the view's galaxy row is then the system's, and a condition
	 * about a people does not hold. */
	COSMIC_ASCENSION_API bool Holds(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules,
		FName RowName, const FEncounterDef& Row, int32 CivIndex, int32 SystemIndex);

	/**
	 * The Standing rows whose conditions hold now, unit by unit, rows in lexical order. For the people the vessel rules: a
	 * Civilization row once (on the capital), a SettledSystem row on each settled system of the people in OwnedSystems
	 * order (the petitions). For a wanderer that is not travelling: each wanderer's card on the system it stands in, when
	 * its owner is within the wanderer's gap (B1). Empty while a card waits, and for a travelling vessel or the
	 * interregnum. SimulateTick reads it at the head of a tick with bTravelAtOrigin: a travelling wanderer then counts as
	 * standing in the system it left, so a journey that ends where it began (its target destroyed on the way) brings no
	 * card for a crisis that stood there throughout, and a real arrival still differs by the system.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicPetitionUnit> StandingHeld(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, bool bTravelAtOrigin = false);

	/**
	 * The end of the knowing stage (section 5.10): puts one card before an embodied ruler with no card waiting. The
	 * candidates are the units of StandingHeld that are not in HeldAtHead (their conditions came to hold in this tick);
	 * when there are none and Tick - Calling.BornTick is a positive multiple of DT_Rules.PetitionPeriodTicks (below 1 read
	 * as 1), the units of the Lottery rows whose conditions hold and that are not among the last choices of the life - and
	 * when there is none, those that are not the most recent Lottery row among them (MVP item 8: two or three periodic
	 * cards against three remembered choices would otherwise silence the slot; a single one is still not put twice). One
	 * candidate is picked by BaseWeight with CosmicSeed::DeriveForName(Seed, ruled CivID, Purpose::Petition, Tick): no
	 * number of the sim stream. The last choices and the most recent Lottery row count the petitions only (B1).
	 * For a wanderer that is not travelling (B1 header section 2.5): the units of its Standing cards that are not in
	 * HeldAtHead; when there are none and bArrived (the vessel arrived in this tick), an arrival card - a Lottery wanderer's
	 * card whose conditions hold - when u < DT_Rules.EncounterChancePerArrival (u = DeriveForName(Seed, the vessel's
	 * SystemID, Purpose::Encounter, Tick) / 2^32; the share read clamped to 0..1, not a number as 0), picked by weight with
	 * u / share. The life's most recent answered wanderer's card is not put again; a Standing card on a people beyond the
	 * wanderer's gap is not put (the arrival card speaks of the place alone and comes there too). Writes Knowing.PendingPetitionRow and PendingPetitionSystemID (NAME_None for a Civilization row, the
	 * vessel's system for a wanderer's card) and nothing else. True when a card was put.
	 * For a ruler, the first contact card (B3 header section 2.9): while Knowing.PendingContactCivID names a people still in
	 * Civs and the row CosmicRows::PetitionFirstContact exists and holds, it is put when no Standing unit came to hold in
	 * that tick - after such a unit (its slot waits one answer; the unit would not come again), before the periodic slot -
	 * with no draw, no period and no last-choice rule.
	 */
	COSMIC_ASCENSION_API bool Present(FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules,
		TConstArrayView<FCosmicPetitionUnit> HeldAtHead, bool bArrived = false);

	/**
	 * Whether the waiting card still stands: the row exists, a vessel of its role is embodied (a ruler for a petition; for a
	 * wanderer's card a wanderer that is not travelling and stands in the card's system), its subject is still there (a
	 * SettledSystem row's system is still a settled system of the ruled people) and the row's conditions hold on it. The
	 * first contact card also needs the people of Knowing.PendingContactCivID in Civs (B3).
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
	 * ST_CA Role.Rebel - section 8.2's fallback of a role slot) and {OtherCiv} (the name of the people OtherCivID - the
	 * first contact card's, B3). With bNamesOther (the first contact card) a people not in Civs - gone, or its slot cleared
	 * by the crossing stage's cleanup while the card waits - is ST_CA Petition.OtherGone; without it {OtherCiv} is empty
	 * words.
	 */
	COSMIC_ASCENSION_API FFormatNamedArguments ArgsOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex, int32 SystemIndex,
		FName OtherCivID = NAME_None, bool bNamesOther = false);

	/** The stance gate a choice of a card about the people OtherCivID meets (B3: the first contact card): the first refusal
	 * of CosmicDiplomacy::StanceBlockOf (bMet true, as the effect asks) of a SetStance in its OnSuccess on the context's
	 * other people (an empty Key) - None when it would be taken, or is held already (Same), or names no other people. The
	 * card greys such a choice, the answer refuses it and the policy passes it by. */
	COSMIC_ASCENSION_API CosmicDiplomacy::ECosmicStanceBlock StanceBlockOfChoice(const FGalaxyState& State, const FCosmicStateIndex& Index,
		const FCosmicRulesDef& Rules, int32 CivIndex, FName OtherCivID, const FEncounterChoice& Choice);

	/**
	 * The gate of a choice of a card (B4 section 2.4, rule 13): the first refusal among the effects of its OnSuccess, in
	 * their order, for the people CivIndex and the card's system SystemIndex - a SetStance as StanceBlockOfChoice reads it
	 * (Same is taken), an EnactEdict by CosmicEdicts::EnactBlockOf on its Key (Held is a refusal too: a choice that would
	 * change nothing draws no number and tells no line), a DelayHazard by CosmicConflict::ResistBlockOf on its Tag. The
	 * answer refuses such a choice before its draw, the card greys it with the reason and the policy passes it by - one
	 * function. Kind None when every gate is open (a choice with no gated effect always is).
	 */
	COSMIC_ASCENSION_API FCosmicChoiceGate ChoiceGateOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex, FName OtherCivID, const FEncounterChoice& Choice);

	/** Takes the waiting card away: both fields of Knowing (PendingPetitionRow, PendingPetitionSystemID). It never touches
	 * Knowing.PendingContactCivID: every answer calls it, and a first contact card waiting behind another petition stays. */
	COSMIC_ASCENSION_API void Clear(FGalaxyState& State);

	/** Records an answered row in the open life: the end of Calling.Lives.Last().LastChoiceRows, the last three kept. */
	COSMIC_ASCENSION_API void Remember(FGalaxyState& State, FName RowName);
}
