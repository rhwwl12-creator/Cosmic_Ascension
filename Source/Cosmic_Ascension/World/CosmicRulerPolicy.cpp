// Cosmic Ascension - World layer: the ruler policy a script plays between ticks (MVP item 9).

#include "World/CosmicRulerPolicy.h"

#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicPetitions.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicNextRung.h"

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
}
