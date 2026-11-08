// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicAscent.h"

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"

bool CosmicAscent::FourthRungOpenOf(const FGalaxyState& State, const FCosmicDataTables& Tables, FName GalaxyRow)
{
	const FGalaxyDef* Galaxy = Tables.FindGalaxy(GalaxyRow);
	if (!Galaxy || Galaxy->AIFourthRungCap <= 0)
	{
		return true;
	}
	int32 Count = 0;
	for (const FCivilizationState& Civ : State.Civs)
	{
		Count += (Civ.GalaxyRow == GalaxyRow && static_cast<int32>(Civ.Tier) >= static_cast<int32>(EAscensionTier::Tier4_Transcendent)) ? 1 : 0;
	}
	for (const FTransitRecord& Transit : State.Transits)
	{
		Count += (Transit.ToGalaxyRow == GalaxyRow && Transit.ArriveTick >= State.Tick) ? 1 : 0;
	}
	return Count < Galaxy->AIFourthRungCap;
}
