// Cosmic Ascension - Sim layer.

#include "Sim/CosmicMegaprojects.h"

#include "Containers/ContainerAllocationPolicies.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicMegaprojectsImpl
{
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
			if (SystemIndex != INDEX_NONE && CosmicMegaprojects::HasCompleted(State.Systems[SystemIndex], RowName, SinceTick))
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

	// Whether SystemIndex is a target of the people CivIndex, whatever MaySettle and the row say: a wild system (no
	// owner, not a dead node) one lane from a settled system the people owns and in that system's GalaxyRow, and, inside
	// the sphere of an unsealed transcendent with a row, a people on Tier 3 or higher (section 4.2 modifier 4).
	bool IsTargetOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 CivIndex, int32 SystemIndex)
	{
		if (!State.Systems.IsValidIndex(SystemIndex))
		{
			return false;
		}
		const FStarSystemState& Target = State.Systems[SystemIndex];
		if (!Target.OwnerCivID.IsNone() || CosmicHazards::IsDeadNode(Target))
		{
			return false;
		}
		bool bFoothold = false;
		for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
		{
			const int32 Foothold = SettledOwnedIndex(State, Index, CivIndex, SystemID);
			if (Foothold != INDEX_NONE && State.Systems[Foothold].GalaxyRow == Target.GalaxyRow && State.Systems[Foothold].Lanes.Contains(Target.SystemID))
			{
				bFoothold = true;
				break;
			}
		}
		if (!bFoothold)
		{
			return false;
		}
		if (static_cast<int32>(State.Civs[CivIndex].Tier) >= static_cast<int32>(EAscensionTier::Tier3_Causal))
		{
			return true;
		}
		// Membership, not a number: no threshold on P, and no term of a vessel that happens to stand nearby.
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			if (!Transcendent.bSealed && Tables.FindTranscendent(Transcendent.TranscendentID) && Transcendent.InfluencedSystems.Contains(Target.SystemID))
			{
				return false;
			}
		}
		return true;
	}
}

bool CosmicMegaprojects::HasCompleted(const FStarSystemState& System, FName RowName, int64 SinceTick)
{
	if (RowName.IsNone())
	{
		return false;
	}
	const FMegaprojectRecord* Record = System.CompletedMegaprojects.FindByPredicate([RowName](const FMegaprojectRecord& Candidate) { return Candidate.Row == RowName; });
	return Record && Record->Tick > 0 && Record->Tick >= SinceTick;
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

bool CosmicMegaprojects::CanStart(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex, FName RowName)
{
	const FMegaprojectDef* Row = Tables.FindMegaproject(RowName);
	if (!Row || RowName == CosmicRows::ProjectSettle || !State.Systems.IsValidIndex(SystemIndex))
	{
		return false;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	if (CosmicHazards::IsDeadNode(System) || !CosmicState::IsSettled(System) || !State.Civs.IsValidIndex(OwnerIndex))
	{
		return false;
	}
	if (static_cast<int32>(State.Civs[OwnerIndex].Tier) < static_cast<int32>(Row->RequiredTier))
	{
		return false;
	}
	if (Tables.IsHazardAnswer(RowName))
	{
		const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
		return Hazard && Hazard->RulerResolutions.Contains(RowName) && System.ResponseMegaproject.IsNone();
	}
	return System.ActiveMegaproject != RowName;
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
		// A row the judgment refuses (FAscensionFeatDef::IsOneRungWithDeed) would never lift the people it builds for.
		if (!Feat || Feat->Path != EPlayerRole::Ruler || Feat->RequiredTier != Civ.Tier || !Feat->IsOneRungWithDeed())
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
	int32 Owned = 0;
	bool bAnySettled = false;
	for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
	{
		const int32 SystemIndex = OwnedIndex(State, Index, CivIndex, SystemID);
		if (SystemIndex != INDEX_NONE)
		{
			++Owned;
			bAnySettled |= CosmicState::IsSettled(State.Systems[SystemIndex]);
		}
	}
	return bAnySettled && Owned < RealmCapOf(Rules, State.Civs[CivIndex].Tier) && !IsSettling(State, Index, CivIndex);
}

bool CosmicMegaprojects::CanSettle(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex)
{
	using namespace CosmicMegaprojectsImpl;

	return MaySettle(State, Index, Rules, CivIndex) && SettleRowAllows(State, Tables, CivIndex) && IsTargetOf(State, Index, Tables, CivIndex, SystemIndex);
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
		if (FMegaprojectRecord* Record = System.CompletedMegaprojects.FindByPredicate([RowName](const FMegaprojectRecord& Candidate) { return Candidate.Row == RowName; }))
		{
			Record->Tick = State.Tick;
		}
		else
		{
			FMegaprojectRecord& Added = System.CompletedMegaprojects.AddDefaulted_GetRef();
			Added.Row = RowName;
			Added.Tick = State.Tick;
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
			if (!Row)
			{
				// A settlement waits for its row: the system does not become settled for want of one.
				if (RowName != CosmicRows::ProjectSettle)
				{
					System.ActiveMegaproject = NAME_None;
					System.MegaprojectProgress = 0.f;
				}
			}
			else if (PayInFull(State.Civs[OwnerIndex].Stockpile, *Row))
			{
				System.MegaprojectProgress += Rate;
				if (System.MegaprojectProgress >= CosmicMegaprojects::RequiredProgressOf(*Row))
				{
					System.ActiveMegaproject = NAME_None;
					System.MegaprojectProgress = 0.f;
					Complete(RowName);
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
