// Cosmic Ascension - World layer: the ruler policy a script plays between ticks (MVP item 9).

#include "World/CosmicRulerPolicy.h"

#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
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

int32 CosmicRulerPolicy::ChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar, FName RowName,
	int32 Diplomacy, const FCosmicRulesDef* Rules)
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
	// A choice the answer takes: payable, and - with the rules - its stance not refused by the gate (B3: the first contact
	// card's people; the answer refuses such a choice).
	const FName Other = RowName == CosmicRows::PetitionFirstContact ? State.Knowing.PendingContactCivID : NAME_None;
	auto Taken = [&](int32 Choice)
	{
		return CosmicMegaprojects::CanPay(State.Civs[CivIndex].Stockpile, CosmicPetitions::CostOf(Row.Choices[Choice]))
			&& (!Rules || CosmicPetitions::StanceBlockOfChoice(State, Index, *Rules, CivIndex, Other, Row.Choices[Choice]) == CosmicDiplomacy::ECosmicStanceBlock::None);
	};
	// The first contact card under a diplomacy policy (B3): reach out.
	if (Diplomacy != 0 && RowName == CosmicRows::PetitionFirstContact)
	{
		for (const int32 Choice : Stays)
		{
			const bool bFriendly = Row.Choices[Choice].OnSuccess.ContainsByPredicate([](const FCosmicEffect& Effect)
			{
				return Effect.Type == EEffectType::SetStance && Effect.Tag == CosmicTags::StanceFriendly();
			});
			if (bFriendly && Taken(Choice))
			{
				return Choice;
			}
		}
	}
	for (int32 Step = 0; Step < Stays.Num(); ++Step)
	{
		const int32 Choice = Stays[(AnswersSoFar + Step) % Stays.Num()];
		if (Taken(Choice))
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
		const int32 Choice = Row ? ChoiceOf(State, Sim.GetStateIndex(), *Row, Counts.Answers, Card, Policy.Diplomacy, &Sim.GetRules()) : INDEX_NONE;
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
	ActDiplomacy(Policy, Sim, Role, Counts);
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

void CosmicRulerPolicy::ActDiplomacy(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FCosmicRulerPolicyCounts& Counts)
{
	const FGalaxyState& State = Sim.GetState();
	const FCosmicStateIndex& Index = Sim.GetStateIndex();
	const FCosmicDataTables& Tables = Sim.GetTables();
	const FCosmicRulesDef& Rules = Sim.GetRules();
	if (Policy.Diplomacy == 0 || Counts.bEnded || State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler)
	{
		return;
	}
	const int32 CivIndex = Index.FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return;
	}
	const FName CivID = State.Civs[CivIndex].CivID;
	// The script plays the player: it reads what the calling met, as a player would.
	auto IsMet = [&State](FName ID) { return State.Knowing.MetCivs.ContainsByPredicate([ID](const FKnownMark& Mark) { return Mark.ID == ID; }); };
	auto TargetOf = [&]() { return State.Civs[CivIndex].FleetTargetSystemID; };

	// (1) The guard: a siege on one of the people's systems is guarded against while the people besieges no one; a guard
	// over a system no longer besieged is withdrawn.
	if (!CosmicConflict::IsBesieging(State, Index, CivIndex))
	{
		FName Besieged;
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && Besieged.IsNone(); ++SystemIndex)
		{
			if (State.Systems[SystemIndex].OwnerCivID == CivID && CosmicConflict::BesiegerOf(State, Index, SystemIndex) != INDEX_NONE)
			{
				Besieged = State.Systems[SystemIndex].SystemID;
			}
		}
		const FName Before = TargetOf();
		if (!Besieged.IsNone() && Before != Besieged)
		{
			if (Role.AllocateFleet(Besieged) == CosmicConflict::ECosmicSiegeBlock::None && TargetOf() != Before)
			{
				++Counts.FleetTargetChanges;
				return;
			}
		}
		else if (Besieged.IsNone() && !Before.IsNone())
		{
			const int32 Guarded = Index.FindSystem(State, Before);
			if (State.Systems.IsValidIndex(Guarded) && State.Systems[Guarded].OwnerCivID == CivID && CosmicConflict::BesiegerOf(State, Index, Guarded) == INDEX_NONE
				&& Role.AllocateFleet(NAME_None) == CosmicConflict::ECosmicSiegeBlock::None && TargetOf() != Before)
			{
				++Counts.FleetTargetChanges;
				return;
			}
		}
	}

	// (2) Friendship, while no first contact card waits: the first met bordering people the people holds no stance toward,
	// when the gate takes friendly and the Influence income keeps one more friendly stance with the edicts' upkeep.
	float Upkeep = 0.f;
	for (const FName Row : State.Civs[CivIndex].ActiveEdicts)
	{
		if (const FEdictDef* Def = Tables.FindEdict(Row))
		{
			Upkeep += CosmicEdicts::CostOf(Def->UpkeepPerTick).Influence;
		}
	}
	const float FriendlyUpkeep = (Rules.FriendlyUpkeepInfluence > 0.f && Rules.FriendlyUpkeepInfluence <= TNumericLimits<float>::Max()) ? Rules.FriendlyUpkeepInfluence : 0.f;
	const bool bKeepsOneMore = State.Civs[CivIndex].IncomePerTick.Influence
		>= Upkeep + FriendlyUpkeep * static_cast<float>(CosmicDiplomacy::FriendlyCountOf(State.Civs[CivIndex]) + 1);
	for (int32 Other = 0; bKeepsOneMore && State.Knowing.PendingContactCivID.IsNone() && Other < State.Civs.Num(); ++Other)
	{
		const FName OtherID = State.Civs[Other].CivID;
		if (Other == CivIndex || !IsMet(OtherID) || !CosmicDiplomacy::Borders(State, Index, CivIndex, Other)
			|| CosmicDiplomacy::StanceOf(State.Civs[CivIndex], OtherID).IsValid())
		{
			continue;
		}
		if (CosmicDiplomacy::StanceBlockOf(State, Index, Rules, CivIndex, OtherID, CosmicTags::StanceFriendly(), true) == CosmicDiplomacy::ECosmicStanceBlock::None
			&& Role.SetStance(OtherID, CosmicTags::StanceFriendly()) == CosmicDiplomacy::ECosmicStanceBlock::None)
		{
			++Counts.StanceChanges;
			return;
		}
	}

	// (3) Treaties: non-aggression, then trade, to the met bordering peoples; the first look of every pair is counted. The
	// attack policy offers none: a non-aggression would close the siege gate on the neighbours it is there to besiege, and
	// a trade would make each siege open with a betrayal.
	const FGameplayTag Treaties[] = { CosmicTags::TreatyNonAggression(), CosmicTags::TreatyTrade() };
	for (int32 Other = 0; Policy.Diplomacy != 2 && Other < State.Civs.Num(); ++Other)
	{
		const FName OtherID = State.Civs[Other].CivID;
		if (Other == CivIndex || !IsMet(OtherID))
		{
			continue;
		}
		const bool bBorders = CosmicDiplomacy::Borders(State, Index, CivIndex, Other);
		for (const FGameplayTag Treaty : Treaties)
		{
			const CosmicDiplomacy::ECosmicTreatyBlock Block = CosmicDiplomacy::TreatyBlockOf(State, Index, Tables, Rules, CivIndex, OtherID, Treaty, true);
			// A first look counts from the second rung (the gate's Tier reason before it is no look at the people).
			const bool bLooks = Block == CosmicDiplomacy::ECosmicTreatyBlock::None || Block == CosmicDiplomacy::ECosmicTreatyBlock::Refused;
			const TTuple<FName, FName, FName> Look(CivID, OtherID, Treaty.GetTagName());
			if (bLooks && !Counts.TreatyLooks.Contains(Look))
			{
				Counts.TreatyLooks.Add(Look);
				if (Block == CosmicDiplomacy::ECosmicTreatyBlock::Refused)
				{
					++(bBorders ? Counts.TreatiesRefusedBorder : Counts.TreatiesRefusedOther);
				}
				else if (!bBorders)
				{
					++Counts.TreatiesProposedOther;
				}
			}
			if (bBorders && Block == CosmicDiplomacy::ECosmicTreatyBlock::None
				&& Role.ProposeTreaty(OtherID, Treaty) == CosmicDiplomacy::ECosmicTreatyBlock::None)
			{
				++Counts.TreatiesSigned;
				++Counts.TreatiesProposedBorder;
				return;
			}
		}
	}

	// (4) The attack policy (measurement only): besiege a hostile people's first system the gate takes, else turn the first
	// met bordering people with such a system hostile - not while its fleet guards one of its own systems a siege still
	// stands on (the guard holds).
	if (Policy.Diplomacy != 2 || CosmicConflict::IsBesieging(State, Index, CivIndex))
	{
		return;
	}
	const int32 Guarded = Index.FindSystem(State, TargetOf());
	if (State.Systems.IsValidIndex(Guarded) && State.Systems[Guarded].OwnerCivID == CivID && CosmicConflict::BesiegerOf(State, Index, Guarded) != INDEX_NONE)
	{
		return;
	}
	auto FirstSiegeOf = [&](int32 Other, bool bAssumeHostile)
	{
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			if (State.Systems[SystemIndex].OwnerCivID == State.Civs[Other].CivID
				&& CosmicConflict::SiegeBlockOf(State, Index, Rules, CivIndex, SystemIndex, bAssumeHostile) == CosmicConflict::ECosmicSiegeBlock::None)
			{
				return SystemIndex;
			}
		}
		return static_cast<int32>(INDEX_NONE);
	};
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		if (Other != CivIndex && CosmicDiplomacy::StanceOf(State.Civs[CivIndex], State.Civs[Other].CivID) == CosmicTags::StanceHostile())
		{
			const int32 Target = FirstSiegeOf(Other, false);
			const FName Before = TargetOf();
			if (Target != INDEX_NONE && Role.AllocateFleet(State.Systems[Target].SystemID) == CosmicConflict::ECosmicSiegeBlock::None && TargetOf() != Before)
			{
				++Counts.FleetTargetChanges;
				return;
			}
		}
	}
	// A hostility with no system to besiege is turned neutral before another opens (the AI's rule: no hostility without
	// a siege).
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		const FName OtherID = State.Civs[Other].CivID;
		if (Other != CivIndex && CosmicDiplomacy::StanceOf(State.Civs[CivIndex], OtherID) == CosmicTags::StanceHostile() && FirstSiegeOf(Other, false) == INDEX_NONE
			&& Role.SetStance(OtherID, FGameplayTag()) == CosmicDiplomacy::ECosmicStanceBlock::None)
		{
			++Counts.StanceChanges;
			return;
		}
	}
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		const FName OtherID = State.Civs[Other].CivID;
		if (Other == CivIndex || !IsMet(OtherID) || !CosmicDiplomacy::Borders(State, Index, CivIndex, Other)
			|| CosmicDiplomacy::StanceOf(State.Civs[CivIndex], OtherID) == CosmicTags::StanceHostile() || FirstSiegeOf(Other, true) == INDEX_NONE)
		{
			continue;
		}
		if (Role.SetStance(OtherID, CosmicTags::StanceHostile()) == CosmicDiplomacy::ECosmicStanceBlock::None)
		{
			++Counts.StanceChanges;
			return;
		}
	}
}

int32 CosmicRulerPolicy::DecisionsOf(const FCosmicRulerPolicyCounts& Counts)
{
	return Counts.Answers + Counts.Started + Counts.FocusChanges + Counts.EdictsEnacted + Counts.EdictsRevoked + Counts.StanceChanges + Counts.TreatiesSigned
		+ Counts.FleetTargetChanges;
}
