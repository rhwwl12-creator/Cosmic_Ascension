# [B-6b8b] brief - WORKING COPY (start stage done 2026-11-10; this copy wins over brief_final.md where they differ)

**FIX STAGE (2026-11-10, findings L1-1/L1-2/L1-3/L2-1/L2-3/L3-1/L3-2) - read `fix/handoff.md`.** Corrected below: the rerun count
(7 runnable, 4 dead - drift 4), the reign-tail wording (양위·폐위·죽음), the `Endless` sentence of §2, the N §5 block (①, 변이,
⑥ `crisis_and_transcendents.md:108`), the DECISIONS_LOG clause, STATUS `:32`/`:38`. Already applied in the tree by the fix stage:
`crisis_and_transcendents.md:108`, STATUS `:38` (rewritten whole - do not re-insert the brief's clause), README `:151/:152`.

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
   6. **mutations (§5), serially** - the 3 new patches and the 7 runnable reruns (below - drift 4), then `apply_after.txt`, README
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
4. **Mutation rerun set re-derived (FIX STAGE, L2-1: 7 runnable, 4 dead - not 9).** Runnable reruns, in this order:
   `B3/b3b_delay_on_rebellion`, `B3/b3b_delay_unguarded`, `B6b/b6b2c_guard_cap_ignored`, `B6b/b6b2c_guard_cap_no_fleet_cap`,
   `B6b/b6b2a_data_laws_knob_four`, `B6b/b6b7_data_ark_margin_half`, `B6b/b6b7_data_upper_rung_not_binary`. Not applying (not
   rerun, not retaken here): `B3/b3b_delay_ignores_gap` (since `[B-3c]` 3f76c66 - verified: applies at 3f76c66^, fails at
   3f76c66), `B3/b3b_delay_outside_cut` (since `[B-6b2a]` 73c1c72 - same check), `B6b/b6b2c_data_ark_margin_half` (since
   `[B-6b7]`), `B4/b4d2_data_resist_factor` (since `[B-6b2a]`). README `:151/:152` are already marked by the fix stage
   ("(no longer applies since [B-3c] / [B-6b2a] - not retaken; [B-6b12]'s whole-B6b rerun decides)"); `:399` (b4d2) is marked
   at the mutation stage with the same wording. So three `Sim.FleetDelay` assertions have no live mutation until `[B-6b12]`:
   the claim's tier-gap delay ('two rungs up is not put off' / 'one rung up, by half') and 'still cut at the int32 limit'.
   Details (as derived at the start stage; the two B3 rows below were wrongly counted as runnable there):
   - DT_Rules patches at {H8B} (`git grep -l 'b/Tools/data/DT_Rules.json' -- Tools/mutations`): `B6b/b6b2a_data_laws_knob_four`
     (applies), `B6b/b6b7_data_ark_margin_half` (applies, `[B-6b7]` retake of `b6b2c_data_ark_margin_half`),
     `B6b/b6b7_data_upper_rung_not_binary` (applies, `[B-6b7]` addition), `B6b/b6b2c_data_ark_margin_half` (**does not apply
     since `[B-6b7]`** - README `:793` already says so; not rerun), `B4/b4d2_data_resist_factor` (does not apply since
     `[B-6b2a]` - mark README `:399` as §5 says; not rerun).
   - [FIX] Of the 6 `Sim.FleetDelay` rows, `b3b_delay_ignores_gap` and `b3b_delay_outside_cut` FAIL in `apply_before.txt`
     (`:186`, `:188`) - 4 runnable, not 6.
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

- `summary_line`: `CosmicBalance: seed 42, 3000 ticks, hash 2557850423, world 2906170550`
- `summary_line_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 1957158585, world 3764704052`
- `ruler_summary_line`: `CosmicBalance: seed 42, 3000 ticks, hash 1189870756, world 3359656751 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 17, answers 17, started 5, focus changes 5, edict changes 5, diplomacy changes 17, decisions 49 (by 600 12, by 6000 -1, by end -1); told by 600 25, by 6000 -1; ruled people T2 at 1175, T3 at 2375`
- `ruler_summary_line_20000`: `CosmicBalance: seed 42, 20000 ticks, hash 1375149537, world 510604211 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 110, answers 110, started 41, focus changes 5, edict changes 18, diplomacy changes 17, decisions 191 (by 600 12, by 6000 90, by end 89); told by 600 25, by 6000 927; ruled people T2 at 1175, T3 at 2375`
- `wanderer_summary_line`: `CosmicBalance: seed 42, 6000 ticks, hash 1631315934, world 489794244 | CosmicBalance ruler: answer 1 build 1 focus 1 edict 1 diplomacy 1; cards 8, answers 8, started 2, focus changes 4, edict changes 3, diplomacy changes 2, decisions 19 (by 600 12, by 6000 19, by end 19); told by 600 25, by 6000 733; ruled people T2 at 1175, T3 at -1 | CosmicBalance wanderer: abdicate 1200 wander 1; left at 1201, crowned at -1 (None, tier 0, path none), journeys 191, scans 191, cards 7, answers 7, reverence peak 0.00, body 3, mind 1`
- `verified_at`: `2026-11-10`; `last_recorded_in_changelog.task`: "B6b B-6b7 (data 2 fourth to fifth: edict_wide_machine_accord/quarantine/warning, feat_laws_and_binding, project_binding P 1800, r 0.375)"; hashes 3,000 2557850423 / 2906170550, 20,000 1957158585 / 3764704052

`last_recorded_in_changelog.task` at {H8B} = "B6b B-6b7 (data 2 fourth to fifth: …)" (`[B-6b8]` was bit-identical and
kept it); in the bit-identical case it stays and only `verified_at` changes. The hash runs at {H8B}: `hashrun.log` =
input-less 2557850423 / 2906170550, `hashrun_policy.log` = ruler 1189870756 / 3359656751 (= the commandlet's 3,000-tick lines).

### `{NE_TEXT}` (N `:648-653` at {H8B}, verbatim - quote it unedited in the commit body)

```text
**ⓔ 지킴 지연의 상한과 W 다시 셈**(#132, 결정 23 — 이 셈을 `[B-6b8b]`의 커밋 메시지에 그대로 적는다).
- **지금**: 지연 = min(`FleetDelayPerPower` 50 × 유효 전력, `FleetDelayCap` 100)(`Sim/CosmicConflict.cpp:395-405`), 수도 규칙 행은 0(`:369`). 신정론의 W: 주장의 가장 긴 머묾 = 카운트다운 300 + 지킴 상한 100 + `edict_vigil`의 덤 100 + 카드 `petition_pressure_measured`의 지연 50 = **550틱** = 55슬롯(`EventSlotTicks` 10 — `DT_Rules.json:37`); 철야 추첨 행 `event_claim_eased` 무게 1.2 × `EventChancePerSlot` 0.02(`:38`) = 슬롯마다 0.024 → 남는 몫 0.976^55 = **0.2629** ≥ 0.25; W(그 행 무게의 상한) = (1 − 0.25^(1/55)) / 0.02 = **1.2445**. 왕좌의 잠금 600(`DT_Rules.json:18`) ≥ 550이라 카드가 한 주장에 두 번 오지 않는다(판의 경우 549/550 — `Tests/CosmicDataTests.cpp:2620-2622`).
- **몫 0.5**: 상한 = min(100, 0.5 × 카운트다운) — 각성 150 → **75**(50%), 역병 200 → 100(그대로), 초신성 전조 300 → 100(그대로, 33%), 주장 300 → 100(그대로, 33%), 반란(수도 규칙) 0. **주장의 550·W 1.2445·잠금 600·판의 경우 모두 그대로**. 역병(해결하는 비은혜 행 `event_plague_repentance` 무게 1.0 → 슬롯마다 0.02): 200 + 100 = 300틱 = 30슬롯 → 0.98^30 = 0.545 그대로. 각성·초신성 전조·반란은 해결하는 비은혜 이벤트 행이 없어 불변식이 건너뛴다(카드의 해결은 세지 않는다). `GuardDelayCapOf`를 불변식이 읽으면 각성의 셈은 느슨해지기만 한다.
- **각성 몫의 위 끝은 정확히 50%** — 띠의 위 끝과 같다: 몫 0.5에서 각성 지연 몫은 구성상 50%를 넘지 못하므로 '띠 안'은 확인일 뿐 깨질 수 있는 시험이 아니다(0.45면 67틱 — 44.7%, 0.4면 60 — 40%). B6a의 64.2%(위 끝 66.7%)를 보면 대부분 상한에 붙는다 — 몫 0.5에서 약 48~50%가 기대값이다.
- **은하 법칙의 덤**(결정 9): 법 행은 주장에 덤을 싣지 않는다(스키마) — W 그대로. 다른 태그의 덤은 불변식이 모든 칙령 행을 한 번씩 더하므로(`Tests/CosmicDataTests.cpp:2288-2293`) 행마다 한 번(합집합)의 은하 걸음과 같은 셈이다: 역병에 덤 B를 주면 (300 + B)가 68슬롯 이하 — **B ≤ 380**이어야 0.25를 지킨다. 같은 커밋에서 불변식을 다시 돌린다.
- **소모의 노브**(`PressureFleetAttrition`)는 잰 적이 없고 넷째 칸 이상에서 걸리지 않으며(§0.6′) AI는 지키지 않는다 — '일어나지 않는다'로 닫는다.
```

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

---

# The brief (brief_final.md) with the working-copy line numbers put in place

# [B-6b8b] implementation brief: #132 guard share 0 → 0.5, with W recounted (decision 23)

Everything below was checked read-only at HEAD `cef8cc7`. "{H8B}" means the tree this commit starts on (the `[B-6b8]` commit). I ran one Python probe outside the project at HEAD: `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b8b\w_recount.py`. It is a Python copy of `TheodicyProblems` (`Tests/CosmicDataTests.cpp:2477-2568` at `cef8cc7`; `:2521-2612` at {H8B}, lambda body unchanged). At share 0 and at share 0.5 it gives exactly N ⓔ's numbers:
- claim: 300+100+100+50 = 550, 55 slots, 0.2629 stays, W 1.2445;
- plague: 300 ticks, 0.5455; [WC: at {H8B} 350 ticks, 35 slots, 0.4931 at both shares - `[B-6b7]`'s `edict_wide_warning` adds +50 to Hazard.Plague; the claim line is unchanged]
- at share 0.3 the claim's longest stand is 540.

Scope: one data value, one test change, one header comment, doc sentences, the closing of OPEN_QUESTIONS #132, and the baselines recorded again (or confirmed bit-identical). No new draw, no saved field, `SaveVersion` stays 1, and no data row is turned on (H `:403`).

**Most likely outcome.** The five lines and both hash runs stay bit-identical. Every runner log in `Saved/Logs/checks*.log` on the current tree prints `delay bought per tag []` and `guarded crises 0 by tag []`. Plan for both outcomes; §3 decides which applies.

---

## 0. Start state (on {H8B}, before any agent runs)

1. **Earlier commits.**
   - `git log --oneline -6` must show `[B-6b4]`, `[B-6b5]`, `[B-6b6]`, `[B-6b7]` and `[B-6b8]`, and `[B-6b9]` only if bundle B called for it. If `[B-6b8]` is not committed, stop.
   - `git status` must be clean apart from the user's known local changes.
   - No UnrealEditor or UBT process may be running (`tasklist`).
2. **Re-derive every line number below on {H8B}.** These HEAD anchors will drift:
   - `DT_Rules.json:27`
   - `Tests/CosmicDiplomacyTests.cpp:1406-1512` (the B6b block `:1474-1510`)
   - `CosmicDataTests.cpp:2477-2568`, `:2822`, `:2948-2967`
   - `Data/CosmicDefinitionRows.h:988-994`
   - `Sim/CosmicConflict.cpp:366-412`
   - `UI/CosmicSystemPanelWidget.cpp:1098` (`[B-6b4]` edits the panel)
   - `World/CosmicRulerPolicy.cpp:711-780`
   - docs: `docs/20:289`, `docs/21:1199/:1518`, `docs/40:237/:366`, `systems.md:78`, `docs/30:30`
   - `OPEN_QUESTIONS.md:101`
   - `STATUS.md:7/:27/:32/:38/:40`
   - H §11's last bullet (`:468` at HEAD, before `## 추가 이야기와 보완` at `:470`) and N §5's end
   - the last B6b line of `DECISIONS_LOG` (`:37` at HEAD; `:39` is the B6a heading)
3. **Confirm that nothing touched the knob in between.** Save `git grep -n FleetDelayCountdownShare cef8cc7` (34 hits) as `<scratchpad>\b6b8b\hits_head.txt` and the same grep on {H8B} as `hits_h8b.txt`, then diff them.
   - The HEAD readers that matter are:
     - `CosmicConflict.cpp` (`GuardDelayCapOf`);
     - the commandlet's `-WithoutB6bData` (`:1597/:1599` at {H8B}) and its `race:` AtCap, which calls `GuardDelayCapOf`;
     - `CosmicTestHelpers.h` (`WithoutB6bData`, `:363`);
     - `CosmicDataTests.cpp:2782/:2845` ({H8B}; HEAD `:2716/:2779`);
     - `CosmicDiplomacyTests.cpp:1480-1507`.
   - The other hits are comments (`CosmicConflict.h:151`, `CosmicBalanceCommandlet.h:124`, `CosmicTestHelpers.h:351`), mutation patches and README `:801`, `schema.json:178`, `test_data_schema.py:1500` ({H8B}), and docs.
   - **Stop only on a new hit in a non-comment `.cpp`/`.h` line.** New doc or patch hits only need re-reading.
   - `DT_Rules` must still hold `0.0`.
4. **Fill these placeholders** from the tree at the start:
   - `{B6b8_BASELINE}`: the result of `git show HEAD:Tools/baselines/seed42.json`, using its keys `summary_line`, `summary_line_20000`, `ruler_summary_line`, `ruler_summary_line_20000`, `wanderer_summary_line` and `last_recorded_in_changelog`. The file has no hash-run key. The hash runs are compared by the runner's `Compare-HashRun` against the commandlet's own lines (`run_checks.ps1:140-156`).
   - `{H_BYTES0}`: `wc -c` of H.
     - H §8's own limit is "150,000바이트 안".
     - This brief adds a **self-imposed margin of 149,000**.
     - H was 146,814 bytes at HEAD, and the five data and presentation commits add about 1.9 KB, so the margin can be under 400 bytes (§6).
   - `{NE_TEXT}`: N §2 ⓔ, verbatim from {H8B} (`:562-567` at HEAD; `:648-653` at {H8B}, text unchanged). Earlier commits may have edited §2.
   - `{W_TABLE}`:
     - First check that `python -I <scratchpad>\b6b8b\w_recount.py Tools\data 0.0` on {H8B} gives the same claim line.
     - Then run `python -I <scratchpad>\b6b8b\w_recount.py Tools\data 0.5` on {H8B}.
     - It yields:
       - `{PLAGUE_BONUS_SUM}`: the plague's longest stand − 300; it must stay ≤ 380 per ⓔ;
       - `{CLAIM_BONUS}`: must stay 100;
       - `{CARD_DELAY_CLAIM}`: must stay 50;
       - `{SKIPPED_ROWS}`: at HEAD the skipped rows are awakening, pre-supernova and rebellion. Extinction is skipped unless some non-grace event resolves `Hazard.Extinction`. The ark is a megaproject, not an event.
     - The galaxy laws from `[B-6b7]` and `enc_witness` from `[B-6b8]` enter here.
     - If the claim line is not 550 / 0.2629, stop. An earlier commit changed W, and ⓔ's claim "W 그대로" needs re-reading before this commit.
   - `{PRED}`: the §3 prediction from {H8B}'s runner logs. It picks between the "moved" and "bit-identical" record variants of §6.
5. **Mutation apply state.** In a scratch copy of {H8B}, run `git apply --check` on every patch under `Tools/mutations/` and save the result as `<scratchpad>\b6b8b\apply_before.txt`.
6. **Clean build.** `git status` must be clean with no leftover mutation; earlier workflows end with mutation runs.

## 1. Data rows

No new rows and no texts. N §1.5 has nothing for this commit.

**`Tools/data/DT_Rules.json:27`** is the only data edit:
```json
		"FleetDelayCountdownShare": 0.5,
```
- Schema `schema.json:178` (`min 0, max 1`) and `test_data_schema.py:1500` ({H8B}) already exist, so 0.5 passes `check_data`. No schema change.
- The runner's `-Data` rebuilds the assets. Commit `DT_Rules.uasset` and `generated.sha1`.

## 2. Code, tests, constants

**No C++ logic change.**
- `GuardDelayCapOf` (`Sim/CosmicConflict.cpp:398-408`): 0.5 × 150 = 75 exactly in float.
- `FleetDelayOf` (`:366-396`) already takes min(own delay, row cap). It returns 0 for Capital-rule rows (the rebellion) at `:369`.
- `Place` (`Sim/CosmicHazards.cpp:219`) is the only path into the world.

The panel reads only `GuardDelayTicksOf` (`UI/CosmicSystemPanelWidget.cpp:1134` at {H8B}; `:1098` at HEAD). Its keys `Guard.Delay` and `Guard.DelayTip` (`ST_CA.csv:339-340`) carry no number, so the panel does not change.

**Which placements change.**
- Only a row with CountdownTicks < 200 has a cap below 100. In the data that is `crisis_ai_awakening` alone (150, `DT_Hazards.json:7`).
- Plague is 200, claim and pre-supernova 300, extinction 250 (cap min(100, 125) = 100). The rebellion gets 0 either way.
- The awakening row is reached only when the owner's `FleetTargetSystemID` is that system and 50 × FleetPower > 75, i.e. **FleetPower > 1.5**.

Two sources cannot reach it:
- The first crisis (tick 150, `Hazard.AIAwakening` on every capital, `GalaxySimulationSubsystem.cpp:2451-2474`) goes through `Place`. At that point the ruled people is still on the first rung, and `FleetPerSettledByTier[0]` is 0 (`DT_Rules.json:89`).
- AI peoples drop a guard on their own system in the AI stage (`:3005-3011`) and clear the target after a siege ends (`:2217`, `:2274`).

One edge to keep in mind:
- After an abdication, death or deposition (tenth stage), the former ruled people keeps its guard. [FIX: if a record cites where a deposition comes from, cite the rebellion row's `DeposeRuler` effect (`DT_Hazards.json:219`) and the tenth stage's `DeposeRuler()` call (`GalaxySimulationSubsystem.cpp:3312`), not `:3066` (only a check in `TickCrossings`).] No role path clears a guard on its own system; `PlayerRoleSubsystem.cpp:1189` clears only a besieging target.
- That guard lasts until its own AI-stage clear in the next tick, which runs after that tick's crisis stage.
- So an awakening placed in that one tick is capped for a non-ruled owner. The commandlet's `conflict:` "delay bought per tag" counts it, because it counts every owner. `pressure:` does not, because it counts only `bIsPlayerRuled` owners (`CosmicBalanceCommandlet.cpp:2679-2680` at {H8B}).

**Header comment** `Data/CosmicDefinitionRows.h:992` (ASCII):
```
	// 0..1 (schema). The C++ default 0 is off; DT_Rules holds 0.5 from [B-6b8b] (decision 23): the awakening's cap
	// 100 -> 75 (0.5 x 150); plague, pre-supernova, claim and extinction keep FleetDelayCap's 100; the rebellion (Capital rule) gains none.
```
This is a header edit, so build with the editor closed; the runner's `Build.bat` covers it. Mirror it in Korean in `docs/21:1199` (§6).

**`Sim.FleetDelay`** (`Tests/CosmicDiplomacyTests.cpp`, HEAD line numbers - [WC: the file is unchanged since `cef8cc7`; every number below holds at {H8B}]):
- File comment `:1406-1410`: add "B6b [B-6b8b] (decision 23): the game's share is 0.5 - the game's awakening row (150) gains 75 from fleet 3, 100 at the share's 0."
- `:1480` would fail at 0.5. Replace it with:
  `TestEqual(TEXT("The game's share is 0.5 ([B-6b8b], decision 23)"), Game.FleetDelayCountdownShare, 0.5f);`
- `:1481` keeps its value (100), but its label becomes false. Relabel it to:
  `TEXT("so the plague's 200 keeps FleetDelayCap's 100 (0.5 x 200 = 100)")`
- Insert right after `const FCosmicDataTables Kept = Sim->GetTables();` (`:1483`) and before `auto WithShare`. `Guard.FleetPower` is 3 and the target is system 1.
  ```cpp
  		// B6b [B-6b8b] (decision 23): the game's awakening row on the board - 150 + the game's share of it, 75, not FleetDelayCap's 100.
  		const FName Awake(TEXT("crisis_test_awake"));
  		const FHazardDef* GameAwakening = Original.FindHazardByTag(Tag(TEXT("Hazard.AIAwakening")));
  		if (TestNotNull(TEXT("(the game's awakening row)"), GameAwakening))
  		{
  			TestEqual(TEXT("(the game's awakening counts 150)"), GameAwakening->CountdownTicks, 150);
  			FHazardDef Row = *GameAwakening;
  			Row.SpawnChancePerTick = 0.f;
  			UDataTable* Copy = CosmicTests::CopyWithout<FHazardDef>(Kept.Hazards, {});
  			Copy->AddRow(Awake, Row);
  			FCosmicDataTables WithAwake = Kept;
  			WithAwake.Hazards = Copy;
  			Sim->SetTables(WithAwake);
  			TestEqual(TEXT("Fleet 3 adds the awakening the game's share of its 150, 75: 225"), PlaceOn(1, Awake), 225);
  			FCosmicRulesDef Zero = Game;
  			Zero.FleetDelayCountdownShare = 0.f;
  			Sim->SetTables(CosmicTests::WithRules(WithAwake, Zero));
  			TestEqual(TEXT("At a share of 0 the row's cap is FleetDelayCap's 100"), CosmicConflict::GuardDelayCapOf(Zero, 150), 100);
  			TestEqual(TEXT("so fleet 3 adds the awakening 100: 250, the bound before [B-6b8b]"), PlaceOn(1, Awake), 250);
  			Sim->SetTables(Kept);
  		}
  ```
  Why this shape:
  - It copies the table instead of calling `AddRow` on the board's own table (`CopyWithout`, `Tests/CosmicTestHelpers.h:222`; `FindHazardByTag`, `CosmicDataTables.h:97`).
  - `Place` does not check spawn conditions. [FIX L2-3 - the old reason ("`Endless` comes first, so tag lookups are unchanged") is false; do not copy it into N §5 or the commit body. Use:] No tag lookup runs while the copy is in place. `Place` reads the row by name and the delay takes the row itself. No tick runs, and the board is put back (`SetTables(Kept)`) before `WithShare`.
  - `LayBoard` copies the game's rules (`:134-154`), so the 0.5 pin sees the game's value.
  - The fleet-1 case from the map is dropped. It repeats the plague's fleet-1 case (250), and no mutation would break it.
  - The share-0 pair restores a guard on the "share 0 = FleetDelayCap" branch. Once the game's share is 0.5, nothing else in the suite checks that branch.
- All older assertions hold at 0.5: 200 / 250 / 300 / 200 / 200 / 6 / 300 / 325 / 50 / MAX_int32 / 200, and every `WithShare` case.

**Tests that do not change (verified at HEAD):**
- `Data.Integrity`'s theodicy check:
  - The game run at `:2888` reads `GuardDelayCapOf` (`:2573`) ({H8B}; HEAD `:2822`/`:2529`). The claim's cap stays 100, so 550 holds.
  - The board cases `:3014-3033` ({H8B}; HEAD `:2948-2967`) are unchanged: 1.2 passes, 549 is refused and 550 passes, 1.3 fails.
- `:2782` and `:2845` ({H8B}; HEAD `:2716`/`:2779`).
- Everything built on `CosmicTests::WithoutB6bData`, which zeroes the share (`CosmicTestHelpers.h:363`): `CosmicEndingTests.cpp:158`, `CosmicKnowingTests.cpp:1875`, `CosmicSuccessionTests.cpp:631`.
- `CosmicDiplomacyUITests.cpp:798` ({H8B}; HEAD `:701`), which reads `GuardDelayTicksOf`.

**Tests whose meaning could change.** These run the ruler policy with `-Diplomacy` on game data:
- `Sim.WandererLoop` (`CosmicWandererTests.cpp:1370-1386`; seed 4, 6,000 ticks, return path before 6,000).
- `CosmicPressureCardTests.cpp:713-815`.
- `Sim.HashRun` (`CosmicHashRunTests.cpp:201/228`). It compares two paths, not pinned numbers, so it is safe.

Rule for any other failure in the full pass:
1. Apply `b6b8b_data_share_off` (§5), rebuild the assets, and rerun that one test.
2. If it passes at share 0, add an `AddInfo` that names the guarded awakening (tick, FleetPower). Pin it again with a label citing `[B-6b8b]` and record it in N §5.
3. If it does not pass at share 0, stop: something besides the knob moved.

**Constants:** none. No `CosmicRows::` entry is needed; tests already name `crisis_ai_awakening` by literal, as in `CosmicDeathTests.cpp:551` ({H8B}).

## 3. Determinism and baselines

**Where the policy guards (an addition to H §5.2 `:334`).** H `:334` is correct: the knob reaches only awakenings on systems the policy guarded, and the parenthesis "R은 `-Guard=0`" only says that R does not run B4's `-Guard=1`. Two things are added:
- **The citation is stale.** At HEAD, `World/CosmicRulerPolicy.cpp:686-708` is the tail of ActEdicts. `ActDiplomacy` starts at `:711`, its role gate is at `:719`, and step (1), the guard of a besieged own system under `-Diplomacy=1 -Guard=0`, runs `:733-780`.
- **The wanderer run guards outside the first reign as well.** The gate checks only `Role == Ruler`, and the commandlet calls `Act` at every boundary (`CosmicBalanceCommandlet.cpp:2838` at {H8B}). So the run can guard before `-Abdicate=1200` and again after a crowning or return.
- Also add the one-tick tail after a reign ends (§2).
- Record these in N §5 and in the H §11 line.

**Prediction from {H8B}'s logs (no new runs).** For each of the five runs, read the runner's `checks_*.log`:
- `CosmicBalance conflict:` "delay bought per tag [...]" (`:3705-3720` at {H8B}; every owner, Delay > 0). This list decides the prediction.
- `pressure:` "guarded crises … by tag" (`:3824-3826` at {H8B}; the ruled people only). Use it for cross-reading.

Let the share-0 entry be `Hazard.AIAwakening=<sum>(<count>)`:
- Absent, or sum ≤ 75: the run is predicted bit-identical.
- sum > 75 × count: the run certainly moves.
- Otherwise: the run may move.

At HEAD all five lists are empty. [WC: at {H8B} too - {PRED} below: all five predicted bit-identical] Input-less runs must have no entry. Record the result as `{PRED}`.

**Breakable checks (on the new build):**
- **(a)** The input-less keys `summary_line` and `summary_line_20000` equal `{B6b8_BASELINE}` letter for letter, with the `wrote …` tail removed (`run_checks.ps1:76`). The runner's two steps `hash run: editor -game vs commandlet` and `… policy vs commandlet` must pass.
- **(b)** Five commandlet runs with the runner's exact arguments (`run_checks.ps1:115-132`; the runner passes no `-Rules`) plus `-Rules=FleetDelayCountdownShare:0` reproduce `{B6b8_BASELINE}` letter for letter.
  - The override is `OverrideRules` (`CosmicBalanceCommandlet.cpp:1368-1393` at {H8B}; the two log lines `:1383`/`:1386`). Each log must show `CosmicBalance: rules FleetDelayCountdownShare = 0` and no `-Rules cannot set`.
  - Use an absolute `-Out` into `Saved\Balance\b6b8b_share0\` and each run's own `-AbsLog`.
  - This proves the knob is the only change.
- **(c)** Optional (five runs): `-WithoutB6bData=1` still prints the `[B-6b5]` lines, because the switch zeroes the share. These are the lines quoted in the `[B-6b6]` brief §3.2 ⓐ, as carried in seed42.json's note.
- **(d)** A two-sided rule for each of the five pairs (new build vs its (b) twin). It reads the (b) twin's `conflict:` `Hazard.AIAwakening=<sum>(<count>)`:
  - (i) A pair that differs (any CSV or `_rows.csv` cell other than `TickMicros`, `SaveBytes` and `SaveRawBytes`, or either hash) must have the tag with sum > 75. A move without it means stop.
  - (ii) A pair whose twin has sum > 75 × count must differ.
  - (iii) A pair whose twin lacks the tag, or has sum ≤ 75, must be identical over the whole CSV and `_rows`, minus the three timing columns.
  - For each moved run, record `{FIRST_MOVED_TICK_<run>}` = the first differing sample. This is a record, not a check.
- **(e)** 0 `end check` and 0 `race check` lines in every log.

**Recording the baselines again.** The final runner `-Tier full -Data -UpdateBaseline` always rewrites `verified_at`, and rewrites the five lines when they moved.
- **If any run moved:**
  - Fill `{MOVED_RUNS}` and `{NEW_HASHES}` from the runner's output.
  - Edit `last_recorded_in_changelog` by hand (UTF-8, no BOM):
    - `task`: "B6b B-6b8b (#132: FleetDelayCountdownShare 0 -> 0.5, decision 23 - only a guarded awakening's cap moves, 100 -> 75)";
    - the hashes;
    - `note`: the reason and, for each moved run, its first moved tick. Keep "Older values: …".
  - The policy hash run moves exactly when ruler 3,000 moves (N §2.2 `:686` at {H8B}).
- **If nothing moved (the expected case):**
  - `last_recorded_in_changelog` stays as it is; only `verified_at` changes.
  - The records use the "bit-identical" variants (§6).
  - Check (b) and bundle K (§4) still run. Together with `Sim.FleetDelay`, they are the commit's only evidence about the world.

## 4. Measurement: bundle K (mandatory; runs while the review lenses read)

H §8 asks for no bundle. Bundle K is still mandatory: if the five lines do not move, it is the only measured evidence that the knob moves only guarded awakenings. It also gives counts for N §5 and for the bundle C hand-off (N §2.3 `:699` at {H8B}).

**Harness.** Copy `<scratchpad>\b6b6\bundleA.py` to `<scratchpad>\b6b8b\bundleK.py`:
- 6 workers;
- a sha1 guard on the module DLL and `DT_Rules.uasset`;
- the same fail rules.
- `bundleA.py:47` already passes `COMMON_ARGS = ["-Rules=ProvisionalEndTick:0"]`. `FParse::Value(…"Rules="…)` (`CosmicBalanceCommandlet.cpp:1485` at {H8B}) reads only the first `-Rules=`, so the twin's argument must be **one** `-Rules=ProvisionalEndTick:0;FleetDelayCountdownShare:0`, never a second `-Rules=`.
- The switch control becomes: every twin log has both `CosmicBalance: rules ProvisionalEndTick = 0` and `CosmicBalance: rules FleetDelayCountdownShare = 0`, and no `-Rules cannot set`. Every on-side log has the first and not the second.

**Runs.** Each run has a share-0 twin, all in `Saved\Balance\b6b8b_bundleK\`:
- R: 60 × 20,000 (the runner's ruler arguments, seeds 1-60);
- wanderer: 60 × 6,000 (the runner's wanderer arguments);
- input-less: 30 × 20,000.

That is 300 runs, about 12-15 minutes. Never run them alongside `Build.bat`, `make_data_tables` or a mutation.

**Judgments, written before the run:**
- **K1:** every input-less pair is identical (the whole CSV and `_rows` minus the three timing columns, plus both hashes), unless that twin's `conflict:` lists `Hazard.AIAwakening` with sum > 75. In that case apply risk 2. Any other difference means stop.
- **K2:** the two-sided rule of §3 (d)(i)-(iii) on every R and wanderer pair, read from each twin's `conflict:` per-tag list. That list counts every owner, so it also covers the one-tick tail after a reign ends.
- **K3 (record only):**
  - `{K_MOVED_R}/60` and `{K_MOVED_W}/60` moved pairs;
  - the awakening's `atcap/all` from `race:` (`:2694-2699` at {H8B}) at 0.5, with the cap now 75;
  - the ends of guarded awakenings (`pressure:` answered / expired / edict);
  - the twins' total `Hazard.AIAwakening` count.
  - The awakening's delay share is at most 75/150 = 50% by construction (N ⓔ), so a band test would be empty.
  - Bundle C reads the 48-50% expectation in the trial cell. If the share here is below 10% of the guarded awakenings' countdown, record "the guard lost its fleet" for bundle C.
  - If every twin's list is empty, record "K: 지킨 각성 0 — 노브가 닿는 판 없음". The judgment for bundle C stays with C.

## 5. Mutations

**New patches.** Each must fail before it gets a README row. Add both data names to the data-patch list at README `:5`.

| Patch | Edit | Breaks |
|---|---|---|
| `B6b/b6b8b_data_share_off.patch` | DT_Rules `0.5` → `0.0` (rebuild the assets after apply and after revert) | `Sim.FleetDelay` (the 0.5 pin; 225 becomes 250) |
| `B6b/b6b8b_data_share_third.patch` | `0.5` → `0.3` (rebuild the assets) | `Data.Integrity`: the claim's longest stand becomes 300+90+100+50 = 540 (probe), so "a throne lock of 549" reports nothing and its `Expect` fails. This shows that the W pin reads the game's share. `Sim.FleetDelay` breaks too. |
| `B6b/b6b8b_share_zero_is_a_share.patch` | `CosmicConflict.cpp:403` (HEAD; `:402` is the comment) `if (!(Share > 0.f))` → `if (Share < 0.f)` | `Sim.FleetDelay`: the new share-0 pair (cap 0, so 150 instead of 250). NaN < 0 is false, NaN × 150 = NaN, and `NaN < Cap` is false, so the result is the cap of 100. The NaN and below-0 cases therefore still pass, and the patch is specific to share 0. |

**Old patches to rerun.** This is the narrow reading of the standing rule (recommended = approved): rerun a patch if it patches a file this commit edits, or if its filter test is one this commit edits. Re-derive the set on {H8B} mechanically:
- `git grep -l 'b/Tools/data/DT_Rules.json' -- Tools/mutations`. At HEAD: `b6b2a_data_laws_knob_four`, `b6b2c_data_ark_margin_half` and `b4d2_data_resist_factor`. [WC: at {H8B} runnable `b6b2a_data_laws_knob_four`, `b6b7_data_ark_margin_half`, `b6b7_data_upper_rung_not_binary`; not applying `b4d2_data_resist_factor` (since [B-6b2a]) and `b6b2c_data_ark_margin_half` (since [B-6b7], README `:793` says so) - drift 4]
- The README rows whose Breaks column is `Sim.FleetDelay`. There are 6 at HEAD: `b3b_delay_on_rebellion`, `_unguarded`, `_ignores_gap`, `_outside_cut`, `b6b2c_guard_cap_ignored` and `b6b2c_guard_cap_no_fleet_cap`. [FIX: `_ignores_gap` (since `[B-3c]`) and `_outside_cut` (since `[B-6b2a]`) do not apply - 4 runnable; README `:151/:152` marked by the fix stage]
- Any `[B-6b6]`-`[B-6b8]` additions to either set.

No patch touches `CosmicDiplomacyTests.cpp` (0 at HEAD).

Expected results:
- `b6b2c_guard_cap_ignored` now also breaks the new 225, since 150 + 100 = 250.
- `b6b2c_guard_cap_no_fleet_cap` (the share's product alone) now also breaks **`Data.Integrity` (the game theodicy)**. Under the game's 0.5 the claim's cap becomes 150, the longest stand 600, 60 slots, and 0.976^60 ≈ 0.233 < 0.25. The 549 board case then reports 600, not 550 (`CosmicDataTests.cpp:3023-3028` at {H8B}). Confirm with `w_recount.py` before the run, and add "also `Data.Integrity` (game theodicy)" to its README row (`:791` at HEAD; `:792` at {H8B} - `:791` is now `b6b2c_guard_cap_ignored`). [WC: confirmed with the probe arithmetic: 300+150+100+50 = 600, 60 slots, 0.976^60 = 0.2328 < 0.25; plague stays 350]
- `b4d2_data_resist_factor` has not applied since `[B-6b2a]`. Its context has `"ResistFleetFactor": 0.5` without a comma, while HEAD `:102` has one. This commit did not break it. Mark its README row (`:398` at HEAD; `:399` at {H8B}) "(no longer applies since [B-6b2a] — not retaken; [B-6b12]'s whole-B6b rerun decides)" and note it in N §5.

**Retakes.** Retake every patch that goes from "applies" in `apply_before.txt` to "does not apply" in `apply_after.txt`, under a `b6b8b_` name. None is expected: the HEAD DT_Rules hunks are `:13-19` and `:97-103`, and the value edit at `:27` shifts no line.

**Running them.**
- Run them serially, after the final runner, never alongside a commandlet. [WC - ORDER CHANGE: serially BEFORE the one final runner, never alongside a commandlet or bundle K (drift 1)]
- Then save `apply_after.txt`.
- Then **rebuild both targets and the assets clean**, so that `[B-6b9]`/`[B-6b10]` start unmutated.

## 6. Docs and records

**docs/30.** Run `python -I Tools/make_config_reference.py`. `docs/30_CONFIG_REFERENCE.md:30` changes from `0.0` to `0.5`; the readers stay `Sim/CosmicConflict.cpp`. The runner's `--check` verifies it.

**Sentences this commit makes false** (fix them in the same commit; re-derive each on {H8B}):
- `docs/20_ARCHITECTURE.md:289`: "(B6b; 몫 `FleetDelayCountdownShare`는 `[B-6b8b]` 전까지 0 — 지금 셈)" → "(B6b; 몫 `FleetDelayCountdownShare` 0.5 — `[B-6b8b]`부터: 각성 150의 상한 75, 역병·초신성 전조·주장·절멸은 `FleetDelayCap` 100, 반란은 0)".
- `docs/21_DATA_MODEL_REFERENCE.md:1199`: "0 이하·수가 아니면 FleetDelayCap만(지금 셈). 장치 값 0(몫 없음 — 비트 그대로), 0.5는 `[B-6b8b]`(그 커밋이 W를 다시 셈한다)" → "0 이하·수가 아니면 FleetDelayCap만(B6b 전의 셈). C++ 기본 0(몫 없음), `DT_Rules` 0.5(`[B-6b8b]`부터 — 각성의 상한 100 → 75; 역병·초신성 전조·주장·절멸은 100, 반란은 0; W 1.2445·가장 긴 머묾 550 그대로)".
- `docs/21:1518`: "몫 0이면 지금 값" → "몫 0이면 B6b 전의 값".
- `docs/40_DEV_GUIDE.md:237`:
  - "(몫 0이면 `FleetDelayCap` — 지금 값)" → "(몫 0이면 `FleetDelayCap`; 게임의 몫은 `[B-6b8b]`부터 0.5 — 주장 300의 상한은 그대로 100)";
  - in the same row's longest-stand formula, "지킴의 `FleetDelayCap`" → "지킴의 `GuardDelayCapOf`".
- `docs/40_DEV_GUIDE.md:366` (the `Sim.FleetDelay` row):
  - "게임의 몫 0이면 `FleetDelayCap`(100, 지금 값)" → "몫 0이면 `FleetDelayCap`(100, B6b 전의 값)";
  - append "`[B-6b8b]`(결정 23): 게임의 몫 0.5의 핀(변이 `b6b8b_data_share_off`), 게임의 각성 행(150)을 판에 두면 함대 3이 75를 더해 225, 몫 0이면 250(변이 `b6b8b_share_zero_is_a_share`)".
- `docs/10_DESIGN/systems.md:78`: "(상한 100 — `FleetDelayPerPower`·`FleetDelayCap`;" → "(상한 100, 그 위기 행 카운트다운의 0.5까지 — 각성은 75; `FleetDelayPerPower`·`FleetDelayCap`·`FleetDelayCountdownShare`;". This file is outside N §1.2's list, but the commit makes the sentence false.
- **`OPEN_QUESTIONS.md:101` (the #132 row): delete it.** This follows N §0.2 `:195` ("**닫는다** … 결정 23 — `[B-6b8b]`") and CLAUDE.md ("결정이 바뀌면 … OPEN_QUESTIONS의 행을 지운다"), and the commit makes the row's "위 끝 66.7%" false.
  - The closure goes into the DECISIONS_LOG line below.
  - The drain side, as N §0.2 asks, gets one B7 line: "영향권 안 소모(#132에서 넘어옴 — 넷째 칸 이상에서 일어나지 않고 AI는 지키지 않는다; B7이 잰다)". Put it in docs/50's B7 entry. At HEAD, B7's work is named inside `docs/50_ROADMAP.md:12`; re-derive the place on {H8B}. If no B7 entry exists there, the DECISIONS_LOG line's "B7로" carries it and `[B-6b12]` places it.
  - The row's stale `CosmicDataTests.cpp:2234` citation goes with the row.
  - The #142 row (`:108` at HEAD) only cites "#132의 셈" for the 550, which stays true. Leave it.
- `STATUS.md:7`: "지킴 상한 `CosmicConflict::GuardDelayCapOf`(몫 `FleetDelayCountdownShare` 0)" → "(몫 `FleetDelayCountdownShare` 0.5 — `[B-6b8b]`부터, 각성의 상한 75)".
- `STATUS.md:38`: [FIX L3-2 - DONE in the tree; do not edit again] label "지킴(#132 — `[B-6b8b]`가 닫았다):", "밖이었다", the inserted clause shortened to "— 몫 0.5로 위 끝 75/150 = 50%", and "고친 길은 … 같은 커밋이 철야 무게 상한을 다시 셌다(W 1.2445·550 그대로); 영향권 안의 소모는 B7로(넷째 칸 이상에서 일어나지 않고 AI는 지키지 않는다)".
- `STATUS.md:40`: "#131·#132는 위의 2" → "#131은 위의 2(#132는 `[B-6b8b]`가 닫았다)".
- Unchanged and still true: `docs/21:1264/:1463/:1516-1517`, `crisis_and_transcendents.md:113` [FIX L3-1: `:108` was NOT still true - fixed by the fix stage, see N §5 ⑥], `docs/70:202`, `docs/20:212/:320`, `STATUS.md:8` (B6a history), and the B4/B6a history in `docs/50:12` and `DECISIONS_LOG`.
- Left to `[B-6b12]` per N §1.2: `docs/15_GLOSSARY.md:89` and the CHANGELOG. Say so in STATUS.

**H §11.** Insert one line after the last §11 bullet (`:468` at HEAD; `:473` at {H8B} = `[B-6b8]`), before `## 추가 이야기와 보완` (`:475` at {H8B}).

Moved variant:
> `- \`[B-6b8b]\`({DATE}): #132 몫 0.5(결정 23) — 각성의 상한만 100 → 75, W 1.2445·가장 긴 머묾 550·잠금 600 그대로; {MOVED_RUNS}. 더한 것: 정책의 지킴은 첫 통치에만 있지 않다 — 양위 전에도, 재즉위·복귀 뒤에도 다시 지킨다; 통치가 끝나면(양위·폐위·죽음) 옛 백성의 제 성계 지킴이 다음 틱 AI 단계까지 한 틱 남는다; §5.2의 인용 \`:686-720\` → \`ActDiplomacy\` (1) \`:733-780\`. 세부 노트 §5.`

Bit-identical variant: the same line with "{MOVED_RUNS}" replaced by "다섯 줄과 해시 판 둘 비트 그대로".

Measure the line with `wc -c`. [WC: {H_BYTES0} = 148,661 and the long bit-identical line is 388 bytes -> 149,049: use the short line (drift 2)] If `{H_BYTES0}` + its size ≥ 149,000, cut it to "- `[B-6b8b]`({DATE}): #132 몫 0.5(결정 23) — 각성만 75, W 그대로. 세부 노트 §5." (about 110 bytes) and move the rest to N §5.

**N §5 `[B-6b8b]` block** (in the form of the `[B-6b3b]` block):
- **나눔**: none.
- **데이터**: the one value.
- **코드**: the header comment only.
- **테스트**: the pin, the relabel, the awakening pair.
- **변이**: 3 new, the reruns with the expected double breaks; state the count "다시 돌린 것 7, 죽은 것 4" (dead: `b3b_delay_ignores_gap` since `[B-3c]`, `b3b_delay_outside_cut` since `[B-6b2a]`, `b6b2c_data_ark_margin_half` since `[B-6b7]`, `b4d2_data_resist_factor` since `[B-6b2a]`) and that the two B3 guards (the claim's tier-gap delay; the int32 cut) have no live mutation until `[B-6b12]` retakes them. Also record the order change (drift 1): mutations before the one final runner.
- **리뷰**: counts.
- **기준값**: old → new, or "비트 그대로"; `{MOVED_RUNS}`; (a)-(e); each moved run's `{FIRST_MOVED_TICK}`.
- **묶음 K**: K1-K3.
- **W 다시 셈**: `{W_TABLE}`.
- **헤더와 다르게 하거나 더한 것**:
  - ① The H `:334` citation `:686-720` → `ActDiplomacy` step (1) `:733-780`. [FIX L1-1/L1-2 - write in Korean exactly:] "정책의 지킴은 첫 통치에만 있지 않다 — 양위 전에도, 재즉위·복귀 뒤에도 다시 지킨다; 통치가 끝나면(양위·폐위·죽음) 옛 백성의 제 성계 지킴이 다음 틱 AI 단계까지 한 틱 남는다" (this is also the part of the H §11 line that moves to N - drift 2).
  - ② The fleet-1 case is dropped and the share-0 pair is added.
  - ③ Stale citations. Re-derive each on {H8B}:
    - ⓔ: `DT_Rules.json:37/:38/:18` → `:39/:40/:19`;
    - ⓔ: `CosmicConflict.cpp:395-405` → `GuardDelayTicksOf` `:411-`;
    - ⓔ: `CosmicDataTests.cpp:2620-2622` → the 549/550 case (`:3023-3029` at {H8B});
    - ⓔ: `:2288-2293` → the edict-bonus loop (`:2574-2579` at {H8B});
    - [WC: ⓔ's plague: "200 + 100 = 300틱 = 30슬롯 → 0.98^30 = 0.545 그대로" → since `[B-6b7]` 200 + 100 + 경보망 50 = 350틱, 35슬롯, 0.98^35 = 0.4931 (ⓔ's own bound B ≤ 380 holds: B = 50)];
    - H §2.10: `GuardDelayTicksOf` `:395-405` → `:411-`;
    - H §2.10: panel `:1084` → `UI/CosmicSystemPanelWidget.cpp:1134` ({H8B}; `[B-6b4]` and later moved it past `:1098`);
    - H §2.10: `FleetDelayOf` `:366-392` → `:366-396`.
  - ④ `DECISIONS_LOG`'s B6b block ends at `:37`, not `:39`. [WC: at {H8B} it ends at `:42` (`[B-6b8]`), the B6a heading is `:44`; write "B6b 블록은 {H8B}에서 `:42`에서 끝난다"]
  - ⑤ #132 is closed here (N §0.2 `:195`), not in `[B-6b12]`.
  - ⑥ [FIX L3-1] One sentence outside this brief's list was also made false and is fixed: `docs/10_DESIGN/crisis_and_transcendents.md:108` "…처음 쟀고 값은 그대로다(#132)." → "…처음 쟀고 B4는 값을 그대로 두었다; B6b `[B-6b8b]`(결정 23 — #132를 닫았다)부터 상한은 그 위기 행 카운트다운의 0.5까지라 각성만 75다(`systems.md` '함대는 시간을 산다')." (the earlier "(상한 +100)" in that line stays: it is still the outer cap). Also STATUS `:38` was rewritten past the brief's insertion (L3-2) and README `:151/:152` were marked dead (L2-1).

**DECISIONS_LOG.** Add after the last B6b line (`:42` at {H8B}, `[B-6b8]`; HEAD `:37`):

> "- `[B-6b8b]`({DATE} — 결정 23의 값, 헤더 §2.10, #132 닫음): 지킴의 지연 상한의 몫 `FleetDelayCountdownShare` 0 → 0.5. 상한 = min(`FleetDelayCap` 100, 0.5 × 행의 카운트다운) — 각성 150의 상한만 100 → 75로 내려가고 역병·초신성 전조·주장·절멸은 100 그대로(반란은 0). 신정론의 W 다시 셈: 주장의 가장 긴 머묾 550(300 + 100 + 철야 100 + 카드 50), 슬롯 55, 남는 몫 0.2629 ≥ 0.25, W ≤ 1.2445, 왕좌 잠금 600 그대로 {W_TABLE의 한 줄}. 영향권 안 소모의 몫은 '넷째 칸 이상에서 일어나지 않고 AI는 지키지 않는다'로 닫고 B7로. 입력 없는 판은 그대로(AI는 지키지 않는다); 정책 판은 공성 받는 제 성계를 지키는 동안(그리고 양위·폐위·죽음 뒤 그 백성의 다음 틱 AI 단계 전까지) 선 각성만 옮는다 — {MOVED_RUNS}, 새 해시 {NEW_HASHES} | 다섯 줄과 해시 판 둘 비트 그대로(시드 42의 판은 지킨 각성이 없다; 묶음 K {K_MOVED_R}/60·{K_MOVED_W}/60)."

Use the side of "|" that applies.

**STATUS.**
- `:27`: the new baseline facts and hashes, or "`[B-6b8b]`도 비트 그대로(시드 42의 판에 지킨 각성이 없다)".
- `:32`: `[B-6b8b]` done, with the `[B-6b12]` leftovers (glossary `:89`, CHANGELOG, and [FIX L2-1] the retake of the two dead B3 `Sim.FleetDelay` guards `b3b_delay_ignores_gap`·`b3b_delay_outside_cut` — 주장의 칸 차 지연과 int32 자름).
- `:7`, `:38` and `:40` as above.

**Commit message:**
- Title: `[B-6b8b] 지킴의 지연 상한 — 몫 0.5(결정 23·#132 닫음), 각성의 상한만 100 → 75, W 1.2445 그대로, Sim.FleetDelay에 각성 225, {MOVED_RUNS 요약 | 비트 그대로}`.
- Body, in order:
  1. `{NE_TEXT}` **verbatim**;
  2. the line "줄 번호(지금): …" with ③'s corrections (the quote itself is not edited);
  3. `{W_TABLE}`;
  4. the five lines old → new, or "비트 그대로";
  5. bundle K's K1-K3;
  6. the mutations;
  7. `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

**Theology.** No text is added. The doc sentences above use no verdict word and no number of people.

## 7. Order of work, parallelism, risks

**Order** (one lean ultracode workflow, effort high):
1. **§0:** the checks, the hit-list diff, `apply_before.txt`, the `{W_TABLE}` probe and `{PRED}`. The probe is Python only and can run at any time.
2. **Implementers, in parallel:**
   - **I1, data and docs:** `DT_Rules.json`, the comment in `CosmicDefinitionRows.h`, the doc sentences, the OPEN_QUESTIONS row and the B7 line, STATUS `:7/:38/:40`, and the `docs/30` regeneration. `python -I Tools/check_data.py` must report 0.
   - **I2, tests and patches:** `Sim.FleetDelay`, plus the three patch files (written but not run).
3. **Build pass:** `run_checks.ps1 -Tier smoke -Data -UpdateBaseline`, with the editor closed. Diff `seed42.json` against {H8B}: the input-less `summary_line` must be unchanged (the first half of check (a)).
4. **In parallel:**
   - **Machine:** check (b) (5 runs), the optional (c), then bundle K (§4), then (d) and (e) over all logs. These share one machine queue, so run them in sequence on it.
   - **Lenses** (read-only; each finding gets a rebut-verify):
     - L1, determinism and baselines: the H `:334` addition and the reign tail, (a)-(e), K1 and K2.
     - L2, tests and mutations: the label truth, the new assertions vs the three patches, the rerun set and the expected double breaks.
     - L3, docs and records: the false sentences, the #132 closure, the byte budget, the ⓔ quote, theology.
5. **Fix.** If a fix touches data or the sim, repeat steps 3-4.
6. [WC - ORDER CHANGE: this runs AFTER step 7's mutations, apply_after, README rows and the clean rebuild (drift 1)] **The one final runner:** `-Tier full -Data -UpdateBaseline`. Its two in-process passes are the two consecutive test runs. Its five lines must equal the build-pass lines and the on-side lines of bundle K.
7. [WC - ORDER CHANGE: BEFORE step 6] **Mutations** (§5), serially.
   - Draft the records (§6) in parallel, both variants; they touch only docs.
   - Then `apply_after.txt`, the README rows, and a clean rebuild of both targets and the assets.
8. `python -I Tools/check_docs.py`, `wc -c` on H, commit, push.

**Risks and resolutions** (recommended = approved):
1. **Another test fails in the full pass.** Use the §2 share-off probe. If it is explained, pin it again with a label. If it is not explained, stop.
2. **An input-less run moves** (K1 or (a)).
   - `FleetDelayOf` reads any owner's target. An AI could keep its own target through a crisis stage only through an edge, such as the one-tick tail after a reign or a transfer between its AI stage and the next crisis stage.
   - Do not change `FleetDelayOf` in this commit; that would add a new hash path (H `:403`).
   - If check (b) still holds (the knob is the only change) and the twin's `conflict:` lists the awakening, accept the move. Record the seed and tick, and correct "AI는 지키지 않는다" in N §5 and the DECISIONS_LOG line.
   - Otherwise stop.
3. **Header over 149,000 bytes** (the self-imposed margin under H §8's 150,000). Use the short H line and move the rest to N §5.
4. **`{W_TABLE}`'s claim line is not 550 / 0.2629 at {H8B}** (a law or card from `[B-6b7]` or `[B-6b8]` changed it). Stop before the edit and re-read ⓔ. `Data.Integrity` is the judge either way.
5. **Mutation rerun scope.** The `[B-6b6]` brief's mechanical rule ("Breaks test defined in an edited test file") would pull in about 119 `CosmicDiplomacyTests.cpp` rows (about 4-6 hours). Use the narrow rule instead: the filter test or the patched file, which is 9 patches at HEAD plus any additions. [WC/FIX: 7 runnable at {H8B}, 4 dead - drift 4] The whole-B6b rerun stays with `[B-6b12]`.
6. **Bundle K's cost or overlap.** If the lenses finish first, wait for the bundle; never start a build under it. If time is short, cut it to input-less 30 + R 30 + wanderer 30 pairs, never to zero. K1 and K2 keep their meaning.
7. **[개발자]** Nothing in this commit needs the developer. The B6a hand checks ①, ③ and ⑨ do not block it (as in `STATUS.md`).

## 추가 이야기와 보완

- `<scratchpad>\b6b8b\w_recount.py` reads the JSON and repeats the invariant's arithmetic, including the sphere factor. `[B-6b9]` to `[B-6b11]` can rerun it whenever a value commit touches `EventChancePerSlot`, `ThroneLockTicks`, the vigil bonus or a card delay, so the commit message can quote W without hand arithmetic. If it is ever committed under `Tools/`, `docs/40` needs one line.
- The commandlet's `conflict:` "delay bought per tag" already predicts which runs a hazard-cap knob can move. One sentence in `docs/40`'s "measurement switches" section, such as "a guard knob moves only runs whose share-0 log lists the tag with a delay above the new cap", would save the next knob commit the derivation. It could go into this commit's `docs/40:366` edit.
- `Guard.DelayTip` (`ST_CA.csv:340`) has no number, so the 75 is invisible to the player. That is consistent with rule 11. If a later presentation commit wants "위기마다 다르다", it is a text-only change.
- The guard that lasts one tick past the end of a reign (§2) is harmless and deterministic, but it is a guard nobody ordered. B7, which gives AI peoples their own guarding, could clear the target in the tenth stage when a reign ends. That would be a new hash path, so it is not done here.

---

## Critique dispositions

1. **Accepted.** I verified that `seed42.json` at HEAD has no hash-run key and that the hash runs go through `Compare-HashRun` (`run_checks.ps1:140-156`). (a) and §0.4 were rewritten.
2. **Accepted.** (d) and K2 were tautologies. They are replaced with the two-sided rule (i)-(iii) on the twin's `conflict:` `Hazard.AIAwakening=<sum>(<count>)`, and `{FIRST_MOVED_TICK}` is now only recorded.
3. **Accepted.** N §0.2 `:195` says "**닫는다** … `[B-6b8b]`", and CLAUDE.md requires deleting the row. The #132 row is now deleted in this commit, with a B7 line for the drain side and STATUS `:40` adjusted. N §5 item ⑤ records it.
4. **Accepted.** I verified that STATUS `:7` says "(몫 `FleetDelayCountdownShare` 0)" and `:38` says "위 끝 66.7%". Both are edited, and `:40` too (from finding 3).
5. **Accepted.** I verified that all five current `checks*.log` print `delay bought per tag []`. Bundle K is now mandatory, both record variants are drafted, the `verified_at`-only note is added, and `{PRED}` picks the variant.
6. **Accepted.** H `:334` is correct as written and its citation `:686-720` is stale (`ActDiplomacy` `:711`, gate `:719`, step (1) `:733-780`). "모자람" was dropped, and the item is now an addition plus a citation fix.
7. **Accepted as plausible.** No role path clears a guard on the people's own system (`PlayerRoleSubsystem.cpp:1189` clears only a besieging target), and the AI-stage clear `:3005-3011` runs after the crisis stage. The one-tick tail is added to §2, §3, the DECISIONS_LOG line and the H line. Attribution uses the `conflict:` list, which counts every owner.
8. **Accepted.** I verified 34 hits at HEAD, including comments, patches and docs. §0.3 is now a saved-list diff that stops only on a new hit in a non-comment `.cpp`/`.h` line.
9. **Accepted.** I verified that `:403` is the `if` and `:402` the comment. The patch row cites `:403`, and the NaN reasoning is included.
10. **Accepted.** Under the game's 0.5 the claim's cap of 150 gives 600 and 60 slots, with 0.976^60 below 0.25. The expected double break goes into §5 and the README row (`:791`), to be confirmed with the probe first.
11. **Accepted.** I verified "지금 셈" at `docs/21:1199`, "지금 값" at `:1518` and `docs/40:366`, and the `FleetDelayCap` term in `docs/40:237`'s formula. The rebellion's 0 comes from `FleetDelayOf` `:369`. All are fixed, and docs/20 now names the four rows that keep 100.
12. **Accepted.** The header comment now says "cap 100 -> 75 (0.5 x 150)", names the rows that keep 100, and says the rebellion gains none.
13. **Accepted.** H §8's limit is 150,000, and the 149,000 is now named as a self-imposed margin. The H line is inserted after the last §11 bullet (`:468`), before `## 추가 이야기와 보완` (`:470`).
14. **Accepted.** I verified H §2.10's stale `:395-405` and `:1084` and the panel's real `:1098`. They are added to ③ (with `:366-392`), and the brief's own panel citation is corrected.
15. **Accepted.** I verified that `bundleA.py:47` already passes `-Rules=ProvisionalEndTick:0` and that only the first `-Rules=` is read (`:1482`). The twin now joins both values with `;`, and the switch control requires both `rules … = 0` log lines.