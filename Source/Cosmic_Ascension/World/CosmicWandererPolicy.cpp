// Cosmic Ascension - World layer: the wanderer policy a script plays between ticks (B1 header section 5).

#include "World/CosmicWandererPolicy.h"

#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "World/CosmicRulerPolicy.h"

int32 CosmicWandererPolicy::PickOf(const FEncounterDef& Row, int32 AnswersSoFar, int32 Crossroads)
{
	const int32 Count = Row.Choices.Num();
	if (Count <= 0)
	{
		return INDEX_NONE;
	}
	if (!CosmicRulerPolicy::IsCrossroadsCard(Row))
	{
		return AnswersSoFar % Count;
	}
	// A crossroads card (B7x [B-7x4m]): the k-th choice that moves a brick under -Crossroads, else the no-value choice.
	if (Crossroads > 0)
	{
		int32 Seen = 0;
		for (int32 Pick = 0; Pick < Count; ++Pick)
		{
			if (CosmicRulerPolicy::MovesABrickOf(Row.Choices[Pick]))
			{
				++Seen;
				if (Seen == Crossroads)
				{
					return Pick;
				}
			}
		}
	}
	for (int32 Pick = 0; Pick < Count; ++Pick)
	{
		if (CosmicRulerPolicy::IsNoValueChoice(Row.Choices[Pick]))
		{
			return Pick;
		}
	}
	return INDEX_NONE;
}

void CosmicWandererPolicy::AbdicateIfDue(const FCosmicWandererPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role,
	FCosmicWandererPolicyCounts& Counts)
{
	const FGalaxyState& State = Sim.GetState();
	Counts.bEnded = Counts.bEnded || State.Ending.Kind.IsValid();
	if (!(Counts.bEnded && !Policy.bPastEnd) && Policy.AbdicateAt > 0 && State.Tick == Policy.AbdicateAt)
	{
		Role.Abdicate();
	}
}

void CosmicWandererPolicy::Act(const FCosmicWandererPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
	FCosmicWandererPolicyCounts& Counts)
{
	const FGalaxyState& State = Sim.GetState();
	const FCosmicStateIndex& Index = Sim.GetStateIndex();
	Counts.bEnded = Counts.bEnded || State.Ending.Kind.IsValid();
	// The first leaving, whatever its cause (the line names it 'left at').
	const bool bWalks = CosmicWanderer::IsEmbodiedWanderer(State);
	if (bWalks && Counts.LeftAt < 0)
	{
		Counts.LeftAt = State.Tick;
	}
	if (!bWalks)
	{
		return;
	}
	for (const FCivilizationState& Civ : State.Civs)
	{
		Counts.ReverencePeak = FMath::Max(Counts.ReverencePeak, Civ.ReverenceToPlayer);
	}
	if (!State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != CardBefore)
	{
		++Counts.Cards;
		++Counts.CardsByRow.FindOrAdd(State.Knowing.PendingPetitionRow);
	}
	if (!Policy.bWander || (Counts.bEnded && !Policy.bPastEnd))
	{
		return;
	}

	// 1. A waiting card: its choices in turn (a wanderer pays nothing, so every choice can be given); on a crossroads card the
	// no-value choice, or the -Crossroads one (PickOf - B7x [B-7x4m]).
	for (int32 Guard = 0; Guard < 8 && !State.Knowing.PendingPetitionRow.IsNone(); ++Guard)
	{
		const FName Card = State.Knowing.PendingPetitionRow;
		const FEncounterDef* Row = Sim.GetTables().FindEncounter(Card);
		const int32 Pick = Row ? PickOf(*Row, Counts.Answers, Policy.Crossroads) : INDEX_NONE;
		if (Pick == INDEX_NONE)
		{
			break;
		}
		const FCosmicPetitionAnswer Taken = Role.AnswerPetition(Pick);
		if (Taken.Outcome == ECosmicPetitionOutcome::Refused)
		{
			break;
		}
		// B6b [B-6b8]: whether the choice taken (Pick, as above - B7x [B-7x4m] reads it after PickOf) has no check - the step back.
		const bool bNoCheck = Row->Choices[Pick].CheckStat == ECosmicCheckStat::NoCheck;
		Counts.Answers += (Taken.Outcome == ECosmicPetitionOutcome::Withdrawn) ? 0 : 1;
		if (Taken.Outcome == ECosmicPetitionOutcome::Withdrawn)
		{
			++Counts.WithdrawnByRow.FindOrAdd(Card);
		}
		else
		{
			++Counts.AnswersByRow.FindOrAdd(Card);
		}
		if (Taken.Outcome == ECosmicPetitionOutcome::Failed)
		{
			++Counts.FailedByRow.FindOrAdd(Card);
		}
		if (bNoCheck && Taken.Outcome != ECosmicPetitionOutcome::Withdrawn)
		{
			++Counts.NoCheckByRow.FindOrAdd(Card);
		}
		if (!State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != Card)
		{
			++Counts.Cards;
			++Counts.CardsByRow.FindOrAdd(State.Knowing.PendingPetitionRow);
		}
	}
	// An answer lifts reverence at once - the coronation it opens may come at this very boundary.
	for (const FCivilizationState& Civ : State.Civs)
	{
		Counts.ReverencePeak = FMath::Max(Counts.ReverencePeak, Civ.ReverenceToPlayer);
	}

	// 2. The throne of the people of the system it stands in, when the gate opens.
	const int32 Here = CosmicWanderer::StandingIndexOf(State, Index);
	if (Here != INDEX_NONE)
	{
		const FName Owner = State.Systems[Here].OwnerCivID;
		const int32 OwnerIndex = Owner.IsNone() ? INDEX_NONE : Index.FindCiv(State, Owner);
		const bool bMet = State.Knowing.MetCivs.ContainsByPredicate([Owner](const FKnownMark& Mark) { return Mark.ID == Owner; });
		if (State.Civs.IsValidIndex(OwnerIndex)
			&& CosmicWanderer::CrownBlockOf(State, Index, Sim.GetRules(), Owner, bMet, !State.Knowing.PendingPetitionRow.IsNone()) == CosmicWanderer::ECrownBlock::None)
		{
			const FCivilizationState& Civ = State.Civs[OwnerIndex];
			const TCHAR* const Path = CosmicWanderer::IsReturn(State, Owner) ? TEXT("return")
				: (Civ.ReverenceToPlayer >= Sim.GetRules().CoronationReverence ? TEXT("reverence") : TEXT("vacuum"));
			const EAscensionTier Tier = Civ.Tier;
			if (Role.BecomeRuler(Owner) == CosmicWanderer::ECrownBlock::None)
			{
				Counts.LastCrownedAt = State.Tick;
				Counts.LastCrownedPath = Path;
				// The first throne taken again is the line's (a later fall and throne do not overwrite it).
				if (Counts.CrownedAt < 0)
				{
					Counts.CrownedAt = State.Tick;
					Counts.CrownedCivID = Owner;
					Counts.CrownedTier = Tier;
					Counts.CrownedPath = Path;
				}
				return;
			}
		}

		// 3. One scan on each arrival.
		if (Counts.ScannedAt != State.Systems[Here].SystemID && Role.ScanCurrentSystem() != INDEX_NONE)
		{
			Counts.ScannedAt = State.Systems[Here].SystemID;
			++Counts.Scans;
		}

		// 4. On: the first lane neighbour with a crisis standing, not straight back unless it must; else the next in turn.
		TArray<FName, TInlineAllocator<8>> Open;
		for (const FName Lane : State.Systems[Here].Lanes)
		{
			if (CosmicWanderer::TravelBlockOf(State, Index, Lane, !State.Knowing.PendingPetitionRow.IsNone()) == CosmicWanderer::ETravelBlock::None)
			{
				Open.Add(Lane);
			}
		}
		FName Target;
		for (const FName Lane : Open)
		{
			const int32 There = Index.FindSystem(State, Lane);
			if ((Lane != Counts.LastLeft || Open.Num() == 1) && State.Systems.IsValidIndex(There) && State.Systems[There].ActiveHazard.IsValid())
			{
				Target = Lane;
				break;
			}
		}
		if (Target.IsNone() && !Open.IsEmpty())
		{
			Target = Open[Counts.Journeys % Open.Num()];
		}
		if (!Target.IsNone() && Role.TravelTo(Target) == CosmicWanderer::ETravelBlock::None)
		{
			Counts.LastLeft = State.Systems[Here].SystemID;
			++Counts.Journeys;
		}
	}
}
