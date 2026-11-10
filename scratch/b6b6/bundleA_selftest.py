"""Self-test of bundleA.py: fabricates logs and CSVs under scratchpad\\b6b6\\selftest\\ and exercises the parser, the pair
check, the switch controls, the run-failure rules, the sha1 guard, the job list and the 4.5 recomputation. No commandlet.

The fake log lines are printed from the commandlet's own format strings (CosmicBalanceCommandlet.cpp - the git show copy
cmdlet_head.cpp next to this file, or `git show HEAD:` read into memory when that copy is missing), so a parser regex that
drifts from the C++ text fails here.

  python -I -X utf8 bundleA_selftest.py [--cpp <CosmicBalanceCommandlet.cpp copy>]
"""
import argparse
import importlib.util
import json
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
spec = importlib.util.spec_from_file_location("bundleA", os.path.join(HERE, "bundleA.py"))
B = importlib.util.module_from_spec(spec)
spec.loader.exec_module(B)

FAILS = []
PASSES = [0]


def check(cond, what):
    if cond:
        PASSES[0] += 1
    else:
        FAILS.append(what)
        print("  FAIL: " + what)

# ------------------------------------------------------------------------------------------ the commandlet's formats


def load_cpp(path):
    if path and os.path.exists(path):
        return open(path, encoding="utf-8", errors="replace").read()
    return subprocess.run(["git", "-C", ROOT, "show", "HEAD:Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp"],
                          capture_output=True, text=True, encoding="utf-8", errors="replace").stdout


def fmt_on_line(cpp, marker, nth=0):
    lines = [l for l in cpp.splitlines() if marker in l]
    m = re.search(r'TEXT\("((?:[^"\\]|\\.)*)"\)', lines[nth])
    return m.group(1).replace('\\"', '"')


PH = re.compile(r"%(%|(\.\d+)?(lld|ll|d|u|g|f|s))")


def placeholders(f):
    return [m for m in PH.finditer(f) if m.group(1) != "%"]


def index_after(f, literal):
    at = f.index(literal) + len(literal)
    for i, m in enumerate(placeholders(f)):
        if m.start() >= at:
            return i
    raise KeyError(literal)


def cprintf(f, overrides=None):
    """C printf of f with a default per placeholder ('-' for %s, 0 for numbers) and overrides {index or literal: value}."""
    ph = placeholders(f)
    vals = ["-" if m.group(3) == "s" else (0.0 if m.group(3) in ("f", "g") else 0) for m in ph]
    for k, v in (overrides or {}).items():
        vals[k if isinstance(k, int) else index_after(f, k)] = v
    py = re.sub(r"%(\.\d+)?(lld|ll|u)", lambda m: "%" + (m.group(1) or "") + "d", f)
    return py % tuple(vals)


class Formats:
    def __init__(self, cpp):
        self.seed = fmt_on_line(cpp, 'TEXT("CosmicBalance: seed %d')
        self.race = fmt_on_line(cpp, 'TEXT("CosmicBalance race: %s")')
        # The race block's parts: from its first part ("home %s cap") to the race line's UE_LOG.
        lines = cpp.splitlines()
        start = next(i for i, l in enumerate(lines) if 'TEXT("home %s cap %d")' in l)
        stop = next(i for i, l in enumerate(lines) if 'TEXT("CosmicBalance race: %s")' in l)
        self.parts = [re.search(r'TEXT\("((?:[^"\\]|\\.)*)"\)', l).group(1) for l in lines[start:stop] if "Parts.Add(FString::Printf(TEXT(" in l]
        self.ending = fmt_on_line(cpp, 'TEXT("CosmicBalance ending: record')
        self.stages = fmt_on_line(cpp, 'TEXT("CosmicBalance stages:')
        self.conflict = fmt_on_line(cpp, 'TEXT("CosmicBalance conflict:')
        self.diplomacy = fmt_on_line(cpp, 'TEXT("CosmicBalance diplomacy:')
        self.wanderer = fmt_on_line(cpp, 'TEXT("CosmicBalance wanderer:')
        self.hazards = fmt_on_line(cpp, 'TEXT("CosmicBalance hazards:')
        self.calling = fmt_on_line(cpp, 'TEXT("CosmicBalance calling: lives')
        self.taken_out = fmt_on_line(cpp, "rows taken out [")
        self.race_check = fmt_on_line(cpp, 'TEXT("CosmicBalance race check: the home cap was passed at %lld boundaries')
        self.csv_header = fmt_on_line(cpp, 'FString Csv = TEXT("Tick,CivID').replace("\\n", "")
        self.rows_header = fmt_on_line(cpp, 'FString Rows = TEXT("Tick,SourceRow').replace("\\n", "")
        self.claims_header = fmt_on_line(cpp, 'FString Csv = TEXT("Claim,').replace("\\n", "")


def ue(line, verbosity="Display"):
    return "[2026.11.09-12.00.00:000][  0]LogCosmic: %s: %s" % (verbosity, line)


def race_line(F, v):
    """The race line from its 17 parts; v holds the values the test reads."""
    p = F.parts
    assert len(p) == 17, "the race line has %d parts, the parser knows 17" % len(p)
    parts = [
        cprintf(p[0], {0: "home_galaxy", 1: 2}),
        cprintf(p[1], {0: v["t4_1"], 1: v["t4_1_civ"], 2: v.get("t4_feat", "-"), 3: v["t4_2"], 4: v["t4_2_civ"], 5: -1, 6: "-", 7: "-"}),
        cprintf(p[2], {0: v["r3"], 1: v["r3_civ"], 2: v["r4"], 3: v["r4_civ"], 4: -1, 5: "-"}),
        cprintf(p[3], {0: -1, 1: "-", 2: "-", 3: -1, 4: -1}),
        cprintf(p[4], {0: 0, 1: v.get("t4_end", 0), 2: v.get("t4_most", 0)}),
        cprintf(p[5], {0: v.get("brake", 0), 1: -1, 2: 0}),
        cprintf(p[6], {0: -1, 1: "-", 2: "-", 3: -1}),
        cprintf(p[7], {0: v["on_ruled"], 1: v["on_ruled_first"], 2: v["on_ai"], 3: v["arks"], 4: 0, 5: v["expired"], 6: v["grace"],
                       7: v.get("grace_keys", ""), 8: 0, 9: 0, 10: 0, 11: 0, 12: 0, 13: v.get("erased", 0), 14: v.get("erased_ai", 0), 15: 0,
                       16: v.get("erased_ai", 0), 17: 0}),
        cprintf(p[8], {0: v["ai_treaties"], 1: v["ai_treaty_first"], 2: 0, 3: 0}),
    ] + [cprintf(x) for x in p[9:16]] + [cprintf(p[16], {0: "100:1 200:2"})]
    return cprintf(F.race, {0: "; ".join(parts)})


def write_log(path, F, v, ticks, off=False, extra=()):
    L = ["[2026.11.09-12.00.00:000][  0]LogInit: Display: Running engine for game: Cosmic_Ascension"]
    if off:
        L.append(ue(cprintf(F.taken_out, {0: B.EXPECTED_REMOVED if v.get("taken_out") is None else v["taken_out"], 1: 1.0, 2: 0.0})))
    L.append(ue(cprintf(F.seed, {0: v.get("seed", 1), 1: ticks, 2: 12345, 3: v.get("world", 999), 4: "C:/x/y.csv"})))
    L.append(ue(cprintf(F.hazards, {0: " Hazard.Extinction %d Hazard.Plague 1" % (v["on_ruled"] + v["on_ai"]), 1: " none", 2: " none", 3: " none"})))
    L.append(ue(cprintf(F.diplomacy)))
    L.append(ue(cprintf(F.conflict, {0: 9, 1: 2, 3: 4})))
    L.append(ue(cprintf(F.stages, {"AI ": v.get("ai_us", 10.5)})))
    if v.get("crowned") is not None:
        L.append(ue(cprintf(F.wanderer, {"crowned at ": v["crowned"], "crowned at %lld (": "civ_2", "tier ": 3, "path ": "Vacuum"})))
    L.append(ue(cprintf(F.calling, {"death chances %d (by end %d, grace %d, branch side %d) [": "Extinction@1200:g:-1,PreSupernova@1500:-:-1"})))
    L.append(ue(cprintf(F.ending, {"record ": "None", "record %s at ": -1, "ruled T3 at ": v["r3"], "ruled T3 at %lld (": v["r3_civ"],
                                    "first AI T3 at ": v["ai3"], "first AI T3 at %lld (": v.get("ai3_civ", "civ_3")})))
    L.append(ue(race_line(F, v)))
    L.extend(extra)
    with open(path, "w", encoding="utf-8-sig", newline="\r\n") as f:
        f.write("\n".join(L) + "\n")


# civ -> (archetype, tick of T3, tick of T4, tick erased or None); civ_7 is the ruled people (empty Expansion)
CIVS = {"civ_3": (300, 1300, None), "civ_4": (500, 1700, None), "civ_5": (800, None, None), "civ_7": (400, 1500, None)}


def write_csv(path, F, ticks_to, civs=CIVS, change=None, micros=0.0):
    """One row per civ per 100-tick sample; change(tick, civ, row) may alter a row (a world that moved)."""
    header = F.csv_header.split(",")
    lines = [F.csv_header]
    for tick in range(0, ticks_to + 1, 100):
        for civ, (t3, t4, gone) in sorted(civs.items()):
            if gone is not None and tick >= gone:
                continue
            tier = 4 if (t4 is not None and tick >= t4) else (3 if tick >= t3 else 2)
            row = {h: "0" for h in header}
            row.update({"Tick": str(tick), "CivID": civ, "Archetype": "arch_x", "Tier": str(tier), "Systems": "4" if tier >= 3 else "3",
                        "Energy": "%.2f" % (100 + tick / 10.0), "Matter": "%.2f" % (50 + tick / 20.0), "Expansion": "" if civ == "civ_7" else "Grew",
                        "TickMicros": "%.1f" % (micros + tick), "SaveBytes": str(1000 + int(micros)), "SaveRawBytes": str(3000 + int(micros)),
                        "DrainPressure": "", "FirstClaimTick": ""})
            if change:
                change(tick, civ, row)
            lines.append(",".join(row[h] for h in header))
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


def write_rows(path, F, entries):
    with open(path, "w", encoding="utf-8") as f:
        f.write(F.rows_header + "\n" + "".join("%d,%s,%d\n" % e for e in entries))


def write_claims(path, F):
    with open(path, "w", encoding="utf-8") as f:
        f.write(F.claims_header + "\n")

# ------------------------------------------------------------------------------------------------------------- tests


ON = {"r3": 400, "r3_civ": "civ_7", "r4": 1500, "r4_civ": "civ_7", "ai3": 300, "t4_1": 1300, "t4_1_civ": "civ_3", "t4_feat": "feat_passage_by_ark",
      "t4_2": 1700, "t4_2_civ": "civ_4", "on_ruled": 1, "on_ruled_first": 900, "on_ai": 2, "arks": 2, "expired": 1, "grace": 1,
      "grace_keys": "1200:civ_5:a", "ai_treaties": 2, "ai_treaty_first": 800, "erased": 1, "erased_ai": 1, "t4_end": 2, "t4_most": 2, "brake": 30}
OFF = dict(ON, t4_1=-1, t4_1_civ="-", t4_feat="-", t4_2=-1, t4_2_civ="-", r4=-1, r4_civ="-", on_ruled=0, on_ruled_first=-1, on_ai=0, arks=0,
           expired=0, grace=0, grace_keys="", ai_treaties=0, ai_treaty_first=-1, erased=0, erased_ai=0, t4_end=0, t4_most=0, brake=0)
NONE_T3 = dict(OFF, r3=-1, r3_civ="-", ai3=-1)


def make_pair(F, folder, jobs, name, on_v, off_v, csv_change=None, rows_on=None, rows_off=None, crowned=None):
    on = jobs[name]
    off = jobs[name + "_off"]
    for job, v, is_off in ((on, on_v, False), (off, off_v, True)):
        vv = dict(v, seed=job["seed"])
        if crowned is not None:
            vv["crowned"] = crowned
        write_log(job["log"], F, vv, job["ticks"], off=is_off)
        write_csv(job["csv"], F, 3000, change=None if is_off else csv_change, micros=7.0 if is_off else 3.0)
        if is_off:
            write_rows(job["rows"], F, rows_off or [])
        else:
            write_rows(job["rows"], F, rows_on if rows_on is not None else [(900, "crisis_species_extinction", 1)])
        write_claims(job["claims"], F)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cpp", default=os.path.join(HERE, "cmdlet_head.cpp"))
    a = ap.parse_args()
    F = Formats(load_cpp(a.cpp))
    folder = os.path.join(HERE, "selftest")
    if os.path.exists(folder):
        shutil.rmtree(folder)
    os.makedirs(folder)

    print("1. job list")
    full = B.build_jobs(r"C:\x\b6b_bundleA")
    check(len(full) == 240, "240 jobs (got %d)" % len(full))
    check(all(j["ticks"] == 20000 for j in full[:120]) and all(j["ticks"] == 6000 for j in full[120:]), "20,000-tick jobs first")
    check(len({j["name"] for j in full}) == 240, "unique names")
    check(sum(1 for j in full if j["off"]) == 120 and all((B.OFF_ARG in j["args"]) == j["off"] for j in full), "off runs carry -WithoutB6bData=1")
    check(all(j["args"][0] == "-Rules=ProvisionalEndTick:0" for j in full), "every run -Rules=ProvisionalEndTick:0")
    cells = {(j["cell"], j["ticks"], len([x for x in full if x["cell"] == j["cell"]])) for j in full}
    check(cells == {("R", 20000, 60), ("inputless", 20000, 60), ("wanderer", 6000, 120)}, "cells R 30x2, input-less 30x2, wanderer 60x2: %s" % cells)
    w = next(j for j in full if j["cell"] == "wanderer")
    check(w["args"] == ["-Rules=ProvisionalEndTick:0"] + B.R_ARGS + ["-Abdicate=1200", "-Wander=1"], "wanderer = R + -Abdicate=1200 -Wander=1")
    check(next(j for j in full if j["cell"] == "inputless")["args"] == ["-Rules=ProvisionalEndTick:0"], "input-less: no policy argument")
    cmd = B.command_of(full[1], r"C:\p\Cosmic_Ascension.uproject", "E.exe")
    check(cmd[:6] == ["E.exe", r"C:\p\Cosmic_Ascension.uproject", "-run=CosmicBalance", "-Seed=1", "-Ticks=20000", "-Out=C:/x/b6b_bundleA/R_1_off.csv"]
          and cmd[-3:] == ["-unattended", "-nosplash", "-AbsLog=C:/x/b6b_bundleA/R_1_off.log"], "command shape: %s" % cmd)

    print("2. parser against the commandlet's formats")
    jobs = {j["name"]: j for j in B.build_jobs(folder, max_seed=2)}
    # R_1: T 300 (AI T3); the on world moves at 400 -> passes, differs after T.
    make_pair(F, folder, jobs, "R_1", ON, OFF,
              csv_change=lambda t, c, r: r.update(Energy="1.00") if (t >= 400 and c == "civ_7") else None,
              rows_on=[(900, "crisis_species_extinction", 1), (1100, "project_ark", 1)], rows_off=[])
    # R_2: neither T3 -> T = the whole run; equal throughout -> passes.
    make_pair(F, folder, jobs, "R_2", NONE_T3, NONE_T3, rows_on=[(500, "log_x", 2)], rows_off=[(500, "log_x", 2)])
    # input-less pairs: equal through T.
    make_pair(F, folder, jobs, "inputless_1", dict(ON, r4=-1, r4_civ="-"), OFF, rows_on=[(900, "crisis_species_extinction", 1)], rows_off=[])
    make_pair(F, folder, jobs, "inputless_2", dict(ON, r4=-1, r4_civ="-", ai3=500, ai3_civ="civ_4"), dict(OFF, ai3=500, ai3_civ="civ_4"),
              rows_on=[(900, "crisis_species_extinction", 1)], rows_off=[])
    # wanderer_1: T 300 but the on CSV differs at 200 -> must fail.
    make_pair(F, folder, jobs, "wanderer_1", ON, OFF, csv_change=lambda t, c, r: r.update(Stability="0.5000") if t >= 200 else None, crowned=3121)
    # wanderer_2: the _rows.csv differs at 600 > T 300 -> passes.
    make_pair(F, folder, jobs, "wanderer_2", ON, OFF, rows_on=[(600, "crisis_species_extinction", 1)], rows_off=[(600, "log_y", 1)], crowned=-1)

    p = B.parse_log(jobs["R_1"]["log"])
    check(p["seed_line"] == {"seed": 1, "ticks": 20000, "hash": 12345, "world": 999}, "seed line %s" % p["seed_line"])
    check(all(v is not None for v in p["race"].values()), "every race part read: %s" % [k for k, v in p["race"].items() if v is None])
    r = p["race"]
    check(r["ai_t4"]["t4_1"] == 1300 and r["ai_t4"]["t4_1_civ"] == "civ_3" and r["ai_t4"]["t4_feat"] == "feat_passage_by_ark" and r["ai_t4"]["t4_2"] == 1700,
          "part 2 %s" % r["ai_t4"])
    check(r["ruled"]["r3"] == 400 and r["ruled"]["r4"] == 1500 and r["ruled"]["r4_civ"] == "civ_7", "part 3 %s" % r["ruled"])
    check(r["extinction"]["on_ruled"] == 1 and r["extinction"]["on_ruled_first"] == 900 and r["extinction"]["on_ai"] == 2 and r["extinction"]["arks"] == 2
          and r["extinction"]["expired"] == 1 and r["extinction"]["grace_keys"] == "1200:civ_5:a" and r["extinction"]["erased_ai"] == 1, "part 8 %s" % r["extinction"])
    check(r["treaties"]["ai_treaties"] == 2 and r["treaties"]["ai_treaty_first"] == 800, "part 9 %s" % r["treaties"])
    check(r["t4civs"]["most"] == 2 and r["brake"]["held"] == 30 and r["home"]["cap"] == 2, "parts 1, 5, 6")
    check(p["ending"] and p["ending"]["r3"] == 400 and p["ending"]["ai3"] == 300 and p["ending"]["r3_civ"] == "civ_7", "ending %s" % p["ending"])
    check(p["ai_us"] == 10.5, "stages AI us %s" % p["ai_us"])
    check(p["conflict"] and p["conflict"]["begun"] == 9 and p["conflict"]["on_ruled"] == 2 and p["conflict"]["ai_sieges"] == 7, "conflict %s" % p["conflict"])
    check(p["hazards"]["placed"] == {"Hazard.Extinction": 3, "Hazard.Plague": 1}, "hazards %s" % p["hazards"])
    check([c["tag"] for c in p["chances"]] == ["Extinction", "PreSupernova"] and p["chances"][0]["flags"] == "g", "calling chances %s" % p["chances"])
    check(p["rows_taken_out"] is None and p["errors"] == [] and p["end_check"] == 0 and p["race_check"] == 0, "clean on log")
    po = B.parse_log(jobs["R_1_off"]["log"])
    check(po["rows_taken_out"] == B.EXPECTED_REMOVED and po["rows_taken_out_lines"] == 1, "off log: rows taken out %s" % po["rows_taken_out"])
    pw = B.parse_log(jobs["wanderer_1"]["log"])
    check(pw["crowned"] and pw["crowned"]["tick"] == 3121 and pw["crowned"]["civ"] == "civ_2", "wanderer crowned %s" % pw["crowned"])
    check(B.threshold_of(p["ending"], 20000) == 300, "T = min(ruled T3, first AI T3)")
    check(B.threshold_of(B.parse_log(jobs["R_2"]["log"])["ending"], 20000) == 20000, "T = the run's ticks when neither reached T3")

    print("3. run failures")
    check(B.run_failures(jobs["R_1"], p, 0) == [], "a good run passes: %s" % B.run_failures(jobs["R_1"], p, 0))
    check(any("exit code 3" in w for w in B.run_failures(jobs["R_1"], p, 3)), "exit code fails")
    bad = os.path.join(folder, "bad")
    os.makedirs(bad)
    bj = B.build_jobs(bad, cells={"R"}, max_seed=1)[0]
    write_log(bj["log"], F, dict(ON, seed=1), 20000, extra=[ue(cprintf(F.race_check, {0: 2, 1: 5000}), "Error")])
    write_csv(bj["csv"], F, 300)
    write_rows(bj["rows"], F, [])
    why = B.run_failures(bj, B.parse_log(bj["log"]), 0)
    check(any("race check" in w for w in why) and any("LogCosmic: Error" in w for w in why) and any("_claims.csv" in w for w in why),
          "race check + error + missing claims: %s" % why)
    why = B.run_failures(bj, B.parse_log(bj["log"]), 0, ignore_errors=[r"race check"])
    check(not any("LogCosmic: Error" in w for w in why) and any("race check" in w for w in why), "--ignore-error drops only the error rule: %s" % why)
    with open(bj["log"], "w", encoding="utf-8") as f:
        f.write(ue(cprintf(F.seed, {1: 20000})) + "\n")
    write_claims(bj["claims"], F)
    why = B.run_failures(bj, B.parse_log(bj["log"]), 0)
    check(any("no 'race:' line" in w for w in why), "missing race line fails: %s" % why)
    os.remove(bj["log"])
    check(B.run_failures(bj, B.parse_log(bj["log"]), None) == ["no log"], "missing log fails")

    print("4. pair check")
    parsed = {n: B.parse_log(j["log"]) for n, j in jobs.items()}
    pr = {n: B.pair_check(jobs[n], jobs[n + "_off"], parsed[n], parsed[n + "_off"]) for n in jobs if not n.endswith("_off")}
    check(pr["R_1"]["ok"] and pr["R_1"]["differs_after_T"] and pr["R_1"]["first_diff_csv"] == 400 and pr["R_1"]["T"] == 300,
          "R_1: differs after T only -> passes %s" % pr["R_1"])
    check(pr["R_2"]["ok"] and not pr["R_2"]["differs_after_T"] and pr["R_2"]["first_diff_csv"] is None, "R_2: equal pair passes %s" % pr["R_2"])
    check(not pr["wanderer_1"]["ok"] and any("csv differs at tick 200 <= T 300" in w for w in pr["wanderer_1"]["why"]),
          "wanderer_1: differs before T -> fails %s" % pr["wanderer_1"]["why"])
    check(pr["wanderer_2"]["ok"] and pr["wanderer_2"]["first_diff_rows"] == 600, "wanderer_2: rows differ after T -> passes %s" % pr["wanderer_2"])
    check(pr["inputless_1"]["ok"], "inputless_1 passes %s" % pr["inputless_1"]["why"])
    # TickMicros/SaveBytes/SaveRawBytes differ on every row (micros 3 vs 7) and are not compared: R_2 equal proves it.
    # T must agree on both sides.
    po2 = dict(parsed["R_1_off"], ending=dict(parsed["R_1_off"]["ending"], ai3=200))
    check(any("T differs" in w for w in B.pair_check(jobs["R_1"], jobs["R_1_off"], parsed["R_1"], po2)["why"]), "a T that differs fails")
    # Tick == T is compared too (Tick <= T): a change at exactly T fails.
    eq = os.path.join(folder, "edge")
    os.makedirs(eq)
    ej = {j["name"]: j for j in B.build_jobs(eq, cells={"R"}, max_seed=1)}
    make_pair(F, eq, ej, "R_1", ON, OFF, csv_change=lambda t, c, r: r.update(Matter="0.00") if t == 300 else None)
    pe = B.pair_check(ej["R_1"], ej["R_1_off"], B.parse_log(ej["R_1"]["log"]), B.parse_log(ej["R_1_off"]["log"]))
    check(not pe["ok"] and pe["first_diff_csv"] == 300, "a difference at Tick == T fails %s" % pe["why"])

    print("5. switch controls")
    quads = [(jobs[n], jobs[n + "_off"], parsed[n], parsed[n + "_off"], pr[n]) for n in pr]
    sc = B.switch_controls(quads)
    check(sc["ok"], "controls pass on the fabricated bundle: %s" % sc["problems"])
    bad_off = dict(parsed["R_1_off"], rows_taken_out="project_binding", rows_taken_out_lines=1)
    sc = B.switch_controls([(jobs["R_1"], jobs["R_1_off"], parsed["R_1"], bad_off, pr["R_1"])])
    check(not sc["ok"] and any("rows taken out [project_binding]" in x for x in sc["problems"]), "wrong rows-taken-out list fails")
    missing_off = dict(parsed["R_1_off"], rows_taken_out=None, rows_taken_out_lines=0)
    check(not B.switch_controls([(jobs["R_1"], jobs["R_1_off"], parsed["R_1"], missing_off, pr["R_1"])])["ok"], "an off log without the line fails")
    on_with = dict(parsed["R_1"], rows_taken_out_lines=1)
    check(not B.switch_controls([(jobs["R_1"], jobs["R_1_off"], on_with, parsed["R_1_off"], pr["R_1"])])["ok"], "an on log with the line fails")
    acting = json.loads(json.dumps(parsed["R_1_off"]))
    acting["race"]["extinction"]["on_ai"] = 1
    acting["race"]["treaties"]["ai_treaties"] = 1
    acting["race"]["ai_t4"]["t4_1"] = 5000
    sc = B.switch_controls([(jobs["R_1"], jobs["R_1_off"], parsed["R_1"], acting, pr["R_1"])])
    check(not sc["ok"] and len([x for x in sc["problems"] if x.startswith("R_1_off")]) == 3, "off run with extinction/treaty/AI T4 fails: %s" % sc["problems"])
    quiet = [(jobs[n], jobs[n + "_off"], parsed["R_2"], parsed[n + "_off"], pr["R_2"]) for n in ("R_2",)]
    sc = B.switch_controls(quiet)
    check(not sc["ok"] and len(sc["problems"]) == 3, "on side with no placement, no AI treaty, no pair differing after T fails: %s" % sc["problems"])

    print("6. sha1 guard")
    g = os.path.join(folder, "guard")
    os.makedirs(os.path.join(g, "Binaries", "Win64"))
    os.makedirs(os.path.join(g, "Content", "CA", "Data"))
    for rel in (r"Binaries\Win64\UnrealEditor-Cosmic_Ascension.dll", r"Content\CA\Data\DT_Hazards.uasset", r"Content\CA\Data\DT_Feats.uasset"):
        with open(os.path.join(g, rel), "wb") as f:
            f.write(rel.encode())
    files = B.guarded_files(g)
    check(len(files) == 3, "guarded files %s" % files)
    before = B.sha1_files(files)
    check(B.sha1_diff(before, B.sha1_files(files)) == [], "unchanged files pass")
    with open(os.path.join(g, r"Content\CA\Data\DT_Feats.uasset"), "ab") as f:
        f.write(b"x")
    d = B.sha1_diff(before, B.sha1_files(files))
    check(len(d) == 1 and d[0].startswith("DT_Feats.uasset"), "a changed asset is caught: %s" % d)

    print("7. 4.5 pieces")
    run = {"r4": 1500, "r4_civ": "civ_7", "ai_t4": [(1300, "civ_3")], "erased": {}}
    d = B.direct_resample([run], 1200, 0.375, 1200, 20000)
    check(d["ruled_first"] == 1 and d["end"]["median"] == 2700, "ruled binds first at 1500+1200: %s" % d["end"])
    d = B.direct_resample([dict(run, erased={"civ_7": 2000})], 1200, 0.375, 1200, 20000)
    check(d["other"] == 1 and d["end"]["median"] == 1300 + 1200 / 0.375, "an erased ruled people does not bind: %s" % d["end"])
    d = B.direct_resample([dict(run, erased={"civ_7": 2000, "civ_3": 3000})], 1200, 0.375, 1200, 20000)
    check(d["none"] == 1, "both erased: no binder")
    d = B.direct_resample([dict(run, erased={"civ_7": 2700})], 1200, 0.375, 1200, 20000)
    check(d["other"] == 1, "erased at the bind tick itself counts as before it (sample bound)")
    fake = {r: {"other_share": B.share(int(k), 100)} for r, k in zip(B.LADDER_R, (5, 10, 40, 30, 50))}
    check(B.pick_r(fake)[0] == 0.4375, "r moves one step into the band: %s" % (B.pick_r(fake),))
    fake[0.375] = {"other_share": B.share(25, 100)}
    check(B.pick_r(fake)[0] == 0.375, "r kept inside the band")
    check(B.pick_r({r: {"other_share": B.share(1, 100)} for r in B.LADDER_R})[0] is None, "no ladder value -> [개발자]")
    check(B.censored_median([100, None, 300]) == 300 and B.censored_median([100, None]) == float("inf"), "censored median")
    s = B.share(10, 20)
    check("재지 않음 — 잰 20" in s["text"] and not s["judged"], "n < 30 is '재지 않음 — 잰 N'")
    lo, hi = B.wilson(30, 100)
    check(abs(lo - 0.2189) < 1e-3 and abs(hi - 0.3958) < 1e-3, "Wilson 30/100 = [0.219, 0.396]: %s %s" % (lo, hi))

    print("8. --check-only over the fabricated folder")
    # The bundle fails (wanderer_1's pair), and the summary is written.
    code = B.main(["--check-only", folder, "--max-seed", "2", "--root", folder])
    check(code == 1, "check-only exits 1 for the failed pair (got %s)" % code)
    with open(os.path.join(folder, "summary.json"), encoding="utf-8") as f:
        sj = json.load(f)
    check(os.path.exists(os.path.join(folder, "summary.md")), "summary.md written")
    pairs = {x["pair"]: x["ok"] for x in sj["pairs"]}
    check(pairs == {"R_1": True, "R_2": True, "inputless_1": True, "inputless_2": True, "wanderer_1": False, "wanderer_2": True}, "pairs %s" % pairs)
    check(sj["controls"]["ok"], "controls in the summary: %s" % sj["controls"]["problems"])
    rc = sj["recompute_45"]
    check(rc["inputs"]["L"] == [100] and rc["inputs"]["FIRST"] == 400 and rc["t"]["t"] == 1500 and rc["t"]["n"] == 1, "4.5 inputs %s t %s" % (rc["inputs"]["L"], rc["t"]))
    check(rc["inputs"]["AIOFF"] == [0, 200, 500], "AIOFF rank medians from the input-less CSVs: %s" % rc["inputs"]["AIOFF"])
    check("1800" in rc["mc"] and "0.375" in rc["mc"]["1800"], "the Monte Carlo ran")
    sp = sj["R three-part split"]
    check(sp["runs"] == 1 and sp["strike wait"]["median"] == 500 and sp["ark build (to 100 ticks)"]["median"] == 200 and sp["treaty wait (to 100 ticks)"]["median"] == 400,
          "three-part split 500/200/400: %s" % sp)
    on = sj["cells"]["R"]["on"]
    check(on["extinction"]["arks"] == 2 and on["extinction"]["completed within the countdown"]["k"] == 2
          and on["extinction"]["completed within the countdown"]["n"] == 3, "completed share = arks/(arks+expired)")
    check(sj["cells"]["wanderer"]["on"]["crowned"]["crowned"]["k"] == 1, "B1 guard: one crowned of the two wanderer on-runs that passed")

    print("\nself-test: %d checks passed, %d failed" % (PASSES[0], len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
