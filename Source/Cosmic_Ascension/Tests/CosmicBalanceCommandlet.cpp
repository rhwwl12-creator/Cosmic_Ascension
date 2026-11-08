// Cosmic Ascension - balance commandlet.

#include "Tests/CosmicBalanceCommandlet.h"

#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Events/CosmicEventSubsystem.h"
#include "Events/CosmicPetitions.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"
#include "GameplayTagContainer.h"
#include "Log/CosmicChronicle.h"
#include "Math/NumericLimits.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSlot.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Compression/OodleDataCompression.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicSuccession.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicCallingPagesWidget.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "World/CosmicDigest.h"
#include "World/CosmicRulerPolicy.h"
#include "World/CosmicWandererPolicy.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/CosmicTestHelpers.h"
#endif

namespace
{
	constexpr int64 SampleEveryTicks = 100;
	// SaveBytes is measured on every tenth sample only: one save of the whole state takes tens of milliseconds.
	constexpr int64 SaveEverySamples = 10;

	// The claims that ended one way, with what their owners held when they ended (section 9.3: why a claim runs out).
	struct FClaimEnds
	{
		int32 Count = 0;
		// Ended with an answer (project_relief in the game's data) in the response slot of the system.
		int32 WithAnswer = 0;
		// The owner's Influence before the tick in which the claim ended, summed.
		double OwnerInfluence = 0.0;
	};

	// What the run counts tick by tick, read from the state only (no hook into the simulation, no draw).
	struct FRunCounts
	{
		int64 FirstClaimTick = -1;
		// Where the first claim stood and the tick its slot emptied (-1 while it stands): the spiritual sight's claim
		// screenshot reads them (CLAUDE.md, the screenshot recipe).
		FName FirstClaimSystemID;
		int64 FirstClaimEndTick = -1;
		int64 FirstJoinTick = -1;
		int64 DeposedTick = -1;
		// The first tick at which some people holds two settled systems: claims can stand from then on (Law.Remnant).
		int64 FirstTwoSettledTick = -1;
		int32 ClaimsAfterTwoSettled = 0;
		FClaimEnds ClaimsJoined;
		FClaimEnds ClaimsStilled;
		int64 ToldLines = 0;
		TMap<FString, int32> Placed;
		TMap<FString, int32> Answered;
		TMap<FString, int32> Expired;
		TMap<FString, int32> Ended;
		// B2: hazards that ended at their count's end with an answering edict held and its upkeep payable - resolved, not
		// expired (an approximation: another edict's upkeep that took the stockpile first in that tick is not seen).
		int32 ResolvedByEdict = 0;
		// B2: the edicts of the people the vessel rules - lines of lapse, held ticks by row; the edicts the AI peoples gave up
		// (one per row a people held before a tick and not after it); the first rebellion (a hazard of a Capital row) placed
		// on the ruled people's capital.
		int32 EdictsLapsed = 0;
		TMap<FName, int64> EdictHeldTicks;
		int32 AIEdictsRevoked = 0;
		int64 FirstRebellionTick = -1;
		// Why the peoples the player does not rule started no settlement, one count per (sample, people) - the tick-0 sample
		// included -, and how many such pairs there were, counted apart from the reasons (PeoplesNotRuled at every sample:
		// the reasons must add up to it).
		TMap<FString, int32> Expansion;
		int64 ExpansionPairs = 0;
		// The ruler's side (-Answer, -Build, -Focus): the cards that came, the answers taken, the projects started, the focus
		// changes taken (MVP item 7), their sum (the decisions) and the sums and told lines at the ends of the first 600 and
		// 6,000 ticks, the sum at the boundary the record of the end first stands, before the input (the decisions the game
		// takes - none after it; -1 without an end);
		// the ticks at which the ruled people reached the second and the third rung (-1 while it has not).
		int32 CardsCome = 0;
		int32 Answers = 0;
		int32 Started = 0;
		int32 FocusChanges = 0;
		int64 DecisionsAtEnd = -1;
		int64 DecisionsAt600 = -1;
		int64 DecisionsAt6000 = -1;
		int64 ToldAt600 = -1;
		int64 ToldAt6000 = -1;
		int64 RuledT2Tick = -1;
		int64 RuledT3Tick = -1;
		// The run's end (MVP item 4; since B6a [B-6a3c] the record State.Ending - header section 2.7), looked at after every
		// tick and before the policy's input - nothing stops at it: the record's kind and tick at the first boundary it stands
		// (empty and -1 while none does), the first tick of the clock (the knob, read apart from the record), and the last
		// chapter of the chronicle at that boundary. The third rung no longer ends a run (B6a [B-6a3a]).
		FGameplayTag FirstEnd;
		int64 FirstEndTick = -1;
		int64 ClockTick = -1;
		int32 ChapterLife = -1;
		int32 ChapterShown = -1;
		int32 ChapterCandidates = -1;
		int32 ChapterMore = -1;
		// The last chapter's candidates by the cap's rank (Log.Calling, own Severity 2, admitted Severity 1, other peoples'
		// Severity 2): what the cap of CosmicChronicle::MaxLifeLines is set from.
		int32 ChapterRanks[4] = { -1, -1, -1, -1 };
		// B3 (header section 5): the diplomacy and conflict lines. Border pairs at 1,000, 3,000, 6,000 and the end; the tick
		// the first contact card came; treaty pairs signed and broken; hostilities opened by tick band (0-3,000, 3,000-6,000,
		// later); the most AIs hostile toward one people and the most besiegers of one people at once; sieges begun (on the
		// ruled people, and before the first end), taken (from the ruled people), held (the attacker repulsed), lifted; the
		// systems taken by people and lost by people; the median fleet at 3,000 and 6,000; the ticks of delay a guard bought
		// per hazard tag (the sum and the count); the fleet power the attrition took (estimated from the state before the
		// tick) and the AI sieges laid inside a higher sphere; the climbs from T2 to T3 that a taking helped (the people held
		// with a taken system among the three settled systems it needed); plagues placed next to a plague (and on the ruled
		// people's systems); the ruled people's Severity-2 conflict lines (up to and including the tick of the first end,
		// the most in one 600-tick block, the ones in the last chapter); the ruler's window - the second settled system, the
		// first system the ruler could besiege (a met owner, the gate None or NonAggression with hostility assumed); the
		// first contact people turned friendly and the tick a non-aggression treaty with it was signed.
		int64 BorderPairs[4] = { -1, -1, -1, -1 };
		int64 FirstContactCardTick = -1;
		int32 TreatiesSignedPairs = 0;
		int32 TreatiesBrokenPairs = 0;
		int32 HostilitiesOpened[3] = { 0, 0, 0 };
		int32 MaxHostileTowardOne = 0;
		int32 MaxSiegersOnOne = 0;
		int32 SiegesBegun = 0;
		int32 SiegesOnRuled = 0;
		int32 SiegesOnRuledByEnd = 0;
		int32 Taken = 0;
		int32 TakenByBand[3] = { 0, 0, 0 };
		int32 TakenFromRuled = 0;
		int32 TakenFromRuledByEnd = 0;
		int32 Held = 0;
		int32 Lifted = 0;
		TMap<FName, int32> TakenBy;
		TMap<FName, int32> LostBy;
		float FleetMedian[2] = { -1.f, -1.f };
		TMap<FString, TPair<int64, int32>> DelayBought;
		double AttritionLost = 0.0;
		int32 AISiegesIntoSphere = 0;
		int32 ThreeWorldsByConquest = 0;
		// The same for the ruled people alone, the ruled people's plagues before the first end, and two peoples besieging
		// each other at once (ticks with one pair or more, and the most pairs at one tick).
		int32 ThreeWorldsByConquestRuled = 0;
		int32 PlaguePlacedRuledByEnd = 0;
		int64 MutualSiegeTicks = 0;
		int32 MaxMutualPairs = 0;
		int32 PlagueSpreadPlaced = 0;
		int32 PlaguePlacedRuled = 0;
		int32 PlagueSpreadRuled = 0;
		// The systems a people took (the people, the system): the T3 climbs that needed one for three settled systems.
		TSet<TPair<FName, FName>> ConqueredBy;
		int32 ConflictSev2 = 0;
		int32 ConflictSev2ByEnd = 0;
		TMap<int64, int32> ConflictSev2Blocks;
		int32 ConflictSev2InChapter = -1;
		int64 SecondSettledTick = -1;
		int64 FirstSiegeTargetTick = -1;
		int32 FirstContactFriendly = -1;
		int64 FirstContactNonAggressionTick = -1;
	};

	// A people before a tick (B3): its fleet's target and since, the target's owner, its fleet and rung, its hostile stances
	// and its treaties ("OtherID|Treaty").
	struct FCivBefore
	{
		FName Target;
		int64 Since = 0;
		FName TargetOwner;
		float Fleet = 0.f;
		EAscensionTier Tier = EAscensionTier::Tier1_Planetary;
		TSet<FName> Hostile;
		TSet<FString> Treaties;
	};

	FCivBefore CivBeforeOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCivilizationState& Civ)
	{
		FCivBefore Before;
		Before.Target = Civ.FleetTargetSystemID;
		Before.Since = Civ.FleetTargetSinceTick;
		const int32 Target = Civ.FleetTargetSystemID.IsNone() ? INDEX_NONE : Index.FindSystem(State, Civ.FleetTargetSystemID);
		Before.TargetOwner = State.Systems.IsValidIndex(Target) ? State.Systems[Target].OwnerCivID : NAME_None;
		Before.Fleet = Civ.FleetPower;
		Before.Tier = Civ.Tier;
		for (const TPair<FName, FCosmicTie>& Tie : Civ.Ties)
		{
			if (Tie.Value.Stance == CosmicTags::StanceHostile())
			{
				Before.Hostile.Add(Tie.Key);
			}
			for (const FGameplayTag& Treaty : Tie.Value.Treaties)
			{
				Before.Treaties.Add(Tie.Key.ToString() + TEXT("|") + Treaty.ToString());
			}
		}
		return Before;
	}

	// B4 (docs/60_DECISIONS/headers/B4_transcendent_voice_and_pressure_cards.md section 5 - the first measurement): the
	// chances the spiritual pressure and the claims have to reach the people the vessel rules, and how far a claimant's
	// voice could be heard. "By end" counts up to and including the tick of the record of the run's end (State.Ending - since
	// B6a the clock's, before which the season after the third rung now runs; B4's split at the end was a split at the ruled
	// third rung, and the ending line splits there again). Read from the state only; the drain's P from
	// UGalaxySimulationSubsystem::ComputeDrainPressureOf (no copy of its rule here).
	struct FSpiritCounts
	{
		// The ruled people's largest drain P by the bands of the tick run (0-1000, 1000-2000, 2000-3000, 3000-6000, later),
		// read at the boundary before the tick as its economy stage reads it; the ticks with a drain and the Stability the
		// drain took (the knob x P, before the floor at zero).
		float DrainMax[5] = { 0.f, 0.f, 0.f, 0.f, 0.f };
		int64 DrainedTicks = 0;
		int64 DrainedTicksByEnd = 0;
		double Drained = 0.0;
		double DrainedByEnd = 0.0;
		// The first tick run with a settled system of the ruled people next to or inside the sphere of an unsealed fallen
		// transcendent, and inside one.
		int64 FirstNextTo = -1;
		int64 FirstInside = -1;
		// Claims placed on systems of the ruled people; of the claims that ended on them (owner before the tick), the ones
		// that joined a sphere and the others, with the others a project answered.
		int32 ClaimsOnRuled = 0;
		int32 ClaimsOnRuledByEnd = 0;
		int32 Joined = 0;
		int32 JoinedByEnd = 0;
		int32 Stilled = 0;
		int32 StilledByRelief = 0;
		// The other hazards that ended on a settled system of the ruled people that a claimant could reach before the tick
		// (CosmicHazards::ClaimantOf): expired with no answer (the claim rule follows them), the expired ones on the people's
		// last unclaimed settled system (Law.Remnant refuses the claim - a part of the expired), answered by a project,
		// resolved by a held answer edict.
		int32 SourcesExpired = 0;
		int32 SourcesExpiredByEnd = 0;
		int32 SourcesAnswered = 0;
		int32 SourcesByEdict = 0;
		int32 SourcesRemnant = 0;
		// The pressure card's two triggers (header decision 8) coming to hold on a settled system of the ruled people, at
		// the boundary after a tick from 1,800 on, while the vessel rules embodied: (a) a claim stands there; (b) its P is
		// 0.9 or more (SystemPressureAtLeast - the part with a dominant source above the people apart). A slot taken: a
		// card waits at that boundary (one that waited before, or one placed in the tick).
		int32 TriggerClaim = 0;
		int32 TriggerClaimTaken = 0;
		int32 TriggerPressure = 0;
		int32 TriggerPressureTaken = 0;
		int32 TriggerPressureHigher = 0;
		TSet<FName> ClaimHeld;
		TSet<FName> PressureHeld;
		// The gate of the policy's vigil row (CosmicEdicts::EnactBlockOf) when (a) came to hold with the slot free.
		int32 VigilOpen = 0;
		int32 VigilHeld = 0;
		int32 VigilSlots = 0;
		int32 VigilCost = 0;
		int32 VigilOther = 0;
		// Met peoples other than the ruled one with a settled system next to or inside such a sphere, at 600, 1,200, the end.
		int32 MetNear[3] = { -1, -1, -1 };
		// The joins: on a known system whose owner is met (an approximation of the scope of telling - section 8.1 - without
		// its tier gap and window), by end, and by the viewer's rung (first or second, third or above); the most in a tick.
		int32 KnownJoins = 0;
		int32 KnownJoinsByEnd = 0;
		int32 KnownJoinsLow = 0;
		int32 KnownJoinsHigh = 0;
		int32 SameTickJoinsMax = 0;
		// The spheres' size at 1,000, 3,000, 6,000 and the end.
		int32 SphereAt[4] = { -1, -1, -1, -1 };
	};

	// B4 ([B-4d2] - header section 2.8, 5): a claim on a settled system of the ruled people that the claim card came for, or
	// that the fleet's time delayed: the tick the card came and the claim's count then, whether the vigil was not held at a
	// boundary from the card on (after the ruler's input) or a project answered the system meanwhile, whether the watch
	// answered its card, and how often the fleet delayed it.
	struct FClaimTrack
	{
		int64 CardTick = -1;
		int32 RemainingAtCard = -1;
		bool bVigilBroken = false;
		bool bRelief = false;
		bool bWatched = false;
		int32 Delays = 0;
	};

	// B4 (header section 5 - the head of the pressure line, with the policy values): the hazards placed on a system the ruled
	// people's fleet guarded before the tick, every one (a delay of 0 included), by tag; the delay the guard bought them, by
	// tag; how they ended. The cards' and the voice's cells follow with their commits.
	struct FPressureCounts
	{
		TMap<FString, int32> GuardedByTag;
		int32 Guarded = 0;
		int32 GuardedByEnd = 0;
		TMap<FString, int64> DelayByTag;
		int64 DelayBought = 0;
		int32 GuardedAnswered = 0;
		int32 GuardedExpired = 0;
		int32 GuardedByEdict = 0;
		// The guarded hazards standing now: system -> tag.
		TMap<FName, FGameplayTag> Standing;
		// The voice ([B-4d1] - header section 2.6, 2.7, 5 measure 8): the fallen's first-person lines (Log.Claim lines whose
		// source is a transcendent row) told and kept untold, the sign lines (Log.Sign) told, the most of each told in one
		// 600-tick block, the first told of each, and the ruled people's fleet worn lines.
		int32 VoiceTold = 0;
		int32 VoiceToldByEnd = 0;
		int32 VoiceUntold = 0;
		int32 SignTold = 0;
		int32 SignToldByEnd = 0;
		TMap<int64, int32> VoiceBlocks;
		TMap<int64, int32> SignBlocks;
		int64 FirstSign = -1;
		int64 FirstVoice = -1;
		int32 FleetWorn = 0;
		// The cards ([B-4d2]): when the claim trigger first held on a system and another card held the slot, one that waited
		// before the tick or one placed in it, and of both the cards that held the slot, by row; the count the fleet's time bought claims and the fleet it took, the claims it
		// delayed twice; the vigil's ease lines (event_claim_eased - the one row that resolves a claim); the ends of the claims
		// on settled systems of the ruled people (joined, eased, a project, other, expired unjoined) and those no claim card
		// came for; of the claims the card came for that kept the vigil and no project from the card to their end, by the
		// band of their count at the card (300 or less, 301-400, 401-550, more), and the eased of each band; the claims whose
		// card the watch answered, and the joined of them.
		int32 ClaimLostWait = 0;
		int32 ClaimLostSame = 0;
		TMap<FString, int32> ClaimLostTo;
		int64 ResistBought = 0;
		double FleetSpent = 0.0;
		int32 DelayedTwice = 0;
		int32 Eased = 0;
		int32 EasedByEnd = 0;
		int32 Ends[5] = { 0, 0, 0, 0, 0 };
		int32 NoCard = 0;
		int32 Band[4] = { 0, 0, 0, 0 };
		int32 BandEased[4] = { 0, 0, 0, 0 };
		int32 Watched = 0;
		int32 WatchedJoined = 0;
		TMap<FName, FClaimTrack> Claims;
		// #131 (header section 5 measure 10): after each time the fleet bought, the ticks until that people's fleet is back to
		// its size before (read at the boundaries); the ones still short at the end.
		struct FRegrowing
		{
			FName CivID;
			int64 Since = 0;
			float Before = 0.f;
		};
		TArray<FRegrowing> Regrowing;
		TArray<int64> Regrown;
		// The ruler's Standing cards that came to hold in a tick and were not put (another one was, or one waited), by row; the
		// units held after the ruler's input at the last boundary (the head of the next tick, as the knowing stage compares).
		TMap<FString, int32> StandingLost;
		TArray<FCosmicPetitionUnit> HeldAfterInput;
	};

	// B6a [B-6a3c] (header section 5.1, 5.2): the season's decisions are counted per this many ticks (docs/00_VISION.md - a
	// decision every 300 to 600 ticks).
	constexpr int64 SeasonWindowTicks = 600;

	// B6a [B-6a3c]: the guarded crises of one tag in one phase - their count, the delay the guard bought them and the sum of
	// their rows' countdowns (the count each was placed with before the guard and the edicts' bonus): #132's share.
	struct FGuardedPhase
	{
		int32 Count = 0;
		int64 Delay = 0;
		int64 Countdown = 0;
	};

	// B6a [B-6a3c] (header section 5.1, notes section 2.1): the ending line - the season after the ruled third rung, read from
	// the state, the record of the run's end (FGalaxyState::Ending) and the policy's own counts (no rule copied). A phase
	// splits B4's counts at the ruled third rung instead of at the end: 0 before the season, 1 the season (after the ruled
	// third rung up to and including the record's tick - the "by end" of every line), 2 after the record.
	struct FEndingCounts
	{
		// The first boundary at which a people the vessel rules stood at the third rung or above (read after the tick and again
		// after the inputs - a throne taken over such a people starts the season there), and that people; -1 while none has.
		// The first reign's people alone is the ruler line's "T3 at" (FRunCounts::RuledT3Tick); this one is any reign's.
		int64 SeasonStart = -1;
		FName SeasonCivID;
		// The boundaries at which the ruler policy's decisions (CosmicRulerPolicy::DecisionsOf) grew, and by how many.
		TArray<TPair<int64, int32>> Decisions;
		int32 DecisionsSoFar = 0;
		// The exact first tick an AI people (not ruled before that tick) climbed to the third rung, read at every boundary;
		// the people and the feat row of that climb (the source row of its Log.Ascension line - None when none was stored).
		int64 FirstAIT3Tick = -1;
		FName FirstAIT3CivID;
		FName FirstAIT3Feat;
		// Per people, per research field: the first boundary its progress stood at 1.0 (the research stage's cap).
		TMap<FName, TMap<FName, int64>> ResearchFull;
		// The first boundary at which some transcendent's bAwareOfPlayer stood, that transcendent, and whether the vessel had
		// left by that boundary (not embodied, not a ruler, or a new life - a leave recorded after the flag in the same tick,
		// which the flag's gate cannot see: B6a header section 2.11).
		int64 AwareTick = -1;
		FName AwareTranscendentID;
		bool bAwareVesselLeft = false;
		// The awareness lines (Log.Transcendent) told, by the record, and the first told.
		int32 AwarenessTold = 0;
		int32 AwarenessToldByEnd = 0;
		int64 FirstAwarenessTold = -1;
		// Claims placed in the season on settled systems of a people the vessel rules, and the first of them.
		int32 SeasonClaims = 0;
		int64 FirstSeasonClaim = -1;
		// By phase: #132 the guarded crises by tag; #142 the claims of the band 401-550 that ended (the pressure line's band
		// 2 - the card came, the vigil held, no relief), and the joined of them; #135 the claim trigger's first holds and the
		// claim cards it lost to a card that waited before the tick and to one put in it; #145 the sieges begun on the ruled
		// people's systems while its fleet regrows after a resistance (FPressureCounts::Regrowing, an entry of an earlier
		// boundary).
		TMap<FString, FGuardedPhase> Guarded[3];
		int32 Band142[3] = { 0, 0, 0 };
		int32 Band142Joined[3] = { 0, 0, 0 };
		int32 ClaimTrigger[3] = { 0, 0, 0 };
		int32 ClaimLostWait[3] = { 0, 0, 0 };
		int32 ClaimLostSame[3] = { 0, 0, 0 };
		int32 SiegesRegrowing[3] = { 0, 0, 0 };
		// #135's first holds, keyed "tick:system:phase:k|w|s" (k kept - the slot was free or held the claim's own card, w lost
		// to a card that waited before the tick, s lost to one put in it), so the runs that share a world are counted once by
		// (seed, tick, system) (notes section 2).
		TArray<FString> ClaimTriggerKeys;
		// #132: by phase, the fleet power the attrition took in the tick from the people the vessel ruled before it (the
		// attrition stage's own estimate, as the conflict line's attrition).
		double RuledAttrition[3] = { 0.0, 0.0, 0.0 };
		// The first boundary, by the record, at which a people the vessel rules counted LowStabilityTicks above 0 (the stretch
		// before a rebellion on its capital - the seed of hand check 6).
		int64 LowStabilityTick = -1;
		// The ruled people's Insight and Influence at the boundary each pressure card came, before the ruler's input, by the
		// phase that boundary reads (#144).
		TArray<float> CardInsight[3];
		TArray<float> CardInfluence[3];
		// petition_exalt: the first boundary it waited for the ruler, and the first choice the ruler policy took on it
		// (CosmicRulerPolicy::ChoiceOf, as Act asks it, read while no answer has been taken; INDEX_NONE: none taken - no
		// -Answer, or every time refused or withdrawn).
		int64 ExaltTick = -1;
		int32 ExaltChoice = INDEX_NONE;
		// The pre-count of petition_exalt (notes section 2): each boundary at which its B6a condition (an embodied ruler, the
		// ruled people at the second rung or above) came to hold in the tick - "tick:kind:row", kind waited (a card waited at
		// the tick's head), floor (no card put), contest (another Standing card put), other (a card of another trigger put),
		// row the card put ("-" for none). It came to hold as the knowing stage compares: it holds at this boundary and did not
		// after the last boundary's inputs, or the tick began a reign (its head is compared empty).
		TArray<FString> ExaltPreCount;
		bool bExaltHeldAfterInput = false;
	};

	// The ruler's Standing petition units whose conditions hold, as CosmicPetitions::StandingHeld reads them but with a card
	// waiting too (B4 [B-4d2] - which cards came to hold in a tick and lost it to the one put).
	TArray<FCosmicPetitionUnit> StandingUnitsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
		const FCosmicRulesDef& Rules, int32 CivIndex)
	{
		TArray<FCosmicPetitionUnit> Held;
		if (!State.Civs.IsValidIndex(CivIndex))
		{
			return Held;
		}
		for (const FName RowName : Tables.GetEncounterRowNames())
		{
			const FEncounterDef* Row = Tables.FindEncounter(RowName);
			if (!Row || Row->Trigger != ECosmicEventTrigger::Standing || CosmicPetitions::IsWandererCard(*Row))
			{
				continue;
			}
			TArray<int32> Units;
			if (Row->Subject == ECosmicEventSubject::Civilization)
			{
				Units.Add(CosmicPetitions::SubjectOf(State, Index, CivIndex, Row->Subject, NAME_None));
			}
			else
			{
				for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
				{
					const int32 SystemIndex = CosmicPetitions::SubjectOf(State, Index, CivIndex, Row->Subject, SystemID);
					if (SystemIndex != INDEX_NONE)
					{
						Units.Add(SystemIndex);
					}
				}
			}
			for (const int32 SystemIndex : Units)
			{
				if (CosmicPetitions::Holds(State, Index, Tables, Rules, RowName, *Row, CivIndex, SystemIndex))
				{
					Held.Add({ RowName, SystemIndex });
				}
			}
		}
		return Held;
	}

	// The median of the values (the lower middle of an even count), -1 for none.
	int64 MedianOf(TArray<int64> Values)
	{
		if (Values.IsEmpty())
		{
			return -1;
		}
		Values.Sort();
		return Values[(Values.Num() - 1) / 2];
	}

	int32 MostOf(const TMap<int64, int32>& Blocks)
	{
		int32 Most = 0;
		for (const TPair<int64, int32>& Block : Blocks)
		{
			Most = FMath::Max(Most, Block.Value);
		}
		return Most;
	}

	FString TagCountsText(const TMap<FString, int32>& Counts)
	{
		TArray<FString> Keys;
		Counts.GetKeys(Keys);
		Keys.Sort();
		TArray<FString> Parts;
		for (const FString& Key : Keys)
		{
			Parts.Add(FString::Printf(TEXT("%s=%d"), *Key, Counts[Key]));
		}
		return FString::Join(Parts, TEXT(" "));
	}

	FString TagTicksText(const TMap<FString, int64>& Counts)
	{
		TArray<FString> Keys;
		Counts.GetKeys(Keys);
		Keys.Sort();
		TArray<FString> Parts;
		for (const FString& Key : Keys)
		{
			Parts.Add(FString::Printf(TEXT("%s=%lld"), *Key, Counts[Key]));
		}
		return FString::Join(Parts, TEXT(" "));
	}

	// B6a [B-6a3c]: the lower middle and the least of the values, -1 for none (as MedianOf).
	float MedianOfFloats(TArray<float> Values)
	{
		if (Values.IsEmpty())
		{
			return -1.f;
		}
		Values.Sort();
		return Values[(Values.Num() - 1) / 2];
	}

	float LeastOfFloats(const TArray<float>& Values)
	{
		float Least = -1.f;
		for (const float Each : Values)
		{
			Least = (Least < 0.f || Each < Least) ? Each : Least;
		}
		return Least;
	}

	// B6a [B-6a3c]: one count by phase as "<before the season>/<the season>/<after the record>".
	FString PhasesText(const int32 (&Counts)[3])
	{
		return FString::Printf(TEXT("%d/%d/%d"), Counts[0], Counts[1], Counts[2]);
	}

	// B6a [B-6a3c] (#144): one stock by phase as "<least>:<median>" each ("-" for none), joined as PhasesText.
	FString StockPhasesText(const TArray<float> (&Values)[3])
	{
		FString Text;
		for (int32 Phase = 0; Phase < 3; ++Phase)
		{
			Text += Phase > 0 ? TEXT("/") : TEXT("");
			Text += Values[Phase].IsEmpty() ? FString(TEXT("-")) : FString::Printf(TEXT("%.1f:%.1f"), LeastOfFloats(Values[Phase]), MedianOfFloats(Values[Phase]));
		}
		return Text;
	}

	// B6a [B-6a3c] (#132): the guarded crises of one phase as "<tag>=<count>:<share>%" - the delay bought over the sum of their
	// countdowns; "-" for none.
	FString GuardedPhaseText(const TMap<FString, FGuardedPhase>& Guarded)
	{
		TArray<FString> Keys;
		Guarded.GetKeys(Keys);
		Keys.Sort();
		TArray<FString> Parts;
		for (const FString& Key : Keys)
		{
			const FGuardedPhase& Each = Guarded[Key];
			Parts.Add(FString::Printf(TEXT("%s=%d:%.1f%%"), *Key, Each.Count, Each.Countdown > 0 ? 100.0 * static_cast<double>(Each.Delay) / static_cast<double>(Each.Countdown) : 0.0));
		}
		return Parts.IsEmpty() ? FString(TEXT("-")) : FString::Join(Parts, TEXT(" "));
	}

	// B6a [B-6a3c]: a people's first ticks at research 1.0 as "<field>=<tick>" over Fields (-1: not reached).
	FString ResearchFullText(const FEndingCounts& Ending, FName CivID, const TArray<FName>& Fields)
	{
		const TMap<FName, int64>* Full = CivID.IsNone() ? nullptr : Ending.ResearchFull.Find(CivID);
		TArray<FString> Parts;
		for (const FName Field : Fields)
		{
			const int64* Tick = Full ? Full->Find(Field) : nullptr;
			Parts.Add(FString::Printf(TEXT("%s=%lld"), *Field.ToString(), Tick ? *Tick : static_cast<int64>(-1)));
		}
		return FString::Join(Parts, TEXT(" "));
	}

	// How near the sphere of an unsealed fallen (Realm.Abyssal) transcendent SystemIndex lies: Inside, Adjacent or None.
	ECosmicSphereReach FallenReachOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 SystemIndex)
	{
		static const FGameplayTag Abyssal = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")));
		ECosmicSphereReach Reach = ECosmicSphereReach::None;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			const FTranscendentDef* Def = Transcendent.bSealed ? nullptr : Tables.FindTranscendent(Transcendent.TranscendentID);
			if (!Def || Def->Allegiance != Abyssal)
			{
				continue;
			}
			const ECosmicSphereReach Here = UGalaxySimulationSubsystem::SphereReachIn(State, Transcendent, SystemIndex);
			if (Here == ECosmicSphereReach::Inside)
			{
				return Here;
			}
			Reach = Here == ECosmicSphereReach::Adjacent ? Here : Reach;
		}
		return Reach;
	}

	// The met peoples but RuledID with a settled system next to or inside such a sphere.
	int32 MetPeoplesNearFallen(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, FName RuledID)
	{
		int32 Count = 0;
		for (const FKnownMark& Mark : State.Knowing.MetCivs)
		{
			const int32 CivIndex = Mark.ID == RuledID ? INDEX_NONE : Index.FindCiv(State, Mark.ID);
			if (!State.Civs.IsValidIndex(CivIndex))
			{
				continue;
			}
			for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
			{
				const int32 SystemIndex = Index.FindSystem(State, SystemID);
				if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Mark.ID && CosmicState::IsSettled(State.Systems[SystemIndex])
					&& FallenReachOf(State, Tables, SystemIndex) != ECosmicSphereReach::None)
				{
					++Count;
					break;
				}
			}
		}
		return Count;
	}

	// The pairs of peoples whose settled systems touch (CosmicDiplomacy::BorderTableOf, each pair once).
	int32 BorderPairsOf(const FGalaxyState& State, const FCosmicStateIndex& Index)
	{
		int32 Sum = 0;
		for (const TArray<int32>& Row : CosmicDiplomacy::BorderTableOf(State, Index))
		{
			Sum += Row.Num();
		}
		return Sum / 2;
	}

	float FleetMedianOf(const FGalaxyState& State)
	{
		TArray<float> Fleets;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Fleets.Add(Civ.FleetPower);
		}
		Fleets.Sort();
		return Fleets.IsEmpty() ? 0.f : Fleets[Fleets.Num() / 2];
	}

	// B5 ([B-5a] - header section 5.1): a death chance - a hazard that expired at Tick on SystemID while a KillPlayer on that
	// system would have reached the vessel (the crisis stage's note on the stage clock - FCosmicStageClock::FExpiry: it asks
	// CosmicHazards::VesselAtRiskOf at the moment Apply would). bGrace: the remnant's grace kept the system at its gate (the
	// effects did not run); bBranch: the expiry took its branch effects; bAfterEnd: the record of the end stood before the tick.
	// Gap: ticks since the last reign ended (deposition or abdication), -1 for none.
	struct FDeathChance
	{
		FGameplayTag Tag;
		int64 Tick = 0;
		FName SystemID;
		bool bGrace = false;
		bool bBranch = false;
		bool bAfterEnd = false;
		int64 Gap = -1;
	};

	// B5: the calling line ([B-5a] measured it first with the data off; since [B-5d2] the game's data kills, and
	// -WithoutDeath=1 is the data-off side). The reign's end and the next throne as the role reads
	// them at the boundaries; the death chances; open question #126 - ticks at which an AI people's aim turned on the ruled
	// people only once the gate's Capital and LastSettled were lifted (CosmicConflict::AIAimOf both ways), split by the ruled
	// people's one-settled-system window, and the same between AI peoples (pair-ticks, for reference); open question #133 -
	// the ruler's window again with every realm cap raised (B'), and the first reasons of that gate while B' had not opened.
	struct FCallingCounts
	{
		int64 ReignEndTick = -1;
		int64 FirstReignEndTick = -1;
		int64 CrownedAfterTick = -1;
		TArray<FDeathChance> Chances;
		int32 BlockedRuledTicks = 0;
		int32 BlockedRuledOneSystem = 0;
		int32 BlockedRuledLater = 0;
		int32 BlockedRuledByEnd = 0;
		int64 FirstBlockedRuled = -1;
		// Runs of consecutive such ticks (a rising edge), and whether the last boundary had one.
		int32 BlockedRuledEpisodes = 0;
		bool bBlockedRuledLast = false;
		int32 BlockedAIPairTicks = 0;
		int64 WindowRaisedTick = -1;
		TMap<FString, int32> WindowRaisedReasons;
		// [B-5b]: the deaths (before the end, by cause, by source row), the first death's tick, role and gap from the last
		// reign's end, every leap (its length, whether the cap cut it, whether death + leap reach the clock), every
		// succession, the first throne after a succession and its way, the end's kind, and the reigns begun while a claim
		// that had its card still stood (old #143 - B5 decision 4 closed it).
		int32 Deaths = 0;
		int32 DeathsByEnd = 0;
		TMap<FString, int32> DeathsByCause;
		TMap<FString, int32> DeathsByRow;
		int64 FirstDeathTick = -1;
		const TCHAR* FirstDeathRole = TEXT("none");
		int64 FirstDeathGap = -1;
		FString Leaps;
		FString Successions;
		int64 FirstSuccessionTick = -1;
		int64 ThroneAfterSuccession = -1;
		const TCHAR* ThroneAfterSuccessionPath = TEXT("none");
		int32 ReignsDuringCardedClaims = 0;
		// The system the vessel stood in at its death (the succession's candidates are counted from it).
		int32 DeathSystemIndex = INDEX_NONE;
		int32 DeathAtRecorded = -1;
		// The ticks run in the interregnum and their time (the leap's wall clock - header section 5.1).
		int64 LeapTicksRun = 0;
		double LeapMicros = 0.0;
		// [B-5r] (balance review method-2, play-5; header section 5.1 - record only): per death "<tick>:<answer>:<reverence>".
		// The answer: 'a' when the epitaph sets the life's newest answer apart as the card of the hazard that closed it
		// (CosmicCallingPages::LastAnswersOf's bOfTheDeath at the boundary where the life closed - what the page shows there),
		// 'c' when that newest answer is the killing hazard's card all the same (CosmicCallingPages::IsCardOfHazard - answered
		// on another system, or its title left out of the epitaph), '-' else. The reverence: the highest ReverenceToPlayer of
		// any people at the boundary before the tick that closed the life (the death clears every people's at that tick's end;
		// the tick's own changes before it are not seen). Per succession, the ticks from the birth to that life's first throne
		// (-1: none by the run's end) - a successor crowned in a leader vacuum at or near its birth tick (#141).
		FString DeathAnswers;
		int32 DeathsSetApart = 0;
		int32 DeathsOnTheCard = 0;
		int32 DeathsAtCrowningReverence = 0;
		TArray<int64> SuccessorThroneGaps;
		int32 SuccessorLifeAt = INDEX_NONE;
	};

	// The names of CosmicConflict::ECosmicSiegeBlock in its order (the calling line).
	const TCHAR* SiegeBlockNameOf(CosmicConflict::ECosmicSiegeBlock Block)
	{
		static const TCHAR* const Names[] = { TEXT("None"), TEXT("NotRuler"), TEXT("Sanctuary"), TEXT("NoSystem"), TEXT("Dead"), TEXT("Unowned"), TEXT("Own"),
			TEXT("NotSettled"), TEXT("Capital"), TEXT("LastSettled"), TEXT("NotAdjacent"), TEXT("Besieged"), TEXT("RealmCap"), TEXT("NoFleet"), TEXT("NonAggression"),
			TEXT("NotHostile"), TEXT("Handler") };
		static_assert(static_cast<int32>(UE_ARRAY_COUNT(Names)) == static_cast<int32>(CosmicConflict::ECosmicSiegeBlock::Handler) + 1, "One name per reason, in the order of the enum");
		const int32 At = static_cast<int32>(Block);
		return (At >= 0 && At < static_cast<int32>(UE_ARRAY_COUNT(Names))) ? Names[At] : TEXT("?");
	}

	// The death chances as "<tag leaf>@<tick>:<flags>:<gap>" (flags: g grace, b branch row, e after the end, - none).
	FString ChancesText(const TArray<FDeathChance>& Chances)
	{
		FString Text;
		for (const FDeathChance& Chance : Chances)
		{
			FString Tag = Chance.Tag.ToString();
			Tag.Split(TEXT("."), nullptr, &Tag, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			FString Flags = FString(Chance.bGrace ? TEXT("g") : TEXT("")) + (Chance.bBranch ? TEXT("b") : TEXT("")) + (Chance.bAfterEnd ? TEXT("e") : TEXT(""));
			Text += FString::Printf(TEXT("%s%s@%lld:%s:%lld"), Text.IsEmpty() ? TEXT("") : TEXT(","), *Tag, Chance.Tick, Flags.IsEmpty() ? TEXT("-") : *Flags, Chance.Gap);
		}
		return Text;
	}

	// A slot before the tick: its hazard and count, the answer in its response slot, the owner's Influence, and the
	// construction slot (a settlement that paid nothing in the tick is a stalled one).
	struct FSlotBefore
	{
		FGameplayTag Tag;
		int32 Remaining = 0;
		FName Answer;
		float OwnerInfluence = 0.f;
		CosmicMegaprojects::FCosmicSlotReading Construction;
		// B2: the edict of the owner that answers the hazard (CosmicEdicts::HeldAnswerOf), and whether the stockpile and
		// the income could pay its upkeep in the tick.
		FName HeldAnswer;
		bool bUpkeepPayable = false;
		// B4: the owner is the ruled people and the system settled; a claimant could reach it (CosmicHazards::ClaimantOf);
		// it is the owner's last unclaimed settled system (Law.Remnant). The last two only for such a slot with a hazard.
		bool bRuledSettled = false;
		bool bClaimReach = false;
		bool bLastUnclaimed = false;
	};

	// Whether Tag is the tag of a Capital row of DT_Hazards (a rebellion). Named apart from the ruler policy's helper: one
	// unity file may hold both.
	bool IsRebellionTag(const FCosmicDataTables& Tables, FGameplayTag Tag)
	{
		const FHazardDef* Hazard = Tag.IsValid() ? Tables.FindHazardByTag(Tag) : nullptr;
		return Hazard && Hazard->SpawnRule == ECosmicHazardSpawn::Capital;
	}

	// The names of CosmicMegaprojects::ECosmicSettleBlock in its order, as the CSV and the log print them. NoLane of a people
	// with no settled system prints NoFoothold (the reason is NoLane in the rule; the commandlet tells the two apart).
	const TCHAR* const SettleBlockNames[] = {
		TEXT("None"),
		TEXT("Sanctuary"),
		TEXT("NoPeople"),
		TEXT("Owned"),
		TEXT("Dead"),
		TEXT("NoLane"),
		TEXT("NoRow"),
		TEXT("RowTier"),
		TEXT("RealmCap"),
		TEXT("Settling"),
		TEXT("Sphere"),
	};
	static_assert(static_cast<int32>(UE_ARRAY_COUNT(SettleBlockNames)) == static_cast<int32>(CosmicMegaprojects::ECosmicSettleBlock::Count), "One name per reason, in the order of the enum");

	// Why the people CivIndex starts no settlement now (CosmicMegaprojects::PeopleSettleBlockOf by name): NoFoothold for a
	// people with no settled system, SettlingStalled for a settlement under way that paid nothing in the tick just run (its
	// construction slot against Before - an empty Before, the tick-0 sample, says Settling).
	FString ExpansionOf(const UGalaxySimulationSubsystem& Sim, int32 CivIndex, const TArray<FSlotBefore>& Before)
	{
		const FGalaxyState& State = Sim.GetState();
		const FCosmicStateIndex& Index = Sim.GetStateIndex();
		const CosmicMegaprojects::ECosmicSettleBlock Block = CosmicMegaprojects::PeopleSettleBlockOf(State, Index, Sim.GetTables(), Sim.GetRules(), CivIndex);
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (Block == CosmicMegaprojects::ECosmicSettleBlock::NoLane)
		{
			const bool bFoothold = Civ.OwnedSystems.ContainsByPredicate([&](FName SystemID)
			{
				const int32 SystemIndex = Index.FindSystem(State, SystemID);
				return State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID && CosmicState::IsSettled(State.Systems[SystemIndex]);
			});
			return bFoothold ? TEXT("NoLane") : TEXT("NoFoothold");
		}
		if (Block == CosmicMegaprojects::ECosmicSettleBlock::Settling)
		{
			for (const FName SystemID : Civ.OwnedSystems)
			{
				const int32 SystemIndex = Index.FindSystem(State, SystemID);
				if (!State.Systems.IsValidIndex(SystemIndex) || SystemIndex >= Before.Num() || State.Systems[SystemIndex].OwnerCivID != Civ.CivID
					|| State.Systems[SystemIndex].ActiveMegaproject != CosmicRows::ProjectSettle)
				{
					continue;
				}
				const CosmicMegaprojects::FCosmicSlotReading Now = CosmicMegaprojects::ReadSlot(State.Systems[SystemIndex], CosmicMegaprojects::ECosmicProjectSlot::Construction);
				return CosmicMegaprojects::PaidNothing(Before[SystemIndex].Construction, Now) ? TEXT("SettlingStalled") : TEXT("Settling");
			}
		}
		const int32 At = static_cast<int32>(Block);
		return (At >= 0 && At < static_cast<int32>(UE_ARRAY_COUNT(SettleBlockNames))) ? SettleBlockNames[At] : TEXT("?");
	}

	int32 SphereSize(const FGalaxyState& State)
	{
		int32 Size = 0;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			Size += Transcendent.InfluencedSystems.Num();
		}
		return Size;
	}

	// The sphere of every instance, as "<TranscendentID> <size>".
	TArray<FString> SpheresOf(const FGalaxyState& State)
	{
		TArray<FString> Spheres;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			Spheres.Add(FString::Printf(TEXT("%s %d"), *Transcendent.TranscendentID.ToString(), Transcendent.InfluencedSystems.Num()));
		}
		return Spheres;
	}

	bool HoldsTwoSettled(const FGalaxyState& State)
	{
		TMap<FName, int32> Settled;
		for (const FStarSystemState& System : State.Systems)
		{
			if (!System.OwnerCivID.IsNone() && CosmicState::IsSettled(System) && ++Settled.FindOrAdd(System.OwnerCivID) >= 2)
			{
				return true;
			}
		}
		return false;
	}

	bool InSomeSphere(const FGalaxyState& State, FName SystemID)
	{
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			if (Transcendent.InfluencedSystems.Contains(SystemID))
			{
				return true;
			}
		}
		return false;
	}

	int32 WildSystems(const FGalaxyState& State)
	{
		int32 Wild = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Wild += (System.OwnerCivID.IsNone() && !CosmicHazards::IsDeadNode(System)) ? 1 : 0;
		}
		return Wild;
	}

	// The peoples met, the birth peoples of the lives left out (section 9.3), and the tick of the first contact (empty while
	// there is none).
	int32 MetCivs(const FGalaxyState& State)
	{
		int32 Met = 0;
		for (const FKnownMark& Mark : State.Knowing.MetCivs)
		{
			const bool bBirth = State.Calling.Lives.ContainsByPredicate([&Mark](const FLifeRecord& Life) { return Life.BirthCivID == Mark.ID; });
			Met += bBirth ? 0 : 1;
		}
		return Met;
	}

	FString FirstContactTick(const FGalaxyState& State)
	{
		const FName First = State.Knowing.FirstContactCivID;
		const FKnownMark* Mark = First.IsNone() ? nullptr : State.Knowing.MetCivs.FindByPredicate([First](const FKnownMark& Each) { return Each.ID == First; });
		return Mark ? FString::Printf(TEXT("%lld"), Mark->Tick) : FString();
	}

	// The beings of the fourth rung and above as section 4.2 counts them (layer 4 of section 3.1): the unsealed Tier 4
	// fallen with a sphere, the independents, and the Tier 4 peoples the player does not rule.
	int32 T4PlusCount(const FGalaxyState& State, const FCosmicDataTables& Tables)
	{
		static const FGameplayTag Abyssal = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")));
		static const FGameplayTag Independent = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Independent")));
		const int32 Fourth = static_cast<int32>(EAscensionTier::Tier4_Transcendent);
		int32 Count = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Count += (static_cast<int32>(Civ.Tier) >= Fourth && !Civ.bIsPlayerRuled) ? 1 : 0;
		}
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			const FTranscendentDef* Def = Tables.FindTranscendent(Transcendent.TranscendentID);
			const bool bFallen = Def && Def->Allegiance == Abyssal && static_cast<int32>(Def->Tier) >= Fourth && !Transcendent.bSealed
				&& !Transcendent.InfluencedSystems.IsEmpty();
			Count += (bFallen || (Def && Def->Allegiance == Independent)) ? 1 : 0;
		}
		return Count;
	}

	// The slot bytes of the state as the game writes them (CosmicSaveSlot::Encode: SaveGameToMemory, Oodle, the header), the
	// engine bytes before packing, and the time of the copy, the serialization, the packing and the CRCs. No file write. One
	// save object for the run; time 0 in the header, so the same state gives the same header.
	int32 SaveBytesOf(UCosmicSaveGame* Save, const FGalaxyState& State, int64& OutRawBytes, double& OutMilliseconds)
	{
		TArray<uint8> Slot;
		OutRawBytes = 0;
		const double Start = FPlatformTime::Seconds();
		Save->State = State;
		CosmicSaveSlot::Encode(*Save, 0, Slot, &OutRawBytes);
		OutMilliseconds = (FPlatformTime::Seconds() - Start) * 1000.0;
		return Slot.Num();
	}

	// The chronicle entries the cap of MVP item 5 may never let go (CosmicChronicle::MayLetGo false): the true floor of its size.
	int32 KeptForGood(const FGalaxyState& State)
	{
		int32 Kept = 0;
		for (const FCosmicLogEntry& Entry : State.Chronicle)
		{
			Kept += CosmicChronicle::MayLetGo(Entry) ? 0 : 1;
		}
		return Kept;
	}

	// B5 [B-5d1] (header section 5.1): the two books' input - the entries of Category over the chronicle and the log, told or not
	// (the books read neither bUntold nor Knowing).
	int32 EntriesOf(const FGalaxyState& State, const FGameplayTag& Category)
	{
		int32 Count = 0;
		for (const TArray<FCosmicLogEntry>* Entries : { &State.Chronicle, &State.Log })
		{
			for (const FCosmicLogEntry& Entry : *Entries)
			{
				Count += Entry.Category == Category ? 1 : 0;
			}
		}
		return Count;
	}

	// The lines the second part of the two books shows, over its groups (the heads not counted).
	int32 ClaimedLinesOf(const FCosmicTwoBooks& Books)
	{
		int32 Lines = 0;
		for (const FCosmicClaimedGroup& Group : Books.Claimed)
		{
			Lines += Group.Lines.Num();
		}
		return Lines;
	}

	// The engine bytes of the last sample packed at three Kraken levels apart (the measurement of MVP item 5: size against
	// time). Only the packing is timed.
	FString PackLevelsOf(UCosmicSaveGame* Save, const FGalaxyState& State)
	{
		TArray<uint8> Raw;
		Save->State = State;
		UGameplayStatics::SaveGameToMemory(Save, Raw);
		FString Text = FString::Printf(TEXT("raw %d"), Raw.Num());
		const int64 Bound = FOodleDataCompression::CompressedBufferSizeNeeded(Raw.Num());
		TArray<uint8> Packed;
		Packed.SetNumUninitialized(static_cast<int32>(Bound));
		const TPair<const TCHAR*, FOodleDataCompression::ECompressionLevel> Levels[] = {
			{ TEXT("superfast"), FOodleDataCompression::ECompressionLevel::SuperFast },
			{ TEXT("veryfast"), FOodleDataCompression::ECompressionLevel::VeryFast },
			{ TEXT("fast"), FOodleDataCompression::ECompressionLevel::Fast },
		};
		for (const TPair<const TCHAR*, FOodleDataCompression::ECompressionLevel>& Level : Levels)
		{
			const double Start = FPlatformTime::Seconds();
			const int64 Size = FOodleDataCompression::Compress(Packed.GetData(), Bound, Raw.GetData(), Raw.Num(), FOodleDataCompression::ECompressor::Kraken, Level.Value);
			Text += FString::Printf(TEXT("; %s %lld in %.1f ms"), Level.Key, Size, (FPlatformTime::Seconds() - Start) * 1000.0);
		}
		return Text;
	}

	// The peoples the player does not rule: the (sample, people) pairs one sample adds to the expansion counts, counted
	// apart from AppendSample, so that a people it skips or a filter it gets wrong shows as a sum that does not add up.
	int32 PeoplesNotRuled(const FGalaxyState& State)
	{
		int32 Count = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Count += Civ.bIsPlayerRuled ? 0 : 1;
		}
		return Count;
	}

	// One CSV row per civilization, the galaxy columns on each, and last the expansion reason of a people the player does
	// not rule (empty for the people the player rules), which it counts into Counts.Expansion.
	void AppendSample(FString& Csv, const UGalaxySimulationSubsystem& Sim, const TArray<FSlotBefore>& Before, FRunCounts& Counts, double TickMicros, const FString& SaveBytes,
		const FString& SaveRawBytes, const TMap<FName, float>* Drains)
	{
		const FGalaxyState& State = Sim.GetState();
		const FString FirstClaim = Counts.FirstClaimTick >= 0 ? FString::Printf(TEXT("%lld"), Counts.FirstClaimTick) : FString();
		const FString Galaxy = FString::Printf(TEXT("%d,%d,%d,%s,%d,%s,%lld,%.1f,%s,%d,%d,%s,%d"),
			State.Knowing.KnownSystems.Num(), MetCivs(State), WildSystems(State), *FirstContactTick(State), T4PlusCount(State, Sim.GetTables()),
			*FirstClaim, Counts.ToldLines, TickMicros, *SaveBytes, State.Log.Num(), State.Chronicle.Num(), *SaveRawBytes, State.ChronicleEviction.Count);
		const TArray<TArray<int32>> Borders = CosmicDiplomacy::BorderTableOf(State, Sim.GetStateIndex());
		for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
		{
			const FCivilizationState& Civ = State.Civs[CivIndex];
			// B3: the people's own hostile stances and its treaties.
			int32 Hostile = 0;
			int32 Treaties = 0;
			for (const TPair<FName, FCosmicTie>& Tie : Civ.Ties)
			{
				Hostile += Tie.Value.Stance == CosmicTags::StanceHostile() ? 1 : 0;
				Treaties += Tie.Value.Treaties.Num();
			}
			int32 Hazards = 0;
			for (const FStarSystemState& System : State.Systems)
			{
				Hazards += (System.OwnerCivID == Civ.CivID && System.ActiveHazard.IsValid()) ? 1 : 0;
			}
			FString Expansion;
			if (!Civ.bIsPlayerRuled)
			{
				Expansion = ExpansionOf(Sim, CivIndex, Before);
				Counts.Expansion.FindOrAdd(Expansion)++;
			}
			// B4: the P the economy stage of the sample's tick drained the people by (read at the boundary before that tick -
			// empty for the tick-0 sample and for a people that came meanwhile), and its stay in a fallen sphere.
			const float* Drain = Drains ? Drains->Find(Civ.CivID) : nullptr;
			const FString DrainText = Drain ? FString::Printf(TEXT("%.3f"), *Drain) : FString();
			Csv += FString::Printf(TEXT("%lld,%s,%s,%d,%d,%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%s,%s,%.3f,%d,%d,%d,%s,%d\n"),
				State.Tick, *Civ.CivID.ToString(), *Civ.ArchetypeRow.ToString(), static_cast<int32>(Civ.Tier) + 1, Civ.OwnedSystems.Num(),
				Civ.Stability, Civ.Stockpile.Energy, Civ.Stockpile.Matter, Civ.Stockpile.Insight, Civ.Stockpile.Influence, Civ.Stockpile.Causality, Hazards,
				*Galaxy, *Expansion, Civ.FleetPower, Hostile, Treaties, Borders.IsValidIndex(CivIndex) ? Borders[CivIndex].Num() : 0, *DrainText, Civ.FallenSphereTicks);
		}
	}

	// -Rules=Name:Value;Name:Value - DT_Rules fields replaced for this run only (section 9.3: a knob measured before it is
	// written into the data). Each value is read as the data importer reads it (ImportText). False, with an error line, for
	// a field the row lacks or a value it cannot read.
	bool OverrideRules(UGalaxySimulationSubsystem& Sim, const FString& Text)
	{
		FCosmicRulesDef Row = Sim.GetRules();
		TArray<FString> Pairs;
		Text.ParseIntoArray(Pairs, TEXT(";"));
		for (const FString& Pair : Pairs)
		{
			FString Name;
			FString Value;
			FProperty* Property = Pair.Split(TEXT(":"), &Name, &Value) ? FCosmicRulesDef::StaticStruct()->FindPropertyByName(FName(*Name)) : nullptr;
			if (!Property || !Property->ImportText_Direct(*Value, Property->ContainerPtrToValuePtr<void>(&Row), nullptr, PPF_None))
			{
				UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -Rules cannot set '%s'"), *Pair);
				return false;
			}
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: rules %s = %s"), *Name, *Value);
		}
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FCosmicRulesDef::StaticStruct();
		Table->AddRow(CosmicRows::Rules, Row);
		FCosmicDataTables Tables = Sim.GetTables();
		Tables.Rules = Table;
		Sim.SetTables(Tables);
		return true;
	}

	FString CountsText(const TMap<FString, int32>& Counts)
	{
		TArray<FString> Keys;
		Counts.GetKeys(Keys);
		Keys.Sort();
		FString Text;
		for (const FString& Key : Keys)
		{
			Text += FString::Printf(TEXT(" %s %d"), *Key, Counts[Key]);
		}
		return Text.IsEmpty() ? TEXT(" none") : Text;
	}

	FString ClaimEndsText(const TCHAR* Name, const FClaimEnds& Ends)
	{
		return FString::Printf(TEXT("%d %s (answer running %d, owner Influence mean %.1f)"), Ends.Count, Name, Ends.WithAnswer,
			Ends.Count > 0 ? Ends.OwnerInfluence / Ends.Count : 0.0);
	}

	// B6a [B-6a3b] (header section 5.1): the end check's comparison - the record's four fields.
	bool EndCheckSame(const FCosmicEndingRecord& A, const FCosmicEndingRecord& B)
	{
		return A.Kind == B.Kind && A.Tick == B.Tick && A.CivID == B.CivID && A.bNoVessel == B.bNoVessel;
	}

	FString EndCheckText(const FCosmicEndingRecord& Record)
	{
		return FString::Printf(TEXT("%s at %lld people %s no vessel %d"), CosmicEndings::NameOf(Record.Kind), Record.Tick, *Record.CivID.ToString(),
			Record.bNoVessel ? 1 : 0);
	}
}

int32 UCosmicBalanceCommandlet::Main(const FString& Params)
{
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
	int32 Seed = 42;
	int64 Ticks = 20000;
	// 0 = the home galaxy row (DT_Galaxies: LivingWorlds, Peoples). Give them only to measure another size (section 9.3).
	int32 Systems = 0;
	int32 Civs = 0;
	int32 TellAll = 1;
	// The ruler's policy (section 9.3; B4 adds -PlayerPolicy for the pressure cards, Phase 5 its calling meaning): -Answer=1 answers every card the tick it comes
	// (CosmicRulerPolicy::ChoiceOf), -Build=1 starts every project the ruler's project key would start
	// (StartNextMegaproject: the answers and the next feat's construction, then a settlement) - both between ticks, as a
	// player's input. 0 (the default) leaves the ruler idle: the world of RunHeadless(Ticks).
	int32 Answer = 0;
	int32 Build = 0;
	int32 Focus = 0;
	// B2 (header section 5): -Edict=1 lets the script hold edicts (CosmicRulerPolicy::ActEdicts), -EdictRow=<row> the row it
	// keeps standing when no hazard asks for a slot.
	int32 Edict = 0;
	FString EdictRowText;
	// The wanderer's policy (B1, World/CosmicWandererPolicy): -Abdicate=<tick> abdicates at that tick's boundary (the tenth
	// stage of the next tick leaves), -Wander=1 walks - answers its cards, takes a throne the gate opens, scans, travels.
	int64 AbdicateAt = 0;
	int32 Wander = 0;
	// B3 (header section 5): -Diplomacy=1 the peace policy (CosmicRulerPolicy::ActDiplomacy - friendship, treaties, the guard),
	// -Diplomacy=2 the attack policy (measurement only); 0 (the default) none.
	int32 Diplomacy = 0;
	FString RulesText;
	FString Out = TEXT("balance.csv");
	FParse::Value(*Params, TEXT("Seed="), Seed);
	FParse::Value(*Params, TEXT("Ticks="), Ticks);
	FParse::Value(*Params, TEXT("Systems="), Systems);
	FParse::Value(*Params, TEXT("Civs="), Civs);
	FParse::Value(*Params, TEXT("TellAll="), TellAll);
	FParse::Value(*Params, TEXT("Answer="), Answer);
	FParse::Value(*Params, TEXT("Build="), Build);
	FParse::Value(*Params, TEXT("Focus="), Focus);
	FParse::Value(*Params, TEXT("Edict="), Edict);
	FParse::Value(*Params, TEXT("EdictRow="), EdictRowText);
	FParse::Value(*Params, TEXT("Abdicate="), AbdicateAt);
	FParse::Value(*Params, TEXT("Wander="), Wander);
	FParse::Value(*Params, TEXT("Diplomacy="), Diplomacy);
	// B4 (header section 2.9 - measurement only): -PlayerPolicy=Cautious|Reckless|Faithful the pressure cards' answers (none:
	// the round of every card), -Guard=1 the guard, -LeaveNearSphere=1 (under a pressure policy) the near-sphere crises left
	// unanswered.
	FString PlayerPolicyText;
	int32 GuardArg = 0;
	int32 LeaveNearSphere = 0;
	FParse::Value(*Params, TEXT("PlayerPolicy="), PlayerPolicyText);
	FParse::Value(*Params, TEXT("Guard="), GuardArg);
	FParse::Value(*Params, TEXT("LeaveNearSphere="), LeaveNearSphere);
	// B4 ([B-4d2] - measurement only): -WithoutPressure=1 runs on the tables without B4's pressure data
	// (CosmicTests::WithoutPressure - the two pressure cards, the vigil's draw row, the seventh edict, the claims'
	// first-person lines): the data-off side of a pair in the same build (the stage times, the worlds).
	int32 WithoutPressure = 0;
	FParse::Value(*Params, TEXT("WithoutPressure="), WithoutPressure);
	FParse::Value(*Params, TEXT("Rules="), RulesText, false);
	FParse::Value(*Params, TEXT("Out="), Out);
	// B5 ([B-5a] - header section 5.1, notes section 2): -SaveAt=<tick> writes the run's slot at that tick's boundary
	// (UCosmicSaveSubsystem::SaveCurrentGalaxy - the game's saving gate is not asked), for a hand check opened with Continue.
	// Only on a run of the game's shape: -TellAll=0 and no argument that changes tables or rules.
	int64 SaveAt = -1;
	FParse::Value(*Params, TEXT("SaveAt="), SaveAt);
	// B5 ([B-5b] - header section 5.1, measurement only): -DeathAt=<tick> records a KillPlayer with no subject at that tick's
	// boundary (cause Death.Age - no game code writes it while ageing is off; in a build with the automation tests only);
	// -WithoutDeath=1 runs on the tables with no KillPlayer (CosmicTests::WithoutDeath - the data-off side of a pair);
	// -PastEnd=1 lets both policies go on after the record of the run's end.
	int64 DeathAt = -1;
	int32 WithoutDeath = 0;
	int32 PastEnd = 0;
	FParse::Value(*Params, TEXT("DeathAt="), DeathAt);
	FParse::Value(*Params, TEXT("WithoutDeath="), WithoutDeath);
	FParse::Value(*Params, TEXT("PastEnd="), PastEnd);
	// B6a ([B-6a6] - header section 5.1, measurement only): -ExaltAsBefore=1 runs on the tables with petition_exalt's
	// condition back to the old PlayerTierAtLeast 2 (CosmicTests::WithExaltAsBefore - the data-off side of a pair in the same
	// build); in a build with the automation tests only, not an argument of the hash run or the runner.
	int32 ExaltAsBefore = 0;
	FParse::Value(*Params, TEXT("ExaltAsBefore="), ExaltAsBefore);
	// The home galaxy row too (-Systems and -Civs 0): the game never builds another size.
	const bool bSaveShape = TellAll == 0 && RulesText.IsEmpty() && WithoutPressure == 0 && Systems == 0 && Civs == 0 && DeathAt < 0 && WithoutDeath == 0
		&& PastEnd == 0 && ExaltAsBefore == 0;
	if (SaveAt >= 0 && !bSaveShape)
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -SaveAt writes only a run of the game's shape (-TellAll=0, no -Rules, -WithoutPressure, -Systems, -Civs, -DeathAt, -WithoutDeath, -PastEnd or -ExaltAsBefore): no save"));
	}
	// A boundary the run never reaches writes nothing: said at once, not found missing after the run.
	if (SaveAt >= 0 && (SaveAt < 1 || SaveAt > Ticks))
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -SaveAt is a tick boundary from 1 to -Ticks (%lld): %lld - no save"), Ticks, SaveAt);
	}
	bool bSavedAt = false;
	const FString OutPath = FPaths::IsRelative(Out) ? FPaths::ProjectSavedDir() / TEXT("Balance") / Out : Out;
	const FString RowsPath = FPaths::GetPath(OutPath) / (FPaths::GetBaseFilename(OutPath) + TEXT("_rows.csv"));

	// The commandlet runs under the editor engine without a GameInstance, and the subsystems live on one (section 9.3).
	UClass* GameInstanceClass = GetDefault<UGameMapsSettings>()->GameInstanceClass.TryLoadClass<UGameInstance>();
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine, GameInstanceClass ? GameInstanceClass : UGameInstance::StaticClass());
	GameInstance->AddToRoot();
	GameInstance->InitializeStandalone();

	int32 Result = 0;
	UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>();
	UPlayerRoleSubsystem* Role = GameInstance->GetSubsystem<UPlayerRoleSubsystem>();
	// A run on rules it could not set would print numbers of other rules: it does not run.
	const bool bRules = Sim && (RulesText.IsEmpty() || OverrideRules(*Sim, RulesText));
	// The tables the run plays on first, then the checks that read them (a kept row the data-off tables do not hold is refused).
	bool bUnpressed = WithoutPressure == 0;
#if WITH_DEV_AUTOMATION_TESTS
	if (Sim && WithoutPressure == 1)
	{
		Sim->SetTables(CosmicTests::WithoutPressure(Sim->GetTables()));
		UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: tables without B4's pressure data"));
		bUnpressed = true;
	}
#endif
	if (!bUnpressed)
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -WithoutPressure is 0 or 1 (1 in a build with the automation tests): %d"), WithoutPressure);
	}
	// B5 ([B-5b]): the death's measuring arguments are a test build's only, like -WithoutPressure.
	bool bDeathArgs = WithoutDeath == 0 && DeathAt < 0 && PastEnd == 0;
#if WITH_DEV_AUTOMATION_TESTS
	bDeathArgs = (WithoutDeath == 0 || WithoutDeath == 1) && (PastEnd == 0 || PastEnd == 1);
	if (Sim && WithoutDeath == 1)
	{
		Sim->SetTables(CosmicTests::WithoutDeath(Sim->GetTables()));
		UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: tables without KillPlayer"));
	}
#endif
	if (!bDeathArgs)
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -WithoutDeath and -PastEnd are 0 or 1, and -DeathAt, -WithoutDeath=1 and -PastEnd=1 need a build with the automation tests: %lld %d %d"), DeathAt, WithoutDeath, PastEnd);
	}
	// B6a ([B-6a6]): -ExaltAsBefore is a test build's only too, the gate of -WithoutDeath (after it: the two compose).
	bool bExaltArg = ExaltAsBefore == 0;
#if WITH_DEV_AUTOMATION_TESTS
	bExaltArg = ExaltAsBefore == 0 || ExaltAsBefore == 1;
	if (Sim && ExaltAsBefore == 1)
	{
		Sim->SetTables(CosmicTests::WithExaltAsBefore(Sim->GetTables()));
		UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: tables with petition_exalt as before"));
	}
#endif
	if (!bExaltArg)
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -ExaltAsBefore is 0 or 1 (1 in a build with the automation tests): %d"), ExaltAsBefore);
	}
	// A kept row the tables do not hold would measure the policy without it under its name: no run (B2).
	const bool bEdictRow = Sim && (EdictRowText.IsEmpty() || Sim->GetTables().FindEdict(FName(*EdictRowText)) != nullptr);
	if (Sim && !bEdictRow)
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -EdictRow names no DT_Edicts row: %s"), *EdictRowText);
	}
	const bool bDiplomacy = Diplomacy >= 0 && Diplomacy <= 2;
	if (!bDiplomacy)
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -Diplomacy is 0, 1 or 2: %d"), Diplomacy);
	}
	ECosmicPressurePolicy PressurePolicy = ECosmicPressurePolicy::Round;
	// -LeaveNearSphere=1 is read under a pressure policy only: a run that could not apply it would measure the policy without
	// it under its name - no run (as -EdictRow).
	const bool bPressure = (PlayerPolicyText.IsEmpty() || CosmicRulerPolicy::PressurePolicyOf(PlayerPolicyText, PressurePolicy)) && (GuardArg == 0 || GuardArg == 1)
		&& (LeaveNearSphere == 0 || LeaveNearSphere == 1) && (LeaveNearSphere == 0 || PressurePolicy != ECosmicPressurePolicy::Round);
	if (!bPressure)
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -PlayerPolicy is Cautious, Reckless or Faithful (B4 has no tribute), -Guard and -LeaveNearSphere 0 or 1, and -LeaveNearSphere=1 needs one of the three policies: %s %d %d"),
			*PlayerPolicyText, GuardArg, LeaveNearSphere);
	}
	if (Sim && Role && bRules && bEdictRow && bDiplomacy && bPressure && bUnpressed && bDeathArgs && bExaltArg)
	{
		// Every line is stored unless -TellAll=0 asks for the scope of telling of a game (section 8.1). The world hash is
		// the same either way.
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->SetTellAll(TellAll != 0);
		}
		Sim->NewGalaxy(Seed, Systems, Civs);
		Sim->GetStageClock() = FCosmicStageClock();
		Sim->GetStageClock().bOn = true;
		const FGalaxyState& State = Sim->GetState();
		const FCosmicStateIndex& StateIndex = Sim->GetStateIndex();
		const FCosmicRulesDef& RulesNow = Sim->GetRules();
		static const FGameplayTag ConflictTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Conflict")));
		static const FGameplayTag SignTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Sign")));
		// B6a [B-6a3c]: the awareness lines and the feat lines of the ending line.
		static const FGameplayTag AwarenessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Transcendent")));
		static const FGameplayTag AscensionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Ascension")));
		UCosmicSaveGame* Save = NewObject<UCosmicSaveGame>();
		Save->AddToRoot();

		FRunCounts Counts;
		FSpiritCounts Spirit;
		FPressureCounts PressureCounts;
		FEndingCounts Ending;
		// B6a [B-6a3c]: the season's start - the first boundary at which a people the vessel rules stands at the third rung or
		// above (read after the tick and again after the inputs).
		const auto NoteSeason = [&State, &Ending]()
		{
			if (Ending.SeasonStart >= 0)
			{
				return;
			}
			if (const FCivilizationState* Ruled = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled && Civ.Tier >= EAscensionTier::Tier3_Causal; }))
			{
				Ending.SeasonStart = State.Tick;
				Ending.SeasonCivID = Ruled->CivID;
			}
		};
		const FName VigilRow = CosmicRulerPolicy::VigilRowOf(Sim->GetTables());
		const int32 SphereStart = SphereSize(State);
		const TArray<FString> SpheresStart = SpheresOf(State);
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Counts.ToldLines += Entry.bUntold ? 0 : 1;
		}
		FString Csv = TEXT("Tick,CivID,Archetype,Tier,Systems,Stability,Energy,Matter,Insight,Influence,Causality,Hazards,KnownSystems,MetCivs,WildSystems,FirstContactTick,T4PlusCount,FirstClaimTick,ToldLines,TickMicros,SaveBytes,LogLen,ChronicleLen,SaveRawBytes,ChronicleLetGo,Expansion,FleetPower,Hostile,Treaties,BorderPeoples,DrainPressure,FallenSphereTicks\n");
		FString Rows = TEXT("Tick,SourceRow,Lines\n");
		double Unused = 0.0;
		TArray<double> TickMicros;
		TArray<FSlotBefore> Before;
		// The tick-0 sample: no tick has run yet, so a settlement under way reads as Settling.
		{
			int64 RawBytes = 0;
			const int32 SlotBytes = SaveBytesOf(Save, State, RawBytes, Unused);
			AppendSample(Csv, *Sim, Before, Counts, 0.0, FString::FromInt(SlotBytes), FString::Printf(TEXT("%lld"), RawBytes), nullptr);
		}
		Counts.ExpansionPairs += PeoplesNotRuled(State);
		TMap<FString, int32> SampleRows;
		double SampleMicros = 0.0;
		int64 SampleTicks = 0;
		int64 Samples = 0;
		const FCosmicRulerPolicy RulerPolicy{ Answer != 0, Build != 0, Focus != 0, Edict != 0, EdictRowText.IsEmpty() ? NAME_None : FName(*EdictRowText), Diplomacy,
			PressurePolicy, GuardArg, LeaveNearSphere != 0, PastEnd != 0 };
		FCosmicRulerPolicyCounts RulerCounts;
		const FCosmicWandererPolicy WandererPolicy{ AbdicateAt, Wander != 0, PastEnd != 0 };
		FCosmicWandererPolicyCounts WandererCounts;
		// B5 ([B-5a]): the calling line's counts, and the rules of the ruler's window B' (every realm cap raised - #133).
		FCallingCounts Calling;
		FCosmicRulesDef RulesRaised = RulesNow;
		for (int32& Cap : RulesRaised.RealmCapByTier)
		{
			Cap = 1000000;
		}
		// The people of the first reign: the ruler line's rung ticks are that people's (B1 - a later throne is the wanderer
		// line's).
		const FName FirstRuled = (!State.Calling.Lives.IsEmpty() && !State.Calling.Lives[0].RuledCivIDs.IsEmpty()) ? State.Calling.Lives[0].RuledCivIDs[0] : NAME_None;
		// B6a [B-6a3b] (header section 5.1): the end check - the record the state must hold at every boundary (empty until the
		// ending predicate first holds, then the predicate's answer at that boundary for the rest of the run), the boundaries
		// at which it did not, and the first of them.
		FCosmicEndingRecord EndCheckExpected;
		int64 EndCheckMisses = 0;
		int64 EndCheckFirstMiss = -1;
		for (int64 Done = 0; Done < Ticks; ++Done)
		{
			Before.Reset();
			for (const FStarSystemState& System : State.Systems)
			{
				const FCivilizationState* Owner = State.Civs.FindByPredicate([&System](const FCivilizationState& Civ) { return Civ.CivID == System.OwnerCivID; });
				FSlotBefore& Slot = Before.Add_GetRef({ System.ActiveHazard, System.HazardTicksRemaining, System.ResponseMegaproject, Owner ? Owner->Stockpile.Influence : 0.f,
					CosmicMegaprojects::ReadSlot(System, CosmicMegaprojects::ECosmicProjectSlot::Construction) });
				const FHazardDef* Hazard = (Owner && System.ActiveHazard.IsValid()) ? Sim->GetTables().FindHazardByTag(System.ActiveHazard) : nullptr;
				Slot.HeldAnswer = Hazard ? CosmicEdicts::HeldAnswerOf(*Owner, *Hazard, Sim->GetTables()) : NAME_None;
				if (const FEdictDef* Held = Slot.HeldAnswer.IsNone() ? nullptr : Sim->GetTables().FindEdict(Slot.HeldAnswer))
				{
					const FResourcePool Upkeep = CosmicEdicts::CostOf(Held->UpkeepPerTick);
					const FResourcePool& Have = Owner->Stockpile;
					const FResourcePool& Gain = Owner->IncomePerTick;
					Slot.bUpkeepPayable = Have.Energy + Gain.Energy >= Upkeep.Energy && Have.Matter + Gain.Matter >= Upkeep.Matter && Have.Insight + Gain.Insight >= Upkeep.Insight
						&& Have.Influence + Gain.Influence >= Upkeep.Influence && Have.Causality + Gain.Causality >= Upkeep.Causality;
				}
				Slot.bRuledSettled = Owner && Owner->bIsPlayerRuled && CosmicState::IsSettled(System);
				if (Slot.bRuledSettled && System.ActiveHazard.IsValid())
				{
					const int32 SystemIndex = Before.Num() - 1;
					Slot.bClaimReach = CosmicHazards::ClaimantOf(State, StateIndex, Sim->GetTables(), RulesNow, SystemIndex) != INDEX_NONE;
					Slot.bLastUnclaimed = CosmicHazards::IsLastUnclaimed(State, StateIndex, Sim->GetTables(), SystemIndex);
				}
			}
			// The AI peoples' edicts before the tick, by people (B2 - what the AI stage gave up), and the ruled people's: the
			// edicts this tick's economy stage applies (a lapse in the tick still applied them), counted as held ticks.
			TMap<FName, TArray<FName>> AIEdictsBefore;
			for (const FCivilizationState& Civ : State.Civs)
			{
				if (!Civ.bIsPlayerRuled)
				{
					AIEdictsBefore.Add(Civ.CivID, Civ.ActiveEdicts);
				}
				else
				{
					for (const FName Row : Civ.ActiveEdicts)
					{
						++Counts.EdictHeldTicks.FindOrAdd(Row);
					}
				}
			}
			// B3: every people before the tick, and the fleet the attrition will take in it (the stage's own estimate - the
			// pressure inside a higher sphere on the target, at most the fleet).
			TMap<FName, FCivBefore> CivsBefore;
			for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
			{
				const FCivilizationState& Civ = State.Civs[CivIndex];
				CivsBefore.Add(Civ.CivID, CivBeforeOf(State, StateIndex, Civ));
				const int32 Target = Civ.FleetTargetSystemID.IsNone() ? INDEX_NONE : StateIndex.FindSystem(State, Civ.FleetTargetSystemID);
				if (State.Systems.IsValidIndex(Target) && Civ.FleetPower > 0.f)
				{
					const float Pressure = UGalaxySimulationSubsystem::ComputeAttritionPressureIn(State, StateIndex, Sim->GetTables(), RulesNow, CivIndex, Target).P;
					if (Pressure > 0.f)
					{
						const double Taken = FMath::Min(static_cast<double>(Civ.FleetPower), static_cast<double>(RulesNow.PressureFleetAttrition) * Pressure);
						Counts.AttritionLost += Taken;
						// B6a [B-6a3c] (#132): the ruled people's, by the phase of the tick about to run (TickPhase below -
						// bEndedBefore is declared after this loop; neither the record nor the season moves before then).
						if (Civ.bIsPlayerRuled)
						{
							Ending.RuledAttrition[Counts.FirstEndTick >= 0 ? 2 : (Ending.SeasonStart >= 0 ? 1 : 0)] += Taken;
						}
					}
				}
			}
			const EPlayerRole RoleBefore = State.Player.Role;
			const int64 TickBefore = State.Tick;
			// B6a [B-6a3c]: the people the vessel rules before the tick (its climb in the tick is not an AI people's).
			const FCivilizationState* RuledBeforeTick = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
			const FName RuledCivBeforeTick = RuledBeforeTick ? RuledBeforeTick->CivID : NAME_None;
			const int32 SphereBefore = SphereSize(State);
			const FName CardBefore = State.Knowing.PendingPetitionRow;
			// B6a [B-6a3c] (the petition_exalt pre-count): the tick about to run begins a reign - its knowing stage compares the
			// Standing petitions with an empty head (read before the tick, as the tick reads it).
			const bool bReignHeadEmpty = CosmicPetitions::IsFirstTickOfReign(State);

			const bool bEndedBefore = Counts.FirstEndTick >= 0;
			// B4: the ruled people's drain P of the tick about to run and how near a fallen sphere its settled systems lie;
			// the spheres before the tick (the joins).
			{
				const int64 TickRun = State.Tick + 1;
				const int32 RuledIndex = State.Civs.IndexOfByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
				if (State.Civs.IsValidIndex(RuledIndex))
				{
					const float Drain = UGalaxySimulationSubsystem::ComputeDrainPressureOf(State, StateIndex, Sim->GetTables(), RulesNow, RuledIndex);
					float& Max = Spirit.DrainMax[TickRun <= 1000 ? 0 : (TickRun <= 2000 ? 1 : (TickRun <= 3000 ? 2 : (TickRun <= 6000 ? 3 : 4)))];
					Max = FMath::Max(Max, Drain);
					if (Drain > 0.f)
					{
						const double Taken = UGalaxySimulationSubsystem::DrainLossOf(RulesNow, Sim->GetTables(), State.Civs[RuledIndex], Drain);
						++Spirit.DrainedTicks;
						Spirit.Drained += Taken;
						Spirit.DrainedTicksByEnd += bEndedBefore ? 0 : 1;
						Spirit.DrainedByEnd += bEndedBefore ? 0.0 : Taken;
					}
					if (Spirit.FirstInside < 0)
					{
						const FCivilizationState& Ruled = State.Civs[RuledIndex];
						for (const FName SystemID : Ruled.OwnedSystems)
						{
							const int32 SystemIndex = StateIndex.FindSystem(State, SystemID);
							if (!State.Systems.IsValidIndex(SystemIndex) || State.Systems[SystemIndex].OwnerCivID != Ruled.CivID || !CosmicState::IsSettled(State.Systems[SystemIndex]))
							{
								continue;
							}
							const ECosmicSphereReach Reach = FallenReachOf(State, Sim->GetTables(), SystemIndex);
							Spirit.FirstNextTo = (Spirit.FirstNextTo < 0 && Reach != ECosmicSphereReach::None) ? TickRun : Spirit.FirstNextTo;
							Spirit.FirstInside = (Spirit.FirstInside < 0 && Reach == ECosmicSphereReach::Inside) ? TickRun : Spirit.FirstInside;
						}
					}
				}
			}
			TArray<TArray<FName>> SpheresBefore;
			for (const FTranscendentState& Transcendent : State.Transcendents)
			{
				SpheresBefore.Add(Transcendent.InfluencedSystems);
			}
			// B4: the CSV's DrainPressure of the tick about to run, when its end is a sample.
			TMap<FName, float> DrainsBefore;
			if ((Done + 1) % SampleEveryTicks == 0 || Done + 1 == Ticks)
			{
				for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
				{
					DrainsBefore.Add(State.Civs[CivIndex].CivID, UGalaxySimulationSubsystem::ComputeDrainPressureOf(State, StateIndex, Sim->GetTables(), RulesNow, CivIndex));
				}
			}
			Sim->GetStageClock().SiegeEnds.Reset();
			Sim->GetStageClock().Expiries.Reset();
			// [B-5b]: the lives, the leap total and the vessel before the tick (a death and a succession inside it).
			const int32 LivesBefore = State.Calling.Lives.Num();
			const int64 LeapTotalBefore = State.Calling.LeapTicksTotal;
			const FPlayerEntityState PlayerAtStart = State.Player;
			// [B-5r]: the highest reverence of any people before the tick (a death in it clears them all at its end).
			float ReverenceAtStart = 0.f;
			for (const FCivilizationState& Civ : State.Civs)
			{
				ReverenceAtStart = FMath::Max(ReverenceAtStart, Civ.ReverenceToPlayer);
			}
			const bool bLeapTick = State.Calling.LifeState == ELifeState::Interregnum;
			const uint64 Start = FPlatformTime::Cycles64();
			Sim->RunHeadless(1);
			const double Micros = static_cast<double>(FPlatformTime::Cycles64() - Start) * FPlatformTime::GetSecondsPerCycle64() * 1.0e6;
			TickMicros.Add(Micros);
			Calling.LeapTicksRun += bLeapTick ? 1 : 0;
			Calling.LeapMicros += bLeapTick ? Micros : 0.0;
			SampleMicros += Micros;
			++SampleTicks;

			// The run's end, read as the presentation reads it at this tick boundary (before the ruler's input of it): the record
			// (B6a [B-6a3c] - written once, after the tenth stage of the tick whose end it is, so it first stands at that tick's
			// boundary).
			{
				const int32 EndTick = Sim->GetRules().ProvisionalEndTick;
				Counts.ClockTick = (Counts.ClockTick < 0 && EndTick > 0 && State.Tick >= EndTick) ? State.Tick : Counts.ClockTick;
				if (!Counts.FirstEnd.IsValid() && State.Ending.Kind.IsValid())
				{
					Counts.FirstEnd = State.Ending.Kind;
					Counts.FirstEndTick = State.Ending.Tick;
					Counts.DecisionsAtEnd = CosmicRulerPolicy::DecisionsOf(RulerCounts);
					const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
					if (!Chapters.IsEmpty() && State.Calling.Lives.IsValidIndex(Chapters.Last().LifeAt))
					{
						Counts.ChapterLife = State.Calling.Lives[Chapters.Last().LifeAt].LifeIndex;
						Counts.ChapterShown = Chapters.Last().Lines.Num();
						Counts.ChapterCandidates = Chapters.Last().Candidates;
						Counts.ChapterMore = Chapters.Last().bMore ? 1 : 0;
						static const FGameplayTag CallingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Calling")));
						const TArray<FCosmicChronicleChapter> Whole = CosmicChronicle::BuildCapped(State, TNumericLimits<int32>::Max());
						FMemory::Memzero(Counts.ChapterRanks);
						for (const FCosmicChronicleLine& Line : Whole.Last().Lines)
						{
							const int32 Rank = Line.Entry.Category == CallingTag ? 0 : !Line.bOwn ? 3 : Line.Entry.Severity >= 2 ? 1 : 2;
							++Counts.ChapterRanks[Rank];
						}
						// B3: the ruled people's Severity-2 conflict lines among the chapter's shown lines.
						Counts.ConflictSev2InChapter = 0;
						for (const FCosmicChronicleLine& Line : Chapters.Last().Lines)
						{
							Counts.ConflictSev2InChapter += (Line.Entry.Category == ConflictTag && Line.Entry.Severity >= 2 && Line.bOwn) ? 1 : 0;
						}
					}
				}
			}

			// B6a [B-6a3b] (header section 5.1): the end check, at this boundary before the ruler's input. The ending predicate is
			// asked until it first holds; from then on its answer at that boundary is the record. The state's record must be
			// empty before it, the same four fields at it and unchanged after it (written once, after the tenth stage of the tick
			// the predicate first held) - an invariant with no fair exception, so a miss fails the run. Nothing is printed while
			// it holds (every batch's log has no end check line).
			{
				if (!EndCheckExpected.Kind.IsValid())
				{
					const FCosmicEndingRecord Predicate = CosmicEndings::EndingOf(State, Sim->GetRules());
					EndCheckExpected = Predicate.Kind.IsValid() ? Predicate : EndCheckExpected;
				}
				if (!EndCheckSame(State.Ending, EndCheckExpected))
				{
					if (EndCheckMisses == 0)
					{
						const TCHAR* Where = !EndCheckExpected.Kind.IsValid() ? TEXT("before the predicate holds")
							: (EndCheckExpected.Tick == State.Tick ? TEXT("where the predicate first holds") : TEXT("after the predicate first held"));
						UE_LOG(LogCosmic, Error, TEXT("CosmicBalance end check: at %lld (%s) the record is %s, the predicate's is %s"), State.Tick, Where,
							*EndCheckText(State.Ending), *EndCheckText(EndCheckExpected));
						EndCheckFirstMiss = State.Tick;
					}
					++EndCheckMisses;
					Result = 1;
				}
			}

			// B6a [B-6a3c] (header section 5.1, notes section 2.1): the ending line's readings of the tick, at this boundary before
			// the inputs. TickPhase is the phase of what happened in the tick (a season that starts at this boundary starts after
			// it - the rung was reached at the end of the AI stage); BoundaryPhase that of what this boundary reads.
			const int32 TickPhase = bEndedBefore ? 2 : (Ending.SeasonStart >= 0 ? 1 : 0);
			NoteSeason();
			const int32 BoundaryPhase = bEndedBefore ? 2 : (Ending.SeasonStart >= 0 ? 1 : 0);
			{
				// The exact first climb of an AI people to the third rung (the AI stage's feat judgment), and its feat row.
				for (int32 CivIndex = 0; Ending.FirstAIT3Tick < 0 && CivIndex < State.Civs.Num(); ++CivIndex)
				{
					const FCivilizationState& Civ = State.Civs[CivIndex];
					const FCivBefore* Was = CivsBefore.Find(Civ.CivID);
					if (!Was || Civ.CivID == RuledCivBeforeTick || Was->Tier >= EAscensionTier::Tier3_Causal || Civ.Tier < EAscensionTier::Tier3_Causal)
					{
						continue;
					}
					Ending.FirstAIT3Tick = State.Tick;
					Ending.FirstAIT3CivID = Civ.CivID;
					for (int32 Index = State.Log.Num() - 1; Index >= 0 && State.Log[Index].Tick > TickBefore; --Index)
					{
						if (State.Log[Index].Category == AscensionTag && State.Log[Index].FocusCivID == Civ.CivID)
						{
							Ending.FirstAIT3Feat = State.Log[Index].SourceRow;
							break;
						}
					}
				}
				// Every people's research fields at 1.0.
				for (const FCivilizationState& Civ : State.Civs)
				{
					for (const TPair<FName, float>& Field : Civ.Research)
					{
						if (Field.Value >= 1.f)
						{
							TMap<FName, int64>& Full = Ending.ResearchFull.FindOrAdd(Civ.CivID);
							if (!Full.Contains(Field.Key))
							{
								Full.Add(Field.Key, State.Tick);
							}
						}
					}
				}
				// The first awareness, and whether the vessel had left by this boundary.
				for (int32 At = 0; Ending.AwareTick < 0 && At < State.Transcendents.Num(); ++At)
				{
					if (State.Transcendents[At].bAwareOfPlayer)
					{
						Ending.AwareTick = State.Tick;
						Ending.AwareTranscendentID = State.Transcendents[At].TranscendentID;
						Ending.bAwareVesselLeft = State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler
							|| State.Calling.Lives.Num() != LivesBefore;
					}
				}
				// The first stretch of low Stability of the ruled people, by the record.
				if (Ending.LowStabilityTick < 0 && !bEndedBefore)
				{
					const FCivilizationState* Ruled = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
					Ending.LowStabilityTick = (Ruled && Ruled->LowStabilityTicks > 0) ? State.Tick : Ending.LowStabilityTick;
				}
			}

			// B5 ([B-5a]): the death chances of the tick - the crisis stage's notes of its expiries, the gap from the last reign
			// end before this tick; then a reign that ended in the tick (the tenth stage leaves the throne).
			for (const FCosmicStageClock::FExpiry& Expiry : Sim->GetStageClock().Expiries)
			{
				if (Expiry.bReached)
				{
					Calling.Chances.Add({ Expiry.Tag, State.Tick, Expiry.SystemID, Expiry.bGrace, Expiry.bBranch, bEndedBefore,
						Calling.ReignEndTick >= 0 ? State.Tick - Calling.ReignEndTick : -1 });
				}
			}
			// [B-5b]: a life that closed in the tick (the last closed life's DiedTick - a leap of 0 began the next already).
			{
				const TArray<FLifeRecord>& Lives = State.Calling.Lives;
				const int32 Closed = State.Calling.LifeState == ELifeState::Interregnum ? Lives.Num() - 1 : Lives.Num() - 2;
				// The boundary's own rule (the controller stops the time on it).
				if (ACosmicPlayerController::LifeClosedAt(State, State.Tick))
				{
					const FLifeRecord& Dead = Lives[Closed];
					++Calling.Deaths;
					Calling.DeathsByEnd += bEndedBefore ? 0 : 1;
					++Calling.DeathsByCause.FindOrAdd(Dead.CauseOfDeath.ToString());
					++Calling.DeathsByRow.FindOrAdd(Dead.CauseSourceRow.ToString());
					if (Calling.FirstDeathTick < 0)
					{
						Calling.FirstDeathTick = State.Tick;
						Calling.FirstDeathRole = RoleBefore == EPlayerRole::Ruler ? TEXT("ruler") : TEXT("wanderer");
						Calling.FirstDeathGap = (RoleBefore == EPlayerRole::Wanderer && Calling.ReignEndTick >= 0) ? State.Tick - Calling.ReignEndTick : -1;
					}
					// The system it stood in: a vessel whose death is recorded does not move in that tick, so it is the place before
					// the tick (the interregnum keeps it; after a leap of 0 the new vessel stands elsewhere).
					const FName DeathPlace = PlayerAtStart.CurrentSystemID;
					Calling.DeathSystemIndex = StateIndex.FindSystem(State, DeathPlace);
					const int64 Leap = State.Calling.LeapTicksTotal - LeapTotalBefore;
					// At the cap: the leap took what the cap left, less than the longest (the draw itself is not seen - it may have been
					// as short).
					const int64 CapLeft = FMath::Max<int64>(0, RulesNow.LeapTotalCap - LeapTotalBefore);
					const bool bCut = Leap == CapLeft && CapLeft < FMath::Max(RulesNow.LeapTicksMin, RulesNow.LeapTicksMax);
					const bool bPastClock = RulesNow.ProvisionalEndTick > 0 && State.Tick + Leap >= RulesNow.ProvisionalEndTick;
					Calling.Leaps += FString::Printf(TEXT("%s%lld@%lld%s%s"), Calling.Leaps.IsEmpty() ? TEXT("") : TEXT(","), Leap, State.Tick, bCut ? TEXT(":cut") : TEXT(""),
						bPastClock ? TEXT(":clock") : TEXT(""));
					// [B-5r] (review method-2): the epitaph's last answers by the presentation's own pure function, at this boundary
					// (reads only); and whether the newest answer is the killing hazard's card at all (the game's IsCardOfHazard).
					const TArray<FCosmicAnswerTitle> Titles = CosmicCallingPages::LastAnswersOf(State, Sim->GetTables(), Closed);
					const bool bSetApart = Titles.ContainsByPredicate([](const FCosmicAnswerTitle& Title) { return Title.bOfTheDeath; });
					const FHazardDef* Killer = Dead.CauseOfDeath.MatchesTagExact(CosmicTags::DeathHazard()) ? Sim->GetTables().FindHazard(Dead.CauseSourceRow) : nullptr;
					const FEncounterDef* Newest = Dead.LastChoiceRows.IsEmpty() ? nullptr : Sim->GetTables().FindEncounter(Dead.LastChoiceRows.Last());
					const bool bOnTheCard = Killer && Newest && CosmicCallingPages::IsCardOfHazard(*Newest, *Killer);
					Calling.DeathsSetApart += bSetApart ? 1 : 0;
					Calling.DeathsOnTheCard += (bOnTheCard && !bSetApart) ? 1 : 0;
					Calling.DeathsAtCrowningReverence += ReverenceAtStart >= RulesNow.CoronationReverence ? 1 : 0;
					Calling.DeathAnswers += FString::Printf(TEXT("%s%lld:%s:%.2f"), Calling.DeathAnswers.IsEmpty() ? TEXT("") : TEXT(","), State.Tick,
						bSetApart ? TEXT("a") : (bOnTheCard ? TEXT("c") : TEXT("-")), ReverenceAtStart);
				}
				// A life that began in the tick after the first: a succession.
				if (Lives.Num() > LivesBefore && LivesBefore > 0)
				{
					const FLifeRecord& Born = Lives.Last();
					const int32 Birth = StateIndex.FindSystem(State, Born.BirthSystemID);
					const CosmicSuccession::FCosmicSuccessionCandidates Found = CosmicSuccession::CandidatesOf(State, StateIndex, Sim->GetTables(), RulesNow, Calling.DeathSystemIndex);
					const TArray<int32> FromDeath = CosmicState::LaneDistancesFrom(State, StateIndex, Calling.DeathSystemIndex);
					const TArray<int32> NearBirth = CosmicState::LaneDistancesFrom(State, StateIndex, Birth, 2);
					bool bNear = false;
					for (int32 SystemIndex = 0; SystemIndex < NearBirth.Num(); ++SystemIndex)
					{
						bNear |= NearBirth[SystemIndex] != INDEX_NONE && State.Systems[SystemIndex].ActiveHazard.IsValid();
					}
					const FLifeRecord& PreviousLife = Lives[Lives.Num() - 2];
					const bool bSamePeople = Born.BirthCivID == PreviousLife.BirthCivID || (!PreviousLife.RuledCivIDs.IsEmpty() && Born.BirthCivID == PreviousLife.RuledCivIDs.Last());
					const FCivilizationState* Owner = State.Civs.FindByPredicate([&Born](const FCivilizationState& Civ) { return Civ.CivID == Born.BirthCivID; });
					// [B-5d1] (header section 5.1, 5.2 bundle 1): the digest page's lines of this interregnum, by the presentation's own
					// pure function (what the pages show at this boundary; reads only).
					const int32 DigestLines = CosmicDigest::LinesOf(*Sim, CosmicDigest::LastClosedLifeOf(State), false).Num();
					Calling.Successions += FString::Printf(TEXT("%s%lld:%d:%d:%d:%d:%d:T%d:D%d"), Calling.Successions.IsEmpty() ? TEXT("") : TEXT(","), State.Tick, Found.Systems.Num(),
						Found.Relaxed, FromDeath.IsValidIndex(Birth) ? FromDeath[Birth] : -1, bSamePeople ? 1 : 0, bNear ? 1 : 0, Owner ? static_cast<int32>(Owner->Tier) + 1 : 0, DigestLines);
					Calling.FirstSuccessionTick = Calling.FirstSuccessionTick < 0 ? State.Tick : Calling.FirstSuccessionTick;
					// [B-5r] (review play-5): this life's first throne is filled in at the input that takes it.
					Calling.SuccessorThroneGaps.Add(-1);
					Calling.SuccessorLifeAt = Lives.Num() - 1;
				}
			}
			if (RoleBefore == EPlayerRole::Ruler && State.Player.Role == EPlayerRole::Wanderer)
			{
				Calling.ReignEndTick = State.Tick;
				Calling.FirstReignEndTick = Calling.FirstReignEndTick < 0 ? State.Tick : Calling.FirstReignEndTick;
			}
			// B5 ([B-5a] - #126): every AI people's aim at this boundary both ways. The ruled people held off only by its capital
			// and its last settled system: the lifted aim turns on it and the plain one does not.
			{
				const int32 RuledHere = State.Civs.IndexOfByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
				bool bBlockedRuled = false;
				for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
				{
					if (State.Civs[CivIndex].bIsPlayerRuled)
					{
						continue;
					}
					const CosmicConflict::FCosmicAIAim Lifted = CosmicConflict::AIAimOf(State, StateIndex, Sim->GetTables(), RulesNow, CivIndex, true);
					if (Lifted.Other == INDEX_NONE)
					{
						continue;
					}
					const CosmicConflict::FCosmicAIAim Plain = CosmicConflict::AIAimOf(State, StateIndex, Sim->GetTables(), RulesNow, CivIndex, false);
					if (Lifted.Other == RuledHere && Plain.Other != RuledHere)
					{
						bBlockedRuled = true;
					}
					else if (Lifted.Other != RuledHere && Lifted.Other != Plain.Other)
					{
						++Calling.BlockedAIPairTicks;
					}
				}
				Calling.BlockedRuledEpisodes += (bBlockedRuled && !Calling.bBlockedRuledLast) ? 1 : 0;
				Calling.bBlockedRuledLast = bBlockedRuled;
				if (bBlockedRuled)
				{
					++Calling.BlockedRuledTicks;
					++(CosmicConflict::SettledCountOf(State, StateIndex, RuledHere) <= 1 ? Calling.BlockedRuledOneSystem : Calling.BlockedRuledLater);
					Calling.BlockedRuledByEnd += bEndedBefore ? 0 : 1;
					Calling.FirstBlockedRuled = Calling.FirstBlockedRuled < 0 ? State.Tick : Calling.FirstBlockedRuled;
				}
			}
			// B5 ([B-5a]): the hand check's save at this boundary (before the inputs, as the game's autosave).
			if (SaveAt >= 0 && State.Tick == SaveAt && bSaveShape)
			{
				UCosmicSaveSubsystem* Saves = GameInstance->GetSubsystem<UCosmicSaveSubsystem>();
				const bool bSaved = Saves && Saves->SaveCurrentGalaxy();
				bSavedAt = bSaved;
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: -SaveAt %lld %s %s"), SaveAt, bSaved ? TEXT("wrote") : TEXT("could not write"),
					Saves ? *FPaths::ConvertRelativePathToFull(Saves->SlotPathOf(State.GalaxyID)) : TEXT("(no save subsystem)"));
			}

			// [B-5b] (-DeathAt): the measured death, recorded at this boundary through Apply (no subject - the ruler is reached
			// too; the birth protection holds); the tenth stage of the next tick carries it out.
			if (DeathAt >= 0 && State.Tick == DeathAt)
			{
				FCosmicEvalContext Ctx = Sim->MakeEvalContext();
				Ctx.CivIndex = INDEX_NONE;
				Ctx.SystemIndex = INDEX_NONE;
				FCosmicEffect Kill;
				Kill.Type = EEffectType::KillPlayer;
				Kill.Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Death.Age")));
				Calling.DeathAtRecorded = UCosmicEventSubsystem::Apply(Kill, Ctx) ? 1 : 0;
			}

			// B3: the first contact card came (read before the policy answers it at this boundary), and the stance toward that
			// people while it waits (the answer's change is the card's).
			if (Counts.FirstContactCardTick < 0 && State.Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact)
			{
				Counts.FirstContactCardTick = State.Tick;
			}
			const bool bContactWaits = State.Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact;
			const FCivilizationState* RuledAtBoundary = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
			const bool bContactFriendlyBefore = RuledAtBoundary && CosmicDiplomacy::StanceOf(*RuledAtBoundary, State.Knowing.FirstContactCivID) == CosmicTags::StanceFriendly();
			// B4 ([B-4d2]): the ruler's Standing cards that came to hold in the tick and were not put.
			{
				const int32 RuledHere = (RuledAtBoundary && State.Player.Role == EPlayerRole::Ruler && State.Calling.LifeState == ELifeState::Embodied)
					? StateIndex.FindCiv(State, RuledAtBoundary->CivID) : INDEX_NONE;
				const FName Pending = State.Knowing.PendingPetitionRow;
				const bool bPutNow = !Pending.IsNone() && Pending != CardBefore;
				for (const FCosmicPetitionUnit& Unit : StandingUnitsOf(State, StateIndex, Sim->GetTables(), RulesNow, RuledHere))
				{
					if (PressureCounts.HeldAfterInput.Contains(Unit))
					{
						continue;
					}
					const FName UnitSystem = State.Systems.IsValidIndex(Unit.SystemIndex) ? State.Systems[Unit.SystemIndex].SystemID : NAME_None;
					const bool bPut = bPutNow && Pending == Unit.Row && (State.Knowing.PendingPetitionSystemID.IsNone() || State.Knowing.PendingPetitionSystemID == UnitSystem);
					if (!bPut)
					{
						++PressureCounts.StandingLost.FindOrAdd(Unit.Row.ToString());
					}
				}
			}
			// B6a [B-6a3c] (notes section 2, the petition_exalt pre-count): its B6a condition came to hold in the tick - what the
			// slot did with it, read before the inputs.
			{
				const bool bExaltHolds = RuledAtBoundary && State.Player.Role == EPlayerRole::Ruler && State.Calling.LifeState == ELifeState::Embodied
					&& RuledAtBoundary->Tier >= EAscensionTier::Tier2_Stellar;
				if (bExaltHolds && (bReignHeadEmpty || !Ending.bExaltHeldAfterInput))
				{
					const FName Placed = State.Knowing.PendingPetitionRow != CardBefore ? State.Knowing.PendingPetitionRow : NAME_None;
					const FEncounterDef* PlacedRow = Placed.IsNone() ? nullptr : Sim->GetTables().FindEncounter(Placed);
					const TCHAR* Kind = !CardBefore.IsNone() ? TEXT("waited")
						: Placed.IsNone() ? TEXT("floor")
						: (PlacedRow && PlacedRow->Trigger == ECosmicEventTrigger::Standing) ? TEXT("contest") : TEXT("other");
					Ending.ExaltPreCount.Add(FString::Printf(TEXT("%lld:%s:%s"), State.Tick, Kind, Placed.IsNone() ? TEXT("-") : *Placed.ToString()));
				}
			}
			// B4: the pressure card's two triggers coming to hold, read before the ruler's input as the card would be, and the
			// vigil's gate when (a) came with the slot free.
			{
				TSet<FName> ClaimNow;
				TSet<FName> PressureNow;
				const bool bRuling = RuledAtBoundary && State.Player.Role == EPlayerRole::Ruler && State.Calling.LifeState == ELifeState::Embodied && State.Tick >= 1800;
				const bool bSlotTaken = !State.Knowing.PendingPetitionRow.IsNone();
				const int32 RuledIndex = RuledAtBoundary ? StateIndex.FindCiv(State, RuledAtBoundary->CivID) : INDEX_NONE;
				for (int32 SystemIndex = 0; bRuling && SystemIndex < State.Systems.Num(); ++SystemIndex)
				{
					const FStarSystemState& System = State.Systems[SystemIndex];
					if (System.OwnerCivID != RuledAtBoundary->CivID || !CosmicState::IsSettled(System))
					{
						continue;
					}
					if (System.ActiveHazard == ClaimTag)
					{
						ClaimNow.Add(System.SystemID);
						if (!Spirit.ClaimHeld.Contains(System.SystemID))
						{
							++Spirit.TriggerClaim;
							++Ending.ClaimTrigger[BoundaryPhase];
							// [B-4d2]: the claim card that came for this claim holds its own slot - a slot taken is another card's
							// (the claim card did not come for it).
							const bool bOwnCard = State.Knowing.PendingPetitionRow == CosmicRows::PetitionPressureClaim && State.Knowing.PendingPetitionSystemID == System.SystemID;
							// B6a [B-6a3c] (#135): the first hold's key - counted once by (seed, tick, system) over the runs that share a world.
							Ending.ClaimTriggerKeys.Add(FString::Printf(TEXT("%lld:%s:%d:%s"), State.Tick, *System.SystemID.ToString(), BoundaryPhase,
								!(bSlotTaken && !bOwnCard) ? TEXT("k") : (State.Knowing.PendingPetitionRow == CardBefore ? TEXT("w") : TEXT("s"))));
							Spirit.TriggerClaimTaken += (bSlotTaken && !bOwnCard) ? 1 : 0;
							if (bSlotTaken && !bOwnCard)
							{
								++(State.Knowing.PendingPetitionRow == CardBefore ? PressureCounts.ClaimLostWait : PressureCounts.ClaimLostSame);
								++(State.Knowing.PendingPetitionRow == CardBefore ? Ending.ClaimLostWait[BoundaryPhase] : Ending.ClaimLostSame[BoundaryPhase]);
								++PressureCounts.ClaimLostTo.FindOrAdd(State.Knowing.PendingPetitionRow.ToString());
							}
							if (!bSlotTaken || bOwnCard)
							{
								const CosmicEdicts::ECosmicEdictBlock Gate = CosmicEdicts::EnactBlockOf(State, StateIndex, Sim->GetTables(), RulesNow, RuledIndex, VigilRow);
								++(Gate == CosmicEdicts::ECosmicEdictBlock::None ? Spirit.VigilOpen : Gate == CosmicEdicts::ECosmicEdictBlock::Held ? Spirit.VigilHeld
									: Gate == CosmicEdicts::ECosmicEdictBlock::Slots ? Spirit.VigilSlots : Gate == CosmicEdicts::ECosmicEdictBlock::Cost ? Spirit.VigilCost : Spirit.VigilOther);
							}
						}
					}
					const FCosmicPressure Reading = UGalaxySimulationSubsystem::ComputePressureIn(State, StateIndex, Sim->GetTables(), RulesNow, SystemIndex);
					if (Reading.P >= 0.9f)
					{
						PressureNow.Add(System.SystemID);
						if (!Spirit.PressureHeld.Contains(System.SystemID))
						{
							++Spirit.TriggerPressure;
							Spirit.TriggerPressureTaken += bSlotTaken ? 1 : 0;
							Spirit.TriggerPressureHigher += (Reading.DominantSource != ECosmicPressureSource::None && Reading.DominantTier > RuledAtBoundary->Tier) ? 1 : 0;
						}
					}
				}
				Spirit.ClaimHeld = MoveTemp(ClaimNow);
				Spirit.PressureHeld = MoveTemp(PressureNow);
			}
			// B4: the guarded hazards placed in the tick (merged into the standing list once the tick's ends are read).
			TMap<FName, FGameplayTag> NewGuarded;
			// B3: the delay a guard bought each hazard placed in the tick, read from its slot before the boundary (the count it
			// was placed with, less the row's countdown and the edicts' bonus on the system - the owner's and, from B6b, the
			// galaxy laws of its galaxy). The bonus is read from the union after the tick, not the one CosmicHazards::Place read
			// in the crisis stage: the two are the same while no galaxy law exists. Once laws exist they can differ both ways -
			// a law the AI's law step enacts, or enacts again after its upkeep lapsed, later in the same tick makes the bought
			// delay too small, and the laws of a people the crossing stage removes in the tick make it too large (as an edict the
			// AI stage revokes after the placement already did before B6b). B6b notes section 5 [B-6b2a]: the capture before the
			// tick is [B-6b3]'s, before the [B-6b7] rows make it live.
			for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && SystemIndex < Before.Num(); ++SystemIndex)
			{
				const FStarSystemState& System = State.Systems[SystemIndex];
				const FSlotBefore& Slot = Before[SystemIndex];
				const bool bPlaced = System.ActiveHazard.IsValid() && (System.ActiveHazard != Slot.Tag || System.HazardTicksRemaining >= Slot.Remaining);
				const FHazardDef* Placed = bPlaced ? Sim->GetTables().FindHazardByTag(System.ActiveHazard) : nullptr;
				const FCivilizationState* Owner = Placed ? State.Civs.FindByPredicate([&System](const FCivilizationState& Civ) { return Civ.CivID == System.OwnerCivID; }) : nullptr;
				if (Owner)
				{
					const int64 Delay = static_cast<int64>(System.HazardTicksRemaining) - FMath::Max(1, Placed->CountdownTicks)
						- CosmicEdicts::CountdownBonusOf(State, StateIndex, Sim->GetTables(), SystemIndex, System.ActiveHazard);
					if (Delay > 0)
					{
						TPair<int64, int32>& Bought = Counts.DelayBought.FindOrAdd(System.ActiveHazard.ToString());
						Bought.Key += Delay;
						++Bought.Value;
					}
					// B4: a hazard placed where the ruled people's fleet stood guard before the tick.
					const FCivBefore* Was = Owner->bIsPlayerRuled ? CivsBefore.Find(Owner->CivID) : nullptr;
					if (Was && Was->Target == System.SystemID)
					{
						const FString Tag = System.ActiveHazard.ToString();
						++PressureCounts.GuardedByTag.FindOrAdd(Tag);
						++PressureCounts.Guarded;
						PressureCounts.GuardedByEnd += bEndedBefore ? 0 : 1;
						PressureCounts.DelayByTag.FindOrAdd(Tag) += Delay > 0 ? Delay : 0;
						PressureCounts.DelayBought += Delay > 0 ? Delay : 0;
						// B6a [B-6a3c] (#132): by phase, with the row's countdown the delay is a share of.
						FGuardedPhase& ByPhase = Ending.Guarded[TickPhase].FindOrAdd(Tag);
						++ByPhase.Count;
						ByPhase.Delay += Delay > 0 ? Delay : 0;
						ByPhase.Countdown += FMath::Max(1, Placed->CountdownTicks);
						// Into the standing list after the ends of the tick are read: a guarded hazard that ended in this tick
						// on the same system is classified first.
						NewGuarded.Add(System.SystemID, System.ActiveHazard);
					}
				}
			}

			// The ruler's input between ticks, before anything below reads the tick: its lines carry this tick and are counted
			// with it, and a hazard an answer ends is seen ending in it. One function with the hash run of a game process
			// (World/CosmicRulerPolicy - MVP item 9), so the cooked build plays the same inputs.
			// The abdication first (B1 header section 5: the boundary's order - the end read, the abdication, the ruler's
			// input, the wanderer's), so that a ruler who leaves at this tick still gives this tick's answers.
			CosmicWandererPolicy::AbdicateIfDue(WandererPolicy, *Sim, *Role, WandererCounts);
			// B4 ([B-4d2]): the claim card that came at this boundary, and the ruled people's claims and fleet before the input.
			const int32 RuledBeforeAct = RuledAtBoundary ? StateIndex.FindCiv(State, RuledAtBoundary->CivID) : INDEX_NONE;
			const float FleetBeforeAct = State.Civs.IsValidIndex(RuledBeforeAct) ? State.Civs[RuledBeforeAct].FleetPower : 0.f;
			TMap<FName, int32> ClaimCountsBefore;
			for (const FStarSystemState& System : State.Systems)
			{
				if (State.Civs.IsValidIndex(RuledBeforeAct) && System.ActiveHazard == ClaimTag && System.OwnerCivID == State.Civs[RuledBeforeAct].CivID)
				{
					ClaimCountsBefore.Add(System.SystemID, System.HazardTicksRemaining);
				}
			}
			const FName CardSystemBefore = State.Knowing.PendingPetitionSystemID;
			const int32 WatchesBefore = RulerCounts.PressureAnswers[2];
			const int32 DiscernsBefore = RulerCounts.PressureAnswers[1];
			if (State.Knowing.PendingPetitionRow == CosmicRows::PetitionPressureClaim && State.Knowing.PendingPetitionRow != CardBefore && ClaimCountsBefore.Contains(CardSystemBefore))
			{
				FClaimTrack& Track = PressureCounts.Claims.FindOrAdd(CardSystemBefore);
				if (Track.CardTick < 0)
				{
					Track.CardTick = State.Tick;
					Track.RemainingAtCard = ClaimCountsBefore[CardSystemBefore];
				}
			}
			const EPlayerRole RoleBeforeInput = State.Player.Role;
			// B6a [B-6a3c] (#144): the ruled people's stock as a pressure card that comes at this boundary finds it; petition_exalt
			// waiting for the ruler for the first time, and the choice the policy's own ChoiceOf takes on it - Act asks it with
			// these arguments first, the card being the one that waits.
			const int32 PressureCardsBeforeAct = RulerCounts.PressureCards;
			const int32 AnswersBeforeAct = RulerCounts.Answers;
			const float InsightBeforeAct = State.Civs.IsValidIndex(RuledBeforeAct) ? State.Civs[RuledBeforeAct].Stockpile.Insight : 0.f;
			const float InfluenceBeforeAct = State.Civs.IsValidIndex(RuledBeforeAct) ? State.Civs[RuledBeforeAct].Stockpile.Influence : 0.f;
			bool bExaltNow = false;
			int32 ExaltPredicted = INDEX_NONE;
			if (Ending.ExaltChoice == INDEX_NONE && State.Knowing.PendingPetitionRow == CosmicRows::PetitionExalt && State.Player.Role == EPlayerRole::Ruler)
			{
				bExaltNow = true;
				Ending.ExaltTick = Ending.ExaltTick < 0 ? State.Tick : Ending.ExaltTick;
				const FEncounterDef* ExaltRow = Sim->GetTables().FindEncounter(CosmicRows::PetitionExalt);
				ExaltPredicted = (RulerPolicy.bAnswer && ExaltRow) ? CosmicRulerPolicy::ChoiceOf(State, StateIndex, *ExaltRow, RulerCounts.Answers, CosmicRows::PetitionExalt,
					RulerPolicy.Diplomacy, &RulesNow, &Sim->GetTables(), RulerPolicy.Pressure) : INDEX_NONE;
			}
			CosmicRulerPolicy::Act(RulerPolicy, *Sim, *Role, CardBefore, RulerCounts);
			CosmicWandererPolicy::Act(WandererPolicy, *Sim, *Role, CardBefore, WandererCounts);
			// B6a [B-6a3c]: what the inputs did for the ending line - the decisions taken at this boundary, the answer to
			// petition_exalt (its card answers first: a refusal stops the answers and a withdrawal counts none, so only its taken
			// answer adds to Answers), the stock of each pressure card that came, and a throne taken over a people at the third rung.
			{
				const int32 DecisionsNow = CosmicRulerPolicy::DecisionsOf(RulerCounts);
				if (DecisionsNow > Ending.DecisionsSoFar)
				{
					Ending.Decisions.Add(TPair<int64, int32>(State.Tick, DecisionsNow - Ending.DecisionsSoFar));
				}
				Ending.DecisionsSoFar = DecisionsNow;
				if (bExaltNow && RulerCounts.Answers > AnswersBeforeAct)
				{
					Ending.ExaltChoice = ExaltPredicted;
				}
				for (int32 Card = PressureCardsBeforeAct; Card < RulerCounts.PressureCards; ++Card)
				{
					Ending.CardInsight[BoundaryPhase].Add(InsightBeforeAct);
					Ending.CardInfluence[BoundaryPhase].Add(InfluenceBeforeAct);
				}
				NoteSeason();
			}
			// B5 ([B-5a]): the first throne taken after a reign ended; [B-5b]: after a succession, and a reign begun while a claim
			// that had its card still stood (old 16 #143, B5 decision 4 - the lock keeps it at 0).
			if (RoleBeforeInput == EPlayerRole::Wanderer && State.Player.Role == EPlayerRole::Ruler)
			{
				Calling.CrownedAfterTick = (Calling.CrownedAfterTick < 0 && Calling.FirstReignEndTick >= 0) ? State.Tick : Calling.CrownedAfterTick;
				if (Calling.ThroneAfterSuccession < 0 && Calling.FirstSuccessionTick >= 0)
				{
					Calling.ThroneAfterSuccession = State.Tick;
					Calling.ThroneAfterSuccessionPath = WandererCounts.LastCrownedPath;
				}
				// [B-5r] (review play-5): the first throne of the life the last succession began - the ticks since its birth.
				if (Calling.SuccessorLifeAt == State.Calling.Lives.Num() - 1 && !Calling.SuccessorThroneGaps.IsEmpty() && Calling.SuccessorThroneGaps.Last() < 0)
				{
					Calling.SuccessorThroneGaps.Last() = State.Tick - State.Calling.Lives.Last().BornTick;
				}
				for (const TPair<FName, FClaimTrack>& Claim : PressureCounts.Claims)
				{
					if (Claim.Value.CardTick >= 0)
					{
						++Calling.ReignsDuringCardedClaims;
						break;
					}
				}
			}
			// B4 ([B-4d2]): the time the fleet bought the ruled people's claims at this boundary, and the watch on a claim card.
			{
				bool bBought = false;
				for (const TPair<FName, int32>& Was : ClaimCountsBefore)
				{
					const int32 SystemIndex = StateIndex.FindSystem(State, Was.Key);
					if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].ActiveHazard == ClaimTag && State.Systems[SystemIndex].HazardTicksRemaining > Was.Value)
					{
						PressureCounts.ResistBought += State.Systems[SystemIndex].HazardTicksRemaining - Was.Value;
						bBought = true;
						FClaimTrack& Track = PressureCounts.Claims.FindOrAdd(Was.Key);
						PressureCounts.DelayedTwice += (++Track.Delays == 2) ? 1 : 0;
					}
				}
				if (bBought && State.Civs.IsValidIndex(RuledBeforeAct))
				{
					PressureCounts.FleetSpent += FMath::Max(0.0, static_cast<double>(FleetBeforeAct) - State.Civs[RuledBeforeAct].FleetPower);
					PressureCounts.Regrowing.Add({ State.Civs[RuledBeforeAct].CivID, State.Tick, FleetBeforeAct });
				}
				// A watch on the claim card itself: it ends the chain, so no discernment came with it (the measured card's further
				// watch follows one).
				const bool bWatchedClaim = RulerCounts.PressureAnswers[2] > WatchesBefore && RulerCounts.PressureAnswers[1] == DiscernsBefore;
				if (FClaimTrack* Track = bWatchedClaim ? PressureCounts.Claims.Find(CardSystemBefore) : nullptr)
				{
					Track->bWatched = true;
				}
			}
			Counts.CardsCome = RulerCounts.CardsCome;
			Counts.Answers = RulerCounts.Answers;
			Counts.Started = RulerCounts.Started;
			Counts.FocusChanges = RulerCounts.FocusChanges;
			// B2: the AI peoples' edicts given up in the tick (a people crowned or removed meanwhile is not counted).
			for (const FCivilizationState& Civ : State.Civs)
			{
				if (const TArray<FName>* Held = Civ.bIsPlayerRuled ? nullptr : AIEdictsBefore.Find(Civ.CivID))
				{
					for (const FName Row : *Held)
					{
						Counts.AIEdictsRevoked += Civ.ActiveEdicts.Contains(Row) ? 0 : 1;
					}
				}
			}

			// B3: what the diplomacy and the conflict changed in this tick and at its boundary.
			{
				const FCivilizationState* RuledNow = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
				const FName RuledID = RuledNow ? RuledNow->CivID : NAME_None;
				// Up to and including the tick of the first end.
				const bool bBeforeEnd = !bEndedBefore;
				// The ends the conflict stage decided in the tick.
				for (const FCosmicStageClock::FSiegeEnd& End : Sim->GetStageClock().SiegeEnds)
				{
					if (End.Kind == FCosmicStageClock::ESiegeEnd::Taken)
					{
						++Counts.Taken;
						++Counts.TakenByBand[State.Tick <= 3000 ? 0 : (State.Tick <= 6000 ? 1 : 2)];
						++Counts.TakenBy.FindOrAdd(End.Attacker);
						++Counts.LostBy.FindOrAdd(End.Owner);
						Counts.ConqueredBy.Add(TPair<FName, FName>(End.Attacker, End.SystemID));
						if (End.Owner == RuledID)
						{
							++Counts.TakenFromRuled;
							Counts.TakenFromRuledByEnd += bBeforeEnd ? 1 : 0;
						}
					}
					else
					{
						++(End.Kind == FCosmicStageClock::ESiegeEnd::Held ? Counts.Held : Counts.Lifted);
					}
				}
				TMap<FName, int32> HostileToward;
				TMap<FName, int32> SiegersOn;
				for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
				{
					const FCivilizationState& Civ = State.Civs[CivIndex];
					const FCivBefore* Was = CivsBefore.Find(Civ.CivID);
					const FCivBefore Now = CivBeforeOf(State, StateIndex, Civ);
					if (Now.TargetOwner != NAME_None && Now.TargetOwner != Civ.CivID)
					{
						++SiegersOn.FindOrAdd(Now.TargetOwner);
					}
					for (const FName Other : Now.Hostile)
					{
						HostileToward.FindOrAdd(Other) += Civ.bIsPlayerRuled ? 0 : 1;
					}
					if (!Was)
					{
						continue;
					}
					const bool bWasSiege = !Was->TargetOwner.IsNone() && Was->TargetOwner != Civ.CivID;
					const bool bTargetMoved = Now.Target != Was->Target || Now.Since != Was->Since;
					const bool bStageEnded = Sim->GetStageClock().SiegeEnds.ContainsByPredicate([&Civ](const FCosmicStageClock::FSiegeEnd& End) { return End.Attacker == Civ.CivID; });
					if (bWasSiege && bTargetMoved && !bStageEnded)
					{
						++Counts.Lifted;
					}
					// A climb from T2 to T3 that a taken system helped: three settled systems or more with the taken ones (the
					// three-worlds feat could have fired), fewer than three without them.
					if (Was->Tier == EAscensionTier::Tier2_Stellar && Civ.Tier == EAscensionTier::Tier3_Causal)
					{
						int32 Settled = 0;
						int32 Taken = 0;
						for (const FName SystemID : Civ.OwnedSystems)
						{
							const int32 System = StateIndex.FindSystem(State, SystemID);
							if (State.Systems.IsValidIndex(System) && State.Systems[System].OwnerCivID == Civ.CivID && CosmicState::IsSettled(State.Systems[System]))
							{
								++Settled;
								Taken += Counts.ConqueredBy.Contains(TPair<FName, FName>(Civ.CivID, SystemID)) ? 1 : 0;
							}
						}
						const bool bHelped = Taken > 0 && Settled >= 3 && Settled - Taken < 3;
						Counts.ThreeWorldsByConquest += bHelped ? 1 : 0;
						Counts.ThreeWorldsByConquestRuled += (bHelped && Civ.bIsPlayerRuled) ? 1 : 0;
					}
					const bool bIsSiege = !Now.TargetOwner.IsNone() && Now.TargetOwner != Civ.CivID;
					if (bIsSiege && bTargetMoved)
					{
						++Counts.SiegesBegun;
						if (Now.TargetOwner == RuledID)
						{
							++Counts.SiegesOnRuled;
							Counts.SiegesOnRuledByEnd += bBeforeEnd ? 1 : 0;
							// B6a [B-6a3c] (#145): begun while the ruled people's fleet regrows after a resistance of an earlier boundary.
							const bool bRegrowing = PressureCounts.Regrowing.ContainsByPredicate([&State, RuledID](const FPressureCounts::FRegrowing& Each)
							{
								return Each.CivID == RuledID && Each.Since < State.Tick;
							});
							Ending.SiegesRegrowing[TickPhase] += bRegrowing ? 1 : 0;
						}
						const int32 System = StateIndex.FindSystem(State, Now.Target);
						if (!Civ.bIsPlayerRuled && State.Systems.IsValidIndex(System)
							&& UGalaxySimulationSubsystem::ComputeAttritionPressureIn(State, StateIndex, Sim->GetTables(), RulesNow, CivIndex, System).P > 0.f)
						{
							++Counts.AISiegesIntoSphere;
						}
					}
					for (const FName Other : Now.Hostile)
					{
						if (!Was->Hostile.Contains(Other))
						{
							++Counts.HostilitiesOpened[State.Tick <= 3000 ? 0 : (State.Tick <= 6000 ? 1 : 2)];
						}
					}
					// Each pair once: from the people whose CivID comes first.
					for (const FString& Treaty : Now.Treaties)
					{
						FString OtherText;
						Treaty.Split(TEXT("|"), &OtherText, nullptr);
						if (!Was->Treaties.Contains(Treaty) && Civ.CivID.LexicalLess(FName(*OtherText)))
						{
							++Counts.TreatiesSignedPairs;
						}
						// The first reign's non-aggression with the people of the first contact (its side holds every treaty).
						if (!Was->Treaties.Contains(Treaty) && Civ.bIsPlayerRuled && Civ.CivID == FirstRuled && FName(*OtherText) == State.Knowing.FirstContactCivID
							&& Treaty.EndsWith(CosmicTags::TreatyNonAggression().ToString()) && Counts.FirstContactNonAggressionTick < 0)
						{
							Counts.FirstContactNonAggressionTick = State.Tick;
						}
					}
					for (const FString& Treaty : Was->Treaties)
					{
						FString OtherText;
						Treaty.Split(TEXT("|"), &OtherText, nullptr);
						const bool bOtherHere = State.Civs.ContainsByPredicate([&OtherText](const FCivilizationState& Each) { return Each.CivID == FName(*OtherText); });
						Counts.TreatiesBrokenPairs += (!Now.Treaties.Contains(Treaty) && bOtherHere && Civ.CivID.LexicalLess(FName(*OtherText))) ? 1 : 0;
					}
					// The first contact people turned friendly by the card's answer (1), not (0), or friendly before it (2) - read
					// at the boundary that answered it, in the first reign.
					if (Civ.bIsPlayerRuled && Civ.CivID == FirstRuled && Counts.FirstContactFriendly < 0 && bContactWaits
						&& State.Knowing.PendingPetitionRow != CosmicRows::PetitionFirstContact)
					{
						const bool bFriendlyNow = CosmicDiplomacy::StanceOf(Civ, State.Knowing.FirstContactCivID) == CosmicTags::StanceFriendly();
						Counts.FirstContactFriendly = bContactFriendlyBefore ? 2 : (bFriendlyNow ? 1 : 0);
					}
				}
				for (const TPair<FName, int32>& Each : HostileToward)
				{
					Counts.MaxHostileTowardOne = FMath::Max(Counts.MaxHostileTowardOne, Each.Value);
				}
				for (const TPair<FName, int32>& Each : SiegersOn)
				{
					Counts.MaxSiegersOnOne = FMath::Max(Counts.MaxSiegersOnOne, Each.Value);
				}
				int32 Mutual = 0;
				for (int32 A = 0; A < State.Civs.Num(); ++A)
				{
					const int32 TargetOfA = StateIndex.FindSystem(State, State.Civs[A].FleetTargetSystemID);
					const int32 B = State.Systems.IsValidIndex(TargetOfA) ? StateIndex.FindCiv(State, State.Systems[TargetOfA].OwnerCivID) : INDEX_NONE;
					if (!State.Civs.IsValidIndex(B) || B <= A)
					{
						continue;
					}
					const int32 TargetOfB = StateIndex.FindSystem(State, State.Civs[B].FleetTargetSystemID);
					Mutual += (State.Systems.IsValidIndex(TargetOfB) && State.Systems[TargetOfB].OwnerCivID == State.Civs[A].CivID) ? 1 : 0;
				}
				Counts.MutualSiegeTicks += Mutual > 0 ? 1 : 0;
				Counts.MaxMutualPairs = FMath::Max(Counts.MaxMutualPairs, Mutual);
				// The ruler's window (first reign's people only while it is ruled).
				const int32 RuledIndex = RuledNow ? StateIndex.FindCiv(State, RuledID) : INDEX_NONE;
				if (State.Civs.IsValidIndex(RuledIndex) && RuledID == FirstRuled)
				{
					if (Counts.SecondSettledTick < 0 && CosmicConflict::SettledCountOf(State, StateIndex, RuledIndex) >= 2)
					{
						Counts.SecondSettledTick = State.Tick;
					}
					for (int32 SystemIndex = 0; Counts.FirstSiegeTargetTick < 0 && SystemIndex < State.Systems.Num(); ++SystemIndex)
					{
						const FName Owner = State.Systems[SystemIndex].OwnerCivID;
						const bool bMetOwner = !Owner.IsNone() && Owner != RuledID
							&& State.Knowing.MetCivs.ContainsByPredicate([Owner](const FKnownMark& Mark) { return Mark.ID == Owner; });
						const CosmicConflict::ECosmicSiegeBlock Gate = bMetOwner ? CosmicConflict::SiegeBlockOf(State, StateIndex, RulesNow, RuledIndex, SystemIndex, true)
							: CosmicConflict::ECosmicSiegeBlock::Own;
						if (Gate == CosmicConflict::ECosmicSiegeBlock::None || Gate == CosmicConflict::ECosmicSiegeBlock::NonAggression)
						{
							Counts.FirstSiegeTargetTick = State.Tick;
						}
					}
					// B5 ([B-5a] - #133): the same window with every realm cap raised (B'), and while it has not opened before the
					// end, the first reason of each met owner's system.
					for (int32 SystemIndex = 0; Calling.WindowRaisedTick < 0 && SystemIndex < State.Systems.Num(); ++SystemIndex)
					{
						const FName Owner = State.Systems[SystemIndex].OwnerCivID;
						const bool bMetOwner = !Owner.IsNone() && Owner != RuledID
							&& State.Knowing.MetCivs.ContainsByPredicate([Owner](const FKnownMark& Mark) { return Mark.ID == Owner; });
						if (!bMetOwner)
						{
							continue;
						}
						const CosmicConflict::ECosmicSiegeBlock Gate = CosmicConflict::SiegeBlockOf(State, StateIndex, RulesRaised, RuledIndex, SystemIndex, true);
						if (Gate == CosmicConflict::ECosmicSiegeBlock::None || Gate == CosmicConflict::ECosmicSiegeBlock::NonAggression)
						{
							Calling.WindowRaisedTick = State.Tick;
						}
						else if (bBeforeEnd)
						{
							++Calling.WindowRaisedReasons.FindOrAdd(SiegeBlockNameOf(Gate));
						}
					}
				}
				if (State.Tick == 1000 || State.Tick == 3000 || State.Tick == 6000)
				{
					Counts.BorderPairs[State.Tick == 1000 ? 0 : (State.Tick == 3000 ? 1 : 2)] = BorderPairsOf(State, StateIndex);
				}
				if (State.Tick == 3000 || State.Tick == 6000)
				{
					Counts.FleetMedian[State.Tick == 3000 ? 0 : 1] = FleetMedianOf(State);
				}
			}

			if (Counts.FirstTwoSettledTick < 0 && HoldsTwoSettled(State))
			{
				Counts.FirstTwoSettledTick = State.Tick;
			}
			// B4 ([B-4d2]): the vigil's ease lines of the tick (the row's line names its system).
			TSet<FName> EasedNow;
			for (int32 Index = State.Log.Num() - 1; Index >= 0 && State.Log[Index].Tick > TickBefore; --Index)
			{
				if (State.Log[Index].SourceRow == CosmicRows::EventClaimEased)
				{
					EasedNow.Add(State.Log[Index].FocusSystemID);
					++PressureCounts.Eased;
					PressureCounts.EasedByEnd += bEndedBefore ? 0 : 1;
				}
			}
			// A hazard is placed when a slot holds a tag it did not hold or holds it with a full count again; it ended when
			// its tag is gone or counted again. An ended hazard was answered when the answer in its response slot completed on
			// this tick (its completion record moved to this tick), and it expired when it was not answered and its count was
			// 1 before the tick (the crisis stage runs the answers before the countdown). A hazard placed and ended inside one
			// tick is not seen here (its lines are in the rows file).
			for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && SystemIndex < Before.Num(); ++SystemIndex)
			{
				const FStarSystemState& System = State.Systems[SystemIndex];
				const FSlotBefore& Slot = Before[SystemIndex];
				const bool bAgain = System.ActiveHazard == Slot.Tag && System.HazardTicksRemaining >= Slot.Remaining;
				if (Slot.Tag.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
				{
					const FString Tag = Slot.Tag.ToString();
					Counts.Ended.FindOrAdd(Tag)++;
					const bool bAnswered = !Slot.Answer.IsNone() && CosmicMegaprojects::HasCompleted(System, Slot.Answer, State.Tick, NAME_None);
					if (bAnswered)
					{
						Counts.Answered.FindOrAdd(Tag)++;
					}
					else if (Slot.Remaining == 1 && !Slot.HeldAnswer.IsNone() && Slot.bUpkeepPayable)
					{
						// Resolved at its count's end by the owner's edict (B2): not an expiry.
						++Counts.ResolvedByEdict;
					}
					else if (Slot.Remaining == 1)
					{
						Counts.Expired.FindOrAdd(Tag)++;
					}
					// B4: the ruled people's claims that ended, and the other hazards that ended within a claimant's reach.
					const bool bByEdict = !bAnswered && Slot.Remaining == 1 && !Slot.HeldAnswer.IsNone() && Slot.bUpkeepPayable;
					if (const FGameplayTag* Guarded = PressureCounts.Standing.Find(System.SystemID); Guarded && *Guarded == Slot.Tag)
					{
						PressureCounts.GuardedAnswered += bAnswered ? 1 : 0;
						PressureCounts.GuardedByEdict += bByEdict ? 1 : 0;
						PressureCounts.GuardedExpired += (!bAnswered && !bByEdict && Slot.Remaining == 1) ? 1 : 0;
						PressureCounts.Standing.Remove(System.SystemID);
					}
					if (Slot.Tag == ClaimTag && Slot.bRuledSettled)
					{
						const bool bJoined = InSomeSphere(State, System.SystemID);
						Spirit.Joined += bJoined ? 1 : 0;
						Spirit.JoinedByEnd += (bJoined && !bEndedBefore) ? 1 : 0;
						Spirit.Stilled += bJoined ? 0 : 1;
						Spirit.StilledByRelief += (!bJoined && bAnswered) ? 1 : 0;
						// [B-4d2]: the claim's end of five, and its card's measure (header section 2.8 - a state, not an answer).
						const bool bEased = !bJoined && EasedNow.Contains(System.SystemID);
						const int32 EndAt = bJoined ? 0 : bEased ? 1 : bAnswered ? 2 : (!bByEdict && Slot.Remaining == 1) ? 4 : 3;
						++PressureCounts.Ends[EndAt];
						const FClaimTrack* Track = PressureCounts.Claims.Find(System.SystemID);
						PressureCounts.NoCard += (!Track || Track->CardTick < 0) ? 1 : 0;
						if (Track && Track->CardTick >= 0 && !Track->bVigilBroken && !Track->bRelief && !bAnswered)
						{
							const int32 Band = Track->RemainingAtCard <= 300 ? 0 : (Track->RemainingAtCard <= 400 ? 1 : (Track->RemainingAtCard <= 550 ? 2 : 3));
							++PressureCounts.Band[Band];
							PressureCounts.BandEased[Band] += bEased ? 1 : 0;
							// B6a [B-6a3c] (#142): the band 401-550 by the phase of the claim's end, and its joined.
							Ending.Band142[TickPhase] += Band == 2 ? 1 : 0;
							Ending.Band142Joined[TickPhase] += (Band == 2 && bJoined) ? 1 : 0;
						}
						if (Track && Track->bWatched)
						{
							++PressureCounts.Watched;
							PressureCounts.WatchedJoined += bJoined ? 1 : 0;
						}
					}
					else if (Slot.Tag != ClaimTag && Slot.bRuledSettled && Slot.bClaimReach)
					{
						const bool bExpired = !bAnswered && !bByEdict && Slot.Remaining == 1;
						Spirit.SourcesAnswered += bAnswered ? 1 : 0;
						Spirit.SourcesByEdict += bByEdict ? 1 : 0;
						Spirit.SourcesExpired += bExpired ? 1 : 0;
						Spirit.SourcesExpiredByEnd += (bExpired && !bEndedBefore) ? 1 : 0;
						Spirit.SourcesRemnant += (bExpired && Slot.bLastUnclaimed) ? 1 : 0;
					}
					if (Slot.Tag == ClaimTag)
					{
						PressureCounts.Claims.Remove(System.SystemID);
						if (Counts.FirstClaimEndTick < 0 && System.SystemID == Counts.FirstClaimSystemID)
						{
							Counts.FirstClaimEndTick = State.Tick;
						}
						FClaimEnds& Ends = InSomeSphere(State, System.SystemID) ? Counts.ClaimsJoined : Counts.ClaimsStilled;
						++Ends.Count;
						Ends.WithAnswer += Slot.Answer.IsNone() ? 0 : 1;
						Ends.OwnerInfluence += Slot.OwnerInfluence;
					}
				}
				if (System.ActiveHazard.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
				{
					Counts.Placed.FindOrAdd(System.ActiveHazard.ToString())++;
					const FCivilizationState* Owner = State.Civs.FindByPredicate([&System](const FCivilizationState& Civ) { return Civ.CivID == System.OwnerCivID; });
					// B3: a plague placed next to one that still stood when the spawn step began (not one that ended by its answer
					// or its count in this tick), and the ruled people's.
					if (System.ActiveHazard == CosmicTags::HazardPlague())
					{
						const bool bRuled = Owner && Owner->bIsPlayerRuled;
						Counts.PlaguePlacedRuled += bRuled ? 1 : 0;
						Counts.PlaguePlacedRuledByEnd += (bRuled && !bEndedBefore) ? 1 : 0;
						for (const FName Lane : System.Lanes)
						{
							const int32 Neighbour = StateIndex.FindSystem(State, Lane);
							const bool bStood = Before.IsValidIndex(Neighbour) && Before[Neighbour].Tag == CosmicTags::HazardPlague() && Before[Neighbour].Remaining > 1
								&& !(!Before[Neighbour].Answer.IsNone() && CosmicMegaprojects::HasCompleted(State.Systems[Neighbour], Before[Neighbour].Answer, State.Tick, NAME_None));
							if (bStood)
							{
								++Counts.PlagueSpreadPlaced;
								Counts.PlagueSpreadRuled += bRuled ? 1 : 0;
								break;
							}
						}
					}
					if (Counts.FirstRebellionTick < 0 && Owner && Owner->bIsPlayerRuled && Owner->HomeSystemID == System.SystemID && IsRebellionTag(Sim->GetTables(), System.ActiveHazard))
					{
						Counts.FirstRebellionTick = State.Tick;
					}
					if (System.ActiveHazard == ClaimTag && Owner && Owner->bIsPlayerRuled)
					{
						++Spirit.ClaimsOnRuled;
						Spirit.ClaimsOnRuledByEnd += bEndedBefore ? 0 : 1;
						// B6a [B-6a3c]: a claim on a settled system of the ruled people in the season.
						if (TickPhase == 1 && CosmicState::IsSettled(System))
						{
							++Ending.SeasonClaims;
							Ending.FirstSeasonClaim = Ending.FirstSeasonClaim < 0 ? State.Tick : Ending.FirstSeasonClaim;
						}
					}
					if (System.ActiveHazard == ClaimTag)
					{
						if (Counts.FirstClaimTick < 0)
						{
							Counts.FirstClaimTick = State.Tick;
							Counts.FirstClaimSystemID = System.SystemID;
						}
						Counts.ClaimsAfterTwoSettled += Counts.FirstTwoSettledTick >= 0 ? 1 : 0;
					}
				}
			}
			PressureCounts.Standing.Append(NewGuarded);
			// B4 ([B-4d2]): the Standing units held after the ruler's input - the next tick's head.
			{
				const FCivilizationState* RuledNow = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
				const bool bRulingNow = RuledNow && State.Player.Role == EPlayerRole::Ruler && State.Calling.LifeState == ELifeState::Embodied;
				PressureCounts.HeldAfterInput = bRulingNow ? StandingUnitsOf(State, StateIndex, Sim->GetTables(), RulesNow, StateIndex.FindCiv(State, RuledNow->CivID))
					: TArray<FCosmicPetitionUnit>();
				// B6a [B-6a3c]: petition_exalt's B6a condition after the inputs (the head the pre-count compares with).
				Ending.bExaltHeldAfterInput = bRulingNow && RuledNow->Tier >= EAscensionTier::Tier2_Stellar;
			}
			// B4 ([B-4d2]): the fleets back to their size before a resistance (#131).
			for (int32 At = PressureCounts.Regrowing.Num() - 1; At >= 0; --At)
			{
				const FPressureCounts::FRegrowing& Each = PressureCounts.Regrowing[At];
				const FCivilizationState* Civ = State.Civs.FindByPredicate([&Each](const FCivilizationState& Other) { return Other.CivID == Each.CivID; });
				if (Civ && Civ->FleetPower >= Each.Before - 1.0e-4f)
				{
					PressureCounts.Regrown.Add(State.Tick - Each.Since);
					PressureCounts.Regrowing.RemoveAt(At);
				}
			}
			// B4 ([B-4d2]): the vigil and a project on the claims a card came for, at this boundary after the ruler's input.
			{
				const FCivilizationState* RuledNow = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
				for (TPair<FName, FClaimTrack>& Each : PressureCounts.Claims)
				{
					if (Each.Value.CardTick < 0)
					{
						continue;
					}
					const int32 SystemIndex = StateIndex.FindSystem(State, Each.Key);
					Each.Value.bVigilBroken = Each.Value.bVigilBroken || !RuledNow || VigilRow.IsNone() || !RuledNow->ActiveEdicts.Contains(VigilRow);
					Each.Value.bRelief = Each.Value.bRelief || (State.Systems.IsValidIndex(SystemIndex) && !State.Systems[SystemIndex].ResponseMegaproject.IsNone());
				}
			}
			if (Counts.FirstJoinTick < 0 && SphereSize(State) > SphereBefore)
			{
				Counts.FirstJoinTick = State.Tick;
			}
			// B4: the joins of the tick (a system new to a sphere), the ones on a known system with a met owner, and the
			// snapshots of the spirit line.
			{
				int32 JoinsNow = 0;
				const bool bLowViewer = Sim->GetViewerTier() <= EAscensionTier::Tier2_Stellar;
				for (int32 At = 0; At < State.Transcendents.Num() && At < SpheresBefore.Num(); ++At)
				{
					for (const FName SystemID : State.Transcendents[At].InfluencedSystems)
					{
						if (SpheresBefore[At].Contains(SystemID))
						{
							continue;
						}
						++JoinsNow;
						const int32 SystemIndex = StateIndex.FindSystem(State, SystemID);
						const FName Owner = State.Systems.IsValidIndex(SystemIndex) ? State.Systems[SystemIndex].OwnerCivID : NAME_None;
						const bool bKnown = State.Knowing.KnownSystems.ContainsByPredicate([SystemID](const FKnownMark& Mark) { return Mark.ID == SystemID; });
						const bool bMet = !Owner.IsNone() && State.Knowing.MetCivs.ContainsByPredicate([Owner](const FKnownMark& Mark) { return Mark.ID == Owner; });
						if (bKnown && bMet)
						{
							++Spirit.KnownJoins;
							Spirit.KnownJoinsByEnd += bEndedBefore ? 0 : 1;
							++(bLowViewer ? Spirit.KnownJoinsLow : Spirit.KnownJoinsHigh);
						}
					}
				}
				Spirit.SameTickJoinsMax = FMath::Max(Spirit.SameTickJoinsMax, JoinsNow);
				const FCivilizationState* RuledNow = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
				if (State.Tick == 600 || State.Tick == 1200)
				{
					Spirit.MetNear[State.Tick == 600 ? 0 : 1] = MetPeoplesNearFallen(State, StateIndex, Sim->GetTables(), RuledNow ? RuledNow->CivID : NAME_None);
				}
				if (State.Tick == 1000 || State.Tick == 3000 || State.Tick == 6000)
				{
					Spirit.SphereAt[State.Tick == 1000 ? 0 : (State.Tick == 3000 ? 1 : 2)] = SphereSize(State);
				}
			}
			// A deposition, not an abdication (B1: the wanderer line's 'left at' tells that one).
			static const FGameplayTag DeposedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed")));
			if (Counts.DeposedTick < 0 && RoleBefore == EPlayerRole::Ruler && State.Player.Role != EPlayerRole::Ruler && !State.Calling.Lives.IsEmpty()
				&& !State.Calling.Lives.Last().ReignEndReasons.IsEmpty() && State.Calling.Lives.Last().ReignEndReasons.Last() == DeposedTag)
			{
				Counts.DeposedTick = State.Tick;
			}
			for (int32 Index = State.Log.Num() - 1; Index >= 0 && State.Log[Index].Tick > TickBefore; --Index)
			{
				SampleRows.FindOrAdd(State.Log[Index].SourceRow.ToString())++;
				Counts.EdictsLapsed += State.Log[Index].SourceRow == CosmicRows::LogEdictLapsed ? 1 : 0;
				if (State.Log[Index].Category == ConflictTag && State.Log[Index].Severity >= 2)
				{
					++Counts.ConflictSev2;
					Counts.ConflictSev2ByEnd += !bEndedBefore ? 1 : 0;
					++Counts.ConflictSev2Blocks.FindOrAdd((State.Log[Index].Tick - 1) / 600);
				}
				Counts.ToldLines += State.Log[Index].bUntold ? 0 : 1;
				// The voice ([B-4d1]): a first-person line is a Log.Claim line whose source is a transcendent row.
				const FCosmicLogEntry& Said = State.Log[Index];
				const int64 SaidBlock = (Said.Tick - 1) / 600;
				if (Said.Category == CosmicTags::LogClaim() && Sim->GetTables().FindTranscendent(Said.SourceRow))
				{
					if (Said.bUntold)
					{
						++PressureCounts.VoiceUntold;
					}
					else
					{
						++PressureCounts.VoiceTold;
						PressureCounts.VoiceToldByEnd += bEndedBefore ? 0 : 1;
						++PressureCounts.VoiceBlocks.FindOrAdd(SaidBlock);
						PressureCounts.FirstVoice = (PressureCounts.FirstVoice < 0 || Said.Tick < PressureCounts.FirstVoice) ? Said.Tick : PressureCounts.FirstVoice;
					}
				}
				else if (Said.Category == SignTag && !Said.bUntold)
				{
					++PressureCounts.SignTold;
					PressureCounts.SignToldByEnd += bEndedBefore ? 0 : 1;
					++PressureCounts.SignBlocks.FindOrAdd(SaidBlock);
					PressureCounts.FirstSign = (PressureCounts.FirstSign < 0 || Said.Tick < PressureCounts.FirstSign) ? Said.Tick : PressureCounts.FirstSign;
				}
				// B6a [B-6a3c] (#140): the awareness lines told.
				if (Said.Category == AwarenessTag && !Said.bUntold)
				{
					++Ending.AwarenessTold;
					Ending.AwarenessToldByEnd += bEndedBefore ? 0 : 1;
					Ending.FirstAwarenessTold = (Ending.FirstAwarenessTold < 0 || Said.Tick < Ending.FirstAwarenessTold) ? Said.Tick : Ending.FirstAwarenessTold;
				}
				PressureCounts.FleetWorn += Said.SourceRow == CosmicRows::LogFleetWorn ? 1 : 0;
			}
			if (const FCivilizationState* Ruled = State.Civs.FindByPredicate([FirstRuled](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled && Civ.CivID == FirstRuled; }))
			{
				Counts.RuledT2Tick = (Counts.RuledT2Tick < 0 && Ruled->Tier >= EAscensionTier::Tier2_Stellar) ? State.Tick : Counts.RuledT2Tick;
				Counts.RuledT3Tick = (Counts.RuledT3Tick < 0 && Ruled->Tier >= EAscensionTier::Tier3_Causal) ? State.Tick : Counts.RuledT3Tick;
			}
			if (State.Tick == 600 || State.Tick == 6000)
			{
				(State.Tick == 600 ? Counts.DecisionsAt600 : Counts.DecisionsAt6000) = CosmicRulerPolicy::DecisionsOf(RulerCounts);
				(State.Tick == 600 ? Counts.ToldAt600 : Counts.ToldAt6000) = Counts.ToldLines;
			}

			if ((Done + 1) % SampleEveryTicks == 0 || Done + 1 == Ticks)
			{
				++Samples;
				FString SaveBytes;
				FString SaveRawBytes;
				if (Samples % SaveEverySamples == 0 || Done + 1 == Ticks)
				{
					int64 RawBytes = 0;
					SaveBytes = FString::FromInt(SaveBytesOf(Save, State, RawBytes, Unused));
					SaveRawBytes = FString::Printf(TEXT("%lld"), RawBytes);
				}
				AppendSample(Csv, *Sim, Before, Counts, SampleTicks > 0 ? SampleMicros / SampleTicks : 0.0, SaveBytes, SaveRawBytes, &DrainsBefore);
				Counts.ExpansionPairs += PeoplesNotRuled(State);
				TArray<FString> Keys;
				SampleRows.GetKeys(Keys);
				Keys.Sort();
				for (const FString& Key : Keys)
				{
					Rows += FString::Printf(TEXT("%lld,%s,%d\n"), State.Tick, *Key, SampleRows[Key]);
				}
				SampleRows.Reset();
				SampleMicros = 0.0;
				SampleTicks = 0;
			}
		}
		// B6a [B-6a3b]: the end check names its first miss in the loop; more than one is counted here, once.
		if (EndCheckMisses > 1)
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicBalance end check: the record missed at %lld boundaries, the first at %lld"), EndCheckMisses, EndCheckFirstMiss);
		}

		if (FFileHelper::SaveStringToFile(Csv, *OutPath) && FFileHelper::SaveStringToFile(Rows, *RowsPath))
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: seed %d, %lld ticks, hash %u, world %u, wrote %s"), Seed, Ticks, Sim->ComputeStateHash(), Sim->ComputeWorldHash(), *OutPath);
			TickMicros.Sort();
			double Sum = 0.0;
			for (const double Each : TickMicros)
			{
				Sum += Each;
			}
			double SaveMilliseconds = 0.0;
			int64 SaveRawBytes = 0;
			const int32 SaveBytes = SaveBytesOf(Save, State, SaveRawBytes, SaveMilliseconds);
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance summary: tell-all %d, %d systems, %d peoples; claims %d placed, %d ended, %d joined; first claim %lld on %s until %lld, first join %lld; first two-settled people %lld, claims after it %d; sphere %d->%d; wild %d; vessel deposed at %lld; tick us mean %.1f p99 %.1f max %.1f; save %d bytes (%lld before packing) in %.1f ms; log %d, chronicle %d (%d kept for good), let go %d, told %lld"),
				TellAll, State.Systems.Num(), State.Civs.Num(), Counts.Placed.FindRef(ClaimTag.ToString()), Counts.ClaimsJoined.Count + Counts.ClaimsStilled.Count,
				Counts.ClaimsJoined.Count, Counts.FirstClaimTick, *Counts.FirstClaimSystemID.ToString(), Counts.FirstClaimEndTick, Counts.FirstJoinTick, Counts.FirstTwoSettledTick, Counts.ClaimsAfterTwoSettled, SphereStart,
				SphereSize(State), WildSystems(State), Counts.DeposedTick,
				TickMicros.Num() ? Sum / TickMicros.Num() : 0.0, TickMicros.Num() ? TickMicros[FMath::Min(TickMicros.Num() - 1, static_cast<int32>(TickMicros.Num() * 0.99))] : 0.0,
				TickMicros.Num() ? TickMicros.Last() : 0.0, SaveBytes, SaveRawBytes, SaveMilliseconds, State.Log.Num(), State.Chronicle.Num(), KeptForGood(State), State.ChronicleEviction.Count, Counts.ToldLines);
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance pack: %s"), *PackLevelsOf(Save, State));
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance claims: %s | %s; spheres %s -> %s"), *ClaimEndsText(TEXT("joined"), Counts.ClaimsJoined),
				*ClaimEndsText(TEXT("stilled"), Counts.ClaimsStilled), *FString::Join(SpheresStart, TEXT(", ")), *FString::Join(SpheresOf(State), TEXT(", ")));
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance hazards: placed%s | answered%s | expired%s | ended%s | resolved by edict %d"), *CountsText(Counts.Placed),
				*CountsText(Counts.Answered), *CountsText(Counts.Expired), *CountsText(Counts.Ended), Counts.ResolvedByEdict);
			// B2 (header section 5): always told, outside the baselines' keys.
			{
				TArray<FString> HeldAtEnd;
				for (const FCivilizationState& Civ : State.Civs)
				{
					if (Civ.bIsPlayerRuled)
					{
						for (const FName Row : Civ.ActiveEdicts)
						{
							HeldAtEnd.Add(Row.ToString());
						}
					}
				}
				TArray<FName> HeldRows;
				Counts.EdictHeldTicks.GetKeys(HeldRows);
				HeldRows.Sort(FNameLexicalLess());
				TArray<FString> HeldTicks;
				for (const FName Row : HeldRows)
				{
					HeldTicks.Add(FString::Printf(TEXT("%s=%lld"), *Row.ToString(), Counts.EdictHeldTicks[Row]));
				}
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance edicts: edict %d row %s; ruler enacted %d, revoked %d, lapsed %d, held at end [%s], held ticks [%s]; AI revoked %d; resolved by edict %d; first rebellion at %lld"),
					Edict, EdictRowText.IsEmpty() ? TEXT("-") : *EdictRowText, RulerCounts.EdictsEnacted, RulerCounts.EdictsRevoked, Counts.EdictsLapsed,
					*FString::Join(HeldAtEnd, TEXT(" ")), *FString::Join(HeldTicks, TEXT(" ")), Counts.AIEdictsRevoked, Counts.ResolvedByEdict, Counts.FirstRebellionTick);
			}
			// Why the AI did not widen its realm (section 9.3): one reason per people the player does not rule at every
			// sample, the tick-0 sample included. The reasons add up to the pairs counted apart, or an error line says so.
			int64 ExpansionSum = 0;
			for (const TPair<FString, int32>& Each : Counts.Expansion)
			{
				ExpansionSum += Each.Value;
			}
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance expansion: %lld (sample, people) pairs of peoples the player does not rule, the tick-0 sample included;%s"),
				Counts.ExpansionPairs, *CountsText(Counts.Expansion));
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance ruler: answer %d build %d focus %d edict %d diplomacy %d; cards %d, answers %d, started %d, focus changes %d, edict changes %d, diplomacy changes %d, decisions %d (by 600 %lld, by 6000 %lld, by end %lld); told by 600 %lld, by 6000 %lld; ruled people T2 at %lld, T3 at %lld"),
				Answer, Build, Focus, Edict, Diplomacy, Counts.CardsCome, Counts.Answers, Counts.Started, Counts.FocusChanges, RulerCounts.EdictsEnacted + RulerCounts.EdictsRevoked,
				RulerCounts.StanceChanges + RulerCounts.TreatiesSigned + RulerCounts.FleetTargetChanges, CosmicRulerPolicy::DecisionsOf(RulerCounts),
				Counts.DecisionsAt600, Counts.DecisionsAt6000, Counts.DecisionsAtEnd, Counts.ToldAt600, Counts.ToldAt6000, Counts.RuledT2Tick, Counts.RuledT3Tick);
			// B3 (header section 5): the diplomacy, the conflict and the four stages' time - always told, outside the baselines' keys.
			{
				Counts.BorderPairs[3] = BorderPairsOf(State, StateIndex);
				int32 Friendly = 0;
				int32 Hostile = 0;
				int32 Trade = 0;
				int32 NonAggression = 0;
				for (const FCivilizationState& Civ : State.Civs)
				{
					for (const TPair<FName, FCosmicTie>& Tie : Civ.Ties)
					{
						Friendly += Tie.Value.Stance == CosmicTags::StanceFriendly() ? 1 : 0;
						Hostile += Tie.Value.Stance == CosmicTags::StanceHostile() ? 1 : 0;
						Trade += Tie.Value.Treaties.HasTagExact(CosmicTags::TreatyTrade()) ? 1 : 0;
						NonAggression += Tie.Value.Treaties.HasTagExact(CosmicTags::TreatyNonAggression()) ? 1 : 0;
					}
				}
				double ViewSum = 0.0;
				int32 Views = 0;
				const TArray<TArray<int32>> Borders = CosmicDiplomacy::BorderTableOf(State, StateIndex);
				for (int32 Viewer = 0; Viewer < Borders.Num(); ++Viewer)
				{
					for (const int32 Other : Borders[Viewer])
					{
						ViewSum += CosmicDiplomacy::ViewOf(State, StateIndex, RulesNow, Viewer, Other);
						++Views;
					}
				}
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance diplomacy: border pairs %lld/%lld/%lld/%lld (1000/3000/6000/end), friendly %d hostile %d, treaties trade %d nonaggression %d (signed %d broken %d), relation mean %.3f; ruler stance changes %d, treaties proposed to borderers %d refused %d, to others %d refused %d; first contact card at %lld (friendly %d, its non-aggression at %lld); ruler window: second settled at %lld, first siege target at %lld, end at %lld"),
					Counts.BorderPairs[0], Counts.BorderPairs[1], Counts.BorderPairs[2], Counts.BorderPairs[3], Friendly, Hostile, Trade / 2, NonAggression / 2,
					Counts.TreatiesSignedPairs, Counts.TreatiesBrokenPairs, Views > 0 ? ViewSum / Views : 0.0, RulerCounts.StanceChanges,
					RulerCounts.TreatiesProposedBorder, RulerCounts.TreatiesRefusedBorder, RulerCounts.TreatiesProposedOther, RulerCounts.TreatiesRefusedOther,
					Counts.FirstContactCardTick, Counts.FirstContactFriendly, Counts.FirstContactNonAggressionTick, Counts.SecondSettledTick,
					Counts.FirstSiegeTargetTick, Counts.FirstEndTick);
				int32 TopTaken = 0;
				for (const TPair<FName, int32>& Each : Counts.TakenBy)
				{
					TopTaken = FMath::Max(TopTaken, Each.Value);
				}
				int32 MostLost = 0;
				for (const TPair<FName, int32>& Each : Counts.LostBy)
				{
					MostLost = FMath::Max(MostLost, Each.Value);
				}
				int32 MostInBlock = 0;
				for (const TPair<int64, int32>& Each : Counts.ConflictSev2Blocks)
				{
					MostInBlock = FMath::Max(MostInBlock, Each.Value);
				}
				TArray<FString> DelayKeys;
				Counts.DelayBought.GetKeys(DelayKeys);
				DelayKeys.Sort();
				TArray<FString> Delays;
				for (const FString& Key : DelayKeys)
				{
					Delays.Add(FString::Printf(TEXT("%s=%lld(%d)"), *Key, Counts.DelayBought[Key].Key, Counts.DelayBought[Key].Value));
				}
				// The quarantine's held ticks: the ruled people's edicts that cut the plague's share (read from the data).
				int64 QuarantineHeld = 0;
				for (const TPair<FName, int64>& Each : Counts.EdictHeldTicks)
				{
					const FEdictDef* Def = Sim->GetTables().FindEdict(Each.Key);
					const float* Factor = Def ? Def->HazardSpawnMultipliers.Find(CosmicTags::HazardPlague()) : nullptr;
					QuarantineHeld += (Factor && *Factor < 1.f) ? Each.Value : 0;
				}
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance conflict: sieges begun %d (on ruled %d by end %d), taken %d (%d/%d/%d by tick bands 0-3000/3000-6000/later; from ruled %d by end %d), held %d, lifted %d; hostilities opened %d (%d/%d/%d); AIs hostile toward one people max %d, sieges on one people max %d; taken by top conqueror %d, lost by one people max %d; fleet median [%.3f %.3f] at 3000/6000; delay bought per tag [%s]; attrition %.3f, AI sieges into a higher sphere %d; T3 climbs a taking helped %d (ruled %d); mutual sieges max %d pairs (%lld ticks); plague placed %d (spread-placed %d; ruled %d, spread %d, ruled by end %d), quarantine held %lld; ruled severity-2 conflict lines %d (by end %d, max %d in 600 ticks, last chapter %d)"),
					Counts.SiegesBegun, Counts.SiegesOnRuled, Counts.SiegesOnRuledByEnd, Counts.Taken, Counts.TakenByBand[0], Counts.TakenByBand[1], Counts.TakenByBand[2],
					Counts.TakenFromRuled, Counts.TakenFromRuledByEnd, Counts.Held,
					Counts.Lifted, Counts.HostilitiesOpened[0] + Counts.HostilitiesOpened[1] + Counts.HostilitiesOpened[2], Counts.HostilitiesOpened[0],
					Counts.HostilitiesOpened[1], Counts.HostilitiesOpened[2], Counts.MaxHostileTowardOne, Counts.MaxSiegersOnOne, TopTaken, MostLost,
					Counts.FleetMedian[0], Counts.FleetMedian[1], *FString::Join(Delays, TEXT(" ")), Counts.AttritionLost, Counts.AISiegesIntoSphere,
					Counts.ThreeWorldsByConquest, Counts.ThreeWorldsByConquestRuled, Counts.MaxMutualPairs, Counts.MutualSiegeTicks,
					Counts.Placed.FindRef(CosmicTags::HazardPlague().ToString()), Counts.PlagueSpreadPlaced, Counts.PlaguePlacedRuled, Counts.PlagueSpreadRuled,
					Counts.PlaguePlacedRuledByEnd, QuarantineHeld,
					Counts.ConflictSev2, Counts.ConflictSev2ByEnd, MostInBlock, Counts.ConflictSev2InChapter);
				const FCosmicStageClock& Clock = Sim->GetStageClock();
				const double PerCycle = FPlatformTime::GetSecondsPerCycle64() * 1.0e6;
				const double Ticked = Clock.Ticks > 0 ? static_cast<double>(Clock.Ticks) : 1.0;
				const double TickMean = TickMicros.Num() ? Sum / TickMicros.Num() : 0.0;
				const double Diplomacy0 = Clock.Cycles[0] * PerCycle / Ticked;
				const double Conflict1 = Clock.Cycles[1] * PerCycle / Ticked;
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance stages: diplomacy %.2f us, conflict %.2f us, crisis %.2f us, AI %.2f us (means of %lld ticks); diplomacy + conflict %.1f%% of the tick mean; economy %.2f us, card presentation %.2f us"),
					Diplomacy0, Conflict1, Clock.Cycles[2] * PerCycle / Ticked, Clock.Cycles[3] * PerCycle / Ticked, Clock.Ticks,
					TickMean > 0.0 ? 100.0 * (Diplomacy0 + Conflict1) / TickMean : 0.0, Clock.Cycles[4] * PerCycle / Ticked, Clock.Cycles[5] * PerCycle / Ticked);
			}
			// The wanderer's side (B1): only when the run asked for it (the lines of the runs before B1 stay as they were).
			if (AbdicateAt > 0 || Wander != 0)
			{
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance wanderer: abdicate %lld wander %d; left at %lld, crowned at %lld (%s, tier %d, path %s), journeys %d, scans %d, cards %d, answers %d, reverence peak %.2f, body %d, mind %d"),
					AbdicateAt, Wander, WandererCounts.LeftAt, WandererCounts.CrownedAt, *WandererCounts.CrownedCivID.ToString(),
					WandererCounts.CrownedAt >= 0 ? static_cast<int32>(WandererCounts.CrownedTier) + 1 : 0, WandererCounts.CrownedPath, WandererCounts.Journeys, WandererCounts.Scans,
					WandererCounts.Cards, WandererCounts.Answers, WandererCounts.ReverencePeak, State.Player.Body, State.Player.Mind);
			}
			// B5 ([B-5a] - header section 5.1): the calling line's first measurement - always told, outside the baselines' keys.
			{
				int32 ByEnd = 0;
				int32 Grace = 0;
				int32 Branch = 0;
				for (const FDeathChance& Chance : Calling.Chances)
				{
					ByEnd += Chance.bAfterEnd ? 0 : 1;
					Grace += Chance.bGrace ? 1 : 0;
					Branch += Chance.bBranch ? 1 : 0;
				}
				TMap<FString, int32> Failed;
				for (const TPair<FName, int32>& Each : WandererCounts.FailedByRow)
				{
					Failed.Add(Each.Key.ToString(), Each.Value);
				}
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance calling: lives %d; reign ended first at %lld, last at %lld; first throne after the first end at %lld; death chances %d (by end %d, grace %d, branch side %d) [%s]; wanderer cards failed by row [%s]; #126 ticks the ruled people was held off only by its capital or last system %d in %d episodes (one settled system %d, more %d; by end %d), first at %lld; AI pair-ticks so held %d; #133 window with the realm caps raised at %lld (plain %lld), reasons before it [%s]"),
					State.Calling.Lives.Num(), Calling.FirstReignEndTick, Calling.ReignEndTick, Calling.CrownedAfterTick, Calling.Chances.Num(), ByEnd, Grace, Branch,
					*ChancesText(Calling.Chances), *CountsText(Failed), Calling.BlockedRuledTicks, Calling.BlockedRuledEpisodes, Calling.BlockedRuledOneSystem, Calling.BlockedRuledLater,
					Calling.BlockedRuledByEnd, Calling.FirstBlockedRuled, Calling.BlockedAIPairTicks, Calling.WindowRaisedTick, Counts.FirstSiegeTargetTick, *CountsText(Calling.WindowRaisedReasons));
				// [B-5r] (review play-5): the successors' first thrones, and how many within one lane's time of the birth (the gaps
				// themselves are listed, so any other N reads off them).
				// One lane's time by the one definition every [B-5r] reader uses (CosmicWanderer::LaneTicksOf - review logic-5).
				const int32 NearTicks = CosmicWanderer::LaneTicksOf(RulesNow);
				FString ThroneGaps;
				int32 ThronesNearBirth = 0;
				for (const int64 Gap : Calling.SuccessorThroneGaps)
				{
					ThroneGaps += FString::Printf(TEXT("%s%lld"), ThroneGaps.IsEmpty() ? TEXT("") : TEXT(","), Gap);
					ThronesNearBirth += (Gap >= 0 && Gap <= NearTicks) ? 1 : 0;
				}
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance calling deaths: death asked at %lld (recorded %d); deaths %d (by end %d) by cause [%s] by row [%s]; first death at %lld as %s, %lld after the last reign's end; leaps [%s] (total %lld, cap %d); successions [%s] (tick:candidates:relaxed:lanes from the death:same people:hazard within 2 lanes:birth rung:digest lines); first throne after a succession at %lld (%s); state %s (leap left %lld); interregnum ticks %lld in %.1f ms; end %s; reigns begun while a carded claim stood %d, claims delayed twice %d; deaths on the killing hazard's card %d set apart by the epitaph, %d more the card all the same; at the crowning reverence %d; per death [%s] (tick:a set apart / c the card all the same / - neither:highest reverence of any people before the tick); successor thrones [%s] (ticks from the birth to that life's first throne, -1 none), within %d ticks of the birth %d of %d"),
					DeathAt, Calling.DeathAtRecorded, Calling.Deaths, Calling.DeathsByEnd, *CountsText(Calling.DeathsByCause), *CountsText(Calling.DeathsByRow), Calling.FirstDeathTick,
					Calling.FirstDeathRole, Calling.FirstDeathGap, *Calling.Leaps, State.Calling.LeapTicksTotal, RulesNow.LeapTotalCap, *Calling.Successions, Calling.ThroneAfterSuccession,
					Calling.ThroneAfterSuccessionPath, State.Calling.LifeState == ELifeState::Interregnum ? TEXT("interregnum") : TEXT("embodied"), State.Calling.PendingLeapTicks,
					Calling.LeapTicksRun, Calling.LeapMicros / 1000.0, CosmicEndings::NameOf(Counts.FirstEnd), Calling.ReignsDuringCardedClaims, PressureCounts.DelayedTwice,
					Calling.DeathsSetApart, Calling.DeathsOnTheCard, Calling.DeathsAtCrowningReverence, *Calling.DeathAnswers, *ThroneGaps, NearTicks, ThronesNearBirth,
					Calling.SuccessorThroneGaps.Num());
				// [B-5d1] (header section 5.1, 5.2 bundle 1; docs/70 section 8.4): the two books at the run's end, by the presentation's
				// own pure function - its input (every Log.Record and Log.Claim entry of the chronicle and the log) and its chapter.
				const FCosmicTwoBooks Books = CosmicChronicle::BuildTwoBooks(State);
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance calling books: entries record %d claim %d; rows %d (cap %d); part one %d of %d (more %d); part two %d lines in %d groups of %d (more %d)"),
					EntriesOf(State, CosmicTags::LogRecord()), EntriesOf(State, CosmicTags::LogClaim()), Books.NumRows(), CosmicChronicle::MaxBookLines, Books.Recorded.Num(),
					Books.RecordedCandidates, Books.bRecordedMore ? 1 : 0, ClaimedLinesOf(Books), Books.Claimed.Num(), Books.ClaimedCandidates, Books.bClaimedMore ? 1 : 0);
				if (SaveAt >= 1 && SaveAt <= Ticks && bSaveShape && !bSavedAt)
				{
					UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -SaveAt %lld wrote no slot"), SaveAt);
				}
			}
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance end: first %s at %lld; clock at %lld, deposed at %lld; chronicle life %d shown %d of %d, more %d, ranks %d/%d/%d/%d"),
				CosmicEndings::NameOf(Counts.FirstEnd), Counts.FirstEndTick, Counts.ClockTick, Counts.DeposedTick, Counts.ChapterLife,
				Counts.ChapterShown, Counts.ChapterCandidates, Counts.ChapterMore, Counts.ChapterRanks[0], Counts.ChapterRanks[1], Counts.ChapterRanks[2], Counts.ChapterRanks[3]);
			// B4 (header section 5): the first measurement - always told, outside the baselines' keys.
			{
				const FCivilizationState* RuledAtEnd = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
				Spirit.MetNear[2] = MetPeoplesNearFallen(State, StateIndex, Sim->GetTables(), RuledAtEnd ? RuledAtEnd->CivID : NAME_None);
				Spirit.SphereAt[3] = SphereSize(State);
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance spirit: ruled drain P max [%.3f %.3f %.3f %.3f %.3f] (0-1000/1000-2000/2000-3000/3000-6000/later), drained ticks %lld (by end %lld), stability drained %.3f (by end %.3f); first next to a fallen sphere at %lld, first inside at %lld; claims on ruled %d (by end %d), joined %d (by end %d), stilled %d (by relief %d); claim sources on ruled settled systems within claim reach: expired %d (by end %d), answered by a project %d, resolved by an edict %d, refused by the remnant law %d; trigger first holds: claim %d (slot taken %d), pressure 0.9 %d (slot taken %d, from a higher source %d); vigil gate when the claim trigger first holds with the slot free: open %d, held %d, slots %d, cost %d, other %d (row %s); met peoples next to or inside a sphere %d/%d/%d at 600/1200/end; known joins (known system, met owner) %d (by end %d; at T1-2 %d, at T3+ %d); sphere %d/%d/%d/%d at 1000/3000/6000/end; same-tick joins max %d"),
					Spirit.DrainMax[0], Spirit.DrainMax[1], Spirit.DrainMax[2], Spirit.DrainMax[3], Spirit.DrainMax[4], Spirit.DrainedTicks, Spirit.DrainedTicksByEnd,
					Spirit.Drained, Spirit.DrainedByEnd, Spirit.FirstNextTo, Spirit.FirstInside, Spirit.ClaimsOnRuled, Spirit.ClaimsOnRuledByEnd, Spirit.Joined,
					Spirit.JoinedByEnd, Spirit.Stilled, Spirit.StilledByRelief, Spirit.SourcesExpired, Spirit.SourcesExpiredByEnd, Spirit.SourcesAnswered,
					Spirit.SourcesByEdict, Spirit.SourcesRemnant, Spirit.TriggerClaim, Spirit.TriggerClaimTaken, Spirit.TriggerPressure, Spirit.TriggerPressureTaken,
					Spirit.TriggerPressureHigher, Spirit.VigilOpen, Spirit.VigilHeld, Spirit.VigilSlots, Spirit.VigilCost, Spirit.VigilOther,
					VigilRow.IsNone() ? TEXT("-") : *VigilRow.ToString(), Spirit.MetNear[0], Spirit.MetNear[1], Spirit.MetNear[2], Spirit.KnownJoins,
					Spirit.KnownJoinsByEnd, Spirit.KnownJoinsLow, Spirit.KnownJoinsHigh, Spirit.SphereAt[0], Spirit.SphereAt[1], Spirit.SphereAt[2], Spirit.SphereAt[3],
					Spirit.SameTickJoinsMax);
				// B4 (header section 5): the pressure line - its head (the policy's values and the guard's cells) here, the
				// cards' and the voice's cells with their commits. Its "by end" cells count up to the record's tick: since B6a the
				// clock's, no longer the ruled third rung's - the ending line splits B4's counts at that rung (B6a header 5.1).
				TMap<FString, int32> Enactments;
				for (const TPair<FName, int32>& Each : RulerCounts.CardEnactments)
				{
					Enactments.Add(Each.Key.ToString(), Each.Value);
				}
				const int32* Tiers = RulerCounts.PressureCardsByTier;
				const int32* Kinds = RulerCounts.PressureAnswers;
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance pressure: policy pressure %s guard %d leave near sphere %d; guarded crises %d by tag [%s] (by end %d), delay bought %lld by tag [%s], ends of guarded crises: answered %d, expired %d, resolved by an edict %d; cards came %d (by end %d; first at %lld; claim card %d, measured card %d; by tier [T1 %d, T2 %d, T3 %d]), claim card lost to a waiting card %d (same tick %d; to [%s]), standing cards lost in their tick [%s], answers by kind [intercede %d, discern %d, watch %d, edict %d, resist %d] (by end %d), choices greyed by reason [%s]; edicts enacted by cards [%s] (revoked by the script %d); resist ticks bought %lld, fleet spent %.2f, claims delayed twice %d, fleet regrown after a resistance %d (median %lld ticks, still short %d); discern insight min %.1f; first-person lines told %d (by end %d, untold %d), sign lines told %d (by end %d), max in 600 ticks %d/%d; first sign at %lld, first claim line at %lld; vigil eased %d (by end %d); theodicy (vigil held from the card to the claim's end, no relief): by remaining band [<=300 %d eased %d, 301-400 %d eased %d, 401-550 %d eased %d, more %d eased %d]; claim ends on ruled [joined %d, eased %d, relief %d, other %d, expired unjoined %d] (no claim card %d); under watch %d, joined %d; fleet worn lines %d"),
					CosmicRulerPolicy::PressurePolicyName(RulerPolicy.Pressure), RulerPolicy.Guard, RulerPolicy.bLeaveNearSphere ? 1 : 0, PressureCounts.Guarded,
					*TagCountsText(PressureCounts.GuardedByTag), PressureCounts.GuardedByEnd, PressureCounts.DelayBought, *TagTicksText(PressureCounts.DelayByTag),
					PressureCounts.GuardedAnswered, PressureCounts.GuardedExpired, PressureCounts.GuardedByEdict,
					RulerCounts.PressureCards, RulerCounts.PressureCardsByEnd, RulerCounts.FirstPressureCardTick, RulerCounts.ClaimCards, RulerCounts.MeasuredCards, Tiers[0], Tiers[1], Tiers[2],
					PressureCounts.ClaimLostWait, PressureCounts.ClaimLostSame, *TagCountsText(PressureCounts.ClaimLostTo), *TagCountsText(PressureCounts.StandingLost), Kinds[0], Kinds[1], Kinds[2], Kinds[3], Kinds[4], RulerCounts.PressureAnswersByEnd,
					*TagCountsText(RulerCounts.PressureGreyed), *TagCountsText(Enactments), RulerCounts.CardEdictsRevoked, PressureCounts.ResistBought,
					PressureCounts.FleetSpent, PressureCounts.DelayedTwice, PressureCounts.Regrown.Num(), MedianOf(PressureCounts.Regrown), PressureCounts.Regrowing.Num(),
					RulerCounts.DiscernInsightMin,
					PressureCounts.VoiceTold, PressureCounts.VoiceToldByEnd,
					PressureCounts.VoiceUntold, PressureCounts.SignTold, PressureCounts.SignToldByEnd, MostOf(PressureCounts.VoiceBlocks), MostOf(PressureCounts.SignBlocks),
					PressureCounts.FirstSign, PressureCounts.FirstVoice, PressureCounts.Eased, PressureCounts.EasedByEnd,
					PressureCounts.Band[0], PressureCounts.BandEased[0], PressureCounts.Band[1], PressureCounts.BandEased[1], PressureCounts.Band[2], PressureCounts.BandEased[2],
					PressureCounts.Band[3], PressureCounts.BandEased[3], PressureCounts.Ends[0], PressureCounts.Ends[1], PressureCounts.Ends[2], PressureCounts.Ends[3],
					PressureCounts.Ends[4], PressureCounts.NoCard, PressureCounts.Watched, PressureCounts.WatchedJoined, PressureCounts.FleetWorn);
			}
			// B6a [B-6a3c] (header section 5.1, notes section 2.1): the ending line - always told, outside the baselines' keys.
			{
				const FCosmicEndingRecord& Record = State.Ending;
				const bool bRecord = Record.Kind.IsValid();
				const bool bRuledPeople = !Record.CivID.IsNone()
					&& State.Calling.Lives.ContainsByPredicate([&Record](const FLifeRecord& Life) { return Life.RuledCivIDs.Contains(Record.CivID); });
				// The season: from the ruled third rung to the record (-1 without either, or with the rung after the record).
				const int64 Season = (bRecord && Ending.SeasonStart >= 0 && Ending.SeasonStart <= Record.Tick) ? Record.Tick - Ending.SeasonStart : -1;
				// Its decisions: those of the boundaries from the season's start up to the record's (the game takes none at that
				// boundary), counted per full window of SeasonWindowTicks from the start (one window for a shorter season; the tail
				// past the last full window counts in the sum only); the longest stretch without one, the start and the record
				// included as its ends.
				int32 SeasonDecisions = 0;
				TArray<int64> PerWindow;
				int64 LongestGap = -1;
				if (Season > 0)
				{
					PerWindow.Init(0, static_cast<int32>(FMath::Max<int64>(1, Season / SeasonWindowTicks)));
					int64 Last = Ending.SeasonStart;
					LongestGap = 0;
					for (const TPair<int64, int32>& Each : Ending.Decisions)
					{
						if (Each.Key < Ending.SeasonStart || Each.Key >= Record.Tick)
						{
							continue;
						}
						SeasonDecisions += Each.Value;
						const int64 Window = (Each.Key - Ending.SeasonStart) / SeasonWindowTicks;
						if (Window < PerWindow.Num())
						{
							PerWindow[static_cast<int32>(Window)] += Each.Value;
						}
						LongestGap = FMath::Max(LongestGap, Each.Key - Last);
						Last = Each.Key;
					}
					LongestGap = FMath::Max(LongestGap, Record.Tick - Last);
				}
				int64 LeastWindow = -1;
				for (const int64 Each : PerWindow)
				{
					LeastWindow = (LeastWindow < 0 || Each < LeastWindow) ? Each : LeastWindow;
				}
				// The research fields every people holds, and the ruled people the line reads them for (the season's people,
				// else the first reign's).
				TArray<FName> Fields;
				for (const FCivilizationState& Civ : State.Civs)
				{
					for (const TPair<FName, float>& Field : Civ.Research)
					{
						Fields.AddUnique(Field.Key);
					}
				}
				Fields.Sort(FNameLexicalLess());
				const FName ResearchRuled = Ending.SeasonCivID.IsNone() ? FirstRuled : Ending.SeasonCivID;
				const bool bAwareByEnd = Ending.AwareTick >= 0 && (!bRecord || Ending.AwareTick <= Record.Tick);
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance ending: record %s at %lld (people %s, a people some life ruled %d, no vessel %d); ruled T3 at %lld (%s), season %lld; season decisions %d in %d windows of %lld (median %lld, least %lld), longest gap %lld; first AI T3 at %lld (%s, feat %s); research 1.0 ruled %s [%s], first AI %s [%s]; aware at %lld (%s, by the record %d, vessel left %d); by the record told: first-person claim lines %d, awareness lines %d (first at %lld); season claims on ruled settled systems %d (first at %lld); by phase before T3/season/after the record: #132 guarded [%s] / [%s] / [%s] ruled attrition %.3f/%.3f/%.3f, #142 band 401-550 %s joined %s, #135 claim trigger %s lost to a waiting card %s same tick %s keys [%s], #145 sieges on ruled while its fleet regrows %s; ruled low stability first at %lld; pressure cards %s stock insight least:median %s, influence least:median %s; petition_exalt at %lld answer %d, pre-count [%s]"),
					CosmicEndings::NameOf(Record.Kind), bRecord ? Record.Tick : static_cast<int64>(-1), Record.CivID.IsNone() ? TEXT("-") : *Record.CivID.ToString(), bRuledPeople ? 1 : 0,
					(bRecord && Record.bNoVessel) ? 1 : 0, Ending.SeasonStart, Ending.SeasonCivID.IsNone() ? TEXT("-") : *Ending.SeasonCivID.ToString(), Season,
					SeasonDecisions, PerWindow.Num(), SeasonWindowTicks, MedianOf(PerWindow), LeastWindow, LongestGap,
					Ending.FirstAIT3Tick, Ending.FirstAIT3CivID.IsNone() ? TEXT("-") : *Ending.FirstAIT3CivID.ToString(), Ending.FirstAIT3Feat.IsNone() ? TEXT("-") : *Ending.FirstAIT3Feat.ToString(),
					ResearchRuled.IsNone() ? TEXT("-") : *ResearchRuled.ToString(), *ResearchFullText(Ending, ResearchRuled, Fields),
					Ending.FirstAIT3CivID.IsNone() ? TEXT("-") : *Ending.FirstAIT3CivID.ToString(), *ResearchFullText(Ending, Ending.FirstAIT3CivID, Fields),
					Ending.AwareTick, Ending.AwareTranscendentID.IsNone() ? TEXT("-") : *Ending.AwareTranscendentID.ToString(), bAwareByEnd ? 1 : 0, Ending.bAwareVesselLeft ? 1 : 0,
					PressureCounts.VoiceToldByEnd, Ending.AwarenessToldByEnd, Ending.FirstAwarenessTold, Ending.SeasonClaims, Ending.FirstSeasonClaim,
					*GuardedPhaseText(Ending.Guarded[0]), *GuardedPhaseText(Ending.Guarded[1]), *GuardedPhaseText(Ending.Guarded[2]),
					Ending.RuledAttrition[0], Ending.RuledAttrition[1], Ending.RuledAttrition[2],
					*PhasesText(Ending.Band142), *PhasesText(Ending.Band142Joined), *PhasesText(Ending.ClaimTrigger), *PhasesText(Ending.ClaimLostWait),
					*PhasesText(Ending.ClaimLostSame), *FString::Join(Ending.ClaimTriggerKeys, TEXT(" ")), *PhasesText(Ending.SiegesRegrowing), Ending.LowStabilityTick,
					*FString::Printf(TEXT("%d/%d/%d"), Ending.CardInsight[0].Num(), Ending.CardInsight[1].Num(), Ending.CardInsight[2].Num()),
					*StockPhasesText(Ending.CardInsight), *StockPhasesText(Ending.CardInfluence), Ending.ExaltTick, Ending.ExaltChoice, *FString::Join(Ending.ExaltPreCount, TEXT(" ")));
			}
			if (ExpansionSum != Counts.ExpansionPairs)
			{
				UE_LOG(LogCosmic, Error, TEXT("CosmicBalance expansion: the counts add up to %lld, not %lld"), ExpansionSum, Counts.ExpansionPairs);
				Result = 1;
			}
		}
		else
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: could not write %s or %s"), *OutPath, *RowsPath);
			Result = 1;
		}
		Save->RemoveFromRoot();
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: no simulation or role subsystem, or -Rules, -EdictRow, -Diplomacy, -PlayerPolicy, -Guard, -LeaveNearSphere, -WithoutPressure, the death's arguments or -ExaltAsBefore refused: nothing run"));
		Result = 1;
	}

	// InitializeStandalone made a world context and a dummy world; Shutdown leaves both behind, and a world destroyed by
	// the exit-time collection alone trips an ensure in its subsystems.
	UWorld* World = GameInstance->GetWorld();
	GameInstance->Shutdown();
	if (World)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	GameInstance->RemoveFromRoot();
	return Result;
}
