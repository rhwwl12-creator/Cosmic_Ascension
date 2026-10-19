// Cosmic Ascension - Core layer. Design: guideline.md section 4.5, 7.2.
// Permanent system identifiers and pure predicates for sanctuary and first home.
// RULE (section 6.1): files under Core/ must never include Data, Sim, Events or presentation headers.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NameTypes.h"

namespace CosmicIDs
{
	// The sanctuary (section 4.5 (1), Law.Hedge): the one node no people owns, settles, claims or destroys. NewGalaxy puts
	// it at the end of State.Systems, without a draw (rule 11 of the generation).
	inline const FName Sanctuary(TEXT("sol"));
	// The home of the first life: system 0 of State.Systems (section 5.10).
	inline const FName FirstHome(TEXT("sys_0"));
}

namespace CosmicState
{
	// Whether SystemID is the sanctuary. The ID alone decides - no state is read -, so the simulation, the presentation and
	// the tests give the same answer.
	inline bool IsSanctuary(FName SystemID)
	{
		return SystemID == CosmicIDs::Sanctuary;
	}

	inline bool IsFirstHome(FName SystemID)
	{
		return SystemID == CosmicIDs::FirstHome;
	}
}
