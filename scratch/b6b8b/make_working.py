"""[B-6b8b] build brief_working.md from brief_final.md: a head (drift, filled values, start-stage results, anchors) and the
brief's body with every re-derived line number put in place ([WC: ...] marks the working-copy changes).
usage: python -I -X utf8 make_working.py"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
body = open(os.path.join(HERE, "brief_final.md"), encoding="utf-8").read()
head = open(os.path.join(HERE, "working_head.md"), encoding="utf-8").read()

# {B6b8_BASELINE}: seed42.json at {H8B} (git show), the 'wrote' tail removed as run_checks.ps1's Remove-WrotePath does.
import json
import re
seed = json.load(open(os.path.join(HERE, "seed42_h8b.json"), encoding="utf-8"))
lines = []
for key in ("summary_line", "summary_line_20000", "ruler_summary_line", "ruler_summary_line_20000", "wanderer_summary_line"):
    lines.append("- `%s`: `%s`" % (key, re.sub(r", wrote .*?(?= \| |$)", "", seed[key])))
lines.append("- `verified_at`: `%s`; `last_recorded_in_changelog.task`: \"%s\"; hashes 3,000 %s / %s, 20,000 %s / %s" % (
    seed["verified_at"], seed["last_recorded_in_changelog"]["task"], seed["last_recorded_in_changelog"]["state_hash_3000"],
    seed["last_recorded_in_changelog"]["world_hash_3000"], seed["last_recorded_in_changelog"]["state_hash_20000"],
    seed["last_recorded_in_changelog"]["world_hash_20000"]))
assert head.count("{{BASELINE}}") == 1 and head.count("{{NE_TEXT}}") == 1
head = head.replace("{{BASELINE}}", "\n".join(lines))
ne = open(os.path.join(HERE, "ne_text.md"), encoding="utf-8").read().rstrip("\n")
head = head.replace("{{NE_TEXT}}", "```text\n" + ne + "\n```")

# (old, new, expected count) - every replacement must hit exactly the expected number of times.
R = [
    ("It is a Python copy of `TheodicyProblems` (`Tests/CosmicDataTests.cpp:2477-2568`).",
     "It is a Python copy of `TheodicyProblems` (`Tests/CosmicDataTests.cpp:2477-2568` at `cef8cc7`; `:2521-2612` at {H8B}, lambda body unchanged).", 1),
    ("- plague: 300 ticks, 0.5455;",
     "- plague: 300 ticks, 0.5455; [WC: at {H8B} 350 ticks, 35 slots, 0.4931 at both shares - `[B-6b7]`'s `edict_wide_warning` adds +50 to Hazard.Plague; the claim line is unchanged]", 1),
    # section 0.3
    ("the commandlet's `-WithoutB6bData` (`:1594/:1596`)", "the commandlet's `-WithoutB6bData` (`:1597/:1599` at {H8B})", 1),
    ("     - `CosmicDataTests.cpp:2716/:2779`;", "     - `CosmicDataTests.cpp:2782/:2845` ({H8B}; HEAD `:2716/:2779`);", 1),
    ("mutation patches and README `:800`, `schema.json:153`, `test_data_schema.py:1493`, and docs.",
     "mutation patches and README `:801`, `schema.json:178`, `test_data_schema.py:1500` ({H8B}), and docs.", 1),
    # section 0.4
    ("(`run_checks.ps1:139-155`)", "(`run_checks.ps1:140-156`)", 2),
    ("verbatim from {H8B} (`:562-567` at HEAD).", "verbatim from {H8B} (`:562-567` at HEAD; `:648-653` at {H8B}, text unchanged).", 1),
    # section 1
    ("- Schema `schema.json:153` (`min 0, max 1`) and `test_data_schema.py:1493` already exist",
     "- Schema `schema.json:178` (`min 0, max 1`) and `test_data_schema.py:1500` ({H8B}) already exist", 1),
    # section 2
    ("The panel reads only `GuardDelayTicksOf` (`UI/CosmicSystemPanelWidget.cpp:1098` at HEAD).",
     "The panel reads only `GuardDelayTicksOf` (`UI/CosmicSystemPanelWidget.cpp:1134` at {H8B}; `:1098` at HEAD).", 1),
    ("(`CosmicBalanceCommandlet.cpp:2676-2677`)", "(`CosmicBalanceCommandlet.cpp:2679-2680` at {H8B})", 1),
    ("**`Sim.FleetDelay`** (`Tests/CosmicDiplomacyTests.cpp`, HEAD line numbers):",
     "**`Sim.FleetDelay`** (`Tests/CosmicDiplomacyTests.cpp`, HEAD line numbers - [WC: the file is unchanged since `cef8cc7`; every number below holds at {H8B}]):", 1),
    ("  - The game run at `:2822` reads `GuardDelayCapOf` (`:2529`).",
     "  - The game run at `:2888` reads `GuardDelayCapOf` (`:2573`) ({H8B}; HEAD `:2822`/`:2529`).", 1),
    ("  - The board cases `:2948-2967` are unchanged", "  - The board cases `:3014-3033` ({H8B}; HEAD `:2948-2967`) are unchanged", 1),
    ("- `:2716` and `:2779`.", "- `:2782` and `:2845` ({H8B}; HEAD `:2716`/`:2779`).", 1),
    ("- `CosmicDiplomacyUITests.cpp:701`, which reads", "- `CosmicDiplomacyUITests.cpp:798` ({H8B}; HEAD `:701`), which reads", 1),
    ("as in `CosmicDeathTests.cpp:550`.", "as in `CosmicDeathTests.cpp:551` ({H8B}).", 1),
    # section 3
    ("(`:3700-3715`; every owner, Delay > 0)", "(`:3705-3720` at {H8B}; every owner, Delay > 0)", 1),
    ("(`:3819-3821`; the ruled people only)", "(`:3824-3826` at {H8B}; the ruled people only)", 1),
    ("At HEAD all five lists are empty.", "At HEAD all five lists are empty. [WC: at {H8B} too - {PRED} below: all five predicted bit-identical]", 1),
    ("(`run_checks.ps1:116-132`; the runner passes no `-Rules`)", "(`run_checks.ps1:115-132`; the runner passes no `-Rules`)", 1),
    ("The override is `OverrideRules` (`CosmicBalanceCommandlet.cpp:1365-1390`).",
     "The override is `OverrideRules` (`CosmicBalanceCommandlet.cpp:1368-1393` at {H8B}; the two log lines `:1383`/`:1386`).", 1),
    ("(`CosmicBalanceCommandlet.cpp:2835`)", "(`CosmicBalanceCommandlet.cpp:2838` at {H8B})", 1),
    ("(N §2.2 `:600`)", "(N §2.2 `:686` at {H8B})", 1),
    # section 4
    ("(N §2.3 `:613`)", "(N §2.3 `:699` at {H8B})", 1),
    ("(`CosmicBalanceCommandlet.cpp:1482`)", "(`CosmicBalanceCommandlet.cpp:1485` at {H8B})", 1),
    ("from `race:` (`:2691-2696`)", "from `race:` (`:2694-2699` at {H8B})", 1),
    # section 5
    ("(`CosmicDataTests.cpp:2957-2962`)", "(`CosmicDataTests.cpp:3023-3028` at {H8B})", 1),
    ("add \"also `Data.Integrity` (game theodicy)\" to its README row (`:791` at HEAD).",
     "add \"also `Data.Integrity` (game theodicy)\" to its README row (`:791` at HEAD; `:792` at {H8B} - `:791` is now `b6b2c_guard_cap_ignored`). [WC: confirmed with the probe arithmetic: 300+150+100+50 = 600, 60 slots, 0.976^60 = 0.2328 < 0.25; plague stays 350]", 1),
    ("Mark its README row (`:398` at HEAD)", "Mark its README row (`:398` at HEAD; `:399` at {H8B})", 1),
    # section 6
    ("Insert one line after the last §11 bullet (`:468` at HEAD), before `## 추가 이야기와 보완`.",
     "Insert one line after the last §11 bullet (`:468` at HEAD; `:473` at {H8B} = `[B-6b8]`), before `## 추가 이야기와 보완` (`:475` at {H8B}).", 1),
    ("    - ⓔ: `CosmicDataTests.cpp:2620-2622` → the 549/550 case (`:2957-2963`);",
     "    - ⓔ: `CosmicDataTests.cpp:2620-2622` → the 549/550 case (`:3023-3029` at {H8B});", 1),
    ("    - ⓔ: `:2288-2293` → the edict-bonus loop (`:2530-2535`);",
     "    - ⓔ: `:2288-2293` → the edict-bonus loop (`:2574-2579` at {H8B});\n"
     "    - [WC: ⓔ's plague: \"200 + 100 = 300틱 = 30슬롯 → 0.98^30 = 0.545 그대로\" → since `[B-6b7]` 200 + 100 + 경보망 50 = 350틱, 35슬롯, 0.98^35 = 0.4931 (ⓔ's own bound B ≤ 380 holds: B = 50)];", 1),
    ("    - H §2.10: panel `:1084` → `UI/CosmicSystemPanelWidget.cpp:1098`;",
     "    - H §2.10: panel `:1084` → `UI/CosmicSystemPanelWidget.cpp:1134` ({H8B}; `[B-6b4]` and later moved it past `:1098`);", 1),
    ("  - ④ `DECISIONS_LOG`'s B6b block ends at `:37`, not `:39`.",
     "  - ④ `DECISIONS_LOG`'s B6b block ends at `:37`, not `:39`. [WC: at {H8B} it ends at `:42` (`[B-6b8]`), the B6a heading is `:44`; write \"B6b 블록은 {H8B}에서 `:42`에서 끝난다\"]", 1),
    ("Add after the last B6b line:", "Add after the last B6b line (`:42` at {H8B}, `[B-6b8]`; HEAD `:37`):", 1),
    # order change, byte budget, rerun set
    ("- Run them serially, after the final runner, never alongside a commandlet.",
     "- Run them serially, after the final runner, never alongside a commandlet. [WC - ORDER CHANGE: serially BEFORE the one final runner, never alongside a commandlet or bundle K (drift 1)]", 1),
    ("6. **The one final runner:**", "6. [WC - ORDER CHANGE: this runs AFTER step 7's mutations, apply_after, README rows and the clean rebuild (drift 1)] **The one final runner:**", 1),
    ("7. **Mutations** (§5), serially.", "7. [WC - ORDER CHANGE: BEFORE step 6] **Mutations** (§5), serially.", 1),
    ("Measure the line with `wc -c`.", "Measure the line with `wc -c`. [WC: {H_BYTES0} = 148,661 and the long bit-identical line is 388 bytes -> 149,049: use the short line (drift 2)]", 1),
    ("At HEAD: `b6b2a_data_laws_knob_four`, `b6b2c_data_ark_margin_half` and `b4d2_data_resist_factor`.",
     "At HEAD: `b6b2a_data_laws_knob_four`, `b6b2c_data_ark_margin_half` and `b4d2_data_resist_factor`. [WC: at {H8B} runnable `b6b2a_data_laws_knob_four`, `b6b7_data_ark_margin_half`, `b6b7_data_upper_rung_not_binary`; not applying `b4d2_data_resist_factor` (since [B-6b2a]) and `b6b2c_data_ark_margin_half` (since [B-6b7], README `:793` says so) - drift 4]", 1),
    ("which is 9 patches at HEAD plus any additions.", "which is 9 patches at HEAD plus any additions. [WC: 9 runnable at {H8B} - drift 4]", 1),
]

missing = []
for old, new, n in R:
    got = body.count(old)
    if got != n:
        missing.append("%d x (want %d): %s" % (got, n, old[:90]))
        continue
    body = body.replace(old, new)
if missing:
    print("REPLACEMENTS THAT DID NOT HIT AS EXPECTED:")
    print("\n".join(missing))
    sys.exit(1)

out = head + "\n---\n\n# The brief (brief_final.md) with the working-copy line numbers put in place\n\n" + body
with open(os.path.join(HERE, "brief_working.md"), "w", encoding="utf-8", newline="\n") as f:
    f.write(out)
print("brief_working.md:", len(out.encode("utf-8")), "bytes;", len(R), "replacements")
