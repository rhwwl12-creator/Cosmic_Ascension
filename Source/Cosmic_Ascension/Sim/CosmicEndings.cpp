// Cosmic Ascension - Sim layer.

#include "Sim/CosmicEndings.h"

#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"
#include "GameplayTagContainer.h"
#include "Sim/CosmicSuccession.h"

FCosmicEndingRecord CosmicEndings::EndingOf(const FGalaxyState& State, const FCosmicRulesDef& Rules)
{
	FCosmicEndingRecord Record;
	const FCallingState& Calling = State.Calling;
	// No life yet, no end.
	if (Calling.Lives.IsEmpty())
	{
		return Record;
	}
	constexpr EAscensionTier FifthRung = EAscensionTier::Tier5_EternalOrder;
	auto StandsOnFifth = [&State](FName CivID)
	{
		return State.Civs.ContainsByPredicate([CivID](const FCivilizationState& Civ) { return Civ.CivID == CivID && Civ.Tier == EAscensionTier::Tier5_EternalOrder; });
	};

	// 1 The ruled people: some life's peak. The people is the last of that life's reigns that stands on the fifth rung now.
	for (const FLifeRecord& Life : Calling.Lives)
	{
		if (Life.PeakTier != FifthRung)
		{
			continue;
		}
		Record.Kind = CosmicTags::EndingRuledPeople();
		for (int32 Reign = Life.RuledCivIDs.Num() - 1; Reign >= 0; --Reign)
		{
			if (StandsOnFifth(Life.RuledCivIDs[Reign]))
			{
				Record.CivID = Life.RuledCivIDs[Reign];
				break;
			}
		}
		break;
	}
	// 2 The vessel's own rung (unreached in B6a: nothing raises the personal rung).
	if (!Record.Kind.IsValid() && Calling.LifeState == ELifeState::Embodied && Calling.PersonalTier == FifthRung)
	{
		Record.Kind = CosmicTags::EndingVessel();
	}
	// 3 Another people on the fifth rung: the first in Civs order.
	if (!Record.Kind.IsValid())
	{
		for (const FCivilizationState& Civ : State.Civs)
		{
			if (Civ.Tier == FifthRung)
			{
				Record.Kind = CosmicTags::EndingOtherPeople();
				Record.CivID = Civ.CivID;
				break;
			}
		}
	}
	// 4 The silence: the tenth stage found no candidate and the calling stays in the interregnum.
	if (!Record.Kind.IsValid() && CosmicSuccession::IsSilent(Calling))
	{
		Record.Kind = CosmicTags::EndingNoCandidate();
	}
	// 5 The clock.
	if (!Record.Kind.IsValid() && Rules.ProvisionalEndTick > 0 && State.Tick >= static_cast<int64>(Rules.ProvisionalEndTick))
	{
		Record.Kind = CosmicTags::EndingClock();
	}
	if (!Record.Kind.IsValid())
	{
		return FCosmicEndingRecord();
	}
	Record.Tick = State.Tick;
	// An end written in the interregnum was met with no vessel - but not a fifth rung reached in the tick the vessel died: the
	// rung was reached at the end of the AI stage, while it stood, before the tenth stage carried the death out.
	const bool bRungOfThisTick = (Record.Kind == CosmicTags::EndingRuledPeople() || Record.Kind == CosmicTags::EndingOtherPeople())
		&& Calling.Lives.Last().DiedTick == State.Tick;
	Record.bNoVessel = Calling.LifeState == ELifeState::Interregnum && !bRungOfThisTick;
	return Record;
}

const TCHAR* CosmicEndings::NameOf(const FGameplayTag& Kind)
{
	if (!Kind.IsValid())
	{
		return TEXT("None");
	}
	if (Kind == CosmicTags::EndingRuledPeople())
	{
		return TEXT("RuledPeople");
	}
	if (Kind == CosmicTags::EndingVessel())
	{
		return TEXT("Vessel");
	}
	if (Kind == CosmicTags::EndingOtherPeople())
	{
		return TEXT("OtherPeople");
	}
	if (Kind == CosmicTags::EndingNoCandidate())
	{
		return TEXT("NoCandidate");
	}
	if (Kind == CosmicTags::EndingClock())
	{
		return TEXT("Clock");
	}
	return TEXT("None");
}
