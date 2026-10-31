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
 * and 6,000, at the boundary the record of the run's end first stands ("by end" - the decisions the game takes; -1 without
 * an end) and in all, the told lines by tick 600 and 6,000, and the ticks at which the first reign's people reached the
 * second and the third rung), and the run's end ("CosmicBalance end: first <kind> at <tick>; clock at, deposed at;
 * chronicle life <n> shown <lines> of <candidates>, more <0|1>" - since B6a [B-6a3c] the kind (CosmicEndings::NameOf) and
 * tick of the record FGalaxyState::Ending, read at the first boundary it stands, "first None at -1" while none does; the
 * clock's tick is the knob's, read apart; looked at after every tick, nothing stops at it - and the uncapped chapter's
 * lines by the cap's four ranks, "ranks a/b/c/d"; the third rung no longer ends a run, B6a [B-6a3a]). Every "by end" count
 * of the other lines counts up to and including the record's tick (the clock's in B6a; B4's "by end" was the ruled
 * third rung - the ending line splits there). The end check (B6a [B-6a3b], header section
 * 5.1) asks the ending predicate (CosmicEndings::EndingOf) at every tick boundary until it first holds: the state's record
 * (FGalaxyState::Ending) must be empty before that boundary, hold the predicate's four fields (kind, tick, people, no
 * vessel) at it and not change after it - a miss prints a "CosmicBalance end check:" error naming both records (the first
 * miss, and their count when there are more) and fails the run; a run that passes prints no end check line.
 * The ending line (B6a [B-6a3c], header section 5.1, notes section 2.1 - outside the three lines the runner compares, a
 * record with no assertion), "CosmicBalance ending:", read from the state, the record and the policy's own counts (no rule
 * copied): the record (kind, tick, its people and whether some life ruled that people - the calling deaths line's old "by a
 * new people" -, no vessel); the season - the first boundary a people the vessel rules stood at the third rung or above
 * (any reign's, read after the tick and again after the inputs) and the record's tick less it (-1 without either); the
 * ruler policy's decisions (CosmicRulerPolicy::DecisionsOf) of the season's boundaries before the record's, per full
 * 600-tick window from its start (one window for a shorter season - median and least) and the longest stretch without
 * one; the exact first tick an AI people (not ruled before that tick) climbed to the third rung, read at every boundary,
 * with its feat row (its Log.Ascension line); the first tick each research field of the season's people (else the first
 * reign's) and of that AI people stood at 1.0; the first boundary some transcendent's bAwareOfPlayer stood, whether by the
 * record and whether the vessel had left by then (a leave recorded later in that tick than the flag); the first-person
 * claim lines (the pressure line's count) and the awareness lines (Log.Transcendent) told by the record; the claims placed
 * in the season on settled systems of the ruled people; B4's counts split at the ruled third rung - before it / the
 * season / after the record: #132 the guarded crises by tag with the delay as a share of their countdowns and the fleet
 * power the attrition took from the people the vessel ruled before the tick, #142 the band 401-550 and its joined, #135
 * the claim trigger's first holds and the cards it lost to a waiting card and in the same tick, with each first hold's
 * key "tick:system:phase:k|w|s" (kept / lost to a waiting card / to one put in the tick - the runs that share a world
 * count it once by seed, tick and system), #145 the sieges begun on the ruled people's systems while its fleet regrows
 * after a resistance; the first boundary by the record a people the vessel rules counted LowStabilityTicks above 0; the
 * ruled people's Insight and Influence at each pressure card's boundary by phase (the cards "a/b/c", each stock
 * "least:median" per phase, "-" for none - #144); the boundary petition_exalt first waited for the ruler with the choice
 * the policy took on it (-1: none taken); and its pre-count (notes section 2) - every boundary its B6a condition (an
 * embodied ruler, the ruled people at the second rung or above) came to hold in the tick (it did not after the last
 * boundary's inputs, or the tick began a reign), as "tick:kind:row" with kind waited (a card waited at the tick's head),
 * floor (no card put), contest (another Standing card put) or other (a card of another trigger put).
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
 * Without them (all off by default) it reads the state only: the world is that of RunHeadless(Ticks) - an ending stops
 * no simulation (B6a [B-6a3b], header section 2.5: the record is written once and the ticks go on, so RunHeadless(Ticks)
 * runs past it as the one-tick loop does; only RunLeap stops at it, and the commandlet never calls RunLeap). The end itself
 * belongs to the presentation and stops nothing here.
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
