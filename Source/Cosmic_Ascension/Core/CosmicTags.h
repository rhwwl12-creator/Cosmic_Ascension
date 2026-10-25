// Cosmic Ascension - Core layer. The gameplay tags the code names by name, one function each (CLAUDE.md rule 9: declared
// in Config/DefaultGameplayTags.ini, obtained with RequestGameplayTag(FName)). A constant header for Tools/check_layers.py
// (its tag literals are not counted). B3 began it with the stances and treaties.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

namespace CosmicTags
{
	// FCosmicTie::Stance (B3, section 5.5). The empty tag is neutral.
	inline FGameplayTag StanceFriendly()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Stance.Friendly")));
		return Tag;
	}

	inline FGameplayTag StanceHostile()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Stance.Hostile")));
		return Tag;
	}

	// FCosmicTie::Treaties (B3, section 5.5).
	inline FGameplayTag TreatyTrade()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Treaty.Trade")));
		return Tag;
	}

	inline FGameplayTag TreatyNonAggression()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Treaty.NonAggression")));
		return Tag;
	}

	// The hazard whose spread the crisis stage multiplies (B3 section 2.7).
	inline FGameplayTag HazardPlague()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Plague")));
		return Tag;
	}
}
