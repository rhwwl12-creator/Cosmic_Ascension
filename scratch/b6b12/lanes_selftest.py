"""Self-test of plan.py, lanes.py and apply_mid.py ([B-6b12] section 5). Single process, light: every build, editor,
commandlet and lane git call is mocked (SimEnv on a virtual clock); the only real processes are read-only git calls on
the project (ls-tree / cat-file of HEAD) and git on a throwaway repo under this folder (apply_mid). Nothing is written
outside <this folder>/selftest/, and C:\\CAL is never created.
usage: python -I -X utf8 lanes_selftest.py [--root <project root>]"""
import argparse
import glob
import json
import os
import shutil
import subprocess
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import apply_mid  # noqa: E402
import lanes  # noqa: E402
import plan as planmod  # noqa: E402

DEFAULT_ROOT = r"c:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
WORK = os.path.join(HERE, "selftest")
DASH = chr(0x2014)  # the README rows' dash
FAILS = []
PASSES = [0]


def check(cond, what):
    if cond:
        PASSES[0] += 1
    else:
        FAILS.append(what)
        print("  FAIL:", what, flush=True)


def section(name):
    print("--", name, flush=True)

# ------------------------------------------------------------------------------------------------ 1. plan on the real README


def test_real_plan(root):
    section("plan.py over the real README and patch files (HEAD, read-only)")
    tree = planmod.Tree(root, "HEAD")
    p = planmod.build_plan(tree)
    items = p["items"]
    check(all(it["kind"] in ("cpp", "asset", "python", "probe") for it in items), "every row has a kind")
    files = [x for x in tree.names(planmod.MUT_DIR) if x.endswith(".patch")]
    rows = {it["path"] for it in items}
    check(set(files) == (rows - set(p["missing"])) | set(p["orphans"]), "every patch file is a row or a listed orphan")
    check(not p["missing"], "every row has its patch file")
    check(not [o for o in p["orphans"] if "/B6b/" in o], "B6b: files and rows one to one")
    check(not p["problems"], "no planner problem: %s" % p["problems"][:3])
    by = {it["tag"]: it for it in items}
    expect = {"b6b6_data_ark_word_in_card": "python", "b6b6_data_extinction_expiry_effect": "asset",
              "b6b2a_data_reach_word_renamed": "asset", "b6a3a_probe_no_awareness_flag": "probe",
              "b4d1_schema_singing_star_by_the_hedge": "python", "b6b1_endured_any_sphere": "cpp"}
    for tag, kind in expect.items():
        if tag in by:
            check(by[tag]["kind"] == kind, "%s is %s (got %s)" % (tag, kind, by[tag]["kind"]))
    if "b6b3a_wanderer_gap_two" in by:
        check(by["b6b3a_wanderer_gap_two"]["live"], "a row that only mentions another patch's 'no longer applies' stays live")
    if "b6a4_clock_tip_reads_rung_rule" in by:
        it = by["b6a4_clock_tip_reads_rung_rule"]
        check(not it["live"] and not it["run"] and "touched" in it["sets"], "a marked older row is listed, not run")
    if "b4d1_schema_singing_star_by_the_hedge" in by:
        check(by["b4d1_schema_singing_star_by_the_hedge"]["where"] == "pool", "the touched B4 Python patch goes to the pool")
    by_name = sum(1 for it in items if ("_data_" in it["tag"]) != (it["kind"] == "asset"))
    print("   rows whose name-based routing ('_data_' -> assets, run_mutations2.py:48) disagrees with the README row: %d" % by_name)
    for l in planmod.summary(p):
        if not l.startswith("ORPHAN"):
            print("   " + l)
    print("   orphans (listed, no row): %d (%s)" % (len(p["orphans"]), ", ".join(sorted({o.split("/")[2] for o in p["orphans"]}))))
    # The working tree, read-only and informational (another workflow may be editing it right now).
    try:
        w = planmod.build_plan(planmod.Tree(root, None))
        print("   working tree: rows %d, to run %d, problems %d, orphans %d" %
              (len(w["items"]), sum(i["run"] for i in w["items"]), len(w["problems"]), len(w["orphans"])))
    except SystemExit as e:
        print("   working tree not readable as a plan right now:", e)
    return p

# ------------------------------------------------------------------------------------------------ 2. a synthetic project


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def patch_text(target, n=1):
    return "diff --git a/%s b/%s\n--- a/%s\n+++ b/%s\n@@ -%d,1 +%d,1 @@\n-a\n+b\n" % (target, target, target, target, n, n)


def make_synthetic(base):
    if os.path.exists(base):
        shutil.rmtree(base)
    S = "Source/Cosmic_Ascension/"
    tests = ["Sim.Alpha", "Sim.Beta", "UI.Chronicle", "Data.Integrity", "Sim.EndingOf", "Sim.Gamma"]
    write(os.path.join(base, S + "Tests/CosmicFakeTests.cpp"), "".join(
        'IMPLEMENT_SIMPLE_AUTOMATION_TEST(F%d, "Cosmic_Ascension.%s", CosmicTests::Flags)\n' % (i, t) for i, t in enumerate(tests)))
    write(os.path.join(base, "Tools/test_data_schema.py"),
          "class SchemaTests(unittest.TestCase):\n    def test_words(self):\n        pass\n    def test_rows(self):\n        pass\n")
    write(os.path.join(base, planmod.NOTES), "x\n**%s**:\n\n| t | a |\n|---|---|\n| `Sim.Gamma` | x |\n\nend\n" % planmod.DATA_ON_MARK)
    rows = []

    def add(folder, tag, target, breaks, desc="the mutation"):
        write(os.path.join(base, "Tools/mutations/%s/%s.patch" % (folder, tag)), patch_text(target))
        rows.append(("| `%s/` | `%s.patch` " + DASH + " %s | %s |") % (folder, tag, desc, breaks))

    A = S + "Sim/CosmicAlpha.cpp"
    for i in range(6):
        add("B6b", "b6b9_alpha_%d" % i, A, "`Cosmic_Ascension.Sim.Alpha`")
    for i in range(4):
        add("B6b", "b6b9_helper_%d" % i, S + "Tests/CosmicTestHelpers.h", "`Cosmic_Ascension.Sim.Beta` (case (%d))" % i)
    add("B6b", "b6b9_beta_two", S + "Sim/CosmicBeta.cpp", "`Cosmic_Ascension.Sim.Beta`, `Cosmic_Ascension.Sim.Alpha`")
    add("B6b", "b6b9_short_name", S + "Sim/CosmicBeta.cpp", "`Cosmic_Ascension.Sim.Beta` (and `Sim.Gamma`)")
    add("B6b", "b6b9_data_rows_gone", "Tools/data/DT_X.json", "`Cosmic_Ascension.Data.Integrity`")
    add("B6b", "b6b9_data_rows_twice", "Tools/data/DT_X.json", "`Cosmic_Ascension.Data.Integrity` (twice)")
    add("B6b", "b6b9_data_word_gone", "Tools/data/DT_Y.json", "`test_words` (`Tools/test_data_schema.py`)")
    add("B6b", "b6b9_schema_rule_off", "Tools/data/schema.json", "`python -I Tools/check_data.py` (rule 8)")
    add("B6b", "b6b9_schema_whole", "Tools/data/schema.json", "`Tools/test_data_schema.py`")
    add("B6b", "b6b9_probe_cap", S + "Sim/CosmicAlpha.cpp", "(probe " + DASH + " commandlet runs, no test)", "**commandlet probe**: the cap")
    add("B6b", lanes.WITNESS_SMOKE, S + "Sim/CosmicGamma.cpp", "`Cosmic_Ascension.Sim.Gamma` (the smoke)")
    add("B6b", "b6b9_stale_filter", S + "Sim/CosmicGamma.cpp", "`Cosmic_Ascension.Sim.Removed`")
    add("B6b", "b6b9_mentions_other", S + "Sim/CosmicGamma.cpp", "`Cosmic_Ascension.Sim.Gamma`",
        "the same as `B5/b5x_old`, which no longer applies since [B-6b3a]")
    add("B5", "b5x_chronicle_line", S + "UI/CosmicChronicleWidget.cpp", "`Cosmic_Ascension.UI.Chronicle`")
    add("B5", "b5x_chronicle_off", S + "UI/CosmicChronicleWidget.cpp", "`Cosmic_Ascension.UI.Chronicle`",
        "the line (no longer applies since [B-6b4] - taken again as `b6b4_x`)")
    add("B5", "b5x_gamma", S + "Sim/CosmicGammaOld.cpp", "`Cosmic_Ascension.Sim.Gamma`")
    add("B5", "b5x_unrelated", S + "Sim/CosmicOther.cpp", "`Cosmic_Ascension.Sim.Alpha`")
    add("B4", "b4x_schema_log", "Tools/data/DT_LogTemplates.json", "`test_rows` (`Tools/test_data_schema.py`)")
    write(os.path.join(base, "Tools/mutations/B1/b1x_orphan.patch"), patch_text(S + "UI/CosmicChronicleWidget.cpp"))
    readme = ("# Mutation patches\n\nA patch of `Tools/data` (the `b6b9_data_rows_` ones and `b6b9_nothing`) needs the assets rebuilt by `Tools/make_data_tables.py` instead of a build,\n"
              "after the apply and again after the revert. The `b6b9_schema_` ones and `b6b9_data_word_gone` are run by the Python tools alone (`python -I Tools/test_data_schema.py`,\n"
              "`python -I Tools/check_data.py`).\n\n| Folder | Patch | Breaks |\n|---|---|---|\n" + "\n".join(rows) + "\n")
    write(os.path.join(base, planmod.README), readme)
    diff = "".join("diff --git a/%s b/%s\n" % (f, f) for f in planmod.DEFAULT_TOUCHED)
    write(os.path.join(base, "phaseA.diff"), diff)
    return base


def test_synthetic_plan(base):
    section("plan.py on a synthetic README (kinds from rows, sets, groups, assertions)")
    p = planmod.build_plan(planmod.Tree(base, None), planmod.DEFAULT_TOUCHED)
    by = {it["tag"]: it for it in p["items"]}
    check(by["b6b9_data_rows_gone"]["kind"] == "asset", "README:5 prefix -> asset")
    check(by["b6b9_data_word_gone"]["kind"] == "python", "a '_data_' name listed at README:6 is Python, not asset")
    check(by["b6b9_schema_rule_off"]["py"]["check_data"], "check_data.py in the cell -> check_data")
    check(by["b6b9_schema_whole"]["py"]["whole"], "a cell naming only the file -> the whole file")
    check(by["b6b9_probe_cap"]["kind"] == "probe" and by[lanes.WITNESS_SMOKE]["kind"] == "probe", "probe rows")
    check(by["b6b9_beta_two"]["filters"] == ["Cosmic_Ascension.Sim.Beta", "Cosmic_Ascension.Sim.Alpha"], "two filters joined")
    check("Cosmic_Ascension.Sim.Gamma" in by["b6b9_short_name"]["filters"], "a short test name in the cell resolves")
    check(by["b6b9_helper_0"]["cost"] == "heavy" and by["b6b9_alpha_0"]["cost"] == "light", "heavy header class")
    check(by["b6b9_mentions_other"]["live"], "'which no longer applies' about another patch is not a mark")
    check(by["b5x_chronicle_line"]["sets"] == ["touched"] and by["b5x_chronicle_line"]["run"], "older touched: Phase A file")
    check(not by["b5x_chronicle_off"]["run"] and by["b5x_chronicle_off"]["sets"], "older marked row listed, not run")
    check("meaning" in by["b5x_gamma"]["sets"], "older meaning: N section 1.3's table test")
    check(not by["b5x_unrelated"]["sets"] and not by["b5x_unrelated"]["run"], "older unrelated row not in the set")
    check(by["b4x_schema_log"]["where"] == "pool" and "touched" in by["b4x_schema_log"]["sets"], "older touched Python -> pool")
    check(any("stale filter Cosmic_Ascension.Sim.Removed" in n for n in p["notes"]), "stale filter logged, not fatal")
    check(any("b6b9_nothing" in n for n in p["notes"]), "a README list entry naming no row is noted")
    check(p["orphans"] == ["Tools/mutations/B1/b1x_orphan.patch"], "orphan listed")
    check(any("b1x_orphan" in n and "Phase A" in n for n in p["notes"]), "an orphan on a Phase A file is noted")
    keys = [g["key"] for g in p["groups"]]
    check(keys[0].endswith("CosmicTestHelpers.h"), "largest group first: the heavy header's four (%s)" % keys[0].split("/")[-1])
    check([g["est"] for g in p["groups"]] == sorted([g["est"] for g in p["groups"]], reverse=True), "groups sorted by cost")
    xg = [g for g in p["groups"] if g["key"] == "Tools/data/DT_X.json"]
    check(len(xg) == 1 and xg[0]["kind"] == "asset" and len(xg[0]["tags"]) == 2, "asset items grouped by their JSON file")
    # assertions: a B6b file without a row, a missing method
    write(os.path.join(base, "Tools/mutations/B6b/b6b9_rowless.patch"), patch_text("Source/x.cpp"))
    readme = open(os.path.join(base, planmod.README), encoding="utf-8").read()
    write(os.path.join(base, planmod.README), readme + "| `B6b/` | `b6b9_schema_no_method.patch` " + DASH + " x | `test_nothing_here` (`Tools/test_data_schema.py`) |\n")
    write(os.path.join(base, "Tools/mutations/B6b/b6b9_schema_no_method.patch"), patch_text("Tools/data/schema.json"))
    p2 = planmod.build_plan(planmod.Tree(base, None), planmod.DEFAULT_TOUCHED)
    check(any("b6b9_rowless" in x for x in p2["problems"]), "a B6b patch file with no row is a problem")
    check(any("test_nothing_here" in x for x in p2["problems"]), "a missing schema method is a problem")
    os.remove(os.path.join(base, "Tools/mutations/B6b/b6b9_rowless.patch"))
    os.remove(os.path.join(base, "Tools/mutations/B6b/b6b9_schema_no_method.patch"))
    write(os.path.join(base, planmod.README), readme)
    return p

# ------------------------------------------------------------------------------------------------ 3. verdicts


REAL_KILLED = """[2026.11.09-15.55.33:479][318]LogAutomationController: Error: Test Completed. Result={Fail} Name={RaceMeasure} Path={Cosmic_Ascension.Sim.RaceMeasure}
[2026.11.09-15.55.33:479][318]LogAutomationController: Error: Expected 'and every other field of the home row is the game's: the same world, the cap alone differs' to be true. [C:\\Users\\rhwwl\\OneDrive\\Documents\\Unreal Projects\\Cosmic_Ascension\\Source\\Cosmic_Ascension\\Tests\\CosmicRaceMeasureTests.cpp(1)]
[2026.11.09-15.55.33:480][319]LogAutomationCommandLine: Display: **** TEST COMPLETE. EXIT CODE: -1 ****
"""
REAL_PASS = """[2026.11.08-12.49.20:004][379]LogAutomationController: Display: Test Completed. Result={Success} Name={DeathPipeline} Path={Cosmic_Ascension.Calling.DeathPipeline}
[2026.11.08-12.49.20:010][382]LogAutomationController: Display: Test Completed. Result={Success} Name={FirstTickOfLife} Path={Cosmic_Ascension.Calling.FirstTickOfLife}
[2026.11.08-12.49.20:104][386]LogAutomationController: Display: Test Completed. Result={Success} Name={LeapDeterminism} Path={Cosmic_Ascension.Calling.LeapDeterminism}
[2026.11.08-12.49.20:110][387]LogAutomationCommandLine: Display: **** TEST COMPLETE. EXIT CODE: 0 ****
"""
MAP_ONLY = """[2026.11.10-01.00.00:000][  9]LogAutomationController: Error: Test Completed. Result={Fail} Name={ContinueEnded} Path={Cosmic_Ascension.UI.ContinueEnded}
[2026.11.10-01.00.00:000][  9]LogAutomationController: Error: The map did not load again within 15 seconds.
[2026.11.10-01.00.00:000][  9]LogAutomationController: Display: Test Completed. Result={Success} Name={Chronicle} Path={Cosmic_Ascension.UI.Chronicle}
[2026.11.10-01.00.00:000][  9]LogAutomationCommandLine: Display: **** TEST COMPLETE. EXIT CODE: -1 ****
LogAutomationController: Error: some later line outside a test
"""


def real_log_lines(root):
    """A few real lines from Saved/Logs/Mutation_*.log, read only, if present."""
    out = []
    try:
        logs = sorted(glob.glob(os.path.join(root, "Saved", "Logs", "Mutation_b6b*.log")))[:8]
    except Exception:  # noqa: BLE001
        return out
    for path in logs:
        try:
            text = open(path, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        sel = [l for l in text.splitlines() if "Test Completed" in l or "LogAutomationController: Error:" in l or "TEST COMPLETE" in l]
        if sel:
            out.append((os.path.basename(path), "\n".join(sel[:12] + [l for l in sel[12:] if "TEST COMPLETE" in l])))
        if len(out) >= 6:
            break
    return out


def test_verdicts(root):
    section("verdict parser (run_mutations2.py's reading, with the map-reload rule)")
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(REAL_KILLED))
    check(v == "KILLED" and e and "Expected" in e[0], "real KILLED excerpt")
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(REAL_PASS))
    check(v == "SURVIVED", "real all-Success excerpt -> SURVIVED")
    v, rr, e = lanes.verdict_of(lanes.parse_test_log("LogInit: nothing ran\n"))
    check(v == "NO RESULT" and not rr, "no Test Completed line -> NO RESULT")
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(MAP_ONLY))
    check(v == "NO RESULT" and rr == ["Cosmic_Ascension.UI.ContinueEnded"], "map reload alone -> NO RESULT, rerun that test alone")
    mixed = REAL_KILLED.replace("**** TEST COMPLETE. EXIT CODE: -1 ****", "x") + MAP_ONLY
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(mixed))
    check(v == "KILLED", "map reload plus a real failure -> KILLED")
    both = MAP_ONLY.replace("within 15 seconds.\n", "within 15 seconds.\n[x]LogAutomationController: Error: Expected 'y' to be 1, but it was 2.\n", 1)
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(both))
    check(v == "KILLED", "a failed test with the map reload and another error -> KILLED")
    check(lanes.strip_wrote("race: a 1 b 2, wrote C:\\CAL\\l1\\Saved\\x.csv | ruler 3") == "race: a 1 b 2 | ruler 3", "Remove-WrotePath form")
    for name, text in real_log_lines(root):
        v, rr, e = lanes.verdict_of(lanes.parse_test_log(text))
        old = "KILLED" if "Result={Fail}" in text else ("SURVIVED" if "Result={" in text else "NO RESULT")
        check(v == old or (v == "NO RESULT" and rr), "real log %s: %s (run_mutations2 reads %s)" % (name, v, old))
        print("   real log %s -> %s" % (name, v))
    section("Python pool kill rule (section 5.6)")
    spec = {"methods": ["test_words"], "check_data": False, "whole": False}
    out = "FAIL: test_words (__main__.SchemaTests.test_words)\n----\nRan 1 test\n\nFAILED (failures=1)\n"
    check(lanes.py_verdict(spec, [("schema", 1, out)])[0] == "KILLED", "FAILED (failures=) naming the row's method -> KILLED")
    out2 = out.replace("test_words", "test_other")
    check(lanes.py_verdict(spec, [("schema", 1, out2)])[0] == "NO RESULT", "failures only in another method -> NO RESULT")
    out3 = "ERROR: test_words (__main__.SchemaTests.test_words)\nTraceback...\nRan 1 test\n\nFAILED (errors=1)\n"
    check(lanes.py_verdict(spec, [("schema", 1, out3)])[0] == "NO RESULT", "errors= alone -> NO RESULT until the lead reads it")
    check(lanes.py_verdict(spec, [("schema", 0, "Ran 1 test\n\nOK\n")])[0] == "SURVIVED", "OK -> SURVIVED")
    cd = {"methods": [], "check_data": True, "whole": False}
    check(lanes.py_verdict(cd, [("check_data", 1, "DT_X.row: rule 8\ncheck_data: 1 problem(s)\n")])[0] == "KILLED", "check_data's rule line -> KILLED")
    check(lanes.py_verdict(cd, [("check_data", 0, "check_data: 0 problem(s)\n")])[0] == "SURVIVED", "check_data clean -> SURVIVED")
    check(lanes.py_verdict(cd, [("check_data", 1, "Traceback\nKeyError\n")])[0] == "NO RESULT", "check_data crash -> NO RESULT")
    whole = {"methods": [], "check_data": False, "whole": True}
    check(lanes.py_verdict(whole, [("schema", 1, out2)])[0] == "KILLED", "the whole file: any FAIL -> KILLED")
    check(lanes.schema_cmd({"methods": ["test_words"], "classes": {"test_words": "SchemaTests"}, "whole": False})[-1] == "SchemaTests.test_words", "named method command")

# ------------------------------------------------------------------------------------------------ 4. the lanes, mocked


class Capture:
    env = None


def run_sim(plan_path, out, lanes_n=3, opts=None, extra=None):
    if os.path.exists(out):
        shutil.rmtree(out)
    cap = Capture()

    def factory(*a, **k):
        cap.env = lanes.SimEnv(*a, **k)
        return cap.env
    argv = ["--plan", plan_path, "--lanes", str(lanes_n), "--out", out] + (extra or [])
    import io
    import contextlib
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        run = lanes.main(argv, env_factory=factory, sim_opts=opts or {})
    return run, cap.env, buf.getvalue()


def results(out):
    return lanes.load_results(out)


def test_lanes(p, base):
    plan_path = os.path.join(WORK, "plan_syn.json")
    json.dump(p, open(plan_path, "w", encoding="utf-8"), ensure_ascii=False)
    tags_lanes = [t for g in p["groups"] for t in g["tags"]]
    planned = [it["tag"] for it in p["items"] if it["run"]]
    smoke = lanes.WITNESS_SMOKE
    probes = os.path.join(WORK, "probes.json")
    json.dump({"b6b9_probe_cap": {"runs": [{"patches": ["self"], "args": ["-Seed=7", "-Ticks=1100", "-Out={LANE}/Saved/p.csv"],
                                              "expect": [r"race check: at \d+ .* past the home cap"], "exit": 1}]}},
              open(probes, "w", encoding="utf-8"))

    section("dry run, 3 lanes: schedule (whole groups, shared queue), one item, end checks, probes, merge")
    opts = {"verdicts": {"b6b9_alpha_3": "SURVIVED", "b6b9_beta_two": "MAPRELOAD"},
            "refuse": ["b6b9_data_rows_twice"], "existing": [lanes.BUNDLE_B],
            "cmdlet_lines": {"b6b9_probe_cap": ["CosmicBalance race check: at 1050 people civ_3:a past the home cap 1, wrote C:/CAL/l1/Saved/p.csv", "summary"]},
            "cmdlet_rc": {"b6b9_probe_cap": 1}}
    out = os.path.join(WORK, "run3")
    run, env, text = run_sim(plan_path, out, 3, opts, ["--probes", probes])
    res = results(out)
    check(all(t in res for t in planned), "every planned item has a verdict (%d of %d)" % (sum(t in res for t in planned), len(planned)))
    lane_of = {t: res[t].get("lane") for t in tags_lanes}
    for g in p["groups"]:
        check(len({lane_of[t] for t in g["tags"]}) == 1, "group %s ran whole on one lane" % g["key"].split("/")[-1])
    order = [json.loads(l)["tag"] for l in open(os.path.join(out, "results.jsonl"), encoding="utf-8") if json.loads(l)["phase"] == "lane"]
    for ln in ("l1", "l2", "l3"):
        mine = [t for t in order if lane_of.get(t) == ln]
        keys = []
        for t in mine:
            k = p["groups"][[i for i, g in enumerate(p["groups"]) if t in g["tags"]][0]]["key"]
            if not keys or keys[-1] != k:
                keys.append(k)
        check(len(keys) == len(set(keys)), "%s took its groups whole and in turn" % ln)
    first_by_lane = {}
    for t in order:
        first_by_lane.setdefault(lane_of[t], t)
    check(any(p["groups"][0]["tags"][0] == t for t in first_by_lane.values()), "the largest group is taken first")
    check(res["b6b9_alpha_3"]["verdict"] == "SURVIVED" and res["b6b9_alpha_0"]["verdict"] == "KILLED", "KILLED / SURVIVED per item")
    check(res["b6b9_beta_two"]["verdict"] == "KILLED" and any("rerun alone" in n for n in res["b6b9_beta_two"]["notes"]),
          "a map-reload NO RESULT is rerun alone, then judged")
    check(res["b6b9_data_rows_twice"]["verdict"] == "KILLED" and "assets after revert: written" in res["b6b9_data_rows_twice"]["notes"],
          "a make_data refusal counts as KILLED; the assets are rebuilt after the revert")
    check("assets: written" in res["b6b9_data_rows_gone"]["notes"], "an asset item rebuilds the assets, then runs its filter")
    check(res["b6b9_probe_cap"]["verdict"] == "KILLED", "a commandlet probe with its expected line and exit code (', wrote' stripped)")
    check(res[smoke]["verdict"] == "KILLED", "the witness smoke: a problem in the mutant run and none in the clean run")
    rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    check(rec["lane_flags"] == "-WaitMutex -DisableUnity", "non-unity flags kept when the gate builds")
    check(all(l["end"].get("clean_build") == "Result: Succeeded" and l["end"].get("status", "").startswith("equal") for l in rec["lanes"]),
          "end of every lane: clean build and status equal to phaseA.diff's files")
    gits = [c for c in env.calls if c[0] == "git"]
    lane_gits = [c for c in gits if os.sep + "sim_CAL" + os.sep in c[1] or c[2][4:6] == ("worktree", "add")]
    check(lane_gits and all(c[2][:4] == tuple(lanes.GITC) for c in lane_gits), "every lane git call carries -c core.eol=lf -c core.autocrlf=false")
    pool_gits = [c for c in gits if os.sep + "py" + os.sep in c[1]]
    check(pool_gits and all(c[2][:4] == tuple(lanes.GITC) for c in pool_gits), "every pool git call carries the same -c pair")
    builds = [c for c in env.calls if c[0] == "build"]
    check(builds[0][1] == "l1" and builds[0][3] == "first", "lane 1 builds first, alone (the gate)")
    eds = [c for c in env.calls if c[0] == "editor"]
    check(all("-unattended" in c[2] for c in eds), "every lane editor launch has -unattended")
    merged = open(os.path.join(out, "b6b_all_mutations.txt"), encoding="utf-8").read()
    check("# rows with no verdict: 0" in merged, "merge: rows with no verdict 0")
    check("b5x_chronicle_off (older touched) no longer applies" in merged, "merge lists the marked older row as not run")
    lines_order = [l.split()[0] for l in merged.splitlines() if l and not l.startswith(("#", " "))]
    idx = {it["tag"]: it["index"] for it in p["items"]}
    seq = [idx[t] for t in lines_order if t in idx]
    check(seq == sorted(seq), "merge in README order")
    disp = open(os.path.join(out, "dispositions.tsv"), encoding="utf-8").read().splitlines()
    check(disp[0].startswith("tag\tfolder\tset\tkind\tverdict") and any(l.startswith("b6b9_alpha_3\t") for l in disp), "dispositions table for the lead")
    check(not os.path.exists(r"C:\CAL"), "C:\\CAL not created")
    wall3 = rec["wall_s"]

    section("one item: the byte restore when apply -R leaves CRLF")
    out = os.path.join(WORK, "run_crlf")
    run, env, text = run_sim(plan_path, out, 2, {"crlf_on_revert": ["b6b9_alpha_1"]})
    res = results(out)
    notes = res["b6b9_alpha_1"]["notes"]
    check(any("restored the snapshot bytes" in n and "line endings" in n for n in notes), "CRLF after apply -R detected and restored")
    lane_root = os.path.join(out, "sim_CAL", res["b6b9_alpha_1"]["lane"])
    target = p["items"][[i for i, it in enumerate(p["items"]) if it["tag"] == "b6b9_alpha_1"][0]]["targets"][0]
    after = env.read(os.path.join(lane_root, target))
    check(after == env._head(target) and b"\r\n" not in after, "the target file holds its snapshot bytes (LF) again")
    check(res["b6b9_alpha_2"]["verdict"] in ("KILLED", "SURVIVED"), "the next item on that file applies cleanly")

    section("the memory gate: max(2.0 GB, peak + 0.5 GB), two lanes below 8 GB")
    out = os.path.join(WORK, "run_mem")
    run, env, text = run_sim(plan_path, out, 3, {"free_seq": [6.0, 1.0, 1.5, 1.9, 3.0], "peak": 3.0})
    rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    check(len(rec["lanes"]) == 2, "6 GB free at the start -> two lanes")
    check(rec["memory_waits"] and rec["memory_waits"][0][1] == "build" and rec["memory_waits"][0][2] >= 15,
          "the gate build waited for memory (%s)" % rec["memory_waits"][:1])
    check("waits for memory before build: free 1.0 GB < 2.0 GB" in text, "each wait is logged")
    check(abs(rec["threshold_gb"] - 3.5) < 1e-6, "threshold after a 3.0 GB peak is 3.5 GB")
    check(abs(run.threshold() - 3.5) < 1e-6 and max(2.0, 0.5 + 1.2) == 2.0, "max(2.0, peak + 0.5)")

    section("the gate: -DisableUnity fails -> -WaitMutex for every lane")
    out = os.path.join(WORK, "run_unity")
    run, env, text = run_sim(plan_path, out, 3, {"fail_nonunity": True})
    rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    builds = [c for c in env.calls if c[0] == "build"]
    check(rec["lane_flags"] == "-WaitMutex" and rec["gate_errors"], "LANE_FLAGS -WaitMutex and the gate's errors recorded")
    check(builds[0][2] == ("-WaitMutex", "-DisableUnity") and all(c[2] == ("-WaitMutex",) for c in builds[1:]),
          "only the first gate build was non-unity; every later build of every lane is adaptive")
    check(all(t in results(out) for t in planned), "every item still ran")

    section("a lane stops when its baseline run fails; a map-reload-only baseline is rerun alone")
    out = os.path.join(WORK, "run_base")
    run, env, text = run_sim(plan_path, out, 3, {"baseline_fail": {"l2": "real", "l3": "map"}})
    rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    l2 = [l for l in rec["lanes"] if l["name"] == "l2"][0]
    l3 = [l for l in rec["lanes"] if l["name"] == "l3"][0]
    res = results(out)
    check(not l2["alive"] and "baseline" in (l2["stop"] or ""), "l2 stopped on its baseline failure")
    check(not any(r.get("lane") == "l2" for r in res.values()), "the stopped lane ran no item")
    check(l3["alive"] and l3["items"] > 0, "l3's map-reload-only baseline was rerun alone and the lane went on")
    check(all(t in res for t in planned), "the other lanes took the stopped lane's share")

    section("NOT APPLIED and BUILD FAILED")
    out = os.path.join(WORK, "run_na")
    run, env, text = run_sim(plan_path, out, 2, {"apply_fail": ["b6b9_alpha_4", "b6b9_data_word_gone"], "build_fail": ["b6b9_alpha_5"]})
    res = results(out)
    check(res["b6b9_alpha_4"]["verdict"] == "NOT APPLIED", "a patch that does not apply -> NOT APPLIED")
    check(res["b6b9_alpha_5"]["verdict"] == "BUILD FAILED", "a mutant that does not compile -> BUILD FAILED")
    check(res["b6b9_data_word_gone"]["verdict"] == "NOT APPLIED", "the pool's NOT APPLIED")

    section("simulated wall time (virtual clock vs the section 5.4 estimate)")
    out = os.path.join(WORK, "run2")
    run, env, text = run_sim(plan_path, out, 2, opts, ["--probes", probes])
    wall2 = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))["wall_s"]
    est3 = lanes.simulate(p, 3)["total_s"]
    est2 = lanes.simulate(p, 2)["total_s"]
    print("   synthetic set: virtual run 3 lanes %.0f s, 2 lanes %.0f s; estimate 3 lanes %.0f s, 2 lanes %.0f s" % (wall3, wall2, est3, est2))
    check(abs(wall3 - est3) / est3 < 0.35 and abs(wall2 - est2) / est2 < 0.35, "the threaded virtual run agrees with the estimate within 35%")

# ------------------------------------------------------------------------------------------------ 4b. the review's breaking cases (F1-F6)

CRASH_PARTIAL = """[2026.11.10-01.00.00:000][  9]LogAutomationController: Display: Test Completed. Result={Success} Name={EdictA} Path={Cosmic_Ascension.Sim.EdictA}
[2026.11.10-01.00.01:000][ 10]LogWindows: Error: === Critical error: ===
[2026.11.10-01.00.01:000][ 10]LogWindows: Error: Assertion failed: Index >= 0 [File:Array.h] [Line: 771]
"""


class ReviewEnv(lanes.SimEnv):
    """The review's mocks in one class: make_data with no log / a RuntimeError after one table, an OSError in one
    item's test run, bundleB refusing or showing a problem in the clean run. opts 'fs': the files of an earlier
    mocked run (a --reuse-lanes rerun finds its lane trees as that run left them)."""

    def __init__(self, *a, **k):
        super().__init__(*a, **k)
        if self.o.get("fs") is not None:
            self.fs = self.o["fs"]

    def editor(self, lane, args, log, hint=None, timeout=None):
        cur = getattr(lane, "current", None)
        py = any(a.startswith("-run=pythonscript") for a in args)
        if py and not getattr(lane, "reverting", False) and cur in self.o.get("md_nolog", ()):
            self.sync.sleep(5)
            return -1073741819, "", "", 0.0
        if py and not getattr(lane, "reverting", False) and cur in self.o.get("md_partial", ()):
            self.sync.sleep(50)
            text = ("LogPython: [make_data_tables] DT_X: 3 rows written and verified\n"
                    "LogPython: Error: RuntimeError: DT_Y does not hold what was written:\n")
            self.write(log, text.encode())
            return 1, text, "", 1.0
        if cur in self.o.get("raise_in", ()) and any("RunTests" in a for a in args):
            raise OSError("simulated: log file locked")
        if cur in self.o.get("crash_in", ()) and any("RunTests" in a for a in args):
            self.sync.sleep(9)
            self.write(log, CRASH_PARTIAL.encode())
            return -1073741819, CRASH_PARTIAL, "", 1.5
        return super().editor(lane, args, log, hint, timeout)

    def run_cmd(self, cmd, cwd, env=None, timeout=None):
        if "bundleB.py" in " ".join(cmd):
            mode = self.o.get("bundle_mode")
            if mode == "refuse":
                self.sync.sleep(1)
                return 2, "refusing to start - running now:\n  UnrealEditor.exe (pid 4)\n"
            if mode == "clean_problem":
                self.sync.sleep(240)
                return 1, "bundle B: 2 jobs, 1 workers, folder x\nwanderer s1: witness first at 120 before the wanderer left at 300\n"
        return super().run_cmd(cmd, cwd, env, timeout)


def run_review(plan_path, out, lanes_n, opts, extra=None, keep=False):
    if os.path.exists(out) and not keep:
        shutil.rmtree(out)
    import io
    import contextlib
    cap = Capture()

    def factory(*a, **k):
        cap.env = ReviewEnv(*a, **k)
        return cap.env
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        run = lanes.main(["--plan", plan_path, "--lanes", str(lanes_n), "--out", out] + (extra or []), env_factory=factory, sim_opts=opts)
    return run, cap.env, buf.getvalue()


def test_review_fixes(p):
    plan_path = os.path.join(WORK, "plan_syn.json")
    planned = [it["tag"] for it in p["items"] if it["run"]]
    smoke = lanes.WITNESS_SMOKE

    section("review F2: a test run that ends without 'TEST COMPLETE' is never SURVIVED")
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(CRASH_PARTIAL))
    check(v == "NO RESULT" and not rr and "test queue emptied" in e[0] and any("Critical error" in x for x in e),
          "crash after one passed test -> NO RESULT with the crash line (%s)" % v)
    crash40 = "".join("[x]LogAutomationController: Display: Test Completed. Result={Success} Name={T%d} Path={Cosmic_Ascension.Sim.T%d}\n" % (i, i)
                      for i in range(40)) + "[x]LogWindows: Error: === Critical error: ===\n"
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(crash40))
    check(v == "NO RESULT", "a baseline log of 40 passes then a crash is not green (%s)" % v)
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(REAL_KILLED.replace("**** TEST COMPLETE. EXIT CODE: -1 ****", "x")))
    check(v == "KILLED", "a real failure before the crash still KILLS")
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(MAP_ONLY.replace("**** TEST COMPLETE. EXIT CODE: -1 ****", "x")))
    check(v == "NO RESULT" and not rr, "a map-reload failure in a cut-short queue is not rerun alone (the rest never ran)")
    v, rr, e = lanes.verdict_of(lanes.parse_test_log(REAL_PASS.replace("**** TEST COMPLETE. EXIT CODE: 0 ****", "x")))
    check(v == "NO RESULT", "the all-Success excerpt without its TEST COMPLETE line -> NO RESULT")

    section("review F1: make_data_tables - written / refused / no result")
    check(lanes.make_data_outcome(0, "LogPython: [make_data_tables] 2 assets saved\nLogPython: [make_data_tables] done\n")[0] == "written", "'done' -> written")
    st_, d_ = lanes.make_data_outcome(1, "LogPython: [make_data_tables] DT_X: 3 rows written and verified\n"
                                         "LogPython: Error: RuntimeError: DT_Y does not hold what was written:\n")
    check(st_ == "refused" and "DT_Y" in d_, "one table written, then a RuntimeError -> refused, not written")
    st_, d_ = lanes.make_data_outcome(-1073741819, "", "")
    check(st_ == "no result" and "rc -1073741819" in d_ and "log empty" in d_, "an empty log -> no result (%s)" % d_)
    st_, d_ = lanes.make_data_outcome(0, "LogPython: [make_data_tables] DT_X: 3 rows written and verified\n")
    check(st_ == "no result", "'written and verified' alone (no 'done') -> no result")
    out = os.path.join(WORK, "rv_makedata")
    run, env, text = run_review(plan_path, out, 2, {"md_nolog": ["b6b9_data_rows_gone"], "md_partial": ["b6b9_data_rows_twice"],
                                                    "verdicts": {"b6b9_data_rows_twice": "SURVIVED"}})
    res = results(out)
    r = res["b6b9_data_rows_gone"]
    check(r["verdict"] == "NO RESULT" and r["errors"][0].startswith("make_data gave no verdict (rc -1073741819")
          and "assets after revert: written" in r["notes"], "an editor death in make_data -> NO RESULT, then the revert (%s)" % r["verdict"])
    r = res["b6b9_data_rows_twice"]
    check(r["verdict"] == "KILLED" and "assets: written" not in r["notes"] and "DT_Y" in " ".join(r["errors"]),
          "one table written then a RuntimeError -> refused (KILLED), never 'assets: written'")
    disp = open(os.path.join(out, "dispositions.tsv"), encoding="utf-8").read()
    check("b6b9_data_rows_gone\t" in disp, "the make_data NO RESULT is on the dispositions table")

    section("review F2: the baseline must end its queue and run the plan's tests")
    out = os.path.join(WORK, "rv_baseline")
    run, env, text = run_review(plan_path, out, 3, {"baseline_fail": {"l2": "crash"}, "baseline_short": {"l3": 4}})
    rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    l1, l2, l3 = rec["lanes"]
    check(l1["alive"] and not l2["alive"] and "test queue emptied" in (l2["stop"] or ""), "a baseline that crashed after 40%% stops its lane (%s)" % l2["stop"])
    check(not l3["alive"] and "4 of the plan's %d tests" % p["tests"] in (l3["stop"] or ""), "a short baseline (4 of %d) stops its lane" % p["tests"])
    check(rec["short_baselines"] == ["l3 baseline ran 4 of the plan's %d tests" % p["tests"]], "the short baseline is recorded for N section 5")
    merged = open(os.path.join(out, "b6b_all_mutations.txt"), encoding="utf-8").read()
    check("# rows with no verdict: 0" in merged and "SHORT BASELINE (N section 5): l3" in merged, "lane 1 took every item; the merge names the short baseline")

    section("review F2: a mutant whose editor crashes after a pass is NO RESULT")
    out = os.path.join(WORK, "rv_crash")
    run, env, text = run_review(plan_path, out, 2, {"crash_in": ["b6b9_alpha_3"], "verdicts": {"b6b9_alpha_3": "SURVIVED"}})
    r = results(out)["b6b9_alpha_3"]
    check(r["verdict"] == "NO RESULT" and "test queue emptied" in r["errors"][0], "crash mid-queue -> NO RESULT (%s)" % r["verdict"])

    section("review F3: an exception inside one item gives its group back")
    grp = p["groups"][0]
    victim = grp["tags"][1]
    out = os.path.join(WORK, "rv_raise")
    run, env, text = run_review(plan_path, out, 2, {"raise_in": [victim]})
    res = results(out)
    mc = json.load(open(os.path.join(out, "mut_counts.json"), encoding="utf-8"))
    rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    lost = [t for t in grp["tags"] if t not in res]
    check(not lost and mc["no_verdict"] == 0, "every tag of the group has a record (lost %s, no_verdict %d)" % (lost, mc["no_verdict"]))
    check(res[victim]["verdict"] == "NO RESULT" and "OSError" in res[victim]["errors"][0], "the raising item is NO RESULT with the exception")
    bad_lane = res[victim]["lane"]
    others = [t for t in grp["tags"][2:]]
    check(all(res[t].get("lane") not in (None, bad_lane) for t in others), "the rest of the group ran on the other lane")
    stopped = [l for l in rec["lanes"] if l["name"] == bad_lane][0]
    check(not stopped["alive"] and "exception in %s" % victim in stopped["stop"] and "status equal to phaseA.diff's files" in stopped["stop"],
          "the lane stops after comparing its status with phaseA.diff (%s)" % stopped["stop"])

    section("review F4: a plan with problems or missing files is refused")
    badp = dict(p, problems=["b6b9_empty: a Python row that names no method, no check_data.py and no file"])
    bp = os.path.join(WORK, "plan_problem.json")
    json.dump(badp, open(bp, "w", encoding="utf-8"), ensure_ascii=False)
    for extra in ([], ["--dry-run"]):
        try:
            run_review(bp, os.path.join(WORK, "rv_problem"), 2, {}, extra)
            check(False, "a plan with problems is refused %s" % extra)
        except SystemExit as e:
            check("b6b9_empty" in str(e), "a plan with problems is refused %s" % (extra or "(mocked run)"))
    badm = dict(p, missing=["Tools/mutations/B6b/b6b9_gone.patch"])
    json.dump(badm, open(bp, "w", encoding="utf-8"), ensure_ascii=False)
    try:
        run_review(bp, os.path.join(WORK, "rv_problem"), 2, {})
        check(False, "a plan with a missing patch file is refused")
    except SystemExit as e:
        check("b6b9_gone" in str(e), "a plan with a missing patch file is refused")
    try:
        with open(os.devnull, "w") as dn:
            import contextlib
            with contextlib.redirect_stdout(dn):
                lanes.main(["--plan", bp, "--estimate"])
        check(True, "--estimate still reads such a plan")
    except SystemExit:
        check(False, "--estimate still reads such a plan")

    section("review F5: the witness smoke needs both bundle B runs to have started")
    out = os.path.join(WORK, "rv_refuse")
    run, env, text = run_review(plan_path, out, 2, {"existing": [lanes.BUNDLE_B], "bundle_mode": "refuse"})
    r = results(out)[smoke]
    check(r["verdict"] == "NO RESULT" and "did not start its jobs (exit 2)" in r["summary"][0], "bundleB refuses -> NO RESULT (%s)" % r["summary"])
    out = os.path.join(WORK, "rv_cleanprob")
    run, env, text = run_review(plan_path, out, 2, {"existing": [lanes.BUNDLE_B], "bundle_mode": "clean_problem"})
    r = results(out)[smoke]
    check(r["verdict"] == "NO RESULT" and "clean run already shows a witness problem" in r["summary"][0], "a problem in the clean run -> NO RESULT")
    out = os.path.join(WORK, "rv_smoke_ok")
    run, env, text = run_review(plan_path, out, 2, {"existing": [lanes.BUNDLE_B]})
    check(results(out)[smoke]["verdict"] == "KILLED", "both runs started, a problem only in the mutant -> KILLED")
    run, env, text = run_review(plan_path, os.path.join(WORK, "rv_smoke_surv"), 2, {"existing": [lanes.BUNDLE_B], "witness_kills": False})
    check(results(os.path.join(WORK, "rv_smoke_surv"))[smoke]["verdict"] == "SURVIVED", "both runs started, no problem anywhere -> SURVIVED")

    section("review F6: a rerun of the dispositions keeps the main run's times")
    out = os.path.join(WORK, "rv_rerun")
    run, env, text = run_review(plan_path, out, 3, {})
    first = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    only = os.path.join(WORK, "rv_only.txt")
    open(only, "w", encoding="utf-8").write("b6b9_alpha_3\n")
    run, env, text = run_review(plan_path, out, 3, {"fs": env.fs}, ["--phase", "lanes", "--reuse-lanes", "--only", only], keep=True)
    check(results(out)["b6b9_alpha_3"]["t"] > 0 and "reusing the existing worktree" in text, "the rerun reused the lanes")
    run, env, text = run_review(plan_path, out, 3, {"fs": env.fs}, ["--phase", "merge"], keep=True)
    rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
    check(rec["wall_s"] == first["wall_s"] and rec["ubt_held_s"] == first["ubt_held_s"] and rec["lanes"] == first["lanes"]
          and rec["memory_waits"] == first["memory_waits"], "the top-level times stay the main run's (%s vs %s)" % (rec["wall_s"], first["wall_s"]))
    check(len(rec["runs"]) == 2 and rec["runs"][1]["wall_s"] < first["wall_s"] and rec["runs"][1]["only"], "both runs kept in 'runs'")
    merged = open(os.path.join(out, "b6b_all_mutations.txt"), encoding="utf-8").read()
    check("wall-clock %.0f s" % first["wall_s"] in merged.split("\n# run 1")[0], "the merge head shows the main run's wall-clock")
    check("# run 1: --phase all" in merged and "# run 2: --phase lanes --only --reuse-lanes" in merged and "busy" in merged.split("# run 2")[1],
          "the merge lists every run with its flags, wall-clock, UBT time, lane busy times and memory waits")

# ------------------------------------------------------------------------------------------------ 5. apply_mid


def git(cwd, *args):
    r = subprocess.run(["git", "-C", cwd] + list(args), capture_output=True)
    if r.returncode != 0:
        raise RuntimeError("git %s: %s" % (" ".join(args), r.stderr.decode("utf-8", "replace")))
    return r.stdout


def test_apply_mid():
    section("apply_mid.py on a throwaway repo (HEAD + phaseA.diff, new and phantom patches)")
    repo = os.path.join(WORK, "repo")
    if os.path.exists(repo):
        def unlock(func, path, exc):
            os.chmod(path, 0o666)
            func(path)
        shutil.rmtree(repo, onerror=unlock)
    os.makedirs(repo)
    git(repo, "init", "-q")
    for k, v in (("core.autocrlf", "false"), ("user.email", "selftest@example.invalid"), ("user.name", "selftest"), ("commit.gpgsign", "false")):
        git(repo, "config", k, v)
    a = "Source/M/a.cpp"
    lines = ["l%d" % i for i in range(1, 21)]
    write(os.path.join(repo, a), "\n".join(lines) + "\n")
    write(os.path.join(repo, "Config/c.ini"), "[x]\n")
    write(os.path.join(repo, "Tools/data/x.json"), "{\n  \"a\": 1\n}\n")

    def mk(old_line_no, new):
        i = old_line_no - 1
        lo, hi = max(0, i - 3), min(len(lines), i + 4)
        body = "".join((" " if k != i else "-") + lines[k] + "\n" for k in range(lo, hi))
        body = body.replace("-" + lines[i] + "\n", "-" + lines[i] + "\n+" + new + "\n")
        return "diff --git a/%s b/%s\n--- a/%s\n+++ b/%s\n@@ -%d,%d +%d,%d @@\n%s" % (a, a, a, a, lo + 1, hi - lo, lo + 1, hi - lo, body)
    write(os.path.join(repo, "Tools/mutations/B6b/p_ok.patch"), mk(17, "mut17"))
    write(os.path.join(repo, "Tools/mutations/B5/p_stale.patch"), mk(3, "mut3"))
    write(os.path.join(repo, "Tools/mutations/B6b/p_crlf.patch"), mk(12, "mut12"))
    write(os.path.join(repo, "Tools/mutations/B6b/p_gone.patch"), mk(10, "mut10"))
    git(repo, "add", "-A")
    git(repo, "commit", "-q", "-m", "base")
    # Phase A: line 3 changes; a new untracked patch; a CRLF phantom; a deleted patch
    lines2 = list(lines)
    lines2[2] = "l3 phase A"
    write(os.path.join(repo, a), "\n".join(lines2) + "\n")
    write(os.path.join(repo, "Tools/mutations/B6b/p_new.patch"),
          "diff --git a/Tools/data/x.json b/Tools/data/x.json\n--- a/Tools/data/x.json\n+++ b/Tools/data/x.json\n@@ -1,3 +1,3 @@\n {\n-  \"a\": 1\n+  \"a\": 2\n }\n")
    crlf = open(os.path.join(repo, "Tools/mutations/B6b/p_crlf.patch"), "rb").read().replace(b"\n", b"\r\n")
    open(os.path.join(repo, "Tools/mutations/B6b/p_crlf.patch"), "wb").write(crlf)
    os.remove(os.path.join(repo, "Tools/mutations/B6b/p_gone.patch"))
    diff = git(repo, "-c", "core.eol=lf", "-c", "core.autocrlf=false", "diff", "HEAD", "--", "Source", "Tools", "Config", ":!Tools/mutations")
    diff_path = os.path.join(WORK, "phaseA_syn.diff")
    open(diff_path, "wb").write(diff)
    empty = os.path.join(WORK, "empty.diff")
    open(empty, "wb").write(b"")
    logs = []
    before = os.path.join(WORK, "apply_before_syn.txt")
    mid = os.path.join(WORK, "apply_mid_syn.txt")
    # 'before' = HEAD alone (apply_state.py's view): the deleted patch is still there, the new one is copied in too
    apply_mid.run(repo, empty, os.path.join(WORK, "as_before"), before, "HEAD", None, logs.append)
    r = apply_mid.run(repo, diff_path, os.path.join(WORK, "as_mid"), mid, "HEAD", before, logs.append)
    st = apply_mid.read_state(mid)
    check(r["files"] == [a], "the Phase A diff names its one file and the scratch copy equals the tree's bytes")
    check(st.get("Tools/mutations/B6b/p_ok.patch") == "OK", "a patch far from the change stays OK")
    check(st.get("Tools/mutations/B5/p_stale.patch") == "FAIL", "a patch on the changed line is FAIL on the Phase A tree")
    check(st.get("Tools/mutations/B6b/p_new.patch") == "OK" and "Tools/mutations/B6b/p_new.patch" in r["copied"], "the new untracked patch is copied in and checked")
    check("Tools/mutations/B6b/p_crlf.patch" not in r["copied"] and st.get("Tools/mutations/B6b/p_crlf.patch") == "OK", "a CRLF phantom keeps the archived LF bytes")
    check("Tools/mutations/B6b/p_gone.patch" in r["gone"] and "Tools/mutations/B6b/p_gone.patch" not in st, "a patch the tree no longer has is removed")
    check(r["compare"]["ok_to_fail"] == ["Tools/mutations/B5/p_stale.patch"], "compare: OK -> FAIL lists the stale one")
    bad = os.path.join(WORK, "bad.diff")
    open(bad, "wb").write(diff.replace(b"l3 phase A", b"l3 something else"))
    try:
        apply_mid.run(repo, bad, os.path.join(WORK, "as_bad"), os.path.join(WORK, "bad.txt"), "HEAD", None, logs.append)
        check(False, "a diff whose result differs from the tree is refused")
    except SystemExit as e:
        check("differ from the project tree" in str(e), "a diff whose result differs from the tree is refused")
    for l in logs[:3]:
        print("   " + l)

# ------------------------------------------------------------------------------------------------ 6. the real plan, estimated


def test_real_estimate(p):
    section("section 5.4 estimate over the HEAD plan (and the threaded dry run)")
    for lanes_n in (3, 2):
        for table in ("low", "mid", "high"):
            r = lanes.simulate(p, lanes_n, table)
            print("   %d lanes %-4s: pool %4.1f + lanes %5.1f + probes %3.1f = %5.1f min (mutex held %5.1f min)" %
                  (lanes_n, table, r["pool_s"] / 60, r["lanes_s"] / 60, r["probes_s"] / 60, r["total_s"] / 60, r["mutex_held_s"] / 60))
    plan_path = os.path.join(WORK, "plan_head.json")
    json.dump(p, open(plan_path, "w", encoding="utf-8"), ensure_ascii=False)
    for n in (3, 2):
        out = os.path.join(WORK, "head_dry%d" % n)
        run, env, text = run_sim(plan_path, out, n, {})
        rec = json.load(open(os.path.join(out, "lane_record.json"), encoding="utf-8"))
        mc = json.load(open(os.path.join(out, "mut_counts.json"), encoding="utf-8"))
        check(mc["no_verdict"] == 0, "HEAD plan dry run %d lanes: every item has a verdict" % n)
        print("   threaded dry run, %d lanes: %.1f min simulated, mutex held %.1f min, items %d" %
              (n, rec["wall_s"] / 60, rec["ubt_held_s"] / 60, mc["planned"]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=DEFAULT_ROOT)
    a = ap.parse_args()
    cal_before = os.path.exists(r"C:\CAL")
    os.makedirs(WORK, exist_ok=True)
    real = test_real_plan(a.root)
    base = make_synthetic(os.path.join(WORK, "syn"))
    p = test_synthetic_plan(base)
    test_verdicts(a.root)
    test_lanes(p, base)
    test_review_fixes(p)
    test_apply_mid()
    test_real_estimate(real)
    if not cal_before:
        check(not os.path.exists(r"C:\CAL"), "C:\\CAL was never created")
    print("selftest: %d passed, %d failed" % (PASSES[0], len(FAILS)))
    for f in FAILS:
        print("FAILED:", f)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.exit(main())
