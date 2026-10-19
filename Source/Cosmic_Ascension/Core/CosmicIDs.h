// Cosmic Ascension - Core layer. Design: guideline.md section 4.5, 7.2.
// Permanent system identifiers and pure predicates for sanctuary and first home.
// RULE (section 6.1): files under Core/ must never include Data, Sim, Events or presentation headers.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NameTypes.h"

namespace CosmicIDs
{
	inline const FName Sanctuary(TEXT("sol"));
	inline const FName FirstHome(TEXT("sys_0"));
}

namespace CosmicState
{
	inline bool IsSanctuary(FName SystemID)
	{
		return SystemID == CosmicIDs::Sanctuary;
	}

	inline bool IsFirstHome(FName SystemID)
	{
		return SystemID == CosmicIDs::FirstHome;
	}
}
