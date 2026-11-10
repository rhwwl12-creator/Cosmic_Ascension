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
