// Cosmic Ascension - UI layer (Presentation). Design: guideline.md section 3.2, 6.3, 7.5, 8.3.
// The top bar's 'next rung': what the next feat row of the people the vessel rules asks, read from the state as the feat
// judgment reads it (UAscensionSubsystem::JudgePeople), with the judgment's own evaluator (UCosmicEventSubsystem::Check).
// Computed from its arguments alone: no UObject, no delegate, no draw, no write, never Knowing.

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"
#include "Data/CosmicConditionTypes.h"

enum class ECosmicTimeUnit : uint8;
struct FCosmicDataTables;
struct FCosmicRulesDef;
struct FCosmicStateIndex;
struct FGalaxyState;

/** What kind of condition one part of the next rung is: the kinds the tooltip names with their own words (the game's feat
 * rows and B6b's upper rungs), and every other. A UI value: never stored. */
enum class ECosmicRungPartKind : uint8
{
	// MegaprojectComplete of a row the tables hold.
	Project,
	// CivResearchAtLeast on one of the four research fields (Physics, Society, Engineering, Transcendence).
	Research,
	// Any other condition: shown met or not, with no number (section 8.3: no pressure, no count).
	Other,
	// CivSettledSystemsAtLeast (MVP item 3): the settled systems held against those asked - a UI label, so a count of
	// systems may show (rule 11).
	Realm,
	// CivHasTreaty (B6b decision 28): the peoples the people shares the treaty with against those asked - a count of
	// peoples, not of persons (rule 11).
	Treaty,
	// CivLawsHeldTicksAtLeast (B6b decision 28): the galaxy laws held against those to hold, and once every one is held
	// the ticks still to hold them.
	Laws,
};

/** One condition of the next feat row, in row order. */
struct COSMIC_ASCENSION_API FCosmicRungPart
{
	ECosmicRungPartKind Kind = ECosmicRungPartKind::Other;

	// UCosmicEventSubsystem::Check of the condition with the judgment's view, on FCosmicNextRung::SystemIndex.
	bool bMet = false;

	// Project: the DT_Megaprojects row. Research: the field. Other: the condition's Key as written.
	FName Key;

	// Project, not met: the progress of the first slot (OwnedSystems order) of a settled system of the people that holds
	// the row (construction or response), -1 when none does. Needed: what the row needs (CosmicMegaprojects::
	// RequiredProgressOf). HazardRow: for an answer, the first DT_Hazards row in lexical order whose RulerResolutions
	// lists it; NAME_None otherwise. Realm: Progress = the settled systems the people holds, Needed = the condition's
	// Value. Treaty: Progress = the peoples the people shares the treaty with (CosmicDiplomacy::TreatyPartnersOf - the
	// evaluator's count), Needed = the condition's Value. Laws: Progress = the galaxy laws the people holds
	// (CosmicEdicts::GalaxyLawCountOf), Needed = CosmicEdicts::GalaxyLawsToHoldOf.
	float Progress = -1.f;
	float Needed = 0.f;
	FName HazardRow;

	// Laws, not met, every law held: the ticks still to hold them within this life (the condition's count less the
	// smaller of LawsHeldTicks and the ticks since the vessel's birth, as the evaluator reads it); -1 otherwise.
	int64 TicksLeft = -1;

	// Project project_binding (CosmicRows::ProjectBinding), not met: no fallen one is left to bind
	// (CosmicSeal::BindTargetOf) and no system holds this life's completion of it - the condition cannot be met in this
	// run (Rung.NoFallen).
	bool bClosed = false;

	// Project project_binding, not met: a system holds this life's completion of it, but not the one the parts are read
	// on - in the game a system the people lost, which it can take back (decision 14; Rung.Lost).
	bool bLost = false;

	// Research: the share the condition asks (its Value) and the share the people holds (Research[Key]).
	float NeededShare = 0.f;
	float Share = 0.f;
};

/** One way up from the rung of the people the vessel rules: a feat row the judgment looks at, and its parts. */
struct COSMIC_ASCENSION_API FCosmicNextRung
{
	// NAME_None when there is nothing to show: no embodied ruler of a living people it rules with a settled system, or no
	// judged row from the people's rung (the fifth has none, for the ladder has no sixth rung).
	FName FeatRow;

	// The settled system the parts are read on: of the people's settled systems in OwnedSystems order, the first on which
	// most parts hold (the judgment needs all of them on one system).
	int32 SystemIndex = INDEX_NONE;

	TArray<FCosmicRungPart> Parts;

	// Whether FeatRow is set and every part holds: then the next feat judgment lifts the people (by this row, or by one
	// before it in lexical order that holds too).
	bool AllMet() const;
};

namespace CosmicNextRung
{
	/**
	 * The ways up, read as UAscensionSubsystem::EvaluateFeats judges the people the vessel rules: only while the calling
	 * is Embodied and a Ruler of a people of Civs it rules (bIsPlayerRuled) that owns a settled system; every DT_Feats
	 * row that FAscensionFeatDef::IsJudgedRulerRowAt the people's Tier, in lexical order - the judgment's order (a Ruler
	 * row of the rung that is not one rung up is passed by without a line: the judgment reports it); each condition is
	 * UCosmicEventSubsystem::Check with the judgment's view - the people, the system, the people's GalaxyRow, SourceRow the
	 * feat row, SinceTick Calling.BornTick (a completion before the vessel's birth counts for nothing). Empty when there is
	 * nothing to show. A pure function.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicNextRung> AllOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/** The first of AllOf: the row the judgment looks at first (FeatRow NAME_None when there is none). */
	COSMIC_ASCENSION_API FCosmicNextRung Of(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules);

	/**
	 * The tooltip's words (ST_CA Rung.*): Rung.Next around the parts joined by CosmicUI::JoinList, in row order - a project
	 * as its DisplayName and, in brackets, Rung.Done when met, Rung.NoFallen when bClosed, Rung.Lost when bLost,
	 * "<progress>/<needed>" (whole numbers) while a slot holds it, Rung.WhenHazard with the DisplayName of the hazard it answers for an answer no slot
	 * holds, Rung.NotStarted otherwise; a research condition as its label (Research.<Field>), the share needed and, in
	 * brackets, the share held (HeldPercent), both in the bar's percent form, the settled systems as Rung.Realm (held and
	 * asked, whole numbers), the treaty as Rung.Treaty (peoples held and asked), the galaxy laws as Rung.Laws (held and to
	 * hold) or, while TicksLeft is above 0, Rung.LawsLeft with the ticks left as a duration in Unit - then every other
	 * condition folded into one Rung.Other with Rung.Met when all of them hold and Rung.NotMet otherwise, no number. A
	 * Laws part adds Rung.LawsReset as the last line. Empty - no tooltip - when Next has no FeatRow. Without Unit: cycles.
	 */
	COSMIC_ASCENSION_API FText TextOf(const FCosmicNextRung& Next, const FCosmicDataTables& Tables);
	COSMIC_ASCENSION_API FText TextOf(const FCosmicNextRung& Next, const FCosmicDataTables& Tables, ECosmicTimeUnit Unit);

	/**
	 * The tooltip over every way up (the top bar's, in the viewer's Unit): TextOf of the one row when there is one; with
	 * more, Rung.NextAny and then one line per row in AllOf's order - Rung.Row around the feat's Title and its parts as
	 * TextOf joins them - and Rung.LawsReset once, as the last line, when any row has a Laws part. Empty when Rungs is
	 * empty. Without Unit: cycles.
	 */
	COSMIC_ASCENSION_API FText TextOfAll(const TArray<FCosmicNextRung>& Rungs, const FCosmicDataTables& Tables);
	COSMIC_ASCENSION_API FText TextOfAll(const TArray<FCosmicNextRung>& Rungs, const FCosmicDataTables& Tables, ECosmicTimeUnit Unit);

	/**
	 * The research parts of every way up, in AllOf's order (the rows, then each row's conditions): what a research focus
	 * serves (MVP item 7). The commandlet's -Focus=1 policy focuses the field of the first one not met; the system panel's
	 * field buttons say which fields the next rung reads. Empty when Rungs is.
	 */
	COSMIC_ASCENSION_API TArray<FCosmicRungPart> ResearchFieldsOf(const TArray<FCosmicNextRung>& Rungs);

	/** Whether the tables hold a DT_Feats row the judgment looks at for a people on Tier (FAscensionFeatDef::IsJudgedRulerRowAt) -
	 * definitions only, no state (B6b decision 27: the clock tip's Chronicle.RuleRung stands on it for Tier4_Transcendent). */
	COSMIC_ASCENSION_API bool HasJudgedRowAt(const FCosmicDataTables& Tables, EAscensionTier Tier);
	/** The same, for a row that also CarriesCondition(Type) (B6b decision 28: the treaty buttons' Diplomacy.TreatyRungTip, Tier3_Causal
	 * and CivHasTreaty). */
	COSMIC_ASCENSION_API bool HasJudgedRowAskingAt(const FCosmicDataTables& Tables, EAscensionTier Tier, EConditionType Type);

	// A share as a whole percent, rounded as the top bar rounds it (FText::AsPercent: Share x 100 in float, then the
	// formatter's TruncateToHalfIfClose and half to even); 0 for a share not above 0 or not a number.
	COSMIC_ASCENSION_API int32 WholePercent(float Share);

	// The held share as the tooltip shows it: WholePercent, but a share that does not meet its condition shows at most one
	// percent under the share needed, and not below 0 (0.496 against 0.5 is 49, never 50: a share under the bar never
	// reads as reaching it).
	COSMIC_ASCENSION_API int32 HeldPercent(float Share, float NeededShare, bool bMet);
}
