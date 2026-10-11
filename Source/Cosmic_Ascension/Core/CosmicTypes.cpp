// Cosmic Ascension - Core layer.

#include "Core/CosmicTypes.h"

float FResourcePool::Get(ECosmicResource Resource) const
{
	switch (Resource)
	{
	case ECosmicResource::Energy: return Energy;
	case ECosmicResource::Matter: return Matter;
	case ECosmicResource::Insight: return Insight;
	case ECosmicResource::Influence: return Influence;
	case ECosmicResource::Causality: return Causality;
	}
	return 0.f;
}

void FResourcePool::Add(ECosmicResource Resource, float Amount)
{
	switch (Resource)
	{
	case ECosmicResource::Energy: Energy += Amount; break;
	case ECosmicResource::Matter: Matter += Amount; break;
	case ECosmicResource::Insight: Insight += Amount; break;
	case ECosmicResource::Influence: Influence += Amount; break;
	case ECosmicResource::Causality: Causality += Amount; break;
	}
}
