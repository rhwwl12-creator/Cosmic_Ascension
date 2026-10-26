// Cosmic Ascension - balance commandlet.

#include "Tests/CosmicBalanceCommandlet.h"

#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
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
#include "Compression/OodleDataCompression.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "World/CosmicRulerPolicy.h"
#include "World/CosmicWandererPolicy.h"

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
		// 6,000 ticks, the sum at the first provisional end (the decisions the game takes - none after it; -1 without an end);
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
		// The provisional end (MVP item 4), looked at after every tick and before the policy's input - nothing stops at it: the
		// first cause and its tick, the first ticks of each cause, and the last chapter of the chronicle at the first end.
		ECosmicProvisionalEnd FirstEnd = ECosmicProvisionalEnd::None;
		int64 FirstEndTick = -1;
		int64 RungTick = -1;
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
	// voice could be heard. "By end" counts up to and including the tick of the first provisional end. Read from the state
	// only; the drain's P from UGalaxySimulationSubsystem::ComputeDrainPressureOf (no copy of its rule here).
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
	};

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
	// The ruler's policy (section 9.3; Phase 5 adds -PlayerPolicy): -Answer=1 answers every card the tick it comes
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
	FParse::Value(*Params, TEXT("Rules="), RulesText, false);
	FParse::Value(*Params, TEXT("Out="), Out);
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
	if (Sim && Role && bRules && bEdictRow && bDiplomacy && bPressure)
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
		UCosmicSaveGame* Save = NewObject<UCosmicSaveGame>();
		Save->AddToRoot();

		FRunCounts Counts;
		FSpiritCounts Spirit;
		FPressureCounts PressureCounts;
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
			PressurePolicy, GuardArg, LeaveNearSphere != 0 };
		FCosmicRulerPolicyCounts RulerCounts;
		const FCosmicWandererPolicy WandererPolicy{ AbdicateAt, Wander != 0 };
		FCosmicWandererPolicyCounts WandererCounts;
		// The people of the first reign: the ruler line's rung ticks are that people's (B1 - a later throne is the wanderer
		// line's).
		const FName FirstRuled = (!State.Calling.Lives.IsEmpty() && !State.Calling.Lives[0].RuledCivIDs.IsEmpty()) ? State.Calling.Lives[0].RuledCivIDs[0] : NAME_None;
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
						Counts.AttritionLost += FMath::Min(static_cast<double>(Civ.FleetPower), static_cast<double>(RulesNow.PressureFleetAttrition) * Pressure);
					}
				}
			}
			const EPlayerRole RoleBefore = State.Player.Role;
			const int64 TickBefore = State.Tick;
			const int32 SphereBefore = SphereSize(State);
			const FName CardBefore = State.Knowing.PendingPetitionRow;

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
			const uint64 Start = FPlatformTime::Cycles64();
			Sim->RunHeadless(1);
			const double Micros = static_cast<double>(FPlatformTime::Cycles64() - Start) * FPlatformTime::GetSecondsPerCycle64() * 1.0e6;
			TickMicros.Add(Micros);
			SampleMicros += Micros;
			++SampleTicks;

			// The provisional end, read as the presentation reads it at this tick boundary (before the ruler's input of it).
			{
				const ECosmicProvisionalEnd End = CosmicEndings::ProvisionalEndOf(State, Sim->GetRules());
				const int32 EndTick = Sim->GetRules().ProvisionalEndTick;
				Counts.RungTick = (Counts.RungTick < 0 && End == ECosmicProvisionalEnd::Rung) ? State.Tick : Counts.RungTick;
				Counts.ClockTick = (Counts.ClockTick < 0 && EndTick > 0 && State.Tick >= EndTick) ? State.Tick : Counts.ClockTick;
				if (Counts.FirstEnd == ECosmicProvisionalEnd::None && End != ECosmicProvisionalEnd::None)
				{
					Counts.FirstEnd = End;
					Counts.FirstEndTick = State.Tick;
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

			// B3: the first contact card came (read before the policy answers it at this boundary), and the stance toward that
			// people while it waits (the answer's change is the card's).
			if (Counts.FirstContactCardTick < 0 && State.Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact)
			{
				Counts.FirstContactCardTick = State.Tick;
			}
			const bool bContactWaits = State.Knowing.PendingPetitionRow == CosmicRows::PetitionFirstContact;
			const FCivilizationState* RuledAtBoundary = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; });
			const bool bContactFriendlyBefore = RuledAtBoundary && CosmicDiplomacy::StanceOf(*RuledAtBoundary, State.Knowing.FirstContactCivID) == CosmicTags::StanceFriendly();
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
							Spirit.TriggerClaimTaken += bSlotTaken ? 1 : 0;
							if (!bSlotTaken)
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
			// was placed with, less the row's countdown and the owner's edicts' bonus).
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
						- CosmicEdicts::CountdownBonusOf(*Owner, Sim->GetTables(), System.ActiveHazard);
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
			CosmicRulerPolicy::Act(RulerPolicy, *Sim, *Role, CardBefore, RulerCounts);
			CosmicWandererPolicy::Act(WandererPolicy, *Sim, *Role, CardBefore, WandererCounts);
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
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance end: first %s at %lld; rung at %lld, clock at %lld, deposed at %lld; chronicle life %d shown %d of %d, more %d, ranks %d/%d/%d/%d"),
				CosmicEndings::NameOf(Counts.FirstEnd), Counts.FirstEndTick, Counts.RungTick, Counts.ClockTick, Counts.DeposedTick, Counts.ChapterLife,
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
				// cards' and the voice's cells with their commits.
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance pressure: policy pressure %s guard %d leave near sphere %d; guarded crises %d by tag [%s] (by end %d), delay bought %lld by tag [%s], ends of guarded crises: answered %d, expired %d, resolved by an edict %d"),
					CosmicRulerPolicy::PressurePolicyName(RulerPolicy.Pressure), RulerPolicy.Guard, RulerPolicy.bLeaveNearSphere ? 1 : 0, PressureCounts.Guarded,
					*TagCountsText(PressureCounts.GuardedByTag), PressureCounts.GuardedByEnd, PressureCounts.DelayBought, *TagTicksText(PressureCounts.DelayByTag),
					PressureCounts.GuardedAnswered, PressureCounts.GuardedExpired, PressureCounts.GuardedByEdict);
			}
			// The rung end and the ruled people's third rung are the same tick, but for a deposition in that very tick (the
			// feats are judged before the tenth stage deposes; afterwards no people is ruled). A measure, not a gate: Result
			// stays, and the line has a prefix of its own so that a tool taking the last "end:" line never takes it.
			const bool bSameTickDeposition = Counts.RuledT3Tick < 0 && Counts.DeposedTick >= 0 && Counts.DeposedTick == Counts.RungTick;
			if (Counts.RungTick != Counts.RuledT3Tick && !bSameTickDeposition)
			{
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance end check: the rung end at %lld is not the ruled people's third rung at %lld"), Counts.RungTick, Counts.RuledT3Tick);
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
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: no simulation or role subsystem, or -Rules, -EdictRow, -Diplomacy, -PlayerPolicy, -Guard or -LeaveNearSphere refused: nothing run"));
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
