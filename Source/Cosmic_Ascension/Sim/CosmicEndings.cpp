// Cosmic Ascension - Sim layer.

#include "Sim/CosmicEndings.h"

#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"

ECosmicProvisionalEnd CosmicEndings::ProvisionalEndOf(const FGalaxyState& State, const FCosmicRulesDef& Rules)
{
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	if (Lives.IsEmpty())
	{
		return ECosmicProvisionalEnd::None;
	}
	for (const FLifeRecord& Life : Lives)
	{
		if (static_cast<int32>(Life.PeakTier) >= static_cast<int32>(EndRung))
		{
			return ECosmicProvisionalEnd::Rung;
		}
	}
	if (Rules.ProvisionalEndTick > 0 && State.Tick >= static_cast<int64>(Rules.ProvisionalEndTick))
	{
		return ECosmicProvisionalEnd::Clock;
	}
	return ECosmicProvisionalEnd::None;
}

const TCHAR* CosmicEndings::NameOf(ECosmicProvisionalEnd End)
{
	switch (End)
	{
	case ECosmicProvisionalEnd::Rung:
		return TEXT("Rung");
	case ECosmicProvisionalEnd::Clock:
		return TEXT("Clock");
	default:
		return TEXT("None");
	}
}
