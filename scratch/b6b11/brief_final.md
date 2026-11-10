# [B-6b11] implementation brief: re-measure under the final values and clock (bundle C)

Every line cite below was read from HEAD 10205b6 with `git show`. Earlier commits [B-6b6] through [B-6b10] will move the line numbers in H, N, DECISIONS_LOG, OPEN_QUESTIONS and STATUS, so each receiving spot is also named by its text.

Abbreviations:
- H = `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` (147,498 bytes at HEAD).
- N = its `_notes.md`.
- cmd = `Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp`.
- `<SP>` = the scratchpad root, `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad`.

**What this commit is.** It is docs only, at high effort (H:398 `| [B-6b11] | **다시 잼(묶음 C)** — 노브를 바꾸면 제 커밋 | high | 미리 적은 규칙으로 알린다 |`).
- **Outputs:**
  - bundle C: 620 commandlet runs on the build of the last code commit, or 560 if the death cell is dropped;
  - the readings that the pre-written rules announce;
  - the hand-check seeds and saves;
  - the records, including the #10 sentence in `OPEN_QUESTIONS.md:20`. N:185 row 10 and N:318 row 15 give that sentence to [B-6b11].
- **No code or data:** it changes no `Source/`, `Tools/data` or `ST_CA.csv` file. So there is no `-Data`, no header build, no baseline move and no mutation (H:402, H:358, N:768).
- **No knob change here.** A knob change is never made inside this commit. It gets its own commit (§7 R8). H:43 calls that commit `[B-6b11]`, and this brief labels it `[B-6b11b]`.

---

## 0. Start state: check the tree before any agent runs

### 0.1 Tree and build

1. `git --no-optional-locks log --oneline -14` shows [B-6b6], [B-6b7], [B-6b8], [B-6b8b], optionally [B-6b9], and [B-6b10], plus any [B-6br]. Record these values:
   - `<BUILD_SHA>`: the last commit that touched `Source/`, `Tools/data` or `Content/CA/Data`.
   - `<B9_RAN>`: whether `git --no-optional-locks log --grep '\[B-6b9\]'` finds anything.
   - `<BR_WORLD>`: whether any [B-6br] after [B-6b8] moved a baseline. Read its N §5 block.
   - `<K_MOVED>`: bundle K's moved R and wanderer seeds (`{K_MOVED_R}`, `{K_MOVED_W}`). They are in the N §5 `[B-6b8b]` block and `Saved\Balance\b6b8b_bundleK\`.
2. **Clean tree, judged by content only.** Never call plain `git status`: it rewrites `.git/index`, and on this PC it lists hundreds of LF/CRLF-only entries. Use exit codes:
   - `git --no-optional-locks diff --quiet --ignore-cr-at-eol HEAD -- Source Tools/data Config`
   - `git --no-optional-locks diff --quiet HEAD -- Content/CA/Data`
   - If either exits non-zero, cross-check each `Content/CA/Data/*.uasset` with `git hash-object` against `git ls-tree HEAD`.
   - Stop and wait only on a real content difference. That means another workflow is active.
   - The bundle sha1 guard covers the DLL (§1.2).
3. **No other workflow is running.** That means no UnrealEditor or UBT process, no mutation folder in use, and the lead confirms. `bundleA.busy_processes()` (`<SP>\b6b6\bundleA.py:108`) checks this once at start; bundleC also calls it before every job launch (§1.2).

### 0.2 Values to read at `<BUILD_SHA>` (all through `git show`)

| Placeholder | Source | HEAD value (sanity only) |
|---|---|---|
| `<C>` | `Tools/data/DT_Rules.json` `ProvisionalEndTick` (HEAD :57). It must equal the C written in the [B-6b10] line of DECISIONS_LOG. | 6000 |
| `<R_COMMITTED>` | DT_Rules `AIUpperRungProgressMultiplier` (HEAD :17), from [B-6b7] and possibly [B-6b9] | 1.0 (off) |
| `<SHARE_COMMITTED>` | DT_Rules `FleetDelayCountdownShare` (HEAD :27). It must be 0.5 after [B-6b8b]. | 0.0 |
| `<W_EASED>` | `Tools/data/DT_Events.json` `event_claim_eased` `BaseWeight` (HEAD :399; the row is at :380) | 1.2 |
| `<CHANCE>` | DT_Rules `EventChancePerSlot` (HEAD :40). `EventSlotTicks` (HEAD :39) must be 10. | 0.02 |
| `<RATIO>` | DT_Rules `AIHostilePowerRatio` (HEAD :97) | 0.79 |
| `<T_DEATH>` | The `-DeathAt` in N §3 '묶음 B' (written by [B-6b8]). Every `death_*.log` in `Saved\Balance\b6b_bundleB\` must say `death asked at <T_DEATH>`. | — |
| `<A_SEC_PER_20K>` | `<B_MINUTES>` from N §3 '묶음 B' × 60 × 6 workers / 540 | — |
| `<BASE_B6b10>` | The five lines and two hash runs of `Tools/baselines/seed42.json` at `<BUILD_SHA>` | — |
| `<H_BYTES>` | The size of H at HEAD at the start, and whether its '추가 이야기와 보완' block is still there or already a pointer | 147,498 |

**Stop and ask the lead if any of these hold:**
- `<SHARE_COMMITTED>` ≠ 0.5;
- C ∉ [6000, 18000] (rule ⓓ at H:349);
- the bundle B death logs disagree on `<T_DEATH>`.

**If `<T_DEATH>` ≥ `<C>`** (recommended, counts as approved): drop the death cell.
- A death after the clock record cannot be a death before the end.
- Write `재지 않음 — 강제 죽음(<T_DEATH>)이 시계(<C>) 뒤` for the death cell's '< 15%' reading and its #10 reference reading.
- The job count drops to 560.

### 0.3 Reference outputs (read only)

- **Decision 36 bridge.** `Saved\Balance\b6a_bundle1\wanderer_<s>.csv` and `_rows.csv`, seeds 1-60, are all present in the working tree. Their header must equal the CSV header at cmd:1716; check it again on the build.
- **Bundle B.**
  - The CSVs are in `<SP>\b6b8\bundleB\`, if that folder still exists.
  - The logs and a sha1 manifest are in `Saved\Balance\b6b_bundleB\`. Check the copied logs against that manifest.
  - Set `<BUNDLE_B_CSV>` to true only if every `R_/slowR_/wanderer_/inputless_/death_*.csv` exists there and matches the manifest's sha1.
  - B-d's on-side predictor reads the bundle B logs' `CosmicBalance conflict:` "delay bought per tag" lists (§1.2).

### 0.4 Which output fills which placeholder

| Placeholder | Filled by | Lands in |
|---|---|---|
| `<C_MINUTES>` | Wall clock of the run (estimate: `<A_SEC_PER_20K>` × (300·C + 320·20,000) / 20,000 / 6 / 60; with the death cell dropped, 240·C) | Header of N §3 '묶음 C' |
| `<FROZEN_SHA1>` | §4.3 freeze step (§7.1 step 6) | Header of N §3 '묶음 C' |
| `<J136>` | §4.3 R1 | N §3, N §5, DECISIONS_LOG line, STATUS Top 5 item 1 |
| `<N142>` (n, excluded, k_e, interval, verdict) | §4.3 R2 | N §3, DECISIONS_LOG (decision 24 rule plus number) |
| `<A132>` | §4.3 R3 | N §3 |
| `<B1>` (on/off own-vessel absolute, L/G/p, median after leaving, paths, gate) | §4.3 R4 | N §3, DECISIONS_LOG (closes DL:68) |
| `<129>` (0.79 on/off, 0.75 on) | §4.3 R5 | N §3, STATUS Top 5 item 4 |
| `<154>` | §4.3 R6 | N §3 |
| `<LT15 ×3>` | §4.3 R7 | N §3 |
| `<KINDS R/slowR>` | §4.3 R8 | N §3, STATUS Top 5 item 3 |
| `<#10>` | §4.3 R9 | N §3; the `OPEN_QUESTIONS.md:20` sentence in this commit |
| `<#158③>` | §4.3 R10 | N §3, STATUS Top 5 item 2; OQ:122 at [B-6b12] |
| `<RUNLEN>` | §4.3 R11 | N §3, DECISIONS_LOG, STATUS Top 5 item 3; `docs/00_VISION.md:32` at [B-6b12] |
| `<HC①②③④⑥>` (seeds, ticks, commands) | §4.4 | N §5 block |

---

## 1. The changes

### 1.1 Data and code

None. If a reading appears to call for a value change, see §7 R8: it goes to [개발자] or to a separate commit.

### 1.2 Harness (outside the repo): `<SP>\b6b11\bundleC.py` and `bundleC_selftest.py`

**Loading.** Load `<SP>\b6b8\bundleB.py` the way it loads bundleA: `BUNDLE_B_PY` env var and `importlib`. Use stdlib only, and run with `python -I -X utf8`.

**Jobs.** bundleC has its **own `build_jobs`**. `bundleB.build_jobs` reads bundleB's module-level `COMMON_ARGS` (bundleB.py:71, :118, which carry `-Rules=ProvisionalEndTick:0`) and `HORIZON` (:123), so it must not be reused.
- **No common arguments.** C runs in game form.
  - `FParse::Value(…"Rules="…)` (cmd:1482) reads only the first `-Rules=`. So a job carries at most one `-Rules=`, and no job argument may contain `ProvisionalEndTick`.
  - A control checks both (F8).
- **Cell arguments** (`R` = bundleA.py:48):
  - `R` = `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`
  - `slowR` = R with `-Focus=0`
  - `death` = R + `-DeathAt=<T_DEATH>`
  - `wanderer` = R + `-Abdicate=1200 -Wander=1`
  - `inputless` = no arguments
  - `inputless075` = `-Rules=AIHostilePowerRatio:0.75` (#129's 0.75 side: H:215 and N:674; see §4.3 R5)
  - `test` = R + `-Rules=FallenCoreMinLanesFromHome:1 -LeaveNearSphere=1 -PlayerPolicy=Faithful -Guard=1`
    - The test bundle definition is at DECISIONS_LOG:104; H:332 cites the old :68.
    - The argument order matches `Saved/Balance/b6a_bundle1/test_Faithful_g1_1.log`.
- **Ticks:** `<C>` for R, slowR, death and wanderer; 20,000 for inputless, inputless075 and test.
- **Seeds:** 1-60; 1-140 for test.
- **Off sides:** `HAS_OFF` is wanderer and inputless only (`OFF_ARG = -WithoutB6bData=1`). Each seed's on and off run go next to each other.
- **Total:** 620 jobs: 300 at C and 320 at 20,000 (560 without the death cell).
- **Options:**
  - `--c N` is required. It must equal `<C>` read through `git show <BUILD_SHA>:Tools/data/DT_Rules.json`.
  - `--t-death`, `--r`, `--share` (default 0.5), `--workers 6`.
  - `--folder <SP>\b6b11\bundleC`, `--archive-dir <root>\Saved\Balance\b6b_bundleC`.
  - `--check-only`, `--dry-run`, `--cell-set B|C`.
  - `--bundle-b-logs`, `--bundle-k-dir`.
- **Keep from bundleB/bundleA:**
  - the sha1 guard, before and after the run, over:
    - every `UnrealEditor-Cosmic_Ascension*.dll`;
    - every `Content/CA/Data/*.uasset` (this includes `ST_CA.uasset`);
    - `Tools/data/generated.sha1`;
    - added here: `Config/DefaultGameplayTags.ini`, `Config/DefaultGame.ini` and `Config/DefaultEngine.ini`;
  - the busy-process guard, called **before every job launch**, so the bundle aborts early instead of finishing a mixed run;
  - the memory guard;
  - absolute `-Out` and `-AbsLog`;
  - one rerun of a failed process. Two failures in one cell withhold that cell's readings (bundleB `judgments` 'withheld').

**Run failures** (`bundleB.run_failures`; H:325 `모든 로그에 end check·race check 줄 0`, with the race check defined at H:316):
- a non-zero exit;
- a missing file;
- any `end check` or `race check` line;
- `LogCosmic: Error`;
- an unreadable `race:` part or ending head.

**Field controls.** A control in the "problem" class invalidates the bundle. A "note" does not.

| # | Control | Class |
|---|---|---|
| F1 | Every run's ending head reads `record <kind> at <t>` with `t ≤ C`. Kind `Clock` appears only at `t == C`. Kind `None` or `t = −1` is a problem, because with C > 0 every run records by C. Replaces bundleB.py:340-341. | problem |
| F2 | The CSV's last tick equals the job's ticks (bundleB `csv_last_tick`). | problem |
| F3 | Off logs:<br>• `rows taken out` equals `EXPECTED_REMOVED` (the ten), once.<br>• The tail reads `AIUpperRungProgressMultiplier 1 (was <R_COMMITTED>), FleetDelayCountdownShare 0 (was <SHARE_COMMITTED>)` with share `0.5`. This replaces bundleB.py:353-354, which require `0`.<br>• Off runs have era 0, witness 0, no extinctions, no AI-step treaties and no AI T4. | problem |
| F4 | On logs:<br>• No `rows taken out` line.<br>• The side part is in the [B-6b8] layout: `era lines %d (by the record %d, first at %lld), witness cards %d (first at %lld, by the record %d; answered %d: …)`.<br>• E ≥ 600 iff era lines > 0; W ≥ 0 iff witness cards > 0.<br>• W is not before the wanderer left and not before the first claim placed (bundleB.py:368-386 as is). | problem |
| F5 | Death logs: `death asked at <T_DEATH>`, with `T_DEATH < C`. A death recorded 0 is a note. | problem / note |
| F6 | Test logs: the pressure line head reads `guard 1 leave near sphere 1` with policy `Faithful` (cmd:3819). | problem |
| F7 | For each test run, compare two counts:<br>• X = `_claims.csv` rows with Ruled=1, CardTick≥0, VigilKept=1, Relief=0, OwnerChanged=0, EndKind∈{joined,eased,other,expired} and 401≤RemainingAtCard≤550 (no tick filter);<br>• P = the pressure line's `401-550 %d` count.<br>The two filters differ by construction:<br>• the band excludes answered claims (cmd:3316 `!bAnswered`), while EndKind `joined` takes precedence over an answer (`Tests/CosmicRaceMeasure.h:101`);<br>• the band needs the system ruled at the end (cmd:3303), while the CSV's `Ruled` is the holder at placement (cmd:2751).<br>So F7 requires X − J ≤ P ≤ X + O, where:<br>• J = X's rows with EndKind=joined;<br>• O = rows passing the same filter except Ruled and OwnerChanged, with OwnerChanged=1.<br>The eased counts get the same bound with J = 0. Rows with OwnerChanged=1 are reported as a note. | stops the #142 reading only |
| F8 | Argument and rules lines:<br>• no job argument contains `ProvisionalEndTick`; each job has at most one `-Rules=`;<br>• each log has `CosmicBalance: rules <key> = <value>` (cmd:1383) for exactly its own keys (test: `FallenCoreMinLanesFromHome = 1`; inputless075: `AIHostilePowerRatio = 0.75`; every other cell: none);<br>• no `-Rules cannot set` (cmd:1380). | problem |

**Bridges, all breakable.** If any breaks, find the cause before reading anything (map D-23).

**Moved-awakening predictor (used by B-a and B-d).** A run "may move" through the [B-6b8b] share if its share-0 reference log's `CosmicBalance conflict:` "delay bought per tag" lists `Hazard.AIAwakening=<sum>(<count>)` with sum > 75. This is bundle K's rule (b6b8b brief §3 (d)(i)). That list counts every owner, so it also covers:
- the ruler policy's step (1) guard of a besieged own system under `-Diplomacy=1 -Guard=0` (`World/CosmicRulerPolicy.cpp:719` returns early only when `!bGuard && (Diplomacy == 0 || ended)`; step (1) at `:733-764`);
- the wanderer's guards outside its first reign;
- the one-tick tail after a reign ends.

| # | Bridge | Source |
|---|---|---|
| B-a | **On/off pairs.** Wanderer pairs use T_B = min(T+1, E, W); inputless pairs use T_B = min(T+1, E) (bundleB.py:236-288).<br>In C the two sides also differ in the share (0.5 on, 0 off: cmd:1596, H:317). A pair that differs below its T_B is therefore explained only if the off log satisfies the predictor. Then record G = the first differing sample and use T_B = min(…, G) for the same-world accounting.<br>An unexplained difference breaks the bridge. | N ⓗ (N:632); bundle K K1/K2 |
| B-b | `wanderer_s` against `R_s` for ticks ≤ 1,200; `death_s` against `R_s` for ticks ≤ `<T_DEATH>` (bundleB.py:816-829; both actions take effect in the tick after the boundary: cmd:2521, `World/CosmicWandererPolicy.cpp:20`). | N ⓗ |
| B-c | **Decision 36.** `wanderer_s_off` CSV and `_rows.csv` against `Saved\Balance\b6a_bundle1\wanderer_s.csv` and `_rows.csv`, every row with Tick < 6,000. This uses `rows_equal_below` and ignores TickMicros, SaveBytes and SaveRawBytes, as bundleB does. | N:674, H:453 |
| B-e | **Follows from B-c.** The off side's own-vessel B1 reading (§4.3 R4) reproduces B6a bundle 1's on side: 40/60, median after leaving 1,350, paths 복귀 22·빈자리 15·경외 3 (B6a N:298). A mismatch with B-c intact means a parser error: fix it before reading B1. | B6a N:298, N:674 |
| B-d | **Added: tests H:325's premise that a clockless run gives any C's end.** It compares C against bundle B, per seed and cell. Rule: CSV rows below C are equal; for inputless, the whole run. If `<BUNDLE_B_CSV>` is false, compare instead the race-line first ticks as min(f, C), with −1 = ∞: first AI T4/T5; ruled T3/T4/T5; `fifth rung first at`; `silence first at`. When a seed matches, it also checks bundle B's E_i for that seed when E_i ≤ C: C's record must be that fifth or silence end at that tick, and otherwise `Clock` at C.<br><br>**Off sides (wanderer, inputless).** Breakable when `<BR_WORLD>` is empty. The off side removes every B6b row and forces both B6b knobs off, so [B-6b8b] and [B-6b9] cannot reach it. A world-moving [B-6br] (H:404) can reach it, so when `<BR_WORLD>` exists, skip and record why.<br><br>**On sides (R, slowR, death, wanderer, inputless).** Per seed, valid only when `<B9_RAN>` is false and `<BR_WORLD>` is empty:<br>• A seed whose bundle B on log satisfies the predictor is "may move". It is compared and recorded (moved yes/no, first differing sample), not broken.<br>• Every other seed must match.<br>• Cross-check: every R or wanderer seed in `<K_MOVED>` must be a "may move" seed. A mismatch is a note to explain.<br>• The count of moved seeds is N §3's measured figure for H:334's "거의". | H:325, H:334, N ⓗ '묶음 B와 C의 R 60은 C까지 같은 세계' |

### 1.3 Docs (§6 gives the exact form)

**Changed here:**
- N §3 '묶음 C';
- the N §5 `[B-6b11]` block;
- the H §11 line, plus the size check;
- the DECISIONS_LOG line and the #142 rule;
- the #10 sentence in `OPEN_QUESTIONS.md:20` (N:185 row 10 and N:318 assign it to `[B-6b11]`; this is a re-measured sentence, not a closing answer);
- STATUS;
- the CHANGELOG.

**Nothing to change:** docs/20, docs/21, docs/30 and docs/50, because no rule, value or table moves.

**Belongs to [B-6b12]:** the other OPEN_QUESTIONS rows that a closing answer reaches (DL:7: #136 `:103`, #142 `:108`, #158 `:122`, #129 `:98`), `docs/00_VISION.md:32`/`:103`, `docs/40` and `docs/15` (N:414-421).

---

## 2. Tests and tests whose meaning changes

- **Automation tests:** none new and none changed, so the count stays at whatever [B-6b10] left. The runner runs `-Tier full` once. Its two in-process passes are the two consecutive runs (N:768).
- **Meaning changes:** none. The tests pinned to the clock were settled by [B-6b10]: `Sim.EndingOf` (`Tests/CosmicEndingTests.cpp:622` at HEAD) and `UI.RunEnd`.
- **Harness self-test** (`bundleC_selftest.py`, in the bundleB_selftest form). It uses synthetic logs and CSVs that must do the following.
  - **Break F1:** a Clock at C−1; a record at C+1; a clockless log with no record (`None`, −1).
  - **Break F3:** share 0.
  - **Break F5.**
  - **Break F7:** a difference outside X − J ≤ P ≤ X + O. A difference inside the bound must pass.
  - **Break F8:** a job with `ProvisionalEndTick`; a job with two `-Rules=`; a missing `rules … = …` line.
  - **Break B-c:** one changed cell at tick 5,990.
  - **B-d:** a different first T4 below C on an unpredicted seed breaks it. The same difference on a predicted seed is recorded and does not break. A difference at or after C leaves B-d alone.
  - **B-a:** a wanderer pair differing below T_B breaks unless the off log has `Hazard.AIAwakening` sum > 75.
  - **B-e / R4:** a crowning after the first death is not counted as own.
  - **R10:** a key at exactly `tick == T_DEATH` or `tick == 1,200` is counted once, from R.
  - **Frozen calculations of §4.3, with expected numbers:**
    - the #142 acceptance at n = 30 and n = 31, with k on each side of the bound;
    - the sign test: L = 9, G = 2 gives p = 0.0654 (noise); L = 10, G = 1 gives p = 0.0117 (`B6b가 줄였다`);
    - the #10 per-death rule;
    - the #158③ dedup across R, death and wanderer.
- **Parser check on real text.** Before the first C run, run `--check-only --cell-set B` over the `Saved\Balance\b6b_bundleB\` logs to exercise the parsers on real text.

---

## 3. Determinism and baselines

- **Bit-identical.** There are no code or data changes. Runner full: the five lines and the two hash runs must equal `<BASE_B6b10>`. Run it on the tree as is, with no `-Data` and no `-UpdateBaseline` (H:358 `[B-6b11]·[B-6b12]은 옮기지 않는다`).
- **No new draw point.** The clock record is inert (`Sim.EndingInert`). After the record, the ruler policy stops focus, edicts and diplomacy (`World/CosmicRulerPolicy.cpp:392`, `:493`, `:518`, `:719`). It still answers cards and builds projects such as arks and the binding (the build loop near `:452` has no end gate; H:479). Only `-Guard=1` keeps guarding after the end (`:719`), so the test cell's guards continue. Test-cell claims placed before C and ending after C still run under the policy's answers and guards; the reading counts them (§4.3 R2).
- **Same-world accounting** (N ⓗ):
  - R and wanderer share a world through tick 1,200, and R and death through `<T_DEATH>`. The action is recorded at that boundary and takes effect in the next tick (cmd:2521; `World/CosmicWandererPolicy.cpp:20`; B6a "1,201에 떠나").
  - On/off pairs share a world to T_B (§1.2 B-a, including the share split G).
  - Bundle B's and C's R 60 are the same world to C except for "may move" seeds, so they are never added together.
  - Each '< 15%' cell has its own denominator of 60. The inputless075 cell is its own world.
- **Run order:**
  1. runner full;
  2. the freeze step (§7.1 step 6);
  3. a sha1 snapshot;
  4. bundle C;
  5. the sha1 recheck;
  6. the hand-check saves (§4.4).

  Never run any of these next to a build, `make_data_tables` or a mutation, including [B-6b12]'s mutation rerun.

---

## 4. Measurement and the pre-written judgments

### 4.1 Runs

These are the 620 (or 560) jobs of §1.2, in the folder `<SP>\b6b11\bundleC\` (outside OneDrive, about 0.65 GB). After the readings, copy every `.log` plus a sha1 manifest of every output to `<root>\Saved\Balance\b6b_bundleC\`; the precedent is the b6b8 brief :467. Record both paths in N §3.

### 4.2 Text carried over verbatim (H §5.3 and N §2.3: written before data was turned on, so do not edit it)

H:354 `**적기만과 C에서 읽는 것**(… 묶음 C의 #136·#142·#132·B1 지킴·#129·#154·'< 15%'·시계 아래의 끝 — 알리되 C의 문이 아니다): 글은 노트 §2.3이다`. N §2.3 (N:669-677), copied as is:

> **C에서 읽는 것 — 알리되 C의 문이 아니다**(묶음 C):
> - **#136**: R 60 — 기록 전에 1인칭 주장 줄이 말해진 판 ≥ 1/3(20/60)이면 닫고, 모자라면 수와 함께 열어 둔다(시계는 이 수로 다시 정하지 않는다). 결박이 그 뒤의 주장을 끝내 곡선을 자른다(B6a 18,000 참고 칸 — 6,000에 9/30, 6,331에 10/30, §0.1).
> - **#142**(결정 24): 시험 칸 — B4의 모형 구간 규칙(B4 헤더 `:362` — 띠마다 30건 이상, 걷힌 몫이 1 − (1 − 0.02 × W)^(슬롯 수)의 95% 구간 안); 바닥 0.25는 `Data.Integrity`의 불변식(`Tests/CosmicDataTests.cpp:2234`)이 지킨다. 셈하는 주장: 카드가 온 틱의 남은 시간이 401~550이고 놓인 틱이 C 전인 주장; 그 끝이 첫 결박의 틱(`SealedTick`) 뒤인 주장은 편입될 수 없어(§0.4′) 분모에서 빼고 따로 적는다. 기대 N: B6a의 13건 가운데 9건은 6,000 뒤였고 결박(모형 약 6,100~7,700)이 그 뒤의 주장을 끝내므로 140시드에 약 9~12건 — '재지 않음 예상 — 잰 N'; 30건에는 약 450시드(모형).
> - **#132**(결정 23): 몫 0.5에서 모든 태그의 지연 몫은 구성상 50% 이하라(각성 75/150·역병 100/200·초신성 전조와 주장 100/300) 띠의 위 끝은 판단이 아니다 — 그 끝은 `Sim.FleetDelay`(각성 150 → 225)가 지키고 #132는 `[B-6b8b]`가 장치로 닫는다. 시험 칸에서 적기만(30건 이상): 각성 지연 몫(기대 약 48~50% — §2 ⓔ; 10% 아래면 지킴이 함대를 잃은 것이라 까닭을 찾는다), 지연이 상한 75에 닿은 지킴의 몫(거의 모두면 상수가 지연을 정한다 — B7 #155 곁에), 지킨 각성과 지키지 않은 각성의 끝.
> - **B1 지킴**(결정 36): 방랑자 짝을 6,000에서 자른다. 끈 쪽은 다리에 의해 B6a 묶음 1의 켠 쪽 세계라 40/60·1,350을 다시 낸다 — 절대 문 ≥ 44/60은 두 쪽 모두 적기만 한다(B6a의 모자람을 B6b에 돌리지 않는다). **B6b의 판단은 짝**: 켠 쪽에서만 떠난 그릇이 즉위·복귀하지 못한 시드 L과 켠 쪽에서만 한 시드 G — L > G이고 부호 검정(양쪽) p < 0.05면 'B6b가 줄였다'로 L의 시드마다 미리 이름한 길(#122의 `FarAbove`, 절멸이 지운 옛 백성, `enc_witness`의 답, #156의 국경 경외(결정 22의 답 ㉮))을 `race:` 줄로 세고, 길로 설명되지 않는 시드가 남으면 그 수와 함께 [개발자]; p ≥ 0.05면 잡음(B3의 규칙). 즉위까지 중앙값 ≤ 1,800(B3 헤더 `:316`)은 켠 쪽의 문 그대로. 길은 B6a 켠 쪽(복귀 22·빈자리 15·경외 3 — §0.1)과 견준다. **다리**(깨질 수 있는 확인): 끈 쪽의 CSV 결정적 칸이 B6a 묶음 1의 같은 시드 방랑자 판(`Saved/Balance/b6a_bundle1/`)과 6,000 아래 모든 행에서 같다.
> - **#129**: 입력 없는 짝 — 넘김의 3/4 경계(0.75/0.79), 판마다 넘김, 백성마다 얻고 잃은 최대, 6,000 뒤 넘김; 적기만(값은 B7 #155); 가장 많이 얻은 백성의 몫 ≤ 50%를 지켜본다.
> - **#154**: 방랑자·입력 없는 판의 늦은 계승 다이제스트 5~8줄 몫(B5 묶음 3의 꼴) — 적기만.
> - **'도약이 일으킨 다른 질서 < 15%'**(`docs/40:480`): 입력 없는·방랑자·강제 죽음 칸에서 기록이 다른 질서 ∧ `bNoVessel`인 몫 — 칸마다 따로(분모 = 그 칸의 60판 — §2 ⓗ), 점과 구간, 적기만(Phase 5의 목표; 스크립트 R은 스스로 죽지 않아 R만 재면 거저 지킨다).
> - **시계 아래의 끝**: R 60·느린 R 60 × C의 기록 종류(집단 승천·다른 질서·시계·침묵)의 몫과 Wilson 구간 — 적기만.

Two cites in that frozen text have drifted at HEAD. Note them in the N §5 block and do not edit §2.3:
- the '< 15%' target is now `docs/40_DEV_GUIDE.md:514`;
- the 0.25 floor is `Tests/CosmicDataTests.cpp:2562`.

The shared rules from H:325 also apply: `30건 이상일 때만 판단, 모자라면 '재지 않음 — 잰 N'; 몫에는 95% 구간(Wilson); 판 단위의 수는 짝 차이; 원인은 탐침 뒤에만; … 같은 세계는 한 번만; 모든 로그에 end check·race check 줄 0`.

### 4.3 Frozen arithmetic for each reading

Write this section into the N §5 block **before the first run** (map item 41; recommended, counts as approved). Save the exact block text to `<SP>\b6b11\frozen_4_3.md` and hash it in the freeze step (§7.1 step 6).

**R1. #136** (R 60)
- k = runs whose ending line's `by the record told: first-person claim lines %d` (cmd:3890) is ≥ 1; n = 60.
- k ≥ 20: `닫는다 — k/60 [Wilson]`.
- k < 20: `열어 둔다 — k/60 [Wilson]`. The clock is not re-decided.
- Also record slowR's k as reference.

**R2. #142** (test 140, from `_claims.csv`; header at cmd:698)
- **Counted claims:**
  - Ruled=1, CardTick≥0, 401≤RemainingAtCard≤550, VigilKept=1, Relief=0, OwnerChanged=0;
  - EndKind ∈ {joined, eased, other, expired};
  - PlacedTick < C, AfterBinding ≠ 1.

  This is the CSV form of the pressure line's band (cmd:3303-3321) plus the C and binding cut. F7 bounds the difference between the two.
- **Listed separately:** the same rows with AfterBinding = 1 (count and seeds), and the rows with OwnerChanged = 1 (count).
- **n < 30:** `재지 않음 — 잰 N` (expected about 9-12). Give k_e/n with Wilson as information.
- **n ≥ 30:**
  - The statistic is the eased share k_e/n (DL:104 and decision 24: `걷힌 몫`).
  - The model is the band midpoint from N ⓕ: p_e = 1 − (1 − `<CHANCE>`·`<W_EASED>`)^47.5. That is 0.6846 at 0.02 and 1.2.
  - The acceptance region is p_e ± 1.96·√(p_e(1−p_e)/n). At n = 30, seen from the joined side, this is N ⓕ's `약 0.15~0.48`.
  - Inside: `모형 구간 안 — k_e/n`.
  - Outside: **[개발자]** with the numbers.
- **Record only, not the judgment:**
  - the joined share with Wilson;
  - the per-claim model Σp_i, where p_i = 1 − (1−0.02W)^⌊R_i/10⌋;
  - B4's practised form: whether the model range across the band, from p(401) to p(550), lies inside the observed Wilson interval;
  - claims whose card came after C.

**R3. #132** (test 140; record only; 30 or more cases)
- **Primary reading:** over the whole run, in B4's form (DL:104, 70 cases), the awakening `delay bought` divided by the summed countdowns. Read:
  - the ending line's `#132 guarded [..]/[..]/[..]`, summing the three phases' `Hazard.AIAwakening=<count>:<share>%` weighted by count;
  - the pressure line's `delay bought … by tag`.
- Also give the by-the-record phases.
- **Cap share:** guards at the row's cap divided by guarded awakenings, from the race part `#132 by the record: guarded at the row's cap by tag [..]`.
- **Ends:** guarded against unguarded, from the race part's `guarded ends by tag` and `ruled unguarded ends by tag`.
- **Below 10%:** find the cause (no [개발자] call) and write it down.

**R4. B1 guard** (wanderer pairs, each run cut at 6,000; **the bridges B-c and B-e must pass first**)
- **Own-vessel reading.** This is the reading B5 and B6a judged: "떠난 그릇 자신의 즉위·복귀" (B5 N:228; B6a N:298).
  - own(s) = the wanderer line's `crowned at t` (cmd:3738) with 0 ≤ t ≤ 6,000 and either `first death at` = −1 or t < `first death at` (`calling deaths:`, cmd:3774).
  - `crowned at` crosses lives (B5 N:229). Report the any-vessel count only as a second line. The bundleB.py:751-773 preview uses the any-vessel reading and is not reused.
- **Absolute counts.** k/60 with Wilson on both sides, record only (the ≥ 44/60 gate is 적기만 on both sides).
- **Pair counts.** L = on not own ∧ off own; G = the reverse.
- **Sign test.** p = `bundleB.sign_test(L, G)` (exact, two-sided).
- **L > G ∧ p < 0.05 → `B6b가 줄였다`.** For each L seed, count the four named paths from the on-side `race:` line. A seed may carry several paths.
  - (a) `crown refused far above %d (first at …)` ≥ 1, with first at ≤ 6,000.
  - (b) `peoples erased … (by an extinction … once ruled %d)` ≥ 1. This counts to the run's end (≤ C), not ≤ 6,000, because there is no first-at.
  - (c) witness `answered %d` ≥ 1 with witness `first at` ≤ 6,000. `first at` is the first card, not the first answer, and `answered` counts to ≤ C.
  - (d) era lines ≥ 1 with era `first at` ≤ 6,000.

  Seeds explained only by (b) or (c) are listed apart, because their explanation is an upper bound.

  Beside the paths, not as a fifth named path, record for each L seed whether its off log satisfies the share predictor of §1.2. The [B-6b8b] knob is B6b too.

  Unexplained seeds left over → **[개발자]** with their count.
- **p ≥ 0.05 → `잡음`.**
- **On-side gate** (N:674, B3 header `:316`): the median of (own crowned − `left at`) over own-crowned on seeds must be ≤ 1,800. The bundleB preview reports the absolute tick instead.
  - Pass: write it.
  - Fail: write `깨짐 — 중앙값 <m>` with the own-vessel path counts set against B6a's 22/15/3, in the B3 `:316` form. It goes to **[개발자]** with the numbers and does not block the commit; any knob change is [B-6b11b].
- **Paths on the on side** are compared with B6a's 복귀 22·빈자리 15·경외 3.

**R5. #129** (record only)
- **Cells:** inputless pairs at the committed `<RATIO>`, and inputless075 (on only, `-Rules=AIHostilePowerRatio:0.75`). That is the 0.75/0.79 pair of N:674 and H:215, which OQ:98 re-measures whenever `EndingOf` changes.
- **Fields:** read `taken %d (%d/%d/%d by tick bands 0-3000/3000-6000/later …)`, `taken by top conqueror %d` and `lost by one people max %d` (cmd:3715).
- **Derived numbers:**
  - transfers per run;
  - transfers after 6,000 (the late band);
  - the top gainer's share = taken by top conqueror / taken. Watch whether it stays ≤ 50%.
- **Report:** 0.79 on, 0.79 off, (on − off), and 0.75 on next to 0.79 on, set against B3's 143→1 and 50 after 6,000 (OQ:98).

**R6. #154** (wanderer and inputless on sides; record only)
- Read the `calling deaths:` successions list `tick:…:D<n>` (cmd:2460, :3774).
- 'Late' = succession tick ≥ 4,000 and ≤ the record tick (B5's cut: B5 N:251 `4,000 전/뒤`).
- Report the share of D in 5..8 among late successions, with Wilson, per cell and pooled. Give the early ones for contrast.
- Pre-written expectation: rises from the fourth rung up are hidden from a first-rung viewer, so late rose items come only up to the third rung (`World/CosmicDigest.cpp:397`).

**R7. '< 15%'** (inputless, wanderer and death on sides; each n = 60)
- k = the ending head `record OtherPeople … no vessel 1` (bundleB `RE_ENDING_HEAD`).
- Point and Wilson per cell; record only.

**R8. Ends under the clock** (R and slowR)
- Record-kind shares over `RuledPeople`, `OtherPeople`, `Clock`, `NoCandidate` and `Vessel`, with Wilson. These map to 집단 승천·다른 질서·시계·침묵, with Vessel listed as it appears. F1 rules out `None`.

**R9. #10** (the pooled cells are frozen here because H does not name them)
- **Pool:** wanderer + inputless, B5's analogue cells (B5 N:236-245). The death cell is given separately as reference, since the switch fixes its one death tick.
- **Unit: per death,** as OQ:20 and B5 N:244 count it (`끝 전 죽음의 약 넷에 하나(15/59)`).
  - n = deaths before the record: the `deaths %d (by end %d)` by-end count.
  - k = pre-record deaths with no succession at or before the record tick. So k/n = 1 − ③.
- Also give B5's ③ (pre-record deaths that succeeded before the record), and the per-run column `시계에 공백기 안` (runs ending inside an interregnum) as B5's table did.
- **30-case rule on the pool:** n < 30 → `재지 않음 — 잰 N`.
- **Where it goes:** the number goes to N §3. This commit rewrites the B5 sentence in `OPEN_QUESTIONS.md:20` with C's numbers and keeps `도약의 길이는 Phase 5 그대로` (N:185, N:318).

**R10. #158 ③** (R + death + wanderer + slowR + test)
- **Keys:** from the ending line's `#135 claim trigger … keys [..]`, in the form `tick:system:phase:k|w|s` (cmd:2602). Use phase ∈ {0, 1}, which is before the record.
- **Count once over shared worlds:**
  - all R keys;
  - death keys only with tick > `<T_DEATH>`;
  - wanderer keys only with tick > 1,200 (the boundary tick's key is shared with R: §3);
  - slowR and test as their own worlds.
- **Counts:** n = distinct (cell family, seed, tick, system). Lost = `w` or `s`.
- **Outcomes:**
  - n < 30 → `재지 않음 — 잰 N`.
  - n ≥ 30 ∧ lost ≥ 10% → **[개발자], header first.** The OQ:122 one-place buffer would be two stored fields of the knowing layer, and that is never a value change in [B-6b11].
  - n ≥ 30 ∧ < 10% → write it closed with the number, so OQ:122 changes at [B-6b12].

**R11. #2 and decision 42** (`<RUNLEN>`)
- The record-tick distribution (min, median, p90) for R and slowR, plus the share ending at the clock.
- '×10 a~b분' stays a placeholder until hand check ⑤. [B-6b12] writes it (DL:31).

### 4.4 Hand-check seeds and saves (H:411-412)

**How to choose.** Pick the smallest qualifying seed. The command must be in game form, because `bSaveShape` (cmd:1513-1514) refuses `-Rules`, `-DeathAt`, `-TellAll≠0` and `-WithoutB6bData`. So seeds never come from the test, death or inputless075 cells. `-Abdicate`, `-Wander` and `-Focus=0` pass the check.

**Commands** (B6a N:413 form):
- **R:** `UnrealEditor-Cmd Cosmic_Ascension.uproject -run=CosmicBalance -Ticks=<C> -Seed=<s> -SaveAt=<t> -TellAll=0 -Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -Out=<abs>`
- **slowR:** the same with `-Focus=0`.
- **Wanderer:** the R form + `-Abdicate=1200 -Wander=1`.
- The expected tick is checked against the log of the cell the seed came from.

**The saves:**
- **①** An R seed with `record RuledPeople` ≤ C. Save at the race line's ruled `T4 at` − 50.
  - If R has none, use slowR (slowR command).
  - If neither has one, write `없음` and raise **[개발자]**: milestone B's console-free ending then rests on the clock end and hand check ⑤.
- **②** An R seed with `record OtherPeople`. Save at the record tick − 30.
- **③** An R seed with `extinction placed on ruled ≥ 1`.
  - Prefer a seed whose race part shows `ruled expiries ≥ 1`, with `died ≥ 1` if one exists (cmd:3970). That serves H:412's `만료까지 가는 시드가 있으면 죽음의 줄이 애도뿐인가`.
  - Save at the first placement's `first at` − 10, and note the expected expiry tick.
- **⑥** Two items when both exist (H:412):
  - (i) era line (#156): an R seed with era `first at` < C, saved at that − 30;
  - (ii) `enc_witness` (#141): an R seed with witness `first at` < C, or else a wanderer seed (wanderer command), saved at that − 30.
  - A missing one is `없음`, plus the windowless screenshot from after [B-6b8].
- **④** uses ①'s save. **⑤** needs no save.

**Procedure:**
- Run them one at a time.
- `-SaveAt` slots are named by `FGuid::NewGuid` (`Sim/GalaxySimulationSubsystem.cpp:736`, `Save/CosmicSaveSubsystem.cpp:297`), so they cannot overwrite a developer save.
- Take the file from the log's `-SaveAt <t> wrote <path>` line (cmd:2515-2516) and move only that file to `Saved\HandCheck\B6b_<n>_<what>_seed<s>_<t>\`.
- Logs and CSVs go to `Saved\Balance\b6b_handcheck\`.
- Write each command, its expected tick and the `wrote` line into the N §5 block.

**If a later code commit lands:** §7 R11.

---

## 5. Mutations

- **New:** none. No assertion is added.
- **Touched:** none. No patched file and no filter test is touched.
- **Whole-B6b rerun:** [B-6b12]'s job (N:768). This commit does not plan it.
- **The harness's own check:** the planted failures in `bundleC_selftest.py` (§2).

---

## 6. Docs and records

1. **N §3 '묶음 C'.** Replace HEAD N:691 `(비어 있다 — `[B-6b11]`이 채운다.)` with content in the B6a N:272-300 form:
   - **Header line:**
     - date; build `<BUILD_SHA>`; 620 (or 560) processes; failures;
     - folder and archive;
     - `end check` and `race check` lines 0;
     - F1-F8 results;
     - B-a to B-e results: which B-d sides ran, the "may move" seeds and the moved count (H:334's "거의"), and each pair's G;
     - sha1 unchanged; `<FROZEN_SHA1>`; `<C_MINUTES>`.
   - **A table per cell:** seeds × ticks, record kinds.
   - **R1-R11** with numbers and Wilson. Record-only items are given as on/off/(on − off), and #129 also as 0.75 against 0.79.
2. **N §5 block** `**`[B-6b11]`**(<date> — 다시 잼(묶음 C))`, appended after the [B-6b10] block. Sections:
   - **나눔:** none.
   - **측정:** the frozen arithmetic of §4.3, written before the run, then a pointer to §3.
   - **손 확인의 시드:** the commands of §4.4.
   - **기준값 그대로:** the five lines and two hash runs as numbers.
   - **리뷰:** counts by lens, confirmed/partial/rebutted.
   - **헤더와 다르게 하거나 더한 것:**
     - B-d as an added bridge, checked per seed with the moved-awakening predictor. R also guards, through the policy's step (1) under `-Diplomacy=1 -Guard=0`, so H:334's "R은 `-Guard=0`" does not mean R never guards.
     - The B-a share split.
     - The B-e check.
     - The inputless075 cell: H:332's list omits it, while H:215 and N:674 call for it.
     - The T_DEATH ≥ C branch, if taken.
     - The two drifted cites.
     - The B1 own-vessel reading and its median measured after leaving.
     - The boundary-tick dedup (> T).
     - F7's tolerance and the OwnerChanged=0 filter in R2.
     - The knob-commit label `[B-6b11b]` (H:43 says `[B-6b11]`).
3. **H §11.**
   - Add one line after [B-6b10]'s, under 400 bytes (target ≤ 370; this form is about 290): `` - `[B-6b11]`(<date>): 다시 잼(묶음 C — 시계 <C>, 620판). 더한 것: #129의 0.75 칸, 묶음 B와의 다리(B-d — 시드마다), B1은 떠난 그릇 자신·떠난 뒤의 중앙값, #10·#154의 칸은 미리 고정; 판단과 수는 `DECISIONS_LOG`. 세부는 노트 §5. ``
   - **Size check:** measure H at the start (`<H_BYTES>`) and after the edit. If H > 149,000 bytes, move at least the excess to N in the same commit:
     - If H '추가 이야기와 보완' (2,710 bytes at HEAD, H:472-479) is still there, move it to N §0 under the heading `헤더 '추가 이야기와 보완'(옮김 — [B-6b11])` and leave a pointer of about 100 bytes.
     - If an earlier commit already made it a pointer, move the details of the oldest §11 lines into N §5 and leave one-line pointers.
     - Either way, the header keeps a '추가 이야기와 보완' heading.
   - Confirm H < 149,000 and that `python -I Tools/check_docs.py` reports 0.
4. **DECISIONS_LOG, B6b section:**
   - **One `[B-6b11]` line** after the [B-6b10] line (HEAD :39 is [B-6b5]'s). As DL:7 requires (`#142의 판단 규칙(결정 24)은 [B-6b11]이 이 절에 적는다`), it carries decision 24's rule as written into this section: the B4 model interval with frozen p_e and the counted-claim filter. It **replaces B6a decision 12's rule**. Add `<N142>`.
   - **The B1 re-judgment `<B1>`.** It closes DL:68's `B6b가 B6a의 마지막 빌드와 짝을 지어 다시 판단한다` through the same-binary pair plus bridge (decision 36).
   - **Numbers:** #136, #132, #129 (0.75/0.79), #154, '< 15%', ends, #10, #158③ and `<RUNLEN>`.
   - **Any [개발자] item raised.**
5. **`OPEN_QUESTIONS.md:20` (#10).** Rewrite the B5 sentence `…6,000틱 MVP 판의 끝 전 죽음 가운데 시계 전에 계승한 몫은 44/59 = 74.6%…`, together with its 넷에 하나, using C's per-death numbers from R9. Keep `도약의 길이 자체는 Phase 5 그대로`.
6. **STATUS.md:**
   - `다음 3작업` item 2 (HEAD :32) → `[B-6b12]` and the hand checks with their seeds.
   - Top 5:
     - item 1 (HEAD :37): C's #136;
     - item 2 (:38): C's #158③, in place of #135's `재지 않음 — 잰 15`;
     - item 3 (:39): `<RUNLEN>` instead of `7/30` and the B6a run length;
     - item 4 (:40): C's #129 transfer numbers next to B3's.
7. **docs/CHANGELOG.md:** journal entry and review counts.
8. **Commit:**
   - Title: `[B-6b11] 다시 잼(묶음 C) — 시계 <C>의 빌드로 620판, #136 <k>/60·#142 <재지 않음 — 잰 N|구간 안>·B1 짝 L <L>/G <G>(p <p>)·#129 0.75/0.79·#10·#158③, 손 확인 시드와 세이브, 비트 그대로`.
   - Body: the runner result and baselines.
   - End it with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`, then push to origin.

---

## 7. Order of work and risks

### 7.1 Order

1. **Workflow start:** §0 (lead).
2. **Harness:** implementer A writes `bundleC.py` and implementer B writes `bundleC_selftest.py`, in parallel.
3. **Harness checks, in order:**
   - the self-test;
   - `--check-only --cell-set B` over bundle B's logs;
   - `--dry-run`: 620 commands; check two jobs per cell by eye, and that no argument contains `ProvisionalEndTick`.
4. **Review lenses**, three, each with rebut-verify per finding:
   - measurement and controls: F1-F8 and B-a to B-e against the H and N text and the ruler policy;
   - frozen statistics: R1-R11 against H:325, N ⓕ, N ⓗ, DL:104, B5 N:228-245 and B6a N:298;
   - hand checks and form: `bSaveShape`, tick choice, file handling.

   Then the fix worker.
5. **Runner `-Tier full`, once.**
6. **Freeze.** Write §4.3 into N §5 and save the block text to `<SP>\b6b11\frozen_4_3.md`. Before job 1, the harness prints the sha1 of these and stores it in `summary`:
   - `bundleC.py`, `bundleC_selftest.py`, `bundleB.py` and `bundleA.py`;
   - `frozen_4_3.md`.

   N §3 quotes them. The docs lens checks that the committed block hashes the same.
7. **Bundle C.**
8. **Readings, then archive.**
9. **Hand-check saves.**
10. **Docs** (§6).
11. **Docs lens:** bytes, verbatim carry-over, placeholder fill, no invented numbers, and the frozen block's hash.
12. **Commit.**

### 7.2 Risks and recommended resolutions (recommended = approved)

- **R1. Another workflow builds or mutates during C.** The sha1 guard and the per-job busy check abort the bundle. Rerun the whole bundle, not part of it, and never mix builds.
- **R2. Bundle B's CSV folder is gone.** B-d falls back to race-line first ticks from the archived logs. The off-side check stays breakable.
- **R3. `<T_DEATH>` ≥ `<C>`.** Drop the death cell (§0.2) and record it.
- **R4. The decision 36 bridge (B-c) or B-e breaks.** Do not read B1.
  - Find the cause by the first differing tick and column.
  - If B-c holds and B-e fails, it is the parser.
  - If a B6b mechanism reaches a world with no rows, that is a [B-6br] fix (H:404).
- **R5. B-d or B-a breaks.** Only an unexplained difference counts: on a seed not predicted "may move", or an off side with no `<BR_WORLD>`. The premise behind [B-6b10]'s C is then broken: stop, find the cause, then [B-6br]. C is not re-decided inside [B-6b11].
- **R6. [개발자] items** (only these). Each goes with its numbers and does not block the commit of the record.
  - #142 with n ≥ 30 outside the interval;
  - B1 unexplained seeds after L > G at p < 0.05;
  - the B1 on-side median > 1,800;
  - #158③ with n ≥ 30 and ≥ 10% (header first);
  - ① `없음`.
- **R7. Failed runs.** Rerun once. Two failures in a cell withhold that cell's readings, written with the names.
- **R8. A developer answer moves a knob.** Make its own commit `[B-6b11b]` (H:43's "따로의 커밋") with:
  - the data change;
  - `-UpdateBaseline` for the moved lines, with a `note` and the new hashes (B6a §5.3 form);
  - runner full `-Data`;
  - mutations for anything touched;
  - a W recount with `<SP>\b6b8b\w_recount.py` if it touches `EventChancePerSlot`, the vigil, the throne lock or a card delay.

  Then rerun bundle C and remake the hand-check saves on that build (H:334 `C에서 읽는 것은 마지막 빌드의 묶음 C가 다시 돈다`, H:411).
- **R9. H crosses 149,000.** Detach as in §6.3. The cap is 150,000; the approved rule is ≤ 149,000 after each commit.
- **R10. Saves.** Hand-check runs write to `Saved\SaveGames\` under GUID names. Run them one at a time, move only the file named in the log's `wrote` line, and never delete the developer's own saves.
- **R11. A code commit lands after [B-6b11]** ([B-6br]).
  - If it moves a baseline, it reruns bundle C on its build and rewrites N §3 '묶음 C' and the affected DECISIONS_LOG numbers (H:334). It also remakes the saves.
  - If it is bit-identical, it remakes only the saves (H:411).

---

## 추가 이야기와 보완

- **Witness column.** The `_claims.csv` 'witness answered' column offered in the b6b8 brief (:763) is outside H, and adding it would be a code commit before C. Recommended: not now. B7 can take it with #141.
- **B-d.** B-d is the only direct test of H:325's claim that a clockless run gives any C's end, and it costs no runs.
  - If it holds, N §3 can say that bundle B's E_i were C's ends for every seed with E ≤ C, apart from the "may move" seeds.
  - It also gives a measured figure for H:334's "거의".
- **#142 power.** About 9-12 counted claims at 140 seeds is the expected result, so `재지 않음 — 잰 N` is no surprise. A judgment would take about 450 test seeds (N ⓕ). That is a measurement-only follow-up outside H and is not proposed here.
- **Drifted cites.** The two drifted cites in the frozen N §2.3 (docs/40:480 → :514; CosmicDataTests.cpp:2234 → :2562) are noted in N §5 and left for [B-6b12]'s re-audit to fix, if the frozen-text rule allows it there.
- **Harness in the repo.** `bundleC.py` and `bundleB.py` can be committed under `Tools/` at [B-6b12] (optional, outside H §8). They are the only executable record of the frozen arithmetic.
- **The 0.75 cell (60 runs at 20,000) costs about 10% of the bundle's time.** If time is tight it may be cut to 30 seeds, never to zero, because H:215 names it.

---

## Critique dispositions

**Critique 1**
1. **Accepted.** H:215 and the frozen N:674 require the 0.75/0.79 pair, and OQ:98 re-measures it when `EndingOf` changes. B6a N:153 is a B7 #155 row, not a ruling against measuring at C. Added the inputless075 cell (60 × 20,000, `-Rules=AIHostilePowerRatio:0.75`), so the total is 620 jobs. Removed `0.75 쪽은 재지 않음`.
2. **Accepted, merged with C2#1.** Verified that `CosmicRulerPolicy.cpp:719` returns early only when `!bGuard && (Diplomacy==0 || ended)` and that step (1) at `:733-764` guards under `-Guard=0`. On-side B-d is now per seed, with the predictor (bundle B log `Hazard.AIAwakening` sum > 75) and a `<K_MOVED>` cross-check. "R does not guard" was removed.
3. **Accepted.** The on side runs at share 0.5 and the off side at share 0 (cmd:1596). B-a now uses the K rule with G recorded and T_B = min(…, G), for wanderer and inputless pairs.
4. **Accepted.** N:185 row 10 and N:318 row 15 assign the OQ:20 sentence to `[B-6b11]`, and DL:7 gives [B-6b12] only closing answers. [B-6b11] now edits OQ:20 (§1.3, §6.5). The "map's conflict" note was dropped.
5. **Accepted, merged with C2#3.** The `git status` check was replaced by `git --no-optional-locks diff --quiet --ignore-cr-at-eol HEAD`, with a `hash-object` fallback for the uassets. Verified locally that the Config diff shrinks to one real line with `--ignore-cr-at-eol`.
6. **Accepted.** The off-side B-d is breakable only when `<BR_WORLD>` is empty; otherwise it is skipped and the reason recorded.
7. **Accepted.** H:412 ③ adds the expiry case and ⑥ names two items. ③ now prefers `ruled expiries ≥ 1` (cmd:3970), ⑥ takes two seeds, and the R, slowR and wanderer command forms are given.
8. **Accepted.** Added §7.2 R11: a baseline-moving code commit after [B-6b11] reruns bundle C; a bit-identical one remakes only the saves (H:334, H:411).
9. **Accepted, merged with C2#9.** Failure branch `깨짐 — 중앙값 <m>` → [개발자], non-blocking; added to R6.
10. **Accepted.** B5 N:244 counts per death (15/59). k is now per death, k/n = 1 − ③, and the per-run column is kept.
11. **Accepted.** Verified that the build loop near `:452` has no end gate and that `:719` stops `-Guard=0` guards after the end. §3 was reworded.
12. **Accepted.** All five verified at HEAD: H:354 (H:355 is blank); rule ⓓ at H:349; `CosmicEndingTests.cpp:622`; DT_Events.json:399; H:325 and H:316.
13. **Accepted, merged with C2#7.** F1 now treats `None`/−1 as a problem.
14. **Accepted.** STATUS Top 5 items 2 (#158③) and 4 (#129) are added to §6.6.
- **Small additions.** The F7 joined∧answered mismatch is accepted (verified: `ClaimEndOf` gives joined precedence, and the band filters `!bAnswered` at cmd:3316); F7 now uses a bound. The `[B-6b11b]` label versus H:43 is accepted and recorded in N §5 and §7 R8, not in the H line (byte budget).

**Critique 2**
1. **Accepted in part (see C1#2).** Per-seed exclusion with the bundle B predictor and the K cross-check is adopted. **Rejected:** "`K_MOVED_*` = 0 as a third validity condition". It would void B-d whenever K moved any seed, while the per-seed exclusion already handles those seeds. **Rejected:** rewriting the H:334 parenthesis here. [B-6b8b]'s brief §6 ① already fixes that cite; [B-6b11] records the point in N §5 only.
2. **Accepted.** Verified B5 N:228-229 and B6a N:298: the judged reading is the vessel that left (40/60, 1,350, paths summing to 40), and `crowned at` crosses lives. R4 now uses own(s) = crowned before the first death. B-e (off side reproduces 40/60, 1,350, 22/15/3) is added as a breakable parser check. The bundleB.py:751-773 preview is not reused.
3. **Accepted** (see C1#5). Plain `git status` is forbidden.
4. **Accepted.** Verified that `DeathAt` at cmd:2521 and `AbdicateAt` at `CosmicWandererPolicy.cpp:20` act at the boundary of tick T, and the claim key carries `State.Tick` (cmd:2602). R10 uses tick > T, and §3 and B-b say "through" T.
5. **Accepted.** The wanderer command form is given for ⑥(ii), and the expected tick is checked against that cell's log.
6. **Accepted.** Added the freeze step: sha1 of the harness files and `frozen_4_3.md`, quoted in N §3 and checked by the docs lens.
7. **Accepted.** Verified bundleB.py:71/:118/:123. bundleC has its own `build_jobs`. F8 forbids `ProvisionalEndTick` and enforces one `-Rules=` (cmd:1482 reads only the first). F1 has no None branch, and a self-test case was added.
8. **Accepted.** Verified that `Ruled` is set at placement (cmd:2751) and the band needs `bRuledSettled` at the end (cmd:3303). OwnerChanged=0 was added to the F7 and R2 filters; OwnerChanged=1 rows are reported as a note and enter F7's bound O.
9. **Accepted** (see C1#9).
10. **Accepted.** Verified the [B-6b8] layout (b6b8 brief :345): the witness `first at` is the first card. Paths (b) and (c) are marked ≤ C as upper bounds and listed apart.
11. **Accepted in part.** The per-job busy check and the Config ini files in the sha1 set are added. **Rejected:** adding the string-table asset. It is `Content/CA/Data/ST_CA.uasset`, already covered by `Content/CA/Data/*.uasset`.
12. **Accepted.** Added `<H_BYTES>` measured at the start, and the fallback of moving the oldest §11 details to N §5 while keeping the '추가 이야기와 보완' heading.
13. **INFO, no change.** Used the GUID-save fact to refine §4.4 and R10: take the file from the `wrote` line.

The revised brief's working notes (HEAD copies of H, N, DL, OQ, STATUS, cmd and the ruler policy) are in `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b11rev\`.