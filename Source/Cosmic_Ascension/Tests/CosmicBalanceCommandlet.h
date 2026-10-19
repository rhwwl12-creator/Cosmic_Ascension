// Cosmic Ascension - balance commandlet. Design: guideline.md section 9.3.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CosmicBalanceCommandlet.generated.h"

/**
 * Headless balance run: generates a galaxy and runs it one tick at a time (RunHeadless(1), timed), reading the state
 * after every tick. Writes one CSV row per civilization every 100 ticks (resources, Tier, Stability, hazard count, and
 * the galaxy columns of section 9.3: KnownSystems, MetCivs, WildSystems, FirstContactTick, T4PlusCount - the beings of
 * the fourth rung as section 4.2 counts them -, FirstClaimTick, ToldLines, TickMicros - the mean of the sample's ticks -,
 * SaveBytes - the state serialized as the save serializes it, every 1,000 ticks, empty otherwise -, LogLen,
 * ChronicleLen) and, last, Expansion - why that people starts no settlement at the sample (CosmicMegaprojects::
 * PeopleSettleBlockOf by name, NoFoothold for NoLane when it owns no settled system, SettlingStalled for Settling when
 * its settlement paid nothing in the tick just run - CosmicMegaprojects::PaidNothing; empty for the people the player
 * rules), a second CSV named after -Out with "_rows" before ".csv" (-Out=balance.csv writes balance_rows.csv) with
 * the lines of every 100-tick sample by SourceRow, and five log lines: the hashes ("CosmicBalance: seed ... hash ...
 * world ..."), a summary (claims placed, ended and joined, the first claim and the first join, the first tick some people
 * holds two settled systems and the claims placed after it, the sphere, wild systems, the tick the vessel was deposed,
 * tick micros mean/p99/max, the bytes and the time of the copy and serialization of the state - not the file write),
 * the claims that ended (joined or stilled, with an answer running or not and their owners' Influence) with the sphere
 * of every instance, the hazards placed, answered, expired and ended by tag (answered: its answer completed in the
 * tick it ended; expired: not answered, on its last count; a hazard placed and ended inside one tick is not counted),
 * and the expansion reasons ("CosmicBalance expansion: <pairs> (sample, people) pairs ...; <reason> <count> ..." - every
 * sample, the tick-0 sample included; the pairs are the peoples the player does not rule, counted apart at every sample,
 * and an error line says so when the reasons do not add up to them).
 * It reads the state only: the world is that of RunHeadless(Ticks) - as long as there are no endings (section 4.4: once
 * an ending stops a run, a later RunHeadless must simulate nothing for that to stay true).
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
