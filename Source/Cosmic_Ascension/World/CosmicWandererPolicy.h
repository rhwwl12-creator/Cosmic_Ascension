// Cosmic Ascension - World layer: the wanderer policy a script plays between ticks (B1).

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"

class UGalaxySimulationSubsystem;
class UPlayerRoleSubsystem;

// The wanderer's policy of the balance commandlet (-Abdicate=<tick>, -Wander=1 - docs/60_DECISIONS/headers/
// B1_wanderer_loop.md section 5) as functions the commandlet and the tests share, as the ruler policy is
// (World/CosmicRulerPolicy). A player's script: it reads the world and knowing as a player reads the screen.
struct FCosmicWandererPolicy
{
	// The tick at whose boundary the ruler abdicates (UPlayerRoleSubsystem::Abdicate); 0 or below: never.
	int64 AbdicateAt = 0;
	// Whether the wanderer walks: answers its cards, takes a throne the gate opens, scans on each arrival and travels.
	bool bWander = false;
	// B5 (header section 5.1 - measurement only): the provisional end is not counted - the policy goes on after it.
	bool bPastEnd = false;
};

// What the policy saw and did so far in a run (the commandlet's wanderer line).
struct FCosmicWandererPolicyCounts
{
	// The tick the vessel first walked (an abdication or a deposition) and the tick it first took a throne again; -1: not yet.
	int64 LeftAt = -1;
	int64 CrownedAt = -1;
	// The people that crowned the vessel, its rung then (0-based, EAscensionTier), and the way: "reverence", "vacuum" or
	// "return" (the reverence path wins when both hold).
	FName CrownedCivID;
	EAscensionTier CrownedTier = EAscensionTier::Tier1_Planetary;
	const TCHAR* CrownedPath = TEXT("none");
	int32 Journeys = 0;
	int32 Scans = 0;
	// The wanderer's cards seen to come (at the tick boundary and after each answer), and its accepted answers (a
	// withdrawn card not counted).
	int32 Cards = 0;
	int32 Answers = 0;
	// B5 ([B-5a] - the calling line, open question #151's material): the answers that failed, by card row.
	TMap<FName, int32> FailedByRow;
	// B5: the last throne taken, and its way (the calling line's throne after a succession).
	int64 LastCrownedAt = -1;
	const TCHAR* LastCrownedPath = TEXT("none");
	// The highest reverence any people held for the vessel while it walked.
	float ReverencePeak = 0.f;
	// The system the vessel last left (the next journey does not go straight back to it unless it must), and the system
	// of the last arrival it scanned.
	FName LastLeft;
	FName ScannedAt;
	// A provisional end has been read at a boundary: no input from then on (the game takes none after it).
	bool bEnded = false;
};

namespace CosmicWandererPolicy
{
	// At the boundary of tick AbdicateAt, before the ruler policy: the ruler abdicates (the command records it; the tenth
	// stage of the next tick leaves). Reads the provisional end first: no input after it.
	void AbdicateIfDue(const FCosmicWandererPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role,
		FCosmicWandererPolicyCounts& Counts);

	// One tick boundary, after the ruler policy (B1 header section 5): counts what a wanderer saw (its first leaving, the
	// cards that came, the reverence held), and with bWander, while the run has not ended: (1) answers a waiting card -
	// its choices in turn, from the count of its answers (a wanderer pays nothing); (2) takes the throne of the people of
	// the system it stands in when the gate says None (CosmicWanderer::CrownBlockOf with the meeting read from knowing);
	// (3) scans once on each arrival; (4) not travelling, sets off to the first lane neighbour TravelTo takes with a crisis
	// standing (in Lanes order, not the system it just left unless that is the only one open), else the (journeys mod open
	// neighbours)th. CardBefore is the pending card before the tick.
	void Act(const FCosmicWandererPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
		FCosmicWandererPolicyCounts& Counts);
}
