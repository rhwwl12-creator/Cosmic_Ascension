"""Self-test of m_awake.py: fabricates bundle A outputs, baseline logs and A' folders under scratchpad\\b6b7\\selftest\\ in
the commandlet's formats (printed from the C++ format strings of CosmicBalanceCommandlet.cpp - the git show copy
scratchpad\\b6b8\\ref\\cmdlet_head.cpp, or scratchpad\\b6b6\\cmdlet_head.cpp) and checks every computation and branch.

  python -I -X utf8 m_awake_selftest.py [--cpp <copy>]
"""
import argparse
import importlib.util
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


M = load("m_awake", os.path.join(HERE, "m_awake.py"))
T = load("bundleA_selftest", os.path.join(SCRATCH, "b6b6", "bundleA_selftest.py"))  # cprintf, fmt_on_line, Formats
A = M.A

FAILS, PASSES = [], [0]


def check(cond, what):
    if cond:
        PASSES[0] += 1
    else:
        FAILS.append(what)
        print("  FAIL: " + what)

# --------------------------------------------------------------------------------------------------- fabrication


def race_line(F, v):
    """The race line from the C++ parts (17), every part with explicit values."""
    p = F.parts
    assert len(p) == 17
    c = T.cprintf
    parts = [
        c(p[0], {0: "home_galaxy", 1: 2}),
        c(p[1], {0: v.get("t4_1", -1), 1: v.get("t4_1_civ", "-"), 2: "feat_passage_by_ark", 3: v.get("t4_2", -1), 4: v.get("t4_2_civ", "-"), 5: -1, 6: "-", 7: "-"}),
        c(p[2], {0: v.get("r3", -1), 1: v.get("r3_civ", "-"), 2: v.get("r4", -1), 3: v.get("r4_civ", "-"), 4: v.get("r5", -1), 5: v.get("r5_civ", "-")}),
        c(p[3], {0: -1, 1: "-", 2: "-", 3: -1, 4: -1}),
        c(p[4], {0: 0, 1: 0, 2: 1}),
        c(p[5], {0: 0, 1: -1, 2: 0}),
        c(p[6], {0: v.get("ai_up", -1), 1: v.get("ai_up_civ", "-"), 2: v.get("ai_up_row", "-"), 3: v.get("ruled_bind", -1)}),
        c(p[7], {1: -1, 7: ""}),
        c(p[8], {0: 1, 1: 900, 2: 0, 3: 0}),
        c(p[9], {0: v.get("laws_ruled", -1), 1: v.get("laws_ai", -1), 2: "-"}),
        c(p[10], {0: -1, 1: "-", 2: "-", 3: "-", 4: 0, 8: -1, 10: -1, 12: -1}),
        c(p[11], {0: "-", 1: -1, 2: -1}),
        c(p[12], {2: -1}),
        c(p[13], {1: -1}),
        c(p[14]),
        c(p[15]),
        c(p[16], {0: "", 1: -1}),
    ]
    return T.cprintf(F.race, {0: "; ".join(parts)})


def write_log(path, F, v, ticks, extra=()):
    ue = T.ue
    L = ["[2026.11.09-12.00.00:000][  0]LogInit: Display: Running engine for game: Cosmic_Ascension"]
    L.append(ue(T.cprintf(F.seed, {0: v.get("seed", 1), 1: ticks, 2: 12345, 3: 999, 4: "C:/x/y.csv"})))
    L.append(ue(T.cprintf(F.pressure, {"first claim line at ": v.get("first_voice", -1), "first sign at ": -1})))
    fields = "Engineering=100 Physics=200 Society=%d Transcendence=-1" % v.get("society", -1)
    L.append(ue(T.cprintf(F.ending, {"record ": v.get("kind", "None"), "record %s at ": v.get("rec", -1), "(people ": v.get("rec_civ", "-"),
                                     "a people some life ruled ": 0, "no vessel ": 0, "ruled T3 at ": v.get("r3", -1), "ruled T3 at %lld (": v.get("r3_civ", "-"),
                                     "first AI T3 at ": v.get("ai3", -1), "first AI T3 at %lld (": v.get("ai3_civ", "-"),
                                     "research 1.0 ruled ": v.get("research_civ", v.get("r4_civ", "civ_7")), "research 1.0 ruled %s [": fields,
                                     "], first AI ": "civ_3", "], first AI %s [": "Engineering=1 Physics=1 Society=1 Transcendence=1",
                                     "first-person claim lines ": v.get("voice", 0)})))
    L.append(ue(race_line(F, v)))
    L.extend(extra)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8-sig", newline="\r\n") as f:
        f.write("\n".join(L) + "\n")


def write_csv(path, F, to, civs, change=None):
    """civs: {civ: (archetype, systems_fn(tick), tier_fn(tick), ruled)}."""
    header = F.csv_header.split(",")
    lines = [F.csv_header]
    for tick in range(0, to + 1, 100):
        for civ, (arch, systems, tier, ruled) in sorted(civs.items()):
            row = {h: "0" for h in header}
            row.update({"Tick": str(tick), "CivID": civ, "Archetype": arch, "Tier": str(tier(tick)), "Systems": str(systems(tick)), "Energy": "1.00", "Matter": "1.00",
                        "Expansion": "" if ruled else "Grew", "DrainPressure": "", "FirstClaimTick": ""})
            if change:
                change(tick, civ, row)
            lines.append(",".join(row[h] for h in header))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


def write_rows(path, F, entries=()):
    with open(path, "w", encoding="utf-8") as f:
        f.write(F.rows_header + "\n" + "".join("%d,%s,%d\n" % e for e in entries))


def write_claims(path, F):
    with open(path, "w", encoding="utf-8") as f:
        f.write(F.claims_header + "\n")


def civs_for(v):
    """The ruled civ_7 with v's archetype and k (k_before until T4's first sample, k after), AIs civ_3/civ_4."""
    t4 = v.get("r4", -1)
    k_before, k_after = v.get("k_before", 3), v.get("k", 4)
    return {
        "civ_7": (v.get("arch", "arch_default"), lambda t: k_after if (t4 >= 0 and t >= t4) else k_before,
                  lambda t: 4 if (t4 >= 0 and t >= t4) else 3, True),
        "civ_3": ("arch_default", lambda t: 4, lambda t: 3, False),
        "civ_4": ("arch_seeker", lambda t: 4, lambda t: 3, False),
    }


def write_run(F, folder, name, v, ticks=20000, csv_to=6000, change=None, extra=()):
    base = os.path.join(folder, name)
    write_log(base + ".log", F, v, ticks, extra)
    write_csv(base + ".csv", F, csv_to, civs_for(v), change)
    write_rows(base + "_rows.csv", F, [(500, "log_x", 1)])
    write_claims(base + "_claims.csv", F)


# Per seed: race parts 2-3, ending, research, voice.
R = {
    1: dict(seed=1, r3=1500, r3_civ="civ_7", r4=3000, r4_civ="civ_7", ai3=1400, ai3_civ="civ_3", t4_1=1000, t4_1_civ="civ_3", t4_2=2400, t4_2_civ="civ_4",
            arch="arch_seeker", k_before=3, k=4, society=2500, kind="OtherPeople", rec=7000, rec_civ="civ_3", voice=0, first_voice=5000),
    2: dict(seed=2, r3=1500, r3_civ="civ_7", r4=3050, r4_civ="civ_7", ai3=1400, ai3_civ="civ_3", arch="arch_default", k_before=4, k=7, society=3600,
            kind="RuledPeople", rec=5000, rec_civ="civ_7", voice=1, first_voice=4000),
    3: dict(seed=3, r3=1500, r3_civ="civ_7", ai3=1400, ai3_civ="civ_3", t4_1=2000, t4_1_civ="civ_3", kind="None", rec=-1, voice=0, first_voice=-1),
    4: dict(seed=4, r3=1500, r3_civ="civ_7", r4=2000, r4_civ="civ_7", ai3=1400, ai3_civ="civ_3", t4_1=1000, t4_1_civ="civ_3", k=4, society=-1,
            kind="OtherPeople", rec=4000, rec_civ="civ_3", voice=0, first_voice=5500),
    5: dict(seed=5, r3=1500, r3_civ="civ_7", r4=2500, r4_civ="civ_7", ai3=1400, ai3_civ="civ_3", t4_1=2600, t4_1_civ="civ_3", k=5, society=2000,
            research_civ="civ_9", kind="None", rec=-1, voice=2, first_voice=8000),
    6: dict(seed=6, r3=1500, r3_civ="civ_7", r4=4000, r4_civ="civ_7", ai3=1400, ai3_civ="civ_3", t4_1=500, t4_1_civ="civ_3", k=6, society=3000,
            kind="OtherPeople", rec=9000, rec_civ="civ_3", voice=0, first_voice=6000),
}
CIVS_JSON = [{"Name": "arch_seeker", "HazardSpawnMultipliers": {"Hazard.AIAwakening": 1.2}}, {"Name": "arch_default", "HazardSpawnMultipliers": {}}]


def S(P, N, R_L, a, k, m):
    lam = 0.0002 * a * k * m
    return max(P, R_L + (math.exp(lam * N) - 1) / lam)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cpp", default=None)
    a = ap.parse_args()
    cpp_path = a.cpp or next((p for p in (os.path.join(SCRATCH, "b6b8", "ref", "cmdlet_head.cpp"), os.path.join(SCRATCH, "b6b6", "cmdlet_head.cpp")) if os.path.exists(p)), None)
    cpp = T.load_cpp(cpp_path)
    F = T.Formats(cpp)
    F.pressure = T.fmt_on_line(cpp, 'TEXT("CosmicBalance pressure:')
    root = os.path.join(HERE, "selftest")
    if os.path.exists(root):
        shutil.rmtree(root)
    os.makedirs(root)
    bundle = os.path.join(root, "b6b_bundleA")
    for seed, v in R.items():
        write_run(F, bundle, "R_%d" % seed, v)
    # Input-less on-runs for the 4.5 resample's Monte Carlo inputs.
    for seed in (1, 2):
        write_run(F, bundle, "inputless_%d" % seed, dict(seed=seed, ai3=1400, ai3_civ="civ_3", kind="None", rec=-1))
    civs_path = os.path.join(root, "DT_Civs.json")
    with open(civs_path, "w", encoding="utf-8") as f:
        json.dump(CIVS_JSON, f)

    print("1. parsing the extra parts")
    p = M.parse_run(os.path.join(bundle, "R_1.log"))
    check(p["ending_x"] == {"kind": "OtherPeople", "tick": 7000, "civ": "civ_3", "some_life_ruled": 0, "no_vessel": 0}, "ending head %s" % p["ending_x"])
    check(p["research"]["civ"] == "civ_7" and p["research"]["fields"]["Society"] == 2500, "research %s" % p["research"])
    check(p["voice_by_record"] == 0 and p["first_voice"] == 5000, "voice %s / first %s" % (p["voice_by_record"], p["first_voice"]))
    check(p["laws"] and p["laws"]["ruled"] == -1 and p["laws"]["ai"] == -1, "laws part %s" % p["laws"])
    check(p["race"]["ruled"]["r4"] == 3000 and p["race"]["ai_t4"]["t4_2"] == 2400, "race parts via bundleA")
    check(M.fields_of("A=1 B=-1 junk C=x") == {"A": 1, "B": -1}, "fields_of")

    print("2. section 0.4 rows")
    factors = M.awakening_factors(CIVS_JSON)
    check(factors == {"arch_seeker": 1.2}, "factors %s" % factors)
    rows, skipped = M.m_rows(bundle, set(range(1, 7)), factors, 1800, 1200)
    by = {r["seed"]: r for r in rows}
    check(sorted(by) == [1, 2, 4, 5, 6] and [s for s, _ in skipped] == [3], "rows 1,2,4,5,6; seed 3 skipped (no ruled T4): %s %s" % (sorted(by), skipped))
    check(by[1]["k"] == 4 and by[1]["a"] == 1.2 and by[1]["R_L"] == 0, "seed 1: k 4 at T4's own sample, a 1.2, R_L 0 %s" % by[1])
    check(by[2]["k"] == 7 and by[2]["a"] == 1.0 and by[2]["R_L"] == 550, "seed 2: k at the first sample >= 3050 (3100) is 7, a 1, R_L 550 %s" % by[2])
    check(math.isinf(by[4]["S"][0.5]) and "never" in by[4]["note"], "seed 4: Society never at 1.0 -> S inf")
    check("civ_9" in by[5]["note"] and by[5]["R_L"] == 0, "seed 5: research people differs -> noted, R_L max(0, 2000 - 2500) = 0")
    for seed, (R_L, a_, k) in {1: (0, 1.2, 4), 2: (550, 1.0, 7), 5: (0, 1.0, 5), 6: (0, 1.0, 6)}.items():
        for m in M.M_LADDER:
            check(abs(by[seed]["S"][m] - S(1800, 1200, R_L, a_, k, m)) < 1e-6, "S seed %d m %s" % (seed, m))
    check(abs(M.season_of(1800, 1200, 0, 1.0, 4, 0.5) - 1800) < 1e-9 and abs(M.season_of(0, 1200, 0, 1.0, 4, 0.5) - 1540) < 1.0,
          "brief's 1,540 at k 4, m 0.5 before the a factor")
    check(abs(M.season_of(0, 1200, 0, 1.0, 7, 0.5) - 1880.5) < 1.0, "brief's 1,881 at k 7")

    print("3. section 0.4 rule branches")
    d = M.decide_m(rows, 1800, 1200)
    med = sorted(r["S"][0.5] for r in rows)[2]
    check(d["n"] == 5 and d["medians"][0.5] == med and d["m"] == 0.5 and not d["developer"], "pick 0.5 (median %s): %s" % (med, d))
    fake = lambda vals: [{"S": dict(zip(M.M_LADDER, v))} for v in vals]  # noqa: E731
    d = M.decide_m(fake([(4000, 3500, 3000)] * 3 + [(3000, 2000, 1000)] * 2), 1800, 1200)
    check(d["m"] == 0.375 and not d["developer"], "pick 0.375 when 0.5's median is above 3,600: %s" % d)
    d = M.decide_m(fake([(5000, 4800, 4500)] * 5), 1800, 1200)
    check(d["m"] == 0.5 and d["developer"], "none fits -> keep 0.5 and [개발자]: %s" % d)
    d = M.decide_m(fake([(4000, 3000, 2000)] * 3 + [(3000, 3000, 2000)] * 3), 1800, 1200)
    check(d["m"] == 0.5 and d["upper_disagrees"], "even count: lower middle picks 0.5, upper middle would not -> flagged: %s" % d)
    d = M.decide_m(fake([(math.inf, math.inf, math.inf)] * 3 + [(2000, 2000, 2000)] * 3), 1800, 1200)
    check(d["m"] == 0.5 and not d["developer"], "inf sorts last; the lower middle of 6 is a finite value")
    d = M.decide_m(fake([(1000, 1000, 1000)] * 4), 1800, 1200)
    check(d["model"] is not None and d["m"] == 0.5 and not d["developer"], "n < 5 -> the model, 0.5: %s" % d)
    d = M.decide_m([], 6000, 1200)
    check(d["model"] is not None and d["developer"] and d["m"] == 0.5, "n 0 with a model above 3,600 (P 6,000) -> [개발자]")
    check(M.lower_median([1, 2, 3, 4]) == 2 and M.upper_median([1, 2, 3, 4]) == 3 and M.lower_median([]) is None, "medians")

    print("4. #136")
    v = M.voice_reading(bundle, set(range(1, 7)))
    check(v["n"] == 6 and v["either"]["k"] == 5 and v["by_record"]["k"] == 2 and v["by_6000"]["k"] == 4 and v["by_min"]["k"] == 3,
          "either 5, by record 2, by 6,000 4 (6,000 inclusive), by min(record, 6,000) 3: %s" % {k: v[k]["k"] for k in ("either", "by_record", "by_6000", "by_min")})
    lo, hi = A.wilson(5, 6)
    check(v["either"]["text"] == "5/6 = 0.833 [%.3f, %.3f]" % (lo, hi), "Wilson text %s" % v["either"]["text"])

    print("5. section 0.5 lines moved")
    bl = os.path.join(root, "baselines")
    base_v = {"checks": (3000, dict(seed=42, t4_1=2260, t4_1_civ="civ_5")), "checks_ruler": (3000, dict(seed=42, t4_1=3000, t4_1_civ="civ_5")),
              "checks_wanderer": (6000, dict(seed=42, r4=5999, r4_civ="civ_7")), "checks_20000": (20000, dict(seed=42, t4_1=3000, t4_1_civ="civ_5", r4=3000, r4_civ="civ_7"))}
    for name, (ticks, vv) in base_v.items():
        write_log(os.path.join(bl, name + ".log"), F, vv, ticks)
    lm = M.lines_moved([os.path.join(root, "nowhere"), bl])
    st = {l["name"]: l["moved"] for l in lm["lines"]}
    check(st == {"checks": True, "checks_ruler": False, "checks_wanderer": True, "checks_20000": True, "checks_ruler_20000": None},
          "T4* < horizon moves (T4* = horizon does not); a missing log is unknown: %s" % st)
    check(lm["phrase"] is None and lm["unknown"] == ["checks_ruler_20000"], "no phrase while a line is unknown")
    l20 = next(l for l in lm["lines"] if l["name"] == "checks_20000")
    check(l20["t4"] == 3000 and l20["who"] == "AI civ_5 and ruled civ_7", "a tie names both: %s" % l20["who"])
    check({h["name"]: h["moved"] for h in lm["hashes"]} == {"hashrun": True, "hashrun_policy": False}, "hash runs follow their 3,000 lines")
    write_log(os.path.join(bl, "checks_ruler_20000.log"), F, dict(seed=42), 20000)
    lm = M.lines_moved([bl])
    check(lm["phrase"] == "기준값 입력 없는 3,000·방랑자 6,000·입력 없는 20,000 줄과 해시 판이 옮았다", "partial phrase: %s" % lm["phrase"])
    allm = os.path.join(root, "baselines_all")
    nonem = os.path.join(root, "baselines_none")
    for name, horizon, _ in M.BASELINE_RUNS:
        write_log(os.path.join(allm, name + ".log"), F, dict(seed=42, t4_1=horizon - 1, t4_1_civ="civ_5"), horizon)
        write_log(os.path.join(nonem, name + ".log"), F, dict(seed=42), horizon)
    check(M.lines_moved([allm])["phrase"] == "기준값 다섯과 해시 판 둘이 옮았다", "all five -> the [B-6b6] phrase")
    check(M.lines_moved([nonem])["phrase"] == "비트 그대로", "none -> 비트 그대로")
    check(M.t4_star({"ai_t4": {"t4_1": -1, "t4_1_civ": "-"}, "ruled": {"r4": -1, "r4_civ": "-"}}) == (-1, "-"), "T4* without any T4")

    print("6. section 0.3 resample")
    rs = M.resample_check(bundle, 1800, 0.375, 1200)
    check(rs["direct"].startswith("2/6 = 0.333") and rs["direct_inside"], "direct resample at P 1,800 r 0.375: 2/6 inside the band: %s" % rs["direct"])
    rs = M.resample_check(bundle, 1800, 0.5, 1200)
    check(rs["direct"].startswith("3/6"), "r 0.5: seed 1's AI binds at 4,600 before the ruled 4,800 -> 3/6: %s" % rs["direct"])
    rs = M.resample_check(bundle, 1800, 0.4, 1200)
    check(rs["direct"] == "-" and not rs["direct_inside"], "an off-ladder r has no row")

    print("7. section 4 (A')")
    a1 = os.path.join(root, "b6b7_A1")
    for seed, vv in R.items():
        extra = [T.ue("CosmicBalance race check: the home cap was passed", "Error")] if seed == 4 else []
        change = None
        if seed == 1:
            change = lambda t, c, r: r.update(Energy="9.00") if t >= 1100 else None  # noqa: E731  after T4* 1,000: fine
        if seed == 6:
            change = lambda t, c, r: r.update(Energy="9.00") if t >= 400 else None  # noqa: E731  before T4* 500: J1 fails
        write_run(F, a1, "R_%d" % seed, vv, change=change, extra=extra)
    j = M.j_checks(bundle, a1, set(range(1, 7)))
    bad = {r["seed"]: r["why"] for r in j["J1"] if not r["ok"]}
    check(list(bad) == [6] and "csv differs at 400 < T4* 500" in bad[6], "J1 fails only seed 6: %s" % bad)
    check(not j["J2_ok"] and [x["seed"] for x in j["J2"]] == [4], "J2 catches the race check line of seed 4: %s" % j["J2"])
    check(not j["J3"]["ok"] and j["J3"]["eligible"] == [1, 2, 5, 6], "J3 fails: Society + ruled T4 in 1, 2, 5, 6 and no laws held: %s" % j["J3"])
    check(not j["J4"]["ok"] and j["J4"]["any_t4"] == [1, 2, 3, 4, 5, 6], "J4 fails: T4s and no binding start: %s" % j["J4"])
    a1b = os.path.join(root, "b6b7_A1b")
    for seed, vv in R.items():
        vv = dict(vv)
        if seed == 2:
            vv["laws_ruled"] = 3600
        if seed == 3:
            vv.update(ai_up=2001, ai_up_civ="civ_3", ai_up_row="project_binding")
        write_run(F, a1b, "R_%d" % seed, vv)
    j = M.j_checks(bundle, a1b, set(range(1, 7)))
    check(j["J1_ok"] and j["J2_ok"] and j["J3"]["ok"] and j["J3"]["held"] == [2] and j["J4"]["ok"] and j["J4"]["wired"] == [3], "J1-J4 pass: %s %s" % (j["J3"], j["J4"]))
    a1c = os.path.join(root, "b6b7_A1c")
    for seed in (1, 2):
        write_run(F, a1c, "R_%d" % seed, dict(seed=seed, r3=1500, r3_civ="civ_7"))
    j = M.j_checks(bundle, a1c, set(range(1, 7)))
    check(j["J3"]["ok"] and not j["J3"]["eligible"] and j["J4"]["ok"] and not j["J4"]["any_t4"], "J3/J4 hold vacuously without T4s")
    check(not j["J1_ok"] and sum(1 for r in j["J1"] if r["why"].startswith("a log")) == 4, "missing A' runs fail J1")

    print("8. probe seeds")
    ps = M.probe_seeds([a1b, os.path.join(bl, "checks_20000.log")])
    check(ps["probe1"][0]["seed"] == 1 and ps["probe1"][0]["tick"] == 2400 and ps["probe1"][0]["ticks_arg"] == 2500, "PROBE_SEED_1 = seed 1 at 2,400: %s" % ps["probe1"][:2])
    check([c["seed"] for c in ps["probe1"]] == [1, 5], "probe 1 candidates: second AI T4 (1) and an AI T4 after the ruled T4 (5): %s" % [c["seed"] for c in ps["probe1"]])
    check(ps["probe2"][0]["seed"] == 4 and ps["probe2"][0]["tick"] == 2000 and [c["seed"] for c in ps["probe2"]] == [4, 6],
          "PROBE_SEED_2 = seed 4 at 2,000 (seed 1 has two AIs before its ruled T4): %s" % [c["seed"] for c in ps["probe2"]])
    check(M.probe_seeds([nonem])["probe1"] == [], "no candidate -> empty")

    print("9. main end to end")
    out = os.path.join(root, "report.md")
    code = M.main(["--root", os.path.join(root, "proj"), "--folder", bundle, "--seeds", "1-6", "--civs-json", civs_path, "--baseline-logs", bl, "--a1", a1b, "--out", out])
    check(code == 0 and os.path.exists(out) and os.path.exists(os.path.join(root, "report.json")), "main writes the report and its json")
    text = open(out, encoding="utf-8").read()
    check("**M_AWAKE = 0.5**" in text and "PROBE_SEED_1: seed 1" in text and "J3 (법의 배선): 통과" in text, "report carries the answers")
    check(M.main(["--root", root, "--folder", bundle, "--civs-json", civs_path, "--baseline-logs", bl, "--out", os.path.join(root, "x.md")]) == 2,
          "--out inside the project root is refused")
    # The real DT_Civs through git show (read-only): arch_seeker 1.2.
    try:
        civs, src = M.load_civs(M.DEFAULT_ROOT)
        check(M.awakening_factors(civs).get("arch_seeker") == 1.2, "git show HEAD:Tools/data/DT_Civs.json: arch_seeker 1.2")
    except Exception as e:  # noqa: BLE001
        print("  (git show not checked: %s)" % e)

    print("\nself-test: %d checks passed, %d failed" % (PASSES[0], len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
