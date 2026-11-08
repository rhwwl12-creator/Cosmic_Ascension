// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicSeal.h"

#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "GameplayTagContainer.h"

int32 CosmicSeal::BindTargetOf(const FGalaxyState& State, const FCosmicDataTables& Tables)
{
	const FGameplayTag AbyssalTag = CosmicTags::RealmAbyssal();
	for (int32 At = 0; At < State.Transcendents.Num(); ++At)
	{
		const FTranscendentState& Transcendent = State.Transcendents[At];
		const FTranscendentDef* Def = Transcendent.bSealed ? nullptr : Tables.FindTranscendent(Transcendent.TranscendentID);
		if (Def && Def->Allegiance == AbyssalTag)
		{
			return At;
		}
	}
	return INDEX_NONE;
}

bool CosmicSeal::Bind(FGalaxyState& State, int32 TranscendentIndex, int64 Tick)
{
	if (!State.Transcendents.IsValidIndex(TranscendentIndex) || State.Transcendents[TranscendentIndex].bSealed)
	{
		return false;
	}
	FTranscendentState& Transcendent = State.Transcendents[TranscendentIndex];
	Transcendent.bSealed = true;
	Transcendent.SealedTick = Tick;
	return true;
}
