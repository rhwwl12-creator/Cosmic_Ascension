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
#include "Sim/CosmicHazards.h"
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

	// The kind of a choice of a pressure card (B4), read from its effects: the vigil, another edict, the fleet's time,
	// discern (a next card), the watch (no cost, no check, no effect, no next card), or none of these.
	enum class EPressureKind : uint8
	{
		Other,
		Vigil,
		Edict,
		Resist,
		Discern,
		Watch,
	};

	EPressureKind KindOf(const FEncounterChoice& Choice, FName Vigil)
	{
		bool bEdict = false;
		bool bResist = false;
		for (const FCosmicEffect& Effect : Choice.OnSuccess)
		{
			if (Effect.Type == EEffectType::EnactEdict && !Vigil.IsNone() && Effect.Key == Vigil)
			{
				return EPressureKind::Vigil;
			}
			bEdict |= Effect.Type == EEffectType::EnactEdict;
			bResist |= Effect.Type == EEffectType::DelayHazard;
		}
		if (bEdict)
		{
			return EPressureKind::Edict;
		}
		if (bResist)
		{
			return EPressureKind::Resist;
		}
		if (!Choice.NextEncounter.IsNone())
		{
			return EPressureKind::Discern;
		}
		const FResourcePool Cost = CosmicPetitions::CostOf(Choice);
		const bool bFree = Cost.Energy <= 0.f && Cost.Matter <= 0.f && Cost.Insight <= 0.f && Cost.Influence <= 0.f && Cost.Causality <= 0.f;
		return (bFree && Choice.CheckStat == ECosmicCheckStat::NoCheck && Choice.OnSuccess.IsEmpty() && Choice.OnFailure.IsEmpty())
			? EPressureKind::Watch : EPressureKind::Other;
	}

	// A pressure card: a choice that enacts an edict or buys a hazard time.
	bool IsPressureCard(const FEncounterDef& Row)
	{
		return Row.Choices.ContainsByPredicate([](const FEncounterChoice& Choice)
		{
			return Choice.OnSuccess.ContainsByPredicate([](const FCosmicEffect& Effect) { return Effect.Type == EEffectType::EnactEdict || Effect.Type == EEffectType::DelayHazard; });
		});
	}

	// The word of a refusal a pressure card's choice met when the card came (the commandlet's pressure line).
	FString GreyedWordOf(const FCosmicChoiceGate& Gate)
	{
		if (Gate.Kind == ECosmicChoiceGateKind::Stance)
		{
			return TEXT("stance");
		}
		if (Gate.Kind == ECosmicChoiceGateKind::Edict)
		{
			return Gate.Edict == CosmicEdicts::ECosmicEdictBlock::Held ? TEXT("edict_held") : Gate.Edict == CosmicEdicts::ECosmicEdictBlock::Tier ? TEXT("edict_tier")
				: Gate.Edict == CosmicEdicts::ECosmicEdictBlock::Slots ? TEXT("edict_slots") : Gate.Edict == CosmicEdicts::ECosmicEdictBlock::Cost ? TEXT("edict_cost")
				: TEXT("edict_other");
		}
		if (Gate.Kind == ECosmicChoiceGateKind::Resist)
		{
			return Gate.Resist == CosmicConflict::ECosmicResistBlock::NoFleet ? TEXT("fleet_none") : Gate.Resist == CosmicConflict::ECosmicResistBlock::Spent ? TEXT("fleet_spent")
				: TEXT("resist_other");
		}
		return FString();
	}

	// Whether the project Row answers the hazard of SystemIndex in a way the pressure policies leave alone (ProjectChoicesOf).
	bool IsLeftAlone(const FCosmicRulerPolicy& Policy, const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 SystemIndex, FName Row)
	{
		static const FGameplayTag SupernovaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.PreSupernova")));
		if (Policy.Pressure == ECosmicPressurePolicy::Round || !State.Systems.IsValidIndex(SystemIndex))
		{
			return false;
		}
		const FStarSystemState& System = State.Systems[SystemIndex];
		const FHazardDef* Hazard = System.ActiveHazard.IsValid() ? Tables.FindHazardByTag(System.ActiveHazard) : nullptr;
		if (!Hazard || !Hazard->RulerResolutions.Contains(Row))
		{
			return false;
		}
		if (Hazard->SpawnRule == ECosmicHazardSpawn::ClaimRule)
		{
			return true;
		}
		return Policy.bLeaveNearSphere && Hazard->SpawnRule == ECosmicHazardSpawn::Draw && System.ActiveHazard != SupernovaTag && CosmicState::IsSettled(System)
			&& CosmicHazards::ClaimantOf(State, Index, Tables, Rules, SystemIndex) != INDEX_NONE;
	}
}

bool CosmicRulerPolicy::PressurePolicyOf(const FString& Word, ECosmicPressurePolicy& Out)
{
	const TPair<const TCHAR*, ECosmicPressurePolicy> Words[] = {
		{ TEXT("Round"), ECosmicPressurePolicy::Round },
		{ TEXT("Cautious"), ECosmicPressurePolicy::Cautious },
		{ TEXT("Reckless"), ECosmicPressurePolicy::Reckless },
		{ TEXT("Faithful"), ECosmicPressurePolicy::Faithful },
	};
	for (const TPair<const TCHAR*, ECosmicPressurePolicy>& Each : Words)
	{
		if (Word.Equals(Each.Key, ESearchCase::IgnoreCase))
		{
			Out = Each.Value;
			return true;
		}
	}
	return false;
}

const TCHAR* CosmicRulerPolicy::PressurePolicyName(ECosmicPressurePolicy Pressure)
{
	switch (Pressure)
	{
	case ECosmicPressurePolicy::Cautious:
		return TEXT("Cautious");
	case ECosmicPressurePolicy::Reckless:
		return TEXT("Reckless");
	case ECosmicPressurePolicy::Faithful:
		return TEXT("Faithful");
	default:
		return TEXT("Round");
	}
}

bool CosmicRulerPolicy::ClaimStandsOn(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	const FName CivID = State.Civs[CivIndex].CivID;
	return State.Systems.ContainsByPredicate([&](const FStarSystemState& System)
	{
		const FHazardDef* Hazard = (System.OwnerCivID == CivID && System.ActiveHazard.IsValid()) ? Tables.FindHazardByTag(System.ActiveHazard) : nullptr;
		return Hazard && Hazard->SpawnRule == ECosmicHazardSpawn::ClaimRule && CosmicState::IsSettled(System);
	});
}

FName CosmicRulerPolicy::GuardTargetOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules,
	int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return NAME_None;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	const int32 Capital = Index.FindSystem(State, Civ.HomeSystemID);
	if (CosmicConflict::GuardBlockOf(State, Index, Rules, CivIndex, Capital) != CosmicConflict::ECosmicGuardBlock::None)
	{
		return NAME_None;
	}
	// The reach of the higher spheres on a system: the nearest of the unsealed transcendents whose row stands above the people.
	auto ReachOf = [&](int32 SystemIndex)
	{
		ECosmicSphereReach Reach = ECosmicSphereReach::None;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			const FTranscendentDef* Def = Transcendent.bSealed ? nullptr : Tables.FindTranscendent(Transcendent.TranscendentID);
			if (!Def || static_cast<int32>(Def->Tier) <= static_cast<int32>(Civ.Tier))
			{
				continue;
			}
			const ECosmicSphereReach Here = UGalaxySimulationSubsystem::SphereReachIn(State, Transcendent, SystemIndex);
			Reach = (Here == ECosmicSphereReach::Inside || Reach == ECosmicSphereReach::Inside) ? ECosmicSphereReach::Inside
				: (Here == ECosmicSphereReach::Adjacent ? Here : Reach);
		}
		return Reach;
	};
	FName Inside;
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (CosmicConflict::GuardBlockOf(State, Index, Rules, CivIndex, SystemIndex) != CosmicConflict::ECosmicGuardBlock::None)
		{
			continue;
		}
		const ECosmicSphereReach Reach = ReachOf(SystemIndex);
		if (Reach == ECosmicSphereReach::Adjacent)
		{
			return SystemID;
		}
		Inside = (Inside.IsNone() && Reach == ECosmicSphereReach::Inside) ? SystemID : Inside;
	}
	return Inside.IsNone() ? Civ.HomeSystemID : Inside;
}

TArray<TPair<int32, FName>> CosmicRulerPolicy::ProjectChoicesOf(const FCosmicRulerPolicy& Policy, const FGalaxyState& State, const FCosmicStateIndex& Index,
	const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex)
{
	TArray<TPair<int32, FName>> Choices = CosmicMegaprojects::Choices(State, Index, Tables, CivIndex, State.Calling.BornTick);
	Choices.RemoveAll([&](const TPair<int32, FName>& Choice) { return CosmicRulerPolicyImpl::IsLeftAlone(Policy, State, Index, Tables, Rules, Choice.Key, Choice.Value); });
	return Choices;
}

FName CosmicRulerPolicy::VigilRowOf(const FCosmicDataTables& Tables)
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
			if (Bonus.Value > 0 && CosmicRulerPolicyImpl::IsCapitalTag(Tables, Bonus.Key))
			{
				return Row;
			}
		}
	}
	return NAME_None;
}

int32 CosmicRulerPolicy::ChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar, FName RowName,
	int32 Diplomacy, const FCosmicRulesDef* Rules, const FCosmicDataTables* Tables, ECosmicPressurePolicy Pressure)
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
	// card's people; the answer refuses such a choice); with the tables too, no gate of the choice refused on the card's
	// system (B4: CosmicPetitions::ChoiceGateOf - the answer's own gate).
	const FName Other = RowName == CosmicRows::PetitionFirstContact ? State.Knowing.PendingContactCivID : NAME_None;
	const int32 CardSystem = CosmicPetitions::SubjectOf(State, Index, CivIndex, Row.Subject, State.Knowing.PendingPetitionSystemID);
	auto Taken = [&](int32 Choice)
	{
		if (!CosmicMegaprojects::CanPay(State.Civs[CivIndex].Stockpile, CosmicPetitions::CostOf(Row.Choices[Choice])))
		{
			return false;
		}
		if (Rules && Tables)
		{
			return CosmicPetitions::ChoiceGateOf(State, Index, *Tables, *Rules, CivIndex, CardSystem, Other, Row.Choices[Choice]).IsOpen();
		}
		return !Rules || CosmicPetitions::StanceBlockOfChoice(State, Index, *Rules, CivIndex, Other, Row.Choices[Choice]) == CosmicDiplomacy::ECosmicStanceBlock::None;
	};
	// A pressure card under a pressure policy (B4 header section 2.9): the first choice the answer takes of the first kind of
	// the policy's order.
	if (Pressure != ECosmicPressurePolicy::Round && Tables && CosmicRulerPolicyImpl::IsPressureCard(Row))
	{
		using CosmicRulerPolicyImpl::EPressureKind;
		const FName Vigil = VigilRowOf(*Tables);
		TArray<EPressureKind, TInlineAllocator<4>> Order;
		switch (Pressure)
		{
		case ECosmicPressurePolicy::Faithful:
			Order = { EPressureKind::Vigil, EPressureKind::Discern, EPressureKind::Edict, EPressureKind::Watch };
			break;
		case ECosmicPressurePolicy::Reckless:
			Order = { EPressureKind::Discern, EPressureKind::Resist, EPressureKind::Watch };
			break;
		default:
			Order = { EPressureKind::Watch };
			break;
		}
		for (const EPressureKind Kind : Order)
		{
			for (const int32 Choice : Stays)
			{
				if (CosmicRulerPolicyImpl::KindOf(Row.Choices[Choice], Vigil) == Kind && Taken(Choice))
				{
					return Choice;
				}
			}
		}
	}
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

namespace CosmicRulerPolicyImpl
{
	// A pressure card seen to come at a boundary (B4 - measurement only): its counts and the refusals its choices meet.
	void NotePressureCard(UGalaxySimulationSubsystem& Sim, FCosmicRulerPolicyCounts& Counts)
	{
		const FGalaxyState& State = Sim.GetState();
		const FName Card = State.Knowing.PendingPetitionRow;
		const FEncounterDef* Row = Card.IsNone() ? nullptr : Sim.GetTables().FindEncounter(Card);
		const int32 Ruled = Sim.GetStateIndex().FindCiv(State, State.Player.RuledCivID);
		if (!Row || !IsPressureCard(*Row) || !State.Civs.IsValidIndex(Ruled))
		{
			return;
		}
		++Counts.PressureCards;
		Counts.PressureCardsByEnd += Counts.bEnded ? 0 : 1;
		Counts.FirstPressureCardTick = Counts.FirstPressureCardTick < 0 ? State.Tick : Counts.FirstPressureCardTick;
		Counts.ClaimCards += Card == CosmicRows::PetitionPressureClaim ? 1 : 0;
		Counts.MeasuredCards += Card == CosmicRows::PetitionPressureMeasured ? 1 : 0;
		static_assert(UE_ARRAY_COUNT(Counts.PressureCardsByTier) == static_cast<int32>(EAscensionTier::Tier5_EternalOrder) + 1, "one bin per rung");
		++Counts.PressureCardsByTier[FMath::Clamp(static_cast<int32>(State.Civs[Ruled].Tier), 0, static_cast<int32>(EAscensionTier::Tier5_EternalOrder))];
		const int32 SystemIndex = CosmicPetitions::SubjectOf(State, Sim.GetStateIndex(), Ruled, Row->Subject, State.Knowing.PendingPetitionSystemID);
		for (const FEncounterChoice& Choice : Row->Choices)
		{
			const FString Word = !CosmicMegaprojects::CanPay(State.Civs[Ruled].Stockpile, CosmicPetitions::CostOf(Choice)) ? FString(TEXT("stockpile"))
				: GreyedWordOf(CosmicPetitions::ChoiceGateOf(State, Sim.GetStateIndex(), Sim.GetTables(), Sim.GetRules(), Ruled, SystemIndex, NAME_None, Choice));
			if (!Word.IsEmpty())
			{
				++Counts.PressureGreyed.FindOrAdd(Word);
			}
		}
	}
}

void CosmicRulerPolicy::Act(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
	FCosmicRulerPolicyCounts& Counts)
{
	using namespace CosmicRulerPolicyImpl;
	const FGalaxyState& State = Sim.GetState();
	Counts.bEnded = Counts.bEnded || State.Ending.Kind.IsValid();

	// The ruler's cards: a wanderer's card after a deposition is not one (B1 - the ruler line stays the ruler's).
	const bool bRuler = State.Player.Role == EPlayerRole::Ruler;
	const bool bCame = bRuler && !State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != CardBefore;
	Counts.CardsCome += bCame ? 1 : 0;
	if (bCame)
	{
		NotePressureCard(Sim, Counts);
	}
	for (int32 Guard = 0; Policy.bAnswer && Guard < 8 && !State.Knowing.PendingPetitionRow.IsNone(); ++Guard)
	{
		const FName Card = State.Knowing.PendingPetitionRow;
		const FEncounterDef* Row = Sim.GetTables().FindEncounter(Card);
		const int32 Choice = Row ? ChoiceOf(State, Sim.GetStateIndex(), *Row, Counts.Answers, Card, Policy.Diplomacy, &Sim.GetRules(), &Sim.GetTables(), Policy.Pressure) : INDEX_NONE;
		if (Choice == INDEX_NONE)
		{
			break;
		}
		const int32 RuledBefore = Sim.GetStateIndex().FindCiv(State, State.Player.RuledCivID);
		const float InsightBefore = State.Civs.IsValidIndex(RuledBefore) ? State.Civs[RuledBefore].Stockpile.Insight : 0.f;
		const FCosmicPetitionAnswer Taken = Role.AnswerPetition(Choice);
		if (Taken.Outcome == ECosmicPetitionOutcome::Refused)
		{
			break;
		}
		// B4 (measurement): the kind of answer a pressure card took.
		if (Taken.Outcome == ECosmicPetitionOutcome::Succeeded && IsPressureCard(*Row))
		{
			const EPressureKind Kind = KindOf(Row->Choices[Choice], VigilRowOf(Sim.GetTables()));
			const int32 KindAt = Kind == EPressureKind::Vigil ? 0 : (Kind == EPressureKind::Discern ? 1 : (Kind == EPressureKind::Watch ? 2
				: (Kind == EPressureKind::Edict ? 3 : (Kind == EPressureKind::Resist ? 4 : INDEX_NONE))));
			if (KindAt != INDEX_NONE)
			{
				++Counts.PressureAnswers[KindAt];
				Counts.PressureAnswersByEnd += Counts.bEnded ? 0 : 1;
			}
			if (Kind == EPressureKind::Discern)
			{
				Counts.DiscernInsightMin = Counts.DiscernInsightMin < 0.f ? InsightBefore : FMath::Min(Counts.DiscernInsightMin, InsightBefore);
			}
		}
		// B4: the edicts a pressure card's accepted answer enacted (decision 9 - MakeRoomFor keeps them while a claim stands).
		const int32 Ruled = Sim.GetStateIndex().FindCiv(State, State.Player.RuledCivID);
		for (const FCosmicEffect& Effect : Taken.Outcome == ECosmicPetitionOutcome::Succeeded ? Row->Choices[Choice].OnSuccess : TArray<FCosmicEffect>())
		{
			if (Effect.Type == EEffectType::EnactEdict && State.Civs.IsValidIndex(Ruled) && State.Civs[Ruled].ActiveEdicts.Contains(Effect.Key))
			{
				Counts.CardEdicts.AddUnique(Effect.Key);
				++Counts.CardEnactments.FindOrAdd(Effect.Key);
			}
		}
		Counts.Answers += (Taken.Outcome == ECosmicPetitionOutcome::Withdrawn) ? 0 : 1;
		const bool bChained = State.Player.Role == EPlayerRole::Ruler && !State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != Card;
		Counts.CardsCome += bChained ? 1 : 0;
		if (bChained)
		{
			NotePressureCard(Sim, Counts);
		}
	}
	for (int32 Guard = 0; Policy.bBuild && Guard < 8; ++Guard)
	{
		// The attack policy leaves its last realm slot to a conquest (a people at its cap besieges nothing - B3): it starts
		// no settlement that would fill it.
		if (Policy.Diplomacy == 2 && State.Player.Role == EPlayerRole::Ruler)
		{
			const int32 CivIndex = Sim.GetStateIndex().FindCiv(State, State.Player.RuledCivID);
			if (State.Civs.IsValidIndex(CivIndex) && ProjectChoicesOf(Policy, State, Sim.GetStateIndex(), Sim.GetTables(), Sim.GetRules(), CivIndex).IsEmpty()
				&& CosmicConflict::OwnedCountOf(State, Sim.GetStateIndex(), CivIndex) + 1 >= CosmicMegaprojects::RealmCapOf(Sim.GetRules(), State.Civs[CivIndex].Tier))
			{
				break;
			}
		}
		// Round: the project key as it is (StartNextMegaproject). A pressure policy builds the first project of its own list
		// (ProjectChoicesOf - no relief on a claim), else the first settlement - the key's rule on the list it keeps (B4).
		if (Policy.Pressure == ECosmicPressurePolicy::Round)
		{
			if (!Role.StartNextMegaproject())
			{
				break;
			}
		}
		else
		{
			const int32 CivIndex = State.Player.Role == EPlayerRole::Ruler ? Sim.GetStateIndex().FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
			if (!State.Civs.IsValidIndex(CivIndex) || State.Calling.LifeState != ELifeState::Embodied)
			{
				break;
			}
			const TArray<TPair<int32, FName>> Kept = ProjectChoicesOf(Policy, State, Sim.GetStateIndex(), Sim.GetTables(), Sim.GetRules(), CivIndex);
			const TArray<int32> Targets = Kept.IsEmpty() ? CosmicMegaprojects::SettleTargets(State, Sim.GetStateIndex(), Sim.GetTables(), Sim.GetRules(), CivIndex) : TArray<int32>();
			const bool bStarted = !Kept.IsEmpty() ? Role.StartMegaproject(State.Systems[Kept[0].Key].SystemID, Kept[0].Value)
				: (!Targets.IsEmpty() && Role.StartMegaproject(State.Systems[Targets[0]].SystemID, CosmicRows::ProjectSettle));
			if (!bStarted)
			{
				break;
			}
		}
		++Counts.Started;
	}
	// The next rung's research (MVP item 7): only while the run has not ended (the game takes no input after it).
	if (Policy.bFocus && !(Counts.bEnded && !Policy.bPastEnd))
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
	if (!Policy.bEdict || (Counts.bEnded && !Policy.bPastEnd) || State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler)
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
		Counts.CardEdictsRevoked += Counts.CardEdicts.Contains(Row) ? 1 : 0;
		Counts.CardEdicts.Remove(Row);
		return true;
	};
	// B4: while a claim stands on a settled system of the people, the edicts the pressure cards enacted stay (decision 9),
	// and under a pressure policy so does the vigil, whoever enacted it (header section 2.9).
	const bool bClaimStands = ClaimStandsOn(State, Tables, CivIndex);
	const FName Vigil = VigilRowOf(Tables);
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
	// Room for Row: a free slot, or one made by revoking a held edict that answers no standing hazard (EdictRow after the
	// others and a galaxy law last - B6b) - only when Row could then be enacted: the slots its one reason and the stockpile
	// paying its cost (EnactBlockOf asks the slots before the cost), so no edict is given up for an enactment that would be
	// refused. For the law step (bForLaw - B6b header section 2.4) a galaxy law is never given up: a law takes a free slot or
	// the slot of an edict that is no law, so the laws never turn over among themselves. Nor is the vigil while (3) would
	// enact it again (discontent counted, no rebellion standing) or while Stability is below RevokeBelow: (3) gave up a law
	// for it, and a law taking its slot back would hand it to (3) at the next boundary - the vigil and a law would swap
	// every two boundaries, and under a steady drain again each time Stability climbs back to 0.3, where the discontent is
	// no longer counted. So the law step leaves a band, as (3) and (4) leave one between their edges: the law takes the
	// vigil's slot back once the discontent is no longer counted and Stability is 0.5 or more, or takes the free slot (4)
	// leaves at RestAt.
	auto MakeRoomFor = [&](FName Row, bool bForLaw)
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
		int32 SpareRank = 3;
		for (const FName Held : TArray<FName>(Civ.ActiveEdicts))
		{
			if (Held == Row || !Tables.FindEdict(Held) || Enacted.Contains(Held) || CosmicEdicts::AnswersStanding(State, Index, Tables, CivIndex, Held)
				|| (bClaimStands && (Counts.CardEdicts.Contains(Held) || (Held == Vigil && Policy.Pressure != ECosmicPressurePolicy::Round))))
			{
				continue;
			}
			// The first held edict of the lowest rank: 0 any other, 1 EdictRow, 2 a galaxy law (never for a law).
			const bool bLaw = CosmicEdicts::IsGalaxyLaw(Tables, Held);
			if (bLaw && bForLaw)
			{
				continue;
			}
			if (bForLaw && Held == Vigil
				&& ((State.Civs[CivIndex].LowStabilityTicks > 0 && !RebellionStands()) || State.Civs[CivIndex].Stability < RevokeBelow))
			{
				continue;
			}
			const int32 Rank = bLaw ? 2 : (Held == Policy.EdictRow ? 1 : 0);
			if (Rank < SpareRank)
			{
				Spare = Held;
				SpareRank = Rank;
			}
			if (Rank == 0)
			{
				break;
			}
		}
		return !Spare.IsNone() && Revoke(Spare);
	};

	// (1) Below 0.5, the edicts that cost Stability go - never a galaxy law (B6b: the laws go only to make room, last).
	for (const FName Held : TArray<FName>(State.Civs[CivIndex].ActiveEdicts))
	{
		const FEdictDef* Def = Tables.FindEdict(Held);
		if (Def && Def->Scope != ECosmicEdictScope::HomeGalaxy && CosmicEdicts::HasStabilityCost(*Def) && State.Civs[CivIndex].Stability < RevokeBelow)
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
		const int32 HazardSystem = Index.FindSystem(State, System.SystemID);
		for (const FName Answer : Hazard->RulerResolutions)
		{
			if (Tables.FindEdict(Answer) && CosmicRulerPolicyImpl::IsLeftAlone(Policy, State, Index, Tables, Rules, HazardSystem, Answer))
			{
				break;
			}
			if (Tables.FindEdict(Answer))
			{
				if (!Revoked.Contains(Answer) && MakeRoomFor(Answer, false))
				{
					Enact(Answer);
				}
				break;
			}
		}
	}
	// (3) The vigil against a coming rebellion: discontent counted, no rebellion yet, the vigil not held.
	if (!Vigil.IsNone() && State.Civs[CivIndex].LowStabilityTicks > 0 && !RebellionStands() && !State.Civs[CivIndex].ActiveEdicts.Contains(Vigil)
		&& !Revoked.Contains(Vigil) && MakeRoomFor(Vigil, false) && Enact(Vigil))
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
		// The vigil of (3), or one a card enacted (B4), once the danger passed - never a card's edict while a claim stands, and
		// under a pressure policy not the vigil either, whoever enacted it.
		const bool bVigilDone = (Held == Counts.VigilEnacted || (Held == Vigil && Counts.CardEdicts.Contains(Held))) && State.Civs[CivIndex].Stability >= RestAt
			&& !RebellionStands() && !(bClaimStands && (Counts.CardEdicts.Contains(Held) || (Held == Vigil && Policy.Pressure != ECosmicPressurePolicy::Round)));
		if (bIdleAnswer || bVigilDone)
		{
			Revoke(Held);
		}
	}
	// (4') B6b (header section 2.4): the galaxy laws the row judged at the people's rung asks for (CosmicEdicts::AsksForLawsOf),
	// those it lacks in table order, each through MakeRoomFor for a law - into a free slot or by giving up an edict that is
	// no law. It stands still while the slots are fewer than DT_Rules.GalaxyLawsToHold (the count could not be reached) or
	// while an answer edict holds a slot (the answer of (2) came first: the law whose slot it took waits until the answer
	// goes in (4)), and it does not take the slot of the vigil while (3) would enact it again or Stability is below 0.5
	// (MakeRoomFor - the law takes the vigil's slot back once the discontent is no longer counted and Stability is 0.5 or
	// more, or takes the free slot (4) leaves) - so the laws never turn over among themselves or with the vigil and cost an
	// enactment, a line and a decision a boundary.
	if (CosmicEdicts::AsksForLawsOf(State.Civs[CivIndex], Tables))
	{
		const FCivilizationState& Ruling = State.Civs[CivIndex];
		const bool bAnswerHolds = Ruling.ActiveEdicts.ContainsByPredicate([&Tables](FName Held) { return CosmicEdicts::IsAnswerEdict(Tables, Held); });
		if (CosmicEdicts::SlotsOf(Ruling, Rules) >= CosmicEdicts::GalaxyLawsToHoldOf(Rules) && !bAnswerHolds)
		{
			for (const FName Law : CosmicEdicts::GalaxyLawRowsOf(Tables))
			{
				if (!State.Civs[CivIndex].ActiveEdicts.Contains(Law) && !Revoked.Contains(Law) && MakeRoomFor(Law, true))
				{
					Enact(Law);
				}
			}
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
	// The guard of -Guard=1 (B4) runs with no diplomacy and after the end too; everything else needs both.
	const bool bGuard = Policy.Guard != 0;
	if ((!bGuard && (Policy.Diplomacy == 0 || (Counts.bEnded && !Policy.bPastEnd))) || State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler)
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
	// over a system no longer besieged is withdrawn - or, under -Guard=1, moved to the system GuardTargetOf keeps (a guard
	// that stands where it should stays, and the steps below run on).
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
		else if (Besieged.IsNone() && bGuard)
		{
			const FName Keep = GuardTargetOf(State, Index, Tables, Rules, CivIndex);
			if (!Keep.IsNone() && Keep != Before && Role.AllocateFleet(Keep) == CosmicConflict::ECosmicSiegeBlock::None && TargetOf() != Before)
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

	if (Policy.Diplomacy == 0 || (Counts.bEnded && !Policy.bPastEnd))
	{
		return;
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
