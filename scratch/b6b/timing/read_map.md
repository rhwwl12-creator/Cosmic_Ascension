# G1 time gate - read-only map of the AI stage (HEAD 84f0d36 = [B-6b8])

Sources: ROOT at HEAD only, exported read-only with `git archive 84f0d36` to
`C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b6b/timing/head/`
(ROOT's working tree differs only by line endings; nothing in ROOT was edited, built or run). Engine facts were read
from `C:/Program Files/Epic Games/UE_5.8/Engine/Source`. Evidence also from the lead's logs and CSVs
(`scratchpad/b6b8/timing/*.log`, `ROOT/Saved/Balance/b6b8_timing/*.csv`, read only) via the helper
`scratchpad/b6b/timing/tools/census.py`. SAND (`scratchpad/b6b/timing_wt`) does not exist yet: this step did not create,
build or touch it.

Line numbers below are HEAD's (`Sim/...` means `Source/Cosmic_Ascension/Sim/...`).

---

## 0. Short answer

The AI timer laps `TickCivAI()` and nothing else. With B6b data the stage does the same walk per people per tick,
but several **table walks that sort or scan row names on every call** grow with B6b's rows, and a few B6b-only steps
run every tick for the peoples they concern:

1. **`FCosmicDataTables::GetFeatRowNames()` (Data/CosmicDataTables.cpp:150)** allocates a TArray and selection-sorts the
   feat names with `FNameLexicalLess` (case-insensitive string compare through the name pool) **on every call**. The AI
   stage calls it about 2-3 times per AI people per tick (AsksForTreatyOf always, JudgePeople, Choices) - roughly 25-36
   calls a tick with 12 peoples. B6b adds two feat rows: 3 rows -> 3 lexical compares, 5 rows -> 10 compares
   (UE's IntroSort uses selection sort, n(n-1)/2 compares, for n <= 8 - Algo/IntroSort.h:63). That is ~+7 string
   compares and +2 `FindFeat` per call, i.e. ~+200-250 lexical compares a tick **from tick 1, at every rung**, which at
   15-40 ns a compare is ~3-9 us/tick. This is the prime suspect for the +6.5 us at 3,000 ticks where nobody is above
   the third rung. Supporting sign: in the CSVs (column TickMicros = mean whole-tick time of each 100-tick block) the
   median block is already +3.4 us (idle3000) / +5.4 us (idle20000) with data on over ticks 200-1100, when all 12
   peoples are on the first rung and no B6b step can run, and +6-9 us over 1100-1500 (second rung, before any third
   rung: the second-rung peoples' capital slot is empty, so Choices adds a third sort per people per tick). These are
   whole-tick numbers (all stages), so they bound, not measure, the AI part.
2. **The treaty step** (`CosmicDiplomacy::AsksForTreatyOf` + `AITreatyPartnerOf`) runs every tick for every third-rung
   AI people without a treaty: 1-4 such peoples from ~1,800 in idle3000, 3-4 for ~16,000 ticks in idle20000 (none
   blocked by cost - Influence is in the thousands - so they are refused by view or hostility and ask again next
   tick). Each ask builds two or more heap `TArray<bool>` border marks (`MarkNeighboursOf`) and walks lanes.
3. **The law step** (`AsksForLawsOf` + `GalaxyLawRowsOf` + 3x `EnactBlockOf`) every tick for each AI people on the
   fourth rung or above: one feat-name sort plus one edict-name sort (10 rows, quicksort path) plus ~20 `FindRow`
   each tick, even after all three laws stand (they return `Held`). Only in the long runs (civ_8 3151-7950, civ_4
   5729-20000 in idle20000).
4. Smaller: `IsHazardAnswer` (alloc + scan of 6 instead of 5 hazard rows), `FindHazardByTag` (alloc + sort of 6 instead
   of 5 hazard names), JudgePeople's extra `FindFeat` and the third-rung feat row's `CheckAll`, Choices' look at
   `project_ark` (T3) and `project_binding` (T4, refused `NoFallen` every tick after the binding).

All of these are pure reads, so each can be cached or reordered **without changing any draw or outcome** (section 6).
The on/off pair is not the same world (world hash 2906170550 vs 3300228750 at 3,000), so the timer plan (section 5)
pairs in-stage laps and call counters with an end-of-run bench that runs the pure parts on the **same final state**
with both table sets - that separates the code cost from world divergence.

---

## 1. What "AI xx us" measures

`UGalaxySimulationSubsystem::SimulateTick` (Sim/GalaxySimulationSubsystem.cpp:1582):

- 1613-1636: `Lap` restarts after `TickResearch` (1627), `Mark(0)` after diplomacy, `Mark(1)` after conflict,
  `Mark(2)` after `TickHazards`, then `TickCivAI(); Mark(3);` (1634-1635). So `Cycles[3]` = `TickCivAI()` exactly,
  including `UAscensionSubsystem::EvaluateFeats` (called at its end).
- `StageClock.Ticks` counts every tick (interregnum too: TickCivAI runs every tick).
- The commandlet prints it at Tests/CosmicBalanceCommandlet.cpp:3736-3738 (locals `Clock`, `PerCycle`, `Ticked` at 3730-3732) as `Cycles[3] * PerCycle / Ticked`.
- Economy = `Cycles[4]` (`TickEconomy` only, 1624-1625) - the normaliser. The B6b counts at the economy end
  (`LawsHeldTicks`, `PressureEnduredTicks`) are in the economy lap, not the AI lap. The two knobs that
  `-WithoutB6bData=1` turns off (`AIUpperRungProgressMultiplier`, `FleetDelayCountdownShare`) are read in
  `AdvanceMegaprojects` / `FleetDelayOf` (crisis stage), not in the AI lap - but they make the off run a different world.

Not in the AI lap: the petitions' `StandingHeld` at the tick head (1596), the knowing stage, the tenth stage, the
commandlet's own per-tick measurement (e.g. its `AsksForTreatyOf` calls at Tests/CosmicBalanceCommandlet.cpp:2833, 2949).

---

## 2. TickCivAI per tick (Sim/GalaxySimulationSubsystem.cpp:2897-2977)

Loop over `State.Civs` (12 peoples in the default galaxy, 61 systems), skipping the player-ruled one (2905). For each
AI people, in order:

| # | Part (call site) | Per-tick work per AI people | B6b? | Draws / writes |
|---|---|---|---|---|
| a | `ClaimLeaveProfiles.HasTagExact` (2911) | tag container lookup | no | - |
| b | **Choices** `CosmicMegaprojects::Choices` (2912; Sim/CosmicMegaprojects.cpp:321-385) then `Start` per choice (2918) | (i) answers: OwnedSystems x `FindSystem`; per settled system with a hazard and a free response slot `FindHazardByTag` (alloc + **sort of all hazard names**) then per answer `CanStart` -> `StartBlockOf` (FindMegaproject, FindCiv, `IsHazardAnswer` = alloc + scan of all hazard rows with `FindHazard` each, `FindHazardByTag` again). (ii) capital path (356-383) only when the capital is settled, owned and its construction slot is empty: **`GetFeatRowNames()`** + `FindFeat` per row; for each judged row's `MegaprojectComplete` key: `IsHazardAnswer` (scan), `CompletedByPeople` and `RunningForPeople` (OwnedSystems walks), `CanStart` (StartBlockOf again incl. `IsHazardAnswer`; for `project_binding` also `CosmicSeal::BindTargetOf`). Returns TArray by value (alloc only when non-empty). | grows: feat rows 3->5, hazard rows 5->6; T3 (on) walks feat_passage_by_ark: `IsHazardAnswer(project_ark)` full scan, nothing to start; T4 walks project_binding: 2x IsHazardAnswer + walks + `NoFallen` after the binding | Start writes slots (rare) |
| c | **Settle** `MaySettle` (2920) -> `Rng.GetFraction()` -> `SettleTargets` -> `Settle` | OwnedSystems walks (OwnedCount, IsSettling), lanes of footholds, `IsTargetOf` -> `SphereRefuses` (Transcendents x `FindTranscendent`) | no (world only) | one draw when MaySettle |
| d | **Revoke** `CosmicEdicts::AIRevokesOf` (2934; CosmicEdicts.cpp:343) | ActiveEdicts x `FindEdict` (+3 when the laws stand; laws are skipped) | grows slightly at T4+ | Revoke |
| e | **Law step** `CosmicEdicts::AsksForLawsOf` (2944; CosmicEdicts.cpp:242) | returns false below T4 before any walk. At T4+: **`GetFeatRowNames()`** + `FindFeat` x5 + `CarriesCondition`; if true `GalaxyLawRowsOf` (CosmicEdicts.cpp:215: **`GetEdictRowNames()` = alloc + sort of 10 edict names** + `RemoveAll` with `FindEdict` x10), then per law `FindEdict` + `EnactBlockOf` (CosmicEdicts.cpp:47: FindEdict, `ActiveEdicts.Contains` -> `Held` once held; else Tier, `CheckAll(EnactConditions)` (empty for the 3 laws), HeldCountOf (FindEdict per edict), SlotsOf (`Research.FindRef`), TryPay probe) | **B6b only** (T4 is unreachable without B6b rows) | Enact (rare) |
| f | **Treaty step** `CosmicDiplomacy::AsksForTreatyOf` (2958; CosmicDiplomacy.cpp:430) | **no tier gate**: always **`GetFeatRowNames()`** + `FindFeat` per row + `IsJudgedRulerRowAt` + `CarriesCondition`. At T3 with data on (feat_passage_by_ark carries CivHasTreaty): an `FCosmicEvalView`, `FindSystem`, `Check(CivHasTreaty)` = `TreatyPartnersOf` (Ties walk). If no treaty -> **`AITreatyPartnerOf`** (CosmicDiplomacy.cpp:461): `BordersOf` (heap `TArray<bool>` Init(12) + `MarkNeighboursOf`: settled systems x lanes x FindSystem/FindCiv), then per bordering AI people `TreatyBlockOf` (389): FindCiv, HasTreaty, StanceOf x2, **`ViewOf` -> `Borders(Other, Civ)` -> another heap `TArray<bool>` + `MarkNeighboursOf(Other)`**, `TreatyBarOf` (FindEdict per edict), cost check | the sort grows at every rung; the ask and the partner search are **B6b only** | Sign (rare), TryPay |
| g | **Diplomacy** `TickAIDiplomacy` (2970; 2979-3049) | Ties walk; `FindSystem`; **`CosmicConflict::AIAimOf`** (3017; CosmicConflict.cpp:487): gate (target/rest/fleet/pacifism, Ties walk, `OwnedCountOf` vs realm cap) then `BordersOf` (heap TArray<bool>), ruled index, per other people: power ratio, border, relation, NA treaty; pile-on check (Civs x StanceOf); **all 61 systems** x (`SiegeBlockWith`: FindCiv, IsLastSettledSystem, IsNextToSettled, Civs walk, OwnedCountOf) + `ComputeAttritionPressureIn` | not B6b code; world-dependent (B6b's NA treaties remove partners from the candidate list - usually fewer scans) | TurnHostile, fleet target (rare) |
| h | **Feats** `GetSubsystem<UAscensionSubsystem>` + `EvaluateFeats` (2974-2976; Ascension/AscensionSubsystem.cpp:30) | ruled people first (54), then each AI people (59-65) -> `JudgePeople` (68): `GetSubsystem<UGalaxySimulationSubsystem>` (70), settled list (OwnedSystems x FindSystem), **brake** `CosmicAscent::FourthRungOpenOf` for an AI people on T3 (95; Sim/CosmicAscent.cpp:10: `FindGalaxy` + Civs + Transits walk), then **`GetFeatRowNames()`** + `FindFeat` per row (100-113), per judged row x settled system an `FCosmicEvalView` + `CheckAll` (122) | the sort grows at every rung; T3 (on, not braked) checks feat_passage_by_ark per settled system (`MegaprojectComplete` fails fast on `CompletedMegaprojects.FindByPredicate`); braked T3 peoples return before the sort (cheaper than off) | Ascend draws (NameNotable) - rare |

Condition costs reached here (Events/CosmicEventSubsystem.cpp:417-667): `MegaprojectComplete` (486) =
`HasCompleted` linear find on the system's records; `CivHasTreaty` (642) = Ties walk; `CivLawsHeldTicksAtLeast` (647)
= `TickCountOf(Key)` (parses the Key string - only in the T4 feat row, judged by CheckAll after the cheap ones);
`CivSettledSystemsAtLeast` (621) = OwnedSystems walk; `CivResearchAtLeast` (600) = TMap find. `CheckAll` (669)
short-circuits.

### The table primitives (Data/CosmicDataTables.cpp) and B6b's growth

| Primitive | Cost per call | Rows off -> on | Lexical compares off -> on |
|---|---|---|---|
| `GetFeatRowNames` (150) | `UDataTable::GetRowNames` (heap alloc, keys copy) + `Sort(FNameLexicalLess)` | 3 -> 5 | 3 -> 10 |
| `GetHazardRowNames` (143) / `FindHazardByTag` (104, after the invalid-tag return) | same + `FindHazard` per row until the tag matches | 5 -> 6 | 10 -> 15 |
| `GetEdictRowNames` (181) / `GalaxyLawRowsOf` | same (n > 8: quicksort partition + selection) + `FindEdict` x n | 7 -> 10 | ~21 -> ~26 (but only on runs: off never has T4) |
| `IsHazardAnswer` (164) | `Hazards->GetRowNames()` (alloc, no sort) + `FindHazard` + `RulerResolutions.Contains` per row until a hit | 5 -> 6 | 0 (one more FindRow on a miss; project_ark is the last row) |
| `FindFeat/FindEdict/FindHazard/...` (79-102) | `UDataTable::FindRowInternal`: RowStruct `IsChildOf` + TMap find | - | - |

A lexical compare is `FName::Compare` -> `CompareDifferentIdsAlphabetically` (Core/Private/UObject/UnrealNames.cpp:2256):
two 2 KB `FNameBuffer`s on the stack, two name-pool resolves, `FCStringAnsi::Strnicmp`. Its cost is not measured yet
(assumed 15-40 ns) - the bench in section 5.4 measures it.

---

## 3. What differs with B6b data at 3,000 ticks (idle, seed 42)

From the logs: same expansion, same conflict counts, same hazards except one extinction placed (on); the AI step
signed 4 non-aggression treaties (first at 1901); nobody reached T4. CSV census (AI peoples per tier, T3 without a
treaty) - identical tiers both sides:

| tick | tiers | T3 without treaty on / off |
|---|---|---|
| 200-1100 | 12 x T1 | 0 / 0 (whole-tick block median +3.4 us on; +5.4 in idle20000) |
| 1100-1500 | T1 -> T2 | 0 / 0 (whole-tick block median +9.0 us on; +6.2 in idle20000) |
| 1500 | 2 T1, 10 T2 | 0 / 0 |
| 1800 | 10 T2, 2 T3 | 2 / 2 |
| 2400 | 9 T2, 3 T3 | 2 / 3 |
| 2700 | 6 T2, 6 T3 | 4 / 6 |
| 3000 | 3 T2, 9 T3 | 2 / 9 |

(off has no treaty step, so its "T3 without treaty" costs nothing.)

At 20,000 (on): from ~4,000 ten T3, one T4 (civ_8, then T5 at 7950) and later civ_4 at T4 (5729-end); 3-4 T3 peoples
never get a treaty (border 1-2 each, Influence > 4,000: refused by view or hostility); 7 peoples braked for
85,291 AI-ticks (braked peoples skip JudgePeople's rows). Off: all 12 on T3 from ~4,000, no judged row at T3.

Hypotheses, ranked (to be confirmed by the counters and the bench):

- **H1 (all ticks, all rungs): feat-name sorts.** ~2.5 calls x 12 peoples x (+7 compares, +2 FindFeat). Predicts a
  constant +3-9 us from tick 1 - consistent with the T1-era CSV gap and with most of the +6.5 us at 3,000.
- **H2 (T3 era): the treaty step's partner search** for T3 peoples without a treaty: ~0.5-1.5 us per asking people per
  tick (2-3 border-mark arrays with heap allocs). Predicts +1-4 us/tick in the T3 era of idle3000 (~+0.5-1.5 averaged),
  +2-5 us/tick over 16,000 ticks of idle20000.
- **H3 (T4 era, long runs only): the law step** (~2 sorts + ~20 FindRow per T4+ AI people per tick) and Choices'
  binding look after the fallen one is sealed.
- **H4: `IsHazardAnswer`/`FindHazardByTag` one row longer;** T3 feat row checks in JudgePeople and Choices. Small.
- **H5: world divergence** (NA treaties change AIAimOf's candidate set; different hazard timing from the
  FleetDelayCountdownShare knob). Expected small at 3,000 (conflict lines identical); measured by the same-state bench.

The crisis stage also grew (9.27 -> 9.98 us at 3,000; 13.51 -> 26.31 us at 20,000) - outside G1, but the same
primitives (`FindHazardByTag` has 19 call sites in Sim/Events) and the galaxy-law hooks (`HookEdictsOf` walks every
people's edicts per system) are the likely family. Not mapped here.

---

## 4. B6b steps - cacheable or skippable without changing any outcome?

Every function in the AI lap except Start/Settle/Revoke/Enact/TryPay/Sign/TurnHostile/Ascend is a pure read of
(state, tables, rules); the tables never change inside a tick (SetTables is the only writer and is never called from a
stage). So:

- Anything that is a function of the tables alone (sorted names, judged rows per rung, which rows carry CivHasTreaty or
  CivLawsHeldTicksAtLeast, which rows are hazard answers, the galaxy-law list, the hazard row of a tag) can be computed
  **once per tick** (or memoised with an exact validity check) with bit-identical results.
- `FindHazardByTag` and `IsHazardAnswer` can be rewritten to walk the RowMap without allocating and without sorting:
  "the lexically first row whose tag matches" = the matching row with the smallest name (compare only on ties), and
  "any row lists this answer" is order-free. Same answers, no header change.
- `AsksForTreatyOf` can return false before the sort when no judged row of the people's rung carries CivHasTreaty
  (per-tick per-rung flag) - same boolean.
- `AITreatyPartnerOf` needs only "== None"; the order inside the conjunction is free. Its border arrays can use an
  inline allocator; `Refused` can be decided without the second border walk when both possible views (stored relation if
  bordering, BaseTarget if not) are on the same side of the bar. The search itself must still run every tick (its answer
  can change with relations and stances): cross-tick caching is not bit-identical without a dependency key.
- The brake (`FourthRungOpenOf`) is already cheap (FindGalaxy + 12-entry walk) and is asked only for AI T3 peoples.

---

## 5. Timer plan (SAND only, never committed)

### 5.1 One new header, Core layer (includable from Data, Sim, Ascension, Tests)

`Source/Cosmic_Ascension/Core/CosmicAIProbe.h` (name unique in the target; a new header needs a full UBT build):

```cpp
// TEMPORARY timing probe for the G1 gate (SAND only - never committed).
#pragma once
#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"

namespace CosmicAIProbe
{
	enum EBlock : int32 { BChoices, BSettle, BRevoke, BLaws, BTreaty, BTreatyPartner, BDiplomacy, BAim, BFeats, BBrake, BJudgeRows, BNum };
	enum ECount : int32 { CFeatNames, CHazardNames, CEdictNames, CHazardAnswer, CHazardByTag, CFindFeat, CFindEdict, CFindHazard,
		CFindProject, CFindGalaxy, CFindTrans, CAsksTreaty, CPartnerCands, CMarkNeighbours, CAimScan, CAimSiegeChecks, CAsksLaws,
		CEnactBlock, CJudgeCheckAll, CChoicesCapital, CNum };
	inline bool GOn = false;    // -AIProbe=1
	inline bool GInAI = false;  // true only inside TickCivAI
	inline uint64 GCycles[BNum] = {};
	inline int64 GHits[BNum] = {};
	inline int64 GCount[CNum] = {};
	inline int64 GTierTicks[5] = {};
	inline int64 GT3BareTicks = 0;
	inline int64 GTreatyReason[16] = {};
	inline void Reset() { FMemory::Memzero(GCycles); FMemory::Memzero(GHits); FMemory::Memzero(GCount); FMemory::Memzero(GTierTicks); GT3BareTicks = 0; FMemory::Memzero(GTreatyReason); }
	FORCEINLINE uint64 Now() { return GOn ? FPlatformTime::Cycles64() : 0; }
	FORCEINLINE void Lap(int32 B, uint64& L) { if (GOn) { const uint64 N = FPlatformTime::Cycles64(); GCycles[B] += N - L; ++GHits[B]; L = N; } }
	struct FScope { int32 B; uint64 T0; explicit FScope(int32 In) : B(In), T0(Now()) {} ~FScope() { if (GOn) { GCycles[B] += FPlatformTime::Cycles64() - T0; ++GHits[B]; } } };
	FORCEINLINE void Count(int32 C) { if (GOn && GInAI) { ++GCount[C]; } }
}
```

### 5.2 Insertion points (every edit keeps the original short-circuit order; the hash must not move)

| Where (HEAD line) | Edit | Name |
|---|---|---|
| Sim/GalaxySimulationSubsystem.cpp `SimulateTick` 1634 | `CosmicAIProbe::GInAI = true; TickCivAI(); CosmicAIProbe::GInAI = false;` (inside the existing Mark(2)..Mark(3) lap) | flag |
| `TickCivAI` 2908/2909 (after the ruled `continue`) | census first, outside the laps: `if (CosmicAIProbe::GOn) { const FCivilizationState& C = State.Civs[CivIndex]; ++CosmicAIProbe::GTierTicks[FMath::Clamp(static_cast<int32>(C.Tier), 0, 4)]; if (C.Tier == EAscensionTier::Tier3_Causal && CosmicDiplomacy::TreatyPartnersOf(C, FGameplayTag()) == 0) { ++CosmicAIProbe::GT3BareTicks; } }` then `uint64 ProbeLap = CosmicAIProbe::Now();` | census |
| `TickCivAI` after 2919 (end of the Choices/Start loop) | `CosmicAIProbe::Lap(CosmicAIProbe::BChoices, ProbeLap);` | BChoices |
| after 2930 (end of the settle `if`) | `Lap(BSettle, ProbeLap)` | BSettle |
| after 2937 (end of the revoke loop) | `Lap(BRevoke, ProbeLap)` | BRevoke |
| 2944-2954 | inside the `if (AsksForLawsOf)` add `CosmicAIProbe::Count(CAsksLaws);`; after 2954 `Lap(BLaws, ProbeLap)` | BLaws |
| 2958-2968 | inside the `if (AsksForTreatyOf)` add `Count(CAsksTreaty)`; replace 2960 by `int32 Partner = INDEX_NONE; { CosmicAIProbe::FScope ProbeP(CosmicAIProbe::BTreatyPartner); Partner = CosmicDiplomacy::AITreatyPartnerOf(...); }`; after 2968 `Lap(BTreaty, ProbeLap)` | BTreaty (of which BTreatyPartner) |
| after 2970 `TickAIDiplomacy(CivIndex);` | `Lap(BDiplomacy, ProbeLap)` | BDiplomacy |
| `TickAIDiplomacy` 3017 | `CosmicConflict::FCosmicAIAim Aimed; { CosmicAIProbe::FScope ProbeA(CosmicAIProbe::BAim); Aimed = CosmicConflict::AIAimOf(...); }` (was `const`) | BAim (inside BDiplomacy) |
| `TickCivAI` 2974-2976 | wrap in `{ CosmicAIProbe::FScope ProbeF(CosmicAIProbe::BFeats); ... }` | BFeats |
| Ascension/AscensionSubsystem.cpp `JudgePeople` 95-98 | `bool bClosed = false; if (!bPlayer && State.Civs[CivIndex].Tier == EAscensionTier::Tier3_Causal) { CosmicAIProbe::FScope ProbeB(CosmicAIProbe::BBrake); bClosed = !CosmicAscent::FourthRungOpenOf(State, Tables, State.Civs[CivIndex].GalaxyRow); } if (bClosed) { return false; }` | BBrake |
| `JudgePeople` before 100 | `CosmicAIProbe::FScope ProbeRows(CosmicAIProbe::BJudgeRows);` (closes at every return) ; before 122 `CosmicAIProbe::Count(CJudgeCheckAll);` | BJudgeRows |
| Data/CosmicDataTables.cpp 150 / 143 / 181 / 164 | first line of the body: `CosmicAIProbe::Count(CFeatNames / CHazardNames / CEdictNames / CHazardAnswer)` | counts |
| Data/CosmicDataTables.cpp 104 | after the `!Tag.IsValid()` return (113): `Count(CHazardByTag)` (its inner GetHazardRowNames also counts CHazardNames) | count |
| Data/CosmicDataTables.cpp 89, 99, 79, 84, 64, 74 | `Count(CFindFeat / CFindEdict / CFindHazard / CFindProject / CFindGalaxy / CFindTrans)` | counts |
| Sim/CosmicDiplomacy.cpp 70 `MarkNeighboursOf` | `CosmicAIProbe::Count(CMarkNeighbours);` | count |
| Sim/CosmicDiplomacy.cpp 470-477 `AITreatyPartnerOf` | `if (Bordering[Other] && !State.Civs[Other].bIsPlayerRuled) { CosmicAIProbe::Count(CPartnerCands); const ECosmicTreatyBlock Block = TreatyBlockOf(...); if (CosmicAIProbe::GOn && CosmicAIProbe::GInAI) { ++CosmicAIProbe::GTreatyReason[FMath::Clamp(static_cast<int32>(Block), 0, 15)]; } if (Block == ECosmicTreatyBlock::None) { return Other; } }` | reasons |
| Sim/CosmicConflict.cpp 515-518 `AIAimOf` | before `TArray<bool> Borders;` `CosmicAIProbe::Count(CAimScan);`; 553-554 split as `if (OwnerCivID == Them.CivID) { Count(CAimSiegeChecks); if (SiegeBlockWith(...) == None && !(Attrition...)) {...} }` | counts |
| Sim/CosmicMegaprojects.cpp before 361 (`Choices` capital path) | `CosmicAIProbe::Count(CChoicesCapital);` | count |
| Sim/CosmicEdicts.cpp 50 `EnactBlockOf` | `Count(CEnactBlock)` | count |

Lap overhead: ~6 `Cycles64` reads per AI people + ~4 scoped pairs = ~100 reads a tick, the same on both sides
(`FPlatformTime::Cycles64` on Windows is QueryPerformanceCounter). The bench measures one pair (5.4) so the overhead can
be subtracted; a pair run with `-AIProbe=0` gives the uninstrumented stage line in the same build.

### 5.3 Commandlet (Tests/CosmicBalanceCommandlet.cpp)

- Parse: after 1513 `FParse::Value(*Params, TEXT("WithoutB6bData="), WithoutB6bData);` add
  `int32 AIProbe = 0; FParse::Value(*Params, TEXT("AIProbe="), AIProbe);`
- Reset: at 1653-1654 (after `NewGalaxy`, where the stage clock is reset and turned on):
  `CosmicAIProbe::Reset(); CosmicAIProbe::GOn = AIProbe != 0;`
- Print: inside the always-told block that prints the stage line, right after that UE_LOG (3736-3738) and before the block's closing `}` at 3739, where
  `Clock`, `PerCycle`, `Ticked`, `State`, `StateIndex`, `RulesNow`, `Sim` are in scope:

```cpp
if (CosmicAIProbe::GOn)
{
	using namespace CosmicAIProbe;
	auto Us = [&](int32 B) { return static_cast<double>(GCycles[B]) * PerCycle / Ticked; };
	auto Per = [&](int64 N) { return static_cast<double>(N) / Ticked; };
	UE_LOG(LogCosmic, Display, TEXT("CosmicProbe AI blocks us/tick: choices %.3f settle %.3f revoke %.3f laws %.3f treaty %.3f (partner %.3f) diplomacy %.3f (aim %.3f) feats %.3f (brake %.3f rows %.3f); laps sum %.3f, stage %.3f"),
		Us(BChoices), Us(BSettle), Us(BRevoke), Us(BLaws), Us(BTreaty), Us(BTreatyPartner), Us(BDiplomacy), Us(BAim), Us(BFeats), Us(BBrake), Us(BJudgeRows),
		Us(BChoices) + Us(BSettle) + Us(BRevoke) + Us(BLaws) + Us(BTreaty) + Us(BDiplomacy) + Us(BFeats), Clock.Cycles[3] * PerCycle / Ticked);
	UE_LOG(LogCosmic, Display, TEXT("CosmicProbe AI counts per tick: featNames %.2f hazardNames %.2f edictNames %.2f hazardAnswer %.2f hazardByTag %.2f findFeat %.2f findEdict %.2f findHazard %.2f findProject %.2f findGalaxy %.2f findTrans %.2f; asksTreaty %.3f partnerCands %.3f markNeighbours %.2f aimScans %.3f aimSiegeChecks %.2f asksLaws %.3f enactBlock %.3f judgeCheckAll %.2f choicesCapital %.2f"),
		Per(GCount[CFeatNames]), Per(GCount[CHazardNames]), Per(GCount[CEdictNames]), Per(GCount[CHazardAnswer]), Per(GCount[CHazardByTag]), Per(GCount[CFindFeat]),
		Per(GCount[CFindEdict]), Per(GCount[CFindHazard]), Per(GCount[CFindProject]), Per(GCount[CFindGalaxy]), Per(GCount[CFindTrans]), Per(GCount[CAsksTreaty]),
		Per(GCount[CPartnerCands]), Per(GCount[CMarkNeighbours]), Per(GCount[CAimScan]), Per(GCount[CAimSiegeChecks]), Per(GCount[CAsksLaws]), Per(GCount[CEnactBlock]),
		Per(GCount[CJudgeCheckAll]), Per(GCount[CChoicesCapital]));
	// census: AI-people ticks per tick by rung, T3 without treaty, treaty block reasons (ECosmicTreatyBlock order) as counts
	...one UE_LOG "CosmicProbe AI census: T1 %.2f T2 %.2f T3 %.2f T4 %.2f T5 %.2f; T3 bare %.2f; reasons [i=n ...]"
	// then the bench of 5.4
}
```

### 5.4 End-of-run bench (pure reads on the final state - no draw, no write; after the hash line is printed)

Inside the same `if (CosmicAIProbe::GOn)`. A helper `Bench(Name, N, TFunctionRef<int64()>)` times N calls with
`Cycles64` and prints `CosmicProbe bench <name> <tables>: <ns/call>` (sum into an `int64` sink that is printed, so
nothing is elided). Two table sets on the **same state**: `const FCosmicDataTables& OnT = Sim->GetTables();` and, under
`#if WITH_DEV_AUTOMATION_TESTS`, `const FCosmicDataTables OffT = CosmicTests::WithoutB6bData(OnT);` (in an off run both
are the off tables). Rules: `RulesNow` for both (the two knobs are not read in the AI lap). Items, each for OnT and OffT:

1. `T.GetFeatRowNames().Num()` x 20,000; `GetHazardRowNames`, `GetEdictRowNames` likewise.
2. `T.IsHazardAnswer(project_dyson)` (miss: full scan), `T.IsHazardAnswer(FName("project_ark"))`, `T.FindHazardByTag(Hazard.Rebellion)`.
3. Per tick-equivalent (x 2,000), summed over the AI peoples of the final state: `CosmicMegaprojects::Choices(State, StateIndex, T, C, 0).Num()`;
   `CosmicDiplomacy::AsksForTreatyOf(...)`; `AITreatyPartnerOf(...)` for those that ask; `CosmicConflict::AIAimOf(..., false).System`;
   `CosmicEdicts::AsksForLawsOf(Civ, T)` and `GalaxyLawRowsOf(T).Num()` for T4+; a JudgePeople replica (settled list,
   brake for T3, `T.GetFeatRowNames()` + `FindFeat` + `IsJudgedRulerRowAt` + per settled system `CheckAll` with an
   `FCosmicEvalView` as at AscensionSubsystem.cpp:116-122 - JudgePeople itself is private).
4. `FName(TEXT("feat_passage_by_ark")).Compare(FName(TEXT("feat_laws_and_binding")))` x 200,000 (one lexical compare).
5. `FPlatformTime::Cycles64() - FPlatformTime::Cycles64()` x 200,000 (probe overhead per read pair).

Attribution: (bench ns on - bench ns off) x (on-run counts) = the code part of the on/off gap; the remainder of the
measured gap = world divergence + noise.

### 5.5 Run protocol (one SAND build, editor closed, command lines contain `b6bt`)

- Build once: `Build.bat Cosmic_AscensionEditor Win64 Development -project="C:\Users\rhwwl\AppData\Local\Temp\b6bt\Cosmic_Ascension.uproject" -WaitMutex -NoHotReloadFromIDE`.
- Before each batch: no python process whose command line holds bundleK, bundleA, bundleB, run_mutations, run_py or
  run_checks, and no UnrealEditor-Cmd (`Get-CimInstance Win32_Process`); else wait (60 s steps, up to 60 min).
- Runs: `UnrealEditor-Cmd.exe C:\Users\rhwwl\AppData\Local\Temp\b6bt\Cosmic_Ascension.uproject -run=CosmicBalance -Seed=42 -Ticks=<n> -AIProbe=1 [-WithoutB6bData=1] -Out=<OUT>\<name>.csv -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog=<OUT>\<name>.log`
  with OUT = `scratchpad/b6b/timing` (never ROOT\Saved).
  Batch 1: 3,000 ticks A B A B A B (on/off alternately, ~10 s each). Batch 2: 20,000 ticks A B A B A B (~15-25 s each).
  Batch 3: one 3,000 pair with `-AIProbe=0` (probe overhead on the stage line).
- Check first that the probe changed nothing: hash/world lines equal the lead's logs (seed 42):
  idle3000 on 2557850423 / 2906170550, off 2816687688 / 3300228750; idle20000 on 1957158585 / 3764704052,
  off 2359424635 / 726895141 (ruler3000 1189870756 / 3359656751, wanderer6000 1631315934 / 489794244, ruler20000
  1375149537 / 510604211).
- Report medians of AI us, economy us, AI/economy, each block, each count; ratios on/off; and the bench deltas x counts.
- End: `git -C SAND -c core.eol=lf checkout -- .` plus delete the untracked `Core/CosmicAIProbe.h` (checkout does not
  remove untracked files), or `git -C ROOT worktree remove --force SAND` - say which.

---

## 6. Bit-identical fix candidates (for [B-6br] if the probe confirms)

| # | Site | Inefficiency | Fix idea - why identical |
|---|---|---|---|
| C1 | Data/CosmicDataTables.cpp:150 `GetFeatRowNames` (callers in the AI lap: CosmicDiplomacy.cpp:438, AscensionSubsystem.cpp:100, CosmicMegaprojects.cpp:361, CosmicEdicts.cpp:248) | heap alloc + selection sort with lexical FName compares on every call, ~25-36 calls a tick; 3 -> 10 compares with B6b's two rows | Compute the sorted list once per `TickCivAI` and hand it down (new overloads taking `TConstArrayView<FName>`; old signatures stay for UI/policy/tests) - the list is a pure function of DT_Feats, which no stage writes. Or an exact-validated memo in FCosmicDataTables (keep the unsorted key sequence and the sorted list; reuse when the RowMap key sequence is element-wise equal; return a const ref) - same set of keys gives the same sorted order, so it is safe even for tests that change a table in place (CosmicAscensionTests.cpp:720 `RemoveRow`). |
| C2 | Sim/CosmicDiplomacy.cpp:438 `AsksForTreatyOf` | no rung gate: sorts and walks every feat row for every AI people every tick | Per tick, per rung: "a judged row of this rung carries CivHasTreaty" (from C1's list); false at once otherwise. Same boolean; the CivHasTreaty `Check` still runs where it ran. |
| C3 | Ascension/AscensionSubsystem.cpp:100-113 `JudgePeople` (+ `GetSubsystem` at :70 per people) | sort + `FindFeat` per row per people per tick | Hoist "judged rows per rung (name, def)" to `EvaluateFeats` once per tick, same lexical order; keep the per-people error line for a malformed Ruler row of the people's rung (computed once, logged as before). Same rows in the same order, same CheckAll calls, same draws in Ascend. |
| C4 | Data/CosmicDataTables.cpp:104 `FindHazardByTag` (AI lap: CosmicMegaprojects.cpp:340, :265; plus 17 sites in other stages) | alloc + sort of all hazard names (10 -> 15 compares) on every call with a valid tag | One pass over `Hazards->GetRowMap()` (one RowStruct check up front), keeping the matching row with the lexically smallest name (compare only when a second row matches - never in game data). Same row as "first in sorted order"; no alloc, no header change. |
| C5 | Data/CosmicDataTables.cpp:164 `IsHazardAnswer` (CosmicMegaprojects.cpp:372, :262, :308; GalaxySimulationSubsystem.cpp:2914) | `GetRowNames` alloc + `FindHazard` per row; one row longer with B6b | Walk the RowMap values directly (order-free any-match); or precompute per tick which feat keys are answers (with C1). Same boolean. |
| C6 | Sim/CosmicMegaprojects.cpp:361-383 `Choices` capital path | T3 (on): looks at feat_passage_by_ark only to skip project_ark (an answer); T4: project_binding re-checked every tick, refused `NoFallen` after the binding | From C1's per-tick list: per rung, the construction keys (judged rows' MegaprojectComplete keys that are not hazard answers), in row/condition order; T3 has none -> loop skipped. Same first startable key. |
| C7 | Sim/CosmicEdicts.cpp:215 `GalaxyLawRowsOf` + :242 `AsksForLawsOf`, called at GalaxySimulationSubsystem.cpp:2944-2953 | per T4+ AI people per tick: feat sort + edict sort (10 rows) + `RemoveAll` with 10 `FindEdict` + 3 `EnactBlockOf` that return `Held` | Law list once per tick (lazily, on the first asking people); per rung "a judged row carries CivLawsHeldTicksAtLeast" from C1. Same laws in the same order through the same gate. |
| C8 | Sim/CosmicDiplomacy.cpp:461-477 `AITreatyPartnerOf` -> :419 `ViewOf` -> :236 `Borders` -> :70 `MarkNeighboursOf` | every tick for each T3 AI people without a treaty: heap `TArray<bool>` for its own borders plus one per bordering candidate (ViewOf recomputes the candidate's borders) | `TArray<bool, TInlineAllocator<16>>` in `Borders`/`AITreatyPartnerOf` (no malloc, same values); and in `AITreatyPartnerOf` skip the candidate's border walk when the stored relation and `BaseTargetOf` both fall below `TreatyBarOf` (Refused either way). Only "== None" is read, so reordering the pure conjunction is identical. A per-tick border table is also possible but needs the argument that nothing in the civ loop changes a settled-to-settled lane (Settle leaves the target unsettled; Start refuses unsettled systems) - verify before using. |
| C9 | Sim/CosmicConflict.cpp:551-561 `AIAimOf` (not B6b) | per candidate people, all 61 systems x `SiegeBlockWith` | Iterate the candidate's own systems sorted by index - identical only if "OwnerCivID == Them.CivID implies SystemID in Them.OwnedSystems" always holds; low priority, outside B6b. |

Verification of any fix: the seven hash/world pairs of 5.5 unchanged, `Cosmic_Ascension.*` twice, then the stage line.

---

## 7. Open points

1. The on/off pair is two worlds (different world hash at 3,000): the on/off stage gap mixes code cost and world
   divergence; only the same-state bench (5.4) splits them.
2. The lexical-compare cost is assumed, not measured (bench item 4). If it is under ~10 ns, H1 explains less and H2/H5
   carry more of the gap.
3. Even after C1-C8, the treaty step for T3 peoples that never find a partner stays a per-tick cost by design (a
   refused ask repeats every tick). Whether the remainder still exceeds 16.05 us at 20,000 is for the lead to judge
   after a re-measure; if it does, the open question is design (ask less often) - a developer question, not [B-6br].
4. C1/C3/C7 change Sim/Ascension/Data signatures (header-first rule for determinism-adjacent code); C4/C5/C8 are body-only.
5. Crisis stage doubled at 20,000 (13.51 -> 26.31 us; tick mean 33.3 -> 57.0 us) - outside the G1 metric, likely the
   same primitives (FindHazardByTag, GetEventRowNames, HookEdictsOf with galaxy laws). Needs its own probe if the tick
   budget matters.
6. SAND does not exist yet; when created, check that the `Content/CA/Data/DT_*.uasset` files are real assets (not LFS
   pointers) before the first run (a pointer would make every table empty and the hashes would not match).
