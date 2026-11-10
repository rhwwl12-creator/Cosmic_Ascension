# [B-6b9] implementation brief: values only, then B′ (conditional commit)

Everything below was read at HEAD `10205b6`:
- **H** = `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` (147,498 bytes at HEAD).
- **N** = `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md`.
- **SP** = `C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad`.

Nothing was written in the project.

**Effort and review:** effort high. H §8 (H:396) requires no ultracode review; its check column says "같은 판단".

---

## 0. Start state

### 0.0 Freezing the readings (now, before `[B-6b8]` runs bundle B)

1. When the lead saves this brief as `SP/b6b9/brief_final.md`, it also writes the sha1 of the text of §1.1–§1.5 to `SP/b6b9/readings.sha1`, with the date.
2. In step 1 of §7, N §5 copies §1.1–§1.5 verbatim with that sha1 and date, under the label "`[B-6b9]`의 읽음 — 묶음 B 전".
3. The readings therefore predate bundle B's data, not just the probes. The R1/R2 classification of bundle B is done by script (`--classify`, §4.1), never by hand.

### 0.1 Whether the commit exists (step 0, before any implementer)

**Step 1. Read `git log --oneline -8`.**
- It must show `[B-6b6]`, `[B-6b7]`, `[B-6b8]` and `[B-6b8b]` after `10205b6`. H §8 runs `[B-6b8]` → `[B-6b8b]` → `[B-6b9]` (H:394-396).
- If `[B-6b8b]` is missing, stop. B′ must run on the build after `[B-6b8b]`, the last build before the clock.

**Step 2. Read N §3 "묶음 B" and the archived `summary.json`** under `Saved\Balance\b6b_bundleB\`.
- Take the bundle B placeholders (§0.3) from there.
- Take `<T_DEATH>` exactly as bundle B ran it, from N §3 묶음 B / `summary.json`. Do not read it again from N §2.

**Step 3. Gates.** Each gate is read from bundle B's record before deciding existence.

| Gate | Condition | Route |
|---|---|---|
| **G1 — time** | Serial runner's AI-stage µs/tick (`summary.json` "AI us" → "serial") above 1.5 × 10.7 = 16.05 µs (`bundleB.py:775-778`; SP/b6b8/brief_final.md:554; N:667 "B6a 10.4~10.7의 +50%를 넘으면 `[B-6b9]` 전에 까닭을 찾는다") | The lead finds the cause first. A code cause goes to `[B-6br]`. Only an unexplained cause goes to [개발자]. `[B-6b9]` waits until the cause is found. |
| **G2 — locks (reading R0)** | ≥ 3/60 R runs in which the binding system was lost and nobody reached the fifth rung | Report #165 (`SealedByCivID`, decision 12 ㉯) to [개발자] "그 수와 함께" (N:667). Decisions 13·14 are revisited together (H:474). **Hold `[B-6b9]`.** A `SealedByCivID` fix is a hash-moving code path, so it would make a single-shot B′ obsolete. [개발자] may release the hold. |

**Step 4. Decide existence** (H:338: "어긋나면 `[B-6b9]`").

| Bundle B outcome | Result |
|---|---|
| A cell is **withheld**: ≥ 2 twice-failed runs (`bundleB.py:622-630`; SP/b6b8/brief_final.md:489-494 "report to [개발자] before any judgment") | No commit; [개발자] with the seeds. Cause first. |
| ① fails (twice-failed R runs count as not reached) | `[B-6b9]` |
| ② fails **and** N §5 ⑧ reads "r one ladder step" (`bundleB.py:509-516`) | `[B-6b9]` |
| ③ length fails | `[B-6b9]` |
| ③ density fails alone ("결정 밀도는 값으로 고치지 않는다 — [개발자]", H:344) | No commit; [개발자] |
| ③ construction assertion violated (some r5 − r4 < 1,200; `judge_3` `violations`; SP/b6b8/brief_final.md:540) | Not a value trigger. Find the cause: `[B-6br]` if code, else [개발자]. |
| ② or ③ **재지 않음** (n < 30, `judged=False`; H:325 "30건 이상일 때만 판단, 모자라면 '재지 않음 — 잰 N'"; `bundleB.py:503-505`, :563-565) | Neither pass nor fail. It does not open `[B-6b9]` by itself. Record it with n; list it for [개발자]. |
| ④ fails ("규칙이 정한다", H:345) | Not a `[B-6b9]` trigger (no C → [개발자], as `[B-6b8]` wrote) |
| ② fails with ⑧'s reading "[개발자] with the split" | No commit for ②; [개발자] |
| Every judgment passes | No commit; STATUS '다음 3작업' → `[B-6b10]`, as `[B-6b8]` already wrote |

When the commit does not exist, the workflow returns `NO-COMMIT: <reason>` and does nothing.

**Step 5. Mixed outcome.**
- If ② went to [개발자] and ① or ③ length also failed, wait for the developer's answer on ② first.
- Only one tuning and one B′ are allowed: H:331 "한 번 고쳐도 어긋나면 그 수와 함께 [개발자]", H:343 "B′로 한 번", N:706 측정-2 "B′ 한 번 그대로".

### 0.2 Re-check on the tree

Read with `git show HEAD:` only. Line numbers move after `[B-6b6]`–`[B-6b8b]`, so find each item by name.

| What | Where at `10205b6` | After the earlier commits |
|---|---|---|
| r | `Tools/data/DT_Rules.json:17` = `1.0` | `<R_A>` from `[B-6b7]` (0.375 unless its paragraph moved it) |
| P_bind | `DT_Megaprojects.json:67` = `1800.0`, an unquoted number (row `project_binding` :63; `CostPerTick` :66) | `<P_BIND_A>` |
| P_ark | absent at HEAD | `project_ark` `RequiredProgress` (150) |
| c | absent at HEAD | `crisis_species_extinction` `SpawnChancePerTick` = `<C_A>` (1e-4 planned) |
| N | absent at HEAD | `feat_laws_and_binding` → `CivLawsHeldTicksAtLeast` `Key` (digits only, `schema.json:1278`) = `<N_A>` |
| Share | `DT_Rules.json:27` = `0.0` | 0.5 after `[B-6b8b]` |
| `-Rules` parsing | `Tests/CosmicBalanceCommandlet.cpp:1482` reads only the first `-Rules=`. :1540 applies it before the off switch (:1586-1597). | — |
| Off switch | `:1590-1597`: removes B6b's rows, sets **r → 1** and **share → 0**. It logs "AIUpperRungProgressMultiplier 1 (was %g), FleetDelayCountdownShare 0 (was %g)". | — |
| Where r acts | `Sim/CosmicMegaprojects.cpp:20-28`; `Data/CosmicDefinitionRows.h:342`: a non-ruled people's construction of a row at the third rung or above | — |
| Ark margin test | `Tests/CosmicDataTests.cpp:2195-2235`: `RequiredProgressOf / AIAscensionSpeedMultiplier < CountdownTicks`. r is not in it. | — |
| r pin | absent at HEAD | added by `[B-6b7]` (SP/b6b7/brief_final.md): 0 < R ≤ 1, and R×16 a whole number |
| Race fields for R1/R2 | `cmd.cpp:3968-3979`. Parts used: "ruled binding started at", "laws first held ruled at … AI at …", "longest held ruled … AI …", "resets ruled … AI …", "binding at … by … ruled rung …", "locked" | — |

### 0.3 Placeholders

Never invent a value. Each placeholder is filled only by the rule named.

| Placeholder | Filled by | Lands in |
|---|---|---|
| `<J1_B>`; `<J2_B>` with (가)/(나)/(다); `<GA_SHARE_B>` and its side; `<J3_LEN_B>`; `<J3_DENSE_B>`; withheld cells; n₂, n₃ | `[B-6b8]` bundle B (`bundleB.py` `judge_1`–`judge_3`, `judgments`), read from N §3 / `summary.json` | §0.1, §1 |
| `<R1_COUNTS>`, `<R2_COUNTS>` | `--classify` over bundle B's logs (§4.1) | N §5; the lever choice |
| `<R_B9>` | `<R_A>` ± 0.0625, only by §1.2 | `DT_Rules.json` r line; `docs/30` regenerated |
| `<C_B9>` | the §1.3 ladder | extinction row |
| `<N_B9>`, `<P_BIND_B9>` | the §1.4 ladder | feat Key; binding row |
| `<P_ARK_B9>` | = 150, unchanged (§1.5) | — |
| `<T_DEATH>` | bundle B's own t (N §3 묶음 B) | `--t-death` |
| `<K_MOVED_R>`, `<K_MOVED_W>` | `[B-6b8b]` bundle K (N §5) | the expected rate in §4.3's pair reading |
| `<J1_BP>`, `<J2_BP>` (+split), `<J3_BP>`, `<SLOW2_BP>`, `<C0_BP>`, `<C_BP>`, `<BP_MINUTES>`, `<BP_SHARE_MOVED>` | B′ (§4.3) | N §3. `[B-6b10]` reads C from B′ (H:349 ⓐ). |
| `<HASHES_B9>` | runner `-UpdateBaseline` | `seed42.json` note, commit message |
| `<MUT_6b6_6b8b>` | the patch scan (§5) | §5 |

---

## 1. The changes

The commit changes values only:
- no new rows;
- no new hash-moving path;
- no code or header change (H:43, H:396, H:403).

Draw counts do not change, because a `Draw` row draws once per settled system per tick, whatever its chance (CLAUDE rule 3). So the `docs/20` draw ledger stays as it is.

### 1.1 Frozen readings (§0.0; recommended = approved)

These fill H's gaps.

**R0 — locks.** G2 in §0.1 holds `[B-6b9]` (reason given there).

**R1 — which bridge ① blames** (H:342 "늦은 다리부터": start from the bridge that is late).
- `--classify` sorts every R run of bundle B that did **not** reach the fifth rung, using the race part 3 fields and `binding at … by …`.
- B = max(P_bind, N).

| Class | Condition | Lever |
|---|---|---|
| (T4) | ruled `r4` = −1, or `r4` > 20,000 − B. The ruled people never got time for the fourth-rung season. | **c up** |
| (T5) | ruled `r4` ≥ 0 within that bound; no binding, or the binder is the ruled people; no fifth-rung record | **N or P_bind down** (R2) |
| (T5-AI-held) | the binding completed by a non-ruled people; that people held its laws at some point (AI `laws first held` ≥ 0); no fifth-rung record | **N down** |
| (T5-AI-never) | the binding completed by a non-ruled people that never held its laws | No value lever: `GalaxyLawsToHold` is not an H §5.3 value (H:338). List it for [개발자]. |
| (other) | the record came first (`NoCandidate`, `Vessel`), or the run is locked or twice-failed | No value lever; list these |

- The lever is the class with the most runs among (T4), (T5) and (T5-AI-held). (T5) and (T5-AI-held) count together as "넷째 → 다섯째".
- If (T5-AI-never) is the largest class of all, ① goes to [개발자] with the counts and no value moves for ①.
- **Tie** between "c" and "넷째 → 다섯째": both levers are live, searched in the order of H's own sentence (c first, then N·P_bind; §1.6). "늦은 다리부터" names the late bridge, not "the later bridge". `[B-6b8]`'s gloss "latest bridge first" (SP/b6b8/brief_final.md:537) is not the authority.

**R2 — N or P_bind** (for ③ length, or for ① through (T5)).
- Take each ③-eligible run (`judge_3`'s n₃) plus each (T5) run with a ruled binding.
- Laws done at = `laws first held ruled` + N when `resets ruled` = 0.
- A run with `resets ruled` > 0 counts as "laws". Law breaks lengthen the laws bridge: N ⓓ's λ, N:619; the field is printed at `cmd.cpp:3976`.
- The last condition is the later of (laws done, the ruled `binding at`).
- Majority "binding" → lower P_bind. Majority "laws" → lower N.
- A tie → P_bind. N:614: "N은 누가인지를 바꾸지 않고 언제를 바꾼다". P_bind moves both who and when.
- Lowering N does nothing while the binding is the last condition, so R2 must choose P_bind there.
- (T5-AI-*) runs are excluded from R2.

**R3 — floor.** H:344 asks only when `[B-6b9]` takes **both** P_bind and N below 1,200.
- `[B-6b9]` keeps max(P_bind, N) ≥ 1,200 always.
- If only a value that breaks this floor would pass, go to [개발자] with the probe numbers.
- This supersedes `[B-6b7]`'s reading "N below 1,200 → [개발자]" (SP/b6b7/brief_final.md:52). That reading asked about N alone. Record the change in N §5.

**R4 — joint changes and probe acceptance.**
- All changed values go into one probe configuration and one B′.
- r moves only by §1.2, from bundle B. It is never tuned by probe.
- Every probe computes ①, ②, ③ length, ③ density and the ④ forecast with the frozen thresholds (`bundleB.py`), labelled "탐침 — 판단 아님".
- A ladder step is **accepted** only if two things hold:
  - the target judgment passes;
  - every judgment that passed (or was measured and passed) in bundle B still passes.
- **If a step makes the target pass but breaks another judgment**, stop at that step and go to [개발자] before B′. Do not skip ahead on the ladder.
- **② out of band from other levers.** The model says c pulls ② down and a lower P_bind pushes it up (N:595/597, :602-605). So when c, N or P_bind changes, a ② forecast outside 15–35% on either side goes to [개발자] before B′.
- **② after an r step alone.** When r is the only change, a forecast on the side opposite to r's move goes to [개발자] (overshoot). A same-side forecast still runs B′, as H:343 ordains.
- A forecast never moves r.

**R5 — after B′.**
- Commit the values with B′'s numbers, whatever B′ shows.
- A B′ judgment that fails goes to [개발자] "그 수와 함께" (H:331, H:343). No value is changed again.
- A B′ discarded by a control, guard or harness failure that the values did not cause may be rerun **once**, with identical values and seeds, after the cause is fixed (`[B-6br]` if code). H:331 limits tuning, not executions, and a deterministic rerun is not a second tuning.

**R6 — the ⑧ tension.** N:706 측정-2 records that the header review declined "띠 밖이면 곧장 [개발자]". `[B-6b8]`'s frozen ⑧ ("[개발자] with the split" when (가) alone is not outside on the same side) narrows H:343. ⑧ stands because it was frozen before the data. Note the tension in N §5 and in the DECISIONS_LOG line, so the developer sees it.

### 1.2 r (only if ② failed and ⑧ reads "r one ladder step")

Rule (H:343, verbatim): "35% 위면 r을 한 칸 내리고 15% 아래면 한 칸 올려 B′로 한 번 — 사다리는 이진수로 정확한 1/16 걸음 0.25·0.3125·0.375·0.4375·0.5 … B′의 점도 밖이거나 사다리 밖이 들면 그 수와 함께 [개발자]".

- **Value:** `<R_B9>` = `<R_A>` − 0.0625 if above the band, + 0.0625 if below. A result outside [0.25, 0.5] goes to [개발자].
- **Edit:** the r line in `DT_Rules.json`. Leave the C++ default `Data/CosmicDefinitionRows.h:941` at `1.f`.

### 1.3 c (only if R1 gives (T4))

- **Ladder:** from `<C_A>` in steps of +0.5e-4, up to a ceiling of 2.5e-4 (from 1e-4: 1.5e-4 → 2e-4 → 2.5e-4).
- Keep the first step that R4 accepts with J1 ≥ 40/60.
- **Ceiling reason:** the model at c 2.5e-4 already puts ② at 12–15% (N:603, :605). At 5e-4 the band would need r between 0.5 and 0.75 (N:607: 0.38 at r 0.75; 0.03 at r 0.5), which is off the ladder.
- Past the ceiling → [개발자].
- **Sanity note, not a check:** the Draw chances are awakening 2e-4, plague 1e-4 and pre-supernova 1e-4, plus c.
- c raises how often extinctions hit rung 4 and above, and therefore the ruler's death (H:421). Record ruler deaths in N §5.

### 1.4 N or P_bind (only if ③ length failed, or R1 gives (T5)/(T5-AI-held))

- **Lever** from R1/R2. (T5-AI-held) always means N.
- **Ladders, relative to the real start values:**
  - P_bind: `<P_BIND_A>` − 300k;
  - N: `<N_A>` − 300k;
  - k = 1, 2. From 1,800 / 1,200 that gives 1,500 → 1,200 and 900 → 600. N:614: 600 and 1,200 behave the same at P 1,800.
- R3 bounds both ladders.
- Keep the first step that R4 accepts:
  - for ③ length: lower median ≤ 3,600 on seeds 1–60;
  - for ① via (T5): J1 ≥ 40/60.
- **Edits:**
  - P_bind: the `project_binding` `RequiredProgress`, as an unquoted number (`1500.0`).
  - N: the feat Key, as digits only.

### 1.5 P_ark, the binding cost and the rest

- **P_ark stays 150.** No branch in H §5.3 moves it; "(나) c·P_ark의 몫" is attribution only (H:343).
  - Record in N §5: "P_ark handed over by `[B-6b6]` (SP/b6b6/brief_final.md:117); no H §5.3 rule moves it."
  - The ark margin test is untouched.
- **The ark counter** offered by SP/b6b8/brief_final.md:347-350 ("expired with the ark standing") is **declined**. A commandlet field is a code change, and this commit is values only (H:396). If `[B-6b7]`/`[B-6b8]` added it, read it. Otherwise B′ writes "재지 않음 (카운터 없음)". Note it in N §5.
- **The binding's `CostPerTick`** (`DT_Megaprojects.json:66`) is not an H §5.3 value (H:338 lists c·P_ark·P_bind·N only).
  - Record "`[B-6b7]` risk 4 closed: no rule moves it" in N §5, with bundle B's unpaid-binding figure U if it was measured.
  - If U dominates the season, route it to [개발자] / `[B-6b11]`.
- **Not levers here** (H:338, H:43): `-FourthRungCap` (probe only), `GalaxyLawsToHold`, `FleetDelayCountdownShare`, `ProvisionalEndTick`.

### 1.6 Search order and data pipeline

**Search order (frozen):**
1. Apply the r step (§1.2), if any.
2. Run ①'s lever ladder per R1 (on a tie: c first, then N·P_bind).
3. If ③ length also failed, run its R2 ladder on top of the kept values.

There is a cap of **6 probes** in total (about 54 min); beyond it, [개발자].

The **final probe** runs on the exact candidate data, with r in the JSON too (no `--rules-extra`). The same-seed equivalence in H:338 then holds for the very data B′ uses.

**Pipeline:**
1. Edit the JSON.
2. Run `python -I Tools/check_data.py`. Schema bounds: r `gt 0` (`schema.json:271`); `RequiredProgress` `min 1` (:869); N Key digits only (:1278).
3. Run `make_data_tables.py` → `.uasset` and `generated.sha1`.
4. Build per §7 before B′.
5. Run the runner `-Tier full -Data -UpdateBaseline` (§3).

Do not use Live Coding.

---

## 2. Tests

**No new assertion is planned.** These existing ones read the new values and must still pass:
- `Data.Integrity`: r ladder pin, ark margin, laws ≤ rows, and the race pin from `[B-6b7]`;
- `Data.SourcesInSync`;
- `Sim.UpperRungDelay`;
- the `WithoutB6bData` tripwire. Its full copy sets its own 0.375 (`CosmicDataTests.cpp:2715`) and does not read the data's r.

**Tests whose meaning moves with c, r or P_bind** (N:436-448; SP/b6b7/brief_final.md §2.4). Rule (N:425): on a break, find the cause before touching a seed or an expected value. Record every outcome in N §5. Never absorb a break with seeds or expected values.

| Test | On a break |
|---|---|
| `Sim.PhaseTwoExit` (`Tests/CosmicSimTests.cpp:2260`, seed 18, 20,000) | An AI binding before the first annexation freezes the sphere. Write the cause in H §11 and N §5, then make the test read annexations before the first binding tick (N:447). Do not change the seed first. |
| `Sim.WandererLoop` (`Tests/CosmicWandererTests.cpp:1241`, seed 4) | Cause first (#122, extinction erasure); write it in H §11. |
| `Sim.PlayerAscendsToT3` (`Tests/CosmicAscensionTests.cpp`) | If an AI's fifth rung stands, widen to "the clock's or another order's" (as `[B-6b7]` §2.4). |
| `Feats.Reachable` | As `[B-6b7]` §2.4. |
| `Sim.RealmCap`, `Sim.CivAIProjects`, `Sim.NotablesCapped`, `Sim.NotablesFromEventsOnly`, `UI.RunEnd`, `UI.EdictWindow` | Same shape; cause first. |
| Any other test that ticks a T4 people on game data, where B6b is not the subject | Put `CosmicTests::WithoutB6bData` outermost (the `[B-6b3b]` precedent). Never switch off B6b data in a test whose subject is B6b (SP/b6b8/brief_final.md §2.4). |

`[B-6br]` is only for a defect found in code.

**Re-grep on the tree before editing** for literals that pin a value that will move:

```
git grep -n "0\.375\|0\.4375\|0\.3125\|0\.25\b\|0\.5\b\|1800\|1,800\|1500\|1200\|1,200\|900\|600\|0\.0001\|1e-4\|0\.00015\|1\.5e-4\|0\.0002\|2e-4\|0\.00025\|2\.5e-4" -- Source/Cosmic_Ascension/Tests Tools/*.py Tools/data/schema.json
```

- A test that reads the game value by name keeps its meaning.
- A literal that pins the old value has its meaning changed:
  - change the literal;
  - add the comment "(값은 `[B-6b9]` — 묶음 B의 판단 <#>)";
  - break that assertion once (§5).
- Literal edits are made **before** the pre-B′ build (§7), so B′ runs on the committed build.

---

## 3. Determinism and baselines

**Lines** (N:660 row `[B-6b9]`: "값이 닿는 줄 | 같다 | 같다 | 같다 | 짝 = `[B-6b5]`"):

| Value | When it starts to move state | Lines expected to move |
|---|---|---|
| c | the extinction row's draw outcomes, from the first eligible system (around the first AI third rung, about 1,521) | all five lines and both hash runs |
| r | the first tick a non-ruled people builds a row at rung 3 or above (`race:` "upper construction first started by an AI at") | a 3,000-tick line moves iff that tick < 3,000 in its run |
| P_bind, N | only once a binding completes, or the feat check passes, under either value | a 3,000-tick line moves only if that happens before 3,000 |

**Record:**
- Run `Tools\run_checks.ps1 -Tier full -Data -UpdateBaseline` once. Its two in-process passes are the two consecutive test runs (N §5 `[B-6b3b]` '일의 속도').
- Put the reason and new hashes in `seed42.json` `note` (H:358).
- `git diff Tools/baselines/seed42.json` names the moved lines. None is claimed unmoved without that evidence.

**Must stay 0:** `end check` and `race check` lines in `Saved\Logs\checks*.log` (grep them). The hash runs must equal the commandlet; the runner compares them.

**Pair ⓐ — a regression guard (H:317).**
- Run the runner's five `Compare-Balance` steps (`run_checks.ps1`: 3,000 none; ruler 3,000; wanderer 6,000; 20,000; ruler 20,000) with `-WithoutB6bData=1` appended.
- Each must print `10205b6`'s `seed42.json` line: 2816687688/3300228750; 3561771757/2462331303; 2665013209/2229239127; 2359424635/726895141; 1611658162/29194274; plus the ruler and wanderer tails.
- Compare with `, wrote …` removed, as `Remove-WrotePath` does (`run_checks.ps1:76`), against `git show 10205b6:Tools/baselines/seed42.json`.
- The off side removes every row whose value changes and sets r → 1 and share → 0. So ⓐ cannot see the change itself; it only shows that nothing outside B6b's rows moved. Any mismatch → stop.
- The evidence of the change is the `seed42.json` diff plus the witnesses in §4.3.

**`SaveVersion` stays 1;** no field is added. Rule 7 does not apply, because these are row values, not save-field initial values.

---

## 4. Measurement

### 4.1 Harness: `SP/b6b9/bundleB.py`, a copy of `SP/b6b8/bundleB.py`

`DEFAULT_BUNDLE_A_PY` (`:56`) still resolves `../b6b6/bundleA.py`. Edits:

1. **`--share` (default `"0"`).** The off check at `:355-356` compares against it. B′ passes `--share 0.5`; otherwise every off log's "(was 0.5)" is flagged.
2. **`--bprime`.** In this mode a failure of ①, ② or ③ length reads "[개발자] — B′ 한 번 (H:331)", not ⑧'s "r one ladder step … then B'" (`:514`).
3. **Defaults.**
   - Folder → `SP\b6b9\bundleBprime` (was `bundleB`, `:1068`).
   - Archive → `<root>\Saved\Balance\b6b_bundleBprime` (was `b6b_bundleB`, `:1151`).
   - Refuse any archive dir ending in `b6b_bundleB`.
4. **Probe flags.**
   - `--rules-extra "Name:Value;…"` is joined into the **single** `-Rules=ProvisionalEndTick:0;…` argument (`:1482` reads only the first).
   - `--on-only` makes probes skip the off side and the pair controls.
   - Every judgment line carries the label "탐침 — 판단 아님".
   - One summary row per probe holds ①, ② with (가)/(나)/(다), ③ length, ③ density and the ④ forecast.
5. **`--classify`** over a bundle folder or the archive logs. It prints the R1 classes and R2 majorities (§1.1) per seed, using the `race:` parts listed in §0.2.
6. **The B′ pair reading (frozen in N §5 before B′).** After `[B-6b8b]`, the R policy guards its own besieged systems under `-Diplomacy=1 -Guard=0`:
   - `World/CosmicRulerPolicy.cpp:711-780`;
   - the wanderer run also guards outside its first reign (SP/b6b8b/brief_final.md §3).

   B′'s on side runs share 0.5 and the off side runs share 0, so a pair can now differ before T_B, which bundle B never faced. For every pair (R, slowR, wanderer, input-less):
   - **Same before T_B.** The pair is checked below T_B as before.
   - **Allowed difference.** A difference below T_B is allowed only if the **off** twin's `conflict:` lists `Hazard.AIAwakening=<sum>(<count>)` with sum > 75. This is `[B-6b8b]` rule (i), and K1/K2 for input-less and R/wanderer.
   - **How an allowed pair counts.** It is reported "moved by the share" and is not a control failure. Its runs are still judged. The count goes in `<BP_SHARE_MOVED>`, set against `<K_MOVED_R>`/`<K_MOVED_W>`.
   - **Any other difference below T_B** is a control failure (R5's rerun clause applies).
   - **Bridges.** The death bridge (< T_DEATH) and the wanderer bridge (< 1,200) are on-side same-build comparisons and keep the strict rule.
7. **B1 preview.** Keep it; it is same-build and not judged.

**Self-test** (`bundleB_selftest.py`, adapted):
- a case with n₃ = 29 must return "재지 않음", not fail;
- a withheld cell must withhold;
- a pre-T_B difference with and without the off twin's awakening sum > 75;
- the `--classify` classes on synthetic race lines.

Also:
- `--check-only` over `SP\b6b8\bundleB` with `--share 0` must reproduce bundle B's verdict. If the scratch folder was cleared, skip this and say so; the archive holds logs only.
- `--dry-run --seeds 61-120` must print 540 jobs. `--check-only` also needs `--seeds 61-120` (`:1069`).

### 4.2 Probes (seeds 1–60, never judged — H:338)

- **Cells:** R 60 and slowR 60, on side only (120 runs, about 9 min at 6 workers).
- **How values are applied:**
  - r goes through `--rules-extra AIUpperRungProgressMultiplier:<R_B9>`, except in the final probe (§1.6);
  - c, N and P_bind go through an uncommitted working-tree JSON edit, then `check_data` and `make_data_tables`, before each probe.
- **Each probe records:** the §4.1 summary row, and the guard sha1s. Apply R4 and the §1.3/§1.4 ladders in the §1.6 order.
- **Stop at [개발자]** in any of these cases:
  - a ladder is exhausted;
  - the 6-probe cap is reached;
  - R3 binds;
  - R4 rejects a step.
- **Guards** (as in bundle B):
  - no editor or UBT running and nothing building;
  - a sha1 guard on the DLLs, on `Content/CA/Data/*.uasset` and on `generated.sha1`.

### 4.3 B′ (on the final `[B-6b9]` data and build)

**Before starting:**
- `git status --porcelain` shows only the intended JSON, test-literal and docs files.
- `check_data` passes.
- The editor target is built from that tree (§7).
- Record `generated.sha1`, the JSON sha1s and the DLL sha1s; `bundle_meta.json` holds the last.

**Command:**

```
python -I -X utf8 SP/b6b9/bundleB.py --seeds 61-120 --t-death <T_DEATH> --r <R_FINAL> --p-bind <P_BIND_FINAL> --n-laws <N_FINAL> --share 0.5 --bprime ^
  --folder SP\b6b9\bundleBprime --archive-dir <root>\Saved\Balance\b6b_bundleBprime [--serial-logs …] [--a-sec-per-20k …]
```

- `<R_FINAL>` = `<R_B9>`, or `<R_A>` if r did not move. The harness checks every off line's "(was r)" against it.
- `<P_BIND_FINAL>`/`<N_FINAL>` = the committed P_bind and N (the harness refuses a real B′ run without `--r`, `--p-bind` and `--n-laws` - the §4.3 witnesses; it also refuses `--on-only`, `--rules-extra` and seeds outside 61-120 in B′ mode). Probes (§4.2) always pass `--p-bind` and `--n-laws` too, so R3's floor is always checked (lead note 2026-11-10, after the harness review).
- **Size and time:** 540 runs at 20,000 ticks with `-Rules=ProvisionalEndTick:0` (H:330). Cells: R, slowR, wanderer and input-less, each with its pair; plus death = R + `-DeathAt`. About 35–42 min at 6 workers (SP/b6b8/brief_final.md:458).

**Controls.** Every bundle B control must hold again (SP/b6b8/brief_final.md:467-505):
- 0 `end check` / `race check` lines;
- the pair reading of §4.1 item 6, and both bridges;
- the switch and field controls;
- the sha1 guard.

A control failure that the values did not cause → R5's single rerun after the fix.

**Witnesses that B′ ran on the intended values:**
- off lines "(was `<R_FINAL>`)";
- in R runs where the ruled people bound: `binding at` − `ruled binding started at` ≥ `<P_BIND_FINAL>`;
- in ③ runs: r5 − r4 ≥ max(P_bind, N);
- after the step-8 clean asset rebuild: `generated.sha1` and the JSON sha1s equal B′'s;
- no source changed after B′ started, and the final runner's DLL equals B′'s `dll_sha1`.

Any mismatch → rerun the identical B′ once (R5).

**What B′ shows:**
- ①–③ with the same thresholds and the same 재지 않음 / withheld rules;
- ② slow, reported (H:343: if its interval lies wholly above 35%, show it to [개발자] with ④'s acceptance);
- ④ recomputed on B′ slowR seeds 61–120 (H:349 ⓐ), giving `<C0_BP>` and `<C_BP>` for `[B-6b10]`;
- the record-only items, the AI µs/tick and `<BP_SHARE_MOVED>`.

### 4.4 Pre-written judgments, carried over verbatim (H §5.3, H:338-345)

> **묶음 B의 판단**(어긋나면 `[B-6b9]` — 탐침은 `-Rules`(r)·`-FourthRungCap`, 행의 값(c·P_ark·P_bind·N)은 커밋하지 않은 데이터 사본으로; 탐침의 판은 판단에 쓰지 않는다 — 탐침은 묶음 B의 시드 1~60으로 하고 B′의 판단과 시계 규칙 ⓐ(B′일 때)는 새 시드 61~120으로 한다(같은 시드의 B′는 결정론으로 탐침의 판 그 자체다)):
>
> | # | 판단 | 칸 | 규칙 | 어긋나면 |
> |---|---|---|---|---|
> | ① | 다섯째 칸에 닿는다 | R 60 | 기록이 집단 승천이나 다른 질서이고 그 틱이 20,000 이하인 판(노트 §2 ⓗ) ≥ 40/60 | 늦은 다리부터 — 다스리는 넷째 칸이 늦으면 c를 올리고, 넷째 → 다섯째가 길면 N·P_bind를 내린다 |
> | ② | '다른 백성이 먼저'의 띠(#161) | R 60의 다섯째 칸 끝(≥ 30) | 다른 질서 15~35% — 분모는 ①의 판, 분자는 기록이 다른 질서이고 그 백성을 어느 생애도 다스리지 않은 판(다스린 적 있는 백성의 다른 질서 — `ending:`의 '어느 생애가 다스린 백성' — 는 분모에 두고 따로 적는다). 점이 띠 밖이면 어긋남; 95% Wilson 구간은 함께 적는다(n = 60에서 구간이 통째로 띠 밖인 것은 ≤ 3/60·≥ 29/60). 다른 질서의 판은 (가) 다른 백성이 결박을 마친 틱에 다스리는 백성이 넷째 칸 이상(r이 닿는 경주)과 (나) 다스리는 백성이 절멸로 지워졌거나 넷째 칸에 닿지 못함(c·P_ark의 몫)으로 갈라 적고, r을 옮기는 판단은 (가)의 몫으로만 한다. 느린 R 60은 같은 셈으로 몫과 구간을 적어 결정 35의 답대로 다룬다(권장: 알린다 — 구간이 통째로 35% 위면(모형 63% — 노트 §2 ⓒ) 시계 규칙의 받아들임과 함께 [개발자]에게 보인다, 시계의 까닭이 사람의 걸음을 느린 R에 두므로; 위 끝을 두기로 하면 그 값은 개발자가 정한다) | 35% 위면 r을 한 칸 내리고 15% 아래면 한 칸 올려 B′로 한 번 — 사다리는 이진수로 정확한 1/16 걸음 0.25·0.3125·0.375·0.4375·0.5(시작값 0.375; 모형으로 R의 다른 질서 9%·17%·26%·34%·43% — 노트 §2 ⓒ); B′의 점도 밖이거나 사다리 밖이 들면 그 수와 함께 [개발자] |
> | ③ | 넷째 칸의 계절 | ①의 판 가운데 다스리는 백성이 다섯째 칸에 선 판(≥ 30) | 넷째 → 다섯째 틱의 중앙값 ≤ 3,600(아래 끝 1,200은 구성상 지켜진다 — 계절 ≥ max(P_bind, N); `[B-6b9]`가 둘 다 1,200 아래로 내릴 때만 묻는다); 그 계절 600틱 창마다 결정 — 판마다(창마다 결정 수의) 중앙값 ≥ 1인 판 ≥ 90%(B6a 묶음 0·1의 ⓑ는 모든 판으로 판단했다 — 30/30·28/28; 창은 노트 §2.1) | 길이는 N·P_bind; 결정 밀도는 값으로 고치지 않는다 — [개발자] |
> | ④ | 시계 C | 느린 R 60 | 아래 규칙 | 규칙이 정한다 |

**The B′ row (H:331):** "B′ | `[B-6b9]`(판단이 부를 때만) | B의 칸, 시드 61~120 | 같은 판단 — 한 번 고쳐도 어긋나면 그 수와 함께 [개발자]".

**H:325:** "**30건 이상일 때만 판단**, 모자라면 '재지 않음 — 잰 N'".

**Clock rule ⓐ as it applies to B′ (H:349):** "ⓐ **칸**: 느린 통치자 판(R의 `-Focus=0`) 시드 1~60(값을 고쳐 B′면 61~120), 시계 없이, 지평 20,000, 묶음 B(값을 고쳤으면 B′)의 빌드."
- ⓑ–ⓕ are unchanged and belong to `[B-6b10]`.
- The frozen arithmetic is `[B-6b8]`'s §4.3, as coded in `bundleB.py`: lower median, half-open windows, a run with 0 windows fails, twice-failed runs.

**Outcome branches inside B′ (R5):**
- ①–③ pass → record; `[B-6b10]` is next, with `<C_BP>`.
- Any of ①, ② or ③ length fails → commit the values with the numbers; [개발자] goes in STATUS Top 5. `[B-6b10]` may still proceed on B′'s ④ (recommended; [개발자] may hold it).
- ③ density fails → [개발자] (never values).
- A judgment is 재지 않음, or a cell is withheld → record it with n; [개발자].
- ② slow wholly above 35% → shown with ④'s acceptance.

---

## 5. Mutations

**New patch.** None is required unless §2 changes a test literal. In that case, one new `Tools/mutations/B6b/b6b9_<name>.patch` must break that assertion, with one README line.

**Old patches to rerun.** These are the patches whose patched file this commit edits, plus those whose README filter names a test in a test file §2 edits (speed rule, N §5 `[B-6b3b]`). Find them by scanning every `Tools/mutations/**/*.patch` whose `+++` target is an edited file. At HEAD:

| Edited file | Patches |
|---|---|
| `DT_Rules.json` (if r moved) | `B6b/b6b2a_data_laws_knob_four`, `B6b/b6b2c_data_ark_margin_half` |
| `DT_Hazards.json` (if c moved) | `b6b2b_data_plague_as_extinction`, `b6b2c_data_binding_answers` |
| `DT_Megaprojects.json` (if P_bind moved) | `b6b2c_data_binding_causality` |
| `DT_Feats.json` (if N moved) | `b6b5_data_feat_says_bound`, `b6b5_data_feat_title_eternal` |
| `Tests/CosmicDataTests.cpp` (if a literal changed) | the 15 B2/B4/B5 patches that target it (`b2a_integrity_duration`, `b4b_*`, `b4d2_theodicy_*`, `b5d2_first_crisis_edge_open`), plus any `[B-6b6]`–`[B-6b8b]` additions |

- Add `<MUT_6b6_6b8b>`: the `[B-6b6]`–`[B-6b8b]` patches on the same files.
- **Dropped:** `B4/b4d2_data_resist_factor`. It has not applied since `[B-6b2a]`; `[B-6b8b]` marked it "not retaken; `[B-6b12]` decides".

**How to run them:**
- Assets are rebuilt after apply and after revert only for README's asset list (README:4-8): `b6b2a_data_laws_knob_four`, `b6b2c_data_ark_margin_half`, `b6b2c_data_binding_answers`.
- `b6b2b_data_*`, `b6b2c_data_binding_causality` and `b6b5_data_*` are checker-only: run `check_data` and `test_data_schema.py`, with no asset rebuild.

**Retakes:**
- Run `git apply --check` on each patch first.
- These contexts will break:
  - `b6b2c_data_binding_causality`: its context has `"RequiredProgress": 1800.0`, if P_bind moved.
  - `b6b2c_data_ark_margin_half`: its context includes the r line (`@@ -13`), so it breaks again if r moved, even though `[B-6b7]` retook it.
- Retake by context only, without changing the mutation. Add the README note "문맥은 `[B-6b9]`에 다시 떴다".
- After the last mutation, rebuild the assets clean and confirm `generated.sha1` equals B′'s (§4.3).
- The whole-B6b rerun stays with `[B-6b12]`.

---

## 6. Docs and records

**H §11:** one line under 400 bytes (`wc -c`):

> "- `[B-6b9]`(2026-11-xx): 값만 — 묶음 B <판단 #>(<J…_B>)에 <값 old→new>(규칙 H §5.3, 읽음 노트 §5); B′ 61~120 ① <J1_BP>/60·② <J2_BP>·③ <J3_BP>, C₀ <C0_BP>. 세부 노트 §5."

**Header size (H:406):** "149,000을 넘기는 커밋은 같은 커밋에서 그만큼을 노트로 뗀다"; the hard limit is 150,000.
- Measure with `wc -c` on H after the edit. `check_docs.py` checks only CLAUDE.md's size (`:42-44`).
- If H is over 149,000, move the "더한 것" tails of the `[B-6b2a]`–`[B-6b3b]` §11 lines to N §5, leaving "세부는 노트 §5".
- Never move §5.3, which is frozen.
- `python -I Tools/check_docs.py` still runs clean.

**N §3:** keep "묶음 B" as it is. Add "### 묶음 B′ (`[B-6b9]`)" beneath it, with:
- every number;
- the archive paths;
- `<BP_MINUTES>`;
- `<BP_SHARE_MOVED>`.

**N §5 `[B-6b9]` block:**
- the readings R0–R6 with the sha1 and date from §0.0;
- `--classify`'s counts;
- the probe table;
- the values;
- the moved lines and hashes;
- the mutation list and retakes;
- ruler deaths if c moved;
- P_ark and the ark counter, and the binding cost / U (§1.5);
- the R3 supersession;
- the ⑧ tension (R6).

**`DECISIONS_LOG`:** one line with:
- the date;
- bundle B's failing judgment and numbers;
- the rule applied;
- old → new values;
- B′'s result;
- the ⑧ note.

**Docs to update.** Re-find each by name, with `grep 1,800|1800|0.375|1e-4|1,200|<R_A>|<C_A>` over `docs/10_DESIGN docs/21* docs/40* docs/50* docs/70*`:
- `docs/30`: regenerate with `python -I Tools/make_config_reference.py`, only if r moved (the generator reads only DT_Rules);
- `docs/21`: the r wording;
- `crisis_and_transcendents.md`: binding values and r;
- the extinction c sentence;
- the `docs/50` work table (date only);
- `docs/20` stays unchanged.

**`STATUS.md`:** '다음 3작업' → `[B-6b10]`. Top 5 gains the [개발자] item if B′ failed or an item went to [개발자]. **`docs/CHANGELOG.md`:** the work-log line.

**Commit message:**
```
[B-6b9] 값만 — <값 old→new>(묶음 B <판단>), B′ ① <J1_BP>/60·② <J2_BP>·③ <J3_BP>·C₀ <C0_BP>, 옮은 줄 <list>와 새 해시 <HASHES_B9>, 짝은 [B-6b5] 그대로
```
Then the trailer `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`. Push afterwards.

**Theology:** a value-only commit adds no text. `check_data` still runs every word list.

---

## 7. Order of work and risks

**Order:**
0. §0.1: gates G1/G2 and existence. Return `NO-COMMIT: <reason>` or the hold if no branch opens.
1. Copy readings R0–R6 into N §5 with the §0.0 sha1.
2. Build and self-test the harness copy (§4.1).
3. Run `--classify` over bundle B and record R1/R2.
4. Probes in the §1.6 order, one at a time, with guards. Stop at the cap or at an R4 rejection.
5. Final JSON, `check_data`, `make_data_tables`. Test literal edits (§2). Then `Build.bat` editor target, with the editor closed. Freeze the tree (§4.3 "before starting").
6. B′ to completion. Docs implementers (docs are not compiled) and the review lenses run alongside: data/values, determinism and baselines, docs/records, each finding rebut-verified. No source or data edit is allowed during B′.
7. Mutations (§5).
8. Clean asset rebuild; the witness checks of §4.3.
9. One runner: `-Tier full -Data -UpdateBaseline`.
10. Off-side ⓐ: five runs.
11. Records, commit, push.

Never start a build, `make_data_tables` or a mutation while a probe or B′ is running.

**Risks, each with its recommended resolution (recommended = approved):**

| # | Risk | Resolution |
|---|---|---|
| 1 | The share blocker (`bundleB.py:355-356`) flags every off run | Fix it with `--share`. |
| 2 | B′ overwrites bundle B's archive | New defaults plus the refusal. |
| 3 | Pairs that differ before T_B because of `[B-6b8b]`'s share under the policy's own guard | The §4.1 item 6 reading. A difference without the off twin's awakening sum > 75 is a real control failure → cause first (`[B-6br]` if code), then R5's single rerun. |
| 4 | A lever pushes ② out of the band, or overshoots after an r step | R4 → [개발자] before B′. |
| 5 | A ladder is exhausted, the cap is hit, or only a value that breaks the floor passes | [개발자] (R3). |
| 6 | B′ fails a judgment | Commit and go to [개발자] (R5). Never tune a second time. |
| 7 | B′ is invalid because of build or data mismatch | Identical rerun once (R5, §4.3 witnesses). |
| 8 | Header over 149,000 bytes | Move bytes in the same commit. |
| 9 | The scratchpad was cleared | Logs are in `Saved\Balance\b6b_bundleB`. Skip the reproduction self-test and note it. |
| 10 | AI-bound stalls ((T5-AI-never)) dominate ① | [개발자]. `GalaxyLawsToHold` is not a lever. |
| 11 | [개발자]-only items | density ③; ② slow wholly above 35%; #165 locks (hold); the floor; anything past a ladder or the cap; (T5-AI-never); 재지 않음 or withheld; whether `[B-6b10]` waits after a failed B′; U dominating the season. |

---

## 추가 이야기와 보완

- **The ladders are mine.** The c ceiling of 2.5e-4, the 300 steps for N and P_bind, and the 6-probe cap are not in H, which gives no step. They are frozen in §0.0 before bundle B so nobody can say they were picked after seeing the data. If the developer wants other ladders, this is the place to say so.
- **P_ark never moves in this design.** If B′'s (나) share alone (erased ruled peoples) sits above 35%, that points at c and P_ark, which H routes to nobody. It would be worth one line in OPEN_QUESTIONS at `[B-6b12]`.
- **Committing the harnesses.** Committing `SP/b6b8/bundleB.py` and this copy under `Tools/` would protect B′ and bundle C against a cleared scratchpad. That is outside H §8; if done, it needs a line in `docs/40`.

---

## Critique dispositions

**Critique 1**

1. **Accepted.** The R policy guards under `-Diplomacy=1 -Guard=0` (`CosmicRulerPolicy.cpp:711-780`, verified), and B′'s off side resets the share to 0 (`cmd.cpp:1594-1596`). Added as §4.1 item 6 with `[B-6b8b]`'s rule (i). Input-less follows `[B-6b8b]` K1 (strict unless the twin's sum > 75) rather than "always strict". Risk 3 rewritten.
2. **Accepted.** Added (T5-AI-held) → N and (T5-AI-never) → [개발자], both excluded from R2. Added the binding `CostPerTick` closure in §1.5.
3. **Accepted**, merged with critique 2 item 1 into R4 (either side for any lever change).
4. **Accepted.** N:436-448's table and its fix column are in §2; `[B-6br]` is only for code defects.
5. **Accepted.** Rows added for 재지 않음, withheld (→ [개발자], per SP/b6b8/brief_final.md:494) and the ③ assertion violation (→ cause first).
6. **Accepted.** `b4d2_data_resist_factor` dropped; the `ark_margin_half` retake made explicit; the patches that filter on the edited test file added.
7. **Accepted.** The tie now runs both levers in H's sentence order. The "latest bridge" authority was removed.
8. **Accepted.** G1 → the lead finds the cause; G2 → [개발자] with the hold and its reason; citation corrected to H:474.
9. **Accepted.** R3 states that it supersedes SP/b6b7:52. R2 notes that N has no effect while the binding is last.
10. **Accepted:** r → 1 at the off switch, the `Remove-WrotePath` comparison, the unquoted `1500.0`, H:474. The runner's commands are now named by step instead of line. The "N:51" citation was replaced by N:619 and `cmd.cpp:3976`.
11. **Accepted, with a refinement.** r acts on any non-ruled construction of a rung-3+ row (`CosmicMegaprojects.cpp:20-28`, `CosmicDefinitionRows.h:342`), not only on the binding. §3's table uses "upper construction first started by an AI at".
12. **Accepted.** P_ark recorded as handed over. The ark counter is declined as a code change in a values-only commit (§1.5).
13. **Accepted.** The effort line was added, and `<T_DEATH>` now comes from bundle B's own t.
14. **Accepted.** R6 records the ⑧ tension. N:706 측정-2 does contain "B′ 한 번 그대로" (verified).

**Critique 2**

1. **Accepted** (R4: every judgment is computed; a step is kept only if the passed judgments still pass; either side). Refinement: a step that passes its target but breaks another judgment stops at [개발자] instead of skipping ahead.
2. **Accepted**, except the route for a withheld cell: it goes to [개발자], not `[B-6br]`. `[B-6b8]`'s frozen rule (SP/b6b8/brief_final.md:494) says "report to [개발자] before any judgment".
3. **Accepted, adapted.** The order is r → ①'s lever per R1 → ③'s lever, not "N/P first". "늦은 다리부터" points at whichever bridge R1 shows late. Cap of 6 probes.
4. **Accepted.** The ladders are relative to `<P_BIND_A>`, `<N_A>` and `<C_A>`.
5. **Accepted.** §0.0 adds the sha1 freeze before bundle B, and `--classify` is done by script.
6. **Accepted, adapted.** Literal edits and the build come before B′, the tree is frozen, and the DLL is compared. A mismatch uses R5's identical rerun.
7. **Accepted, adapted.** `generated.sha1` and the JSON sha1s are compared rather than `.uasset` bytes, which may not be byte-stable across rebuilds. Log witnesses were added: "(was r)", `binding at` − `ruled binding started at` (field verified at `cmd.cpp:3968-3969`), and r5 − r4.
8. **Accepted** (R5's single deterministic rerun).
9. **Accepted:** `[B-6b7]` §2.4's rule, the grep widened to `Tools/*.py` and `schema.json`, and the extra patterns.
10. **Accepted:** the filter-test patches, and the split between README's asset-rebuild and checker-only lists (README:4-8, verified).
11. **Accepted:** r → 1 and share → 0; ⓐ called a regression guard; the `Remove-WrotePath` comparison.
12. **Accepted:** serial µs > 16.05; the lock stop labelled R0 with its reason.
13. **Accepted:** the ceiling sentence corrected per N:607; the sum check reduced to a sanity note.
14. **Partly rejected.** "B′ 한 번 그대로" is at N:706 (측정-2, verified), so that citation stays, alongside H:331 and H:343. The N:51 correction is accepted.
15. **Accepted.** Header size is checked with `wc -c`; `check_docs.py` checks only CLAUDE.md (`:42-44`, verified).
16. **Accepted.** The final probe uses the exact candidate data with r in the JSON (§1.6).