// Cosmic Ascension - World layer: the wanderer policy a script plays between ticks (B1 header section 5).

#include "World/CosmicWandererPolicy.h"

#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicPetitions.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"

void CosmicWandererPolicy::AbdicateIfDue(const FCosmicWandererPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role,
	FCosmicWandererPolicyCounts& Counts)
{
	const FGalaxyState& State = Sim.GetState();
	Counts.bEnded = Counts.bEnded || CosmicEndings::ProvisionalEndOf(State, Sim.GetRules()) != ECosmicProvisionalEnd::None;
	if (!Counts.bEnded && Policy.AbdicateAt > 0 && State.Tick == Policy.AbdicateAt)
	{
		Role.Abdicate();
	}
}

void CosmicWandererPolicy::Act(const FCosmicWandererPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
	FCosmicWandererPolicyCounts& Counts)
{
	const FGalaxyState& State = Sim.GetState();
	const FCosmicStateIndex& Index = Sim.GetStateIndex();
	Counts.bEnded = Counts.bEnded || CosmicEndings::ProvisionalEndOf(State, Sim.GetRules()) != ECosmicProvisionalEnd::None;
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
	Counts.Cards += (!State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != CardBefore) ? 1 : 0;
	if (!Policy.bWander || Counts.bEnded)
	{
		return;
	}

	// 1. A waiting card: its choices in turn (a wanderer pays nothing, so every choice can be given).
	for (int32 Guard = 0; Guard < 8 && !State.Knowing.PendingPetitionRow.IsNone(); ++Guard)
	{
		const FName Card = State.Knowing.PendingPetitionRow;
		const FEncounterDef* Row = Sim.GetTables().FindEncounter(Card);
		if (!Row || Row->Choices.IsEmpty())
		{
			break;
		}
		const FCosmicPetitionAnswer Taken = Role.AnswerPetition(Counts.Answers % Row->Choices.Num());
		if (Taken.Outcome == ECosmicPetitionOutcome::Refused)
		{
			break;
		}
		Counts.Answers += (Taken.Outcome == ECosmicPetitionOutcome::Withdrawn) ? 0 : 1;
		if (Taken.Outcome == ECosmicPetitionOutcome::Failed)
		{
			++Counts.FailedByRow.FindOrAdd(Card);
		}
		Counts.Cards += (!State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != Card) ? 1 : 0;
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
