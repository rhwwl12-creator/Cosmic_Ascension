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

	// B6a [B-6a5]: the fallen's awareness line (B6a header section 2.11) - the one second person, once per run, told at the
	// end of the expiry whose claim made its claimant aware of the player. Heard in the feed only: the life chapter leaves it
	// out (CosmicChronicle) and the two books read Log.Claim and Log.Record alone.
	inline FGameplayTag LogTranscendent()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Transcendent")));
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

	// B5 [B-5c]: the pages and the digest. The log categories the digest leaves to the chronicle (Log.Calling, Log.Record -
	// with LogClaim above), the seats whose change is a throne item, and the hazard whose taking of a throne the digest reads.
	inline FGameplayTag LogCalling()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Calling")));
		return Tag;
	}

	inline FGameplayTag LogRecord()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Record")));
		return Tag;
	}

	inline FGameplayTag NotableRuler()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Ruler")));
		return Tag;
	}

	inline FGameplayTag NotableRegent()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Regent")));
		return Tag;
	}

	inline FGameplayTag HazardRebellion()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Rebellion")));
		return Tag;
	}

	// B5 [B-5c]: the causes of death the epitaph names (FLifeRecord::CauseOfDeath) and the reign's ends the reign end's page
	// names (Reign.Died above is the death's).
	inline FGameplayTag DeathHazard()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Death.Hazard")));
		return Tag;
	}

	inline FGameplayTag DeathEncounter()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Death.Encounter")));
		return Tag;
	}

	inline FGameplayTag DeathAge()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Death.Age")));
		return Tag;
	}

	inline FGameplayTag ReignAbdicated()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")));
		return Tag;
	}

	inline FGameplayTag ReignDeposed()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed")));
		return Tag;
	}

	// B6a: the kinds of FCosmicEndingRecord::Kind (B6a header section 2.1, decision 6-ga) - saved names, facts only, never a
	// screen title. The empty tag is a run that goes on.
	inline FGameplayTag EndingRuledPeople()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ending.RuledPeople")));
		return Tag;
	}

	inline FGameplayTag EndingVessel()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ending.Vessel")));
		return Tag;
	}

	inline FGameplayTag EndingOtherPeople()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ending.OtherPeople")));
		return Tag;
	}

	inline FGameplayTag EndingNoCandidate()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ending.NoCandidate")));
		return Tag;
	}

	inline FGameplayTag EndingClock()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ending.Clock")));
		return Tag;
	}

	// B7x [B-7x1]: the existence bricks (docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md, rule R1) - tags of
	// FCivilizationState::Traits that Sim/CosmicExistenceBricks reads with HasTagExact (from [B-7x2]). Saved names, never renamed.
	inline FGameplayTag BrickFormSociety()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Form.Society")));
		return Tag;
	}

	inline FGameplayTag BrickFormLivingNebula()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Form.LivingNebula")));
		return Tag;
	}

	// The shell closed around its own star; not the fallen sphere (InSphere/FallenSphereTicks) nor Trait.Faith.BookShut.
	inline FGameplayTag BrickFormShutSphere()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Form.ShutSphere")));
		return Tag;
	}

	inline FGameplayTag BrickFormKeptLaw()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Form.KeptLaw")));
		return Tag;
	}

	inline FGameplayTag BrickDietDarkMatter()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Diet.DarkMatter")));
		return Tag;
	}

	inline FGameplayTag BrickDietStarEater()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Diet.StarEater")));
		return Tag;
	}

	inline FGameplayTag BrickDietIdleFeed()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Diet.IdleFeed")));
		return Tag;
	}

	inline FGameplayTag BrickDietDistanceTension()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Diet.DistanceTension")));
		return Tag;
	}

	inline FGameplayTag BrickDietCrisisParasite()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Diet.CrisisParasite")));
		return Tag;
	}

	inline FGameplayTag BrickSpaceNomad()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Space.Nomad")));
		return Tag;
	}

	inline FGameplayTag BrickSpaceClosedPort()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Space.ClosedPort")));
		return Tag;
	}

	inline FGameplayTag BrickSpaceFoldedSmall()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Space.FoldedSmall")));
		return Tag;
	}

	inline FGameplayTag BrickInstinctSwarmTech()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Instinct.SwarmTech")));
		return Tag;
	}

	inline FGameplayTag BrickInstinctHiveMerge()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Instinct.HiveMerge")));
		return Tag;
	}

	inline FGameplayTag BrickMutationStarTow()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Mutation.StarTow")));
		return Tag;
	}

	inline FGameplayTag BrickMutationDustBand()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Mutation.DustBand")));
		return Tag;
	}

	inline FGameplayTag BrickMutationInwardHunger()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Brick.Mutation.InwardHunger")));
		return Tag;
	}

	// B7x [B-7x2]: the hazards no brick touches - the parasite's countdown bonus passes them by, as a galaxy law's hooks do
	// (schema, B6b header 3.3): the claim, the extinction and the supernova precursor.
	inline FGameplayTag HazardClaim()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
		return Tag;
	}

	inline FGameplayTag HazardExtinction()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Extinction")));
		return Tag;
	}

	inline FGameplayTag HazardPreSupernova()
	{
		static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.PreSupernova")));
		return Tag;
	}

	// B7x [B-7x2]: whether a tag is an existence brick - under the root Brick., read without case as an FName is (the seed
	// bricks of NewGalaxy look for one).
	inline bool IsBrick(const FGameplayTag& Tag)
	{
		return Tag.IsValid() && Tag.GetTagName().ToString().StartsWith(TEXT("Brick."), ESearchCase::IgnoreCase);
	}

	// B7x [B-7x2] (rule R10): the seven star classes from the hottest to the coolest - the one list of them: the generation and
	// the economy's star energy read it (Sim/GalaxySimulationSubsystem.cpp), and a star eater's capital dims along it
	// (CosmicBricks::DimmerStarClassOf).
	inline constexpr int32 StarClassCount = 7;

	inline FGameplayTag StarClassAt(int32 HotToCool)
	{
		static const FGameplayTag Tags[StarClassCount] = {
			FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.O"))),
			FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.B"))),
			FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.A"))),
			FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.F"))),
			FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.G"))),
			FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.K"))),
			FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.M"))),
		};
		return (HotToCool >= 0 && HotToCool < StarClassCount) ? Tags[HotToCool] : FGameplayTag();
	}
}
