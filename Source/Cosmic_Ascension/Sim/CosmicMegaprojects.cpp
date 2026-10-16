// Cosmic Ascension - Sim layer.

#include "Sim/CosmicMegaprojects.h"

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
	// A cost member as it is paid: below 0, or not a number, reads as 0 (a compare, not a CRT call - section 9.2).
	float CostOf(float Value)
	{
		return (Value > 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 0.f;
	}

	// Pays the cost of one tick from Stockpile when it can be paid in full; nothing otherwise.
	bool PayInFull(FResourcePool& Stockpile, const FResourcePool& Cost)
	{
		const float Energy = CostOf(Cost.Energy);
		const float Matter = CostOf(Cost.Matter);
		const float Insight = CostOf(Cost.Insight);
		const float Influence = CostOf(Cost.Influence);
		const float Causality = CostOf(Cost.Causality);
		if (Stockpile.Energy < Energy || Stockpile.Matter < Matter || Stockpile.Insight < Insight || Stockpile.Influence < Influence || Stockpile.Causality < Causality)
		{
			return false;
		}
		Stockpile.Energy -= Energy;
		Stockpile.Matter -= Matter;
		Stockpile.Insight -= Insight;
		Stockpile.Influence -= Influence;
		Stockpile.Causality -= Causality;
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

TArray<FName, TInlineAllocator<2>> CosmicMegaprojects::Advance(FCosmicEvalContext& Ctx, int32 SystemIndex, float AIRate)
{
	using namespace CosmicMegaprojectsImpl;

	TArray<FName, TInlineAllocator<2>> Completed;
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
		Completed.Add(RowName);
		const FHazardDef* Hazard = Ctx.Tables.FindHazardByTag(System.ActiveHazard);
		if (Hazard && Hazard->RulerResolutions.Contains(RowName))
		{
			CosmicHazards::Resolve(Ctx, SystemIndex);
		}
	};

	// The construction slot. A system being settled is left alone until settlement brings its own rule (work item 9).
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		const FName RowName = System.ActiveMegaproject;
		if (!RowName.IsNone() && RowName != CosmicRows::ProjectSettle)
		{
			const FMegaprojectDef* Row = Ctx.Tables.FindMegaproject(RowName);
			if (!Row)
			{
				System.ActiveMegaproject = NAME_None;
				System.MegaprojectProgress = 0.f;
			}
			else if (PayInFull(State.Civs[OwnerIndex].Stockpile, Row->CostPerTick))
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
			else if (PayInFull(State.Civs[OwnerIndex].Stockpile, Row->CostPerTick))
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
