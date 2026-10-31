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
 * SaveBytes - the slot the game would write (CosmicSaveSlot::Encode: the engine bytes packed behind the header), every 1,000
 * ticks, empty otherwise -, LogLen, ChronicleLen, SaveRawBytes - the engine bytes before packing, on the same samples -,
 * ChronicleLetGo - FGalaxyState.ChronicleEviction.Count), Expansion - why that people starts no settlement at the sample (CosmicMegaprojects::
 * PeopleSettleBlockOf by name, NoFoothold for NoLane when it owns no settled system, SettlingStalled for Settling when
 * its settlement paid nothing in the tick just run - CosmicMegaprojects::PaidNothing; empty for the people the player
 * rules) - and, last, B3's four: FleetPower, Hostile (the people's own hostile stances), Treaties (the treaties it holds)
 * and BorderPeoples (the peoples its systems border - CosmicDiplomacy::BorderTableOf), a second CSV named after -Out with "_rows" before ".csv" (-Out=balance.csv writes balance_rows.csv) with
 * the lines of every 100-tick sample by SourceRow, and eight log lines: the hashes ("CosmicBalance: seed ... hash ...
 * world ..."), a summary (claims placed, ended and joined, the first claim and the first join, the first tick some people
 * holds two settled systems and the claims placed after it, the sphere, wild systems, the tick the vessel was deposed,
 * tick micros mean/p99/max, the slot bytes, the engine bytes before packing and the time of the copy, serialization and packing
 * of the state - not the file write -, the chronicle entries the cap may never let go, "kept for good", and the entries it let
 * go, "let go"), the packing
 * levels ("CosmicBalance pack: raw <n>; superfast <n> in <ms> ms; veryfast ...; fast ..." - the last state's engine bytes at three
 * Kraken levels apart),
 * the claims that ended (joined or stilled, with an answer running or not and their owners' Influence) with the sphere
 * of every instance, the hazards placed, answered, expired and ended by tag (answered: its answer completed in the
 * tick it ended; expired: not answered, on its last count; a hazard placed and ended inside one tick is not counted),
 * the expansion reasons ("CosmicBalance expansion: <pairs> (sample, people) pairs ...; <reason> <count> ..." - every
 * sample, the tick-0 sample included; the pairs are the peoples the player does not rule, counted apart at every sample,
 * and an error line says so when the reasons do not add up to them), and the ruler's side ("CosmicBalance ruler: ..." -
 * the policy's arguments, the cards that came, the answers taken, the projects started, the focus changes taken, the
 * edict and diplomacy commands taken ("edict changes", "diplomacy changes"), the decisions - answers taken, a withdrawn
 * card not counted, plus projects started, focus changes, edict commands and diplomacy commands - by tick 600
 * and 6,000, at the first provisional end ("by end" - the decisions the game takes; -1 without an end) and in all, the told
 * lines by tick 600 and 6,000, and the ticks at which the ruled people reached the second and the third rung), and the provisional end ("CosmicBalance end: first <cause>
 * at <tick>; clock at, deposed at; chronicle life <n> shown <lines> of <candidates>, more <0|1>" - looked at after
 * every tick, nothing stops at it - and the uncapped chapter's lines by the cap's four ranks, "ranks a/b/c/d"; the third
 * rung no longer ends a run (B6a [B-6a3a]), so the clock is the only cause, and the old "CosmicBalance end check:" line,
 * which compared the rung end with the ruled people's third rung, is gone - [B-6a3b] brings a new one).
 * The ruler's policy (World/CosmicRulerPolicy, shared with the hash run of a game process - MVP item 9), between ticks
 * after each tick and before the tick's counts: -Answer=1 answers every card the tick it comes (the first choice the
 * stockpile can pay, from choice (answers so far mod choices) on), -Build=1 presses the
 * project key (UPlayerRoleSubsystem::StartNextMegaproject) until it refuses (with -Diplomacy=2 never into the last realm
 * slot - left to a conquest), -Focus=1 focuses the next rung's research
 * while the run has not ended (UPlayerRoleSubsystem::SetResearchFocus with the field of the first research condition of
 * CosmicNextRung::AllOf that is not met, the even focus when none is - MVP item 7), -Edict=1 holds edicts (B2),
 * -Abdicate=<tick> and -Wander=1 the wanderer's policy (B1), -Diplomacy=1 the peace diplomacy and -Diplomacy=2 the attack
 * one (B3, CosmicRulerPolicy::ActDiplomacy - the baselines play 1; with -Answer=1 the first contact card reaches out under
 * either). Three
 * more lines say B3's side, with or without them: "CosmicBalance diplomacy:" (border pairs, stances, treaties, the first contact card),
 * "CosmicBalance conflict:" (sieges begun, taken, held and lifted, hostilities, fleets, the third rungs a taking helped,
 * mutual sieges, the plague's spread and the ruled people's plague) and
 * "CosmicBalance stages:" (the mean time of the diplomacy, conflict, crisis and AI stages - FCosmicStageClock, on for the
 * run only). -Rules=Name:Value;Name:Value replaces
 * fields of the DT_Rules row for the run (read as the data importer reads them; a field it cannot set stops the run).
 * Without them (all off by default) it reads the state only: the world is that of RunHeadless(Ticks) - as long as there
 * are no endings (section 4.4: once an ending stops a run, a later RunHeadless must simulate nothing for that to stay
 * true). The MVP's provisional end belongs to the presentation and stops nothing.
 *
 *   UnrealEditor-Cmd Cosmic_Ascension.uproject -run=CosmicBalance -Seed=42 -Ticks=20000 -Out=balance.csv
 *
 * The numbers of systems and peoples are those of the home galaxy row (0, the default of -Systems= and -Civs=); give
 * them only to measure another size. -TellAll=0 tells with the scope of a game (default 1: every line stored).
 * -Out is relative to Saved/Balance unless it is an absolute path. A commandlet has no GameInstance, so Main creates
 * one to get the subsystems. The death and archetype arguments of section 9.3 and the named player policies arrive with
 * Phase 5.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicBalanceCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
