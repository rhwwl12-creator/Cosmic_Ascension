"""[B-6b8] start stage, item 10: brief_working.md = brief_final.md with the placeholders filled, the anchors re-derived on
HEAD 220129d, the changes since the brief and the lead's order change folded in, and a header saying where each value came
from. LF, UTF-8 without BOM. Scratch only (writes nothing in the project)."""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "brief_final.md")
DST = os.path.join(HERE, "brief_working.md")

HEADER = r"""# [B-6b8] brief - WORKING COPY (start stage done 2026-11-10; this copy wins over brief_final.md where they differ)

HEAD = `220129d` [B-6b7] on `6ae9060` [B-6b6], `10205b6` [B-6b5], `529cab9` [B-6b4] = the brief's [H‴]. Line numbers in the
body below are the brief's (written at `529cab9`); the table "Re-derived anchors" gives each one on `220129d`. Find every
edit by its text.

## Filled values and their sources

| Placeholder | Value | Source |
|---|---|---|
| `<T_DEATH>` | **5000** (harness `--t-death 5000` -> `-DeathAt=5000`) | N §2 '`[B-6b6]`의 다시 셈', N:636: "t = 5,000 (R에서 넷째 칸에 닿은 판 n 30, 중앙값 4,983.5를 100 단위로 올림 …; n ≥ 15라 표시 없음)" - no H §11 flag needed |
| `<N>` | **1200** | `Tools/data/DT_Feats.json:99` (`feat_laws_and_binding`, `CivLawsHeldTicksAtLeast` Key "1200"); N §5 [B-6b7] '값과 까닭' (N:864) |
| `<P_bind>` | **1800** | `Tools/data/DT_Megaprojects.json:67` (`project_binding` `RequiredProgress` 1800.0); same paragraph |
| `<r>` | **0.375** | `Tools/data/DT_Rules.json:17` (`AIUpperRungProgressMultiplier`); the off line reads `(was 0.375)` (`Saved\Balance\b6b7_off\*.log`). `FleetDelayCountdownShare` is 0.0 (`DT_Rules.json:27`) -> `(was 0)` |
| `<A_SEC_PER_20K>` | **11 s** (wall time per 20,000-tick run, 6 workers in parallel: bundle A's R cell median 11 s - on 30 and off 30, min 9, max 17; input-less on 10 s / off 9 s; the 6,000-tick wanderer runs 8 s, so about 6-7 s of each run is engine start). A′ (`b6b7_A1`, R on, 20,000) median 12 s | N §3 '묶음 A' header line (N:709): 240 runs + 15 in **6.8 min at 6 workers** (logs span 23:11:04-23:17:52). The header line gives no per-run figure; the per-run value is read from the bundle's own logs ('Log file open' to 'Log file closed', `Saved\Balance\b6b_bundleA\*.log`). By the header line's tick share alone (6.8 min x 6 x 60 s / (3.12 M / 20,000)) it would be 15.7 s |
| `<B_MINUTES>` | **about 16.5 min** (= 11 x 540 / 6 / 60; 18 min at A′'s 12 s; at most 23.5 min by the tick-share reading) | brief §4.1 formula. The brief's "about 35-42 min" is too high - H:325's '약 22분' lies inside the range. Record the measured value in N §3 |
| `<B6b7_KEYS>` | the five whole lines below ('wrote' path removed as `Remove-WrotePath`) | `Tools/baselines/seed42.json` at HEAD (= the task's baselines; `verified_at` 2026-11-10) |
| `<J1>` `<J2>` `<J3>` `<C>` `<B_LOCKED>` `<B_AI_US>` | after bundle B | - |

`<B6b7_KEYS>`:
- `summary_line`: `CosmicBalance: seed 42, 3000 ticks, hash 2557850423, world 2906170550`
- `ruler_summary_line`: `CosmicBalance: seed 42, 3000 ticks, hash 1189870756, world 3359656751 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 17, answers 17, started 5, focus changes 5, edict changes 5, diplomacy changes 17, decisions 49 (by 600 12, by 6000 -1, by end -1); told by 600 25, by 6000 -1; ruled people T2 at 1175, T3 at 2375`
- `wanderer_summary_line`: `CosmicBalance: seed 42, 6000 ticks, hash 1631315934, world 489794244 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 8, answers 8, started 2, focus changes 4, edict changes 3, diplomacy changes 2, decisions 19 (by 600 12, by 6000 19, by end 19); told by 600 25, by 6000 733; ruled people T2 at 1175, T3 at -1 | CosmicBalance wanderer: abdicate 1200 wander 1; left at 1201, crowned at -1 (None, tier 0, path none), journeys 191, scans 191, cards 7, answers 7, reverence peak 0.00, body 3, mind 1`
- `summary_line_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 1957158585, world 3764704052`
- `ruler_summary_line_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 1375149537, world 510604211 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 110, answers 110, started 41, focus changes 5, edict changes 18, diplomacy changes 17, decisions 191 (by 600 12, by 6000 90, by end 89); told by 600 25, by 6000 927; ruled people T2 at 1175, T3 at 2375`

ⓐ's off keys ([B-6b5]'s, unchanged): 3,000 2816687688 / 3300228750; ruler 3,000 3561771757 / 2462331303; wanderer 6,000
2665013209 / 2229239127; 20,000 2359424635 / 726895141; ruler 20,000 1611658162 / 29194274. Today's off line (eight rows):
`rows taken out [feat_passage_by_ark feat_laws_and_binding project_ark project_binding crisis_species_extinction
edict_wide_machine_accord edict_wide_quarantine edict_wide_warning], AIUpperRungProgressMultiplier 1 (was 0.375),
FleetDelayCountdownShare 0 (was 0)` - after this commit's rows: the same plus `event_era_endured enc_witness` at the end
(the helper's `Note` order, `Tests/CosmicTestHelpers.h:381-391`: Feats, Megaprojects, Hazards, Edicts, Events, Encounters).
`bundleB.py`'s `EXPECTED_REMOVED` already holds the ten.

## Start-stage results (brief §0, items 1-10)

1. **Commits and tree.** `git log --oneline -5`: 220129d [B-6b7], 6ae9060 [B-6b6], 10205b6 [B-6b5], 529cab9 [B-6b4],
   cef8cc7 [B-6b3b]. `git diff --numstat`: no line with a non-zero count (237 ' M' entries are OneDrive stat noise; the one
   untracked file is `docs/OVERVIEW.html`, not touched). No UnrealEditor / UnrealBuildTool / dotnet / LiveCoding process.
2. **N §1.5 holds C6 and C7** (N:515-520 `event_era_endured`, N:521-526 `enc_witness`). `scratchpad\b6b8\check_texts.py`:
   every text of the brief's two JSON rows (§1.1, §1.2) stands quoted in N §1.5 letter for letter (3 era lines; title, body,
   two labels, two success lines, one failure line and the empty one; Mind 3 / NoCheck); Rule W's lists (28 + 4 words) and
   '기록' outside the success column give 0 hits on C7. merged.md was not used.
3. **Schema at HEAD.** `silence_words` (schema.json:53), `dawn_words` (:73), `desolation_words` (:74) exist; the era rule
   is schema.json:595-598 (`when has_type CivPressureEnduredTicksReached` -> `LogTemplates` lacks the three). The walker pin
   is **316** (`Tools/test_data_schema.py:3887`) - set the new value from the schema after Rules W and E, never by hand.
4. **Placeholders**: table above.
5. **Header size.** `wc -c` H = **148,283**. The §6 line template is 382 bytes with its newline (397 for [B-6b7]'s) ->
   about 148,665 < 149,000: no move of H body to N is needed (margin about 330 bytes after the line; keep the line < 400).
   A separate `Sim.WandererLoop` / `Sim.PhaseTwoExit` cause sentence (§6) would also have to fit - measure before adding.
6. **Anchors re-derived** - table below.
7. **Mutation apply state**: `scratchpad\b6b8\apply_before.txt` (by `scratchpad\b6b6\apply_state.py` on a `git archive
   HEAD` copy in `scratchpad\b6b8\apply_head_copy\`): **1,058 patches, 914 apply, 144 do not** - byte-identical to
   [B-6b7]'s `apply_after.txt`. Patches whose `+++ b/` file this commit edits (rerun-set base, §5): DT_Events.json 3,
   DT_Encounters.json 5, schema.json 77, test_data_schema.py 1, CosmicDataTests.cpp 15, CosmicB6bRowNames.h 0,
   CosmicWandererPolicy.h 0, CosmicWandererPolicy.cpp 12 (b1d_* x8, b6a3c_wanderer_policy_reads_clock, b6b3b_wanderer_* x3 -
   as the brief says), CosmicBalanceCommandlet.cpp 2; plus the filter-test patches (README 'Breaks') of the test files edited.
8. **Base.** No editor/UBT process. `Saved\Balance\b6b7_on\`'s five whole key lines equal seed42.json letter for letter
   (ruler and wanderer tails included, 'wrote' aside - `scratchpad\b6b8\check_base.py`: ALL SAME), so no build and no base
   runs were made: the folder was copied byte for byte to **`Saved\Balance\b6b8_base\`** (22 files, sha1 lists identical).
   The DLL is the [B-6b7] clean Rebuild (`UnrealEditor-Cosmic_Ascension.dll` 2026-11-10 03:36).
9. Item 9 of §0 is item 8 above (base copies).
10. **Tripwire**: `Tests/CosmicDataTests.cpp:2775-2779` pins `Today` == the eight names (FeatPassageByArk, FeatLawsAndBinding,
    ProjectArk, ProjectBinding, CrisisSpeciesExtinction, EdictWideMachineAccord, EdictWideQuarantine, EdictWideWarning) - the
    array stops before `EventEraEndured` and `EncWitness`; `Ten` (:2771-2773) has all ten, the full-copy check `Removed ==
    Ten` is :2837. Message today: "Today's data: WithoutB6bData takes out [B-6b6]'s and [B-6b7]'s rows and project_binding
    (taken: %s) - [B-6b8] updates this pin with its rows"; comment (1) :2759-2762 "Today's data holds eight of B6b's ten rows".

## Re-derived anchors (brief line -> HEAD 220129d)

Code:
- `Tests/CosmicDataTests.cpp:2693-2711` (tripwire) -> comment :2758-2767, `Ten` :2771-2773, `Today` pin :2774-2779, full copy
  :2781- (`FullRules` r 0.375 :2783), `Removed == Ten` :2837; theodicy `TheodicyProblems` :2521 (game check :2890); named-card
  copy list :1998-2003 -> :2001-2004.
- `Tests/CosmicB6bRowNames.h:32,35` -> **:33** ("// DT_Events: the era of the endured pressure - [B-6b8].") and **:36**
  ("// DT_Encounters: the wanderer's witness - [B-6b8]."); :18, :22, :25, :28 already read "in the data from …".
- `Tests/CosmicTestHelpers.h:353` -> :354 ("A name the table does not hold yet takes nothing out"); `Note` order :381-391.
  `Tests/CosmicBalanceCommandlet.h:123` unchanged.
- `Tests/CosmicBalanceCommandlet.cpp`: race fields :642-650 -> :643-648 (`EraFirst` :647; add `WitnessFirst` beside it);
  `MedianOf` :765-774 -> :766-775; refusal :1515-1517 -> :1516-1518 (`bSaveShape` :1514-1515 has no `Abdicate`/`Wander` term,
  so `-SaveAt` with `-Abdicate=1200 -Wander=1` is accepted - risk 8's check answered); off line :1596 -> :1597;
  `NoteRuledRungs` :1691-1702 -> :1692-1705; binding :2228-2239 -> :2236-2240 (`BindingRuledRung` :2240);
  `RaceWitnessCardsBefore` :2861 -> :2862 (policy `Act` :2863); input block :2949 -> the B6b [B-6b3b] block :2886-2960
  (`WitnessCardsByRecord` :2950, era loop :2951-2958); CSV sample :3541 -> :3542; `ending:` line :3890 -> :3891;
  witness counts :3922-3925 -> :3923-3926; race parts :3955-3986 -> :3958-3988 (era/witness part :3986-3987).
- `World/CosmicWandererPolicy.h:44-48` unchanged (`WithdrawnByRow` :48). `.cpp:46-85` -> :46-88: the card count :46-50 runs
  only after `if (!bWalks) return;` (:38-41 - `IsEmbodiedWanderer`), then `!bWander` returns at :51-54; pick
  `Counts.Answers % Row->Choices.Num()` :65; withdrawn :71-74; answered :75-78; failed :79-82; chain count :83-87. So in cells
  without `-Wander` the witness is counted only while the vessel walks (a deposed vessel walks; a ruler never sees it).
- `Events/CosmicPetitions.cpp`: `WithinWandererGap` :171-175 (unchanged); `StandingHeld` :225 (wanderer part :236-254); `Present` :273;
  `LastAnswered` :293 (`HeldAtHead` exclusion :297 - a unit that held at the tick's head is not a candidate, so the card
  comes once per arrival or per claim landing under the standing vessel, not on every tick it stands there); derived integer
  :304; the arrival lottery only when no Standing candidate :307-323 (pick :326); `PassingRollsOf` :470-478 -> :492-500.
- `Events/CosmicEventSubsystem.cpp`: `ApplyAll` :971-977 (unchanged); slot rolls :1044-1053 -> `Rolls` :1033, drawn :1044;
  Standing walk :1121-1135 (`FirePair` :1107-1118); category share :1169 (unchanged); `FireResolved` :1229; endured window
  :655-660.
- `Sim/GalaxySimulationSubsystem.cpp:1933` counts `PressureEnduredTicks` (economy stage; `EnduredPressure` :1721/:1874 - the
  drain taken with the capital in no unsealed fallen sphere); `Core/GalaxyState.h:335-339`.
- `Sim/CosmicEndings.cpp:90-111` -> `NameOf` :85-111 (kinds RuledPeople/Vessel/OtherPeople/NoCandidate/Clock/None).
- `Tests/CosmicUpperRungTests.cpp`: `Sim.EraSelector` :698 -> :700, window cases :727-735, "An empty Key is the context
  people" :767. `Tests/CosmicBookTests.cpp`: `Sim.PressureEndured` :358, `WithDrain` :373; `Events.BookCycle` (the brief's
  "Sim.BookCycle") :482, game section :615-660 unchanged (619-652).
- `Data/CosmicRowNames.h`: `EpitaphLeavesOut` :161, comment :147-153 (brief :149-154). `Tests/CosmicCallingPagesTests.cpp:1050`
  `Eulogies` (unchanged). `UI/CosmicCallingPagesWidget.cpp:96-105` (unchanged; `LastAnswersOf` :141).
- `Tools/run_checks.ps1:116-132` -> the five `Compare-Balance` steps :115 (checks), :119 (checks_ruler), :126 (checks_wanderer,
  `-Ticks 6000`), :131-132 (20,000 pair, full tier only); comment "…at 3,121 - by the vacuum" :121-125 (already says "at none
  before 6,000 from [B-6b6]'s data"). Smoke filter :59 = `Sim.Determinism+Sim.Save+Data+Sim.KnowingInert`.
- Test count at HEAD: **223** automation tests.

Data, schema, checker:
- `DT_Events.json:380-419` unchanged (`event_claim_eased` :379-418, `]` :419; 12 rows; LF, tabs). `event_book_closed` :2-49.
- `DT_Encounters.json:1596-1691` unchanged (`petition_pressure_measured` :1595-1690, `]` :1691; 18 rows); `enc_star_watch`
  :1303-1376 -> :1302-1377; "당신은 관측소를 떠났다" :1367 (unchanged).
- `schema.json`: about-text :21 (unchanged); `civ_selector` :37 -> :39; `reverence_reach` :39 -> :41; `epitaph_leaves_out`
  :109 -> :130; category pair `Event.Civ.Society` :113 -> :134; DT_Events `LogTemplates` forbidden :450 -> :474; Civilization
  `slots_in [Civ, Hazard]` :466 -> :491; slot vessel rule :480-494 -> :504-; Lottery weight :503 -> :522; Named / SettledSystem
  with the condition :525/:530 -> :549/:554; era words rule -> :595-598 (DT_Events rules end :599); DT_Encounters
  `required_rows` :1003; the `enc_` wanderer rule :994-1000 -> :1059-1066; `EnactEdict`/`DelayHazard` on `enc_` :1086-1092 ->
  :1151-1177; rule 8′ :1129-1149 -> :1195-1216 (last DT_Encounters rule = the `row_not petition_pressure_measured` rule
  :1212-1216, rules close :1217); `SystemOwnedBy` key :1204 -> :1271; `ChangeReverence` key :1244 -> :1311.
- `cosmic_data_schema.py`: `has_cond` on effects :215-216 (unchanged); `holds` :1317, `has_cond` :1367-1373 (unchanged);
  `row:` tests :600-623 -> :605-632.
- `test_data_schema.py`: `expect` :368-371 - **any** problem line that carries all the words passes, so extra messages are
  fine and `expect` cases need no Rule E message; the passing cases Rule E breaks (each gets `cond("SystemOwnedBy",
  key="Ruled")` and `eff("ChangeReverence", key="Borders", value=0.2)`): **:2763** `harvest(Trigger="Standing", BaseWeight=0,
  Conditions=[endured])` (the brief's :2456), the era-words setup `endured` **:2402** (used by the passing case :2404 and
  the breaking cases :2405-2408; the case :2409 has no condition and stays), and the **rule_setups entry of the era rule
  :3852-3853** (`update_event(Trigger="Standing", BaseWeight=0, Conditions=[endured])` - "the setup of the rule has to pass by
  itself", :3878); :2764-2770 are `expect` cases (fine). Epitaph test :616-632 -> :622-639 (`eulogies` :637). Walker pin :3887.

Docs:
- H: §2.7 :184-195 (row :192, no `EpithetRow` :193); §2.8 :196-205 (fields :200, no Resolve/Delay :201, parity :203); §5.2
  :323-335 (shared rules and '약 22분' :325 - the brief's "H:328"; B row :330; before [B-6b8b] :334); §5.3 :336-355 (:338,
  ② :343, ③ :344, C = 6,000 :351, record-only :354); §8 rows :391 [B-6b5], :394 [B-6b8], :395 [B-6b8b]; rule :403; §9
  :410-412; §11 :461-472 (last line [B-6b7] :472; new line goes after it, before the blank :473 and `## 추가 이야기와 보완`
  :474). The brief's H:472/H:474 (#165 revisit, short shape) now sit in §10-§11 text - find by words.
- N: §1.3 :425 ("까닭부터"); `Sim.WandererLoop` row :443; `Sim.PhaseTwoExit` row :447; §1.5 :473-526 (C6 :515-520, C7
  :521-526); ⓗ N:573 -> **:658**; N §2.1 '증언 성공·실패·물러남' N:586 -> **:671**; N §2.2 baseline table N:599 -> :676-690
  ([B-6b8] row **:688**); bundle C's B1-guard reading N:614 -> **:699**; '[B-6b6]의 다시 셈' :621-637 (t :636); §3 '묶음 A'
  :707-722, '묶음 A′' :724-739, '묶음 B' :741-743 (empty), '묶음 C' :745-747; §5 [B-6b7] block :862-874 (end of file :874 -
  the [B-6b8] block goes after it).

## Changes since the brief was written ([B-6b7] 220129d) - folded in

- **Tripwire**: pins eight names now (above); this commit makes it ten (§2.1 stands).
- **`Sim.PhaseTwoExit`** already runs on `CosmicTests::WithoutB6bData(Original)` (`Tests/CosmicSimTests.cpp:2284-2289`);
  the switch takes the era and witness rows out too, so §2.4's PhaseTwoExit row is moot - keep the test so. Finding a seed
  that works on the full game data is [B-6b12]'s.
- **`Feats.Reachable`** (`Tests/CosmicAscensionTests.cpp:1760`) now pins an AI fifth rung on the full game data
  (:1779-1781, input-less seed 42, 20,000). §2.4's "WithoutB6bData outermost" would take out the rows it pins: if the era or
  the witness moves it, find the cause and record it (N §5) as for `Sim.WandererLoop` - never switch B6b data off there.
  (Input-less seed 42 deposes its ruler at about 1,072, so the era needs 600 drained ticks before that; a witness needs a
  walking vessel on a claimed rung-1/2 system - both unlikely; the probe tells.)
- **Tables**: DT_Edicts 10 rows, DT_Feats 5 rows (after this commit DT_Events 13, DT_Encounters 19).
- **[B-6b7]'s hand-offs to bundle B** (N §3 '묶음 A′', N §5 [B-6b7]): **(L4-2)** read the ruled fourth->fifth season as
  measured (not the model) in judgment ③, counting with each law break the ticks the laws stood empty; **(L4-3)** record the
  ruled people's third-rung workload under AI-held home-galaxy laws per 600-tick window (decisions and cards in the window
  'first AI holds the laws -> earliest of ruled T4 / record / 6,000') as a **record-only** item if the bundle's lines give
  it, else '재지 않음'. **#136 stays with bundle C** ([B-6b11]).
- **Ark counter** (§2.3 item 4): [B-6b7] did not add one (no 'ark standing' counter in the commandlet) -> bundle B writes
  "방주를 둔 채 만료 — 재지 않음 (카운터 없음)".
- **Harness**: `bundleB.py` already back-ported [B-6b9]'s F4 guard (self-test section 14: "--check-only writes nothing under
  --root or into an archived copy"); `--summary-dir` is required for `--check-only` on an archive; `--t-death 5000`;
  self-test 135/135 (`selftest_out.txt`, rerun 2026-11-10 04:02 by the harness stage, which is working in this folder at the same time).
- **Baselines (lesson 3)**: after a runner with `-UpdateBaseline`, put `Tools/baselines/seed42.json` back to HEAD's shape
  (indent 4, LF, final newline, no BOM, no ') with an adaptation of `scratchpad\b6b7\fix_seed42.py` into
  `scratchpad\b6b8\` (OLD = the five `<B6b7_KEYS>` above; NEW = the probe's `b6b8_on` lines; compare the five whole lines
  with `scratchpad\b6b8\check_base.py <root> Saved\Balance\b6b8_on`).
- **OPEN_QUESTIONS**: #164-#166 are reserved for [B-6b12], #167 is taken - a new question takes #168 on. Records are dated
  2026-11-10.

## ORDER CHANGE (lead decision - record it in N §5 [B-6b8] as a deviation from brief §7)

The C++, asset-rebuild and commandlet mutations run **before** the final runner and bundle B, so that a test fixed for a
surviving mutation is already in the build bundle B runs on; after bundle B only records and analysis change. The
[B-6b8b] design already exists (`scratchpad\b6b8b\brief_final.md`) - do not start one. §5 'Running' and §7 'Order' below
are rewritten accordingly (marked [WORKING COPY]).

## Lesson 6 - tests beyond §2.4's table that run game data long or put a wanderer on a claim (tier = runner tier)

Heuristic over every test body (`scratchpad\b6b8\grep_long_tests.py`, `grep_tests_detail.py`; output
`grep_long_tests_out.txt`). "Era" = a ruled people may endure 600 drained ticks on the game's DT_Events; "witness" = a walking
vessel may stand on a claimed system on the game's DT_Encounters (presentation only - no test answers it).

Smoke tier (run by the probe's smoke pass):
- `Sim.KnowingInert` (`CosmicKnowingTests.cpp:1567`) - game tables (`WithRules(Original)`), eight 20,000-tick runs: seed 42 with
  the inherited edict revoked (a ruled people kept for the whole run - **era likely**), a wanderer standing in its birth
  system (**witness possible**), seed 18 deposed (walks). Compares world hash and sim stream between runs that differ only in
  sight/telling - should hold if the era and the witness are blind to knowing; L2 reads it.
- `Sim.Determinism` (`CosmicSimTests.cpp:508`, 3,000 x2), `Sim.SaveNeutral` (`CosmicSaveSlotTests.cpp:501`, 2,000),
  `Sim.SaveSlotForm` (:130, 1,000), `Sim.SaveRoundTrip` (`CosmicSimTests.cpp:1786`, 1,000 + 500 + 500): game data, seed 42 ruler
  deposed at about 1,070 - self-comparisons; era only if 600 drained ruled ticks come first.
- `Data.Integrity` - the tripwire (an intended change).

Full tier only (the final runner):
- `Feats.Reachable` (`CosmicAscensionTests.cpp:1760`, 20,000) - see above.
- `Log.NoConsecutiveRepeat` (`CosmicLogTests.cpp:42`, 20,000 seed 42) - no sentence twice in a row: the era line has three
  variants and fires once per stretch; watch it.
- `Sim.NotablesFromEventsOnly` (`CosmicHazardTests.cpp:2984`, 20,000 seed 42), `Sim.NoNaN` (`CosmicSimTests.cpp:591`, 10,000
  seed 7), `Sim.TellStreamSeparate` (:1661, 3,000), `Sim.EndingRecordOnce` (`CosmicEndingTests.cpp:655`, 6,005),
  `Sim.StartBlockReasons` (`CosmicAscensionTests.cpp:951`, 2,000 at :1131-1134), `Sim.PlayerAscendsToT3` (:1785, ruled people
  to T3), `UI.Chronicle` (`CosmicChronicleScreenTests.cpp:600`, 1,500 - counts chronicle lines; its fifth-rung part is on
  `WithoutB6bData`), `Sim.HashRun` (`CosmicHashRunTests.cpp:59`, ruler policy to the second rung), `Sim.FirstContactTimeline`
  (`CosmicKnowingTests.cpp:1775`, seeds 1-50), `Sim.LayersInRange` (1,000), `Sim.FrameStep` (734), `Events.NotableDrawFixed`
  (`CosmicEventTests.cpp:1880`, 1,000) - era possible.
- `Sim.PetitionPresent` (`CosmicPetitionTests.cpp:139`) §3 :302-318 - 700 ticks with and without the game's encounters, world
  hash equal: no wanderer before 700 (seed 42 deposed about 1,070), the era is in DT_Events on both sides - expect unchanged.
- **Witness possible** (game DT_Encounters kept, vessel set to Wanderer on a claimed system): `Log.ClaimVoice`
  (`CosmicHazardTests.cpp:1767`; `TablesWith` keeps `Original.Encounters`, wanderer at :1964 and :2033 on claimed system 2)
  and `UI.SpiritualSight` (`CosmicSpiritualSightTests.cpp:228`; wanderer :508/:962, claims set by hand). A presented witness
  fills `Knowing.PendingPetitionRow` - check what each asserts after the claim.
- Not reached (own DT_Encounters/DT_Events or B6b data off): `Sim.AwarenessOnce` and `UI.ClaimMayFollow` (empty
  DT_Encounters), `Sim.ExtinctionExpiry` (Encounters nullptr), `Sim.SightWalkMoved`, `Sim.PhaseTwoExit` (`WithoutB6bData`),
  `Events.RowOrderInert` (own DT_Events).

---

"""

REPL = [
    # section 0 placeholders
    ("   - **`<T_DEATH>`**: from N §2,", "   - **`<T_DEATH>`** [= **5000**, N:636]: from N §2,"),
    ("   - **`<N>`, `<P_bind>`, `<r>`**: from [H‴] data", "   - **`<N>`, `<P_bind>`, `<r>`** [= **1200 / 1800 / 0.375**]: from [H‴] data"),
    ("   - **`<A_SEC_PER_20K>`**: bundle A's measured wall time", "   - **`<A_SEC_PER_20K>`** [= **11 s**; see the header table]: bundle A's measured wall time"),
    ("   - **`<B6b7_KEYS>`**: the five lines", "   - **`<B6b7_KEYS>`** [= the five lines in the header]: the five lines"),
    ("8. **Clean build first** of both targets on a clean [H‴]. Earlier workflows end with mutation runs.",
     "8. **Clean build first** of both targets on a clean [H‴]. Earlier workflows end with mutation runs. [WORKING COPY: not needed - "
     "the binaries are [B-6b7]'s clean Rebuild and `b6b7_on`'s keys equal seed42.json, so item 9 is a copy]"),
    ("   - each key must equal `<B6b7_KEYS>`.",
     "   - each key must equal `<B6b7_KEYS>`. [WORKING COPY: done as a byte copy of `Saved\\Balance\\b6b7_on\\` -> `b6b8_base\\` "
     "(22 files), whose five keys equal `<B6b7_KEYS>` letter for letter]"),
    ("10. **Tripwire at [H‴]** (after [B-6b7]): the `Data.Integrity` pin reads the eight names before `EventEraEndured` and `EncWitness`. If it does not, stop.",
     "10. **Tripwire at [H‴]** (after [B-6b7]): the `Data.Integrity` pin reads the eight names before `EventEraEndured` and `EncWitness`. If it does not, stop. "
     "[WORKING COPY: confirmed - `CosmicDataTests.cpp:2775-2779`]"),
    # section 1.3
    ("- Edit `Tests/CosmicB6bRowNames.h:32` and `:35`:", "- Edit `Tests/CosmicB6bRowNames.h:32` and `:35` [HEAD: :33 and :36]:"),
    # section 2.1
    ("- Assert `Today == Ten`. `Ten` is the existing array (HEAD :2704-2706)",
     "- Assert `Today == Ten`. `Ten` is the existing array (HEAD :2704-2706) [220129d: :2771-2773; the pin :2774-2779]"),
    # section 2.4
    ("| `Sim.PhaseTwoExit` (input-less, 20,000) | The era before the deposition (world); the witness before the deposed vessel (knowing only) | Same as `Sim.WandererLoop`: never switch off B6b data; find the cause and record it in H §11 and N §5 |",
     "| `Sim.PhaseTwoExit` (input-less, 20,000) | The era before the deposition (world); the witness before the deposed vessel (knowing only) | Same as `Sim.WandererLoop`: never switch off B6b data; find the cause and record it in H §11 and N §5 "
     "[WORKING COPY: moot - since [B-6b7] the test runs on `CosmicTests::WithoutB6bData(Original)` (`CosmicSimTests.cpp:2284-2289`), which takes the era and witness rows out too; keep it so - a seed that works on full game data is [B-6b12]'s] |"),
    ("| `Feats.Reachable` (seed 42 at 20,000; non-ruled tiers) | An early era moves reverence | Re-run. Its subject is not B6b, so `WithoutB6bData` outermost if the cause is the era or the witness |",
     "| `Feats.Reachable` (seed 42 at 20,000; non-ruled tiers) | An early era moves reverence | Re-run. Its subject is not B6b, so `WithoutB6bData` outermost if the cause is the era or the witness "
     "[WORKING COPY: since [B-6b7] it pins an AI fifth rung on the full data (`CosmicAscensionTests.cpp:1779-1781`), so the switch would take out what it pins: on a break find the cause and record it as for `Sim.WandererLoop`] |"),
    ("| Any other test that runs a ruled people under pressure for ≥ 600 ticks",
     "| [WORKING COPY: the grep's list is in the header, 'Lesson 6'] Any other test that runs a ruled people under pressure for ≥ 600 ticks"),
    # section 4.1
    ("| death | R + `-DeathAt=<T_DEATH>` (H taken literally",
     "| death | R + `-DeathAt=5000` [`<T_DEATH>`] (H taken literally"),
    ("- **Time:** H:328's \"약 22분\" is too low. The bundle is 10.8 M ticks against bundle A's 3.12 M, so `<B_MINUTES>` = `<A_SEC_PER_20K>` × 540 / workers / 60, which comes to about 35-42 min at 6 workers. Record the measured value in N §3.",
     "- **Time:** H:328's \"약 22분\" is too low. The bundle is 10.8 M ticks against bundle A's 3.12 M, so `<B_MINUTES>` = `<A_SEC_PER_20K>` × 540 / workers / 60, which comes to about 35-42 min at 6 workers. Record the measured value in N §3. "
     "[WORKING COPY: '약 22분' is at H:325. Filled: 11 s x 540 / 6 / 60 = **about 16.5 min** (18 min at A′'s 12 s; 23.5 min at most by the header line's tick share) - each run carries about 6-7 s of engine start, so the 35-42 min estimate and 'too low' do not hold; H's 22 min is inside the range]"),
    ("Its control is the calling line \"death asked at `<T_DEATH>` (recorded r)\".",
     "Its control is the calling line \"death asked at `<T_DEATH>` (recorded r)\" [= \"death asked at 5000\"]."),
    # section 4.2
    ("- Also match `AIUpperRungProgressMultiplier 1 (was <r>), FleetDelayCountdownShare 0 (was 0)` (`CosmicBalanceCommandlet.cpp:1596`).",
     "- Also match `AIUpperRungProgressMultiplier 1 (was 0.375), FleetDelayCountdownShare 0 (was 0)` (`CosmicBalanceCommandlet.cpp:1596` [HEAD :1597])."),
    # section 2.3 item 4
    ("   - If [B-6b7] already added it, read it in bundle B.",
     "   - If [B-6b7] already added it, read it in bundle B. [WORKING COPY: it did not - no such counter at HEAD]"),
    # section 5 running
    ("- **C++, asset-rebuild and commandlet patches** run serially after bundle B, followed by a clean rebuild of both targets.",
     "- **C++, asset-rebuild and commandlet patches** run serially after bundle B, followed by a clean rebuild of both targets. "
     "[WORKING COPY - ORDER CHANGE: they run serially **before** the final runner and bundle B (after the review fixes), each followed by its "
     "restore; then `apply_after`, retakes, README and one clean rebuild of both targets; the final runner then runs on that build and bundle B on the "
     "same build. `b6b8_cmdlet_witness_first_any_card` is checked with the `bundleB.py --max-seed 2` wanderer smoke at that point]"),
    ("| `b6b8_cmdlet_witness_first_any_card` (`WitnessFirst` set on any card's rise) | the `bundleB.py --max-seed 2` smoke on the wanderer cell (its W controls stop it); run serially in step 9, then a clean rebuild |",
     "| `b6b8_cmdlet_witness_first_any_card` (`WitnessFirst` set on any card's rise) | the `bundleB.py --max-seed 2` smoke on the wanderer cell (its W controls stop it); run serially in step 9, then a clean rebuild "
     "[WORKING COPY: in the mutation step before the final runner] |"),
    # section 7 order
    ("6. **Final runner pass** `-Tier full -Data -UpdateBaseline`, with the editor closed.",
     "6. [WORKING COPY - ORDER CHANGE: first the **C++, asset and commandlet mutations** serially (old step 9: new patches plus "
     "the rerun set; a surviving one is fixed in the test, never the patch, and the fix repeats step 3's build), then `apply_after`, "
     "retakes, README, and one clean rebuild of both targets; then:] **Final runner pass** `-Tier full -Data -UpdateBaseline`, with the editor closed."),
    ("8. **Judgments**, then **L5, records and docs**, with a rebut-verify per finding.",
     "8. **Judgments**, then **L5, records and docs**, with a rebut-verify per finding. [WORKING COPY: the [B-6b8b] design in step 7 "
     "already exists - `scratchpad\\b6b8b\\brief_final.md`; do not start one]"),
    ("9. **C++, asset and commandlet mutations**, serially; then `apply_after`, retakes, README, and a clean rebuild.",
     "9. ~~**C++, asset and commandlet mutations**, serially; then `apply_after`, retakes, README, and a clean rebuild.~~ "
     "[WORKING COPY - ORDER CHANGE: moved before step 6; nothing here]"),
    # section 6 H line date
    ("> - `[B-6b8]`(2026-11-xx):", "> - `[B-6b8]`(2026-11-10):"),
]


def main():
    text = open(SRC, encoding="utf-8").read()
    for old, new in REPL:
        n = text.count(old)
        if n != 1:
            raise SystemExit("replacement not unique (%d): %r" % (n, old[:90]))
        text = text.replace(old, new)
    out = HEADER + text
    if not out.endswith("\n"):
        out += "\n"
    with open(DST, "w", encoding="utf-8", newline="\n") as f:
        f.write(out)
    b = open(DST, "rb").read()
    assert b"\r" not in b and not b.startswith(b"\xef\xbb\xbf")
    print("written", DST, len(b), "bytes")


if __name__ == "__main__":
    main()
