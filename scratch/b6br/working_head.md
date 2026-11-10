# [B-6br] brief - WORKING COPY (start stage, 2026-11-10; this copy wins over brief_final.md where they differ)

{HBR} = HEAD `8b6c5a7` [B-6b8b] (pushed) on `84f0d36` [B-6b8]. No later commit (`[B-6b10]` not in). The body below
(after the `---`) is brief_final.md with `{HBR}`, `{APPROVAL_DATE}`, `{H_BYTES0}` and `{TESTS0}` put in place; `{BASE}` and
`{OFF}` are defined in "Filled placeholders" below and stay as names in the body (the lines are long). Find every edit by
its text anyway; the line numbers re-derived on {HBR} are listed under "Anchors".

## Start-state verdict (section 0)

- **0.1 PASS.** `git log --oneline -4`: `8b6c5a7` [B-6b8b] on `84f0d36` [B-6b8], `220129d`, `6ae9060`.
  `git diff --ignore-cr-at-eol --stat HEAD` empty (ROOT differs by line endings only; the one untracked file is
  `docs/OVERVIEW.html` - never touch). No UnrealEditor / UnrealEditor-Cmd / UnrealBuildTool / ShaderCompileWorker and no
  harness python was running at the start (free memory 10.1 GB). `git worktree list`: ROOT, `scratchpad/b7x/int`
  (`integrate/b7x`), `scratchpad/b7x/wt` (`design/b7x-composable-existence`) - and now `b6br/before_wt` (0.6).
- **0.2 PASS** - every anchor found by its text (table below; small line drift only).
- **0.3 PASS.** `callers_hbr.txt` 81 lines - every caller takes the value (a range-for or a copy into a local; two
  test lines call `.IsEmpty()` / `.Num()` on the temporary: `Tests/CosmicDataTests.cpp:1838`,
  `Tests/CosmicRaceMeasureTests.cpp:382` - value use, fine). `answer_callers_hbr.txt` 12 lines = declaration +
  definition + **10 call sites**. Thread grep: **0 hits**. `copies_hbr.txt` 355 lines; outside `Tests/` only
  `Sim/GalaxySimulationSubsystem.cpp:648` (`FCosmicDataTables Loaded;`) and the member `Sim/GalaxySimulationSubsystem.h:685`;
  `GetTables` returns `const FCosmicDataTables&` at `Sim/GalaxySimulationSubsystem.h:304`. Commandlet copies:
  `Tests/CosmicBalanceCommandlet.cpp:1391` (`FCosmicDataTables Tables = Sim.GetTables();`) and `:1598` (the rvalue
  `WithoutB6bData(Sim->GetTables(), &Removed)` into `SetTables`); `Sim->NewGalaxy(Seed, Systems, Civs)` at `:1652`;
  `FPaths::IsRelative(Out)` at `:1528`. New names (`FCosmicSortedRowNames`, `*NamesMemo`, `SortedRowNamesOf`,
  `SameKeySequence`): 0 hits. `FCosmicDataTables *{`: 0. `sizeof(FCosmicDataTables`: 0.
- **0.4** filled (below).
- **0.5 PASS.** Start build: `Target is up to date` (nothing built, exit 0; `root_start_build.log`, UTF-16, and
  `root_start_build.utf8.log`). Ten runs, each 10-13 s, exit 0; logs `before\<name>.log`, CSVs
  `<ROOT>\Saved\Balance\b6br_before\<name>{.csv,_rows.csv,_claims.csv}` (30 files). The five on-lines equal `{BASE}`
  letter for letter (`before_lines.py` composes them as the runner does, `Remove-WrotePath`'s regex). The five off-lines
  equal the `[B-6b5]` lines. 0 `end check` / `race check` / `LogCosmic: Error` in all ten. Summary: `before_lines.tsv`.
- **0.6 DONE** - before tree built (exit 0, 152 s, -DisableUnity) and sane; see "Before tree" below.
- **0.7 PASS.** `apply_before.txt` on ROOT: **1,078 patches; 931 apply; 147 fail** - identical to
  `<SP>\b6b8b\apply_after.txt` (diff empty).
- **0.8** Engine not updated (5.8.3, the brief's facts verified 2026-11-10 stand).

## Drift from the brief (read first)

1. **Harness slip in 0.5, corrected (no stop).** The first take of `off_checks` and `off_checks_20000` passed the flag as
   characters (`- W i t h o u t B 6 b D a t a = 1` in the log's command line: a one-element PowerShell `if` result
   unrolled to a string, and splatting a string passes its characters), so those two ran ON and printed the on-lines.
   `before_runs.ps1` now wraps the extra arguments in `@(...)`; both were retaken (`before_runs.txt` records the retake)
   and print the `[B-6b5]` lines. Lesson for I2's `run_timing_ab.ps1` (section 6) and for section 4 (d): build every
   extra-argument list as an array with `@(...)`, never as the output of an `if` with one element, and check the
   `Command Line:` of one off log for `-WithoutB6bData=1` (no spaces). Also: `powershell -File x.ps1 -List a,b` passes
   `'a,b'` as one string.
2. **The `[Adaptive Build]` line is printed only when UBT compiles.** ROOT's start build was up to date and its output has
   no `[Adaptive Build]` line at all (0), and no `Disabling optimizations` (0). So section 6's fallback "run section 0.5's
   `Build.bat` on ROOT once right before section 6 (nothing builds) and save its output" will NOT give the after side's
   line. Take it from the final runner's 'build: editor target' output (that build compiles this commit's edits), or
   from the mutation stage's rebuild output (`run_mutation.ps1` / `run_mutations2.py`), as `[B-6b8b]` did
   (`<SP>\b6b8b\mut\rebuild_editor.txt` has one `[Adaptive Build] Excluded from Cosmic_Ascension unity file:` line and 0
   `Disabling optimizations`). If neither echoes it, UBT's own `C:\Users\rhwwl\AppData\Local\UnrealBuildTool\Log.txt` of
   that build (overwritten by the next UBT run - copy it right after).
3. **ROOT's adaptive set re-checked:** `git status` lists 80 of the module's 108 `.cpp` (Ascension 1/1, Core 1/1,
   module file 1/1, Data 1/1, Events 2/2, Log 2/2, Player 1/1, Save 2/2, Sim 13/13; Tests 39/58, UI 7/13, World 10/13).
   The before worktree is clean (`git status --porcelain` 0 lines), as the brief says.
4. **Line numbers:** `GalaxyLawRowsOf`'s `RemoveAll` is `Sim/CosmicEdicts.cpp:218` (brief :217). `run_checks.ps1`:
   `Compare-Balance` `:81-112`, `$baselinePath = ...` at **`:100`** (brief :101) - section 2.6 inserts just before
   `:100`; the five balance steps at `:115`, `:119`, `:126`, `:131`, `:132`. Everything else as the brief says.
5. **Commandlet run time:** about 10-13 s per run here, the 20,000-tick runs included (the brief's section 6 estimate of
   20-25 minutes for 44 processes stands roughly: about 44 x 12 s + the quiet polls).
6. **Start-build stage times (one sample each, loaded? no - free memory about 10 GB, no other harness; recorded only,
   not section 6):** AI / economy / crisis us per tick:
   checks 19.34 / 1.81 / 11.03; checks_ruler 16.79 / 1.63 / 10.08; checks_wanderer 20.10 / 2.93 / 19.72;
   checks_20000 23.29 / 2.90 / 27.56; checks_ruler_20000 20.30 / 2.75 / 26.37. AI / economy = 10.7 / 10.3 / 6.9 (the
   brief's start ratios 11.04 / 10.34 / 6.87 - consistent; section 9 step 5's stop bars 7.7 / 7.2 / 4.8 stand).
   Off: off_checks 10.91 / 1.58 / 8.81; off_checks_ruler 10.36 / 1.67 / 9.01; off_checks_wanderer 11.34 / 2.64 / 12.22;
   off_checks_20000 12.33 / 3.21 / 13.71; off_checks_ruler_20000 11.53 / 3.41 / 13.53.
   These are R5's stand-in "before" if the before tree cannot be built (the lead decides).

## Filled placeholders

- `{HBR}` = `8b6c5a7`.
- `{APPROVAL_DATE}` = **2026-11-10** (the lead approved section 1.3's header text under the developer's standing
  milestone-B approval, recommended option). The exact line for N section 5 and the commit body:
  "헤더 먼저 — 지침서 §1.3의 머리 글을 리드가 2026-11-10에 승인(개발자의 상시 승인, 권장안)한 뒤 소스".
- `{H_BYTES0}` = **148,811** (`wc -c` of H at {HBR}; equal to `git show 8b6c5a7:` of it).
- `{TESTS0}` = **225** (summing command at {HBR}); after this commit 227.
- `{BASE}` (`git show 8b6c5a7:Tools/baselines/seed42.json`, `Remove-WrotePath` applied; copy `seed42_hbr.json`):
  - `summary_line`: `CosmicBalance: seed 42, 3000 ticks, hash 2557850423, world 2906170550`
  - `ruler_summary_line`: `CosmicBalance: seed 42, 3000 ticks, hash 1189870756, world 3359656751 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 17, answers 17, started 5, focus changes 5, edict changes 5, diplomacy changes 17, decisions 49 (by 600 12, by 6000 -1, by end -1); told by 600 25, by 6000 -1; ruled people T2 at 1175, T3 at 2375`
  - `wanderer_summary_line`: `CosmicBalance: seed 42, 6000 ticks, hash 1631315934, world 489794244 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 8, answers 8, started 2, focus changes 4, edict changes 3, diplomacy changes 2, decisions 19 (by 600 12, by 6000 19, by end 19); told by 600 25, by 6000 733; ruled people T2 at 1175, T3 at -1 | CosmicBalance wanderer: abdicate 1200 wander 1; left at 1201, crowned at -1 (None, tier 0, path none), journeys 191, scans 191, cards 7, answers 7, reverence peak 0.00, body 3, mind 1`
  - `summary_line_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 1957158585, world 3764704052`
  - `ruler_summary_line_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 1375149537, world 510604211 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 110, answers 110, started 41, focus changes 5, edict changes 18, diplomacy changes 17, decisions 191 (by 600 12, by 6000 90, by end 89); told by 600 25, by 6000 927; ruled people T2 at 1175, T3 at 2375`
- `{OFF}` (the five `-WithoutB6bData=1` lines on the start build = the `[B-6b5]` lines, no difference):
  - `off_checks`: `CosmicBalance: seed 42, 3000 ticks, hash 2816687688, world 3300228750`
  - `off_checks_ruler`: `CosmicBalance: seed 42, 3000 ticks, hash 3561771757, world 2462331303 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 16, answers 16, started 4, focus changes 5, edict changes 4, diplomacy changes 15, decisions 44 (by 600 12, by 6000 -1, by end -1); told by 600 25, by 6000 -1; ruled people T2 at 1175, T3 at 2375`
  - `off_checks_wanderer`: `CosmicBalance: seed 42, 6000 ticks, hash 2665013209, world 2229239127 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 22, answers 22, started 17, focus changes 4, edict changes 6, diplomacy changes 8, decisions 57 (by 600 12, by 6000 57, by end 56); told by 600 25, by 6000 804; ruled people T2 at 1175, T3 at -1 | CosmicBalance wanderer: abdicate 1200 wander 1; left at 1201, crowned at 3121 (civ_7, tier 2, path vacuum), journeys 96, scans 96, cards 1, answers 1, reverence peak 0.00, body 1, mind 1`
  - `off_checks_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 2359424635, world 726895141`
  - `off_checks_ruler_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 1611658162, world 29194274 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 105, answers 105, started 36, focus changes 5, edict changes 11, diplomacy changes 15, decisions 172 (by 600 12, by 6000 69, by end 68); told by 600 25, by 6000 873; ruled people T2 at 1175, T3 at 2375`
- `before_lines.py <log dir> <seed42 json> [<tsv>]` (in `<OUT>`) checks a folder of the ten logs against these; section 4
  (d) and section 6 may reuse it (the on-line check and the off-line check are its two verdicts).

## Anchors re-derived on {HBR} (find by text; these are the current lines)

| Anchor | Line at {HBR} |
|---|---|
| `Data/CosmicDataTables.h` `CosmicLabels` namespace | `:14-17`; `USTRUCT()` `:19` |
| accessor comments | `:99-126` |
| `IsHazardAnswer` decl (with its comment) / closing `};` | `:128-131` / `:132` |
| `Data/CosmicDataTables.cpp` anonymous namespace / `FindRowQuiet` | `:9-32` / `:27-31`; `UObject/NameTypes.h` include `:7` |
| eight bodies | Event `:129`, Transcendent `:136`, Hazard `:143`, Feat `:150`, Encounter `:157`, Edict `:181`, Galaxy `:188`, Civ `:195-200` |
| `IsHazardAnswer` body | `:164-179` |
| `Tools/run_checks.ps1` `Remove-WrotePath` / `Compare-Balance` / `$baselinePath` | `:76` / `:81-112` / `:100` |
| five balance steps | `:115`, `:119`, `:126`, `:131`, `:132` |
| `docs/21_DATA_MODEL_REFERENCE.md` `UPROPERTY(Transient) FCosmicDataTables Tables;` | `:1536` |
| `docs/40_DEV_GUIDE.md` runner line / `Data.Integrity` row / one-assertion rule | `:54` / `:237` / `:622` |
| `docs/45_ENGINE_NOTES.md` `FNameLexicalLess` / `UDataTable::AddRow` | `:26` / `:72` |
| N threshold "AI 단계 µs/틱 — B6a 10.4~10.7의 +50%를 넘으면 `[B-6b9]` 전에 까닭을 찾는다" | inside `:694` (the line starts "- **적기만**") |
| N "  - AI 단계 µs/틱(러너의 직렬 판과 묶음의 켬 − 끔" | `:757` |
| N end (`[B-6b8b]` block) | last line `:921` (file 921 lines) |
| H §11 heading / last bullet `[B-6b8b]` / `## 추가 이야기와 보완` | `:461` / `:474` / `:476` |
| `DECISIONS_LOG.md` `[B-6b8b]` line / B6a heading | `:43` / `:45` |
| STATUS 갱신 / 코드 사실 / 기준값 / 다음 작업 2 | `:3` / `:26` / `:27` / `:32` |
| `Tools/mutations/README.md` last `b6b8b_` row / data-patch list | `:1021` / `:5` |
| `Sim/CosmicEdicts.cpp` `GalaxyLawRowsOf` `RemoveAll` | `:218` |
| old patches touching `Data/CosmicDataTables*` / `Tools/run_checks.ps1` | 0 / 0 (rerun set: none) |

## Before tree (section 0.6)

- Worktree `<OUT>\before_wt` at `8b6c5a7` (detached), clean; all 18 LFS files smudged (0 pointers);
  `Content\CA\Data\DT_Feats.uasset` 19,982 bytes, magic `C1 83 2A 9E`. No `lfs checkout` needed.
- Junction `C:\Users\rhwwl\AppData\Local\Temp\b6br_b` -> `<OUT>\before_wt`.
- Build: `Build.bat Cosmic_AscensionEditor Win64 Development -Project="C:\Users\rhwwl\AppData\Local\Temp\b6br_b\Cosmic_Ascension.uproject" -WaitMutex -NoHotReloadFromIDE -DisableUnity`,
  output `<OUT>\before_wt_build.log` (+ `.exit`). BUILD_RESULT_PLACEHOLDER

---

