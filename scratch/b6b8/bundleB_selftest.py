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
        c(p[6], {0: -1, 1: "-", 2: "-", 3: -1}),
        c(p[7], {0: v.get("on_ruled", 0), 1: -1, 2: v.get("on_ai", 0), 7: "", 15: v.get("erased_ruled", 0)}),
        c(p[8], {0: v.get("ai_treaties", 0), 1: -1, 2: 0, 3: 0}),
        c(p[9], {0: -1, 1: -1, 2: "-"}),
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
       decisions=(), locked=0, ok=True, ruled_erased=()):
    job = {"name": "%s_%d" % (cell, seed), "cell": cell, "seed": seed, "off": False, "ticks": 20000}
    p = {"log_exists": True, "ending_x": {"kind": kind, "tick": rec, "civ": civ, "some_life_ruled": some, "no_vessel": 0},
         "race": {"ruled": {"r3": r3, "r3_civ": "civ_7", "r4": r4, "r4_civ": r4civ, "r5": r5, "r5_civ": r5civ}, "extinction": {"erased_ruled": erased_ruled}},
         "race_b": {"binding": {"tick": binding[0], "by": binding[1], "civ": binding[2], "rung": binding[3], "locked": locked, "no_result": 0, "retaken": 0,
                                "vessel_left": 0},
                    "decisions": {"pairs": list(decisions)}}}
    p["ruled_erased"] = list(ruled_erased) if ruled_erased is not None else None
    return job, p, ok


def every(start, end, step=300):
    return [(t, 1) for t in range(start, end, step)]


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
    check(B.dist([1, 2, 3, 4])["median"] == 2 and B.dist([5])["median"] == 5 and B.dist([None, float("inf")])["n"] == 0
          and B.dist([1, 2, 3, 4])["text"].startswith("n 4 median 2 "), "review M4: the record-only median is the lower middle")

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
    na = dict(binding=(8000, "AI", "civ_3", 3), erased_ruled=1, ruled_erased=[("civ_7", 7900, 8000)])
    j = B.judge_2(r2(16, splits=[na] * 12))
    check(not j["passed"] and j["side"] == "above" and len(j["split"]["ga"]) == 4 and "[개발자]" in j["b6b9_reading"]["reading"], "above but (가) 4/40 inside -> [개발자]")
    j = B.judge_2(r2(4))
    check(not j["passed"] and j["side"] == "below" and j["b6b9_reading"]["reading"].startswith("r one ladder step up"), "4/40 below; (가) below -> r up")
    j = B.judge_2(r2(6, n_ruled=4))
    check(not j["judged"] and j["text"] == "재지 않음 — 잰 10", "n2 < 30 -> 재지 않음")
    cases = [dict(binding=(8000, "AI", "civ_3", 4)),                                 # (가)
             dict(binding=(8000, "AI", "civ_5", 5)),                                 # (가), binder != record people
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=[("civ_7", 7900, 8000)]),  # (나): erased by an extinction
             dict(binding=(8000, "AI", "civ_3", 3), r4=-1),                          # (나): ruled T4 -1
             dict(binding=(8000, "AI", "civ_3", 3), r4=8100),                        # (나): ruled T4 after the binding
             dict(binding=(8000, "former", "civ_7", 0), r4=-1),                      # (다): ruled rung 0, never folded into (나)
             dict(binding=(8000, "AI", "civ_3", 3), r4=7000),                        # (다): every other
             dict(binding=(8000, "ruled", "civ_7", 4), r4=7000),                     # (다): the binder was the ruled people
             dict(binding=(-1, "-", "-", 0)),                                        # (다): no binding printed
             # review M2: a later life's ruled people erased after the binding - rung 0 stays (다), never folded into (나)
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=[("civ_7", 12000, 12100)]),
             # the erasure's sample straddles the cut (last 7,950 < 8,000 < 8,050): not placed for sure - (다), printed
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=[("civ_7", 7950, 8050)]),
             # no CSV (an archive's logs alone): the run-wide count decides, flagged
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=None),
             # no binding: the cut is the record (9,000) - erased at 8,500 is (나)
             dict(binding=(-1, "-", "-", 0), erased_ruled=1, ruled_erased=[("civ_7", 8400, 8500)])]
    items = []
    for i, kw in enumerate(cases, 1):
        kw = dict(kw)
        r4 = kw.pop("r4", 7000)
        items.append(mk("R", i, "OtherPeople", 9000, "civ_3", 0, r4=r4, r4civ="civ_7", **kw))
    items += [mk("R", s, "RuledPeople", 9000, "civ_7") for s in range(len(cases) + 1, 41)]
    j = B.judge_2(items)
    got = {k: [f["seed"] for f in v] for k, v in j["split"].items()}
    check(got == {"ga": [1, 2], "na": [3, 4, 5, 12, 13], "da": [6, 7, 8, 9, 10, 11]} and j["ga_binder_is_record_people"] == 1, "(가)/(나)/(다): %s" % got)
    fields = {f["seed"]: f for v in j["split"].values() for f in v}
    check(fields[10]["erased ruled before the cut"] is False and fields[11]["erased ruled near the cut"] == ["civ_7"]
          and fields[12].get("erased ruled timing unknown (no csv) - run-wide count") is True and fields[3]["erased ruled before the cut"] is True,
          "review M2: the erasure's time against the cut, the straddling sample printed, the missing CSV flagged")
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
    items = r3([1800] * 15 + [3000] * 15)
    plain = B.judge_3(r3([1800] * 15 + [3000] * 15))
    for i, (_, p, _) in enumerate(items):
        p["race_b"]["laws"] = {"resets_r": 0 if i < 15 else 2, "dropped": 1 if i == 0 else 0}
    j = B.judge_3(items)
    lb = j["length by breaks (run-wide)"]
    check(lb["0"] == {"n": 15, "median": 1800} and lb[">=1"] == {"n": 15, "median": 3000} and lb["unread"] == {"n": 0, "median": None}
          and j["ruled laws dropped at an input"] == 1 and j["empty law ticks"].startswith("재지 않음"), "review M1 (L4-2): the length by the run's law breaks %s" % lb)
    check(j["passed"] == plain["passed"] and j["length"] == plain["length"] and plain["length by breaks (run-wide)"]["unread"]["n"] == 30,
          "the split leaves (3)'s verdict as it was; a run without the laws part is 'unread'")

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

    print("11b. record-only fixes of the review (M1 L4-3, M3, M5, M6)")

    def tw(seed, ai, r3, r4=-1, rec=-1, decisions=(), rows=None):
        job, p, ok = mk("R", seed, "RuledPeople" if rec >= 0 else "None", rec, r3=r3, r4=r4, decisions=decisions)
        p["race_b"]["laws"] = {"ai": ai}
        if rows is not None:
            job["rows"] = rows
        return job, p, ok
    rows_path = os.path.join(root, "tw_rows.csv")
    write_rows(rows_path, F, ((3000, "petition_plague", 9), (3100, "petition_plague", 2), (5400, "enc_witness", 1), (5500, "petition_plague", 5),
                              (4000, "event_era_endured", 7), (4100, "project_relief", 3)))
    w = B.third_rung_workload([tw(1, 3000, 2000, r4=5400, decisions=every(3000, 5400), rows=rows_path),   # 4 whole windows of 2
                               tw(2, -1, 2000),                                                          # no AI hold: 재지 않음
                               tw(3, 3000, 3500),                                                        # ruled T3 after the hold: 재지 않음
                               tw(4, 3000, 2000, r4=2900),                                               # e <= s: 0 windows
                               tw(5, 3000, 2000, rec=7000, decisions=every(3000, 7000))])               # e = 6,000 (before the record)
    key = lambda prefix: next(k for k in w if k.startswith(prefix))  # noqa: E731
    check(w["runs"] == 3 and w[key("재지 않음")] == [2, 3] and w["zero windows"] == [4] and w[key("decisions per 600")] == 2,
          "review M1 (L4-3): the window from the first AI law hold, unmeasured and 0-window runs: %s" % w)
    check(w[key("cards per 600")] == {"enc_witness": 0.25, "petition_plague": 0.5, "project_relief": 0.75} and w["window ticks"] == 2400
          and w["runs without _rows.csv"] == [5], "cards: petition_/enc_ and A′'s two named rows in (s, e] per 600 (not the era's line): %s" % w)
    xs = [mk("R", s, binding=(8000, "ruled", "civ_7", 4))[0:2] for s in (1, 2, 3)]
    for (_, p), v in zip(xs, (-1, 0, 1)):
        p["race_b"]["binding"]["vessel_left"] = v
    xs[0][1]["race"]["t4civs"] = {"at_record": 2, "at_end": 3, "most": 3}
    xs[1][1]["race"]["t4civs"] = {"at_record": -1, "at_end": 1, "most": 1}
    ce = B.cell_extras(xs)
    check(ce["binding"]["vessel left before T5"] == "1/2", "review M3: left before T5 over the runs the ruled people bound (-1 is a marker): %s" % ce["binding"])
    check(ce["T4 civs at the record"].startswith("n 1 median 2 ") and ce["T4 civs no record"] == 1, "review M6: T4 civs at the record, the runs without one counted")
    mf = os.path.join(root, "m5")
    jr = next(j for j in B.build_jobs(mf, 4400, cells={"R"}, max_seed=1) if not j["off"])
    write_job(F, jr, dict(seed=1), extra=[T.ue("CosmicBalance calling: lives 2; reign ended first at 1500, last at 1500; first throne after the first end at -1")],
              tier7=TIER_1)
    pr = {jr["name"]: B.parse_log(jr["log"])}
    rr = {jr["name"]: {"ok": True}}
    rec = B.record_only([jr], pr, rr)
    left = rec["vessels left at T4+"]
    check(pr[jr["name"]]["reign_ended"] == 1500 and pr[jr["name"]]["left_at"] is None and len(left) == 1 and left[0]["cell"] == "R"
          and left[0]["left"] == 1500 and left[0]["tier"] == 4 and rec["R reigns ended"] == 1, "review M5: an R log's reign end reads on the #96 line: %s" % left)
    saved = B.left_tick
    B.left_tick = lambda p: p.get("left_at")
    try:
        check(B.record_only([jr], pr, rr)["vessels left at T4+"] == [], "(the fallback broken once: the R entry is gone - the case above would fail)")
    finally:
        B.left_tick = saved

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
    default_folder = os.path.join(HERE, "bundleB")
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

    print("14. --check-only writes nothing under --root or into an archived copy ([B-6b9] review F4, back-ported)")

    def run(argv):
        sink = io.StringIO()
        with contextlib.redirect_stdout(sink):
            code = B.main(argv)
        return code, sink.getvalue()

    def read(path):
        with open(path, "rb") as f:
            return f.read()

    ck = ["--max-seed", "2", "--t-death", "4400", "--r", "0.375"]
    proj_e = os.path.join(root, "proj_e")
    arch_b = os.path.join(proj_e, "Saved", "Balance", "b6b_bundleB")   # bundle B's archive under the project tree
    clean_bundle(F, arch_b)
    with open(os.path.join(arch_b, "summary.json"), "w", encoding="utf-8") as f:
        json.dump({"header": {"verdict": True}, "judgments": {"1": {"passed": False}}}, f)
    before = read(os.path.join(arch_b, "summary.json"))
    code, out = run(["--check-only", arch_b, "--root", proj_e] + ck)
    check(code == 2 and "under the project tree" in out and read(os.path.join(arch_b, "summary.json")) == before
          and not os.path.exists(os.path.join(arch_b, "summary.md")),
          "F4 E: --check-only under --root without --summary-dir is refused, the archived summary.json untouched: %s" % out.strip()[-160:])
    code, out = run(["--check-only", arch_b, "--root", proj_e, "--summary-dir", os.path.join(proj_e, "x")] + ck)
    check(code == 2 and "under the project tree" in out and not os.path.exists(os.path.join(proj_e, "x")), "F4: a --summary-dir under --root is refused too")
    code, out = run(["--check-only", arch_b, "--root", proj_e, "--summary-dir", os.path.join(root, "e_sum")] + ck)
    sj = json.load(open(os.path.join(root, "e_sum", "summary.json"), encoding="utf-8")) if os.path.exists(os.path.join(root, "e_sum", "summary.json")) else {}
    check(code == 0 and os.path.exists(os.path.join(root, "e_sum", "summary.md")) and read(os.path.join(arch_b, "summary.json")) == before
          and sj.get("header", {}).get("summary_dir") == os.path.join(root, "e_sum") and sj.get("header", {}).get("verdict") is True,
          "F4: with --summary-dir outside --root the folder under --root is read and left as it was (exit %s): %s" % (code, out.strip()[-160:]))
    copy_dir = os.path.join(root, "archive_copy")   # an archived copy outside the root (manifest.sha1)
    shutil.copytree(arch_b, copy_dir)
    with open(os.path.join(copy_dir, "manifest.sha1"), "w", encoding="utf-8") as f:
        f.write("0  R_1.log\n")
    snap = {n: read(os.path.join(copy_dir, n)) for n in os.listdir(copy_dir)}
    code, out = run(["--check-only", copy_dir, "--root", proj_e] + ck)
    check(code == 2 and "manifest.sha1" in out, "F4: an archived copy (manifest.sha1) needs --summary-dir: %s" % out.strip()[-160:])
    code, out = run(["--check-only", copy_dir, "--root", proj_e, "--summary-dir", copy_dir] + ck)
    check(code == 2 and "manifest.sha1" in out, "F4: --summary-dir pointing at the archived copy itself is refused")
    code, out = run(["--check-only", copy_dir, "--root", proj_e, "--summary-dir", os.path.join(root, "copy_sum")] + ck)
    check(code == 0 and os.path.exists(os.path.join(root, "copy_sum", "summary.json")), "F4: with --summary-dir outside the root the copy is judged: %s" % out.strip()[-160:])
    code, out = run(["--check-only", copy_dir, "--root", proj_e, "--summary-dir", os.path.join(root, "copy_sum"), "--archive", "--archive-dir", os.path.join(root, "a2")] + ck)
    check(code == 2 and "--archive with --summary-dir" in out and not os.path.exists(os.path.join(root, "a2")), "--archive with --summary-dir is refused")
    plain = os.path.join(root, "plain_check")   # a scratch folder outside the root and not an archive: judged in place
    shutil.copytree(arch_b, plain)
    code, out = run(["--check-only", plain, "--root", proj_e, "--archive"] + ck)
    check(code == 2 and "an archive under the project tree" in out and "--archive-dir" in out,
          "F4: --check-only --archive into the default <root>\\Saved\\Balance\\b6b_bundleB is refused: %s" % out.strip()[-160:])
    code, out = run(["--check-only", plain, "--root", proj_e, "--archive", "--archive-dir", copy_dir] + ck)
    check(code == 2 and "manifest.sha1" in out, "F4: --check-only --archive into an existing archive (manifest.sha1) is refused")
    code, out = run(["--check-only", plain, "--root", proj_e, "--archive", "--archive-dir", plain] + ck)
    check(code == 2 and "the checked folder" in out, "--check-only --archive into the checked folder itself is refused")
    check({n: read(os.path.join(copy_dir, n)) for n in os.listdir(copy_dir)} == snap and read(os.path.join(arch_b, "summary.json")) == before,
          "F4: the archived copy and the folder under --root are byte-identical after every refusal")
    code, out = run(["--check-only", plain, "--root", proj_e] + ck)
    check(code == 0 and read(os.path.join(plain, "summary.json")) != before, "a scratch folder outside --root is still judged in place")
    code, out = run(["--root", proj_e, "--t-death", "4400", "--summary-dir", os.path.join(root, "s"), "--folder", os.path.join(root, "never2")])
    check(code == 2 and "--summary-dir is for --check-only" in out and not os.path.exists(os.path.join(root, "never2")), "--summary-dir without --check-only is refused")
    code, out = run(["--check-only", arch_b, "--root", proj_e, "--dry-run"] + ck)
    check(code == 0 and read(os.path.join(arch_b, "summary.json")) == before, "a --check-only dry run writes nothing (no refusal needed)")
    # --cell-set A: bundle A's summary.json in the checked folder is the reference; never under --root, never into it.
    aproj = os.path.join(root, "aproj")
    ain = os.path.join(aproj, "Saved", "Balance", "b6b_bundleA")
    shutil.copytree(afolder, ain)
    a_before = {n: read(os.path.join(ain, n)) for n in os.listdir(ain)}
    code, out = run(["--cell-set", "A", "--check-only", ain, "--max-seed", "2", "--root", aproj, "--summary-dir", os.path.join(aproj, "Saved", "x")])
    check(code == 2 and "under the project tree" in out and not os.path.exists(os.path.join(aproj, "Saved", "x")), "F4 (A): a --summary-dir under --root is refused")
    code, out = run(["--cell-set", "A", "--check-only", afolder, "--max-seed", "2", "--root", aproj, "--summary-dir", afolder])
    check(code == 2 and "the checked folder" in out, "F4 (A): --summary-dir = bundle A's folder is refused (its summary.json is the reference)")
    code, out = run(["--cell-set", "A", "--check-only", ain, "--max-seed", "2", "--root", aproj, "--summary-dir", os.path.join(root, "a_check3")])
    check(code == 1 and "every part read" in out and {n: read(os.path.join(ain, n)) for n in os.listdir(ain)} == a_before
          and os.path.exists(os.path.join(root, "a_check3", "summary.json")),
          "F4 (A): bundle A under --root is read, its verdict reproduces and the folder stays byte-identical (exit %s)" % code)

    print("\nself-test: %d checks passed, %d failed" % (PASSES[0], len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
