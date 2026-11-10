"""Self-test of bundleB.py: fabricates logs, CSVs and claims files under scratchpad\\b6b8\\selftest\\ in the commandlet's
formats (printed from the C++ format strings of the git show copy scratchpad\\b6b8\\ref\\cmdlet_head.cpp; the side part's
[B-6b8] layout from the brief's 2.3) and checks the job list, the parser (both side layouts), the run-failure rules, the
per-cell bridges, the switch and field controls, every judgment branch of H 5.3 (1)-(4) with the frozen arithmetic, the
record-only readings, the archive, the --cell-set A check-only against bundleA.py's own verdict and the dry run.
No commandlet.

  python -I -X utf8 bundleB_selftest.py [--cpp <copy>]
"""
import argparse
import contextlib
import copy
import importlib.util
import io
import json
import math
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SCRATCH = os.path.dirname(HERE)


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


B = load("bundleB", os.path.join(HERE, "bundleB.py"))
T = load("bundleA_selftest", os.path.join(SCRATCH, "b6b6", "bundleA_selftest.py"))
A = B.A

FAILS, PASSES = [], [0]
SIDE_NEW = ("era lines %d (by the record %d, first at %lld), witness cards %d (first at %lld, by the record %d; answered %d: by the check %d, "
            "failed %d, without a check %d; gone before the answer %d)")   # brief 2.3 - arrives with [B-6b8]


def check(cond, what):
    if cond:
        PASSES[0] += 1
    else:
        FAILS.append(what)
        print("  FAIL: " + what)

# --------------------------------------------------------------------------------------------------- fabrication


def race_line(F, v):
    p = F.parts
    assert len(p) == 17
    c = T.cprintf
    side_fmt = SIDE_NEW if v.get("layout", "new") == "new" else p[12]
    if v.get("layout", "new") == "new":
        side = c(side_fmt, {0: v.get("era", 0), 1: v.get("era_rec", 0), 2: v.get("era_first", -1), 3: v.get("wit", 0), 4: v.get("wit_first", -1),
                            5: v.get("wit_rec", 0), 6: v.get("answered", 0), 7: v.get("passed", 0), 8: v.get("failed", 0), 9: v.get("nocheck", 0), 10: v.get("gone", 0)})
    else:
        side = c(side_fmt, {0: v.get("era", 0), 1: 0, 2: v.get("era_first", -1), 3: v.get("wit", 0), 4: 0, 5: 0, 6: 0, 7: 0})
    b = v.get("binding", (-1, "-", "-", 0))
    parts = [
        c(p[0], {0: "home_galaxy", 1: 2}),
        c(p[1], {0: v.get("t4_1", -1), 1: v.get("t4_1_civ", "-"), 2: "-", 3: v.get("t4_2", -1), 4: "-", 5: -1, 6: "-", 7: "-"}),
        c(p[2], {0: v.get("r3", -1), 1: v.get("r3_civ", "-"), 2: v.get("r4", -1), 3: v.get("r4_civ", "-"), 4: v.get("r5", -1), 5: v.get("r5_civ", "-")}),
        c(p[3], {0: -1, 1: "-", 2: "-", 3: -1, 4: -1}),
        c(p[4], {0: 0, 1: 0, 2: v.get("most", 1)}),
        c(p[5], {0: 0, 1: -1, 2: 0}),
        c(p[6], {0: -1, 1: "-", 2: "-", 3: v.get("ruled_binding", -1)}),
        c(p[7], {0: v.get("on_ruled", 0), 1: -1, 2: v.get("on_ai", 0), 7: "", 15: v.get("erased_ruled", 0)}),
        c(p[8], {0: v.get("ai_treaties", 0), 1: -1, 2: 0, 3: 0}),
        c(p[9], {0: v.get("laws_ruled", -1), 1: v.get("laws_ai", -1), 2: v.get("laws_ai_civ", "-"), 5: v.get("resets_r", 0)}),
        c(p[10], {0: b[0], 1: b[1], 2: b[2], 3: "sys_1", 4: b[3], 8: -1, 9: v.get("locked", 0), 10: -1, 12: -1}),
        c(p[11], {0: v.get("p162_civ", "-"), 1: v.get("shown", -1), 2: v.get("once", -1)}),
        side,
        c(p[13], {1: -1, 2: v.get("returns", 0)}),
        c(p[14]),
        c(p[15], {0: v.get("claims", 0)}),
        c(p[16], {0: " ".join("%d:%d" % d for d in v.get("decisions", [])), 1: -1}),
    ]
    if v.get("drop_binding"):
        parts[10] = "binding gone"
    return T.cprintf(F.race, {0: "; ".join(parts)})


def write_log(path, F, v, ticks=20000, off=False, cell="R", extra=()):
    ue = T.ue
    L = ["[2026.11.09-12.00.00:000][  0]LogInit: Display: Running engine for game: Cosmic_Ascension"]
    if off or v.get("taken_out_on"):
        L.append(ue(T.cprintf(F.taken_out, {0: v.get("taken_out", B.EXPECTED_REMOVED), 1: v.get("was_r", 0.375), 2: v.get("was_share", 0.0)})))
    L.append(ue(T.cprintf(F.seed, {0: v.get("seed", 1), 1: ticks, 2: 12345, 3: 999, 4: "C:/x/y.csv"})))
    L.append(ue(T.cprintf(F.stages, {"AI ": v.get("ai_us", 10.5)})))
    if cell == "wanderer":
        L.append(ue(T.cprintf(F.wanderer, {"abdicate ": 1200, "wander ": 1, "left at ": v.get("left", 1200), "crowned at ": v.get("crowned", -1),
                                          "crowned at %lld (": "civ_7", "tier ": 3, "path ": "Vacuum"})))
    if cell == "death":
        L.append(ue(T.cprintf(F.deaths, {"death asked at ": v.get("death_at", 4400), "(recorded ": v.get("recorded", 1)})))
    L.append(ue(T.cprintf(F.ending, {"record ": v.get("kind", "None"), "record %s at ": v.get("rec", -1), "(people ": v.get("rec_civ", "-"),
                                     "a people some life ruled ": v.get("some", 0), "no vessel ": 0, "ruled T3 at ": v.get("r3", 2000), "ruled T3 at %lld (": "civ_7",
                                     "first AI T3 at ": v.get("ai3", 1800), "first AI T3 at %lld (": "civ_3"})))
    L.append(ue(race_line(F, v)))
    L.extend(extra)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8-sig", newline="\r\n") as f:
        f.write("\n".join(L) + "\n")


def write_csv(path, F, changes=(), to=20000, step=100, tier7=lambda t: 3):
    """Two civs every `step` ticks; changes = [(tick, label)] - from each change on, civ_3's Energy carries the label."""
    header = F.csv_header.split(",")
    lines = [F.csv_header]
    for tick in range(0, to + 1, step):
        label = "1.00"
        for t, lab in sorted(changes):
            if tick >= t:
                label = "%.2f" % (100 + sum(map(ord, lab)))  # a number per label (the CSV reader parses Energy)
        for civ in ("civ_3", "civ_7"):
            row = {h: "0" for h in header}
            row.update({"Tick": str(tick), "CivID": civ, "Archetype": "arch_default", "Tier": str(tier7(tick) if civ == "civ_7" else 3), "Systems": "4",
                        "Energy": label if civ == "civ_3" else "1.00", "Matter": "1.00", "Expansion": "" if civ == "civ_7" else "Grew",
                        "TickMicros": str(tick % 7), "SaveBytes": str(tick % 5), "SaveRawBytes": str(tick % 3), "DrainPressure": "", "FirstClaimTick": ""})
            lines.append(",".join(row[h] for h in header))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


def write_rows(path, F, entries=((500, "log_x", 1),)):
    with open(path, "w", encoding="utf-8") as f:
        f.write(F.rows_header + "\n" + "".join("%d,%s,%d\n" % e for e in entries))


def write_claims(path, F, placed=()):
    cols = F.claims_header.split(",")
    with open(path, "w", encoding="utf-8") as f:
        f.write(F.claims_header + "\n")
        for i, t in enumerate(placed, 1):
            row = {c: "0" for c in cols}
            row.update(Claim=str(i), SystemID="sys_%d" % i, Owner="civ_3", PlacedTick=str(t), EndKind="joined")
            f.write(",".join(row[c] for c in cols) + "\n")


def write_job(F, job, v, changes=(), placed=(), extra=(), tier7=lambda t: 3):
    write_log(job["log"], F, v, job["ticks"], job["off"], job["cell"], extra)
    write_csv(job["csv"], F, changes, tier7=tier7)
    write_rows(job["rows"], F)
    write_claims(job["claims"], F, placed)


OFF_V = dict(kind="None", rec=-1)
TIER_1 = lambda t: 4 if t >= 1000 else 3  # noqa: E731  seed 1's ruled people stands at T4 from 1,000 in every run


def clean_bundle(F, folder):
    """Every pair, bridge and control passes (seeds 1-2)."""
    jobs = {j["name"]: j for j in B.build_jobs(folder, 4400, max_seed=2)}
    tier_1 = TIER_1
    on1 = dict(seed=1, kind="OtherPeople", rec=9000, rec_civ="civ_3", era=1, era_rec=1, era_first=900, r3=2000, r3_civ="civ_7", on_ai=1, ai_treaties=1,
               t4_1=5000, t4_1_civ="civ_3", binding=(8000, "AI", "civ_3", 4), p162_civ="civ_3", shown=1, once=0, decisions=[(2100, 1), (2500, 2)])
    on2 = dict(seed=2, kind="RuledPeople", rec=8000, rec_civ="civ_7", r3=2000, r3_civ="civ_7", r4=4000, r4_civ="civ_7", r5=8000, r5_civ="civ_7",
               binding=(8000, "ruled", "civ_7", 4), decisions=[(4100, 1)])
    for seed, on, ch in ((1, on1, [(1000, "A")]), (2, on2, [(2000, "B")])):
        tier = tier_1 if seed == 1 else (lambda t: 3)
        write_job(F, jobs["R_%d" % seed], on, ch, tier7=tier)
        write_job(F, jobs["R_%d_off" % seed], dict(OFF_V, seed=seed), [], tier7=tier)
        write_job(F, jobs["slowR_%d" % seed], dict(on, era=0, era_rec=0, era_first=-1), [(2000, "S")], tier7=tier)
        write_job(F, jobs["slowR_%d_off" % seed], dict(OFF_V, seed=seed), [], tier7=tier)
        write_job(F, jobs["inputless_%d" % seed], dict(OFF_V, seed=seed), [], tier7=tier)
        write_job(F, jobs["inputless_%d_off" % seed], dict(OFF_V, seed=seed), [], tier7=tier)
        write_job(F, jobs["death_%d" % seed], dict(on, recorded=1 if seed == 1 else 0), list(ch) + [(4400, "D")], tier7=tier)
    # wanderer_1: the era at 900 (as R_1), W 1,500 - T_B 900; wanderer_2: no era, W 1,500 - T_B 1,500 (its change is at 1,500).
    write_job(F, jobs["wanderer_1"], dict(on1, wit=1, wit_first=1500, wit_rec=1, answered=1, passed=1, left=1200, crowned=3000), [(1000, "A"), (1500, "W")],
              placed=[1400, 1600], tier7=tier_1)
    write_job(F, jobs["wanderer_1_off"], dict(OFF_V, seed=1, left=1200, crowned=5000), [], tier7=tier_1)
    write_job(F, jobs["wanderer_2"], dict(on2, wit=2, wit_first=1500, wit_rec=2, answered=2, nocheck=1, failed=1, left=1200, crowned=-1), [(1500, "W")],
              placed=[1500])
    write_job(F, jobs["wanderer_2_off"], dict(OFF_V, seed=2, left=1200, crowned=2500), [])
    return jobs

# --------------------------------------------------------------------------------------------- synthetic judgments


def mk(cell, seed, kind="None", rec=-1, civ="-", some=0, r3=-1, r4=-1, r4civ="-", r5=-1, r5civ="-", binding=(-1, "-", "-", 0), erased_ruled=0,
       decisions=(), locked=0, ok=True):
    job = {"name": "%s_%d" % (cell, seed), "cell": cell, "seed": seed, "off": False, "ticks": 20000}
    p = {"log_exists": True, "ending_x": {"kind": kind, "tick": rec, "civ": civ, "some_life_ruled": some, "no_vessel": 0},
         "race": {"ruled": {"r3": r3, "r3_civ": "civ_7", "r4": r4, "r4_civ": r4civ, "r5": r5, "r5_civ": r5civ}, "extinction": {"erased_ruled": erased_ruled}},
         "race_b": {"binding": {"tick": binding[0], "by": binding[1], "civ": binding[2], "rung": binding[3], "locked": locked, "no_result": 0, "retaken": 0,
                                "vessel_left": 0},
                    "decisions": {"pairs": list(decisions)}}}
    return job, p, ok


def every(start, end, step=300):
    return [(t, 1) for t in range(start, end, step)]


# ------------------------------------------------------------------------------------------------ [B-6b9] additions


def quiet(fn, *args):
    sink = io.StringIO()
    with contextlib.redirect_stdout(sink):
        code = fn(*args)
    return code, sink.getvalue()


def mkc(seed, kind="None", rec=-1, civ="-", r4=5000, r5=-1, binding=(-1, "-", "-", 0), locked=0, laws=(-1, -1, "-", 0), ok=True, ruled_binding=-1,
        decisions=()):
    """A synthetic R run for --classify: mk plus the laws part and the upper part's 'ruled binding started at'."""
    job, p, ok = mk("R", seed, kind, rec, civ, r4=r4, r4civ="civ_7", r5=r5, r5civ="civ_7", binding=binding, locked=locked, decisions=decisions, ok=ok)
    p["race_b"]["laws"] = {"ruled": laws[0], "ai": laws[1], "ai_civ": laws[2], "resets_r": laws[3]}
    p["race"]["upper"] = {"ruled_binding": ruled_binding}
    return job, p, ok


def run_set(items):
    return [j for j, _, _ in items], {j["name"]: p for j, p, _ in items}, {j["name"]: {"ok": ok} for j, _, ok in items}


def reached(seed):
    """A run of (1) whose ruled people stood on the fifth rung (n3): r4 4,000, r5 5,800, ruled binding at 5,700."""
    return mkc(seed, "RuledPeople", 5850, "civ_7", r4=4000, r5=5800, binding=(5700, "ruled", "civ_7", 4), laws=(3000, -1, "-", 0),
               decisions=every(4000, 5800))


class FakeRunner:
    """Stands in for the commandlet: reads -AbsLog and -Rules from the command, writes the synthetic outputs of
    scenario[job name] (dict: v, changes, extra, exit, drop_rules) and records every command."""

    def __init__(self, F, scenario, default_v=None, touch=None):
        self.F, self.scenario, self.default_v, self.touch, self.cmds = F, scenario, default_v or {}, touch, []

    def __call__(self, cmd):
        self.cmds.append(list(cmd))
        log = next(c[len("-AbsLog="):] for c in cmd if c.startswith("-AbsLog="))
        name = os.path.splitext(os.path.basename(log))[0]
        base = log[:-len(".log")]
        job = {"name": name, "cell": name.split("_")[0], "off": name.endswith("_off"), "ticks": 20000,
               "log": log, "csv": base + ".csv", "rows": base + "_rows.csv", "claims": base + "_claims.csv"}
        sc = self.scenario.get(name, {})
        v = dict(self.default_v, seed=int(name.split("_")[1]))
        v.update(sc.get("v", {}))
        extra = list(sc.get("extra", ()))
        rules = next((c for c in cmd if c.startswith("-Rules=")), "")
        for pair in rules[len("-Rules="):].split(";"):
            if pair and not sc.get("drop_rules"):
                n, val = pair.split(":")
                extra.insert(0, T.ue("CosmicBalance: rules %s = %s" % (n, val)))
        if job["off"]:
            v = dict(OFF_V, **{k: x for k, x in v.items() if k in ("seed", "was_share", "was_r")})
        write_job(self.F, job, v, sc.get("changes", ()), extra=extra)
        if self.touch:
            self.touch(name)
        return sc.get("exit", 0)


def fake_project(root):
    proj = os.path.join(root, "proj")
    for rel in ("Cosmic_Ascension.uproject", "Binaries/Win64/UnrealEditor-Cosmic_Ascension.dll", "Content/CA/Data/DT_Rules.uasset", "Tools/data/generated.sha1"):
        path = os.path.join(proj, *rel.split("/"))
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as f:
            f.write(rel + "\n")
    editor = os.path.join(root, "UnrealEditor-Cmd.exe")
    with open(editor, "w", encoding="utf-8") as f:
        f.write("fake\n")
    return proj, editor


def b6b9_tests(F, root):
    print("14. the frozen readings (brief 0.0)")
    ok, actual, frozen, msg = B.readings_check(os.path.join(HERE, "brief_final.md"), os.path.join(HERE, "readings.sha1"))
    check(ok and actual == frozen and actual.startswith("5891b075"), "the brief's 1.1-1.5 still hash to readings.sha1: %s" % msg)
    brief = A.read_text(os.path.join(HERE, "brief_final.md"))
    rd = os.path.join(root, "readings")
    os.makedirs(rd)
    sha = os.path.join(rd, "readings.sha1")
    with open(sha, "w", encoding="utf-8") as f:
        f.write("%s  sections 1.1-1.5\n" % frozen)

    def brief_ok(text, name):
        path = os.path.join(rd, name)
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(text)
        return B.readings_check(path, sha)[0]
    check(brief_ok(brief.replace("\n", "\r\n"), "crlf.md"), "a CRLF copy hashes the same")
    check(brief_ok(brief.replace("cap of **6 probes**", "cap of **7 probes**"), "s16.md") and "cap of **6 probes**" in brief, "an edit in 1.6 (outside the readings) keeps the hash")
    check(not brief_ok(brief.replace("ceiling of 2.5e-4", "ceiling of 3e-4"), "s13.md") and "ceiling of 2.5e-4" in brief, "an edit in 1.3 breaks the hash")
    check(not brief_ok(brief.replace("### 1.6 ", "### 1.7 "), "nohead.md"), "a missing '### 1.6 ' heading is refused")
    check(not B.readings_check(os.path.join(rd, "crlf.md"), os.path.join(rd, "none.sha1"))[0], "a missing readings.sha1 is refused")

    print("15. --rules-extra, --on-only, probe and B' job lists")
    rx = B.parse_rules_extra("AIUpperRungProgressMultiplier:0.3125")
    check(rx == [("AIUpperRungProgressMultiplier", "0.3125")] and B.parse_rules_extra(None) == [] and B.parse_rules_extra("") == [], "rules-extra parsed")
    bad = 0
    for text in ("ProvisionalEndTick:5", "A:1;A:2", "nocolon", "X:1:2", "X:", "9X:1", "X:a b"):
        try:
            B.parse_rules_extra(text)
        except ValueError:
            bad += 1
    check(bad == 7, "malformed, duplicate or ProvisionalEndTick extras refused (%d/7)" % bad)
    # [review F3] only r goes through -Rules (brief 4.2; 1.5 'Not levers here'), with a value > 0
    bad = []
    for text in ("FleetDelayCountdownShare:0.5", "GalaxyLawsToHold:3", "FourthRungCap:2", "AIUpperRungProgressMultiplier:0",
                 "AIUpperRungProgressMultiplier:-0.25", "AIUpperRungProgressMultiplier:x", "AIUpperRungProgressMultiplier:nan"):
        try:
            B.parse_rules_extra(text)
        except ValueError as e:
            bad.append(str(e))
    check(len(bad) == 7 and "not a -Rules lever" in bad[0] and "number > 0" in bad[3], "F3: a non-r name or a value <= 0 is refused: %s" % bad)
    pj = B.build_jobs("x", None, {"R", "slowR"}, seeds=range(1, 61), rules_extra=rx + [("FourthRungCap", "2")], on_only=True)
    rules = [[a for a in j["args"] if a.startswith("-Rules=")] for j in pj]
    check(len(pj) == 120 and not any(j["off"] for j in pj), "probe: R 60 + slowR 60, on side only (%d)" % len(pj))
    check(all(r == ["-Rules=ProvisionalEndTick:0;AIUpperRungProgressMultiplier:0.3125;FourthRungCap:2"] for r in rules), "one -Rules= argument with the extras joined: %s" % rules[0])
    check(B.build_jobs("x", 4400)[0]["args"][0] == "-Rules=ProvisionalEndTick:0", "without extras the argument is unchanged")
    no_rows = os.path.join(root, "no_probe_rows.jsonl")   # never the real scratchpad\b6b9\probe_rows.jsonl
    pv = ["--p-bind", "1800", "--n-laws", "1200", "--probe-rows", no_rows]
    code, out = quiet(B.main, ["--dry-run", "--probe", "--probe-label", "P1 r", "--rules-extra", "AIUpperRungProgressMultiplier:0.3125", "--root", os.path.join(root, "proj")] + pv)
    cmds = [l for l in out.splitlines() if l and not l.startswith("#")]
    check(code == 0 and len(cmds) == 120 and all("-Rules=ProvisionalEndTick:0;AIUpperRungProgressMultiplier:0.3125" in c and "_off" not in c for c in cmds)
          and "probe_P1_r" in cmds[0], "probe dry run: 120 commands, joined -Rules, no off run, folder probe_P1_r")
    code, out = quiet(B.main, ["--dry-run", "--seeds", "61-120", "--t-death", "4400", "--bprime", "--share", "0.5", "--root", os.path.join(root, "proj")])
    lines = out.splitlines()
    check(code == 0 and len([l for l in lines if not l.startswith("#")]) == 540 and "# 540 jobs" in lines[-2] and "R_61" in lines[0] and "bundleBprime" in lines[0]
          and "mode bprime, share 0.5" in lines[-1], "B' dry run --seeds 61-120: 540 jobs into bundleBprime")
    for argv, needle in ((["--rules-extra", "ProvisionalEndTick:1"], "--rules-extra"), (["--share", "half"], "not a number"),
                         (["--bprime", "--seeds", "1-60"], "61-120"), (["--probe"], "--probe-label"), (["--probe", "--probe-label", "x", "--seeds", "1-61"], "1-60"),
                         (["--probe", "--bprime", "--probe-label", "x"], "exclude"), (["--accept-against", "x.json"], "for a probe"),
                         (["--archive-dir", os.path.join(root, "Saved", "Balance", "b6b_bundleB")], "ends in b6b_bundleB"),
                         (["--archive-dir", os.path.join(root, "B6B_BUNDLEB") + os.sep], "ends in b6b_bundleB")):
        code, out = quiet(B.main, ["--dry-run", "--root", os.path.join(root, "proj"), "--t-death", "4400"] + argv)
        check(code == 2 and needle in out, "refused %s: %s" % (argv, out.strip()[-120:]))
    code, out = quiet(B.main, ["--dry-run", "--root", os.path.join(root, "proj"), "--t-death", "4400", "--archive-dir", os.path.join(root, "b6b_bundleBprime")])
    check(code == 0, "an archive dir ending in b6b_bundleBprime is allowed")
    # [review F2, F3, F6] the reviewer's breaking command lines, each refused before anything runs
    proj_ = os.path.join(root, "proj")
    bsum0 = os.path.join(root, "bundleB_summary0.json")
    with open(bsum0, "w", encoding="utf-8") as f:
        json.dump({"judgments": {"1": {"passed": False}, "2": {"passed": True}, "3": {"length_ok": True, "density_ok": True}, "4": {"passed": True}}}, f)
    badsum = os.path.join(root, "not_a_summary.json")
    with open(badsum, "w", encoding="utf-8") as f:
        f.write("{}")
    probe = ["--dry-run", "--probe", "--probe-label", "x", "--seeds", "1-2", "--root", proj_, "--probe-rows", no_rows]
    acc = ["--p-bind", "1800", "--n-laws", "1200", "--accept-against", bsum0]
    bpr = ["--bprime", "--share", "0.5", "--t-death", "4400", "--root", proj_]
    for argv, needle, label in (
            (probe + ["--changed", "C,n"] + acc + ["--target", "1"], "not among", "F2 B6: --changed 'C,n'"),
            (probe + ["--changed", "p_bind"] + acc + ["--target", "1"], "not among", "F2 B3: --changed p_bind"),
            (probe + ["--changed", "c"] + acc, "--target", "F2 B4: c without --target"),
            (probe + ["--changed", "r"] + acc, "--r-step", "F2 B5: r without --r-step"),
            (probe + ["--changed", "c", "--target", "1", "--r-step", "down"] + acc, "without r", "F2: --r-step without r"),
            (probe + acc + ["--target", "1"], "R4 needs --changed", "F2: --accept-against without --changed"),
            (probe + ["--p-bind", "1800", "--n-laws", "1200", "--changed", "c", "--target", "1"], "R4's inputs", "F2: --changed without --accept-against"),
            (probe + ["--changed", "c", "--target", "1", "--p-bind", "1800", "--n-laws", "1200", "--accept-against", badsum], "not bundle B's summary.json",
             "F2: an --accept-against without judgments"),
            (probe + ["--n-laws", "600"], "--p-bind and --n-laws", "F2 L: --n-laws 600 without --p-bind"),
            (probe + ["--p-bind", "1800"], "--p-bind and --n-laws", "F2: --p-bind without --n-laws"),
            (bpr + ["--seeds", "61-130", "--dry-run"], "61-120", "F3 F: B' seeds 61-130"),
            (bpr + ["--seeds", "61-62", "--dry-run", "--on-only"], "--on-only", "F3 C: B' --on-only"),
            (bpr + ["--seeds", "61-62", "--dry-run", "--rules-extra", "AIUpperRungProgressMultiplier:0.3125"], "never --rules-extra", "F3 D: B' --rules-extra"),
            (bpr + ["--seeds", "61-62", "--r", "0.375"], "4.3 witnesses", "F3: a B' run without --p-bind/--n-laws"),
            (bpr + ["--seeds", "61-62", "--p-bind", "1800", "--n-laws", "1200"], "4.3 witnesses", "F3: a B' run without --r")):
        code, out = quiet(B.main, argv)
        check(code == 2 and needle in out, "%s refused: %s" % (label, out.strip()[-160:]))
    code, out = quiet(B.main, probe + ["--changed", "r", "--r-step", "down"] + acc)
    check(code == 0, "R4 inputs r + --r-step without --target pass the check (r alone): %s" % out.strip()[-160:])
    code, out = quiet(B.main, probe + ["--changed", "c,N", "--target", "3len"] + acc)
    check(code == 0, "R4 inputs c,N + --target 3len pass the check: %s" % out.strip()[-160:])
    code, out = quiet(B.main, bpr + ["--seeds", "61-120", "--dry-run"])
    check(code == 0, "a B' dry run needs no witness values")

    print("16. the share control, the rules lines and the delays")
    folder = os.path.join(root, "clean")
    jobs = clean_bundle(F, folder)
    parsed = {n: B.parse_log(j["log"]) for n, j in jobs.items()}
    jl = list(jobs.values())
    okr = {n: {"ok": True} for n in jobs}
    check(B.controls(jl, parsed, okr, 4400, 0.375, 0.0)["ok"], "share 0: bundle B's off logs pass")
    c = B.controls(jl, parsed, okr, 4400, 0.375, 0.5)
    check(not c["ok"] and sum("FleetDelayCountdownShare was 0, not 0.5" in x for x in c["problems"]) == 8, "share 0.5 flags every off log that says (was 0): %d" % len(c["problems"]))
    pp = copy.deepcopy(parsed)
    for n in pp:
        if n.endswith("_off"):
            pp[n]["off_knobs"]["share"] = "0.5"
    check(B.controls(jl, pp, okr, 4400, 0.375, 0.5)["ok"], "share 0.5 with (was 0.5) passes")
    c = B.controls(jl, parsed, okr, 4400, 0.375, 0.0, [("AIUpperRungProgressMultiplier", "0.3125")])
    check(not c["ok"] and any("no 'CosmicBalance: rules ProvisionalEndTick = 0'" in x for x in c["problems"]), "with extras, a log without the rules lines fails")
    pp = copy.deepcopy(parsed)
    for n in pp:
        pp[n]["rules_lines"] = [("ProvisionalEndTick", "0"), ("AIUpperRungProgressMultiplier", "0.3125")]
    check(B.controls(jl, pp, okr, 4400, 0.375, 0.0, [("AIUpperRungProgressMultiplier", "0.3125")])["ok"], "every rules line present passes")
    lg = os.path.join(root, "delays.log")
    write_log(lg, F, dict(OFF_V), off=True, extra=[T.ue(T.cprintf(F.conflict, {"delay bought per tag [": "Hazard.AIAwakening=80(1) Hazard.Plague=40(2)"})),
                                                  T.ue("CosmicBalance: rules ProvisionalEndTick = 0")])
    pl = B.parse_log(lg)
    check(pl["delays"] == {"Hazard.AIAwakening": (80, 1), "Hazard.Plague": (40, 2)} and B.awakening_sum(pl) == 80 and pl["rules_lines"] == [("ProvisionalEndTick", "0")],
          "delays %s and rules lines %s read" % (pl["delays"], pl["rules_lines"]))
    write_log(lg, F, dict(OFF_V), off=True, extra=[T.ue(T.cprintf(F.conflict, {"delay bought per tag [": ""}))])
    check(B.awakening_sum(B.parse_log(lg)) == 0 and B.awakening_sum(parsed["R_1_off"]) is None, "an empty list sums 0; no conflict line is None")

    print("17. B' pair reading (4.1 item 6)")
    write_csv(jobs["R_2_off"]["csv"], F, [(500, "A")])   # the off twin moves (the bridges compare on sides only)
    poff = copy.deepcopy(parsed["R_2_off"])
    for s, share, expect, label in ((80, 0.5, True, "sum 80 > 75 at share 0.5: allowed"), (75, 0.5, False, "sum 75: not allowed"),
                                    (None, 0.5, False, "no conflict line: not allowed"), (80, 0.0, False, "share 0 (bundle B): strict")):
        poff["delays"] = None if s is None else {"Hazard.AIAwakening": (s, 1)}
        pr = B.pair_check(jobs["R_2"], jobs["R_2_off"], parsed["R_2"], poff, False, share)
        check(pr["ok"] == expect and pr["moved_by_share"] == expect and (expect or any("differs at tick 500" in w for w in pr["why"])), "%s: %s" % (label, pr["why"]))
    poff["delays"] = {"Hazard.AIAwakening": (80, 1)}
    poff["ending"]["ai3"] = 1700
    pr = B.pair_check(jobs["R_2"], jobs["R_2_off"], parsed["R_2"], poff, False, 0.5)
    check(pr["ok"] and pr["moved_by_share"] and any("T differs" in w for w in pr["allowed"]), "'T differs' is allowed with the sum")
    j2 = dict(jobs["R_2"], rows=os.path.join(root, "missing_rows.csv"))
    pr = B.pair_check(j2, jobs["R_2_off"], parsed["R_2"], poff, False, 0.5)
    check(not pr["ok"] and any("missing rows file" in w for w in pr["why"]), "a missing file is never allowed")
    pp = copy.deepcopy(parsed)
    pp["R_2_off"]["delays"] = {"Hazard.AIAwakening": (90, 1)}
    for n in pp:
        if n.endswith("_off"):
            pp[n]["off_knobs"]["share"] = "0.5"
    res = B.analyse(jl, pp, okr, 4400, 0.375, False, None, 0.5)
    check(res["record"]["moved by the share"] == {"count": 1, "by cell": {"R": ["R_2"]}} and B.verdict(res, {"sha1_ok": True}),
          "analyse counts one pair moved by the share; verdict passes: %s" % [(p["pair"], p["why"]) for p in res["pairs"] + res["bridges"] if not p["ok"]])
    write_csv(jobs["death_2"]["csv"], F, [(2000, "B"), (3000, "Z")])
    res = B.analyse(jl, pp, okr, 4400, 0.375, False, None, 0.5)
    check(any(b["ok"] is False and "death_2" in b["pair"] for b in res["bridges"]) and not B.verdict(res, {}), "the death bridge stays strict under share 0.5")
    jobs = clean_bundle(F, folder)

    print("18. routes, --bprime and the archive defaults")

    def r2b(n_other, n_ruled):
        items = [mk("R", s, "OtherPeople", 9000, "civ_3", 0, r4=7000, r4civ="civ_7", binding=(8000, "AI", "civ_3", 4)) for s in range(1, n_other + 1)]
        items += [mk("R", n_other + s, "RuledPeople", 9000, "civ_7") for s in range(1, n_ruled + 1)]
        return items
    j = B.judge_2(r2b(16, 24), bprime=True)
    check(not j["passed"] and j["b6b9_reading"]["reading"] == B.BPRIME_ROUTE, "B': (2) above reads '[개발자] — B′ 한 번 (H:331)'")
    check(B.judge_2(r2b(16, 24))["b6b9_reading"]["reading"].startswith("r one ladder step down"), "bundle B keeps (8)'s reading")
    check(B.judge_2(r2b(10, 30), bprime=True)["b6b9_reading"]["reading"] == "-", "B' inside the band: no route")
    items = r2b(16, 14) + [mk("R", s) for s in range(31, 61)]   # (1) 30/60 fails; (2) 16/30 above; (3) n3 0
    jl_, pp_, rs_ = run_set(items)
    rb = B.judgments(jl_, pp_, rs_, bprime=True)["routes"]
    rB = B.judgments(jl_, pp_, rs_)["routes"]
    check(rb["1"] == B.BPRIME_ROUTE and rb["2"] == B.BPRIME_ROUTE and rb["3 length"].startswith("재지 않음 (n 0)"), "B' routes: %s" % rb)
    check(rB["1"].startswith("[B-6b9]") and rB["2"] == "[B-6b9] (r one ladder step)" and "withheld" not in rB, "bundle B routes: %s" % rB)
    L = []
    for i in range(1, 31):
        L.append(mk("R", i, "RuledPeople", 9000, "civ_7", r4=4000, r4civ="civ_7", r5=4000 + (3700 if i <= 16 else 1800), r5civ="civ_7",
                    decisions=every(4000, 7700) if i > 4 else ()))
    L += [mk("R", s, "RuledPeople", 9000, "civ_7") for s in range(31, 61)]
    jl_, pp_, rs_ = run_set(L)
    rb = B.judgments(jl_, pp_, rs_, bprime=True)["routes"]
    check(rb["1"] is None and rb["3 length"] == B.BPRIME_ROUTE and "결정 밀도" in rb["3 density"], "B': (3) length -> [개발자] once, density -> [개발자]: %s" % rb)
    L[0] = mk("R", 1, "RuledPeople", 9000, "civ_7", r4=4000, r4civ="civ_7", r5=5000, r5civ="civ_7", decisions=every(4000, 5000))
    jl_, pp_, rs_ = run_set(L)
    rs_["R_40"] = {"ok": False}
    rs_["R_41"] = {"ok": False}
    rB = B.judgments(jl_, pp_, rs_)["routes"]
    check("3 construction" in rB and "[1]" in rB["3 construction"] and "withheld" in rB and "R" in rB["withheld"], "violations and a withheld cell are routed: %s" % rB)
    code, out = quiet(B.main, ["--check-only", folder, "--max-seed", "2", "--t-death", "4400", "--r", "0.375", "--root", os.path.join(root, "proj2"), "--archive"])
    check(code == 0 and os.path.exists(os.path.join(root, "proj2", "Saved", "Balance", "b6b_bundleBprime", "manifest.sha1")), "the default archive is b6b_bundleBprime")
    sd = os.path.join(root, "summary_elsewhere")
    os.remove(os.path.join(folder, "summary.json"))
    code, out = quiet(B.main, ["--check-only", folder, "--max-seed", "2", "--t-death", "4400", "--summary-dir", sd, "--root", os.path.join(root, "proj")])
    check(code == 0 and os.path.exists(os.path.join(sd, "summary.json")) and not os.path.exists(os.path.join(folder, "summary.json")),
          "--check-only --summary-dir writes the summary there, not into the checked folder")
    # [review F4 case E] bundle B's archived copy under <root>\Saved\Balance\b6b_bundleB: never overwritten
    proj_e = os.path.join(root, "proj_e")
    arch_b = os.path.join(proj_e, "Saved", "Balance", "b6b_bundleB")
    os.makedirs(arch_b)
    with open(os.path.join(arch_b, "summary.json"), "w", encoding="utf-8") as f:
        json.dump({"header": {"verdict": True}, "judgments": {"1": {"passed": False}}}, f)
    for s in (1, 2):
        write_log(os.path.join(arch_b, "R_%d.log" % s), F, dict(r4=-1, seed=s))
    before = open(os.path.join(arch_b, "summary.json"), encoding="utf-8").read()
    code, out = quiet(B.main, ["--check-only", arch_b, "--share", "0", "--seeds", "1-2", "--cells", "R", "--root", proj_e])
    after = open(os.path.join(arch_b, "summary.json"), encoding="utf-8").read()
    check(code == 2 and "under the project tree" in out and before == after and not os.path.exists(os.path.join(arch_b, "summary.md")),
          "F4 E: --check-only under --root without --summary-dir is refused, the archived summary.json untouched: %s" % out.strip()[-160:])
    code, out = quiet(B.main, ["--check-only", arch_b, "--share", "0", "--seeds", "1-2", "--cells", "R", "--root", proj_e, "--summary-dir", os.path.join(proj_e, "x")])
    check(code == 2 and "under the project tree" in out, "F4: a --summary-dir under --root is refused too")
    copy_dir = os.path.join(root, "archive_copy")   # an archived copy outside the root (manifest.sha1)
    shutil.copytree(arch_b, copy_dir)
    with open(os.path.join(copy_dir, "manifest.sha1"), "w", encoding="utf-8") as f:
        f.write("0  R_1.log\n")
    before = open(os.path.join(copy_dir, "summary.json"), encoding="utf-8").read()
    code, out = quiet(B.main, ["--check-only", copy_dir, "--share", "0", "--seeds", "1-2", "--cells", "R", "--root", proj_e])
    check(code == 2 and "manifest.sha1" in out and before == open(os.path.join(copy_dir, "summary.json"), encoding="utf-8").read(),
          "F4: an archived copy (manifest.sha1) needs --summary-dir")
    code, out = quiet(B.main, ["--check-only", copy_dir, "--share", "0", "--seeds", "1-2", "--cells", "R", "--root", proj_e, "--summary-dir", os.path.join(root, "copy_sum")])
    check(code in (0, 1) and os.path.exists(os.path.join(root, "copy_sum", "summary.json")) and before == open(os.path.join(copy_dir, "summary.json"), encoding="utf-8").read(),
          "F4: with --summary-dir outside the root the copy is read and left as it was")

    print("19. --classify (R1 classes and R2)")
    B_ = 1800
    runs = [mkc(1, r4=-1),                                                                    # T4: no ruled T4
            mkc(2, r4=18201),                                                                 # T4: r4 > 20,000 - 1,800
            mkc(3, r4=18200),                                                                 # T5: r4 on the bound, no binding
            mkc(4, binding=(9000, "ruled", "civ_7", 4), laws=(8000, -1, "-", 0)),             # T5: bound by the ruled people
            mkc(5, binding=(9000, "AI", "civ_3", 4), laws=(-1, 7000, "civ_3", 0)),            # T5-AI-held
            mkc(6, binding=(9000, "former", "civ_4", 4), laws=(-1, -1, "-", 0)),              # T5-AI-never (former is not ruled)
            mkc(7, "Vessel", 8000),                                                           # other: the record came first
            mkc(8, "NoCandidate", 8100),                                                      # other
            mkc(9, binding=(9000, "AI", "civ_3", 4), locked=1),                               # other: locked
            mkc(10, ok=False),                                                                # other: twice-failed
            mkc(11, r4=-1, binding=(9000, "AI", "civ_3", 4), laws=(-1, 7000, "civ_9", 0)),    # T4 by the table's order; AI-bound flagged
            reached(12)]                                                                      # reached: not classified
    unread = mkc(13)
    unread[1]["race_b"]["laws"] = None
    runs.append(unread)                                                                       # other: unreadable
    runs.append(mkc(14, binding=(9000, "AI", "civ_3", 4), laws=(-1, 7000, "civ_9", 0)))       # T5-AI-held, first holder not the binder
    c = B.classify(*run_set(runs), B_, 1200)
    got = {r["seed"]: r["cls"] for r in c["runs"]}
    check(got == {1: "T4", 2: "T4", 3: "T5", 4: "T5", 5: "T5-AI-held", 6: "T5-AI-never", 7: "other", 8: "other", 9: "other", 10: "other", 11: "T4", 13: "other",
                  14: "T5-AI-held"}, "classes: %s" % got)
    check(c["counts"] == {"T4": 3, "T5": 2, "T5-AI-held": 2, "T5-AI-never": 1, "other": 5} and c["ai_bound_in_T4"] == [11] and c["held_first_holder_not_binder"] == [14],
          "counts %s; AI-bound in T4 %s" % (c["counts"], c["ai_bound_in_T4"]))
    why = {r["seed"]: r["why"] for r in c["runs"]}
    check("twice-failed" in why[10] and "locked 1" in why[9] and "Vessel" in why[7] and "unreadable (laws)" in why[13] and "18200" in why[2], "reasons: %s" % why)
    check(c["lever_1"].startswith("넷째 → 다섯째 4 > T4 3") and "R2" in c["lever_1"], "(T5)+(T5-AI-held) 4 > (T4) 3: %s" % c["lever_1"])
    check(sorted((x["seed"], x["from"], x["vote"]) for x in c["r2"]) == [(4, "T5", "laws"), (12, "n3", "binding")], "R2 set: n3 runs and (T5) runs with a ruled binding: %s" % c["r2"])
    check(c["r2_lever"] == "P_bind" and "tie -> P_bind" in c["r2_text"], "R2 1-1 tie -> P_bind: %s" % c["r2_text"])
    c = B.classify(*run_set([mkc(1, r4=-1), mkc(2, r4=-1), mkc(3)]), B_, 1200)
    check(c["lever_1"].startswith("c up"), "T4 2 > 1: %s" % c["lever_1"])
    c = B.classify(*run_set([mkc(1, r4=-1), mkc(3)]), B_, 1200)
    check(c["lever_1"].startswith("tie") and "c first" in c["lever_1"] and "R2 표 없음" in c["lever_1"], "a tie runs both, c first: %s" % c["lever_1"])
    c = B.classify(*run_set([mkc(1, r4=-1), mkc(5, binding=(9000, "AI", "civ_3", 4), laws=(-1, 7000, "civ_3", 0)),
                             mkc(6, binding=(9000, "AI", "civ_3", 4), laws=(-1, 7000, "civ_3", 0))]), B_, 1200)
    check(c["lever_1"].startswith("넷째 → 다섯째 2 > T4 1: N"), "(T5-AI-held) > (T5) -> N: %s" % c["lever_1"])
    never = [mkc(s, binding=(9000, "AI", "civ_3", 4)) for s in range(1, 4)]
    c = B.classify(*run_set(never + [mkc(4, r4=-1), mkc(5, "Vessel", 100), mkc(6, "Vessel", 100), mkc(7, "Vessel", 100)]), B_, 1200)
    check(c["lever_1"].startswith("[개발자]") and c["counts"]["T5-AI-never"] == 3 and c["counts"]["other"] == 3, "(T5-AI-never) largest (tie with other) -> [개발자]: %s" % c["lever_1"])
    c = B.classify(*run_set([reached(s) for s in range(1, 5)]), B_, 1200)
    check(c["runs"] == [] and c["lever_1"].startswith("none") and c["counts"] == {k: 0 for k in B.CLASSES}, "no unreached run: empty classes, no lever")
    c = B.classify(*run_set([]), B_, 1200)
    check(c["runs"] == [] and c["r2_lever"] is None and "R2 표 없음" in c["r2_text"], "an empty cell: nothing to classify")
    bindp = {"race_b": {"binding": {"tick": 3000, "by": "ruled"}, "laws": {"ruled": 1000, "resets_r": 0}}}
    votes = []
    for ruled, resets, btick, by in ((1000, 0, 3000, "ruled"), (1000, 0, 2000, "ruled"), (1000, 0, 2200, "ruled"), (1000, 2, 9000, "ruled"), (-1, 0, 3000, "ruled"),
                                     (1000, 0, 3000, "AI"), (1000, 0, -1, "-")):
        bindp["race_b"]["binding"].update(tick=btick, by=by)
        bindp["race_b"]["laws"].update(ruled=ruled, resets_r=resets)
        votes.append(B.r2_vote(bindp, 1200)[0])
    check(votes == ["binding", "laws", "even", "laws", "laws", None, None], "R2 votes (binding last, laws last, even, resets, never held, AI binding, none): %s" % votes)
    check(B.r2_majority(["laws", "laws", "binding"])[0] == "N" and B.r2_majority(["binding", "binding", "laws"])[0] == "P_bind"
          and B.r2_majority(["laws", "binding"])[0] == "P_bind" and B.r2_majority([])[0] is None, "R2 majority, tie and empty")
    # Through main on written logs: an archive-like folder (logs only) and runs.json marking one run twice-failed.
    cf = os.path.join(root, "classify_in")
    os.makedirs(cf)
    logs = {1: dict(r4=-1), 2: dict(r4=5000, r4_civ="civ_7", binding=(9000, "AI", "civ_3", 4), laws_ai=7000, laws_ai_civ="civ_3"),
            3: dict(r4=5000, r4_civ="civ_7", binding=(9000, "ruled", "civ_7", 4), laws_ruled=8500, ruled_binding=7000),
            4: dict(kind="Vessel", rec=7000), 5: dict(r4=-1)}
    for s, v in logs.items():
        write_log(os.path.join(cf, "R_%d.log" % s), F, dict(v, seed=s))
    with open(os.path.join(cf, "runs.json"), "w", encoding="utf-8") as f:
        json.dump({"R_5": {"ok": False}}, f)
    out_dir = os.path.join(root, "classify_out")
    code, out = quiet(B.main, ["--classify", cf, "--seeds", "1-5", "--p-bind", "1800", "--n-laws", "1200", "--summary-dir", out_dir])
    cj = json.load(open(os.path.join(out_dir, "classify.json"), encoding="utf-8")) if code == 0 else {}
    check(code == 0 and cj.get("counts") == {"T4": 1, "T5": 1, "T5-AI-held": 1, "T5-AI-never": 0, "other": 2} and "5891b075" in out,
          "--classify over logs only: %s (exit %s) %s" % (cj.get("counts"), code, out[-300:]))
    check(bool(cj.get("r2")) and cj["r2"][0]["vote"] == "laws" and "8500 + 1200" in cj["r2"][0]["why"], "R2 read from the log's laws and binding parts: %s" % cj.get("r2"))
    check("<R1_COUNTS>" in out and "| 2 | (T5-AI-held) |" in out, "the markdown table names each class")
    code, out = quiet(B.main, ["--classify", cf, "--seeds", "1-5", "--summary-dir", out_dir])
    check(code == 2 and "--p-bind and --n-laws" in out, "--classify without bundle B's values is refused")
    code, out = quiet(B.main, ["--classify", cf, "--seeds", "1-5", "--p-bind", "1800", "--n-laws", "1200", "--brief", os.path.join(rd, "s13.md")])
    check(code == 2 and "refusing to classify" in out and "the readings changed" in out, "--classify refuses a brief whose readings moved")
    code, out = quiet(B.main, ["--classify", cf, "--seeds", "1-5", "--p-bind", "1800", "--n-laws", "1200", "--root", root, "--summary-dir", os.path.join(root, "x")])
    check(code == 2 and "under the project tree" in out, "--classify never writes under --root")
    # [review F1 case A] one log of five, no runs.json: refused, nothing written
    cm = os.path.join(root, "classify_missing")
    os.makedirs(cm)
    write_log(os.path.join(cm, "R_1.log"), F, dict(r4=-1, seed=1))
    code, out = quiet(B.main, ["--classify", cm, "--seeds", "1-5", "--p-bind", "1800", "--n-laws", "1200", "--summary-dir", os.path.join(root, "cm_out")])
    check(code == 2 and "missing logs R_2, R_3, R_4, R_5" in out and not os.path.exists(os.path.join(root, "cm_out", "classify.json")),
          "F1 A: missing logs are refused, not counted twice-failed: %s" % out.strip()[-200:])
    # [review F1 case A2] an empty folder with --seeds 1-60
    ce = os.path.join(root, "classify_empty")
    os.makedirs(ce)
    code, out = quiet(B.main, ["--classify", ce, "--seeds", "1-60", "--p-bind", "1800", "--n-laws", "1200", "--summary-dir", os.path.join(root, "ce_out")])
    check(code == 2 and "missing logs R_1, R_2" in out and " ..." in out, "F1 A2: an empty folder is refused: %s" % out.strip()[-200:])
    code, out = quiet(B.main, ["--classify", cf, "--seeds", "", "--p-bind", "1800", "--n-laws", "1200", "--summary-dir", os.path.join(root, "ce_out")])
    check(code == 2 and "names no R run" in out, "F1: an empty --seeds is refused")
    # A missing log that runs.json records as failed after two attempts stays (other) 'twice-failed' (brief 1.1)
    with open(os.path.join(cm, "runs.json"), "w", encoding="utf-8") as f:
        json.dump({"R_2": {"ok": False, "attempts": [{"attempt": 1}, {"attempt": 2}]}}, f)
    code, out = quiet(B.main, ["--classify", cm, "--seeds", "1-2", "--p-bind", "1800", "--n-laws", "1200", "--summary-dir", os.path.join(root, "cm_out2")])
    cj = json.load(open(os.path.join(root, "cm_out2", "classify.json"), encoding="utf-8")) if code == 0 else {}
    check(code == 0 and [(r["seed"], r["cls"], r["why"]) for r in cj.get("runs", [])][1:] == [(2, "other", "twice-failed")],
          "F1: a recorded twice-failed run without a log is classified (other) twice-failed: %s" % out.strip()[-200:])
    with open(os.path.join(cm, "runs.json"), "w", encoding="utf-8") as f:
        json.dump({"R_2": {"ok": False, "attempts": []}}, f)
    code, out = quiet(B.main, ["--classify", cm, "--seeds", "1-2", "--p-bind", "1800", "--n-laws", "1200", "--summary-dir", os.path.join(root, "cm_out3")])
    check(code == 2 and "missing logs R_2" in out, "F1: runs.json ok false without two attempts does not excuse a missing log")
    os.remove(os.path.join(cm, "runs.json"))
    # A log-only failure (no runs.json entry) carries its reason, not 'twice-failed'
    write_log(os.path.join(cm, "R_2.log"), F, dict(r4=-1, seed=2), extra=[T.ue("CosmicBalance end check: x", "Error")])
    code, out = quiet(B.main, ["--classify", cm, "--seeds", "1-2", "--p-bind", "1800", "--n-laws", "1200", "--summary-dir", os.path.join(root, "cm_out4")])
    cj = json.load(open(os.path.join(root, "cm_out4", "classify.json"), encoding="utf-8")) if code == 0 else {}
    why2 = {r["seed"]: (r["cls"], r["why"]) for r in cj.get("runs", [])}.get(2, ("", ""))
    check(code == 0 and why2[0] == "other" and why2[1].startswith("failed: ") and "end check" in why2[1] and "twice-failed" not in why2[1],
          "F1: a log-only failure reads 'failed: <why>': %s" % (why2,))

    print("20. witnesses (4.3)")
    wr = [mkc(1, "RuledPeople", 5850, "civ_7", r4=4000, r5=5800, binding=(5700, "ruled", "civ_7", 4), ruled_binding=3900, decisions=every(4000, 5800)),
          mkc(2, binding=(9000, "ruled", "civ_7", 4), ruled_binding=7000)]
    w = B.witnesses(*run_set(wr), 1800, 1200)
    check(w["ok"] and w["checked"] == {"binding": 2, "season": 1}, "binding - start 1,800 and 2,000 >= 1,800; season 1,800 >= max(1,800, 1,200): %s" % w)
    w = B.witnesses(*run_set(wr), 1900, 1200)
    check(not w["ok"] and len(w["problems"]) == 2 and "< P_bind 1900" in w["problems"][0] and "< max(P_bind, N) 1900" in w["problems"][1], "P_bind 1,900: %s" % w["problems"])
    w = B.witnesses(*run_set(wr + [mkc(3, binding=(9000, "ruled", "civ_7", 4), ruled_binding=-1), mkc(4, binding=(9000, "AI", "civ_3", 4), ruled_binding=8000)]), 1800, None)
    check(w["ok"] and w["checked"] == {"binding": 2, "season": 0}, "no start printed or an AI binding: not a witness; N unknown: no season check")
    check(B.witnesses(*run_set(wr), None, None)["ok"], "no values: nothing checked")
    res = {"pairs": [], "bridges": [], "controls": {"ok": True}, "witnesses": B.witnesses(*run_set(wr), 1900, 1200)}
    check(not B.verdict(res, {}), "a witness mismatch fails the verdict (R5's identical rerun)")

    print("21. ladders and the floor (1.3, 1.4, R3)")
    check(B.ladder_c(1e-4) == [1.5e-4, 2e-4, 2.5e-4] and B.ladder_c(2e-4) == [2.5e-4] and B.ladder_c(2.5e-4) == [], "c ladder to the ceiling 2.5e-4: %s" % B.ladder_c(1e-4))
    check(B.ladder_drop(1800, 1200) == ([1500, 1200], False) and B.ladder_drop(1200, 1800) == ([900, 600], False), "P_bind 1,800 -> 1,500 -> 1,200; N 1,200 -> 900 -> 600")
    check(B.ladder_drop(1200, 900) == ([], True) and B.ladder_drop(1500, 1100) == ([1200], True), "R3 stops a ladder at max < 1,200")
    check(B.floor_ok(1200, 600) and B.floor_ok(900, 1200) and not B.floor_ok(1199, 1100), "floor: max(P_bind, N) >= 1,200")
    code, out = quiet(B.main, ["--ladders", "--c-a", "1e-4", "--p-bind-a", "1800", "--n-a", "1200"])
    check(code == 0 and "0.0001 -> 0.00015 -> 0.0002 -> 0.00025" in out and "1800 -> 1500 -> 1200" in out and "1200 -> 900 -> 600" in out, "--ladders: %s" % out.strip())
    check(quiet(B.main, ["--ladders", "--c-a", "1e-4"])[0] == 2, "--ladders needs all three start values")
    code, out = quiet(B.main, ["--probe", "--probe-label", "x", "--p-bind", "1100", "--n-laws", "900", "--root", os.path.join(root, "proj")])
    check(code == 2 and "R3" in out, "a probe below the floor is refused: %s" % out.strip())

    print("22. R4's acceptance (1.1)")
    base = {"1": {"passed": False}, "2": {"passed": True}, "3": {"length_ok": True, "density_ok": True}, "4": {"passed": True}}

    def row(j1=True, j2=True, side=None, ln=True, dn=True, j4=True, withheld=(), n2=40):
        return {"1": {"passed": j1}, "2": {"passed": j2, "side": side, "n2": n2}, "3 length": {"passed": ln}, "3 density": {"passed": dn}, "4": {"passed": j4},
                "withheld": list(withheld)}
    check(B.r4_decision(row(), base, "1", {"c"}) == "accepted", "target passes, nothing broke: accepted")
    check(B.r4_decision(row(j1=False), base, "1", {"c"}).startswith("next step"), "target still fails: next step")
    d = B.r4_decision(row(dn=False), base, "1", {"c"})
    check(d.startswith("[개발자]") and "3 density" in d and "stop at this step" in d, "target passes but (3) density breaks: %s" % d)
    d = B.r4_decision(row(j2=False, side="below"), base, "1", {"c"})
    check(d.startswith("[개발자]") and "below the band after a change of c" in d, "(2) below after c: %s" % d)
    d = B.r4_decision(row(j2=None, side=None, n2=20), dict(base, **{"2": {"passed": None}}), "1", {"N"})
    check(d.startswith("[개발자]") and "재지 않음" in d, "(2) unmeasured after N: %s" % d)
    base_r = dict(base, **{"2": {"passed": False}})
    check(B.r4_decision(row(j2=False, side="above"), base_r, None, {"r"}, "down").startswith("accepted"), "r down, (2) still above (same side): B' still runs")
    d = B.r4_decision(row(j2=False, side="below"), base_r, None, {"r"}, "down")
    check(d.startswith("[개발자]") and "overshoot" in d, "r down, (2) below (opposite side): %s" % d)
    check(B.r4_decision(row(withheld=["R"]), base, "1", {"c"}).startswith("[개발자]: a cell is withheld"), "a withheld probe cell: [개발자]")
    d = B.r4_decision(row(j1=False), dict(base, **{"1": {"passed": True}, "3": {"length_ok": False, "density_ok": True}}), "3len", {"P_bind"})
    check(d.startswith("[개발자]") and "breaks 1" in d, "a (3) step that breaks (1), which passed: %s" % d)
    # [review F2 B2-B5] inputs that used to read 'accepted' now raise
    for args, needle, label in (((row(j2=False, side="below"), base_r, "1", {"n"}), "not among", "B2 {'n'}"),
                                ((row(j2=False, side="above"), base_r, "1", {"p_bind"}), "not among", "B3 {'p_bind'}"),
                                ((row(j1=False), base, None, {"c"}), "--target", "B4 target None with c"),
                                ((row(j2=False, side="below"), base_r, None, {"r"}, None), "--r-step", "B5 r without r_step"),
                                ((row(), base, "1", set()), "--changed", "nothing changed"),
                                ((row(), base, "1", {"c"}, "down"), "without r", "r_step without r")):
        try:
            got = B.r4_decision(*args)
        except ValueError as e:
            got = "ValueError: %s" % e
        check(got.startswith("ValueError") and needle in got, "F2 %s: %s" % (label, got))
    d = B.r4_decision(row(j2=False, side="below"), base_r, "1", {"r", "c"}, "down")
    check(d.startswith("[개발자]") and "below the band after a change of c" in d, "r with c: the either-side rule of c applies: %s" % d)

    print("23. a probe and a B' end to end (the commandlet mocked)")
    proj, editor = fake_project(root)
    saved = (B.run_command, B.A.busy_processes, B.A.free_memory_gb, B.A.git_head)
    try:
        B.A.free_memory_gb = lambda: (32.0, 64.0)
        B.A.git_head = lambda r: "fake head"
        B.A.busy_processes = lambda: ["UnrealEditor.exe (pid 1)"]
        fr = FakeRunner(F, {})
        B.run_command = fr
        code, out = quiet(B.main, ["--probe", "--probe-label", "busy", "--seeds", "1-2", "--root", proj, "--editor", editor, "--folder", os.path.join(root, "pbusy"),
                                   "--p-bind", "1800", "--n-laws", "1200", "--probe-rows", os.path.join(root, "busy_rows.jsonl")])
        check(code == 2 and "UnrealEditor.exe" in out and fr.cmds == [], "an editor running: refused before any run")
        B.A.busy_processes = lambda: []
        on = dict(kind="RuledPeople", rec=8000, rec_civ="civ_7", r3=2000, r3_civ="civ_7", r4=4000, r4_civ="civ_7", r5=8000, r5_civ="civ_7",
                  binding=(8000, "ruled", "civ_7", 4), decisions=[(4100, 1)])
        fr = FakeRunner(F, {"slowR_2": {"exit": 3}}, default_v=on)
        B.run_command = fr
        rows_path = os.path.join(root, "probe_rows.jsonl")
        bsum = os.path.join(root, "bundleB_summary.json")
        with open(bsum, "w", encoding="utf-8") as f:
            json.dump({"judgments": base}, f)
        pf = os.path.join(root, "probe1")
        code, out = quiet(B.main, ["--probe", "--probe-label", "P1 r 0.3125", "--seeds", "1-2", "--workers", "2", "--root", proj, "--editor", editor, "--folder", pf,
                                   "--rules-extra", "AIUpperRungProgressMultiplier:0.3125", "--probe-rows", rows_path, "--p-bind", "1800", "--n-laws", "1200",
                                   "--accept-against", bsum, "--target", "1", "--changed", "r", "--r-step", "down"])
        names = sorted(os.path.basename([c for c in cmd if c.startswith("-AbsLog=")][0]) for cmd in fr.cmds)
        check(code == 0 and names == ["R_1.log", "R_2.log", "slowR_1.log", "slowR_2.log", "slowR_2.log"], "probe ran R and slowR on side only, slowR_2 twice: %s (exit %s)" % (names, code))
        check(all(sum(c.startswith("-Rules=") for c in cmd) == 1 and "-Rules=ProvisionalEndTick:0;AIUpperRungProgressMultiplier:0.3125" in cmd for cmd in fr.cmds),
              "each command carries the one joined -Rules= argument")
        runs_json = json.load(open(os.path.join(pf, "runs.json"), encoding="utf-8"))
        check(runs_json["slowR_2"]["ok"] is False and len(runs_json["slowR_2"]["attempts"]) == 2, "a twice-failed run: two attempts, not ok")
        md = open(os.path.join(pf, "summary.md"), encoding="utf-8").read()
        jl_lines = [l for l in md.splitlines() if l.startswith("- ") and any(s in l for s in ("① R", "② R", "③ R", "④", "② 느린"))]
        check(len(jl_lines) == 5 and all(l.startswith("- [탐침 — 판단 아님] ") for l in jl_lines) and "## 탐침의 줄" in md and "탐침 — 판단 아님" in md.splitlines()[0],
              "every judgment line carries the probe label: %s" % jl_lines)
        rows = [json.loads(l) for l in open(rows_path, encoding="utf-8")]
        check(len(rows) == 1 and rows[0]["probe"] == "P1 r 0.3125" and rows[0]["label"] == "탐침 — 판단 아님" and set(rows[0]) >= {"1", "2", "3 length", "3 density", "4", "r4"}
              and rows[0]["2"]["ga"] == 0 and len(rows[0]["guard_sha1s"]) == 3, "one probe row with (1)-(4), the split, R4 and the guard sha1s: %s" % sorted(rows[0]))
        check("R4:" in md and "R3: max(P_bind 1800, N 1200)" in md, "the probe summary carries R4 and R3")
        meta = json.load(open(os.path.join(pf, "bundle_meta.json"), encoding="utf-8"))
        check(sorted(meta["guard_sha1s"]) == ["Binaries/Win64/UnrealEditor-Cosmic_Ascension.dll", "Content/CA/Data/DT_Rules.uasset", "Tools/data/generated.sha1"]
              and meta["sha1_ok"], "bundle_meta holds every guarded sha1: %s" % sorted(meta["guard_sha1s"]))
        fr = FakeRunner(F, {"R_1": {"drop_rules": True}}, default_v=on)
        B.run_command = fr
        code, out = quiet(B.main, ["--probe", "--probe-label", "P2", "--seeds", "1", "--cells", "R", "--root", proj, "--editor", editor, "--folder", os.path.join(root, "probe2"),
                                   "--rules-extra", "AIUpperRungProgressMultiplier:0.3125", "--probe-rows", rows_path, "--p-bind", "1800", "--n-laws", "1200"])
        check(code == 1 and "-Rules did not take" in out, "a log without its rules line fails the probe")
        dll = os.path.join(proj, "Binaries", "Win64", "UnrealEditor-Cosmic_Ascension.dll")

        def touch(name):
            with open(dll, "a", encoding="utf-8") as f:
                f.write("rebuilt\n")
        fr = FakeRunner(F, {}, default_v=on, touch=touch)
        B.run_command = fr
        code, out = quiet(B.main, ["--probe", "--probe-label", "P3", "--seeds", "1", "--cells", "R", "--root", proj, "--editor", editor, "--folder", os.path.join(root, "probe3"),
                                   "--probe-rows", rows_path, "--p-bind", "1800", "--n-laws", "1200"])
        check(code == 1 and "CHANGED - discard the bundle" in out, "a DLL that changes during the runs fails the sha1 guard")
        # B': R and input-less on seeds 61-62; R_61 differs below T_B with the twin's awakening sum 80 (allowed);
        # inputless_62 differs with sum 70 (a control failure), then with 90 (allowed).
        conflict = lambda s: [T.ue(T.cprintf(F.conflict, {"delay bought per tag [": "Hazard.AIAwakening=%d(1)" % s}))]  # noqa: E731
        plain = dict(kind="None", rec=-1, was_share=0.5, was_r=0.375)
        sc = {"R_61": {"changes": [(500, "X")]}, "R_61_off": {"extra": conflict(80)}, "inputless_62": {"changes": [(700, "Y")]},
              "inputless_62_off": {"extra": conflict(70)}}
        bp = os.path.join(root, "bprime")
        arch = os.path.join(root, "Saved", "Balance", "b6b_bundleBprime")
        fr = FakeRunner(F, sc, default_v=plain)
        B.run_command = fr
        argv = ["--bprime", "--share", "0.5", "--r", "0.375", "--seeds", "61-62", "--cells", "R,inputless", "--workers", "2", "--root", proj, "--editor", editor,
                "--folder", bp, "--archive-dir", arch, "--k-moved-r", "3", "--k-moved-w", "2", "--p-bind", "1800", "--n-laws", "1200"]
        code, out = quiet(B.main, argv)
        sj = json.load(open(os.path.join(bp, "summary.json"), encoding="utf-8"))
        bad = {p["pair"]: p["why"] for p in sj["pairs"] if not p["ok"]}
        check(code == 1 and list(bad) == ["inputless_62"] and any("sum 70 is not > 75" in w for w in bad["inputless_62"]), "B': sum 70 is a control failure: %s %s" % (bad, sj["controls"]["problems"]))
        check(sj["record"]["moved by the share"] == {"count": 1, "by cell": {"R": ["R_61"]}} and "<BP_SHARE_MOVED> 1" in out and "3/2" in out,
              "B': R_61 counted moved by the share, against K 3/2")
        check(os.path.exists(os.path.join(arch, "manifest.sha1")) and sj["header"]["mode"] == "bprime" and "묶음 B′" in out, "B' archived to b6b_bundleBprime")
        sc["inputless_62_off"] = {"extra": conflict(90)}
        code, out = quiet(B.main, argv)
        check(code == 0 and "bprime: PASS" in out, "B' with every differing pair allowed passes: %s" % out.strip()[-300:])
        code, out = quiet(B.main, ["--check-only", bp, "--bprime", "--share", "0", "--r", "0.375", "--seeds", "61-62", "--cells", "R,inputless", "--root", proj,
                                   "--summary-dir", os.path.join(root, "bp_share0")])
        check(code == 1 and "FleetDelayCountdownShare was 0.5, not 0" in out, "the same B' read with --share 0: every off log flagged")
        # [review F3 cases C, D] through the real-run path (mocks on): refused before any command
        fr = FakeRunner(F, {}, default_v=plain)
        B.run_command = fr
        common = ["--bprime", "--share", "0.5", "--seeds", "61-62", "--cells", "R,inputless", "--root", proj, "--editor", editor, "--no-archive",
                  "--p-bind", "1800", "--n-laws", "1200"]
        code, out = quiet(B.main, common + ["--on-only", "--r", "0.4375", "--folder", os.path.join(root, "bp_on_only")])
        check(code == 2 and fr.cmds == [] and "--on-only" in out, "F3 C: B' --on-only refused, no command run: %s" % out.strip()[-160:])
        code, out = quiet(B.main, common + ["--r", "0.375", "--rules-extra", "AIUpperRungProgressMultiplier:0.3125", "--folder", os.path.join(root, "bp_rules")])
        check(code == 2 and fr.cmds == [] and "never --rules-extra" in out, "F3 D: B' --rules-extra refused, no command run")
        # [review F5 case G] a judged probe whose (2) lies above the band (16/40, all ga): no r reading, no route
        oth = dict(kind="OtherPeople", rec=9000, rec_civ="civ_3", some=0, r3=2000, r3_civ="civ_7", r4=7000, r4_civ="civ_7", binding=(8000, "AI", "civ_3", 4))
        fr = FakeRunner(F, {"R_%d" % s: {"v": oth} for s in range(1, 17)}, default_v=on)
        B.run_command = fr
        pg = os.path.join(root, "probe_g")
        rows_g = os.path.join(root, "probe_rows_g.jsonl")
        code, out = quiet(B.main, ["--probe", "--probe-label", "G", "--seeds", "1-40", "--cells", "R", "--workers", "2", "--root", proj, "--editor", editor, "--folder", pg,
                                   "--probe-rows", rows_g, "--p-bind", "1800", "--n-laws", "1200"])
        md = open(os.path.join(pg, "summary.md"), encoding="utf-8").read()
        sj = json.load(open(os.path.join(pg, "summary.json"), encoding="utf-8"))
        j2 = sj["judgments"]["2"]
        check(len(fr.cmds) == 40 and j2["judged"] and j2["side"] == "above" and len(j2["split"]["ga"]) == 16, "G: the probe's (2) is judged above the band: %s %s" % (j2["share"]["text"], j2["side"]))
        check("r one ladder step" not in md and "B' on seeds 61-120" not in md and "  - [탐침 — 판단 아님] (가)만의 몫 0.400 — 탐침은 r을 옮기지 않는다 (R4)" in md,
              "F5 G: the probe summary has no r reading, only the labelled share: %s" % [l for l in md.splitlines() if "(가)만의" in l])
        check("routes" not in sj["judgments"] and "r one ladder step" not in json.dumps(sj, ensure_ascii=False) and j2["b6b9_reading"]["reading"].startswith("[탐침 — 판단 아님]"),
              "F5: summary.json of a probe has no routes and no r reading")
        # [review F6] the 6-probe cap (brief 1.6): a 7th new label is refused, a rerun of a used label is not
        rows_cap = os.path.join(root, "probe_rows_cap.jsonl")
        with open(rows_cap, "w", encoding="utf-8") as f:
            for i in (1, 2, 3, 3, 4, 5, 6):
                f.write(json.dumps({"probe": "P%d" % i, "label": B.PROBE_LABEL}) + "\n")
        fr = FakeRunner(F, {}, default_v=on)
        B.run_command = fr
        cap = ["--probe", "--seeds", "1", "--cells", "R", "--root", proj, "--editor", editor, "--probe-rows", rows_cap, "--p-bind", "1800", "--n-laws", "1200"]
        code, out = quiet(B.main, cap + ["--probe-label", "P7", "--folder", os.path.join(root, "probe_p7")])
        check(code == 2 and fr.cmds == [] and "6-probe cap" in out and "[개발자]" in out, "F6: a 7th probe is refused before any run: %s" % out.strip()[-160:])
        code, out = quiet(B.main, cap + ["--probe-label", "P7", "--dry-run"])
        check(code == 2 and "6-probe cap" in out, "F6: the dry run of a 7th probe is refused too")
        code, out = quiet(B.main, cap + ["--probe-label", "P3", "--folder", os.path.join(root, "probe_p3")])
        check(code in (0, 1) and len(fr.cmds) == 1 and sum(1 for _ in open(rows_cap, encoding="utf-8")) == 8, "F6: a used label reruns (one run, one more row)")
        with open(rows_cap, "a", encoding="utf-8") as f:
            f.write("not json\n")
        code, out = quiet(B.main, cap + ["--probe-label", "P3", "--dry-run"])
        check(code == 2 and "cannot be counted" in out, "F6: a rows file that cannot be counted is refused")
        rows5 = os.path.join(root, "probe_rows_5.jsonl")
        with open(rows5, "w", encoding="utf-8") as f:
            for i in range(1, 6):
                f.write(json.dumps({"probe": "P%d" % i}) + "\n")
        code, out = quiet(B.main, cap[:-6] + ["--probe-rows", rows5, "--p-bind", "1800", "--n-laws", "1200", "--probe-label", "P6", "--dry-run"])
        check(code == 0, "F6: the 6th probe is allowed: %s" % out.strip()[-120:])
    finally:
        B.run_command, B.A.busy_processes, B.A.free_memory_gb, B.A.git_head = saved

    print("24. --check-only over scratchpad\\b6b8\\bundleB with --share 0 (bundle B's own verdict)")
    b8 = os.path.join(SCRATCH, "b6b8", "bundleB")
    if os.path.exists(os.path.join(b8, "summary.json")):
        want = json.load(open(os.path.join(b8, "summary.json"), encoding="utf-8"))["header"]["verdict"]
        code, out = quiet(B.main, ["--check-only", b8, "--share", "0", "--summary-dir", os.path.join(root, "b8_check")])
        check((code == 0) == bool(want), "bundle B's verdict reproduced: %s vs %s" % (code, want))
    else:
        print("  skipped: %s has no bundle (cleared or not run yet) - the archive holds logs only" % b8)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cpp", default=None)
    a = ap.parse_args()
    cpp_path = a.cpp or next((p for p in (os.path.join(HERE, "ref", "cmdlet_head.cpp"), os.path.join(SCRATCH, "b6b6", "cmdlet_head.cpp")) if os.path.exists(p)), None)
    cpp = T.load_cpp(cpp_path)
    F = T.Formats(cpp)
    F.deaths = T.fmt_on_line(cpp, 'TEXT("CosmicBalance calling deaths:')
    root = os.path.join(HERE, "selftest")
    if os.path.exists(root):
        shutil.rmtree(root)
    os.makedirs(root)

    print("1. job list")
    full = B.build_jobs(r"C:\x\bundleB", 4400)
    by_cell = {}
    for j in full:
        by_cell[j["cell"]] = by_cell.get(j["cell"], 0) + 1
    check(len(full) == 540 and by_cell == {"R": 120, "slowR": 120, "wanderer": 120, "inputless": 120, "death": 60}, "540 jobs: %s" % by_cell)
    check(len({j["name"] for j in full}) == 540 and all(j["ticks"] == 20000 for j in full), "unique names, all 20,000")
    check(all((B.OFF_ARG in j["args"]) == j["off"] for j in full) and not any(j["off"] for j in full if j["cell"] == "death"), "off flags; no off death")
    check(all(j["args"][0] == "-Rules=ProvisionalEndTick:0" for j in full), "every run without the clock")
    get = lambda n: next(j for j in full if j["name"] == n)  # noqa: E731
    check(get("slowR_1")["args"] == ["-Rules=ProvisionalEndTick:0", "-Answer=1", "-Build=1", "-Focus=0", "-Edict=1", "-Diplomacy=1", "-TellAll=0"], "slowR = R with -Focus=0")
    check(get("death_7")["args"] == ["-Rules=ProvisionalEndTick:0"] + B.R_ARGS + ["-DeathAt=4400"] and "-Wander=1" not in get("death_7")["args"], "death = R + -DeathAt, no -Wander")
    check(get("wanderer_3_off")["args"][-3:] == ["-Abdicate=1200", "-Wander=1", B.OFF_ARG], "wanderer off")
    check(get("inputless_2")["args"] == ["-Rules=ProvisionalEndTick:0"], "input-less: no policy")
    check(len(B.build_jobs("x", None, seeds=B.parse_seeds("61-120"))) == 540 and B.build_jobs("x", None)[-1]["args"][-1] == "-DeathAt=<T_DEATH>",
          "B' seeds 61-120; unset T_DEATH shows as a placeholder")
    check(full[0]["name"] == "R_1" and full[1]["name"] == "R_1_off", "pairs adjacent")

    print("2. parser and run failures")
    folder = os.path.join(root, "clean")
    jobs = clean_bundle(F, folder)
    p = B.parse_log(jobs["wanderer_2"]["log"])
    s = p["race_b"]["side"]
    check(s["layout"] == "new" and s["wit"] == 2 and s["wit_first"] == 1500 and s["answered"] == 2 and s["nocheck"] == 1 and s["failed"] == 1, "new side layout %s" % s)
    check(p["race_b"]["binding"]["tick"] == 8000 and p["race_b"]["binding"]["by"] == "ruled" and p["race_b"]["binding"]["rung"] == 4 and p["race_b"]["binding"]["system"] == "sys_1",
          "binding %s" % p["race_b"]["binding"])
    check(p["left_at"] == 1200 and p["ending_x"]["kind"] == "RuledPeople" and p["race"]["ruled"]["r5"] == 8000, "left at, ending head, r5")
    p1 = B.parse_log(jobs["R_1"]["log"])
    check(p1["race_b"]["decisions"]["pairs"] == [(2100, 1), (2500, 2)] and p1["race_b"]["p162"] == {"civ": "civ_3", "shown": 1, "once": 0}, "decisions and #162")
    check(p1["race_b"]["laws"]["ruled"] == -1, "laws")
    pd = B.parse_log(jobs["death_2"]["log"])
    check(pd["death"] == {"at": 4400, "recorded": 0}, "death asked %s" % pd["death"])
    po = B.parse_log(jobs["R_1_off"]["log"])
    check(po["rows_taken_out"] == B.EXPECTED_REMOVED and po["off_knobs"] == {"r": "0.375", "share": "0"}, "off line: ten names, r 0.375, share 0: %s" % po["off_knobs"])
    old = os.path.join(root, "old.log")
    write_log(old, F, dict(layout="old", era=0, wit=3, kind="Vessel", rec=7000))
    po = B.parse_log(old)
    check(po["race_b"]["side"]["layout"] == "old" and po["race_b"]["side"]["wit"] == 3 and po["race_b"]["side"]["wit_first"] is None, "old side layout read")
    check(all(B.run_failures(j, B.parse_log(j["log"]), 0) == [] for j in jobs.values()), "every clean run passes")
    weird = os.path.join(root, "weird")
    wj = B.build_jobs(weird, 4400, cells={"R"}, max_seed=1)[0]
    write_job(F, wj, dict(kind="Sideways", rec=100))
    check(any("outside" in w for w in B.run_failures(wj, B.parse_log(wj["log"]), 0)), "an unknown ending kind fails the run")
    write_job(F, wj, dict(drop_binding=True))
    check(any("B parts unreadable: binding" in w for w in B.run_failures(wj, B.parse_log(wj["log"]), 0)), "an unreadable binding part fails the run")
    write_job(F, wj, dict(), extra=[T.ue("CosmicBalance end check: x", "Error")])
    w = B.run_failures(wj, B.parse_log(wj["log"]), 3)
    check(any("exit code 3" in x for x in w) and any("end check" in x for x in w) and any("LogCosmic: Error" in x for x in w), "bundle A's rules carried: %s" % w)

    print("3. check-only over the clean bundle (pairs, bridges, controls)")
    code = B.main(["--check-only", folder, "--max-seed", "2", "--t-death", "4400", "--r", "0.375", "--root", os.path.join(root, "proj"),
                   "--archive", "--archive-dir", os.path.join(root, "archive")])
    with open(os.path.join(folder, "summary.json"), encoding="utf-8") as f:
        sj = json.load(f)
    check(code == 0, "clean bundle passes (exit %s): controls %s; pairs %s; bridges %s" % (code, sj["controls"]["problems"], [(x["pair"], x["why"]) for x in sj["pairs"] if not x["ok"]],
                                                                                     [(x["pair"], x["why"]) for x in sj["bridges"] if not x["ok"]]))
    tb = {x["pair"]: x["T_B"] for x in sj["pairs"]}
    check(tb == {"R_1": 900, "R_2": 1801, "slowR_1": 1801, "slowR_2": 1801, "wanderer_1": 900, "wanderer_2": 1500, "inputless_1": 1801, "inputless_2": 1801},
          "T_B per cell: %s" % tb)
    check(len(sj["bridges"]) == 4 and all(b["ok"] for b in sj["bridges"]), "death < 4,400 and wanderer < 1,200 bridges pass")
    check(any("recorded 0" in n and "[2]" in n for n in sj["controls"]["notes"]), "death recorded 0 is a note: %s" % sj["controls"]["notes"])
    check(os.path.exists(os.path.join(root, "archive", "manifest.sha1")) and os.path.exists(os.path.join(root, "archive", "R_1.log")), "archive: logs and manifest")
    man = open(os.path.join(root, "archive", "manifest.sha1"), encoding="utf-8").read().split("\n")
    check(len([l for l in man if l.strip()]) == len([f for f in os.listdir(folder) if os.path.isfile(os.path.join(folder, f))]), "the manifest lists every output file")
    check(not os.path.exists(os.path.join(root, "proj")), "nothing written under --root")
    rec = sj["record"]
    left = rec["vessels left at T4+"]
    check(len(left) == 1 and left[0]["seed"] == 1 and left[0]["tier"] == 4 and left[0]["civ"] == "civ_7", "a vessel that left a T4 people: %s" % left)
    check(rec["B1 preview"]["pairs"] == 2 and rec["B1 preview"]["L (only off)"] == 1 and rec["B1 preview"]["G (only on)"] == 0, "B1 preview %s" % rec["B1 preview"])
    check(sj["record"]["death vs R on"] == [], "the death cell's records equal R on's here")

    print("4. pairs and bridges that break")
    parsed = {n: B.parse_log(j["log"]) for n, j in jobs.items()}
    pr = B.pair_check(jobs["slowR_1"], jobs["slowR_1_off"], dict(parsed["slowR_1"]), parsed["slowR_1_off"], False)
    check(pr["ok"], "slowR_1 base passes")
    # A difference at T_B - 1 fails; at T_B passes (R_1: T_B 900).
    write_csv(jobs["R_1"]["csv"], F, [(500, "A")], tier7=TIER_1)
    pr = B.pair_check(jobs["R_1"], jobs["R_1_off"], parsed["R_1"], parsed["R_1_off"], False)
    check(not pr["ok"] and any("csv differs at tick 500 < 900" in w for w in pr["why"]), "a difference below T_B fails: %s" % pr["why"])
    write_csv(jobs["R_1"]["csv"], F, [(900, "A")], tier7=TIER_1)
    check(B.pair_check(jobs["R_1"], jobs["R_1_off"], parsed["R_1"], parsed["R_1_off"], False)["ok"], "a difference at T_B passes (Tick < T_B)")
    # W is read in the wanderer cell only (or every cell with --w-all-cells).
    pw = copy.deepcopy(parsed["slowR_1"])
    pw["race_b"]["side"].update(wit=1, wit_first=1000)
    write_csv(jobs["slowR_1"]["csv"], F, [(1000, "S")], tier7=TIER_1)
    check(not B.pair_check(jobs["slowR_1"], jobs["slowR_1_off"], pw, parsed["slowR_1_off"], False)["ok"], "slowR ignores W: a difference at 1,000 < T + 1 fails")
    check(B.pair_check(jobs["slowR_1"], jobs["slowR_1_off"], pw, parsed["slowR_1_off"], True)["ok"], "--w-all-cells: T_B = W = 1,000 passes")
    # T must agree only when the bridge is T + 1.
    poff = copy.deepcopy(parsed["R_2_off"])
    poff["ending"]["ai3"] = 1700
    check(any("T differs" in w for w in B.pair_check(jobs["R_2"], jobs["R_2_off"], parsed["R_2"], poff, False)["why"]), "T differs with T_B = T + 1 fails")
    poff1 = copy.deepcopy(parsed["R_1_off"])
    poff1["ending"]["ai3"] = 1700
    write_csv(jobs["R_1"]["csv"], F, [(1000, "A")], tier7=TIER_1)
    check(B.pair_check(jobs["R_1"], jobs["R_1_off"], parsed["R_1"], poff1, False)["ok"], "T may differ when E < T + 1 (the world moved at E)")
    # The old layout with witness cards leaves W unknown in the wanderer cell.
    pold = copy.deepcopy(parsed["wanderer_2"])
    pold["race_b"]["side"] = dict(pold["race_b"]["side"], layout="old", wit_first=None)
    pr = B.pair_check(jobs["wanderer_2"], jobs["wanderer_2_off"], pold, parsed["wanderer_2_off"], True)
    check(not pr["ok"] and "W unknown" in pr["why"][0], "old layout with cards: no bridge in the wanderer cell")
    br = B.bridge_check(jobs["R_1"], jobs["death_1"], 4400, "death")
    check(br["ok"], "death bridge passes")
    write_csv(jobs["death_1"]["csv"], F, [(1000, "A"), (4300, "D")], tier7=TIER_1)
    br = B.bridge_check(jobs["R_1"], jobs["death_1"], 4400, "death")
    check(not br["ok"] and "csv differs at tick 4300 < 4400" in br["why"][0], "death differing before T_DEATH fails: %s" % br["why"])
    write_csv(jobs["wanderer_2"]["csv"], F, [(1100, "W")])
    check(not B.bridge_check(jobs["R_2"], jobs["wanderer_2"], 1200, "w")["ok"], "wanderer differing from R on before 1,200 fails")
    # Restore the clean files.
    jobs = clean_bundle(F, folder)
    parsed = {n: B.parse_log(j["log"]) for n, j in jobs.items()}

    print("5. controls that break")
    jl = list(jobs.values())
    ok_results = {n: {"ok": True} for n in jobs}
    base = B.controls(jl, parsed, ok_results, 4400, 0.375)
    check(base["ok"], "clean controls pass: %s" % base["problems"])

    def broken(mutate, t_death=4400, r=0.375, results=None, jlist=None):
        pp = copy.deepcopy(parsed)
        mutate(pp)
        return B.controls(jlist or jl, pp, results or ok_results, t_death, r)

    cases = [
        ("wrong ten-name line", lambda pp: pp["R_1_off"].update(rows_taken_out="feat_passage_by_ark"), "rows taken out"),
        ("two taken-out lines", lambda pp: pp["R_1_off"].update(rows_taken_out_lines=2), "rows taken out"),
        ("share was 0.5", lambda pp: pp["R_1_off"].update(off_knobs={"r": "0.375", "share": "0.5"}), "FleetDelayCountdownShare was 0.5"),
        ("r was 0.5", lambda pp: pp["R_1_off"].update(off_knobs={"r": "0.5", "share": "0"}), "AIUpperRungProgressMultiplier was 0.5"),
        ("no knob tail", lambda pp: pp["R_1_off"].update(off_knobs=None), "no 'AIUpperRungProgressMultiplier"),
        ("off era", lambda pp: pp["R_1_off"]["race_b"]["side"].update(era=1), "off run with era"),
        ("off extinction", lambda pp: pp["R_1_off"]["race"]["extinction"].update(on_ai=1), "off run with extinctions"),
        ("off AI treaty", lambda pp: pp["R_1_off"]["race"]["treaties"].update(ai_treaties=1), "AI-step treaties"),
        ("off AI T4", lambda pp: pp["R_1_off"]["race"]["ai_t4"].update(t4_1=3000), "AI T4 at 3000"),
        ("Clock", lambda pp: pp["R_2"]["ending_x"].update(kind="Clock"), "record Clock"),
        ("on taken-out line", lambda pp: pp["R_2"].update(rows_taken_out_lines=1), "an on log has"),
        ("era without first", lambda pp: pp["R_1"]["race_b"]["side"].update(era_first=-1), "E >= 600 iff"),
        ("era before 600", lambda pp: pp["R_1"]["race_b"]["side"].update(era_first=500), "E >= 600 iff"),
        ("first without era", lambda pp: pp["R_2"]["race_b"]["side"].update(era_first=700), "E >= 600 iff"),
        ("witness without first", lambda pp: pp["wanderer_2"]["race_b"]["side"].update(wit_first=-1), "W >= 0 iff"),
        ("first without witness", lambda pp: pp["R_2"]["race_b"]["side"].update(wit_first=800), "W >= 0 iff"),
        ("W before left", lambda pp: pp["wanderer_2"].update(left_at=1600), "before the wanderer left"),
        ("W before the first claim", lambda pp: pp["wanderer_1"]["race_b"]["side"].update(wit_first=1300), "before the first claim placed"),
        ("old layout on", lambda pp: pp["R_2"]["race_b"]["side"].update(layout="old"), "layout before [B-6b8]"),
        ("death asked elsewhere", lambda pp: pp["death_1"]["death"].update(at=4300), "death asked at 4300"),
        ("no death line", lambda pp: pp["death_1"].update(death=None), "no 'calling deaths"),
    ]
    for label, mut, needle in cases:
        c = broken(mut)
        check(not c["ok"] and any(needle in x for x in c["problems"]), "%s -> %s: %s" % (label, needle, c["problems"]))
    c = broken(lambda pp: [pp[n].update(off_knobs={"r": "0.5" if n == "R_1_off" else "0.375", "share": "0"}) for n in pp if n.endswith("_off")], r=None)
    check(not c["ok"] and any("disagree on the committed r" in x for x in c["problems"]), "r values disagree without --r")
    c = broken(lambda pp: pp["death_1"]["death"].update(at=4300), t_death=None)
    check(not c["ok"] and any("disagree on the asked tick" in x for x in c["problems"]), "death ticks disagree without --t-death")
    c = broken(lambda pp: [pp[n]["race_b"]["side"].update(era=0, era_first=-1, wit=0, wit_first=-1, era_rec=0) for n in pp if not n.endswith("_off")])
    check(c["ok"] and any("재지 않음 — 잰 0" in n for n in c["notes"]), "no era and no witness anywhere is a note, not a stop: %s" % c["notes"])
    c = broken(lambda pp: pp["R_2"]["ending_x"].update(kind="Clock"), results=dict(ok_results, R_2={"ok": False}))
    check(c["ok"], "a twice-failed run is not a control problem")
    short = os.path.join(root, "short.csv")
    write_csv(short, F, [], to=19500)
    jl2 = [dict(j, csv=short) if j["name"] == "R_2" else j for j in jl]
    c = broken(lambda pp: None, jlist=jl2)
    check(not c["ok"] and any("the CSV ends at 19500" in x for x in c["problems"]), "a CSV short of 20,000 fails")

    print("6. frozen arithmetic")
    check(B.lower_median([4, 1, 3, 2]) == 2 and B.lower_median([5]) == 5 and B.lower_median([]) is None, "lower median")
    check(B.window_median([(1000, 1), (1599, 1), (1600, 2), (2799, 1), (2800, 5)], 1000, 2900) == (2, 3), "windows [1000,1600) 2, [1600,2200) 2, [2200,2800) 1 -> lower median 2; 2,800 is past the last whole window")
    check(B.window_median([(1000, 1), (1700, 1), (2300, 1), (2900, 9)], 1000, 2900)[1] == 3 and B.window_median([(2900, 9)], 1000, 2900) == (0, 3),
          "a decision at the end tick (and past the last whole window) is outside")
    check(B.window_median([(1100, 1)], 1000, 1599) == (None, 0), "0 whole windows -> (None, 0)")
    check(B.window_median([(1000, 1), (1600, 1)], 1000, 3400) == (0, 4), "4 windows [1,1,0,0] -> lower median 0")
    check(abs(B.sign_test(5, 0) - 0.0625) < 1e-12 and B.sign_test(0, 0) == 1.0 and abs(B.sign_test(3, 3) - 1.0) < 1e-12, "sign test")

    print("7. judgment (1)")
    R40 = [mk("R", s, "RuledPeople" if s <= 20 else "OtherPeople", 9000, "civ_3") for s in range(1, 41)] + [mk("R", s) for s in range(41, 61)]
    j = B.judge_1(R40)
    check(j["judged"] and j["passed"] and j["J1"] == 40, "40/60 passes")
    j = B.judge_1(R40[:39] + [mk("R", 40, "Vessel", 9000)] + R40[40:])
    check(j["judged"] and not j["passed"] and j["J1"] == 39, "Vessel is not a fifth-rung end: 39/60 fails")
    j = B.judge_1(R40[:39] + [mk("R", 40, "OtherPeople", 20001, "civ_3")] + R40[40:])
    check(not j["passed"], "a record after 20,000 is not reached")
    j = B.judge_1([mk("R", s, "OtherPeople", 9000, "civ_3", ok=(s != 1)) for s in range(1, 41)] + R40[40:])
    check(not j["passed"] and j["failed"] == [1], "a twice-failed run counts as not reached")
    j = B.judge_1(R40[:20])
    check(not j["judged"] and j["passed"] is None and j["text"] == "재지 않음 — 잰 20", "n < 30 -> '재지 않음 — 잰 N'")

    print("8. judgment (2)")
    ga = dict(binding=(8000, "AI", "civ_3", 4))

    def r2(n_other, n_once=0, splits=None, n_ruled=None):
        items, s = [], 1
        for i in range(n_other):
            kw = (splits[i] if splits and i < len(splits) else ga)
            items.append(mk("R", s, "OtherPeople", 9000, "civ_3", 0, r4=7000, r4civ="civ_7", **kw))
            s += 1
        for _ in range(n_once):
            items.append(mk("R", s, "OtherPeople", 9000, "civ_4", 1, **ga))
            s += 1
        for _ in range((40 - n_other - n_once) if n_ruled is None else n_ruled):
            items.append(mk("R", s, "RuledPeople", 9000, "civ_7"))
            s += 1
        while s <= 60:
            items.append(mk("R", s))
            s += 1
        return items
    j = B.judge_2(r2(10, 2))
    check(j["judged"] and j["passed"] and j["n2"] == 40 and j["J2"] == 10 and j["once_ruled"] == [11, 12], "10/40 inside; once-ruled 2 in the denominator only: %s" % j["text"])
    check(len(j["split"]["ga"]) == 10 and j["ga_binder_is_record_people"] == 10 and j["b6b9_reading"]["reading"] == "-", "all (가), binder = record people")
    j = B.judge_2(r2(16))
    check(not j["passed"] and j["side"] == "above" and j["b6b9_reading"]["reading"].startswith("r one ladder step down"), "16/40 above; (가) alone above -> r down: %s" % j["b6b9_reading"])
    na = dict(binding=(8000, "AI", "civ_3", 3), erased_ruled=1)
    j = B.judge_2(r2(16, splits=[na] * 12))
    check(not j["passed"] and j["side"] == "above" and len(j["split"]["ga"]) == 4 and "[개발자]" in j["b6b9_reading"]["reading"], "above but (가) 4/40 inside -> [개발자]")
    j = B.judge_2(r2(4))
    check(not j["passed"] and j["side"] == "below" and j["b6b9_reading"]["reading"].startswith("r one ladder step up"), "4/40 below; (가) below -> r up")
    j = B.judge_2(r2(6, n_ruled=4))
    check(not j["judged"] and j["text"] == "재지 않음 — 잰 10", "n2 < 30 -> 재지 않음")
    cases = [dict(binding=(8000, "AI", "civ_3", 4)),                                 # (가)
             dict(binding=(8000, "AI", "civ_5", 5)),                                 # (가), binder != record people
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1),                 # (나): erased by an extinction
             dict(binding=(8000, "AI", "civ_3", 3), r4=-1),                          # (나): ruled T4 -1
             dict(binding=(8000, "AI", "civ_3", 3), r4=8100),                        # (나): ruled T4 after the binding
             dict(binding=(8000, "former", "civ_7", 0), r4=-1),                      # (다): ruled rung 0, never folded into (나)
             dict(binding=(8000, "AI", "civ_3", 3), r4=7000),                        # (다): every other
             dict(binding=(8000, "ruled", "civ_7", 4), r4=7000),                     # (다): the binder was the ruled people
             dict(binding=(-1, "-", "-", 0))]                                        # (다): no binding printed
    items = []
    for i, kw in enumerate(cases, 1):
        kw = dict(kw)
        r4 = kw.pop("r4", 7000)
        items.append(mk("R", i, "OtherPeople", 9000, "civ_3", 0, r4=r4, r4civ="civ_7", **kw))
    items += [mk("R", s, "RuledPeople", 9000, "civ_7") for s in range(10, 41)]
    j = B.judge_2(items)
    got = {k: [f["seed"] for f in v] for k, v in j["split"].items()}
    check(got == {"ga": [1, 2], "na": [3, 4, 5], "da": [6, 7, 8, 9]} and j["ga_binder_is_record_people"] == 1, "(가)/(나)/(다): %s" % got)
    j = B.judge_2(r2(3, n_ruled=57))
    check(j["wholly_outside"] and not j["passed"], "3/60: the Wilson interval wholly below the band")
    S = [mk("slowR", s, "OtherPeople", 9000, "civ_3", 0, **ga) for s in range(1, 41)] + [mk("slowR", s, "RuledPeople", 9000, "civ_7") for s in range(41, 51)]
    j = B.judge_2_slow(S)
    check(j["show_developer"] and j["passed"] is None and j["share"]["lo"] > 0.35, "slowR: interval wholly above 35% -> shown to [개발자], report only")
    j = B.judge_2_slow(r2(10))
    check(not j["show_developer"], "slowR inside: nothing to show")

    print("9. judgment (3)")

    def r3(lengths, dense=True, extra=()):
        items = []
        for i, L in enumerate(lengths, 1):
            r4 = 4000
            items.append(mk("R", i, "RuledPeople", r4 + L + 50, "civ_7", r4=r4, r4civ="civ_7", r5=r4 + L, r5civ="civ_7",
                            decisions=every(r4, r4 + L) if (dense if isinstance(dense, bool) else dense(i)) else ()))
        return items + list(extra)
    j = B.judge_3(r3([1800] * 30))
    check(j["judged"] and j["passed"] and j["length"] == 1800 and j["dense"] == 30, "30 runs of 1,800, dense: pass")
    j = B.judge_3(r3([1800] * 14 + [3700] * 16))
    check(j["judged"] and not j["passed"] and not j["length_ok"] and j["length"] == 3700, "lower median 3,700 > 3,600 fails length")
    j = B.judge_3(r3([1800] * 15 + [3700] * 15))
    check(j["length"] == 1800 and j["length_ok"], "even count: the lower middle (1,800)")
    j = B.judge_3(r3([1800] * 30, dense=lambda i: i > 3))
    check(j["passed"] and j["dense"] == 27, "27/30 dense = 90% passes")
    j = B.judge_3(r3([1800] * 30, dense=lambda i: i > 4))
    check(not j["passed"] and not j["density_ok"] and j["length_ok"], "26/30 fails density only")
    j = B.judge_3(r3([1800] * 29 + [500]))
    check(j["zero_windows"] == [30] and j["dense"] == 29 and [v["seed"] for v in j["violations"]] == [30], "0 windows counts failing and r5 - r4 < 1,200 is reported")
    apart = [mk("R", 31, "RuledPeople", 9000, "civ_7", r4=4000, r4civ="civ_7", r5=6000, r5civ="civ_9"),
             mk("R", 32, "RuledPeople", 5000, "civ_7", r4=4000, r4civ="civ_7", r5=6000, r5civ="civ_7"),
             mk("R", 33, "OtherPeople", 9000, "civ_3", r4=4000, r4civ="civ_7", r5=8000, r5civ="civ_7"),
             mk("R", 34, "RuledPeople", 9000, "civ_7", r4=4000, r4civ="civ_7", r5=6000, r5civ="civ_7", ok=False)]
    j = B.judge_3(r3([1800] * 30, extra=apart))
    reasons = {a["seed"]: a["why"] for a in j["apart"]}
    check(j["n3"] == 30 and sorted(reasons) == [31, 32, 33] and "다른 재위" in reasons[31] and "record before r5" in reasons[32] and "record OtherPeople" in reasons[33],
          "runs with r5 reported apart: %s" % reasons)
    j = B.judge_3(r3([1800] * 29))
    check(not j["judged"] and j["text"] == "재지 않음 — 잰 29", "n3 < 30 -> 재지 않음")

    print("10. judgment (4) and the clock")

    def s4(Es, r3=3000, dense=True, recs=None, ok=lambda i: True):
        items = []
        for i, E in enumerate(Es, 1):
            if E is None:
                rec = (recs or {}).get(i, -1)
                items.append(mk("slowR", i, "NoCandidate" if rec >= 0 else "None", rec, r3=r3, decisions=every(r3, 20000) if dense else (), ok=ok(i)))
            else:
                items.append(mk("slowR", i, "OtherPeople", E, "civ_3", r3=r3, decisions=every(r3, 20000) if dense else (), ok=ok(i)))
        return items
    Es = [5000 + 100 * i for i in range(50)] + [None] * 10
    j = B.judge_4(s4(Es))
    check(j["E48"] == 9700 and j["C0"] == 10000 and j["C"] == 10000 and j["i_count"] == 50 and j["passed"], "48th E 9,700 -> C0 10,000, C 10,000; accepted: %s" % j["text"])
    j = B.judge_4(s4([4000 + 10 * i for i in range(60)]))
    check(j["C0"] == 5000 and j["C"] == 6000 and j["passed"], "C0 5,000 -> the floor 6,000")
    j = B.judge_4(s4([9000] * 48 + [None] * 12))
    check(j["C0"] == 9000, "a 48th E on a multiple of 1,000 stays")
    j = B.judge_4(s4([19500] * 50 + [None] * 10))
    check(j["C0"] == 20000 and j["C"] == 18000 and j["i_count"] == 0 and not j["i_ok"] and not j["passed"] and "no C" in j["text"], "C0 20,000 -> cap 18,000, (i) 0 < 30 fails")
    j = B.judge_4(s4([7000] * 40 + [None] * 20))
    check(math.isinf(j["C0"]) and j["C"] == 18000 and j["i_count"] == 40 and j["passed"], "fewer than 48 ends: C0 inf, C 18,000, (i) 40")
    j = B.judge_4(s4(Es, dense=False))
    check(j["i_ok"] and not j["ii_ok"] and not j["passed"], "(ii) fails without decisions")
    j = B.judge_4(s4(Es, r3=-1))
    check(not j["judged"] and j["C"] is None and "no C" in j["text"], "fewer than 30 runs reach T3 -> no C")
    j = B.judge_4(s4([None] * 60, r3=3000, recs={i: 3500 for i in range(1, 32)}))
    check(len(j["ii_not_reached"]) == 0 and j["ii_zero_windows"] == list(range(1, 32)) and not j["ii_ok"],
          "(ii) ends at the record of any kind (silence at 3,500): 0 whole windows fail")
    j = B.judge_4(s4([None] * 60, r3=3200, recs={i: 3100 for i in range(1, 32)}))
    check(j["ii_not_reached"] == list(range(1, 32)) and not j["judged"], "ruled T3 at or after the end -> not reached, excluded (29 left -> no C)")
    j = B.judge_4(s4(Es, ok=lambda i: i not in (1, 2)))
    check(j["n"] == 58 and j["rank"] == 47 and j["excluded"] == [1, 2], "twice-failed runs excluded: nearest rank ceil(0.8 x 58) = 47")
    j = B.judge_4(s4(Es[:20]))
    check(not j["judged"] and j["text"] == "재지 않음 — 잰 20", "n < 30 -> 재지 않음")

    print("11. record only and withheld judgments")
    jobs_l = [mk("R", s, "None", -1, binding=(5000, "AI", "civ_3", 4), locked=1)[0:2] for s in range(1, 4)]
    jl3 = [x[0] for x in jobs_l]
    pp3 = {x[0]["name"]: x[1] for x in jobs_l}
    rs3 = {n: {"ok": True} for n in pp3}
    rec = B.record_only(jl3, {n: dict(p, race={"ruled": p["race"]["ruled"]}, crowned=None) for n, p in pp3.items()}, rs3)
    check(rec["locks"]["count"] == 3 and rec["locks"]["flag_165"], "3 locked runs without a fifth-rung record -> #165 to [개발자]")
    jd = B.judgments(jl3, pp3, {"R_1": {"ok": False}, "R_2": {"ok": False}, "R_3": {"ok": True}})
    check(jd["withheld"] == ["R"] and jd["failed"]["R"] == ["R_1", "R_2"], "two twice-failed runs in a cell -> judgments withheld")

    print("12. --cell-set A against bundleA.py's own verdict")
    afolder = os.path.join(root, "bundleA")
    os.makedirs(afolder)
    ajobs = {j["name"]: j for j in A.build_jobs(afolder, max_seed=2)}
    T.make_pair(F, afolder, ajobs, "R_1", T.ON, T.OFF, csv_change=lambda t, c, r: r.update(Energy="1.00") if (t >= 400 and c == "civ_7") else None,
                rows_on=[(900, "crisis_species_extinction", 1), (1100, "project_ark", 1)], rows_off=[])
    T.make_pair(F, afolder, ajobs, "R_2", T.NONE_T3, T.NONE_T3, rows_on=[(500, "log_x", 2)], rows_off=[(500, "log_x", 2)])
    T.make_pair(F, afolder, ajobs, "inputless_1", dict(T.ON, r4=-1, r4_civ="-"), T.OFF, rows_on=[(900, "crisis_species_extinction", 1)], rows_off=[])
    T.make_pair(F, afolder, ajobs, "inputless_2", dict(T.ON, r4=-1, r4_civ="-", ai3=500, ai3_civ="civ_4"), dict(T.OFF, ai3=500, ai3_civ="civ_4"),
                rows_on=[(900, "crisis_species_extinction", 1)], rows_off=[])
    T.make_pair(F, afolder, ajobs, "wanderer_1", T.ON, T.OFF, csv_change=lambda t, c, r: r.update(Stability="0.5000") if t >= 200 else None, crowned=3121)
    T.make_pair(F, afolder, ajobs, "wanderer_2", T.ON, T.OFF, rows_on=[(600, "crisis_species_extinction", 1)], rows_off=[(600, "log_y", 1)], crowned=-1)
    sink = io.StringIO()
    with contextlib.redirect_stdout(sink):
        a_all = A.main(["--check-only", afolder, "--max-seed", "2", "--root", afolder])
        b_all = B.main(["--cell-set", "A", "--check-only", afolder, "--max-seed", "2", "--summary-dir", os.path.join(root, "a_check")])
        a_ri = A.main(["--check-only", afolder, "--max-seed", "2", "--cells", "R,inputless", "--root", afolder])
        b_ri = B.main(["--cell-set", "A", "--check-only", afolder, "--max-seed", "2", "--cells", "R,inputless", "--summary-dir", os.path.join(root, "a_check2")])
    out = sink.getvalue()
    check(a_all == b_all == 1 and a_ri == b_ri == 0, "bundle A's verdicts reproduce: all cells %s/%s, R+input-less %s/%s" % (a_all, b_all, a_ri, b_ri))
    check("bundle B parser over bundle A's logs: every part read" in out, "the old side layout and every B part read on bundle A's format")
    check(B.main(["--cell-set", "A"]) == 2, "--cell-set A needs --check-only")

    print("13. dry run and refusals")
    default_folder = os.path.join(HERE, "bundleBprime")   # [B-6b9]: the copy's default (was bundleB)
    existed = os.path.exists(default_folder)
    sink = io.StringIO()
    with contextlib.redirect_stdout(sink):
        code = B.main(["--dry-run", "--t-death", "4400", "--a-sec-per-20k", "70", "--root", os.path.join(root, "proj")])
    lines = sink.getvalue().splitlines()
    cmds = [l for l in lines if not l.startswith("#")]
    check(code == 0 and len(cmds) == 540 and "-DeathAt=4400" in cmds[-1] and "# 540 jobs" in lines[-2] and "~ 105.0 min" in lines[-2], "dry run: 540 commands: %s" % lines[-2])
    check(os.path.exists(default_folder) == existed and not os.path.exists(os.path.join(root, "proj")), "the dry run creates no folder")
    sink = io.StringIO()
    with contextlib.redirect_stdout(sink):
        code = B.main(["--root", os.path.join(root, "proj"), "--folder", os.path.join(root, "never")])
    check(code == 2 and "needs --t-death" in sink.getvalue() and not os.path.exists(os.path.join(root, "never")), "a real run without --t-death is refused before anything starts")

    b6b9_tests(F, root)

    print("\nself-test: %d checks passed, %d failed" % (PASSES[0], len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
