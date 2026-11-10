# [B-6b10] the clock: working notes (brief section 0 on {P}) - these win over brief_final.md

Written 2026-11-10 by the lead's section-0 agent. Read-only work only: nothing in the repository was edited.

## 0. Verdict

- **{P} = `5ed07e1` [B-6br]** (pushed; `main...origin/main` level). `[B-6b9]` was not opened (every bundle-B judgment passed).
- **Branch M, C = 9,000.** All five gates of brief section 0.3 pass (section 3 below). The independent recount (lens L4) matches N section 3 exactly (section 4 below).
- Slow-R judgment 2 = 31/60 = 0.517 [0.393, 0.638], wholly above 35%: **outcome B** of brief section 4 (c). It is a report, not a gate. The developer already saw it with C's acceptance on 2026-11-10; the records say so (section 2, `<SLOWR_J2>`).

## 1. Section 0.1 on {P}

### 1.1 History and processes

- `git log --oneline -8`: 5ed07e1 [B-6br], 8b6c5a7 [B-6b8b], 84f0d36 [B-6b8], 220129d [B-6b7], 6ae9060 [B-6b6], 10205b6 [B-6b5], 529cab9 [B-6b4], cef8cc7 [B-6b3b]. No `[B-6b9]`.
- `tasklist`: no UnrealEditor, UBT or dotnet build process.
- `status_before.txt` = `<scratchpad>/b6b10/status_before.txt`, 232 lines. Every file this commit edits shows either nothing or a stat-only ` M` there (`CosmicEndingTests.cpp`, `CosmicLeapTests.cpp`, `CosmicSaveSessionTests.cpp`, `Save/CosmicSaveSubsystem.cpp`): `git diff --quiet HEAD --` is true for all 24 staged or guarded paths, and none holds a CR byte. So the working-tree copies equal {P}; the brief's `git diff --quiet` guard of section 5 holds for `World/CosmicGameMode.cpp` and `Save/CosmicSaveSubsystem.cpp`. Risk 12 compares against this snapshot (stat-only ` M` lines may come and go with OneDrive; compare paths, then content with `git diff --quiet HEAD`).
- `Tools/mutations` is clean against {P}.

### 1.2 Anchors re-derived on {P} (brief line -> {P} line; "same" = unchanged)

| What | brief (10205b6) | {P} 5ed07e1 |
|---|---|---|
| `DT_Rules.json` `"ProvisionalEndTick": 6000,` | :57 | same (:51 `PetitionPeriodTicks` 300 same) |
| `Data/CosmicDefinitionRows.h` section header / "Start value 6000" / default | :1163 / :1169 / :1171 | **:1164 / :1170 / :1172** (+1) |
| `Tests/CosmicTestHelpers.h` include `Data/CosmicDefinitionRows.h` / `EndRunAt` / `FindGameInstance` | :19 / :64-72 / :74 | same |
| `Tests/CosmicEndingTests.cpp` banner :135, blank :136, `EndingClockAt` comment+function | :137-143 | same; `namespace CosmicEndingTests` opens :38 |
| same: `Sim.EndingOf` comment "the game's clock is 6,000" / test-local `= 6000` / game-clock assert | :396 / :416 / :622 | same |
| same: seven unqualified `EndingClockAt` calls | :674 :762 :803 :812 :951 :986 :1021 | same (tests at :655 EndingRecordOnce, :784 EndingInert, :924 RunLeapStopsAtEnding) |
| `Tests/CosmicChronicleScreenTests.cpp` (`UI.RunEnd` = :181-599) "at exactly tick 6,000" / assert / "(10) ... tick 6,000" / "(the climb may pass tick 6,000)" | :174 / :219 / :390 / :413-414 | **:176 / :221 / :392 / :416** |
| `Tests/CosmicLeapTests.cpp` comment / call / guard | :349-352 / :356 / :367 | same; `LeapRules` def :53, callers :245 :426 :579 :711 :841 :939 same |
| `Tests/CosmicSaveSessionTests.cpp` `Sim.SaveOldEndFlows` header comment / IMPLEMENT | :634-643 / - | :634-643 / :644 |
| same: `bWasBroadcasting` / `ON_SCOPE_EXIT` / `Rules` / `PastRecordBy` | :660 / :662-671 / :673 / :684 | :660 / :662-671 / :673 / **:683** |
| same: "(6,201)" label / "Ending.Clock at 6,000" label / "(6,300)" comment / case (4) end / `return true;` | :752 / :739 / :772 / :782 / :783 | same |
| same: includes `Core/CosmicTags.h` :9, `Save/CosmicSaveSubsystem.h` :33, `Tests/CosmicTestHelpers.h` :36 | - | as listed |
| `World/CosmicGameMode.cpp` latch `if (!Ending.Kind.IsValid())` / comment with 'Clock at tick 6000' | :1215 / :1229 | same |
| `Save/CosmicSaveSubsystem.cpp` includes (no `Core/CosmicTags.h`) / `MarkOf` / `bEnded` | :3-23 / :124 / :132 | same |
| `Sim/GalaxySimulationSubsystem.h` `GetRules` / `GetTables` | :295 / :304 | same |
| Knob readers: `Sim/CosmicEndings.cpp` / `UI/CosmicTopBarWidget.cpp` | :68 / :221 :887 | same (no other non-test reader) |
| Policies read the record | `CosmicRulerPolicy.cpp:392`, `CosmicWandererPolicy.cpp:19 :31` | same |
| Record write | `GalaxySimulationSubsystem.cpp:1668-1674` | same |
| Commandlet: `ClockTick` set / end check / race check / `:clock` / calling deaths / `end:` line | :1919-1920 / :1953-1955, :1968 / - / :2423 / :3774 / :3792 | **:1923 / :1971, :3574 / :2028, :3580 / :2428 / :3779 / :3797** |
| Commandlet CSV headers: main / `_claims` | :1716 / :698 | **:1719 / :701** |
| `Tools/run_checks.ps1`: smoke filter / `Compare-Balance` / compare / balance steps / hash runs | :59 / :81 / :87-98 / :119-131 / :135-155 | :59 / :81 / :86-99 / **:124-141** (3,000 :124 :128 :135; 20,000 :140 :141) / :153 onwards |
| `docs/20_ARCHITECTURE.md` end paragraph / old-save parenthesis (and :197 "6,000에 끝난 B6a 세이브도 끝난 채다") | :89 / :196 | same |
| `docs/21_DATA_MODEL_REFERENCE.md` copy comment | :1147 | same (both spans present verbatim) |
| `docs/30_CONFIG_REFERENCE.md` | :60 | same |
| `docs/40_DEV_GUIDE.md` LeapDeterminism / EndingOf / RunEnd / SaveOldEndFlows / B6a baseline history `first Clock at 6000` | :249 / :308 / :317 / :329 / :498 | **:251 / :310 / :319 / :331 / :502** |
| `docs/50_ROADMAP.md` (both spans) | :12 | same |
| `docs/00_VISION.md` clock marker / end marker / MVP history | :18 / :20 / :32 | same (both spans present verbatim) |
| `OPEN_QUESTIONS.md` #2 / #129 / #136 | :12 / :98 / :103 | :12 / :98 / **:102** |
| `STATUS.md` | :3 :7 :8 :26 :27 :29-33 :39 | same (Top 5 item 3 = :39; marker at :8) |
| `DECISIONS_LOG.md` B6b block; insert after | after `[B-6b9]`/`[B-6b8b]` | **after the `[B-6br]` bullet at :44** |
| H section 5.3 frozen lines (slow-R / rule / branches) | :343 / :349 / :351-352 | same (`sed -n '343p;349p;351,352p'` unchanged since 10205b6; H changed only by five section-11 lines) |
| H section 11: insert after | `[B-6b9]`/`[B-6b8b]` line | **after the `[B-6br]` line at :475** |
| H movable bullets ('추가 이야기와 보완') "판의 길이가 비전보다 짧을 공산" / "정책은 끝 뒤에도 답하고 짓는다" | :476 / :479 | **:481 (425 bytes with LF) / :484 (490 bytes)** |
| N #2 row / mismatch 35 / VISION:18 split / `LeapRules` "N:450" / section 2.2 `[B-6b10]` row / section 3 bundle B / bundle C | :184 / :343 / :400 / :450 / :661 / :685-687 / - | :184 / **:344 / :422 / :451 / :688 / :742-764 / :766-768** |
| N section 5: insert after | - | after the `[B-6br]` block (:924-938, the file's end; N is 938 lines, 501,469 bytes) |

### 1.3 Header size (section 0.1 step 3, brief 6.4, risk 9)

- H = **148,981 bytes** at {P} (`git show HEAD:` and the tree agree).
- The section-11 line with this commit's values (below) is **357 bytes** with its LF: 148,981 + 357 = 149,338 > 149,000.
- **Move, same commit:** H:481 "판의 길이가 비전보다 짧을 공산" (425 bytes; C now answers it) into N section 5 `[B-6b10]`, leaving `- **판의 길이가 비전보다 짧을 공산**: → 노트 §5 \`[B-6b10]\`` (77 bytes with LF). Result about 148,981 + 357 - 425 + 77 = **148,990** (< 149,000; margin 10). If the final line grows by more than 10 bytes, also move H:484 (490 bytes) the same way. Never move section 5.3. Check `wc -c` on H after the edit.
- The planned line (the "노트 :450" of the brief is **노트 :451** on {P}):
  `- \`[B-6b10]\`(2026-11-10): 시계 C = 9,000(§5.3의 규칙 — 느린 R 시드 1~60, C₀ 9,000, (i) 56/60·(ii) 55/55). 더한 것: \`EndingClockAt\`을 \`CosmicTests\`로(\`using\` 한 줄), \`Sim.SaveOldEndFlows\` (5)와 변이 둘; 시계 핀은 \`LeapRules\` 대신 \`CosmicLeapTests.cpp:356\`의 부름에(노트 :451과 다름 — 공유). 세부는 노트 §5.`
  (measured with :450 = 357 bytes; :451 is the same length.)

### 1.4 Parent CSVs and logs (section 0.1 step 4)

- `Saved/Logs/checks_20000.log`, `checks_ruler_20000.log`, `checks_wanderer.log` (and `checks.log`, `checks_ruler.log`) hold {P}'s `seed42.json` lines exactly (`summary_line_20000`, `ruler_summary_line_20000`, `wanderer_summary_line`, `summary_line`, `ruler_summary_line` - all MATCH, ruler and wanderer tails included). They are `[B-6br]`'s last runner (12:39, before its 13:45 commit; `[B-6br]` was bit-identical). No rerun needed.
- Copied to `<scratchpad>/b6b10/parent_csv/` (20 files: for each of `checks`, `checks_ruler`, `checks_wanderer`, `checks_20000`, `checks_ruler_20000` the main CSV, `_rows`, `_claims` and the log); sha1 list `<scratchpad>/b6b10/parent_csv.sha1`.
- Error lines in the five parent logs: no `CosmicBalance end check`, no `race check`, no `LogCosmic: Error` (0 / 0 / 0 in each).

### 1.5 Parent record kinds (section 0.1 step 5 - from the `first <kind> at <tick>` part of the `CosmicBalance end:` line)

| Run | Parent `end:` line | Parent record | What must happen at C = 9,000 |
|---|---|---|---|
| input-less 20,000 (`checks_20000`) | `first Clock at 6000; clock at 6000` | **Clock@6000** | moves: both hashes, by the record alone; whole CSV, `_rows`, `_claims` equal the parent's on every row (TickMicros, SaveBytes, SaveRawBytes out). Expected new record (not a gate): OtherPeople at about 7,950 (civ_8 stands on the fifth rung at 7,950 - `[B-6b7]` note in `seed42.json`; bit-identical since). `clock at 9000`. |
| ruler 20,000 (`checks_ruler_20000`) | `first RuledPeople at 5899; clock at 6000` | **RuledPeople@5899 (not Clock@6000)** | **must NOT move**: line bit-identical, whole main CSV, `_rows` and `_claims` equal the parent's on every row; only the `end:` line's `clock at` (6000 -> 9000) and any `:clock` tags may differ. |
| wanderer 6,000 (`checks_wanderer`) | `first Clock at 6000; clock at 6000` | **Clock@6000** | moves: both hashes; the record at 6,000 disappears ('by end' too); rows with Tick < 6000 equal (claims: PlacedTick < 6000 and 0 <= EndTick < 6000). `clock at -1`. |

- **Consequence for the records.** H section 5.4 / N section 2.2's `[B-6b10]` row (N:688) was written when the ruler 20,000 ended by the clock; since `[B-6b7]` it ends by the ruled people's end at 5,899, so its column "20,000 세계(정책이 6,000 뒤로 이어 간다)" does not apply. Expected moving lines: **two** (input-less 20,000, wanderer 6,000). The ruler 20,000 stays: "unchanged (parent record RuledPeople at 5,899)". Write this in the commit message, the `seed42.json` note (drop the ruler clause per brief 3.3), N section 5, DECISIONS_LOG and STATUS '기준값'. If the ruler 20,000 line moves, that is not C: stop and find the cause.
- `clock at` arrival check after the runner (brief 3.3): `checks_20000.log` and `checks_ruler_20000.log` show `clock at 9000`; `checks_wanderer.log` shows `clock at -1`. Any `clock at 6000` = stale `DT_Rules.uasset`: stop.
- **seed42 moves check (review L1-1; brief 3.3 refers here).** Order: final runner (`-Tier full -Data -UpdateBaseline`) -> `fix_seed42.py` (no flags) -> this check -> only then the hand edit of `last_recorded_in_changelog` / the note. Run from ROOT: `python -I -X utf8 <scratchpad>/b6b10/fix/check_seed42_moves.py`. It loads `git show HEAD:Tools/baselines/seed42.json` and the tree file, strips `, wrote .*?(?= \| |$)` (run_checks.ps1's `Remove-WrotePath`) and requires: (a) `summary_line`, `ruler_summary_line`, `ruler_summary_line_20000` equal HEAD letter for letter (ruler tails included; the ruler 20,000 holds `hash 1375149537, world 510604211`); (b) `summary_line_20000` and `wanderer_summary_line` differ from HEAD; (c) no other top-level key differs except `verified_at`. Exit 1 on (a) or (b) = stop (brief risk 5 / section 3.2), nothing committed. Cross-check: `git diff -U0 Tools/baselines/seed42.json` at that point shows only `summary_line_20000`, `wanderer_summary_line` (and `verified_at` if the date changed). Only after PASS may the records say '1375149537 / 510604211 그대로'. Self-tested (fix/st_*.json): tree as now -> STOP (b); moved pair + other wrote path -> PASS; ruler hash or tail changed -> STOP (a); `ticks` changed -> STOP (c).

## 2. Section 0.2 - placeholders (from N section 3 '묶음 B' and its archive; nothing invented)

| Placeholder | Value | Source |
|---|---|---|
| `<SEEDS>` | 1~60 | N :744 (bundle B, not B′) |
| `<E_LIST>` (ascending) | 5,749·5,820·6,009·6,074·6,241·6,341·6,459·6,758·6,772·6,805·7,059·7,063·7,078·7,332·7,439·7,458·7,472·7,495·7,549·7,580·7,624·7,658·7,759·7,766·7,774·7,809·7,881·7,894·7,944·7,971·7,975·7,980·7,985·8,014·8,036·8,040·8,063·8,080·8,126·8,152·8,158·8,180·8,190·8,268·8,268·8,359·8,413·8,439·8,448·8,550·8,616·8,724·8,730·8,738·8,961·8,998·9,009·9,028·9,567·10,794 | N :749 `<E_1..E_60>` sorted; all 60 finite (29 RuledPeople, 31 OtherPeople) |
| `<E48>` | 8,439 (seed 25) | N :749, summary.json `E48` |
| `<C0>` | 9,000 | N :749 |
| `<C>` | 9000 (code/JSON) | ⓓ min(18,000, max(6,000, 9,000)); ⓔ passes |
| `<C_TEXT>` | 9,000 | - |
| `<C_NEXT_TEXT>` | 9,001 | - |
| `<C_MIN>` | 15 ("about 15 minutes" / "약 15분") | 9,000 / 600 = 15.0, ".0" dropped |
| `<I>` | 56 (56/60) | N :749 |
| `<IID>/<IIM>` | 55/55 (= 1.000 [0.935, 1.000]) | N :749 |
| `<IIK>` | 5 (seeds 20·30·32·35·53) | N :749 |
| `<F>` | 0 (summary.json `failed {}`, `withheld []`; N :744 "두 번 실패한 판 없음"; `slowR_35_off` failed once and passed on rerun - not twice) | no F note needed |
| `<SLOWR_J2>` | 31/60 = 0.517 [0.393, 0.638] (가름 (가) 15·(나) 16·(다) 0, 다스린 적 있는 백성 0) | N :747; Wilson rechecked |
| slow-R outcome | **B** - interval wholly above 35%. Already shown to [개발자] on 2026-11-10 with C's acceptance. Records: DECISIONS_LOG "구간이 통째로 35% 위라 [개발자]에게 보였다(2026-11-10, C의 받아들임과 함께)"; STATUS Top 5 and N section 5 carry the brief's line "[개발자] — 느린 R의 다른 질서 0.517 (0.393~0.638), 시계의 받아들임 (i) 56/60·(ii) 55/55와 함께; 위 끝을 둘지는 개발자" marked as shown on 2026-11-10 | brief 4 (c) |
| `<OLD_H20K>/<OLD_W20K>` | 1957158585 / 3764704052 | {P} `seed42.json` `summary_line_20000` |
| `<OLD_HR20K>/<OLD_WR20K>` | 1375149537 / 510604211 (expected unchanged) | `ruler_summary_line_20000` |
| `<OLD_HW6K>/<OLD_WW6K>` | 1631315934 / 489794244 | `wanderer_summary_line` |
| 3,000 lines (must stay) | input-less 2557850423 / 2906170550; ruler 1189870756 / 3359656751 | `summary_line`, `ruler_summary_line` |
| `<H20K>/<W20K>`, `<HR20K>/<WR20K>`, `<HW6K>/<WW6K>` | **pending** - this commit's `-UpdateBaseline` run (`<HR20K>/<WR20K>` must equal the OLD pair) | brief 3 |
| `<MUT_N>` | 3 new (`b6b10_latch_reads_the_knob`, `b6b10_markof_reads_the_knob`, `b6b10_data_clock_six_thousand`) | brief 5.1, branch M |
| `<MUT_OLD_N>` | **39** (section 5 below) | computed on {P} |
| `<DATE>` | the commit date (2026-11-10 if it lands today) | - |
| `<k>` (DECISIONS_LOG "변이 새 <k>") | 3 | = `<MUT_N>` |

`seed42.json` `last_recorded_in_changelog` (branch M, after `fix_seed42.py`): `task` "B6b B-6b10 (the clock's last value C = 9000 by the pre-written rule, decision 35) - ..."; `state_hash_20000`/`world_hash_20000` = `<H20K>/<W20K>`; `_3000` unchanged; `note` per brief 3.3 with the ruler clause replaced by "the ruler 20,000 unchanged (parent record RuledPeople at 5,899; rows equal on every row)".

## 3. Section 0.3 - the branch

| Gate (any one -> X) | Reading on {P} | Result |
|---|---|---|
| (1) ⓔ fails | (i) 56 >= 30; (ii) 55/55 >= 90% | pass |
| (2) fewer than 30 runs reach ruled T3 before their end | 55 judged (5 excluded) | pass |
| (3) N section 3 `<C>` = "없음 (...)" | C = 9,000 | pass |
| (4) slowR withheld (two or more twice-failed runs) | summary.json `withheld []`, `failed {}`; N :744 "두 번 실패한 판 없음" | pass |
| (5) bundle-B lines all good | archive `summary.md`: "sha1 지킴: unchanged (14 files)", "스위치·필드 대조: 통과" (끈 쪽의 r 0.375), "짝(Tick < T_B): 240/240 통과", "다리 ...: 120/120 통과", "check 줄: end check 0, race check 0", "판정: **PASS**"; N :744 says the same (`LogCosmic: Error` 0 too) | pass |

**Branch M** (6,000 < C = 9,000 <= 18,000): all of brief section 1. Not S, not X. C is not clipped at 18,000, so (i) does not bite (H:352).

## 4. L4 - archive check and independent recount (brief 4 and 7.1 step 1)

- Archive `Saved/Balance/b6b_bundleB/manifest.sha1` (2,164 entries, CRLF): the 540 `.log` files in the archive copy all match; all 2,164 files in the scratchpad original `scratchpad/b6b8/bundleB/` match (logs, CSVs, `bundle_meta.json`, `runs.json`, `summary.json`, `summary.md`). 0 mismatches.
- Fresh script `<scratchpad>/b6b10/recount_c.py` (not copied from `bundleB.py`; L4 rules 1-6 restated in its docstring) over `slowR_<1..60>.log`; output `<scratchpad>/b6b10/recount_c_out.txt`:
  - kinds: OtherPeople 31, RuledPeople 29; E per seed identical to N :749;
  - E48 8439, C0 9000, C 9000; (i) 56/60 pass; (ii) 55/55, excluded 5 [20, 30, 32, 35, 53], zero-window [] -> pass;
  - E lower median 7,971 (5,749~10,794), matching N :764.
- **Match with N section 3: exact.** The knob may move.

## 5. Mutation rerun set on {P} (brief 5.2; written now so I1 and the lead share one list)

`git grep -l -F "+++ b/<file>" HEAD -- Tools/mutations` per edited file, plus README filter rows; apply status from `git apply --check` on {P} (`<scratchpad>/b6b10/apply_before.txt`: 941 OK / 147 FAIL over 1,088 patches - identical to `scratchpad/b6br/apply_after.txt`).

- filter `Sim.SaveOldEndFlows` (3): `b6a3c_latch_reads_the_predicate`, `b6a3c_cosmicend_state_tick`, `b6a3c_markof_reads_clock`.
- patched file `Tests/CosmicTestHelpers.h` (19): `b2c_helper_keeps_edict`; `b4d2_unpressed_{drops_more,keeps_cards,keeps_ease,keeps_edict,keeps_voice}`; `b5d2_without_death_keeps_expiry`; `b6a6_exalt_as_before_is_the_game`; `b6b3b_fourth_cap_{every_row,home_row_blank,home_row_only}`; `b6b3b_without_b6b_{default_rules,drops_log_rows,drops_other_rows,keeps_binding,keeps_extinction,keeps_knobs}`; `b6b7_without_b6b_keeps_fifth_feat` (not in the brief's list - added at `[B-6b7]`); `b6b7_without_b6b_keeps_laws`. All 19 apply on {P}; re-check after the insertion (risk 7).
- filter `Sim.EndingOf` (4): `b6a3b_other_people_first`, `b6a3b_other_people_reads_knowing`, `b6a3b_no_vessel_exception_dropped`, `b6a3b_silent_ignores_leap`.
- filter `UI.RunEnd` (5 standing): `b6b4_fifth_rung_gold`, `b6b4_fifth_rung_glow`, `b6b4_fifth_rung_chime`, `b6b4_fifth_rung_opens_sight`, `b6b4_run_end_skips_presentation`.
- filter `Calling.LeapDeterminism` (3 standing): `b5b_countdown_never_succeeds`, `b5b_runleap_no_boundary`, `b6a3c_advanceleap_no_second_call`.
- patched file `Tools/data/DT_Rules.json` (5 standing, data patches - rebuild the assets for each): `b6b2a_data_laws_knob_four`, `b6b7_data_ark_margin_half`, `b6b7_data_upper_rung_not_binary`, `b6b8b_data_share_off`, `b6b8b_data_share_third`.
- No old patch touches `CosmicEndingTests.cpp`, `CosmicChronicleScreenTests.cpp`, `CosmicLeapTests.cpp`, `CosmicSaveSessionTests.cpp` or `Data/CosmicDefinitionRows.h`.
- **`<MUT_OLD_N>` = 3 + 19 + 4 + 5 + 3 + 5 = 39.** Brief cross-check (about 42): the difference is the two extra skips below plus `b6b7_without_b6b_keeps_fifth_feat`.
- **Skipped, named (4):** `b4d2_data_resist_factor` (README: no longer applies since [B-6b2a]); `b6b2c_data_ark_margin_half` (README: no longer applies since [B-6b7], retaken as `b6b7_data_ark_margin_half`); **`b6a3a_rung_end_back`** (UI.RunEnd filter - FAIL on {P}: it patches `ProvisionalEndOf` in `Sim/CosmicEndings.cpp`, deleted by `[B-6a3c]`; N :876 already says it "서지 않는다"; in the 147-FAIL registry since before `[B-6b6]`); **`b5b_tenth_outside_guard`** (LeapDeterminism filter - FAIL on {P} at `GalaxySimulationSubsystem.cpp:1574` context; in the same registry). The fix stage (review L2-2) marked both README rows in the existing style (`b5b_tenth_outside_guard`: no longer applies since [B-6a3b], checked by `git apply --check` per commit - OK at e1388d9 [B-6a3a], FAIL from 0782920 [B-6a3b]; `b6a3a_rung_end_back`: since [B-6a3c] - OK at 0782920, FAIL from db3d8c4), "not retaken; [B-6b12]'s whole-B6b rerun decides"; N section 5 and the commit message say so.
- **Recorded exemption** (brief 5.2): tests `Sim.EndingRecordOnce`, `Sim.EndingInert`, `Sim.RunLeapStopsAtEnding`; their README rows on {P}: `b6a3b_record_before_tenth_stage`, `b6a3b_record_overwritten`, `b6a3b_record_draws`, `b6a3b_stage_reads_record`, `b6a3c_runleap_never_stops`, `b6a3c_runleap_stops_past_the_record`, `b6a3c_runheadless_stops_at_the_record` (7; `b6a3b_no_vessel_exception_dropped` also names EndingRecordOnce but is rerun via `Sim.EndingOf`). Valid only with the `using` declaration (bodies byte-identical).
- After the last revert, `git apply --check` over all patches must again give 941 OK / 147 FAIL plus the 3 new `b6b10_` patches as OK (compare with `apply_before.txt`).

## 6. Other corrections to the brief for {P}

- H section 11 and DECISIONS_LOG go after the `[B-6br]` entries (not `[B-6b9]`/`[B-6b8b]`).
- The brief's `bundleB.py` line citations (`judge_4` :573-616 etc.) are off on the current file (`window_median` :450, `judge_3` :576, `judge_4` :633); the recount does not depend on them.
- The brief's §2.2 row text (N:688) is still verbatim as quoted in brief 3.2.
- `Sim.SaveOldEndFlows` case (5): `PastRecordBy` is declared at :683 (the brief's :684); nothing else moves.
- `docs/40:502` (the brief's :498) stays unedited (B6a baseline history).
- `OPEN_QUESTIONS.md` #136 is :102 (the brief's :103); not edited here.

## 7. Before the commit (review L3-1 - extends brief 6.6)

- The placeholder grep gains `P_[A-Z_]+` and must come back empty on the staged diff:
  `git diff --cached | grep -nE '<(C|C_TEXT|C_NEXT_TEXT|C_MIN|C0|E48|E_LIST|SEEDS|I|IID|IIM|IIK|F|SLOWR_J2|DATE|MUT_N|MUT_OLD_N|k|OLD|NEW|[A-Z]*20K|[A-Z]*6K|OLD_[A-Z0-9_]+|P_[A-Z_]+)>'`
  (or, before staging, `grep -nE '<P_[A-Z_]+>|<(H|W|HR|WR|REC)20K>|<(HW|WW)6K>' docs/60_DECISIONS/DECISIONS_LOG.md docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md STATUS.md`).
- The markers after the fix stage (9 in all): DECISIONS_LOG.md:52 `<P_CSV_PASS>` `<P_MUT_RESULT>`; N:946 `<P_TEST_PASS>`; N:947 `<P_MUT_RESULT>` `<P_APPLY_AFTER>`; N:948 `<P_CLOCK_AT>`; N:949 `<P_CSV_PASS>` `<P_ERRLINES>`; N:950 `<P_REVIEW>`. Fill each with the measured result (two test passes; mutation outcome; apply-check after the last revert; clock-at lines; csv_cmp result; error-line counts; review). `<P_APPLY_AFTER>` and `<P_CLOCK_AT>` are simply deleted once the runner / apply check confirms the sentence before each.
- Hash markers still open: `<H20K>` `<W20K>` `<REC20K>` `<HW6K>` `<WW6K>` (DECISIONS_LOG, N, STATUS) - from the runner, after the seed42 moves check of section 1.5.
- The commit message's mutation paragraph carries the same clause as N:947 on the knob-following filters (`UI.LeapBoundary`·`UI.VesselShown` - not rerun, [B-6b12]) and says the two dead README rows were marked; if it reuses the M template's rule wording, write '80번째 백분위를 1,000의 배수로 올려'.
