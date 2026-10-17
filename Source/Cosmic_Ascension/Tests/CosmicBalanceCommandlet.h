// Cosmic Ascension - balance commandlet. Design: guideline.md section 9.3.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CosmicBalanceCommandlet.generated.h"

/**
 * Headless balance run: generates a galaxy and runs it one tick at a time (RunHeadless(1), timed), reading the state
 * after every tick. Writes one CSV row per civilization every 100 ticks (resources, Tier, Stability, hazard count, and
 * the galaxy columns of section 9.3: KnownSystems, MetCivs, WildSystems, FirstContactTick, T4PlusCount, FirstClaimTick,
 * ToldLines, TickMicros - the mean of the sample's ticks -, SaveBytes - every 1,000 ticks, empty otherwise -, LogLen,
 * ChronicleLen), a second CSV <Out>_rows.csv with the lines of every 100-tick sample by SourceRow, and three log lines:
 * the hashes ("CosmicBalance: seed ... hash ... world ..."), a summary (claims placed, ended and joined, the first claim
 * and the first join, the sphere, wild systems, the tick the vessel was deposed, tick micros mean/p99/max, save bytes)
 * and the hazards placed, expired and ended by tag. It reads the state only: the world is that of RunHeadless(Ticks).
 *
 *   UnrealEditor-Cmd Cosmic_Ascension.uproject -run=CosmicBalance -Seed=42 -Ticks=20000 -Out=balance.csv
 *
 * The numbers of systems and peoples are those of the home galaxy row (0, the default of -Systems= and -Civs=); give
 * them only to measure another size. -TellAll=0 tells with the scope of a game (default 1: every line stored).
 * -Out is relative to Saved/Balance unless it is an absolute path. A commandlet has no GameInstance, so Main creates
 * one to get the subsystems. The policy, death and archetype arguments of section 9.3 arrive with Phase 5.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicBalanceCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
