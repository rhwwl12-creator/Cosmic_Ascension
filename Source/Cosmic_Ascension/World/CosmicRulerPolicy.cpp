// Cosmic Ascension - World layer: the ruler policy a script plays between ticks (MVP item 9).

#include "World/CosmicRulerPolicy.h"

#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Core/CosmicIDs.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicNextRung.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicRulerPolicyImpl
{
	// Stability below which the script revokes an edict that costs Stability, and at or above which it lets one stand again
	// (the resting level - apart from the first, so one boundary does not swing).
	constexpr float RevokeBelow = 0.5f;
	constexpr float RestAt = 0.6f;

	// Whether Tag is the tag of a Capital row of DT_Hazards (a rebellion - read from the data, not named).
	bool IsCapitalTag(const FCosmicDataTables& Tables, FGameplayTag Tag)
	{
		const FHazardDef* Hazard = Tag.IsValid() ? Tables.FindHazardByTag(Tag) : nullptr;
		return Hazard && Hazard->SpawnRule == ECosmicHazardSpawn::Capital;
	}

	// The vigil: the first DT_Edicts row, lexical, that adds to the countdown of a Capital row's hazard.
	FName VigilRowOf(const FCosmicDataTables& Tables)
	{
		for (const FName Row : Tables.GetEdictRowNames())
		{
			const FEdictDef* Def = Tables.FindEdict(Row);
			if (!Def)
			{
				continue;
			}
			for (const TPair<FGameplayTag, int32>& Bonus : Def->HazardCountdownBonus)
			{
				if (Bonus.Value > 0 && IsCapitalTag(Tables, Bonus.Key))
				{
					return Row;
				}
			}
		}
		return NAME_None;
	}
}

int32 CosmicRulerPolicy::ChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar)
{
	const int32 CivIndex = CosmicPetitions::RuledCivIndexOf(State, Index);
	const int32 Count = Row.Choices.Num();
	if (!State.Civs.IsValidIndex(CivIndex) || Count <= 0)
	{
		return INDEX_NONE;
	}
	// The script never leaves the throne of its own will (B1): a choice whose success records the end of the reign is not
	// among those it turns through - for every row of today the same list as before.
	TArray<int32, TInlineAllocator<4>> Stays;
	for (int32 Choice = 0; Choice < Count; ++Choice)
	{
		const bool bLeaves = Row.Choices[Choice].OnSuccess.ContainsByPredicate([](const FCosmicEffect& Effect) { return Effect.Type == EEffectType::DeposeRuler; });
		if (!bLeaves)
		{
			Stays.Add(Choice);
		}
	}
	for (int32 Step = 0; Step < Stays.Num(); ++Step)
	{
		const int32 Choice = Stays[(AnswersSoFar + Step) % Stays.Num()];
		if (CosmicMegaprojects::CanPay(State.Civs[CivIndex].Stockpile, CosmicPetitions::CostOf(Row.Choices[Choice])))
		{
			return Choice;
		}
	}
	return INDEX_NONE;
}

void CosmicRulerPolicy::Act(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
	FCosmicRulerPolicyCounts& Counts)
{
	const FGalaxyState& State = Sim.GetState();
	Counts.bEnded = Counts.bEnded || CosmicEndings::ProvisionalEndOf(State, Sim.GetRules()) != ECosmicProvisionalEnd::None;

	// The ruler's cards: a wanderer's card after a deposition is not one (B1 - the ruler line stays the ruler's).
	const bool bRuler = State.Player.Role == EPlayerRole::Ruler;
	Counts.CardsCome += (bRuler && !State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != CardBefore) ? 1 : 0;
	for (int32 Guard = 0; Policy.bAnswer && Guard < 8 && !State.Knowing.PendingPetitionRow.IsNone(); ++Guard)
	{
		const FName Card = State.Knowing.PendingPetitionRow;
		const FEncounterDef* Row = Sim.GetTables().FindEncounter(Card);
		const int32 Choice = Row ? ChoiceOf(State, Sim.GetStateIndex(), *Row, Counts.Answers) : INDEX_NONE;
		if (Choice == INDEX_NONE)
		{
			break;
		}
		const FCosmicPetitionAnswer Taken = Role.AnswerPetition(Choice);
		if (Taken.Outcome == ECosmicPetitionOutcome::Refused)
		{
			break;
		}
		Counts.Answers += (Taken.Outcome == ECosmicPetitionOutcome::Withdrawn) ? 0 : 1;
		Counts.CardsCome += (State.Player.Role == EPlayerRole::Ruler && !State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != Card) ? 1 : 0;
	}
	for (int32 Guard = 0; Policy.bBuild && Guard < 8 && Role.StartNextMegaproject(); ++Guard)
	{
		++Counts.Started;
	}
	// The next rung's research (MVP item 7): only while the run has not ended (the game takes no input after it).
	if (Policy.bFocus && !Counts.bEnded)
	{
		FName Field = NAME_None;
		for (const FCosmicRungPart& Part : CosmicNextRung::ResearchFieldsOf(CosmicNextRung::AllOf(State, Sim.GetStateIndex(), Sim.GetTables(), Sim.GetRules())))
		{
			if (!Part.bMet)
			{
				Field = Part.Key;
				break;
			}
		}
		Counts.FocusChanges += Role.SetResearchFocus(Field) ? 1 : 0;
	}
	ActEdicts(Policy, Sim, Role, Counts);
}

void CosmicRulerPolicy::ActEdicts(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FCosmicRulerPolicyCounts& Counts)
{
	using namespace CosmicRulerPolicyImpl;

	const FGalaxyState& State = Sim.GetState();
	const FCosmicStateIndex& Index = Sim.GetStateIndex();
	const FCosmicDataTables& Tables = Sim.GetTables();
	const FCosmicRulesDef& Rules = Sim.GetRules();
	if (!Policy.bEdict || Counts.bEnded || State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler)
	{
		return;
	}
	const int32 CivIndex = Index.FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return;
	}
	// The people's capital, and whether a rebellion stands on it.
	const int32 Capital = Index.FindSystem(State, State.Civs[CivIndex].HomeSystemID);
	auto RebellionStands = [&]() { return State.Systems.IsValidIndex(Capital) && CosmicRulerPolicyImpl::IsCapitalTag(Tables, State.Systems[Capital].ActiveHazard); };
	TArray<FName> Enacted;
	TArray<FName> Revoked;
	auto Revoke = [&](FName Row)
	{
		if (Enacted.Contains(Row) || !Role.RevokeEdict(Row))
		{
			return false;
		}
		Revoked.Add(Row);
		++Counts.EdictsRevoked;
		Counts.VigilEnacted = Counts.VigilEnacted == Row ? NAME_None : Counts.VigilEnacted;
		return true;
	};
	auto Enact = [&](FName Row)
	{
		if (Revoked.Contains(Row) || Role.EnactEdict(Row) != CosmicEdicts::ECosmicEdictBlock::None)
		{
			return false;
		}
		Enacted.Add(Row);
		++Counts.EdictsEnacted;
		return true;
	};
	// Room for Row: a free slot, or one made by revoking a held edict that answers no standing hazard (EdictRow last) -
	// only when Row could then be enacted: the slots its one reason and the stockpile paying its cost (EnactBlockOf asks the
	// slots before the cost), so no edict is given up for an enactment that would be refused.
	auto MakeRoomFor = [&](FName Row)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (CosmicEdicts::HeldCountOf(Civ, Tables) < CosmicEdicts::SlotsOf(Civ, Rules))
		{
			return true;
		}
		const FEdictDef* Wanted = Tables.FindEdict(Row);
		if (!Wanted || CosmicEdicts::EnactBlockOf(State, Index, Tables, Rules, CivIndex, Row) != CosmicEdicts::ECosmicEdictBlock::Slots
			|| !CosmicMegaprojects::CanPay(Civ.Stockpile, CosmicEdicts::CostOf(Wanted->EnactCost)))
		{
			return false;
		}
		FName Spare;
		for (const FName Held : TArray<FName>(Civ.ActiveEdicts))
		{
			if (Held == Row || !Tables.FindEdict(Held) || Enacted.Contains(Held) || CosmicEdicts::AnswersStanding(State, Index, Tables, CivIndex, Held))
			{
				continue;
			}
			if (Held != Policy.EdictRow)
			{
				Spare = Held;
				break;
			}
			Spare = Spare.IsNone() ? Held : Spare;
		}
		return !Spare.IsNone() && Revoke(Spare);
	};

	// (1) Below 0.5, the edicts that cost Stability go.
	for (const FName Held : TArray<FName>(State.Civs[CivIndex].ActiveEdicts))
	{
		const FEdictDef* Def = Tables.FindEdict(Held);
		if (Def && CosmicEdicts::HasStabilityCost(*Def) && State.Civs[CivIndex].Stability < RevokeBelow)
		{
			Revoke(Held);
		}
	}
	// (2) An answer to each hazard standing on a settled system of the people that no held edict answers.
	for (const FStarSystemState& System : State.Systems)
	{
		if (System.OwnerCivID != State.Civs[CivIndex].CivID || !System.ActiveHazard.IsValid() || !CosmicState::IsSettled(System))
		{
			continue;
		}
		const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
		if (!Hazard || !CosmicEdicts::HeldAnswerOf(State.Civs[CivIndex], *Hazard, Tables).IsNone())
		{
			continue;
		}
		for (const FName Answer : Hazard->RulerResolutions)
		{
			if (Tables.FindEdict(Answer))
			{
				if (!Revoked.Contains(Answer) && MakeRoomFor(Answer))
				{
					Enact(Answer);
				}
				break;
			}
		}
	}
	// (3) The vigil against a coming rebellion: discontent counted, no rebellion yet, the vigil not held.
	const FName Vigil = VigilRowOf(Tables);
	if (!Vigil.IsNone() && State.Civs[CivIndex].LowStabilityTicks > 0 && !RebellionStands() && !State.Civs[CivIndex].ActiveEdicts.Contains(Vigil)
		&& !Revoked.Contains(Vigil) && MakeRoomFor(Vigil) && Enact(Vigil))
	{
		Counts.VigilEnacted = Vigil;
	}
	// (4) An answer with nothing to answer goes, and the vigil of (3) once the danger passed - never EdictRow.
	for (const FName Held : TArray<FName>(State.Civs[CivIndex].ActiveEdicts))
	{
		if (Held == Policy.EdictRow || Enacted.Contains(Held))
		{
			continue;
		}
		const bool bIdleAnswer = CosmicEdicts::IsAnswerEdict(Tables, Held) && !CosmicEdicts::AnswersStanding(State, Index, Tables, CivIndex, Held);
		const bool bVigilDone = Held == Counts.VigilEnacted && State.Civs[CivIndex].Stability >= RestAt && !RebellionStands();
		if (bIdleAnswer || bVigilDone)
		{
			Revoke(Held);
		}
	}
	// (5) EdictRow in a free slot - a row that costs Stability only at the resting level.
	const FEdictDef* Kept = Policy.EdictRow.IsNone() ? nullptr : Tables.FindEdict(Policy.EdictRow);
	const FCivilizationState& Civ = State.Civs[CivIndex];
	if (Kept && !Civ.ActiveEdicts.Contains(Policy.EdictRow) && CosmicEdicts::HeldCountOf(Civ, Tables) < CosmicEdicts::SlotsOf(Civ, Rules)
		&& (!CosmicEdicts::HasStabilityCost(*Kept) || Civ.Stability >= RestAt))
	{
		Enact(Policy.EdictRow);
	}
}

int32 CosmicRulerPolicy::DecisionsOf(const FCosmicRulerPolicyCounts& Counts)
{
	return Counts.Answers + Counts.Started + Counts.FocusChanges + Counts.EdictsEnacted + Counts.EdictsRevoked;
}
