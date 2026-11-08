// Cosmic Ascension - Sim layer.

#include "Sim/CosmicMegaprojects.h"

#include "Containers/ContainerAllocationPolicies.h"
#include "Core/CosmicIDs.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicSeal.h"
#include "Sim/CosmicStateIndex.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicMegaprojectsImpl
{
	// The delay above the third rung (B6b header 2.6, decision 17): DT_Rules.AIUpperRungProgressMultiplier, read by a compare
	// as AIAscensionSpeedMultiplier is (a value not above 0, or not a number, reads as 1 with one error line for each slot that
	// reads it).
	float UpperRungMultiplierOf(const FCosmicRulesDef& Rules)
	{
		const float Multiplier = Rules.AIUpperRungProgressMultiplier;
		if (!(Multiplier > 0.f && Multiplier <= TNumericLimits<float>::Max()))
		{
			UE_LOG(LogCosmic, Error, TEXT("DT_Rules.AIUpperRungProgressMultiplier %f is not above 0: read as 1"), Multiplier);
			return 1.f;
		}
		return Multiplier;
	}

	// A cost member as it is paid: below 0, not a number or not finite reads as 0 (a compare, not a CRT call - section 9.2).
	float CostOf(float Value)
	{
		return (Value > 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 0.f;
	}

	// Pays the cost of one tick of Row from Stockpile when it can be paid in full; nothing otherwise.
	bool PayInFull(FResourcePool& Stockpile, const FMegaprojectDef& Row)
	{
		const FResourcePool Cost = CosmicMegaprojects::CostPerTickOf(Row);
		if (!CosmicMegaprojects::CanPay(Stockpile, Cost))
		{
			return false;
		}
		Stockpile.Energy -= Cost.Energy;
		Stockpile.Matter -= Cost.Matter;
		Stockpile.Insight -= Cost.Insight;
		Stockpile.Influence -= Cost.Influence;
		Stockpile.Causality -= Cost.Causality;
		return true;
	}

	// The index of a system of OwnedSystems when the people CivIndex owns it and it is settled; INDEX_NONE otherwise. An
	// entry whose owner is another people counts for nothing, as in the economy stage.
	int32 SettledOwnedIndex(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, FName SystemID)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		const bool bOwnedAndSettled = State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == State.Civs[CivIndex].CivID
			&& CosmicState::IsSettled(State.Systems[SystemIndex]);
		return bOwnedAndSettled ? SystemIndex : INDEX_NONE;
	}

	// Whether the people CivIndex holds a completion of RowName on a settled system it owns since SinceTick.
	bool CompletedByPeople(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, FName RowName, int64 SinceTick)
	{
		for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
		{
			const int32 SystemIndex = SettledOwnedIndex(State, Index, CivIndex, SystemID);
			if (SystemIndex != INDEX_NONE && CosmicMegaprojects::HasCompleted(State.Systems[SystemIndex], RowName, SinceTick, State.Civs[CivIndex].CivID))
			{
				return true;
			}
		}
		return false;
	}

	// Whether RowName runs in the construction slot of a settled system the people CivIndex owns.
	bool RunningForPeople(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, FName RowName)
	{
		for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
		{
			const int32 SystemIndex = SettledOwnedIndex(State, Index, CivIndex, SystemID);
			if (SystemIndex != INDEX_NONE && State.Systems[SystemIndex].ActiveMegaproject == RowName)
			{
				return true;
			}
		}
		return false;
	}

	// The index of a system of OwnedSystems when the people CivIndex owns it, settled or being settled; INDEX_NONE
	// otherwise.
	int32 OwnedIndex(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, FName SystemID)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		return (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == State.Civs[CivIndex].CivID) ? SystemIndex : INDEX_NONE;
	}

	// Whether the project_settle row lets the people CivIndex settle: the row exists and its RequiredTier is at most the
	// people's Tier.
	bool SettleRowAllows(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex)
	{
		const FMegaprojectDef* Row = Tables.FindMegaproject(CosmicRows::ProjectSettle);
		return Row && static_cast<int32>(State.Civs[CivIndex].Tier) >= static_cast<int32>(Row->RequiredTier);
	}

	// Whether a settled system the people CivIndex owns lies one lane from Target, in Target's GalaxyRow (a foothold).
	bool HasFoothold(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, const FStarSystemState& Target)
	{
		for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
		{
			const int32 Foothold = SettledOwnedIndex(State, Index, CivIndex, SystemID);
			if (Foothold != INDEX_NONE && State.Systems[Foothold].GalaxyRow == Target.GalaxyRow && State.Systems[Foothold].Lanes.Contains(Target.SystemID))
			{
				return true;
			}
		}
		return false;
	}

	// Whether a sphere refuses Target to the people CivIndex: the people stands below Tier 3 and Target lies in the sphere
	// of an unsealed transcendent with a row (section 4.2 modifier 4). Membership, not a number: no threshold on P, and no
	// term of a vessel that happens to stand nearby.
	bool SphereRefuses(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex, const FStarSystemState& Target)
	{
		if (static_cast<int32>(State.Civs[CivIndex].Tier) >= static_cast<int32>(CosmicMegaprojects::SphereLiftTier))
		{
			return false;
		}
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			if (!Transcendent.bSealed && Tables.FindTranscendent(Transcendent.TranscendentID) && Transcendent.InfluencedSystems.Contains(Target.SystemID))
			{
				return true;
			}
		}
		return false;
	}

	// Whether SystemIndex is a target of the people CivIndex, whatever MaySettle and the row say: a wild system (no
	// owner, not a dead node, not the sanctuary) with a foothold of the people, that no sphere refuses it.
	bool IsTargetOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 CivIndex, int32 SystemIndex)
	{
		if (!State.Systems.IsValidIndex(SystemIndex))
		{
			return false;
		}
		const FStarSystemState& Target = State.Systems[SystemIndex];
		return !CosmicState::IsSanctuary(Target.SystemID) && Target.OwnerCivID.IsNone() && !CosmicHazards::IsDeadNode(Target)
			&& HasFoothold(State, Index, CivIndex, Target) && !SphereRefuses(State, Tables, CivIndex, Target);
	}

	// How many systems the people CivIndex owns (settled or being settled); bAnySettled says whether one is settled.
	int32 OwnedCount(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, bool& bAnySettled)
	{
		int32 Owned = 0;
		bAnySettled = false;
		for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
		{
			const int32 SystemIndex = OwnedIndex(State, Index, CivIndex, SystemID);
			if (SystemIndex != INDEX_NONE)
			{
				++Owned;
				bAnySettled |= CosmicState::IsSettled(State.Systems[SystemIndex]);
			}
		}
		return Owned;
	}
}

bool CosmicMegaprojects::HasCompleted(const FStarSystemState& System, FName RowName, int64 SinceTick, FName CivID)
{
	if (RowName.IsNone())
	{
		return false;
	}
	const FMegaprojectRecord* Record = System.CompletedMegaprojects.FindByPredicate([RowName](const FMegaprojectRecord& Candidate) { return Candidate.Row == RowName; });
	if (!Record || Record->Tick <= 0 || Record->Tick < SinceTick)
	{
		return false;
	}
	const FName Asked = CivID.IsNone() ? System.OwnerCivID : CivID;
	const FName Maker = Record->CivID.IsNone() ? System.OwnerCivID : Record->CivID;
	return !Asked.IsNone() && Maker == Asked;
}

FResourcePool CosmicMegaprojects::CostPerTickOf(const FMegaprojectDef& Row)
{
	using namespace CosmicMegaprojectsImpl;
	FResourcePool Cost;
	Cost.Energy = CostOf(Row.CostPerTick.Energy);
	Cost.Matter = CostOf(Row.CostPerTick.Matter);
	Cost.Insight = CostOf(Row.CostPerTick.Insight);
	Cost.Influence = CostOf(Row.CostPerTick.Influence);
	Cost.Causality = CostOf(Row.CostPerTick.Causality);
	return Cost;
}

bool CosmicMegaprojects::CanPay(const FResourcePool& Stockpile, const FResourcePool& Cost)
{
	return !(Stockpile.Energy < Cost.Energy || Stockpile.Matter < Cost.Matter || Stockpile.Insight < Cost.Insight || Stockpile.Influence < Cost.Influence
		|| Stockpile.Causality < Cost.Causality);
}

float CosmicMegaprojects::RequiredProgressOf(const FMegaprojectDef& Row)
{
	// A compare, so a value that is not a number reads as 1 (section 9.2: no CRT call).
	return (Row.RequiredProgress >= 1.f && Row.RequiredProgress <= TNumericLimits<float>::Max()) ? Row.RequiredProgress : 1.f;
}

CosmicMegaprojects::FCosmicSlotReading CosmicMegaprojects::ReadSlot(const FStarSystemState& System, ECosmicProjectSlot Slot)
{
	FCosmicSlotReading Reading;
	const bool bConstruction = Slot == ECosmicProjectSlot::Construction;
	Reading.Row = bConstruction ? System.ActiveMegaproject : System.ResponseMegaproject;
	Reading.Progress = bConstruction ? System.MegaprojectProgress : System.ResponseProgress;
	return Reading;
}

bool CosmicMegaprojects::PaidNothing(const FCosmicSlotReading& Before, const FCosmicSlotReading& After)
{
	// An unpaid tick does no arithmetic on the progress, so the compare is exact.
	return !After.Row.IsNone() && After.Row == Before.Row && After.Progress == Before.Progress;
}

CosmicMegaprojects::ECosmicStartBlock CosmicMegaprojects::StartBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName)
{
	// The refusals of the rule of section 5.7, one reason each. Each is a pure read and the rule is their conjunction, so
	// None holds exactly where the Phase 2 body of CanStart took the row (Sim.StartBlockReasons holds the two together).
	const FMegaprojectDef* Row = Tables.FindMegaproject(RowName);
	if (!Row)
	{
		return ECosmicStartBlock::NoRow;
	}
	if (RowName == CosmicRows::ProjectSettle)
	{
		return ECosmicStartBlock::SettleRow;
	}
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return ECosmicStartBlock::NoSystem;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	// Asked once, as before (it only reports a stale index).
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	if (CosmicHazards::IsDeadNode(System))
	{
		return ECosmicStartBlock::Dead;
	}
	if (!CosmicState::IsSettled(System))
	{
		return ECosmicStartBlock::NotSettled;
	}
	if (!State.Civs.IsValidIndex(OwnerIndex))
	{
		return ECosmicStartBlock::NoOwner;
	}
	// The hazard before the rung: an answer to no hazard standing here is no button at all, whatever the rung.
	const bool bAnswer = Tables.IsHazardAnswer(RowName);
	if (bAnswer)
	{
		const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
		if (!Hazard)
		{
			return ECosmicStartBlock::NoHazard;
		}
		if (!Hazard->RulerResolutions.Contains(RowName))
		{
			return ECosmicStartBlock::OtherHazard;
		}
	}
	if (static_cast<int32>(State.Civs[OwnerIndex].Tier) < static_cast<int32>(Row->RequiredTier))
	{
		return ECosmicStartBlock::RowTier;
	}
	// The binding after the rung, before the slots (B6b header 2.5): no fallen one left, nothing to start.
	if (RowName == CosmicRows::ProjectBinding && CosmicSeal::BindTargetOf(State, Tables) == INDEX_NONE)
	{
		return ECosmicStartBlock::NoFallen;
	}
	if (bAnswer)
	{
		if (System.ResponseMegaproject == RowName)
		{
			return ECosmicStartBlock::Answering;
		}
		return System.ResponseMegaproject.IsNone() ? ECosmicStartBlock::None : ECosmicStartBlock::ResponseBusy;
	}
	// A construction slot that holds another row takes this one over (section 5.7): only the same row refuses.
	return System.ActiveMegaproject == RowName ? ECosmicStartBlock::Running : ECosmicStartBlock::None;
}

bool CosmicMegaprojects::CanStart(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName)
{
	return StartBlockOf(State, Index, Tables, SystemIndex, RowName) == ECosmicStartBlock::None;
}

bool CosmicMegaprojects::Start(FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName)
{
	if (!CanStart(State, Index, Tables, SystemIndex, RowName))
	{
		return false;
	}
	FStarSystemState& System = State.Systems[SystemIndex];
	if (Tables.IsHazardAnswer(RowName))
	{
		System.ResponseMegaproject = RowName;
		System.ResponseProgress = 0.f;
	}
	else
	{
		System.ActiveMegaproject = RowName;
		System.MegaprojectProgress = 0.f;
	}
	return true;
}

TArray<TPair<int32, FName>> CosmicMegaprojects::Choices(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 CivIndex, int64 SinceTick)
{
	using namespace CosmicMegaprojectsImpl;

	TArray<TPair<int32, FName>> Result;
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return Result;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];

	// The answers: one per settled system it owns whose hazard has an answer it can take.
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = SettledOwnedIndex(State, Index, CivIndex, SystemID);
		if (SystemIndex == INDEX_NONE || !State.Systems[SystemIndex].ResponseMegaproject.IsNone())
		{
			continue;
		}
		const FHazardDef* Hazard = Tables.FindHazardByTag(State.Systems[SystemIndex].ActiveHazard);
		if (!Hazard)
		{
			continue;
		}
		for (const FName Answer : Hazard->RulerResolutions)
		{
			if (CanStart(State, Index, Tables, SystemIndex, Answer))
			{
				Result.Emplace(SystemIndex, Answer);
				break;
			}
		}
	}

	// The construction the next feat names, on the capital while its construction slot is empty.
	const int32 Capital = Index.FindSystem(State, Civ.HomeSystemID);
	if (!State.Systems.IsValidIndex(Capital) || State.Systems[Capital].OwnerCivID != Civ.CivID || !CosmicState::IsSettled(State.Systems[Capital]) || !State.Systems[Capital].ActiveMegaproject.IsNone())
	{
		return Result;
	}
	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
		// Only a row the judgment looks at (the one filter of the judgment, this list and the top bar's next rung): a row it
		// refuses would never lift the people it builds for.
		if (!Feat || !Feat->IsJudgedRulerRowAt(Civ.Tier))
		{
			continue;
		}
		for (const FCosmicCondition& Condition : Feat->CompletionConditions)
		{
			if (Condition.Type != EConditionType::MegaprojectComplete || Tables.IsHazardAnswer(Condition.Key) || CompletedByPeople(State, Index, CivIndex, Condition.Key, SinceTick)
				|| RunningForPeople(State, Index, CivIndex, Condition.Key))
			{
				continue;
			}
			if (CanStart(State, Index, Tables, Capital, Condition.Key))
			{
				Result.Emplace(Capital, Condition.Key);
				return Result;
			}
		}
	}
	return Result;
}

int32 CosmicMegaprojects::RealmCapOf(const FCosmicRulesDef& Rules, EAscensionTier Tier)
{
	const int32 TierIndex = static_cast<int32>(Tier);
	return Rules.RealmCapByTier.IsValidIndex(TierIndex) ? Rules.RealmCapByTier[TierIndex] : 0;
}

bool CosmicMegaprojects::IsSettling(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	using namespace CosmicMegaprojectsImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
	{
		const int32 SystemIndex = OwnedIndex(State, Index, CivIndex, SystemID);
		if (SystemIndex != INDEX_NONE && State.Systems[SystemIndex].ActiveMegaproject == CosmicRows::ProjectSettle)
		{
			return true;
		}
	}
	return false;
}

bool CosmicMegaprojects::MaySettle(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 CivIndex)
{
	using namespace CosmicMegaprojectsImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	bool bAnySettled = false;
	const int32 Owned = OwnedCount(State, Index, CivIndex, bAnySettled);
	return bAnySettled && Owned < RealmCapOf(Rules, State.Civs[CivIndex].Tier) && !IsSettling(State, Index, CivIndex);
}

CosmicMegaprojects::ECosmicSettleBlock CosmicMegaprojects::SettleBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex)
{
	using namespace CosmicMegaprojectsImpl;

	// Law.Hedge (section 4.5 (1)): the first reason, whoever asks.
	if (State.Systems.IsValidIndex(SystemIndex) && CosmicState::IsSanctuary(State.Systems[SystemIndex].SystemID))
	{
		return ECosmicSettleBlock::Sanctuary;
	}
	if (!State.Civs.IsValidIndex(CivIndex) || !State.Systems.IsValidIndex(SystemIndex))
	{
		return ECosmicSettleBlock::NoPeople;
	}
	const FStarSystemState& Target = State.Systems[SystemIndex];
	if (!Target.OwnerCivID.IsNone())
	{
		return ECosmicSettleBlock::Owned;
	}
	if (CosmicHazards::IsDeadNode(Target))
	{
		return ECosmicSettleBlock::Dead;
	}
	// A foothold is a settled system: past this, MaySettle's first part holds.
	if (!HasFoothold(State, Index, CivIndex, Target))
	{
		return ECosmicSettleBlock::NoLane;
	}
	if (!Tables.FindMegaproject(CosmicRows::ProjectSettle))
	{
		return ECosmicSettleBlock::NoRow;
	}
	if (!SettleRowAllows(State, Tables, CivIndex))
	{
		return ECosmicSettleBlock::RowTier;
	}
	bool bAnySettled = false;
	if (OwnedCount(State, Index, CivIndex, bAnySettled) >= RealmCapOf(Rules, State.Civs[CivIndex].Tier))
	{
		return ECosmicSettleBlock::RealmCap;
	}
	if (IsSettling(State, Index, CivIndex))
	{
		return ECosmicSettleBlock::Settling;
	}
	if (SphereRefuses(State, Tables, CivIndex, Target))
	{
		return ECosmicSettleBlock::Sphere;
	}
	return ECosmicSettleBlock::None;
}

bool CosmicMegaprojects::CanSettle(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex)
{
	return SettleBlockOf(State, Index, Tables, Rules, CivIndex, SystemIndex) == ECosmicSettleBlock::None;
}

TArray<int32> CosmicMegaprojects::SettleTargets(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex)
{
	using namespace CosmicMegaprojectsImpl;

	TArray<int32> Targets;
	if (!MaySettle(State, Index, Rules, CivIndex) || !SettleRowAllows(State, Tables, CivIndex))
	{
		return Targets;
	}
	// The candidates are the lane neighbours of the settled footholds, in Systems order: the same list a walk over every
	// system would give, without the walk (section 9.4).
	TArray<int32, TInlineAllocator<16>> Candidates;
	for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
	{
		const int32 Foothold = SettledOwnedIndex(State, Index, CivIndex, SystemID);
		if (Foothold == INDEX_NONE)
		{
			continue;
		}
		for (const FName Lane : State.Systems[Foothold].Lanes)
		{
			const int32 Neighbour = Index.FindSystem(State, Lane);
			if (State.Systems.IsValidIndex(Neighbour))
			{
				Candidates.AddUnique(Neighbour);
			}
		}
	}
	Candidates.Sort();
	for (const int32 SystemIndex : Candidates)
	{
		if (IsTargetOf(State, Index, Tables, CivIndex, SystemIndex))
		{
			Targets.Add(SystemIndex);
		}
	}
	return Targets;
}

CosmicMegaprojects::ECosmicSettleBlock CosmicMegaprojects::PeopleSettleBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex)
{
	using namespace CosmicMegaprojectsImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return ECosmicSettleBlock::NoPeople;
	}
	// The people part of SettleBlockOf's order, with the first part of MaySettle before it (no foothold at all).
	bool bAnySettled = false;
	const int32 Owned = OwnedCount(State, Index, CivIndex, bAnySettled);
	if (!bAnySettled)
	{
		return ECosmicSettleBlock::NoLane;
	}
	if (!Tables.FindMegaproject(CosmicRows::ProjectSettle))
	{
		return ECosmicSettleBlock::NoRow;
	}
	if (!SettleRowAllows(State, Tables, CivIndex))
	{
		return ECosmicSettleBlock::RowTier;
	}
	if (Owned >= RealmCapOf(Rules, State.Civs[CivIndex].Tier))
	{
		return ECosmicSettleBlock::RealmCap;
	}
	if (IsSettling(State, Index, CivIndex))
	{
		return ECosmicSettleBlock::Settling;
	}
	// What surrounds the people: the candidates of SettleTargets, each read by SettleBlockOf itself (one rule with the
	// panel). Past the people's reasons only Sanctuary, Owned, Dead, NoLane (a foothold of another galaxy row), Sphere or
	// None can come. A reason added to ECosmicSettleBlock takes its rank here, or it scores 0 and a people boxed in by it
	// reads NoLane. The sanctuary scores 0 on purpose: it is never a target and gives a people no reason of its own, so a
	// people whose only neighbour it is has no lane to settle by (it has no lane before v0.8 step 5).
	static_assert(static_cast<int32>(ECosmicSettleBlock::Count) == 11, "A new settle reason: give it its rank among a people's neighbour reasons (the sanctuary scores 0 on purpose)");
	auto Strength = [](ECosmicSettleBlock Block)
	{
		return Block == ECosmicSettleBlock::Sphere ? 4 : Block == ECosmicSettleBlock::Owned ? 3 : Block == ECosmicSettleBlock::Dead ? 2 : Block == ECosmicSettleBlock::NoLane ? 1 : 0;
	};
	ECosmicSettleBlock Strongest = ECosmicSettleBlock::NoLane;
	for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
	{
		const int32 Foothold = SettledOwnedIndex(State, Index, CivIndex, SystemID);
		if (Foothold == INDEX_NONE)
		{
			continue;
		}
		for (const FName Lane : State.Systems[Foothold].Lanes)
		{
			const int32 Neighbour = Index.FindSystem(State, Lane);
			if (!State.Systems.IsValidIndex(Neighbour))
			{
				continue;
			}
			const ECosmicSettleBlock Block = SettleBlockOf(State, Index, Tables, Rules, CivIndex, Neighbour);
			if (Block == ECosmicSettleBlock::None)
			{
				return ECosmicSettleBlock::None;
			}
			if (Strength(Block) > Strength(Strongest))
			{
				Strongest = Block;
			}
		}
	}
	return Strongest;
}

bool CosmicMegaprojects::Settle(FCosmicEvalContext& Ctx, int32 CivIndex, int32 SystemIndex)
{
	if (!CanSettle(Ctx.State, Ctx.Index, Ctx.Tables, Ctx.Rules, CivIndex, SystemIndex) || !CosmicHazards::Transfer(Ctx, SystemIndex, CivIndex))
	{
		return false;
	}
	FStarSystemState& System = Ctx.State.Systems[SystemIndex];
	System.ActiveMegaproject = CosmicRows::ProjectSettle;
	System.MegaprojectProgress = 0.f;
	return true;
}

FName CosmicMegaprojects::Advance(FCosmicEvalContext& Ctx, int32 SystemIndex, ECosmicProjectSlot Slot, float AIRate)
{
	using namespace CosmicMegaprojectsImpl;

	FName Completed;
	FGalaxyState& State = Ctx.State;
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return Completed;
	}
	const int32 OwnerIndex = Ctx.Index.FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
	if (!State.Civs.IsValidIndex(OwnerIndex))
	{
		return Completed;
	}
	const float Rate = State.Civs[OwnerIndex].bIsPlayerRuled ? 1.f : AIRate;

	// The completion of RowName: its record, the empty slot, the vessel's life, then the hazard it answers.
	auto Complete = [&](FName RowName)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		// The record names the people that completed it last (B3, section 16 #73) - a re-completion rewrites it too.
		const FName CompleterID = State.Civs[OwnerIndex].CivID;
		if (FMegaprojectRecord* Record = System.CompletedMegaprojects.FindByPredicate([RowName](const FMegaprojectRecord& Candidate) { return Candidate.Row == RowName; }))
		{
			Record->Tick = State.Tick;
			Record->CivID = CompleterID;
		}
		else
		{
			FMegaprojectRecord& Added = System.CompletedMegaprojects.AddDefaulted_GetRef();
			Added.Row = RowName;
			Added.Tick = State.Tick;
			Added.CivID = CompleterID;
		}
		System.MegaprojectCompletedTick = State.Tick;
		const FCivilizationState& Owner = State.Civs[OwnerIndex];
		if (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && State.Player.RuledCivID == Owner.CivID && !State.Calling.Lives.IsEmpty())
		{
			State.Calling.Lives.Last().MegaprojectsCompleted.Add(RowName);
		}
		Completed = RowName;
		const FHazardDef* Hazard = Ctx.Tables.FindHazardByTag(System.ActiveHazard);
		if (Hazard && Hazard->RulerResolutions.Contains(RowName))
		{
			CosmicHazards::Resolve(Ctx, SystemIndex);
		}
	};

	// The construction slot. A settlement is one more construction row (section 5.7).
	if (Slot == ECosmicProjectSlot::Construction)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		const FName RowName = System.ActiveMegaproject;
		if (!RowName.IsNone())
		{
			const FMegaprojectDef* Row = Ctx.Tables.FindMegaproject(RowName);
			// The binding asks for its target on every call, before it pays (B6b header 2.5): a binding earlier in Systems
			// order in this pass may just have bound the one fallen. Read here, never once for a pass or a tick.
			const bool bBinding = RowName == CosmicRows::ProjectBinding;
			const int32 BindTarget = (Row && bBinding) ? CosmicSeal::BindTargetOf(State, Ctx.Tables) : INDEX_NONE;
			if (!Row)
			{
				// A settlement waits for its row: the system does not become settled for want of one.
				if (RowName != CosmicRows::ProjectSettle)
				{
					System.ActiveMegaproject = NAME_None;
					System.MegaprojectProgress = 0.f;
				}
			}
			else if (bBinding && BindTarget == INDEX_NONE)
			{
				// No fallen one left: the binding ends without result - nothing paid, recorded, listed or told.
				System.ActiveMegaproject = NAME_None;
				System.MegaprojectProgress = 0.f;
			}
			else if (PayInFull(State.Civs[OwnerIndex].Stockpile, *Row))
			{
				// The delay above the third rung (B6b header 2.6): the construction slot of a people the player does not rule,
				// a row of the third rung or above. At its knob of 1 the product is the rate itself, bit for bit.
				const bool bUpper = !State.Civs[OwnerIndex].bIsPlayerRuled && static_cast<int32>(Row->RequiredTier) >= static_cast<int32>(EAscensionTier::Tier3_Causal);
				System.MegaprojectProgress += bUpper ? Rate * UpperRungMultiplierOf(Ctx.Rules) : Rate;
				if (System.MegaprojectProgress >= CosmicMegaprojects::RequiredProgressOf(*Row))
				{
					System.ActiveMegaproject = NAME_None;
					System.MegaprojectProgress = 0.f;
					Complete(RowName);
					// The seal, after the record, in one place (docs/50). Between the ask and here only the record and the life
					// list were written (no hazard row answers with the binding - Data.Integrity), so Bind does not fail.
					if (bBinding)
					{
						CosmicSeal::Bind(State, BindTarget, State.Tick);
					}
				}
			}
		}
	}

	// The response slot: it answers the hazard that stands there now, or it ends.
	if (Slot == ECosmicProjectSlot::Response)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		const FName RowName = System.ResponseMegaproject;
		if (!RowName.IsNone())
		{
			const FMegaprojectDef* Row = Ctx.Tables.FindMegaproject(RowName);
			const FHazardDef* Hazard = Ctx.Tables.FindHazardByTag(System.ActiveHazard);
			if (!Row || !Hazard || !Hazard->RulerResolutions.Contains(RowName))
			{
				System.ResponseMegaproject = NAME_None;
				System.ResponseProgress = 0.f;
			}
			else if (PayInFull(State.Civs[OwnerIndex].Stockpile, *Row))
			{
				System.ResponseProgress += Rate;
				if (System.ResponseProgress >= CosmicMegaprojects::RequiredProgressOf(*Row))
				{
					System.ResponseMegaproject = NAME_None;
					System.ResponseProgress = 0.f;
					Complete(RowName);
				}
			}
		}
	}
	return Completed;
}
