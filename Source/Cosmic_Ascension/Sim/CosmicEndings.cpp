// Cosmic Ascension - Sim layer.

#include "Sim/CosmicEndings.h"

#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"

ECosmicProvisionalEnd CosmicEndings::ProvisionalEndOf(const FGalaxyState& State, const FCosmicRulesDef& Rules)
{
	// No life yet, no end; the third rung no longer ends a run (B6a [B-6a3a]).
	if (State.Calling.Lives.IsEmpty())
	{
		return ECosmicProvisionalEnd::None;
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
