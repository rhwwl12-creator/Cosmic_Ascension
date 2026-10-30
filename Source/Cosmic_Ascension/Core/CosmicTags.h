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

	// The fallen's first-person line of a claim that joined its sphere (B4 section 2.6) - Log.Claim is that line alone: the
	// claim's narrator lines are Log.Crisis (the hazard row) or Log.Record, and its sign is Log.Sign (log_sign_claim).
	inline FGameplayTag LogClaim()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")));
		return Tag;
	}

	// B5: the role effect of a death (FCallingState::PendingEffect), the reign's end of a death, and the realm of the fallen
	// whose sphere a succession avoids.
	inline FGameplayTag EffectKillPlayer()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect.KillPlayer")));
		return Tag;
	}

	inline FGameplayTag ReignDied()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Died")));
		return Tag;
	}

	inline FGameplayTag RealmAbyssal()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")));
		return Tag;
	}
}
