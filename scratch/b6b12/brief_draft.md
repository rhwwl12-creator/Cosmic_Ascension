# [B-6b12] implementation brief: docs (H §7), the re-audit, and the whole-B6b mutation rerun

Verified at HEAD `10205b6` [B-6b5], read only through `git show HEAD:`. The working tree was not opened.

- H = `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` (147,498 bytes, 479 lines).
- N = its `_notes.md` (336,642 bytes, 791 lines).
- Line numbers are HEAD lines. [B-6b6]..[B-6b11] land first and move them, so re-find every place by the first words of its paragraph (the N:385 rule).
- H §8 (H:400) sets this commit at **xhigh**: runner full (bit-identical) and the re-audit workflow.
- The speed rule (N:768) applies:
  - one `-Tier full` run per commit; its two in-process passes are the "two consecutive runs";
  - the whole-B6b mutation rerun happens here, once.
- Recommended options count as approved. Items marked **[개발자]** are for the developer only.

---

## 0. Start state (the lead checks this before anything runs)

### 0.1 The tree

1. Run `git log --oneline -14`. Expect `[B-6b6]`, `[B-6b7]`, `[B-6b8]`, `[B-6b8b]`, `[B-6b9]` (only if a judgment called for it), `[B-6b10]`, `[B-6b11]` and any `[B-6br]` on top of `10205b6`. HEAD should be the last of these. Write the list down; the CHANGELOG and DECISIONS_LOG lines are built from it.
2. Check the tree is clean.
   - `git status --porcelain` is empty, or shows only EOL phantoms: `git diff --ignore-cr-at-eol --stat` is empty.
   - The main tree is LF (`git ls-files --eol` shows `i/lf w/lf` at HEAD).
3. Check that nothing is running. `tasklist /FI "IMAGENAME eq UnrealEditor.exe"` and `UnrealEditor-Cmd.exe` show nothing.
   - An open editor's Live Coding mutex blocks every UBT build on this engine (`HotReload.cs`).
   - Other workflows' commandlets must also be finished.
4. Check free memory (`Get-CimInstance Win32_OperatingSystem` → FreePhysicalMemory). Three lanes need ≥ 8 GB free; with less, use two (§5.3).
5. Check that the scratch harnesses the probes need still exist:
   - `${SCR}/b6b8/bundleB.py` (the `b6b8_cmdlet_witness_first_any_card` smoke);
   - `${SCR}/b6b6/i1_pymut.py`;
   - `${SCR}/b6b6/apply_state.py`;
   - `${SCR}/b6b6/run_final_mut.py`.
   - `${SCR}` = `C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad`.
6. Leave the other worktree `${SCR}/b7x/wt` (branch `design/b7x-composable-existence`) alone.

### 0.2 What the earlier commits must have left

Check each item. If one is missing, do what the last column says.

| Item | Where | If missing |
|---|---|---|
| N §3 "묶음 A", "묶음 B" (and B′), "묶음 C" filled | N §3 (N:679-691 at HEAD) | Stop. [B-6b12] fills placeholders from these records and never measures (§4) |
| One H §11 line and one N §5 block per commit [B-6b6]..[B-6b11] | H §11 (H:461-470), N §5 | Ask the lead of that commit. Do not write another commit's line |
| `DECISIONS_LOG` lines `[B-6b6]`..`[B-6b11]`. `[B-6b10]` copied the clock rule verbatim (H:347). `[B-6b11]` wrote the #142 rule (decision 24, `DECISIONS_LOG.md:7`) | B6b block of `DECISIONS_LOG.md` | Write the missing copy in this commit, verbatim from H §5.3 / decision 24, and say so in N §5 |
| #132 row deleted, and its B7 drain line placed (b6b8b brief :285) | `OPEN_QUESTIONS.md:101`; `docs/50:12` B7 part | Do it in G-questions / G-plan (§1.3) |
| `b4d2_data_resist_factor` row marked "(no longer applies since [B-6b2a] — not retaken; [B-6b12]'s whole-B6b rerun decides)" (b6b8b brief :259) | `Tools/mutations/README.md` (:398 at HEAD) | Phase A step A3 (§1.1) acts on it either way |
| The `b6b7_probe_*` rows (typed "commandlet probe") and their seeds and ticks (`PROBE_SEED_1/2`) | README; N §5 `[B-6b7]`; `docs/40:502` | Not probed at [B-6b7] ("not probed — no seed"): carry that record forward and do not run the probe |
| The `b6b8_cmdlet_witness_first_any_card` row and its command | README; N §5 `[B-6b8]` | As above |
| Tests: count `<TESTS_N>`, and one `docs/40` table row each for `Sim.EraEndured` and `Sim.WitnessCard` | `git grep -h 'IMPLEMENT_[A-Z_]*AUTOMATION_TEST' -- Source` (223 at HEAD); `docs/40` test table | G-dev adds the missing rows |
| Hand-check seeds and ticks for H §9 ② (`-SaveAt` command lines) | N §5 `[B-6b11]` | Record "not chosen" in STATUS. The saves are the developer's to make with this commit's build (§7.4, risk 12) |
| Which [B-6b5] hand-offs were already done. ㉗ (the hand read of '방주' next to '절멸') and ㉘ (the epitaph title '업적 — 방주와 조약') belong to [B-6b6] | N §5 `[B-6b6]` | Not done: add them to hand check ③ in STATUS. No code |
| Whether `docs/15` rows were already added ('기계 살핌' etc. — b6b7 brief :605 left them to [B-6b12]) | `docs/15_GLOSSARY.md` | G-words adds them (§1.3) |
| Whether H is still under 149,000 bytes after the [B-6b6]..[B-6b11] lines | `wc -c` H | A commit that crossed it should have moved text to N. If not, [B-6b12] moves it (§6.1) |

### 0.3 Counts at HEAD 10205b6 (re-measure at [B-6b11]; numbers below are the HEAD values)

- **B6b patches:** 283 `Tools/mutations/B6b/*.patch` files and 283 `| \`B6b/\` |` README rows, one to one.
  - By commit: b6b1 7, b6b2a 31, b6b2b 30, b6b2c 40, b6b3a 33, b6b3b 40, b6b4 63, b6b5 39.
  - Every patch touches one file. The most-hit targets are `Tools/data/schema.json` 40, `Sim/GalaxySimulationSubsystem.cpp` 39, `Tests/CosmicRaceMeasure.h` 27, `UI/CosmicSystemPanelWidget.cpp` 20, `UI/CosmicNextRung.cpp` 14, `UI/CosmicEdictWidget.cpp` 13, `World/CosmicRulerPolicy.cpp` 12, `Sim/CosmicHazards.cpp` 10 and `Tests/CosmicTestHelpers.h` 9. There are 4 on `UI/CosmicChronicleWidget.cpp` and 8 on `DT_LogTemplates.json`.
  - Kinds by README row (mapper count, re-take it with the planner in §5.2): 222 C++, 6 asset-rebuild data, 55 Python-only (43 `_schema_` + 12 `_data_`).
- **Other blocks:** B1 71, B2 89, B3 151, B4 159, B5 224, B6a 56.
- **Tests:** 223 automation tests; checker unit tests 94 (`[B-6b5]`, N:786).
- **Sizes:** `CLAUDE.md` 16,348 bytes, LF in the working tree, against the 16,384 that `Tools/check_docs.py:42-43` prints a NOTE over. H is 147,498 bytes.
- **Stale markers to fix:** 21 lines name `[B-6b12]` (or `[B-6b6]`..`[B-6b11]` with '가 연다/고친다/닫는다/옮긴다') at HEAD:
  - 11 in living docs, all fixed here: crisis :24/:40/:115/:117/:133/:175, `pillars.md:108`, `systems.md:163`, `docs/20:134`, `docs/40:510`, `docs/50:231`;
  - 8 in DECISIONS_LOG, which are dated records — only :5 and :7 change;
  - 2 in STATUS, which is rewritten.
  - Probe 8 re-finds them at [B-6b11].
- **Provenance markers to keep:** the 48 '`[B-6bN]`부터' markers.
  - They stay, like the 8 '`[B-6aN]`부터' lines (27 with B5's) that `5455e56` kept after `[B-6a7]` (for example `docs/20:89` "B6a `[B-6a3a]`부터").
  - Fix one only if it names a commit that never happened (`[B-6b9]` when no B′ ran) or a value that changed.

### 0.4 Placeholders: never invent; each is filled by its rule from the named record

The whole text of each pre-written rule is in §4. Every receiver is a HEAD line; re-find each one by its first words.

| Name | Filled from (record at [B-6b11]) | Rule | Receivers |
|---|---|---|---|
| `<J1>` | N §3 묶음 B (B′ if [B-6b9] ran) | H §5.3 ① | `docs/50:12` B6b part; DECISIONS_LOG open-questions line (#161); CHANGELOG; STATUS:7 |
| `<J2>`, `<J2_가>`, `<J2_나>`, `<J2_다>`, `<J2_ONCE>`, `<J2_SLOW>` with Wilson intervals | same | H §5.3 ② and the reading in N §5 `[B-6b8]` ⑧ | #161 closure; `docs/50:12`; STATUS |
| `<J3_LEN>`, `<J3_DENSITY>` | same | H §5.3 ③ | `docs/50:12`; STATUS |
| `<C>`, `<C0>`, `<E48>`, `<ACCEPT_i>`, `<ACCEPT_ii>` | N §3 묶음 B/B′ and the `[B-6b10]` DECISIONS_LOG line | H §5.3 ④, the clock rule and H:351 | #2 (`OPEN_QUESTIONS.md:12`); `docs/00_VISION.md:18`, `:103`; `crisis_and_transcendents.md:140`, `:154`, `:207`; `docs/15:109`, `:111`; `docs/40:470`. If ⓔ failed, there is no C: #2 stays open with the numbers and the [개발자] answer |
| `<WALL_A>~<WALL_B>` | hand check ⑤'s `CosmicEnd` line in N §5 (developer) | decision 42 (DECISIONS_LOG.md:31) | `docs/00_VISION.md:32`; #2 closure. If ⑤ is not done: no minutes; write "손 확인 ⑤ 뒤" and list it in STATUS |
| `<B136>` | N §3 묶음 C | N §2.3 #136 | `OPEN_QUESTIONS.md:103` |
| `<B142_N>`, `<B142_VERDICT>` | N §3 묶음 C, test cell | N §2.3 #142 and decision 24 | `OPEN_QUESTIONS.md:108` |
| `<B132_*>` | N §3 묶음 C | N §2.3 #132 (record only) | the B7 list in `docs/50:12` |
| `<B1_L>`, `<B1_G>`, `<B1_p>`, `<B1_PATHS>`, `<B1_UNEXPLAINED>` | N §3 묶음 C | N §2.3 B1 guard | DECISIONS_LOG open-questions line; `docs/50:12` |
| `<B129_*>` | N §3 묶음 C | N §2.3 #129 | `OPEN_QUESTIONS.md:98`; `systems.md:80` |
| `<B154_*>` | N §3 묶음 C | N §2.3 #154 | `OPEN_QUESTIONS.md:118` |
| `<LEAP_OO_INPUTLESS>`, `<LEAP_OO_WANDERER>`, `<LEAP_OO_DEATH>` | N §3 묶음 C | N §2.3 '< 15%' | `docs/40:514` |
| `<END_KINDS_R>`, `<END_KINDS_SLOW>` | N §3 묶음 C | N §2.3 '시계 아래의 끝' | `docs/50:12`; STATUS |
| `<B_LOCKED>` | N §3 묶음 B | N §2.3 적기만, the lock rule | the new #165 row's 걸림 |
| `<B_AI_US>` | N §3 묶음 B | N §2.3 적기만, +50% rule | `docs/50:12`; STATUS |
| `<B96_T4_LEFT>` | N §3 묶음 B (b6b8 brief :556) | H 추가 이야기 (H:475) | `OPEN_QUESTIONS.md:70` (#96) |
| `<SHORT_ARC>` | N §0.2 at [B-6b11]: did [B-6b8] open the candidate (b6b8 brief :558, H:476)? | N §0.2 (N:218) | new #167, only if opened and still open after #2 |
| `<B10_SHARE>` | N §3 묶음 C, only if [B-6b11] measured it | none pre-written (N §2.3 has no #10 entry) | `OPEN_QUESTIONS.md:20`. Otherwise write '재지 않음 — 미리 적은 목록 밖' and label the 6,000 sentence as B5's |
| `<B158_*>` | same | #158 ③'s own rule (`OPEN_QUESTIONS.md:122`) | `OPEN_QUESTIONS.md:122`, with the same fallback |
| `<C_EXT>`, `<ARK_COST>`, `<P_BIND_FINAL>`, `<N_FINAL>`, `<R_FINAL>`, `<M_AWAKE>` | `Tools/data/*.json` at HEAD = [B-6b11], read by script, never typed | values | `docs/50` §12.1; `docs/15`; `pillars.md:108`; DECISIONS_LOG |
| `<MUT_*>` | §5 merge file | counts | `docs/40:40`; DECISIONS_LOG; CHANGELOG; H §11; N §5; commit message |
| `<REAUDIT_*>` | the workflow's record.md (§7.2) | counts | same |
| `<BASE_*>` | `Tools/baselines/seed42.json` at [B-6b11] | the five lines and the two hash runs | STATUS:27; DECISIONS_LOG; commit message |
| `<TESTS_N>`, `<DATA_ROWS>` | `git grep` / JSON row counts at [B-6b11] | counts | STATUS:26; `docs/40` |
| `<DATE>` | commit date | — | everywhere |

Before any agent runs, put every filled value in `${SCR}/b6b12/values.json`, with its source line. Agents read that file and never re-derive a number.

---

## 1. The changes

### 1.1 Phase A: code, data text and mutation housekeeping (lead or one implementer, before the lanes)

Phase A is the only part of the commit that touches `Source/`, `Tools/` or `Config/`. Once it is done, those three folders are **frozen** for the docs workflow. The one exception is `Tools/mutations/README.md`, which only the lead edits, in §5.7.

**A1. The chronicle chapter header's rung (decision 30; [B-6b4] ⑱ at N:776, [B-6b5] ㉞ at N:790).**

`UI/CosmicChronicleWidget.cpp:290` today shows "칸 1" for every life whose peak is the first rung. The epitaph omits the rung in that case (`UI/CosmicCallingPagesWidget.cpp:407-410`). The chronicle header should follow the same rule.

Replace HEAD :290 with:
```cpp
		// The rung only above the first, as the epitaph's (B6b decision 30): PeakTier is the life's peak - the rung at
		// birth, raised when the people the vessel rules climbs above it (FLifeRecord::PeakTier) - so it reads as the people's.
		const FText Rung = Life.PeakTier == EAscensionTier::Tier1_Planetary ? FText::GetEmpty()
			: FText::Format(INVTEXT("{0} {1}"), Label(TEXT("Panel.Tier")), FText::AsNumber(static_cast<int32>(Life.PeakTier) + 1));
```
- `CosmicUI::JoinList` skips empty texts (`UI/CosmicUILabels.h:207-219`), so `JoinList({ People, Rung, Lasted })` at :292 stays as it is.
- This is presentation only: no Core Logic file, no saved field, no draw. It is a `.cpp`-only change, so no header and no editor-closed full build.

Change the tests in `Tests/CosmicChronicleScreenTests.cpp` (all in `UI.Chronicle`):
- :674-676: the expected facts mirror the rule. `Rung` is empty when `Life.PeakTier == Tier1_Planetary`.
- :1089-1093 (the two lives, both at peak 1): drop `RungOne` from `FirstFacts` and `CalledFacts`. Message: `"A life whose peak is the first rung shows no rung (B6b decision 30)"`.
- :864-872 (the block that sets `Mutable.PeakTier = Tier3_Causal` and reopens the page): add an assertion that the life chapter's facts row is `JoinList({ <people>, "Panel.Tier 3", <lasted> })`. Message: `"A life whose people reached the third rung shows it"`. Read the row through the existing page-text accessor (`GetPageTexts`).

New mutations, kept as `Tools/mutations/B6b/b6b12_*.patch` with one README row each:
- `b6b12_chronicle_rung_at_first`: the condition is dropped, so 'rung 1' shows. Breaks `Cosmic_Ascension.UI.Chronicle` (the :1089 case).
- `b6b12_chronicle_rung_never`: `Rung` is always empty. Breaks `Cosmic_Ascension.UI.Chronicle` (the new :864 case).

**A2. One meaning for the binding verb ([B-6b5] ⑭ at N:790; N §1.6 rule 4 and the `binding_words` note at N:542).**

At HEAD the stem '묶' appears in four places: the two story lines (`log_fallen_bound` at `DT_LogTemplates.json:422-424` and `Sight.Bound` at `ST_CA.csv:52`) and two B3 variants. Reword the two B3 variants so that '묶' is the binding's verb alone. Keep the slots, the variant count and the order.
- `Tools/data/DT_LogTemplates.json:28` (`log_breakthrough_society`[0]):
  - from: `"{Civ}의 사람들이 오랜 다툼 하나를 약속으로 묶었다. 서로를 조금 덜 두려워하게 되었다."`
  - to: `"{Civ}의 사람들이 오랜 다툼 하나를 약속으로 매듭지었다. 서로를 조금 덜 두려워하게 되었다."`
- `:236` (`log_treaty_broken`[1]):
  - from: `"{Civ}|hpp(과,와) {OtherCiv} 사이의 {Treaty}|hpp(이,가) 끝났다. 서로를 묶던 약속이 풀렸다."`
  - to: `"{Civ}|hpp(과,와) {OtherCiv} 사이의 {Treaty}|hpp(이,가) 끝났다. 서로를 잇던 약속이 풀렸다."`
- The named row keeps `{OtherCiv}`. `test_the_nameless_lines` (`Tools/test_data_schema.py:3318-3330`) asserts this.
- No test pins these strings. `git grep '약속으로\|묶던'` finds only the data.
- Run `python -I Tools/check_data.py` and `python -I Tools/test_data_schema.py`, then `make_data_tables.py` with the editor closed. This makes the runner `-Data` (§3).
- The W2 lens (§7.2) reads the two variants.
- No mutation patch has a hunk within three lines of :28 or :236 (checked at HEAD).

**A3. `b4d2_data_resist_factor` (b6b8b brief :259; the docs/40:40 rule for an older block's patch).**

That patch is the only one that broke `Sim.PressureChain` on the data value `"ResistFleetFactor": 0.5`:
- `git grep -l ResistFleetFactor -- Tools/mutations` gives `b4b_schema_resist_min_off` (the schema's min), `b4c_resist_line_raw_factor` (a code line) and `b6b2a_data_laws_knob_four` (context only).
- So unless [B-6b6]..[B-6b11] already retook it, take the same mutation again as `Tools/mutations/B6b/b6b12_data_resist_factor.patch`.
  - Make it on the final tree: in a scratch copy, set 0.5 → 0.6 at the line, keep `git diff`'s context, and use LF.
  - It is an asset patch: add it to README:5's list.
  - It breaks `Cosmic_Ascension.Sim.PressureChain`.
  - Change the old B4 row's mark to "(no longer applies since [B-6b2a] - taken again at [B-6b12] as `b6b12_data_resist_factor`)". That is the form of the `b5c_panel_no_warning` row at README:483.
- If the lane shows it SURVIVED, `Sim.PressureChain` no longer reads the data value. Delete the new patch, and mark the old row "(no longer applies since [B-6b2a] - not retaken: `Sim.PressureChain` no longer reads the game value, [B-6b12])". Record the finding in N §5.

**Making the patches.** Write each patch from the Phase A bytes of its file as a list of exact replacements, using difflib with `a/<path>`/`b/<path>` headers. `${SCR}/make_patches.py` and `${SCR}/make_mutations.py` were used this way at [B-6b2a] and [B-6b2b]. Do not use `git diff` in the main tree: it would carry Phase A too. Check each patch with `git apply --check` on the main tree.

**A4. README.**
- Add rows for the two `b6b12_chronicle_*` patches and (A3) `b6b12_data_resist_factor`.
- Add `b6b12_data_resist_factor` to the data list at README:5.
- Nothing else in the README changes before the merge (§5.7).

**A5. Main-tree check before the lanes start** (about 10 min; this is not the commit's runner):
1. Build the editor target (`Build.bat Cosmic_AscensionEditor Win64 Development -Project=… -WaitMutex`).
2. Run `UnrealEditor-Cmd … -game -nullrhi … "-ExecCmds=Automation RunTests Cosmic_Ascension.UI.Chronicle+Cosmic_Ascension.Data; Quit"`. All must pass on the clean code.
3. Write `${SCR}/b6b12/phaseA.diff` = `git diff HEAD -- Source Tools Config ':!Tools/mutations/README.md'` (text only: `.uasset` files are LFS and are rebuilt in each lane).
4. Write `${SCR}/b6b12/apply_before.txt` with `apply_state.py <root> HEAD …` and `apply_mid.txt` on the tree after Phase A, in a scratch copy with Phase A applied.
   - Expected: no older-block patch goes from OK to FAIL. Hunk check at HEAD: none within three lines of `CosmicChronicleWidget.cpp:290` or `DT_LogTemplates.json:28`/`:236`.

### 1.2 What [B-6b12] does not change in code (decided; record each in N §5)

- **Pointer comments to closed rows stay.**
  - Cases: `Tests/CosmicTestHelpers.h:326` "(#1, #141)"; `Tools/data/schema.json:1037` and its key in `Tools/test_data_schema.py:3663` "(guideline 16 #1, #141)"; the commandlet's `#132`/`#162`/`#156` labels.
  - These are provenance. A closed row still resolves through its DECISIONS_LOG closure, which names #164.
  - Cost of editing them: changing :326 breaks `B6a/b6a6_exalt_as_before_is_the_game.patch`, whose hunk `@@ -322,6 +322,7 @@` has :326 as its first context line. The schema text needs `-Data`, and the commandlet labels are measurement output.
  - Rule for the workflow: a `Source/`, `Tools/` or `Config/` comment changes in [B-6b12] only if it states something false about the code at [B-6b11] (route `comment`, §7.3).
- **`Diplomacy.SlotsAll` with the end switch off** ([B-6b5] ㉓): no code.
  - The switch is `ACosmicPlayerController::IsProvisionalEndEnabled()` (`World/CosmicGameMode.h:359`). Only `-ProvisionalEnd=0` turns it off, for a session or for tests.
  - In the game it is always on, so the fifth rung's slot never comes and the tooltip is true (`UI/CosmicEdictWidget.cpp:1233-1245`).
  - G-arch writes one clause at the slot tooltip in `docs/20`: "끝 스위치를 끈 개발용 판(`-ProvisionalEnd=0`)에서는 다섯째 칸의 자리가 오지만 툴팁은 게임의 규칙을 말한다".
- **The old feats' narrative overlap** ([B-6b5] ㉕): no change.
  - §1.6 rule 1 forbids the same sentence or frame only. The overlap is the same event in other words (L6c-2).
  - W2 re-reads it. Only a same-frame repeat becomes a `data-text` finding.
- **`silence_words` widened to the whole log column** ([B-6b5] ㉚; b6b7 brief :680): not here. One line goes in `docs/50`'s B7 list (G-plan).
- **The `has_cond` schema operator** (b6b7 brief :679): goes in 추가 이야기 only.
- **Committing `bundleA.py`/`bundleB.py`**: no. Run scripts stay outside the repo (docs/40:40 "회차 스크립트(저장소 밖 — 남는 것은 패치다)").

### 1.3 Docs, by owner group (the workflow's editors; one editor per file — WORKPLAN.md:98)

**Rules for every editor:**
- Read code and data with `git show HEAD:` (HEAD = [B-6b11]).
- Read docs from the working tree.
- Take numbers only from `values.json`.
- No new verse anywhere (H:405; the B5 header :450 rule). No agent reads anything under `Downloads/jehoba`.
- Plain words: name work by what it does, and use no coined shorthand.
- A closed row that is still mentioned is written as '옛 §16 #N — B6b 결정 K가 닫았다', the form at `crisis_and_transcendents.md:101`/`:150`.
- '`[B-6bN]`부터' markers stay as provenance (§0.3).

**G-questions: `docs/60_DECISIONS/OPEN_QUESTIONS.md`.** Row actions follow N §0.2 (N:179-219). DECISIONS_LOG.md:7 assigns the row edits to this commit.

Rows to delete (each closure goes into the DECISIONS_LOG open-questions line, G-records):
1. **#1 (:11).** First move its "개인 칸을 올리는 블록이 같은 커밋에서 할 일 셋" into #164 as ①, ② and ③, keeping each task.
   - Do not move ②'s trap sentence ("그 커밋에서 `Sim.ExaltArrives`의 옛 행 쪽이 먼저 깨지고 …"). N:311 row 8 says it goes with #1.
2. **#2 (:12)**, only if ④ was accepted (C exists).
3. **#107 (:80), #121 (:93) and #122 (:94).**
4. **#136 (:103)**, if `<B136>` ≥ 20/60. Otherwise keep it with the number, and re-point its 걸림 to the B7 header (the lead's call — recommended: B7, since the first-person claim lines live in the B7 card and edict block).
5. **#138 (:105).** Its W-column escort chain goes to #164.
6. **#141 (:107).** First split it:
   - 대면·정신의 닻·피난 호위, B5 결정 9의 다시 물음, and B6a ① and ③ go to #164;
   - B6a ② (the B1 guard) is answered by `<B1_*>` in DECISIONS_LOG;
   - the play-5 crowning share is already in #158.
7. **#156 (:120), #160 (:124), #161 (:125) and #162 (:126).**
8. **#132**: [B-6b8b] deleted it; verify only.

Rows to edit:

| Row (HEAD line) | Edit |
|---|---|
| #10 (:20) | "6,000틱 판 … 넷에 하나 … 공백기 안에서 끝난다" → `<B10_SHARE>` or the fallback (§0.4) |
| #60 (:46) | Add "B6b(`<DATE>`, A3): ②는 방랑자의 칸이 오르지 않아 일어나지 않는다; 걸림 그대로" |
| #67 (:48) | In the 결정 사항 cell, "모든 생애의 `PeakTier`는 #1의 블록(B5 결정 9)" → "#1은 B6b가 닫았다(독립 — 칸 공명 없음, A3)" |
| #90 (:65) | Add "절멸(행·만료 훅·방주)은 B6b에 들었다(결정 3); 아공간 침식은 이 행 그대로" |
| #96 (:70) | Add the fourth-rung case with `<B96_T4_LEFT>`: a first-rung wanderer who left a people at T4+ meets `FarAbove` on crowning and on return (decision 41) |
| #98 (:72) | Drop ② (closed by decision 32: 회색·끊기지 않음·Severity 0·금색 없음). Remove '봉인 업적' from the title. Keep ①③④ |
| #100 (:73) | "이 대안은 #1과 함께 묻는다" → "#1은 B6b가 닫았다(공명 없음) — 대안은 #96과 함께 [개발자]" |
| #104 (:77) | Verify only ([B-6b4] fixed it) |
| #129 (:98) | Add "B6b(묶음 C): `<B129_*>`". In the title, "(B6b·B7 헤더 — …)" → B7 |
| #137 (:104) | Add "'조공 없이'는 #156(B6b가 닫았다 — 결정 22)의 빈 조건이다" |
| #142 (:108) | Rewrite to decision 24's rule and `<B142_*>`. Use '재지 않음 — 잰 N' when N < 30. At ≥ 30 and outside the model band, [개발자]. If it stays open, the title points to B7 |
| #146 (:111) | Add "B6b: 게이트의 `bSealed == false`(`crisis_and_transcendents.md:221`)가 처음으로 거짓이 될 수 있다 — 결박이 판 안에서 봉인을 쓴다(VS의 교리는 침묵 하나)" |
| #154 (:118) | Add `<B154_*>`. Drop "또는 B6b" so the 걸림 is the #152 block alone |
| #158 (:122) | Remove "옛 #135 몫 ③의 다시 잼은 B6b 헤더 — …" from the title, and add `<B158_*>` or the fallback. 걸림 stays B7 |
| #163 (:127) | Split: "봉인의 글은 B6b가 정했다(결정 32)". Remove '봉인' from "(T5 문턱·두 길·후원 카드·봉인이 오는 블록 …)". The rest stays Phase 6+ |

**New rows after :127.** Columns are `# | 결정 사항 | 권장안 | 영향`; the text comes from N:214-216.
- **#164** '방랑자의 대면·정신의 닻·피난 호위' (방랑자의 칸이 오르는 블록 — 방랑자 업적, Phase 4; #141·#138에서 나뉨, B6b 결정 20·21; `[B-6b12]`가 열었다).
  - 권장안: N:214's 기본값과 근거, plus the three tasks from #1 and the #141 parts above.
  - Add 'T4 봉인 단독 해제' (crisis :57, N 어긋남 18: record only).
  - Add the §4.1 end 'Tier 4에서는 직접 대면 조우가 열린다' (어긋남 31).
  - 영향: §3.2, §4.1, §4.2, §5.9.
- **#165** '결박한 성계를 잃은 백성과 결박한 이의 필드'.
  - 걸림: if `<B_LOCKED>` ≥ 3/60, [개발자], together with decisions 12 ㉯, 13 and 14 (H:474). Otherwise the Phase 6+ seal bundle.
  - 권장안: N:215.
- **#166** '언약 공동체의 \'다시는\' — 같은 위기의 재발 면역' (Phase 4 특성 행의 블록). 권장안: N:216.
- **#167** '판의 길이와 비전의 호': only on `<SHORT_ARC>` and if it is still open after #2 (N:218). 걸림: Phase 5 balance. Give C and the vision's §2.4 rows (`docs/00_VISION.md:171` '초월 ~90분').
- **No new row for `silence_words`.** Its line goes in `docs/50`'s B7 list (G-plan).

**G-records: `docs/60_DECISIONS/DECISIONS_LOG.md` and `docs/CHANGELOG.md`.**
- DL:5: "구현 `[B-6b1]`부터)" → "구현 `[B-6b1]`~`[B-6b12]`)". Make the date a range "2026-11-08~`<DATE>`".
- DL:7: past tense ("이 절은 커밋마다 자랐다 … `[B-6b12]`가 모았다"), and point to the two new lines below.
- Check that the lines `[B-6b6]`..`[B-6b11]` (and `[B-6b9]` and `[B-6br]` if they exist) are present (§0.2).
- New line `[B-6b12]`, after the last B6b line, in the form of DL:54 `[B-6a7]`:
  > `- \`[B-6b12]\`(<DATE> — 헤더 §7의 문서 커밋과 노트 §1.2; 동작 그대로): 문서를 B6b의 코드에 맞추고 결정에 매인 문장(결정 1·9·12·19·20·21·22·23·26·29·30·32·35·38·40·41·42)을 고쳤다; \`CLAUDE.md\` 규칙 15 'A0~B6a' → 'A0~B6b'(결정 38); VS 정의의 수량(\`docs/50\` §12.1 — 결정 40); 벽시계의 잣대(결정 42 — <WALL or '손 확인 ⑤ 뒤'>). 코드는 연대기 장 머리의 칸 하나(결정 30 — 첫째 칸이면 칸을 내지 않는다)와 B3 줄 둘의 '묶'(결박의 동사는 하나 — 노트 §1.6 규칙 4)뿐이고 다섯 줄과 해시 판 둘이 비트 그대로(<BASE_*>). B6b 변이 전체를 다시 돌렸다 — <MUT_*>. 고친 자리와 재감사의 수는 \`docs/CHANGELOG.md\`의 B6b 줄.`
- New line '열린 물음(B6b 전체 — 행은 `[B-6b12]`가 고쳤다; 할 일의 표는 노트 §0.2, 판단의 수는 노트 §3)', in the form of DL:55:
  - **닫은 것:** #1 (A3, 결정 15), #2 (C=`<C>` — 결정 35의 규칙: C₀ `<C0>`, (i) `<ACCEPT_i>`, (ii) `<ACCEPT_ii>`; 결정 42), #107 (결정 19 — `[B-6b3a]`), #121 (결정 19·35 — 지금대로), #122 (결정 41), #132 (`[B-6b8b]`), #136 (if closed), #138 (결정 21), #141 (결정 20 — 나눔), #156 (결정 22 ㉮), #160 (결정 9·40·A2), #161 (결정 16·17·35 — ① `<J1>`, ② `<J2>` …), #162 (결정 29), #98 ② (결정 32).
  - **나눈 것:** #141 → #164; #163 → 봉인의 글; #138's W → #164.
  - **고친 것:** #10, #60, #67, #90, #96, #100, #129, #137, #142, #146, #154, #158.
  - **새로 연 것:** #164, #165, #166 (and #167).
  - **B1 지킴:** `<B1_*>` (N §2.3 — this replaces #141's B6a hand-off ②).
- New line '남은 것', in the form of DL:56:
  - the developer's hand checks — B1~B6a (DL:56) and H §9 ①~⑥ — with B6a ①·③·⑨ on the B6a build (H:410);
  - the saves (`Saved\HandCheck\B6b_*`, made by the developer with this commit's build — §7.4 risk 12);
  - the verse cross-check (§7.5);
  - the next block, the B7 header, with the "B6b가 넘긴 것" list in `docs/50:12`.
- **CHANGELOG:** insert after :4 "## 2026-11-08~`<DATE>` — B6b 넷째·다섯째 칸과 경주". At HEAD there is no B6b line; [B-6b6], [B-6b7] and [B-6b8b] defer to here, and [B-6b8] may have added a work-log line, which moves into this section. Contents:
  - the header approval line (적대 리뷰 발견 57 — 확인 41·일부 16·반박 0; DL:8, N §4);
  - one bullet per commit, `[B-6b1]`…`[B-6b12]` (and `[B-6b8b]`, `[B-6b9]`, `[B-6br]` if any), each condensed from its DECISIONS_LOG line and N §5 block: what, review counts, mutation counts, baselines;
  - the last bullet ends with "재감사: (리드가 채운다)" (the B6a7 practice).

**G-plan: `docs/50_ROADMAP.md`, `WORKPLAN.md` and `CLAUDE.md`.**
- **`docs/50:12`** (one 30,618-byte line), the B6b part from "→ B6b 넷째·다섯째 칸의 업적 행과 AI(B6a 다음…" up to "→ B7":
  - Rewrite as "B6b 넷째·다섯째 칸과 경주(끝, `<DATE>`: 헤더 `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md`, 커밋은 `git log --grep 'B-6b'`)".
  - Then say what went in (the list in the next bullet, condensed); then `<J1>`, `<J2…>`, `<J3…>`, C, `<END_KINDS_*>` and `<B_AI_US>`; then the hand checks pending (H §9).
  - Add a "B6b가 넘긴 것" group to the B7 part:
    - 프로필 외교 (AI끼리·AI가 제안하고 끊는 조약), 조약 깨기, AI 칙령 일반;
    - 영향권 안 소모 (#132의 둘째 몫 — `<B132_*>`; keep [B-6b8b]'s line if it placed one);
    - #129, #155, #158, and #136/#142 if open;
    - `silence_words`의 자리 — 로그 열 전체와 `edict_sabbath`의 범위 (b6b7 brief :680; [B-6b5] ㉚).
  - In the end-condition column: "…§12.1의 수량 … B6b 헤더가 … 고친다" → "고쳤다(B6b 결정 40)". Add "B6b 뒤: ±30% 지표는 재지 않았다 — 미리 적은 읽기 밖" unless 묶음 C recorded them.
- **`docs/50:15-26` '현재 코드와 예정'**: add, after :26, one "- B6b: …" line covering:
  - saved counters `LawsHeldTicks`·`PressureEnduredTicks`;
  - `ECosmicEdictScope`/`FEdictDef.Scope` and the galaxy-edict hooks; the AI's two steps;
  - the extinction expiry hook `CosmicHazards::LetPeopleGo`·`KillsTheRulerOf`;
  - `CosmicSeal` (`BindTargetOf`·`Bind`), `NoFallen`, `project_binding`;
  - `CosmicAscent::FourthRungOpenOf`; `AIUpperRungProgressMultiplier`; `CosmicConflict::GuardDelayCapOf` with `FleetDelayCountdownShare` 0.5;
  - `Core/CosmicKnowing`, crowning's `FarAbove`, the six `…_far` lines, `CosmicChronicle::PeoplesOfEveryLife`;
  - `race:` with `-WithoutB6bData`/`-FourthRungCap`;
  - `CosmicAscensionOverlay::LookOf`; the new word lists;
  - the data rows (extinction, ark, two feats, three laws, era, `enc_witness`);
  - the clock `<C>`.
- **:46 (risk row)**: add B6b's rebaselines. [B-6b1] moved the hash only; then add whichever of [B-6b6], [B-6b7], [B-6b8], [B-6b8b], [B-6b9] and [B-6b10] moved lines, taken from seed42.json notes. Add the split pattern: `[B-6b2a]`~`[B-6b5]` bit-identical, `[B-6b6]`~`[B-6b8]` data.
- **Phase 4 lines:**
  - :197: "차원 방주 따위" → the ark came with B6b.
  - :198: "남은 Hazard 3종" → 2 (SubspaceErosion·Cult). Extinction's row, hook and ark are B6b. The '피난 호위 조우 체인' stays in this item (#164). `enc_witness` is B6b `[B-6b8]`.
  - :208: "…폐위를 이어 적는다, Phase 4" → B6b `[B-6b2b]`.
- **:224** (6a): "…B6b로 옮겼다" → "B6b가 들였다". **:231**: delete the seal sentence from the Phase 6+ list (its own marker asks for this).
- **§12.1 (decision 40, H:457):**
  - :269 → "봉인되지 않은 타락 하나와 제동이 들인 AI 많아야 둘(다스리는 백성은 세지 않는다; 제동 없이 넷째 칸에 오른 다스리는 백성이 섭정 아래 AI가 되면 그 생애마다 하나씩 더할 수 있다)".
  - :271 → "T1 → T5(통치자 길)".
  - :272 → extinction is in; the data has `<DATA_ROWS DT_Hazards>` rows; 아공간 #90.
  - :274: verify `enc_witness`.
  - :275 → "5(통치자 행 — T1→T2 하나·T2→T3 둘·T3→T4 하나·T4→T5 하나; 방랑자 길은 Phase 4)".
  - :276 → "거대 프로젝트 8(…+결박 — 데이터 `<n>`행) / 칙령 12(법칙 셋 — 데이터 `<n>`, 안식·낭독 B7)".
  - :292 → "고쳤다(B6b 결정 40)".
- **`WORKPLAN.md`:**
  - :56: "절멸·교단은 Phase 4" → "절멸은 B6b, 교단은 B7".
  - :61: add B6b's commit shape — 필드 → 장치 둘(셋·둘로 나눔) → 표현 → 글 → 데이터 셋 → 지킴 → 값 → 시계 → 다시 잼 → 문서. Also add the 2026-11-09 speed rule: runner full once per commit = two passes; per-commit mutations = new ones plus those touched; the whole-block rerun once at the block's end, in parallel lanes outside OneDrive. Change "(B1~B6a가 이렇게 했다)" to B1~B6b.
  - :65: "AIFourthRungCap은 B6b" → "B6b가 들였다(`FourthRungOpenOf`); 착지·출발의 셈은 6a".
- **`CLAUDE.md:48`**, rule 15: "**현재 코드**(A0~B6a" → "(A0~B6b" (±0 bytes). Leave rules 4, 13 and 14 as they are. Assert the size is ≤ 16,384 bytes.

**G-words: `docs/15_GLOSSARY.md` and `docs/70_CONTENT_GUIDE.md`.**
- **`docs/15`: new rows (N:414).** Screen names come from N §1.5; the binding's screen name is '항성 기관' (`DT_Megaprojects.json:64`). The b6b7 brief's '중력 기관' is stale (N:790 ⑬).
  - 절멸 위기: `Hazard.Extinction`, `crisis_species_extinction`, `CosmicHazards::LetPeopleGo`/`KillsTheRulerOf`, `log_record_extinct`/`log_death_extinction`; 사람의 수 없음, 애도.
  - 방주: `project_ark`, `feat_passage_by_ark`, `Rung.WhenHazard`; 구출의 말, 절멸의 줄에는 서지 않는다, #50.
  - 결박: `project_binding` '항성 기관', `CosmicSeal::BindTargetOf`/`Bind`, `NoFallen`, `log_fallen_bound`, `Sight.Bound`, `bSealed`/`SealedTick`; 설계 낱말 '봉인', 화면 '묶이다'(행위자 없는 수동), 헌정명 '결박'은 Phase 4.
  - 은하 법칙: `ECosmicEdictScope`/`FEdictDef.Scope`, `edict_wide_machine_accord` '기계 살핌'·`_quarantine` '은하 검역'·`_warning` '은하 경보망', `GalaxyLawsToHold`, `Edict.Wide`/`TipWide`; 화면은 '칙령'.
  - 법을 지닌 틱: `LawsHeldTicks`, `CivLawsHeldTicksAtLeast`, `Rung.LawsReset`.
  - 견딘 영압의 시대: `PressureEnduredTicks`, `CivPressureEnduredTicksReached`, `event_era_endured`, 선택자 `Ruled`, 열쇠 `Borders`; 사람에게 붙지 않는다.
  - 경주 / 다른 백성이 먼저: `race:`, `race check`, `Tests/CosmicRaceMeasure.h`, `Ending.OtherPeople`.
  - 셋째 칸 위의 지연: `AIUpperRungProgressMultiplier`.
  - 증언 카드: `enc_witness` '그늘의 가장자리' (label per N §1.5).
- **`docs/15`: existing rows.**
  - :75 칙령: the widening is in; drop #160.
  - :89 지킴: `GuardDelayCapOf`/`FleetDelayCountdownShare` 0.5 (b6b8b brief :292).
  - :109 and :111: `<C>`; drop #2.
  - :129 비문: "다스리는 동안 백성이 이른 가장 높은 칸(첫째 칸이면 없음)". Add the chronicle header to the same row, since A1 follows the rule.
  - :139 고향 은하: "고향 은하에 닿는 칙령".
  - :151: #161 closed.
  - Check for a B6b clause in :17 위기, :63 경외도 (`Borders` +0.2, witness +0.1), :74 거대 프로젝트, :85 조약 (`CivHasTreaty`, the AI treaty step, `Rung.Treaty`), :86 국경 and :136 치명 위기의 경고.
- **`docs/70`: new block after :178 ('끝의 글과 알아챔의 줄'), before :180 ('서막의 문장'):** "**넷째·다섯째 칸의 글**(B6b — 헤더 §3.5, 노트 §1.6)".
  - It holds rules 2-7 condensed from N:531-536: ark; extinction; binding (point to ⑨); laws ("B2 규칙 그대로; 화면은 '칙령'"); the era line; `enc_witness`. Rule 1 is already at :129.
  - Add one sentence for A2: "'묶다'는 결박의 동사 하나다 — 이야기의 줄 둘(`log_fallen_bound`·`Sight.Bound`) 밖에서는 쓰지 않는다(B3의 두 변형은 `[B-6b12]`가 '매듭지었다'·'잇던'으로 고쳤다)".
  - Name the word lists only (the words live in `schema.json`).
  - Cite no verse that is not already in `theology_and_bestiary_notes.md:259` (C.11). The N §1.6 '대조 필요' verses (벧전 3:20-21·계 20:1-3·막 3:27·행 4:20·요 3:11·계 1:19·수 6:3·창 13:17) stay out.
- **`docs/70`: other lines.**
  - :55 (the far line): the three variants are true at any rung, and the fifth-rung climb ends the run on the same tick (#162 ①). "한 백성은 판에 한 번만 셋째 칸에 오르므로" → at most once per rung from the third.
  - :117: add the UI-label exceptions `Rung.Treaty`, `Diplomacy.Slots` and `Rung.Realm` (DECISIONS_LOG.md:214; [B-6b5] ㉖).
  - :174, at the end: "보이지 않는 백성이 낸 엔딩의 글 전체는 B6b(§16 #162)" → "B6b가 정했다(결정 29)". Add the 어긋남 37 note (N:345): the cause line hides an unshown people while the chronicle body keeps the told name (#49, DECISIONS_LOG.md:167). The rule stays; reversing it is [개발자].
  - :186 ⑨: the text is current. The verse check is §7.5.
  - :214: "봉인(`SealedTick`)이 오는 블록에서 … '결박되었다.'" → `SealedTick` is written since B6b `[B-6b2c]`, but no two-books line exists, so it stays 예정 under Phase 6+ (#163).

**G-arch: `docs/20_ARCHITECTURE.md` and `docs/21_DATA_MODEL_REFERENCE.md`.**
- **`docs/20`:**
  - :89 → present tense (its 6,000 mentions are [B-6b10]'s; verify).
  - :109 and :321: #107 → '옛 — 결정 19'.
  - :127: "§16 #1" → closed.
  - :134: "#98 ②의 행은 `[B-6b12]`가 닫는다" → "닫았다(결정 32)".
  - :146 and :151: verify ([B-6b4]).
  - The slot-tooltip clause from §1.2.
- **`docs/21`:**
  - :1692: the digest's "봉인·구한 성계는 없다" → there is a binding now but no digest item for it (item kinds belong to the #152 block).
  - Closed-row references: :253 and :631 (#156); :763 and :1585 (#138); :1011, :1198 and :1658 (#161); :1147 (#2 → `<C>`, unless [B-6b10] already did it); :1264 ("§16 #1·#141" → #164); :1458 (#107); :1469 (#122); :1781 (#162/#121).
- **Copy-vs-code check (G1 lens, §7.2)** of the B6b copies:
  - the two `FCivilizationState` fields; `ECosmicEdictScope`; the three conditions;
  - `GalaxyLawsToHold`, `AIUpperRungProgressMultiplier` and `FleetDelayCountdownShare`;
  - `ECrownBlock::FarAbove`, `NoFallen`, `CosmicSeal`, `FourthRungOpenOf`, `GuardDelayCapOf`, `LetPeopleGo`/`KillsTheRulerOf` and `CosmicKnowing`;
  - the `ECosmicRungPartKind` additions; `ClockTipOf`/`NoteOf`.

**G-design: `crisis_and_transcendents.md`, `systems.md`, `pillars.md`, `DESIGN_AUDIT_AND_IMPROVEMENT.md` and `docs/00_VISION.md`.**
- **`crisis_and_transcendents.md`:**
  - :24, :115, :117 and :133: drop "`[B-6b12]`가 연다", now that #164 exists.
  - :40 and :175: "…B6b 노트 §1.6의 2/3(헤더 §3.5)이고 `[B-6b12]`가 `docs/70`에 옮긴다" → point to the new docs/70 block. At :40, add §16 #166 to '다시는'.
  - :30: Extinction has a row since [B-6b6].
  - :101: at the end of "Tier 4에서는 직접 대면 조우가 열린다", add "(방랑자의 칸이 오르는 블록 — §16 #164)".
  - :102: "…§3.2의 봉인 업적이다" → the binding project `project_binding`.
  - :149 and :187: "§16 #141" → #164.
  - :151 and :153: "…닿게 하는 것은 B6b" → present tense.
  - :140, :154 and :207: `<C>` in place of "마지막 값은 B6b, §16 #2" and "(§16 #2, 약 18,000틱)".
  - :230: the binding is read as `MegaprojectComplete project_binding`; ruler rows do not carry `TranscendentSealed`.
  - :260: Covenant '다시는' → §16 #166.
- **`systems.md`:**
  - :87: "수직 슬라이스 7종 … 차원 방주" → 8 (방주 `project_ark` + 결박 `project_binding` '항성 기관'; the data has `<n>` rows).
  - :128: the '방주' word guard covers rows `project_ark`/`feat_passage_by_ark` only.
  - :163: drop "§12.1의 수는 `[B-6b12]`가 고친다".
  - :95, :162 and :225: #121 → '옛 §16 #121 — B6b가 지금대로 닫았다' (at :95, no '통치자 전용' mark).
  - :417: #161 → '옛 — 결정 16'.
  - :80: "(B6b·B7 — §16 #129·#155) 다시 잰다" → `<B129_*>` and "B7 — #155".
- **`pillars.md:108`:** drop "§12.1의 수는 `[B-6b12]`가 고친다, §16 #160" and point to decision 40. Check that "N은 B6b의 측정이 정한다" now reads `<N_FINAL>`. :106's 'T4 봉인 단독 해제' is record only (#164).
- **`DESIGN_AUDIT_AND_IMPROVEMENT.md`:**
  - :76: the sight uses the ruled people's tier only (어긋남 6).
  - :78: '4칸(법칙)' → 초월자 (어긋남 17).
  - :83-84, :355-356 and :387-388: '개인 Tier ≥ 3' and 공명 → "#1은 B6b가 닫았다(A3)".
  - :245: `Way.Given` → `Way.Received` (어긋남 16).
- **`docs/00_VISION.md`:**
  - :18 ("1판"): B6b brings the mass ascension and the other order inside the run; T5's two ways stay Phase 6+; the clock is `<C>`.
  - :103: if C ≠ 18,000, add one line with `<C>` and the rule (decision 35).
  - :32: `<WALL_A>~<WALL_B>` (P5), or "손 확인 ⑤ 뒤".
  - :20 and :32's 6,000 are [B-6b10]'s; verify.
  - :17 ('1시간 | 한 칸', 어긋남 14): revisit only with P5.

**G-dev: `docs/40_DEV_GUIDE.md` (and the `docs/30` check only).**
- :40, after the B6a sentence:
  > "B6b: `B6b/`에 `<MUT_TOTAL>`개(b6b1 7·b6b2a 31·b6b2b 30·b6b2c 40·b6b3a 33·b6b3b 40·b6b4 63·b6b5 39·b6b6 `<n>`·b6b7 `<n>`·b6b8 `<n>`·b6b8b `<n>`·[b6b9 `<n>`]·b6b10 `<n>`·b6b11 `<n>`·b6b12 `<n>` — 그 가운데 파이썬만 `<n>`·에셋을 다시 짓는 데이터 패치 `<n>`·커맨드렛 탐침 `<n>`). 커밋마다는 새 변이와 그 커밋이 닿은 파일·테스트의 옛 변이만 돌리고(B6b `[B-6b3b]`부터 — 노트 §5 '일의 속도'), 블록의 변이 전체는 블록 끝의 문서 커밋에서 한 번 돌린다 — B6b는 `[B-6b12]`가 OneDrive 밖의 작업 트리 셋에서 나란히(`-WaitMutex -DisableUnity`) 돌렸다(깨짐 `<MUT_KILLED>`/`<MUT_RUN>`, 다시 뜸 `<MUT_RETAKEN>`)."
  - `<MUT_*>` stay placeholders until §5.7.
- :335: "(#1·#141)" → "(#164)".
- :470: "…B6b의 시계 결정(§16 #2)에 넘긴다" → handed over; C=`<C>`.
- :502: the race-check probes — [B-6b7]'s result plus "`[B-6b12]`가 다시 돌렸다 — `<PROBE_RESULT>`".
- :510:
  - Drop "…노트 §1의 어긋남 5가 `[B-6b12]`에 둔 문장을 여기에 먼저 적는다".
  - Write what `World/CosmicRulerPolicy.cpp` does: focus (:493), edicts (:518) and diplomacy (:719, :776) stop once `Counts.bEnded` is set (unless `-PastEnd`). Card answers and builds go on. Under `-Guard=1` the guard, step (1) of `ActDiplomacy` (:733-775), goes on after the end too, and the check at :776 stops the steps after it.
- :514: "< 15%(B6b — … 함께 잰다, §16 #161)" → "B6b는 재고 적는다 — 입력 없는 `<LEAP_OO_INPUTLESS>`·방랑자 `<LEAP_OO_WANDERER>`·강제 죽음 `<LEAP_OO_DEATH>`; 문은 Phase 5". The B6a clause "테스트로만 선다" → present tense.
- Test table:
  - one row per test and `<TESTS_N>`;
  - verify the [B-6b6]..[B-6b11] rows (`Data.Integrity` pins, `Feats.Reachable`, `UI.NextRung`/`UI.RunEnd`/`Sim.EndingOf` at C, `Sim.EraEndured`, `Sim.WitnessCard`, `Sim.FleetDelay` 225);
  - add A1's two cases to the `UI.Chronicle` row.
- `docs/30`: `python -I Tools/make_config_reference.py --check` only. Phase A changes no knob-reading file.

### 1.4 What the lead writes (after the workflow and the lanes)

- The H §11 line (§6.2).
- The N §5 `[B-6b12]` block (§6.3).
- N §0.2: a closing line under the table: "(`[B-6b12]`이 행을 모두 고쳤다 — `DECISIONS_LOG`의 '열린 물음(B6b 전체)')". N §3: nothing new.
- The README merge (§5.7).
- The `<MUT_*>`, `<REAUDIT_*>` and runner fills in DECISIONS_LOG, CHANGELOG, `docs/40:40`, STATUS and the commit message.

---

## 2. Tests

- **No new test.** `<TESTS_N>` stays at [B-6b11]'s count. Checker unit tests are unchanged.
- **Meaning changes in one test, `UI.Chronicle`** (A1): two expectations lose the first-rung '칸 1', and one new case shows '칸 3'.
- **Other tests:**
  - `UI.RunEnd`, in the same file `Tests/CosmicChronicleScreenTests.cpp:179`, is not edited.
  - `Data.SourcesInSync` passes after `make_data_tables` (A2 changes `DT_LogTemplates.json` and therefore the stamp).
  - The variant-count checks are unchanged: A2 keeps 3 and 3.
- **The new assertions break once each** in the lanes (§5): `b6b12_chronicle_rung_at_first` and `b6b12_chronicle_rung_never`. If one survives, tighten the case and run that patch again before the commit.
- **Old patches whose filter or patched file this commit touches** (speed rule), run in the lanes beside the whole-B6b set:
  - every patch of any block on `UI/CosmicChronicleWidget.cpp` (16 outside B6b at HEAD);
  - every README row whose Breaks test is defined in `Tests/CosmicChronicleScreenTests.cpp` (`UI.Chronicle`, `UI.RunEnd`: 17 live rows outside B6b at HEAD);
  - the B4 Python patch on `DT_LogTemplates.json` (`B4/b4d1_schema_singing_star_by_the_hedge`).

---

## 3. Determinism and baselines

- **Bit-identical**: the five lines and the two hash runs must equal `Tools/baselines/seed42.json` as of [B-6b11]. H:358 says [B-6b11] and [B-6b12] do not move baselines. No `-UpdateBaseline`.
- **Why it holds:**
  - A1 is presentation only.
  - A2 changes `FCosmicLogEntry::Message`, which is "Display only, not hashed" (`Core/GalaxyState.h:60`). The variant count and order are the same, so `PushVariants`' `DrawTell` draws are the same count.
  - Comments and docs move nothing.
  - No draw site, saved field, tick stage or knob changes.
- **The runner:** `powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier full -Data`, once, editor closed, after the lanes have finished (they share the UBT mutex). `-Data` is required because `DT_LogTemplates.json` changed (`Tools/run_checks.ps1:48-52`).
  - **Assets:** commit `Content/CA/Data/DT_LogTemplates.uasset` and `Tools/data/generated.sha1`.
  - **Assets regenerated without a source change:** `git status Content/` might show a regenerated `.uasset` whose table did not change. Restore it with `git checkout --` and run `Data.Integrity`/`Data.SourcesInSync` again in the same pass. ([B-6b5] committed only the assets of changed tables.)
- **If any line moves**, stop: a UI or text change cannot move it. Find the cause before anything else. A fix that moves a baseline is never part of [B-6b12] (H §8, the review-fix bullet); it becomes a `[B-6br]` after this commit, listed in STATUS.
- **Hand-check saves (H §9 ②):** H:411 says to "rebuild with the last code commit's build" ("마지막 코드 커밋의 빌드로 다시 만든다"), which is now [B-6b12].
  - The saves are sim state. A UI or text change cannot alter them, and the bit-identical runner is the proof (the B6a7 precedent, STATUS:31: "`[B-6a7]`은 주석만 고쳐 `[B-6a6]`의 빌드와 같은 판을 돈다").
  - Recommended: no rerun. The developer makes or opens them with this commit's build.
  - If [B-6b11] wrote `-SaveAt` command lines in N §5, the lead may run them (cheap, about 6 commandlets) and compare each `-SaveAt` line's hashes with [B-6b11]'s record. Record "같다" or the difference in N §5.

---

## 4. Measurement and the pre-written judgments

[B-6b12] measures nothing. It copies outcomes into the receivers in §0.4. The only runs are the mutation lanes and the two probes plus the smoke (§5).

The pre-written rules below are carried verbatim. Each placeholder is filled by the branch the record shows.

**H §5.3, the intro (H:338):**
> **묶음 B의 판단**(어긋나면 `[B-6b9]` — 탐침은 `-Rules`(r)·`-FourthRungCap`, 행의 값(c·P_ark·P_bind·N)은 커밋하지 않은 데이터 사본으로; 탐침의 판은 판단에 쓰지 않는다 — 탐침은 묶음 B의 시드 1~60으로 하고 B′의 판단과 시계 규칙 ⓐ(B′일 때)는 새 시드 61~120으로 한다(같은 시드의 B′는 결정론으로 탐침의 판 그 자체다)):

| # | 판단 | 칸 | 규칙 | 어긋나면 |
|---|---|---|---|---|
| ① | 다섯째 칸에 닿는다 | R 60 | 기록이 집단 승천이나 다른 질서이고 그 틱이 20,000 이하인 판(노트 §2 ⓗ) ≥ 40/60 | 늦은 다리부터 — 다스리는 넷째 칸이 늦으면 c를 올리고, 넷째 → 다섯째가 길면 N·P_bind를 내린다 |
| ② | '다른 백성이 먼저'의 띠(#161) | R 60의 다섯째 칸 끝(≥ 30) | 다른 질서 15~35% — 분모는 ①의 판, 분자는 기록이 다른 질서이고 그 백성을 어느 생애도 다스리지 않은 판(다스린 적 있는 백성의 다른 질서 — `ending:`의 '어느 생애가 다스린 백성' — 는 분모에 두고 따로 적는다). 점이 띠 밖이면 어긋남; 95% Wilson 구간은 함께 적는다(n = 60에서 구간이 통째로 띠 밖인 것은 ≤ 3/60·≥ 29/60). 다른 질서의 판은 (가) 다른 백성이 결박을 마친 틱에 다스리는 백성이 넷째 칸 이상(r이 닿는 경주)과 (나) 다스리는 백성이 절멸로 지워졌거나 넷째 칸에 닿지 못함(c·P_ark의 몫)으로 갈라 적고, r을 옮기는 판단은 (가)의 몫으로만 한다. 느린 R 60은 같은 셈으로 몫과 구간을 적어 결정 35의 답대로 다룬다(권장: 알린다 — 구간이 통째로 35% 위면(모형 63% — 노트 §2 ⓒ) 시계 규칙의 받아들임과 함께 [개발자]에게 보인다, 시계의 까닭이 사람의 걸음을 느린 R에 두므로; 위 끝을 두기로 하면 그 값은 개발자가 정한다) | 35% 위면 r을 한 칸 내리고 15% 아래면 한 칸 올려 B′로 한 번 — 사다리는 이진수로 정확한 1/16 걸음 0.25·0.3125·0.375·0.4375·0.5(시작값 0.375; 모형으로 R의 다른 질서 9%·17%·26%·34%·43% — 노트 §2 ⓒ); B′의 점도 밖이거나 사다리 밖이 들면 그 수와 함께 [개발자] |
| ③ | 넷째 칸의 계절 | ①의 판 가운데 다스리는 백성이 다섯째 칸에 선 판(≥ 30) | 넷째 → 다섯째 틱의 중앙값 ≤ 3,600(아래 끝 1,200은 구성상 지켜진다 — 계절 ≥ max(P_bind, N); `[B-6b9]`가 둘 다 1,200 아래로 내릴 때만 묻는다); 그 계절 600틱 창마다 결정 — 판마다(창마다 결정 수의) 중앙값 ≥ 1인 판 ≥ 90%(B6a 묶음 0·1의 ⓑ는 모든 판으로 판단했다 — 30/30·28/28; 창은 노트 §2.1) | 길이는 N·P_bind; 결정 밀도는 값으로 고치지 않는다 — [개발자] |
| ④ | 시계 C | 느린 R 60 | 아래 규칙 | 규칙이 정한다 |

**The clock rule (H:349):**
> ⓐ **칸**: 느린 통치자 판(R의 `-Focus=0`) 시드 1~60(값을 고쳐 B′면 61~120), 시계 없이, 지평 20,000, 묶음 B(값을 고쳤으면 B′)의 빌드. ⓑ 판마다 **E** = 첫 다섯째 칸 끝(집단 승천이나 다른 질서)의 틱 — 시계가 없으므로 그 종류의 기록의 틱; 20,000 안에 다섯째 칸이 없거나 침묵이 먼저 선 판은 E = ∞. ⓒ **C₀** = E의 80번째 백분위수 — 60판을 올림차순으로 놓은 48번째 값(가장 가까운 순위)을 1,000의 배수로 올림(∞면 ∞). ⓓ **C = min(18,000, max(6,000, C₀))**. ⓔ **받아들임** — 둘 다: (i) E ≤ C인 판 ≥ 30/60; (ii) 다스리는 셋째 칸의 틱부터 그 판의 첫 기록(침묵 포함)과 C 가운데 이른 쪽까지 600틱의 온 창마다 결정 수의 판마다 중앙값 ≥ 1인 판 ≥ 90%(셋째 칸에 닿지 않은 판은 빠진다; 셈은 노트 §2.1). 하나라도 어긋나면 값을 적지 않고 그 수와 함께 [개발자]에게 묻는다. ⓕ C는 6,000에 끝난 B6a 세이브를 다시 열지 않는다 — 끝은 기록이 정한다(`SaveVersion` 1 그대로, 이주 없음 — A4).

**H:351:**
> **C = 6,000이면**(ⓓ의 바닥 — 모형의 아래 끝이다): `[B-6b10]`은 노브를 옮기지 않는다 — 다섯 줄·해시 판 둘·6,000에 박힌 테스트(`Sim.EndingOf` `Tests/CosmicEndingTests.cpp:621`·`UI.RunEnd`)는 그대로이고, 커밋은 규칙과 잰 수를 `DECISIONS_LOG`에 옮기고 '잠정' 문장(`Data/CosmicDefinitionRows.h:1108`의 머리 줄·`:1114`, `docs/30`, 6,000을 말하는 문서의 표지 — 노트 §1.2)과 `Sim.SaveOldEndFlows`의 더한 경우(C와 상관없이 — §6)만 든다. §5.4(노트 §2.2)의 `[B-6b10]` 행과 §6의 시계 테스트 고침은 C > 6,000일 때만이다.

**N §2.3, which H calls "이 절의 본문" (N:665-677):**
> - **적기만**(B — 칸은 §2.1): 넷째 칸 이상 백성의 수(기록 때·판 끝·최대)와 제동의 거절; 절멸(놓임·방주·만료·지워진 백성·통치자의 죽음); 결박(틱·결박한 쪽·잠김); #162의 보임; #156의 시대 줄과 #141의 `enc_witness`; AI 단계 µs/틱 — B6a 10.4~10.7의 +50%를 넘으면 `[B-6b9]` 전에 까닭을 찾는다. 결박한 성계를 잃어 아무도 다섯째 칸에 닿지 못한 판이 R 60에서 3판 이상이면 #165(결정 12 ㉯ `SealedByCivID`)를 그 수와 함께 [개발자]에게.
>
> **C에서 읽는 것 — 알리되 C의 문이 아니다**(묶음 C):
> - **#136**: R 60 — 기록 전에 1인칭 주장 줄이 말해진 판 ≥ 1/3(20/60)이면 닫고, 모자라면 수와 함께 열어 둔다(시계는 이 수로 다시 정하지 않는다). 결박이 그 뒤의 주장을 끝내 곡선을 자른다(B6a 18,000 참고 칸 — 6,000에 9/30, 6,331에 10/30, §0.1).
> - **#142**(결정 24): 시험 칸 — B4의 모형 구간 규칙(B4 헤더 `:362` — 띠마다 30건 이상, 걷힌 몫이 1 − (1 − 0.02 × W)^(슬롯 수)의 95% 구간 안); 바닥 0.25는 `Data.Integrity`의 불변식(`Tests/CosmicDataTests.cpp:2234`)이 지킨다. 셈하는 주장: 카드가 온 틱의 남은 시간이 401~550이고 놓인 틱이 C 전인 주장; 그 끝이 첫 결박의 틱(`SealedTick`) 뒤인 주장은 편입될 수 없어(§0.4′) 분모에서 빼고 따로 적는다. 기대 N: B6a의 13건 가운데 9건은 6,000 뒤였고 결박(모형 약 6,100~7,700)이 그 뒤의 주장을 끝내므로 140시드에 약 9~12건 — '재지 않음 예상 — 잰 N'; 30건에는 약 450시드(모형).
> - **#132**(결정 23): 몫 0.5에서 모든 태그의 지연 몫은 구성상 50% 이하라(각성 75/150·역병 100/200·초신성 전조와 주장 100/300) 띠의 위 끝은 판단이 아니다 — 그 끝은 `Sim.FleetDelay`(각성 150 → 225)가 지키고 #132는 `[B-6b8b]`가 장치로 닫는다. 시험 칸에서 적기만(30건 이상): 각성 지연 몫(기대 약 48~50% — §2 ⓔ; 10% 아래면 지킴이 함대를 잃은 것이라 까닭을 찾는다), 지연이 상한 75에 닿은 지킴의 몫(거의 모두면 상수가 지연을 정한다 — B7 #155 곁에), 지킨 각성과 지키지 않은 각성의 끝.
> - **B1 지킴**(결정 36): 방랑자 짝을 6,000에서 자른다. 끈 쪽은 다리에 의해 B6a 묶음 1의 켠 쪽 세계라 40/60·1,350을 다시 낸다 — 절대 문 ≥ 44/60은 두 쪽 모두 적기만 한다(B6a의 모자람을 B6b에 돌리지 않는다). **B6b의 판단은 짝**: 켠 쪽에서만 떠난 그릇이 즉위·복귀하지 못한 시드 L과 켠 쪽에서만 한 시드 G — L > G이고 부호 검정(양쪽) p < 0.05면 'B6b가 줄였다'로 L의 시드마다 미리 이름한 길(#122의 `FarAbove`, 절멸이 지운 옛 백성, `enc_witness`의 답, #156의 국경 경외(결정 22의 답 ㉮))을 `race:` 줄로 세고, 길로 설명되지 않는 시드가 남으면 그 수와 함께 [개발자]; p ≥ 0.05면 잡음(B3의 규칙). 즉위까지 중앙값 ≤ 1,800(B3 헤더 `:316`)은 켠 쪽의 문 그대로. 길은 B6a 켠 쪽(복귀 22·빈자리 15·경외 3 — §0.1)과 견준다. **다리**(깨질 수 있는 확인): 끈 쪽의 CSV 결정적 칸이 B6a 묶음 1의 같은 시드 방랑자 판(`Saved/Balance/b6a_bundle1/`)과 6,000 아래 모든 행에서 같다.
> - **#129**: 입력 없는 짝 — 넘김의 3/4 경계(0.75/0.79), 판마다 넘김, 백성마다 얻고 잃은 최대, 6,000 뒤 넘김; 적기만(값은 B7 #155); 가장 많이 얻은 백성의 몫 ≤ 50%를 지켜본다.
> - **#154**: 방랑자·입력 없는 판의 늦은 계승 다이제스트 5~8줄 몫(B5 묶음 3의 꼴) — 적기만.
> - **'도약이 일으킨 다른 질서 < 15%'**(`docs/40:480`): 입력 없는·방랑자·강제 죽음 칸에서 기록이 다른 질서 ∧ `bNoVessel`인 몫 — 칸마다 따로(분모 = 그 칸의 60판 — §2 ⓗ), 점과 구간, 적기만(Phase 5의 목표; 스크립트 R은 스스로 죽지 않아 R만 재면 거저 지킨다).
> - **시계 아래의 끝**: R 60·느린 R 60 × C의 기록 종류(집단 승천·다른 질서·시계·침묵)의 몫과 Wilson 구간 — 적기만.

**How [B-6b12] uses each outcome** (the branch shown by the record decides; never re-judge):
- **①②③** go into docs/50 and STATUS as passed or failed, with the number and the B′ outcome.
  - If B′ also failed, or a value left the ladder, the [개발자] answer that [B-6b9] recorded is quoted.
  - If that answer is still missing, STATUS's Top 5 gets it as item 1, and #161 stays open with the numbers instead of being deleted.
- **④** (C, or no C) is handled in §0.4.
- **#136 / #142 / B1 / `<B_LOCKED>`** follow the branches written above.

---

## 5. Mutations: the whole-B6b rerun (three lanes and a Python pool, outside OneDrive)

### 5.1 Set

The set is every `Tools/mutations/B6b/*.patch` row of the README at the end of Phase A:
- 283 at HEAD, plus [B-6b6]..[B-6b11]'s patches (about +7 C++, +10 asset, +14 Python, plus probes — the mapper's projection), plus the 2-3 `b6b12_*` patches;
- plus the touched older-block set from §2;
- minus rows marked "no longer applies", which are listed and not run.

Expected total: about 320 patches, plus about 25 touched old ones.

### 5.2 Kind by README row, never by the name

`run_mutations2.py:48`'s `"_data_" in tag` and `run_py_mutations.py:9`'s `"_schema_" in tag` misroute patches: for example, `b6b6_data_ark_word_in_card` is caught only by `test_data_schema.py`. So the planner `${SCR}/b6b12/plan.py` reads the README rows instead:

| Kind | Rule | Run |
|---|---|---|
| Asset | Listed at README:5 | apply → `make_data_tables` (lane editor, `-run=pythonscript`; must log "written and verified"; a refusal counts as KILLED, docs/40:40) → filter → `git apply -R` → `make_data_tables` again |
| Python | Listed at README:6, or the Breaks cell names no `Cosmic_Ascension.` test | the backticked `test_*` methods of `test_data_schema.py`; the whole file if the cell names only the file; `check_data.py` if the cell names it |
| Probe | Row typed "commandlet probe" (`b6b7_probe_*`), and `b6b8_cmdlet_witness_first_any_card` | build → the row's commandlet or `bundleB.py` command → expected exit code and lines (b6b7 brief :504-518: exit code 1 and the `race check` line naming the climber; the b6b8 smoke: `python -I ${SCR}/b6b8/bundleB.py --root C:\CAL\lN --max-seed 2 --workers 1` stops on its W controls) → revert → rebuild |
| C++ | Everything else | apply → build → filter(s) joined with `+` (five rows name 2-3) |

The planner asserts that the set of `.patch` files equals the set of rows, and that every C++ filter exists in Tests (`git grep IMPLEMENT_`).

### 5.3 Lane setup (engine facts verified in `C:/Program Files/Epic Games/UE_5.8`)

- **One UBT mutex per engine install.**
  - `UnrealBuildTool.cs:442-447` builds `GetUniqueMutexForPath("UnrealBuildTool_Mutex", <UBT assembly>)`.
  - `-WaitMutex` blocks (`SingleInstanceMutex.Acquire`); without it a second build throws at once.
  - Builds therefore serialize across lanes, and the editor test phase (about 60% of a cycle) runs in parallel.
- **`Build.bat:17-31`** holds a lock in `%tmp%`. Give each lane its own `TMP`/`TEMP` (`C:\CAL\tmp\lN`), so the lanes wait on the UBT mutex instead of polling the batch lock.
- **Path length.** UBT fails any action path ≥ 260 characters (`Actions/ActionGraph.cs:293-297`). Lane roots are `C:\CAL\l1`..`C:\CAL\l3`: outside OneDrive and short.
- **Unity.** UBT's adaptive unity reads `git status --porcelain` (`System/SourceFileWorkingSet.cs:118-120`). Build every lane, every time, with `-DisableUnity` (`Configuration/Descriptors/TargetDescriptor.cs:138`), so each apply or revert recompiles one translation unit.

Steps for each lane (N = 1..3):
1. `git -c core.eol=lf worktree add --detach C:\CAL\lN HEAD`. LF mirrors the main tree's bytes. Check that the `.uasset` files are smudged, not LFS pointers (each > 1 KB).
2. `git -C C:\CAL\lN apply ${SCR}/b6b12/phaseA.diff`. `git status --porcelain` then lists exactly the Phase A files.
3. `make_data_tables` in the lane. It must log "written and verified".
4. `Build.bat Cosmic_AscensionEditor Win64 Development -Project=C:\CAL\lN\Cosmic_Ascension.uproject -WaitMutex -DisableUnity`. The first build is about 5-8 min of mutex time per lane; the three are serialized.
5. Baseline: `UnrealEditor-Cmd C:\CAL\lN\Cosmic_Ascension.uproject -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput "-ExecCmds=Automation RunTests Cosmic_Ascension; Quit" "-TestExit=Automation Test Queue Empty" -AbsLog=…` (the `run_checks.ps1:63` form). Every test must pass, or the lane stops. This guards against false kills from the non-unity build.

Memory: one test editor uses about 1.6 GB.
- Three lanes need about 5 GB, plus one build under the mutex.
- Before each editor launch, wait while < 2.0 GB is free (copy `free_memory_gb()` from `bundleA.py:135-144`) and log the waits.
- Use two lanes if < 8 GB is free at the start. Closing VS Code and Chrome frees about 15-20 GB (mapper 3).

### 5.4 Schedule

- Group items by target file; asset items are grouped by their JSON.
- Cost classes:
  - light C++, about 21 s;
  - heavy header, 60-110 s: `CosmicTestHelpers.h`, `GalaxyState.h`, `CosmicKnowing.h`;
  - asset, about 130 s;
  - probe, about 3 min.
- A shared queue holds the groups, largest first. Each lane thread pulls whole groups. The largest group (`GalaxySimulationSubsystem.cpp`, 39+) takes about 14 min.
- Expected wall-clock: setup 15-25 min, then about 45-55 min. The mutex hold time (about 42 min) is the floor.

### 5.5 One item

1. Snapshot the target's bytes.
2. `git apply <patch>`. If it fails → `NOT APPLIED`.
3. Build or `make_data`. A failed build → `BUILD FAILED`.
4. Test, with `-AbsLog=C:\CAL\lN\Saved\Logs\Mutation_<tag>.log`.
5. Verdict, parsed from the log as `run_mutations2.py` does: KILLED, SURVIVED or NO RESULT.
6. `git apply -R`.
7. Compare the bytes with the snapshot and restore from the snapshot if they differ. This is the CRLF lesson in `run_final_mut.py:53-64`.
8. Output line: `lane tag VERDICT [filters] Ns`, plus up to 4 error lines.

At the end of each lane: a clean build (`Result: Succeeded`), and `git status --porcelain` showing only the Phase A files.

### 5.6 Python pool, at the same time as the lanes

- 4 workers, using the `${SCR}/b6b6/i1_pymut.py` design.
- Copies are `Source`, `Tools`, `Config` and `CLAUDE.md` from the main tree after Phase A (Phase A froze them), in `C:\CAL\py\c0..c3`.
- Each worker runs the named methods (§5.2). The whole suite takes 88 s; a named method takes 0.3-4.8 s (mapper 3, measured).
- First a clean green run in every copy; at the end, a clean run of both tools in every copy.

### 5.7 Merge and dispositions (lead)

Write `${SCR}/b6b12/b6b_all_mutations.txt` in README order, in the `b6b3b_all_mutations.txt` line format. Include:
- counts by kind and verdict;
- the list of anything not KILLED;
- rows with no verdict (must be 0);
- busy time per lane and the wall-clock.

Dispositions. Never commit with an unexplained survivor.
- **SURVIVED.** Find the cause first.
  - (i) The test's meaning moved with data ([B-6b6]..[B-6b10]; the N §1.3 table "데이터를 켜면 뜻이 바뀌는 테스트") → take the same mutation again **under the same name** (same-block rule, docs/40:40) so that it breaks the test as it now stands.
  - (ii) The assertion was lost → tighten the test (bit-identical) and run that patch again.
  - (iii) Flaky → run it once more.
- **NOT APPLIED or BUILD FAILED** (a later B6b commit moved the code) → retake under the same name. Count it in `<MUT_RETAKEN>`.
- **NO RESULT** → run once more. If the filter name no longer exists, fix the README row.
- **Touched older-block patches:**
  - KILLED → nothing.
  - SURVIVED → retake under `b6b12_` only if no other live patch breaks that assertion. Mark the old row in the docs/40:40 form.
- **Probes:** record the seed, tick and line in N §5 and in `docs/40:502` (G-dev placeholder `<PROBE_RESULT>`).

README edits (lead only):
- the `b6b12_*` rows;
- retake marks;
- nothing else.

Then:
- `apply_after.txt` on the final tree (after any §7.3 comment fixes), compared with `apply_mid.txt`. Every OK → FAIL is handled by the docs/40:40 rule.
- Keep the lanes for B7's "new + touched" runs (an incremental `git checkout --detach <sha>` plus build). Write `git worktree remove C:\CAL\lN` in N §5 for when they are no longer wanted.

---

## 6. Docs and records: limits, exact lines, message

### 6.1 Limits

- **`CLAUDE.md` ≤ 16,384 bytes** (16,348 at HEAD, LF). The rule 15 edit is ±0 bytes. `check_docs.py` only prints a NOTE, so the workflow asserts the limit itself.
- **H < 149,000 bytes after the [B-6b12] line.** If it is over, move the longest [B-6b6]..[B-6b11] §11 details into N §5 in this commit, and say so in N §5.
- **No new line over 2,000 bytes** except `docs/50:12` (already one line).
- `python -I Tools/check_docs.py` returns 0, and the other four editor-free checks (`Tools/run_checks.sh`) return 0.
- No EOL-only changes: `git diff --ignore-cr-at-eol --stat` equals `git diff --stat`, and every changed file is `w/lf`.

### 6.2 The H §11 line

Insert it after the last §11 bullet, before `## 추가 이야기와 보완`. With placeholders it is 357 bytes; re-check with `wc -c`.
> - `[B-6b12]`(<DATE>): 문서(§7)·재감사·B6b 변이 전체(<MUT_TOTAL> — 깨짐 <MUT_KILLED>, 다시 뜸 <MUT_RETAKEN>). 더한 것: 연대기 장 머리의 칸은 첫째 칸이면 없다(결정 30); B3 줄 둘의 '묶'을 뺐다(묶음 동사는 결박 하나); `b4d2_data_resist_factor`를 다시 떴다. 비트 그대로. 세부는 노트 §5.

### 6.3 The N §5 `[B-6b12]` block

Use the form of the `[B-6b5]` block, N:781-791.
- **나눔:** none.
- **코드:** A1.
- **데이터:** A2.
- **변이:** `<MUT_*>` by kind (C++, asset, Python, probes); the retakes by name; the touched older-block set; A3's outcome; the lanes' times and memory waits; the probe lines.
- **문서:** files by group.
- **재감사:** `<REAUDIT_*>` in the CHANGELOG form.
- **기준값:** "다섯 줄과 해시 판 둘 비트 그대로" with the values.
- **헤더와 다르게 하거나 더한 것:**
  - ① the code in a docs commit (A1, A2 — handed here by [B-6b4] ⑱ and [B-6b5] ⑭ and ㉞);
  - ② the pointer comments left as provenance (§1.2);
  - ③ `SlotsAll`: no code;
  - ④ ㉕: no change;
  - ⑤ the lanes in place of the main-tree rerun;
  - ⑥ the hand-check saves not regenerated (§3);
  - ⑦ any H text moved to N.

### 6.4 STATUS.md

- **:3:** the 갱신 line.
- **:7:** the B6b paragraph becomes "끝(`<DATE>`, `git log --grep 'B-6b'`, 헤더…)" with the outcomes. Drop the [B-6b8b] share-0 text if it is still there.
- **:26:** `<TESTS_N>`; data counts (`DT_Hazards`, `DT_Megaprojects`, `DT_Feats`, `DT_Edicts`, `DT_Events`, `DT_Encounters` at [B-6b11]).
- **:27:** `<BASE_*>`.
- **:31-33 (next three):**
  1. Hand checks B1~B6b, by the developer, without the console. That is H §9 ①~⑥, with B6a ①·③·⑨ on the B6a build: `git worktree add C:\CAL\b6a 5455e56` plus a build, if they were not done before [B-6b6]. Add [B-6b6]'s ㉗ and ㉘ hand reads if they are still open, and P5's `CosmicEnd` line if it is pending.
  2. The B7 header (xhigh, adversarial ultracode review), with the "B6b가 넘긴 것" list from `docs/50:12`.
  3. As now.
  - Also list the verse cross-check (§7.5) as a developer item.
- **:37-41 (Top 5):** replace the B6a → B6b hand-offs with B6b → B7. Remove #2 (if closed), #107, #121, #122, #161 and #162. Add #164, #165 and #167 as they apply.

### 6.5 Commit message

- **Title:** `[B-6b12] 문서 — B6b 끝: 열린 물음·결정 기록·로드맵·용어·글의 규칙, 재감사(발견 <N>), B6b 변이 전체 <M>개 다시 돌림(모두 깨짐), 연대기 장 머리의 칸(결정 30)·B3 줄 둘의 '묶', 비트 그대로`
- **Body, in order:**
  - the files by group;
  - the re-audit counts;
  - the mutation counts and retakes (and A3's outcome);
  - the five lines and two hash runs, bit-identical, with their values;
  - `<TESTS_N>`;
  - `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Push to origin.

---

## 7. Order of work, the workflow and risks

### 7.1 Order

1. **§0 checks** (lead, about 15 min): `values.json`, `apply_before.txt`.
2. **Phase A** (§1.1; about 30-40 min, including A5's build and test): `phaseA.diff`, `apply_mid.txt`.
3. **In parallel:**
   - (a) the lanes and the Python pool (§5; about 70-80 min);
   - (b) the docs re-audit workflow (§7.2; about 90-110 min), docs only.
   - Neither touches the other's files.
4. **Lead tail:**
   1. merge the lanes (§5.7);
   2. apply the `comment` findings (§7.3), then run `apply_after.txt` and re-run the patches on any touched file in the **main** tree (serial);
   3. the lead's records (§1.4) and the placeholder fills;
   4. `check_docs` and the size checks;
   5. **runner `-Tier full -Data` once** (§3);
   6. commit, then push.

### 7.2 The re-audit workflow (one script, about 45 agents)

This follows the B6a7 lesson that cross-group leftovers should be routed in the same run. The script is `b6b12-docs-reaudit-wf_*.js`. Template strings contain no backticks (WORKPLAN.md:98).

1. **Probe** (lead script `${SCR}/b6b12/reaudit/probe.py`, not an agent; the sweep reuses it). Each probe gives its HEAD value:
   1. README ↔ patch files ↔ tests (283/283/0 orphans at HEAD).
   2. Tests ↔ `docs/40` rows (223, 0 missing; 9 rows name future tests and keep their block markers).
   3. N §1.5 texts = `Tools/data` verbatim (`\|` read as `|`).
   4. Draw-site ledger: `git diff 5455e56 -- Source ':!…/Tests'` grepped for `GetRng|Rng\.|FRandomStream|DrawTell|CosmicSeed::Derive|FRand|RandRange|Purpose::|PushRow|PushVariants|PushWithSign|NoteMet|NoteKnown`. At HEAD it shows 0 sim-draw changes; the added `PushRow` calls are the far lines and `LogFallenBound`.
   5. New chapter:verse since 5455e56 that are not in C.11 (0 at HEAD).
   6. Sizes: `CLAUDE.md`, H, N.
   7. The five editor-free scripts.
   8. Stale markers: the 21 forward markers; 'A0~B6a'; 'Nothing reads it yet'; '절멸·교단은 Phase 4'; '그 행은 아직 없다'; 'Way.Given'; '개인 Tier ≥ 3'; '#1의 블록'; '6,000' wherever it means the clock and C ≠ 6,000 (N:343); the binding called 예정 (`docs/20:134`, `docs/70:214`, `docs/21:1674`); pointers to closed rows that read as open. **Not** the '`[B-6bN]`부터' provenance markers.
   9. Row counts per `DT_*.json` for §12.1.
2. **Edit:** 7 editors at xhigh, one per group in §1.3. Their inputs:
   - `values.json`;
   - the H §5.3/§7/§8/§10/§11 lines;
   - N §0.2, §1 (the 17 rows that name `[B-6b12]`: N:308, 309, 311, 315, 316, 319, 320, 323, 330, 331, 339, 340, 342, 343, 345, 350, 351), §1.2, §1.5, §1.6, §2.3, §3, §5;
   - the briefs' hand-offs (b6b6 :419/:610; b6b7 :502/:536/:591/:605/:616/:679; b6b8 :556/:636/:661; b6b8b :259/:285/:292/:335; those of the [B-6b9]..[B-6b11] briefs).
   - The records editor ends the CHANGELOG with "재감사: (리드가 채운다)". STATUS is left to the wave-2 writer (step 8); H §11 and N §5 belong to the lead.
3. **Audit** (read-only, on snapshots taken after the edit barrier; 10 agents at high):
   - **G1 code facts ×3:** (docs/20+21), (docs/40+15+70), (design+plan+questions+records). Check every sentence of the group's [B-6b12] diff **and** its block diff `5455e56..WT` against code and data at HEAD, using the N §5 and H §11 deviations as drift seeds.
   - **G2 header and lists ×2.** Each listed item is done, and done right. Nothing unasked was done. The B7/B8/Phase 4/Phase 6+ pointers are kept. Numbers match `values.json`.
   - **G3 docs among themselves ×2.** The same fact told differently; duplicates where a link should be; stale § or line references; terms against docs/15; plain words; rule 11; OPEN_QUESTIONS, DECISIONS_LOG and STATUS agreeing on every closed or opened row.
   - **W1 determinism ledger.**
     - Compare docs/20 §9.1/§9.2, the appendix tick order, CLAUDE.md rule 4, docs/21 §7.4, N §0.4 and H §2.11/§2.12 with `StepOnce` and the stage bodies.
     - Check that baseline records agree across N §2.2, `seed42.json` notes, CHANGELOG, STATUS and DECISIONS_LOG.
   - **W2 rule-11 texts.** Every B6b text since 5455e56 and A2's two variants. Checks:
     - an agentless binding;
     - extinction ends in mourning, with no people counts;
     - messiah signals 2/9/10/11 (`theology_and_bestiary_notes.md:236-245`);
     - God never an entity or lever;
     - first person for the fallen only;
     - no '신이 되었다';
     - the name sieve.
     - It flags verses and never adds one.
   - **W3 tests and mutations ledger** (static part only; the rerun numbers are the lead's). Checks:
     - probes 1 and 2;
     - the N §1.3 dispositions;
     - the docs/40:40 sentence;
     - the README head lists.
   - **Finding schema:** `{id, lens, file, where (heading or first words + line), problem, evidence (file:line), fix (exact text or instruction), severity, route}`. `route` ∈ {`doc`, `comment`, `data-text`, `behaviour`, `developer`, `later-block`, `mutation-merge`}.
4. **Verify:** refuters at high, ≤ 10 findings per batch, giving CONFIRMED, PARTIAL (with a corrected fix) or REFUTED (with evidence). This is the `b6b5-text-audit-wf_080e3631-462.js` pattern. Mechanical probe facts skip the refuter.
5. **Fix wave 1:** owner fixers at xhigh, one per group that has findings. Each gets every CONFIRMED or PARTIAL `doc` finding whose file is in its set, verifies it, applies it and returns `{id, outcome applied|partly|not_applied|deferred, note, files}`.
6. **Check** (1 agent at high): every applied fix is in place; the five scripts; the EOL check; the CLAUDE.md and H byte counts.
7. **Critic** (1 agent at xhigh, read-only, schema'd): reads the whole `git diff` for missing H §7, N §1.2 and N §0.2/§1 items, old facts left outside the edited files, and cross-group contradictions.
8. **Fix wave 2:** owners with items, plus one STATUS writer, which leaves the rerun and runner numbers as placeholders.
9. **Sweep** (read-only): probes 6-8 and the scripts again; returns the residue. The lead fixes the residue by hand; there is no third round.

Output: `${SCR}/b6b12/reaudit/record.md` with "발견 N(렌즈별) — 반영 a·일부 b·두지 않음 c; 비평 d; 둘째 차례 e; 마지막 훑기 f". This is `<REAUDIT_*>`.

### 7.3 The routes the workflow may not apply

- **`comment`** (a `Source/`, `Tools/` or `Config/` comment found false): the lead applies it after the lanes. If the file is a `.h`, the editor stays closed and the runner's build covers it. Then run `apply_after.txt`, and re-run the patches on that file in the main tree.
- **`data-text`**: only if slots, variant count and order are kept. The lead applies it after the lanes and re-runs the patches on that JSON. More than 3 such texts → `[B-6br]` after this commit.
- **`behaviour`** → never here. It goes to STATUS item 1 as `[B-6br]`, and the docs describe the code as it is.
- **`developer`** → a new OPEN_QUESTIONS row or a STATUS item.
- **`later-block`** → a pointer only.
- **`mutation-merge`** → §5.7.

### 7.4 Risks and recommended resolutions (recommended = approved)

1. **A lane or the pool touches the main tree** → forbidden. Lanes live at `C:\CAL\…`. The main tree only gets builds in A5 and the final runner.
2. **The UBT mutex and Live Coding.** No editor may be open (any project's Live Coding blocks every UBT build on this engine). No runner, and no other workflow's build, may run while the lanes do: they queue behind the mutex.
3. **Memory.** Below 8 GB free at the start, use 2 lanes. The per-launch wait is logged.
4. **Survivors from data-on meaning changes** are expected. Use the §5.7 dispositions, and record the retakes by name in N §5.
5. **A probe without a seed:** carry [B-6b7]'s "not probed — no seed". Do not invent a seed.
6. **`bundleB.py` missing from scratch:** mark the smoke "not run — harness gone" in N §5. Do not rebuild the harness.
7. **A docs agent edits code:** the freeze is in every prompt. A diff on `Source/`, `Tools/` or `Config/` at the check step is reverted by the checker and turned into a `comment` finding.
8. **H crosses 149,000:** handled in §6.1.
9. **④ failed (no C):** #2 stays open, with the numbers and the [개발자] question. Every `<C>` receiver says "잠정 6,000 — #2" (as [B-6b10] left it).
10. **P5 not done:** no minutes; "손 확인 ⑤ 뒤" in `docs/00_VISION.md:32` and STATUS.
11. **The critic hits the usage limit** (the B4e precedent): run the critic alone again with the same schema.
12. **Hand-check saves:** see §3. The developer may ask for them to be made with this build. Six `-SaveAt` commandlets take about 10 min, after the runner.

### 7.5 [개발자] items

- **The verse cross-check** ([B-6b5] ⑩): `docs/70:186` ⑨'s 유 1:6·계 20:2-3 (present since before B6b) and the N §1.6 '대조 필요' list. These go through the developer's tool. Agents never read `Downloads/jehoba`. Add them to `theology_and_bestiary_notes.md:259` (C.11) or remove them; no verse is added in [B-6b12].
- **Hand checks:** H §9 ①~⑥, B6a ①·③·⑨, and P5's `CosmicEnd` line.
- **Any [개발자] branch left open by ②, ③ or ④, by `<B_LOCKED>` ≥ 3/60, by B1-guard seeds not explained by a named path, or by #142 ≥ 30 outside the band:** these go into OPEN_QUESTIONS and STATUS Top 5 as they stand.

---

## 추가 이야기와 보완

- **A machine guard for '묶'.** After A2, '묶' stands only in `log_fallen_bound` and `Sight.Bound`. A `row_not`-style rule, like [B-6b6]'s '방주' guard, could keep it there in every text column, so the next text would not need a text audit to hold the one meaning. That is a schema rule, a Python mutation and a test. Recommended for the first text commit of B7, not here (it needs `-Data` again and widens the schema).
- **Make `probe.py` a standing check.** Especially probe 3 (N §1.5 copied verbatim into data) and probe 1 (README ↔ patches ↔ tests). This is a Phase 5 tooling question.
- **A kind column in the README.** A fourth column (C++ / asset / Python / probe) would stop every runner from inferring kinds and misrouting `_data_`/`_schema_` names. It would be a mechanical README edit at the start of B7, with no patch changes.
- **Lanes for B7.** Keeping `C:\CAL\l1..l3` makes B7's "new + touched" runs incremental. The main tree no longer needs to be blocked during a commit's mutation step.
- **`has_cond` as a schema operator** (b6b7 :679): it would move the binding pin from C++ into `check_data`. This belongs to B7 or Phase 4, with the next rule that needs it.
- **`silence_words` and `edict_sabbath`:** the line in `docs/50`'s B7 list carries it into the B7 header. No OPEN_QUESTIONS row, because the B7 header is the next task.
