# [B-6b8b] brief - WORKING COPY (start stage done 2026-11-10; this copy wins over brief_final.md where they differ)

{H8B} = HEAD `84f0d36` [B-6b8] (pushed) on `220129d` [B-6b7], `6ae9060` [B-6b6], `10205b6` [B-6b5], `529cab9` [B-6b4],
`cef8cc7` [B-6b3b]. The brief was written read-only at `cef8cc7`. In the body below (after the `---`) every line number
that moved is put in place (marked "{H8B}" or "[WC: ...]"); find every edit by its text anyway.

## Drift from the brief (read first)

1. **ORDER CHANGE (lead decision, record it in N §5 `[B-6b8b]` under '헤더와 다르게 하거나 더한 것' or the 변이 line):
   mutations run BEFORE the one final runner, as in `[B-6b8]`.** New §7 order:
   1. §0 (this stage - done);
   2. I1 data and docs, I2 tests and patches (parallel);
   3. build pass `run_checks.ps1 -Tier smoke -Data -UpdateBaseline` (then `fix_seed42.py` with no flags);
   4. machine: check (b), optional (c), bundle K, then (d)/(e) - in parallel with the lenses L1-L3;
   5. fix (a data or sim fix repeats 3-4);
   6. **mutations (§5), serially** - the 3 new patches and the 9 runnable reruns (below), then `apply_after.txt`, README
      rows, retakes if any, and a clean rebuild of both targets and the assets (so the measured build is the committed build);
   7. **the one final runner** `-Tier full -Data -UpdateBaseline` (its two in-process passes are the two consecutive test
      runs; its five lines must equal the build pass's and bundle K's on-side lines), then `fix_seed42.py` (no flags);
   8. records (drafted in parallel with 6-7), `python -I Tools/check_docs.py`, `wc -c` on H; the lead commits and pushes.
   §5's "after the final runner, never alongside a commandlet" becomes "before the final runner, never alongside a
   commandlet or bundle K"; the clean rebuild after the mutations stays.
2. **The H §11 line must be the SHORT variant (risk 3 applies now, not "maybe").** `{H_BYTES0}` = **148,661** bytes
   (brief guessed ~148,700). The long bit-identical line is 388 bytes -> 149,049 >= the self-imposed 149,000; the moved
   variant is longer still. Use "- `[B-6b8b]`(2026-11-10): #132 몫 0.5(결정 23) — 각성만 75, W 그대로. 세부 노트 §5."
   (100 bytes -> 148,761) and move the rest (the policy guards after a crowning or return and one tick past a reign's end;
   the §5.2 citation `:686-720` -> `ActDiplomacy` (1) `:733-780`; the five lines' result) to N §5. H §8's own 150,000
   holds either way.
3. **ⓔ's plague line is stale since `[B-6b7]`** (not this commit's doing): `edict_wide_warning` (`DT_Edicts.json:180`)
   adds `Hazard.Plague` +50 (and `Hazard.AIAwakening` +50), so the plague's longest stand is 200 + 100 + 50 = **350**,
   35 slots, 0.98^35 = **0.4931** (ⓔ says 300 / 0.545 그대로). ⓔ's own bound for a plague bonus (B ≤ 380) holds (B = 50).
   The claim line is exactly ⓔ's (550 / 55 / 0.2629 / W 1.2445). The commit body still quotes ⓔ verbatim; put the plague
   correction in the "줄 번호(지금)" line (③) and in N §5 ③, and use the 350 / 0.4931 in `{W_TABLE}` and DECISIONS_LOG.
4. **Mutation rerun set re-derived (9 runnable, same count as the brief, different members):**
   - DT_Rules patches at {H8B} (`git grep -l 'b/Tools/data/DT_Rules.json' -- Tools/mutations`): `B6b/b6b2a_data_laws_knob_four`
     (applies), `B6b/b6b7_data_ark_margin_half` (applies, `[B-6b7]` retake of `b6b2c_data_ark_margin_half`),
     `B6b/b6b7_data_upper_rung_not_binary` (applies, `[B-6b7]` addition), `B6b/b6b2c_data_ark_margin_half` (**does not apply
     since `[B-6b7]`** - README `:793` already says so; not rerun), `B4/b4d2_data_resist_factor` (does not apply since
     `[B-6b2a]` - mark README `:399` as §5 says; not rerun).
   - README rows whose Breaks is `Sim.FleetDelay` (6, unchanged): `B3/b3b_delay_on_rebellion` (`:149`), `b3b_delay_unguarded`
     (`:150`), `b3b_delay_ignores_gap` (`:151`), `b3b_delay_outside_cut` (`:152`), `B6b/b6b2c_guard_cap_ignored` (`:791`),
     `B6b/b6b2c_guard_cap_no_fleet_cap` (`:792`).
   - `[B-6b6]`-`[B-6b8]` additions: none to the Sim.FleetDelay set; to the DT_Rules set the two `b6b7_` above. No patch touches
     `CosmicDiplomacyTests.cpp` (0) or `Data/CosmicDefinitionRows.h` (0).
   - None of the three runnable DT_Rules patches has a hunk on `:27` (hunks: `b6b7_data_ark_margin_half` `:13-19`,
     `b6b7_data_upper_rung_not_binary` `:14-20`, `b6b2a_data_laws_knob_four` `:98-103`), so no retake is expected.
5. **The commandlet's `-WithoutB6bData` line text changes** after the edit: "FleetDelayCountdownShare 0 (was 0)" becomes
   "(was 0.5)" (`CosmicBalanceCommandlet.cpp:1599`). Nothing in the repo greps that text; a harness or check (c) that
   expects "(was 0)" must accept "(was 0.5)".
6. **Tree state.** `git status` lists 236 ' M' files plus untracked `docs/OVERVIEW.html` (the user's - never touch). The
   ' M' entries are stat-only noise (OneDrive / the system `core.autocrlf=true` warning; `.git/config` has false): `git diff
   HEAD --name-only` is empty, `git diff --ignore-cr-at-eol` is empty, and `git hash-object` equals the HEAD blob on the files
   checked (as `[B-6b8]`'s working copy recorded: "237 ' M' entries are OneDrive stat noise"). Content-clean = §0.1/§0.6
   hold. Judge "clean" with `git diff HEAD --name-only` (empty), not `git status --short`.
7. Anchors that moved (all others hold; full table below): `CosmicDataTests.cpp` (+44 / +66), `CosmicBalanceCommandlet.cpp`
   (+3 after `:645`, +5 after `:2950`), the panel `:1098 -> :1134`, `CosmicDiplomacyUITests.cpp:701 -> :798`, `schema.json`
   `:153 -> :178`, `test_data_schema.py:1493 -> :1500`, README `:791/:798/:800 -> :792/:799/:801` (see table), `run_checks.ps1`
   ±1, H §11 end `:468 -> :473` (`## 추가 이야기와 보완` `:470 -> :475`), N ⓔ `:562-567 -> :648-653`, N §2.2 `:600 -> :686`,
   N §2.3 `:613 -> :699`, DECISIONS_LOG last B6b line `:37 -> :42` (B6a heading `:39 -> :44`). `Sim/`, `Data/`, `Player/`,
   `World/CosmicRulerPolicy.*`, `CosmicDiplomacyTests.cpp`, `CosmicTestHelpers.h` line numbers, and every doc sentence the
   commit edits are at the brief's numbers.
8. `docs/50_ROADMAP.md:12` has a B7 entry ("→ B7 AI 백성 5행·…" up to "→ B8 서막 문안"). Put the drain line at its end:
   replace the unique "`#145(B6a에서 잰 0)) → B8`" with "`#145(B6a에서 잰 0); B6b가 넘긴 것 — 영향권 안 소모(#132에서
   넘어옴 — 넷째 칸 이상에서 일어나지 않고 AI는 지키지 않는다; B7이 잰다)) → B8`".
9. ④ of N §5's "헤더와 다르게 하거나 더한 것" ("B6b block ends at `:37`, not `:39`") is stale as written: at {H8B} the B6b
   block ends at `:42`. Write it with the {H8B} number or drop it (no doc cites the B6b block's end by line; N:864's
   `DECISIONS_LOG.md:39` is a `[B-6b5]` record).

## Filled placeholders

| Placeholder | Value | Source |
|---|---|---|
| `{H8B}` | `84f0d36` ([B-6b8], pushed) | `git log` |
| `{H_BYTES0}` | **148,661** bytes | `wc -c docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` on {H8B} |
| `{PRED}` | **all five runs predicted bit-identical** -> the "bit-identical" record variants of §6 | `Saved/Logs/checks*.log` of the `[B-6b8]` final runner (07:36-07:38, N §5 `[B-6b8]`: final full runner 07:29-07:38; their five summary lines equal `seed42.json` at {H8B}): every log prints `delay bought per tag []`, `guarded crises 0 by tag []` and `#132 by the record: guarded at the row's cap by tag []` (no `Hazard.AIAwakening` entry anywhere); 0 `end check` / `race check` lines |
| `{PLAGUE_BONUS_SUM}` | **50** (350 - 300; `edict_wide_warning`) - ≤ 380 holds | probe |
| `{CLAIM_BONUS}` | **100** (`edict_vigil`) | probe |
| `{CARD_DELAY_CLAIM}` | **50** (`petition_pressure_measured`) | probe |
| `{SKIPPED_ROWS}` | `crisis_ai_awakening`, `crisis_pre_supernova`, `crisis_rebellion`, `crisis_species_extinction` (no resolving non-grace event row; the ark is a megaproject) | probe |
| `{W_TABLE}` | below | `python -I w_recount.py Tools\data 0.0` / `0.5` / `0.3` on {H8B} |
| `{B6b8_BASELINE}` | below | `git show HEAD:Tools/baselines/seed42.json` ({H8B}) |
| `{NE_TEXT}` | below (N `:648-653` at {H8B}, byte-identical to `cef8cc7:562-567`) | N |
| `{DATE}` | 2026-11-10 (unless the commit slips past midnight) | - |
| `{MOVED_RUNS}`, `{NEW_HASHES}`, `{FIRST_MOVED_TICK_*}`, `{K_MOVED_R}`, `{K_MOVED_W}` | after the measurements | - |

### `{W_TABLE}` (Python copy of `TheodicyProblems`, `Tests/CosmicDataTests.cpp:2521-2612` at {H8B}; the C++ invariant is the judge)

| Share | Claim (`crisis_claim`) | Plague (`crisis_plague`) |
|---|---|---|
| 0 ({H8B} as is) | 300 + 100 + 100 + 50 = **550** ticks, 55 slots, 0.0240 a slot, stays **0.2629**, W **1.2445**, lock 600 ≥ 550 | 200 + 100 + 50 + 0 = 350, 35 slots, 0.0200 a slot, stays 0.4931 |
| **0.5 (this commit)** | **550**, 55 slots, **0.2629**, W **1.2445**, lock 600 ≥ 550 (cap min(100, 150) = 100) | 350, 35 slots, 0.4931 (cap min(100, 100) = 100) |
| 0.3 (`b6b8b_data_share_third`) | 300 + **90** + 100 + 50 = **540**, 54 slots, 0.2693 -> the 549 board case reports nothing and its `Expect` fails | 200 + 60 + 50 = 310, 31 slots, 0.5346 |
| 0.5 without `FleetDelayCap` (`b6b2c_guard_cap_no_fleet_cap`) | 300 + **150** + 100 + 50 = **600**, 60 slots, 0.976^60 = **0.2328 < 0.25** -> `Data.Integrity` (game theodicy) fails; the 549 case reports 600, the 550 case fails (lock 550 < 600) | 350 (0.5 x 200 = 100), unchanged |

Skipped by the invariant at every share: awakening, pre-supernova, rebellion, extinction. The galaxy laws of `[B-6b7]` enter
as one more edict row each (`edict_wide_warning`: awakening +50, plague +50 - the plague's 350); `enc_witness` (`[B-6b8]`)
has no `DelayHazard` on the claim, so the card delay stays 50. Claim line = 550 / 0.2629 at {H8B}: **the stop condition
does not fire.**

One line for DECISIONS_LOG's "{W_TABLE의 한 줄}": "(역병은 `[B-6b7]`의 경보망 덤 50으로 350틱·35슬롯·남는 몫 0.4931 — 몫과 상관없이 그대로; 각성·초신성 전조·반란·절멸은 해결하는 비은혜 행이 없어 건너뛴다)".

### `{B6b8_BASELINE}` (the `wrote …` tail removed, as `Remove-WrotePath` does)

{{BASELINE}}

`last_recorded_in_changelog.task` at {H8B} = "B6b B-6b7 (data 2 fourth to fifth: …)" (`[B-6b8]` was bit-identical and
kept it); in the bit-identical case it stays and only `verified_at` changes. The hash runs at {H8B}: `hashrun.log` =
input-less 2557850423 / 2906170550, `hashrun_policy.log` = ruler 1189870756 / 3359656751 (= the commandlet's 3,000-tick lines).

### `{NE_TEXT}` (N `:648-653` at {H8B}, verbatim - quote it unedited in the commit body)

{{NE_TEXT}}

## Start-stage results (brief §0, items 1-6)

1. **Commits, tree, processes.** `git log --oneline -8`: 84f0d36 [B-6b8], 220129d [B-6b7], 6ae9060 [B-6b6], 10205b6 [B-6b5],
   529cab9 [B-6b4], cef8cc7 [B-6b3b], 2a90efc [B-6b3a], 71cb3ca [B-6b2c]. No `[B-6b9]` (bundle B's ① ② ③ passed, ④ gave
   C 9,000 - no B′). Tree content-clean (drift 6). No UnrealEditor / UnrealBuildTool / dotnet / ShaderCompileWorker /
   Build.bat process (Get-CimInstance Win32_Process).
2. **Line numbers re-derived** - table below; the body carries them.
3. **Knob hits.** `hits_head.txt` (`git grep -n FleetDelayCountdownShare cef8cc7`, 34 lines) and `hits_h8b.txt` (same on
   `84f0d36`, 36 lines), diffed by file and text: the only new hits are 2 lines of N (the `[B-6b8]` measurement and 나눔
   lines); 4 doc lines changed text (STATUS:7, docs/20:289, docs/21:1463, docs/40:508) with the same file counts. **No new
   non-comment `.cpp`/`.h` hit** - the code readers are the brief's (moved only by line): `CosmicConflict.cpp:401`
   (`GuardDelayCapOf`), commandlet `:1597/:1599` (`-WithoutB6bData`) and `race:` AtCap `:2698` (calls `GuardDelayCapOf`),
   `CosmicTestHelpers.h:363`, `CosmicDataTests.cpp:2782/:2845`, `CosmicDiplomacyTests.cpp:1480/:1487`; comments
   `CosmicConflict.h:151`, `CosmicBalanceCommandlet.h:124`, `CosmicTestHelpers.h:351`; the field `CosmicDefinitionRows.h:994`.
   `DT_Rules.json:27` holds `0.0`. Stop condition not met.
4. **Placeholders** - filled above.
5. **Mutation apply state.** Scratch copy of {H8B} (`git -c core.eol=lf archive 84f0d36 Source Tools` ->
   `scratchpad\b6b8b\apply_head_copy`, byte-identical to the working tree), `git apply --check` of every patch
   (`scratchpad\b6b8b\apply_state.py`): **1,075 patches, 928 apply, 147 fail** -> `scratchpad\b6b8b\apply_before.txt`. It is
   line-for-line `scratchpad\b6b8\apply_after.txt` (the `[B-6b8]` end state). Use `apply_state.py <root> <copy of the new
   tree> apply_after.txt` for the after state.
6. **Clean build.** No leftover mutation (content = HEAD). The editor DLL `Binaries/Win64/UnrealEditor-Cosmic_Ascension.dll`
   sha1 `07204afab949` (2026-11-10 06:47) = `[B-6b8]`'s final build (`scratchpad\b6b8\final_build_sha1.txt`); `generated.sha1`
   and the `Content/CA/Data` assets equal HEAD.

## Re-derived anchors (`cef8cc7` -> {H8B} `84f0d36`)

| Brief (HEAD `cef8cc7`) | {H8B} | Note |
|---|---|---|
| `DT_Rules.json:27` | `:27` | `"FleetDelayCountdownShare": 0.0,` |
| `DT_Rules.json:37/:38/:18` (ⓔ), `:89`, `:102` | `:39/:40/:19`, `:89`, `:102` | EventSlotTicks / EventChancePerSlot / ThroneLockTicks |
| DT_Rules hunks `:13-19`, `:97-103` | `b6b7_data_ark_margin_half` `:13-19`, `b6b7_data_upper_rung_not_binary` `:14-20`, `b6b2a_data_laws_knob_four` `:98-103` (`b4d2` `:97-102` dead) | none touches `:27` |
| `DT_Hazards.json:7` | `:7` | awakening 150 |
| `Tests/CosmicDiplomacyTests.cpp:1406-1512`, block `:1474-1510`, `:1480/:1481/:1483`, `:112`, `:134-154` | same | file unchanged; comment `:1406-1410`, test `:1411-1514`; LayBoard `:132-156` |
| `CosmicDataTests.cpp:2477-2568` | `:2521-2612` (comment `:2503-2520`) | lambda unchanged |
| `:2529` (GuardDelayCapOf), `:2530-2535` (edict loop) | `:2573`, `:2574-2579` | |
| `:2822` (game run) | `:2888` | |
| `:2948-2967` (board), `:2957-2963` (549/550) | `:3014-3033`, `:3023-3029` | |
| `:2716/:2779` | `:2782/:2845` | |
| `Data/CosmicDefinitionRows.h:988-994`, `:992` | same | `:992` = "// 0..1 (schema). Off (0) until [B-6b8b] sets its value." |
| `Sim/CosmicConflict.cpp:366-412` (`:369`, `:398-408`, `:402/:403`, `:411`) | same | |
| `Sim/CosmicHazards.cpp:219`; `GalaxySimulationSubsystem.cpp:2451-2474`, `:3005-3011`, `:2217`, `:2274`; `PlayerRoleSubsystem.cpp:1189` | same | Sim/Player unchanged |
| `World/CosmicRulerPolicy.cpp:686-708`, `:711`, `:719`, `:733-780` | same | |
| `UI/CosmicSystemPanelWidget.cpp:1098` | `:1134` | |
| `CosmicDiplomacyUITests.cpp:701` | `:798` | |
| `CosmicTestHelpers.h:222`, `:351`, `:363`; `CosmicDataTables.h:97` | same | |
| `CosmicDeathTests.cpp:550` | `:551` | |
| `CosmicWandererTests.cpp:1370-1386`, `CosmicPressureCardTests.cpp:713-815`, `CosmicHashRunTests.cpp:201/228`, `CosmicEndingTests.cpp:158`, `CosmicKnowingTests.cpp:1875`, `CosmicSuccessionTests.cpp:631` | same | files unchanged |
| commandlet `:1365-1390`, `:1482`, `:1594/:1596`, `:2676-2677`, `:2691-2696`, `:2835`, `:3700-3715`, `:3819-3821` | `:1368-1393` (log lines `:1383/:1386`), `:1485`, `:1597/:1599`, `:2679-2680`, `:2694-2699`, `:2838`, `:3705-3720`, `:3824-3826` | |
| `run_checks.ps1:76`, `:116-132`, `:139-155` | `:76`, `:115-132`, `:140-156` (steps `:155/:156`) | |
| `ST_CA.csv:339-340` | same | `Guard.Delay`, `Guard.DelayTip` |
| `schema.json:153`, `test_data_schema.py:1493` | `:178`, `:1500` | |
| README `:5`, `:398` (b4d2), `:791` (no_fleet_cap), `:800` | `:5`, `:399`, `:792`, `:801` | `:791` = `b6b2c_guard_cap_ignored`; B3 delay rows `:149-152` |
| `docs/20:289`, `docs/21:1199/:1518`, `docs/40:237/:366`, `systems.md:78`, `docs/30:30` | same | all target strings found verbatim |
| `OPEN_QUESTIONS.md:101` (#132), `:108` (#142) | same | |
| `STATUS.md:7/:27/:32/:38/:40` | same | target strings found verbatim at 7/38/40 |
| H `:334`, `:403`, §2.10 `:220` | same | `:220` holds `:366-392`, `:395-405`, `:1084` |
| H §11 last bullet `:468`, `## 추가 이야기와 보완` `:470` | `:473` (`[B-6b8]`), `:475` | |
| N ⓔ `:562-567`; N §0.2 `:195`; N §2.2 `:600`; N §2.3 `:613`; N §5 end | `:648-653`; `:195`; `:686`; `:699`; `:908` (last line of the `[B-6b8]` block `:896-908`) | |
| DECISIONS_LOG last B6b line `:37`, B6a heading `:39` | `:42`, `:44` | |
| `docs/50_ROADMAP.md:12` B7 entry | exists (chars ~13,422-14,484 of the line) | drift 8 |
| `docs/15_GLOSSARY.md:89` | same | left to `[B-6b12]` |
| `scratchpad\b6b6\bundleA.py:47` | same | `COMMON_ARGS = ["-Rules=ProvisionalEndTick:0"]` |
