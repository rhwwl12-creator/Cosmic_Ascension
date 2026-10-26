// Cosmic Ascension - Sim layer (Core Logic). Design: docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md section 2.2,
// docs/10_DESIGN/systems.md section 5.5.
// The single home of the diplomacy rules: the pair records (FCivilizationState::Ties) and their normal form, the border
// between two peoples, the relation's drift target and the view a people holds, the friendly slots, and the gates of a
// stance and of a treaty. Pure functions of the state and the tables - no draw, no state of their own. The commands
// (UPlayerRoleSubsystem::SetStance / ProposeTreaty / BreakTreaty), the diplomacy and AI stages, the peoples tab and the
// commandlet's policy read them. The world never reads knowing: whether a people is met is the caller's argument.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FCivilizationState;
struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FCosmicTie;
struct FEthicsAxes;
struct FGalaxyState;

namespace CosmicDiplomacy
{
	/** Why a stance cannot be set, in the order StanceBlockOf asks (the first reason wins). Not a UENUM: never saved,
	 * hashed or a data column. NotRuler and Handler are the command's own. */
	enum class ECosmicStanceBlock : uint8
	{
		None,
		NotRuler,       // the command: no embodied ruler with a ruled people
		NoCiv,          // OtherID is not a living people
		Self,           // the people itself
		NotMet,         // the ruled people has not met it (the caller's bMet)
		Same,           // it already holds that stance
		FriendlySlots,  // friendly: every friendly slot is taken
		Tier,           // hostile: below the second tier (no war in the first ten minutes)
		NonAggression,  // hostile: the pair holds a non-aggression treaty
		Handler,        // the command was called from a handler of a Core Logic delegate (an error line)
	};

	/** Why a treaty cannot be signed, in the order TreatyBlockOf asks. Not a UENUM. */
	enum class ECosmicTreatyBlock : uint8
	{
		None,
		NotRuler,
		NoCiv,
		Self,
		NotMet,
		Held,       // the pair already holds it
		Tier,       // the proposer is below the second tier
		Hostile,    // either side is hostile toward the other
		Refused,    // the other people's view of the proposer (ViewOf) is below the bar
		Cost,       // the proposer cannot pay DT_Rules.TreatyCostInfluence
		Handler,
	};

	/** The tie of Civ toward OtherID, or nullptr (neutral, no treaty). */
	COSMIC_ASCENSION_API const FCosmicTie* TieOf(const FCivilizationState& Civ, FName OtherID);

	/** Civ's stance toward OtherID: Stance.Friendly, Stance.Hostile or the empty tag (neutral). */
	COSMIC_ASCENSION_API FGameplayTag StanceOf(const FCivilizationState& Civ, FName OtherID);

	COSMIC_ASCENSION_API bool HasTreaty(const FCivilizationState& Civ, FName OtherID, FGameplayTag Treaty);

	/** The treaties Civ holds with OtherID. */
	COSMIC_ASCENSION_API int32 TreatyCountOf(const FCivilizationState& Civ, FName OtherID);

	/** The trade treaties Civ holds with every partner (the economy stage's income). */
	COSMIC_ASCENSION_API int32 TradeTreatyCountOf(const FCivilizationState& Civ);

	/** The peoples Civ is friendly toward. */
	COSMIC_ASCENSION_API int32 FriendlyCountOf(const FCivilizationState& Civ);

	/** DT_Rules.FriendlySlotsByTier[Civ.Tier] (out of range = 0, below 0 = 0). */
	COSMIC_ASCENSION_API int32 FriendlySlotsOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules);

	/** Sets Civ's stance toward OtherID, keeping the normal form (an entry that becomes neutral with no treaty is removed). */
	COSMIC_ASCENSION_API void SetStanceOf(FCivilizationState& Civ, FName OtherID, FGameplayTag Stance);

	/** Adds Treaty to both peoples' ties - the one way a treaty is made, so the pair stays symmetric. */
	COSMIC_ASCENSION_API void Sign(FGalaxyState& State, int32 A, int32 B, FGameplayTag Treaty);

	/** Removes Treaty from both peoples' ties (normal form kept). False when the pair did not hold it. */
	COSMIC_ASCENSION_API bool Break(FGalaxyState& State, int32 A, int32 B, FGameplayTag Treaty);

	/** Removes CivID's key from every people's Ties (the crossing stage, where it removes the Relations keys). */
	COSMIC_ASCENSION_API void ForgetCiv(FGalaxyState& State, FName CivID);

	/** Whether a settled system of A and a settled system of B are lane neighbours (CosmicState::IsSettled - the adjacency
	 * predicate of rule 13). Lanes only join systems of one galaxy row; the sanctuary is no one's. */
	COSMIC_ASCENSION_API bool Borders(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 A, int32 B);

	/** Borders(A, B) for every B at once: Out[B] (one entry per element of State.Civs; false for A itself) - one walk of A's
	 * settled systems instead of one a pair (the diplomacy stage's drift and the AI stage's scan, B3). */
	COSMIC_ASCENSION_API void BordersOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 A, TArray<bool>& Out);

	/** For every people (Civs order), the peoples it borders, in Civs order (the commandlet's border pairs; the diplomacy
	 * and AI stages walk BordersOf). */
	COSMIC_ASCENSION_API TArray<TArray<int32>> BorderTableOf(const FGalaxyState& State, const FCosmicStateIndex& Index);

	/** (|dPacifism| + |dSpiritualism| + |dOpenness|) / 3, in 0..2. */
	COSMIC_ASCENSION_API float EthicsDistanceOf(const FEthicsAxes& A, const FEthicsAxes& B);

	/** The drift target of Viewer's relation toward Other: -RelationEthicsWeight x distance + the term of the stance Other
	 * holds toward Viewer (RelationStanceTerms) + RelationTreatyTerm x the pair's treaties. It reads no relation. */
	COSMIC_ASCENSION_API float TargetRelationOf(const FGalaxyState& State, const FCosmicRulesDef& Rules, int32 Viewer, int32 Other);

	/** The view Viewer holds of Other: the stored Relations[Other] when the two border each other, else the target without
	 * the stance term (a pair that does not border does not drift, so neither the default 0 nor a stance read at once may
	 * pass a bar). The acceptance of a treaty, the peoples tab, the AI's hostility and the policy read this. */
	COSMIC_ASCENSION_API float ViewOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 Viewer, int32 Other);

	/** The word band of a view: 0 hostile (<= -0.5), 1 cold (<= -0.2), 2 indifferent (< 0.2), 3 warm (< 0.5), 4 trusting. */
	COSMIC_ASCENSION_API int32 ViewBandOf(float View);

	/** The sum of the TreatyAcceptBonus of the edicts Civ holds (values above 0 and finite). */
	COSMIC_ASCENSION_API float TreatyAcceptBonusOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables);

	/** The view another people must hold of Civ to take its treaty: DT_Rules.TreatyAcceptRelation (0 when not finite) less
	 * TreatyAcceptBonusOf - the bar TreatyBlockOf asks, and the one the peoples tab tells. */
	COSMIC_ASCENSION_API float TreatyBarOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/** The influence a treaty asks of its proposer: DT_Rules.TreatyCostInfluence when above 0 and finite, else 0. */
	COSMIC_ASCENSION_API float TreatyCostOf(const FCosmicRulesDef& Rules);

	/** The first reason CivIndex cannot set Stance (Friendly, Hostile or empty) toward OtherID, NoCiv .. NonAggression. */
	COSMIC_ASCENSION_API ECosmicStanceBlock StanceBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
		int32 CivIndex, FName OtherID, FGameplayTag Stance, bool bMet);

	/** The first reason CivIndex cannot sign Treaty with OtherID, NoCiv .. Cost. */
	COSMIC_ASCENSION_API ECosmicTreatyBlock TreatyBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 CivIndex, FName OtherID, FGameplayTag Treaty, bool bMet);

	/** Turns CivIndex hostile toward Other and breaks the pair's trade treaty (header section 2.2): when it held one, the other
	 * people's view of CivIndex falls by Penalty at once (the ruler's command passes BetrayalRelationPenalty, the AI 0). The
	 * gate is the caller's. True when a trade treaty was broken. */
	COSMIC_ASCENSION_API bool TurnHostile(FGalaxyState& State, int32 CivIndex, int32 Other, float Penalty);

	/** The display name of a stance - ST_CA Stance.Friendly, Stance.Hostile, or Stance.Neutral for the empty tag (the
	 * {Stance} slot of a line). */
	COSMIC_ASCENSION_API FText StanceNameOf(FGameplayTag Stance);

	/** The display name of a treaty - ST_CA Treaty.Trade or Treaty.NonAggression (the {Treaty} slot of a line). */
	COSMIC_ASCENSION_API FText TreatyNameOf(FGameplayTag Treaty);

	/** Lowers Viewer's relation toward Other by Amount at once ([-1, 1]; an Amount that is not above 0 and finite does
	 * nothing - and writes no entry). */
	COSMIC_ASCENSION_API void LowerRelation(FCivilizationState& Viewer, FName OtherID, float Amount);
}
