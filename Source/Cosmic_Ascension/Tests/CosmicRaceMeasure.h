// Cosmic Ascension - the balance commandlet's race measures (B6b [B-6b3b], header section 5.1, notes section 2.1): the pure
// predicates the race line, the race check and the claims file read, in one header so that one test holds each of them
// (Cosmic_Ascension.Sim.RaceMeasure; BrakeHeldOf in Sim.FourthRungBrake) and a mutation of one breaks it. Each reads the state
// and the tables only - no draw, no write to the simulation, never Knowing - and asks the game's own windows (EndingOf,
// FourthRungOpenOf, the static CheckAll, IsGalaxyLaw) rather than copying a rule; two copies, each pinned: FourthRungCountOf to
// its window by Sim.RaceMeasure, and BrakeHeldOf's walk of the judgment (JudgePeople's) to the judgment by Sim.FourthRungBrake.
// Not behind WITH_DEV_AUTOMATION_TESTS (the commandlet builds in every target), so nothing of Tests/CosmicTestHelpers.h.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicEventSubsystem.h"
#include "Sim/CosmicAscent.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicStateIndex.h"

namespace CosmicRaceMeasure
{
	// One people of the home galaxy across a tick (the race check): its rung at the boundary before the tick and after it (a
	// people gone in the tick keeps After = Before), and whether it is the people the feat judgment judged first in the tick
	// (the ruled people of UAscensionSubsystem::EvaluateFeats - the brake never holds it).
	struct FRungStep
	{
		FName CivID;
		EAscensionTier Before = EAscensionTier::Tier1_Planetary;
		EAscensionTier After = EAscensionTier::Tier1_Planetary;
		bool bRuled = false;
	};

	// The race check of one tick (header section 5.1): asked when the cap is above 0 and a people other than the judged ruled
	// one climbed to the fourth rung in the tick; it holds when the peoples on the fourth rung or above before the tick plus
	// those that climbed to it in the tick - the ruled people counted, the judgment sees it first - stay at or below the cap.
	// The count before the tick is the one the judgment began from: Civs shrinks only at the crossing stage, after the
	// judgment, and no rung goes down.
	struct FFourthRungCheck
	{
		bool bAsked = false;
		int32 Before = 0;
		int32 Climbed = 0;
		int32 Cap = 0;

		bool Holds() const
		{
			return !bAsked || Before + Climbed <= Cap;
		}
	};

	inline FFourthRungCheck FourthRungCheckOf(TConstArrayView<FRungStep> Steps, int32 Cap)
	{
		constexpr int32 Fourth = static_cast<int32>(EAscensionTier::Tier4_Transcendent);
		FFourthRungCheck Check;
		Check.Cap = Cap;
		for (const FRungStep& Step : Steps)
		{
			const bool bStood = static_cast<int32>(Step.Before) >= Fourth;
			const bool bClimbed = !bStood && static_cast<int32>(Step.After) >= Fourth;
			Check.Before += bStood ? 1 : 0;
			Check.Climbed += bClimbed ? 1 : 0;
			Check.bAsked |= Cap > 0 && bClimbed && !Step.bRuled;
		}
		return Check;
	}

	// The peoples of the galaxy GalaxyRow on the fourth rung or above, the ruled people among them: T4Civs of docs/21 section
	// 7.4 without Inbound (the crossings, empty until Phase 6a). A copy of the count inside CosmicAscent::FourthRungOpenOf
	// (Sim/CosmicAscent.cpp), not moved into the game - the b6b2c_brake_* mutations patch that loop -, pinned by Sim.RaceMeasure;
	// 6a, which fills Transits, revisits it.
	inline int32 FourthRungCountOf(const FGalaxyState& State, FName GalaxyRow)
	{
		int32 Count = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Count += (Civ.GalaxyRow == GalaxyRow && static_cast<int32>(Civ.Tier) >= static_cast<int32>(EAscensionTier::Tier4_Transcendent)) ? 1 : 0;
		}
		return Count;
	}

	// The end of the fifth rung that stands now: CosmicEndings::EndingOf when its kind is RuledPeople, Vessel or OtherPeople,
	// an empty record otherwise. EndingOf ranks the silence and the clock after the three, so a fifth rung that stands is never
	// hidden by them - the first boundary this answers is the first fifth-rung end, whatever the record holds.
	inline FCosmicEndingRecord FifthRungEndOf(const FGalaxyState& State, const FCosmicRulesDef& Rules)
	{
		const FCosmicEndingRecord End = CosmicEndings::EndingOf(State, Rules);
		const bool bFifth = End.Kind == CosmicTags::EndingRuledPeople() || End.Kind == CosmicTags::EndingVessel() || End.Kind == CosmicTags::EndingOtherPeople();
		return bFifth ? End : FCosmicEndingRecord();
	}

	// A claim's end of five (B4 [B-4d2], the commandlet's pressure line; the claims file's EndKind): 0 joined (a sphere took the
	// system), 1 eased (the vigil's line of the tick), 2 relief (its answer completed), 4 expired (its count ran out, not by an
	// edict), 3 another end - in that order of precedence.
	inline int32 ClaimEndOf(bool bJoined, bool bEased, bool bAnswered, bool bExpired)
	{
		return bJoined ? 0 : bEased ? 1 : bAnswered ? 2 : bExpired ? 4 : 3;
	}

	// The word of an end of ClaimEndOf, and of the two the claims file adds: 5 input (gone at an input - B4 rule 8' forbids it),
	// 6 standing (still standing when the run ends). "-" for any other.
	inline const TCHAR* ClaimEndNameOf(int32 End)
	{
		static const TCHAR* const Names[] = { TEXT("joined"), TEXT("eased"), TEXT("relief"), TEXT("other"), TEXT("expired"), TEXT("input"), TEXT("standing") };
		return (End >= 0 && End < static_cast<int32>(UE_ARRAY_COUNT(Names))) ? Names[End] : TEXT("-");
	}

	// Whether the brake on the fourth rung held the people CivIndex at this boundary (notes section 2.1): a people the player
	// does not rule, on the third rung, its galaxy's fourth rung closed (CosmicAscent::FourthRungOpenOf), and some DT_Feats row
	// the judgment would ask of it (IsJudgedRulerRowAt the third rung, in GetFeatRowNames order) holding on one of its settled
	// systems (OwnedSystems order, the owner's own, CosmicState::IsSettled) by the static UCosmicEventSubsystem::CheckAll on the
	// view the judgment builds (UAscensionSubsystem::JudgePeople, SinceTick 0 for a people not ruled). This is the judgment's
	// walk without Ascend. It reads the state at the boundary after the tick, not in the middle of the judgment (a climb later
	// in that tick's judgment is already counted). Sim.FourthRungBrake is the test that holds the two together.
	inline bool BrakeHeldOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex)
	{
		if (!State.Civs.IsValidIndex(CivIndex))
		{
			return false;
		}
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (Civ.bIsPlayerRuled || Civ.Tier != EAscensionTier::Tier3_Causal || CosmicAscent::FourthRungOpenOf(State, Tables, Civ.GalaxyRow))
		{
			return false;
		}
		TArray<int32, TInlineAllocator<8>> Settled;
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = Index.FindSystem(State, SystemID);
			if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID && CosmicState::IsSettled(State.Systems[SystemIndex]))
			{
				Settled.Add(SystemIndex);
			}
		}
		for (const FName FeatRow : Tables.GetFeatRowNames())
		{
			const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
			if (!Feat || !Feat->IsJudgedRulerRowAt(EAscensionTier::Tier3_Causal))
			{
				continue;
			}
			for (const int32 SystemIndex : Settled)
			{
				FCosmicEvalView View(State, Index, Tables, Rules);
				View.CivIndex = CivIndex;
				View.SystemIndex = SystemIndex;
				View.GalaxyRow = Civ.GalaxyRow;
				View.SourceRow = FeatRow;
				View.SinceTick = 0;
				if (UCosmicEventSubsystem::CheckAll(Feat->CompletionConditions, View))
				{
					return true;
				}
			}
		}
		return false;
	}

	// One people's galaxy laws at the boundary before a tick (CosmicEdicts::IsGalaxyLaw - the laws of the other peoples that
	// reach a system's hooks), with its galaxy.
	struct FPeopleLaws
	{
		FName GalaxyRow;
		TArray<FName> Laws;
	};

	/**
	 * The peoples' galaxy laws as they stood when a hazard was placed in the tick (B6b notes section 5 [B-6b2a], the leftover of
	 * the delay a guard bought): the crisis stage places before the AI stage enacts, and after the economy stage's upkeep, so a law
	 * enacted in the tick did not reach the placement and a people the crossing stage took in the tick still held its laws.
	 * Out gets After's Systems and Civs with, for each people, every galaxy law not in its entry of Before dropped (a people
	 * with no entry - one that arrived in the tick - loses all of them; the edicts of the people's scope stay), and a minimal
	 * people (CivID, GalaxyRow, ActiveEdicts = its laws) appended, in Before's order, for each entry absent from After that held
	 * a law. False, with Out untouched, when nothing was dropped or appended (After's laws are the placement's). What it cannot
	 * see: a law that lapsed and was enacted again inside the tick, and a law the AI stage revoked (as before B6b).
	 */
	inline bool LawsAsPlacedOf(const TMap<FName, FPeopleLaws>& Before, const FGalaxyState& After, const FCosmicDataTables& Tables, FGalaxyState& Out)
	{
		TArray<FCivilizationState> Civs = After.Civs;
		bool bChanged = false;
		for (FCivilizationState& Civ : Civs)
		{
			const FPeopleLaws* Held = Before.Find(Civ.CivID);
			const int32 Dropped = Civ.ActiveEdicts.RemoveAll([&Tables, Held](FName Row) { return CosmicEdicts::IsGalaxyLaw(Tables, Row) && !(Held && Held->Laws.Contains(Row)); });
			bChanged |= Dropped > 0;
		}
		for (const TPair<FName, FPeopleLaws>& Entry : Before)
		{
			const FName CivID = Entry.Key;
			if (!Entry.Value.Laws.IsEmpty() && !After.Civs.ContainsByPredicate([CivID](const FCivilizationState& Civ) { return Civ.CivID == CivID; }))
			{
				FCivilizationState& Gone = Civs.AddDefaulted_GetRef();
				Gone.CivID = CivID;
				Gone.GalaxyRow = Entry.Value.GalaxyRow;
				Gone.ActiveEdicts = Entry.Value.Laws;
				bChanged = true;
			}
		}
		if (!bChanged)
		{
			return false;
		}
		Out.Systems = After.Systems;
		Out.Civs = MoveTemp(Civs);
		return true;
	}
}
