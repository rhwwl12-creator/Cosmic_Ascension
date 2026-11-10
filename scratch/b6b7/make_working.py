"""[B-6b7] start stage, item 7: brief_working.md = brief_final.md with the placeholders filled and a header saying where each
value came from. LF, UTF-8 without BOM. Scratch only."""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "brief_final.md")
DST = os.path.join(HERE, "brief_working.md")

HEADER = r"""# [B-6b7] brief - WORKING COPY (start stage done 2026-11-10; this copy wins over brief_final.md where they differ)

Line 1 of brief_final.md (kept below) is a banner the harness put on the file when it was saved ('<' shown as '<\' in a few
prose placeholders such as <\tick>); it is not part of the brief and carries no instruction.

## Filled values and their sources

| Placeholder | Value | Source |
|---|---|---|
| HEAD (H''') | `6ae9060` [B-6b6] on `10205b6` [B-6b5], `529cab9` [B-6b4], `cef8cc7` [B-6b3b] | `git log --oneline -4`; `git diff` empty (the ~240 ' M' entries are OneDrive stat noise; `docs/OVERVIEW.html` untracked, not touched) |
| N section 1.5 | holds C4 `feat_laws_and_binding` (title, narrative, three LogTemplates) and C5 `edict_wide_machine_accord` / `_quarantine` / `_warning` (names, Summaries) | N lines ~500-510 at HEAD; letter for letter the texts in section 1.1/1.2 below (no fallback to merged.md) |
| `P_BIND_A` | **1800** (`RequiredProgress` stays `1800.0` - `DT_Megaprojects.json:67` unchanged, `DT_Megaprojects.uasset` not in the commit set) | N section 2 '`[B-6b6]`의 다시 셈': resampled at P_bind 1,800 (guard: not at 1,200); its row at r 0.375 = 9/30 = 0.30 [0.17, 0.48] (MC 0.287) - inside 15-35%; re-read by m_awake.py section 0.3: 9/30 = 0.300, MC 1725/6000 = 0.287 |
| `N_A` | **1200** (the feat's Key `"1200"`) | same paragraph: 'N = 1,200 그대로' |
| `R_A` | **0.375** (`DT_Rules.json:17` `1.0` -> `0.375`; off line '(was 0.375)') | same paragraph: 'r = 0.375 그대로' (no [개발자]) |
| t | 5,000 (n 30) - written there; [B-6b8]'s, not data | same paragraph |
| `M_AWAKE` | **0.5** | `m_awake.py` over `Saved\Balance\b6b_bundleA\` (R on, seeds 1-30): n 30, median k 5 (7 x 4, 23 x 5), a 1.2 in all 30, R_L 0 in 27 (323/370/485 in seeds 30/12/27); median S 1,800 at m 0.5, 0.375 and 0.25 (max 2,107.7 / 1,985.9 / 1,875.7) - largest m with median S <= 3,600. Output `scratchpad\b6b7\m_awake_out.md` (+ .json). Written into N section 2 as '`[B-6b7]`의 m' |
| #136 base ([B-6b6] A, R on) | by 6,000: **7/30 = 0.23 [0.12, 0.41]**; anywhere in 20,000: 18/30 = 0.60 [0.42, 0.75] (no record: the clock is off) | m_awake.py; curve 6,000 7 / 6,400 9 / 7,000 11 / 8,000 13 / 10,000 15 / 12,000 17 / 18,000-20,000 18 (of 30). Compare A' with the same two readings |
| T4* per baseline line ([B-6b6] = `Saved\Balance\b6b6_on\`, = `b6b7_base\`) | checks 3,000: **-1** (none); checks_ruler 3,000: **-1**; checks_wanderer 6,000: **3197** (AI civ_2; second AI civ_5 at 3362; ruled T4 none); checks_20000: **3149** (AI civ_8; second civ_3 at 3699; ruled none); checks_ruler_20000: **3651** (AI civ_6; second civ_10 at 3993; ruled T4 5168 civ_0) | race: parts 2-3 of the five logs |
| `LINES_MOVED_B6B7` | **can move: wanderer 6,000, input-less 20,000, ruler 20,000; cannot move: input-less 3,000, ruler 3,000, and both hash runs** (hashrun pairs with checks, hashrun_policy with checks_ruler) | T4* < horizon |
| Commit-title phrase | `기준값 방랑자 6,000·입력 없는 20,000·통치자 20,000 줄이 옮았다` (if all three move; drop any that stays) | m_awake.py section 0.5 |
| `AI_T5_MIN` | T4*_AI + 1 + ceil(1800 / 0.375) = T4*_AI + 4,801: wanderer **7,998** (> 6,000 horizon); 20,000 **7,950**; ruler 20,000 **8,452** | section 3.1 formula. All three lines record `Clock at 6000` at [B-6b6] (the ending line), and every AI_T5_MIN is above 6,000, so no record can be `OtherPeople` |
| `RULED_T5_MIN` | ruled T4 + 1,800 (>= ruled T4 + 1,200). Ruler 20,000: ruled T4 5168 at [B-6b6] -> 6,968 > 6,000. A `RuledPeople` record before the clock would need a ruled T4 <= 4,199 in the new world (ruled T4 cannot come before T4* 3651) | section 3.1 |
| Pre-stated ending kinds | all five lines: unchanged - 3,000 lines `None`, wanderer 6,000 / 20,000 / ruler 20,000 `Clock at 6000` (ruler 20,000: `RuledPeople` only if its ruled T4 moves to <= 4,199) | ending lines of b6b6_on |
| `PROBE_SEED_1` (candidate) | seed 42, wanderer run (runner args), second AI T4 at 3362 after civ_2 at 3197 -> `-Ticks=3462`; next candidates 42@3699 (input-less 20,000), 42@3993 (ruler 20,000). These are [B-6b6] ticks after T4*: re-read the climb tick on the [B-6b7] build (b6b7_on / A' logs) before probing | m_awake.py section 5 |
| `PROBE_SEED_2` | none in the five baseline logs ('not probed - no seed' unless A' gives one; m_awake.py --a1 looks) | m_awake.py section 5 |

## Item 6 - grep on H''' (`git grep -n "HasJudgedRowAt\|LawsReset\|RuleRung\|until a row reaches the fifth\|게임의 표는 거짓" HEAD -- Source docs`)

Matches the brief's list exactly for the pins: `Tests/CosmicChronicleScreenTests.cpp:955/960/962/964/966`, `World/CosmicGameMode.h:544`,
`docs/40_DEV_GUIDE.md:318`. The other hits are code that reads the helper or the label (no absence pin):
`UI/CosmicNextRung.cpp:35/323/351/372`, `.h:122/130/144-145`, `UI/CosmicTopBarWidget.cpp:222/547`, `.h:71-72`,
`Tests/CosmicThirdRungTests.cpp:849/982` (label reads), `docs/20:75/89`, `docs/21:1266/1782`, records in headers/notes/CHANGELOG.

Assertions and WithFeat users that change with section 2.2 (HEAD line numbers):
- `Tests/CosmicChronicleScreenTests.cpp:960` TestFalse "The game's tables judge no row up to the fifth rung yet" -> TestTrue (section 2.2).
- `:962`, `:964`, `:966` - `WithFeat(Sim->GetTables(), FifthFeat, …)` -> on `CosmicTests::WithoutB6bData(Sim->GetTables())` (section 2.2).
- `:531` `UI.RunEnd` - `WithFeat(CosmicTests::WithRules(Original, NoClock), FifthFeat, FifthRow)`: rerun; on a break build it on `WithoutB6bData`.
- `:955` (and `:948`, `:950`-`:952`, pure `ClockTipOf` calls) follow by themselves.
- `WithFeat` is defined at `:164` (the only file that uses it).
- No other test pins `Chronicle.RuleRung`/`Rung.LawsReset`/a fifth-rung title as absent on game tables.

Other tests the new rows reach (rerun, section 2.4; HEAD line numbers):
- `Tests/CosmicThirdRungTests.cpp` section 9b (`UI.NextRung`, `:742-966`): copies the game's edicts and `AddRow`s three `<law>` rows under the same
  `edict_wide_*` names - `UDataTable::AddRow` replaces a row of the same name (`Engine/Source/Runtime/Engine/Private/DataTable.cpp:519-535`,
  `RemoveRowInternal` then add), so the count stays 3; its own `DT_Feats` is a `NewTable`. Expected unchanged; rerun.
- `Tests/CosmicDataTests.cpp`: `LawProblems` `:2112`; the game check `:2149`; `LawEdicts = CopyOf` `:2168`; the Expect `:2182`; "Two laws … pass" `:2184`;
  `BindingProblems` `:2206`; `TheodicyProblems` `:2478` (game check `:2845`); the `WithoutB6bData` block `:2715-2734` (comment `:2716-2718`,
  pin `:2733-2734`); full copy from `:2724` (`FullRules` 0.375 `:2738`, `Expected` 1 `:2801`).
- `Tests/CosmicAscensionTests.cpp`: `Feats.Reachable` `:1760`; `Sim.PlayerAscendsToT3` `:1784` (`bEndOnTheWay` `:1849/:1853`, `:1857`, `:1858`, `:1860`);
  `Sim.StartBlockReasons` `:951`; `Sim.CivAIProjects` `:1160`.
- `Tests/CosmicUpperRungTests.cpp`: `LayBoard` `:322`; `Sim.ArkAnswers` `:1280` (sets r 0.25 at `:1294`); `Sim.UpperRungDelay` `:1331` (`:1349`);
  `Sim.Binding` `:1639`; the comment "(… game's binding row's values are [B-6b7]'s)" at `:1205-1206`.
- `Tests/CosmicSimTests.cpp:2268` `Sim.PhaseTwoExit`; `Tests/CosmicSettlementTests.cpp:1006` `Sim.RealmCap`; `Tests/CosmicHazardTests.cpp:2932` `Sim.NotablesCapped`;
  `Tests/CosmicEdictUITests.cpp:262` `UI.EdictWindow` (`EdictsOfTest` `:96`, used `:307`).
- `Tests/CosmicBalanceCommandlet.cpp`: comment `:537` ("the wiring's probe is [B-6b7]"); `RaceCap` `:2000`; `Each.Key == RaceRuledJudged` `:2010`;
  'quarantine held' in the `conflict:` line `:3715`.
- `Tests/CosmicB6bRowNames.h`: `:18` already reads "in the data from [B-6b6], [B-6b7]" ([B-6b6] did it); only `:28` "- [B-6b7]" -> "- in the data from [B-6b7]" is left.
- `Data/CosmicDefinitionRows.h:938-941` (the binary-exact note and the `1.f` default - unchanged).

## Item 7 - mutation apply state

`scratchpad\b6b7\apply_before.txt` (written by `scratchpad\b6b6\apply_state.py` on a `git archive HEAD` copy in
`scratchpad\b6b7\apply_head_copy\`): 1,047 patches, 904 apply, 143 do not - identical to [B-6b6]'s `apply_after.txt`.
`B6b/b6b2c_data_ark_margin_half` and `B6b/b6b2c_data_binding_causality` apply now (the first will stop with r's line; the
second keeps applying, `P_BIND_A` = 1800). No [B-6b6] patch has the end of `DT_Feats.json` as context: only
`b6b5_data_feat_says_bound` (hunk at :61) and `b6b5_data_feat_title_eternal` (:1) touch `DT_Feats.json`, both far from the end.

## Item 8 - base and pool

- No UnrealEditor / UBT / dotnet-UBT process (tasklist).
- `Saved\Balance\b6b6_on\`'s five whole key lines (Remove-WrotePath, with ruler / wanderer parts) equal `Tools/baselines/seed42.json` -> copied
  byte for byte to `Saved\Balance\b6b7_base\` (20 files; `scratchpad\b6b7\check_base.py`).
- `Saved\Balance\b6b_bundleA\` is there (R / input-less 30 + 30 pairs, wanderer 60 pairs, summary).
- `bundleA.py` has only `--cells` / `--max-seed` and always runs on/off pairs, so the section 4 pool is
  `scratchpad\b6b7\bundleA7.py` (imports bundleA.py; `--only A1,on,off`, `--seeds`, `--dry-run`, `--check-only`, `--r-a`,
  `--base`/`--on`): 37 jobs (A1 R_1-30 at 20,000 into `b6b7_A1\` with bundleA's command; `on` checks_20000 / checks_ruler_20000 into
  `b6b7_on\` with the runner's arguments; `off` the five runner runs + `-WithoutB6bData=1` into `b6b7_off\`), the 34 20,000-tick jobs
  first; it checks (a) the off line and the whole key vs `git show 10205b6:Tools/baselines/seed42.json`, (b) base vs on for Tick < T4*
  (whole run and key when no T4*), (c) 0 check lines, plus bundleA's fail rules; sha1 guard = dll + DT_* + ST_CA. Self-test
  `scratchpad\b6b7\bundleA7_selftest.py`: 15/15 (out: `bundleA7_selftest_out.txt`). J1-J4 and #136 on A' = `m_awake.py --a1 <b6b7_A1>`.
  Before (b): copy the build pass's `checks.*`, `checks_ruler.*`, `checks_wanderer.*` (csv, _rows, _claims, log) into `b6b7_on\`.

---

"""

REPL = [
    ('"Hazard.AIAwakening": <M_AWAKE>}', '"Hazard.AIAwakening": 0.5}'),
    ('"Key": "<N_A digits, e.g. 1200>"', '"Key": "1200"'),
    ("(각성 몫 <M_AWAKE>·", "(각성 몫 0.5·"),
    ("(N <N_A>)·결박 <P_BIND_A>·r <R_A>", "(N 1,200)·결박 1,800·r 0.375"),
    ("\"§5.3 ②의 한 걸음은 <R_A>에서\"", "\"§5.3 ②의 한 걸음은 <R_A>에서\" [R_A = 0.375: not needed]"),
    ("\"값 <R_A>(`[B-6b7]` — 묶음 A의 다시 셈)\"", "\"값 0.375(`[B-6b7]` — 묶음 A의 다시 셈)\""),
    ("feat_laws_and_binding(N <N_A>)·결박의 값(<P_BIND_A>)·r <R_A>", "feat_laws_and_binding(N 1,200)·결박의 값(1,800)·r 0.375"),
    ("<LINES_MOVED_B6B7 phrase>", "기준값 방랑자 6,000·입력 없는 20,000·통치자 20,000 줄이 옮았다"),
    ("AIUpperRungProgressMultiplier 1 (was <R_A as %g>)", "AIUpperRungProgressMultiplier 1 (was 0.375)"),
    ("- `RULED_T5_MIN` = ruled T4 + `P_BIND_A`,", "- `RULED_T5_MIN` = ruled T4 + `P_BIND_A` [= ruled T4 + 1,800; ruler 20,000 at [B-6b6]: 5168 + 1800 = 6,968],"),
    ("At 1,800 / 0.375 that is +4,801; at `R_A` 0.5, +3,601.",
     "At 1,800 / 0.375 that is +4,801; at `R_A` 0.5, +3,601. [Filled: wanderer 3197 -> 7,998; 20,000 3149 -> 7,950; ruler 20,000 3651 -> 8,452 - all above the 6,000 clock record]"),
    ("- `RequiredProgress` (:67): 1800.0 → `P_BIND_A` (a float with `.0`).",
     "- `RequiredProgress` (:67): 1800.0 → `P_BIND_A` (a float with `.0`). [P_BIND_A = 1800: no change, DT_Megaprojects not in the commit set]"),
    ("- `\"AIUpperRungProgressMultiplier\": 1.0` → `R_A`", "- `\"AIUpperRungProgressMultiplier\": 1.0` → `R_A` [= 0.375]"),
    ("   - `PROBE_SEED_1`: from the five baseline runs or A′,",
     "   - `PROBE_SEED_1` [candidate: seed 42 wanderer, second AI T4 3362 at [B-6b6] -> -Ticks=3462; re-read on the [B-6b7] build]: from the five baseline runs or A′,"),
    ("   - `PROBE_SEED_2`: a run where", "   - `PROBE_SEED_2` [none in the five baseline logs; look in A′]: a run where"),
]


def main():
    text = open(SRC, encoding="utf-8").read()
    for old, new in REPL:
        n = text.count(old)
        if n != 1:
            raise SystemExit("replacement source found %d times: %r" % (n, old[:80]))
        text = text.replace(old, new)
    with open(DST, "w", encoding="utf-8", newline="\n") as f:
        f.write(HEADER + text)
    print("wrote", DST)


if __name__ == "__main__":
    main()
