"""Self-test of bundleC.py: fabricates logs, CSVs and claims files under scratchpad\\b6b11\\selftest\\ in the commandlet's
formats (printed from the C++ format strings of the git show copy scratchpad\\b6b11\\cmd_head.cpp through bundleA_selftest's
Formats; the side part in the [B-6b8] layout), and checks the job list, the refusals, the parser, every control F1-F8, the
bridges B-a..B-e, the frozen arithmetic R1-R11 with hand-computed numbers, the hand-check selection, the driver (mocked
commandlet, busy check, rerun, archive), the dry run and --cell-set B. No commandlet, no git, no build: every external call
goes through bundleC.HOOKS, swapped here. Single process, light.

  python -I -X utf8 bundleC_selftest.py [--cpp <copy>]
"""
import argparse
import contextlib
import importlib.util
import io
import json
import math
import os
import shutil
import sys
import threading
import time
import types

HERE = os.path.dirname(os.path.abspath(__file__))
SCRATCH = os.path.dirname(HERE)


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


C = load("bundleC", os.path.join(HERE, "bundleC.py"))
B, A = C.B, C.A
T = load("bundleA_selftest", os.path.join(SCRATCH, "b6b6", "bundleA_selftest.py"))

FAILS, PASSES = [], [0]
SIDE_NEW = ("era lines %d (by the record %d, first at %lld), witness cards %d (first at %lld, by the record %d; answered %d: by the check %d, "
            "failed %d, without a check %d; gone before the answer %d)")
CC, TD = 6000, 4400


def check(cond, what):
    if cond:
        PASSES[0] += 1
    else:
        FAILS.append(what)
        print("  FAIL: " + what)


def quiet(fn, *a, **k):
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        r = fn(*a, **k)
    return r, buf.getvalue()

# ----------------------------------------------------------------------------------------------------------- formats


def fill(fmt, pairs):
    """cprintf with (literal, offset, value) overrides: the offset-th placeholder after the literal's first occurrence."""
    over = {}
    for lit, off, val in pairs:
        over[T.index_after(fmt, lit) + off] = val
    return T.cprintf(fmt, over)


class F:
    pass


def formats(cpp):
    f = T.Formats(cpp)
    f.deaths = T.fmt_on_line(cpp, 'TEXT("CosmicBalance calling deaths:')
    f.pressure = T.fmt_on_line(cpp, 'TEXT("CosmicBalance pressure:')
    f.rules = T.fmt_on_line(cpp, 'TEXT("CosmicBalance: rules %s = %s")')
    return f

# ------------------------------------------------------------------------------------------------------- fabrication


def race_line(Fm, v):
    p = Fm.parts
    assert len(p) == 17, len(p)
    c = T.cprintf
    side = c(SIDE_NEW, {0: v.get("era", 0), 1: v.get("era", 0), 2: v.get("era_first", -1), 3: v.get("wit", 0), 4: v.get("wit_first", -1),
                        5: v.get("wit", 0), 6: v.get("answered", 0), 7: v.get("answered", 0), 8: 0, 9: 0, 10: 0})
    parts = [
        c(p[0], {0: "home_galaxy", 1: 2}),
        c(p[1], {0: v.get("t4_1", -1), 1: "civ_3", 2: "-", 3: -1, 4: "-", 5: v.get("t5", -1), 6: "-", 7: "-"}),
        c(p[2], {0: v.get("r3", 2000), 1: "civ_7", 2: v.get("r4", -1), 3: "civ_7", 4: v.get("r5", -1), 5: "civ_7"}),
        c(p[3], {0: v.get("fifth", -1), 1: "-", 2: "-", 3: -1, 4: v.get("silence", -1)}),
        c(p[4], {0: 0, 1: 0, 2: 1}),
        c(p[5], {0: 0, 1: -1, 2: 0}),
        c(p[6], {0: -1, 1: "-", 2: "-", 3: -1}),
        c(p[7], {0: v.get("on_ruled", 0), 1: v.get("on_ruled_first", -1), 2: v.get("on_ai", 0), 7: "", 8: v.get("ruled_expiries", 0), 9: v.get("died", 0),
                 17: v.get("once_ruled", 0)}),
        c(p[8], {0: v.get("ai_treaties", 0), 1: -1, 2: 0, 3: 0}),
        c(p[9], {0: -1, 1: -1, 2: "-"}),
        c(p[10], {0: -1, 1: "-", 2: "-", 3: "-", 4: 0, 8: -1, 10: -1, 12: -1}),
        c(p[11], {0: "-", 1: -1, 2: -1}),
        side,
        c(p[13], {0: v.get("crown_refused", 0), 1: v.get("crown_first", -1)}),
        c(p[14], {0: v.get("at_cap", ""), 1: v.get("g_ends", ""), 2: v.get("u_ends", "")}),
        c(p[15], {0: 0}),
        c(p[16], {0: "", 1: -1}),
    ]
    if v.get("break_race"):
        parts[13] = "crown gone"
    return T.cprintf(Fm.race, {0: "; ".join(parts)})


def write_log(path, Fm, v, ticks, off=False, cell="R"):
    ue = T.ue
    L = ["[2026.11.10-12.00.00:000][  0]LogInit: Display: Running engine for game: Cosmic_Ascension"]
    for k, val in v.get("rules", []):
        L.append(ue(T.cprintf(Fm.rules, {0: k, 1: val})))
    if off:
        L.append(ue(T.cprintf(Fm.taken_out, {0: v.get("taken_out", C.EXPECTED_REMOVED), 1: v.get("was_r", 1.0), 2: v.get("was_share", 0.5)})))
    L.append(ue(T.cprintf(Fm.seed, {0: v.get("seed", 1), 1: ticks, 2: 12345, 3: 999, 4: "C:/x/y.csv"})))
    L.append(ue(T.cprintf(Fm.stages, {"AI ": 10.5})))
    if not v.get("drop_conflict"):
        L.append(ue(fill(Fm.conflict, [("), taken ", 0, v.get("taken", 3)), ("), taken ", 1, v.get("bands", (1, 1, 1))[0]), ("), taken ", 2, v.get("bands", (1, 1, 1))[1]),
                                       ("), taken ", 3, v.get("bands", (1, 1, 1))[2]), ("taken by top conqueror ", 0, v.get("top", 1)),
                                       ("lost by one people max ", 0, v.get("lost", 1)), ("delay bought per tag [", 0, v.get("delay", ""))])))
    if cell == "wanderer":
        L.append(ue(T.cprintf(Fm.wanderer, {"abdicate ": 1200, "wander ": 1, "left at ": v.get("left", 1201), "crowned at ": v.get("crowned", -1),
                                           "crowned at %lld (": "civ_7", "tier ": 3, "path ": v.get("path", "none")})))
    L.append(ue(fill(Fm.deaths, [("death asked at ", 0, v.get("asked", -1)), ("(recorded ", 0, v.get("recorded", 0)), ("); deaths ", 0, v.get("deaths", 0)),
                                 ("(by end ", 0, v.get("by_end", 0)), ("first death at ", 0, v.get("fd", -1)), ("successions [", 0, v.get("succ", "")),
                                 ("per death [", 0, v.get("per", ""))])))
    pr = v.get("pressure")
    if pr:
        L.append(ue(fill(Fm.pressure, [("policy pressure ", 0, pr.get("policy", "Faithful")), ("policy pressure %s guard ", 0, pr.get("guard", 1)),
                                       ("leave near sphere ", 0, pr.get("leave", 1)), ("guarded crises ", 0, pr.get("guarded", 0)),
                                       ("guarded crises %d by tag [", 0, pr.get("gtags", "")), ("delay bought ", 0, pr.get("delay", 0)),
                                       ("delay bought %lld by tag [", 0, pr.get("dtags", "")), ("401-550 ", 0, pr.get("band", 0)),
                                       ("401-550 %d eased ", 0, pr.get("eased", 0))])))
    g = v.get("g132", ("-", "-", "-"))
    L.append(ue(fill(Fm.ending, [("record ", 0, v.get("kind", "Clock")), ("record %s at ", 0, v.get("rec", CC)), ("(people ", 0, v.get("civ", "-")),
                                 ("some life ruled ", 0, 0), ("no vessel ", 0, v.get("novessel", 0)), ("ruled T3 at ", 0, v.get("r3", 2000)),
                                 ("ruled T3 at %lld (", 0, "civ_7"), ("first AI T3 at ", 0, v.get("ai3", 1800)), ("first AI T3 at %lld (", 0, "civ_3"),
                                 ("first-person claim lines ", 0, v.get("fp", 0)), ("#132 guarded [", 0, g[0]), ("#132 guarded [", 1, g[1]),
                                 ("#132 guarded [", 2, g[2]), ("keys [", 0, v.get("keys", ""))])))
    L.append(ue(race_line(Fm, v)))
    L.extend(v.get("extra", []))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8-sig", newline="\r\n") as f:
        f.write("\n".join(L) + "\n")


def write_csv(path, Fm, to, changes=(), extra_ticks=()):
    header = Fm.csv_header.split(",")
    lines = [Fm.csv_header]
    for tick in sorted(set(list(range(0, to + 1, 100)) + [t for t in extra_ticks if t <= to])):
        label = "1.00"
        for t, lab in sorted(changes):
            if tick >= t:
                label = "%.2f" % (100 + sum(map(ord, lab)))
        for civ in ("civ_3", "civ_7"):
            row = {h: "0" for h in header}
            row.update({"Tick": str(tick), "CivID": civ, "Archetype": "arch_default", "Tier": "3", "Systems": "4", "Energy": label if civ == "civ_3" else "1.00",
                        "Matter": "1.00", "Expansion": "" if civ == "civ_7" else "Grew", "TickMicros": str(tick % 7), "SaveBytes": str(tick % 5),
                        "SaveRawBytes": str(tick % 3), "DrainPressure": "", "FirstClaimTick": ""})
            lines.append(",".join(row[h] for h in header))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


def claim(**kw):
    r = {"Claim": "1", "SystemID": "sys_1", "Owner": "civ_7", "Ruled": "1", "PlacedTick": "1000", "PlacedCount": "1", "Guard": "1", "GuardDelay": "0",
         "Vigil": "1", "CardTick": "1100", "RemainingAtCard": "450", "Resisted": "0", "VigilKept": "1", "Relief": "0", "OwnerChanged": "0", "EndTick": "1500",
         "EndKind": "eased", "RemainingAtEnd": "0", "SealedTick": "-1", "AfterBinding": "0"}
    r.update({k: str(v) for k, v in kw.items()})
    return r


def write_claims(path, Fm, rows=()):
    cols = Fm.claims_header.split(",")
    with open(path, "w", encoding="utf-8") as f:
        f.write(Fm.claims_header + "\n")
        for i, r in enumerate(rows, 1):
            r = dict(r, Claim=str(i))
            f.write(",".join(r[c] for c in cols) + "\n")


def write_job(Fm, job, v, changes=(), claims=(), extra_ticks=()):
    write_log(job["log"], Fm, v, job["ticks"], job["off"], job["cell"])
    write_csv(job["csv"], Fm, job["ticks"], changes, extra_ticks)
    with open(job["rows"], "w", encoding="utf-8") as f:
        f.write(Fm.rows_header + "\n500,log_x,1\n")
    write_claims(job["claims"], Fm, claims)

# ------------------------------------------------------------------------------------------------- the clean fixture


def specs():
    """(name -> (v, changes, claims, extra_ticks)) for bundle C seeds 1-2 (test 1-2): every control and bridge passes."""
    g = ("Hazard.AIAwakening=2:48.0%", "Hazard.AIAwakening=1:50.0% Hazard.Plague=1:40.0%", "-")
    test_claims = [claim(EndKind="eased"), claim(EndKind="joined", SystemID="sys_2"), claim(EndKind="eased", RemainingAtCard="300")]
    base_r = dict(fp=1)
    s = {}
    for seed in (1, 2):
        r = dict(base_r, seed=seed, kind="RuledPeople", rec=5000, r4=3000, r5=5000, era=1, era_first=900) if seed == 1 else \
            dict(base_r, seed=seed, kind="OtherPeople", rec=5500, novessel=0)
        s["R_%d" % seed] = (r, [], [], ())
        s["slowR_%d" % seed] = (dict(seed=seed, kind="Clock", rec=CC), [], [], ())
        s["death_%d" % seed] = (dict(seed=seed, kind="Clock", rec=CC, asked=TD, recorded=1, deaths=1, by_end=1, fd=4401, per="4401:-:0.00"), [(4500, "D")], [], ())
        s["wanderer_%d" % seed] = (dict(seed=seed, kind="Clock", rec=CC, wit=1, wit_first=1500, answered=1, crowned=2600, path="return", fp=1),
                                   [(1600, "W")], [claim(PlacedTick=1400)], ())
        s["wanderer_%d_off" % seed] = (dict(seed=seed, kind="Clock", rec=CC, crowned=2500, path="vacuum"), [], [], (5990,))
        s["inputless_%d" % seed] = (dict(seed=seed, kind="Clock" if seed == 1 else "NoCandidate", rec=CC if seed == 1 else 5000, r3=-1), [(7000, "I")], [], ())
        s["inputless_%d_off" % seed] = (dict(seed=seed, kind="Clock" if seed == 1 else "NoCandidate", rec=CC if seed == 1 else 5000, r3=-1), [], [], ())
        s["inputless075_%d" % seed] = (dict(seed=seed, rules=[("AIHostilePowerRatio", "0.75")], taken=4, bands=(1, 1, 2), top=3), [], [], ())
        s["test_%d" % seed] = (dict(seed=seed, rules=[("FallenCoreMinLanesFromHome", "1")], g132=g, at_cap="Hazard.AIAwakening:1/3",
                                    g_ends="Hazard.AIAwakening:2/1/0", u_ends="Hazard.AIAwakening:0/1/0",
                                    pressure=dict(guarded=3, gtags="Hazard.AIAwakening=3", delay=220, dtags="Hazard.AIAwakening=220", band=2, eased=1)),
                               [], test_claims, ())
    return s


def make_c(Fm, folder, t_death=TD, mutate=None):
    jobs = {j["name"]: j for j in C.build_jobs(folder, CC, t_death, max_seed=2)}
    sp = specs()
    if mutate:
        mutate(sp)
    for name, j in jobs.items():
        v, ch, cl, ex = sp[name]
        write_job(Fm, j, v, ch, cl, ex)
    return jobs


def make_b6a(Fm, folder, seeds=(1, 2)):
    for s in seeds:
        # B6a's own wanderer log (B-e per seed, review L1-2): the same own-vessel reading as C's off side.
        write_log(os.path.join(folder, "wanderer_%d.log" % s), Fm, specs()["wanderer_%d_off" % s][0], 6000, cell="wanderer")
        write_csv(os.path.join(folder, "wanderer_%d.csv" % s), Fm, 6000, (), (5990,))
        with open(os.path.join(folder, "wanderer_%d_rows.csv" % s), "w", encoding="utf-8") as f:
            f.write(Fm.rows_header + "\n500,log_x,1\n")


def make_bundle_b(Fm, logs, csvs, mutate=None):
    """Bundle B's outputs for seeds 1-2 (20,000 ticks, no clock): same worlds as C below C, later changes after it."""
    os.makedirs(logs, exist_ok=True)
    os.makedirs(csvs, exist_ok=True)
    sp = specs()
    bs = {}
    for j in B.build_jobs(csvs, TD, max_seed=2):
        name = j["name"]
        if name in sp:
            v, ch, cl, ex = sp[name]
            v = dict(v)
            if v.get("kind") == "Clock":
                v.update(kind="None", rec=-1)
            ch = list(ch) + ([(6500, "late")] if j["cell"] != "inputless" else [])
        else:
            v, ch, cl, ex = dict(seed=j["seed"], kind="None", rec=-1), [(6500, "late")], [], ()
        bs[name] = (v, ch, cl, ex, j)
    if mutate:
        mutate(bs)
    for name, (v, ch, cl, ex, j) in bs.items():
        j = dict(j, log=os.path.join(logs, name + ".log"))
        write_job(Fm, j, v, ch, cl, ex)
    write_manifest(logs, csvs)


def write_manifest(logs, csvs):
    lines = []
    for d in (logs, csvs):
        for n in sorted(os.listdir(d)):
            p = os.path.join(d, n)
            if os.path.isfile(p) and n != "manifest.sha1":
                lines.append("%s  %s" % (A.sha1_files([p])[p], n))
    with open(os.path.join(logs, "manifest.sha1"), "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


def ctx_of(root, **kw):
    ctx = {"c": CC, "t_death": TD, "r": 1.0, "share": 0.5, "chance": 0.02, "w_eased": 1.2, "ratio": 0.79, "build_sha": "abc1234", "b9_ran": False,
           "br_world": "", "k_moved_r": [], "k_moved_w": [], "drop_death": False, "root": os.path.join(root, "proj"), "editor": "UnrealEditor-Cmd.exe",
           "b6a_dir": os.path.join(root, "b6a"), "extinction_countdown": 250}
    ctx.update(kw)
    return ctx


def analyse_dir(folder, ctx, logs=None, csvs=None, cells=None):
    jobs = C.build_jobs(folder, ctx["c"], ctx["t_death"], cells, max_seed=2, drop_death=ctx["drop_death"])
    results = {}
    parsed = C.evaluate(jobs, results, [])
    bstate = C.bundle_b_state(logs, csvs, ctx["t_death"], 2) if logs else None
    bparsed = {n: C.parse_log(j["log"]) for n, j in bstate["jobs"].items()} if bstate and bstate["logs_ok"] is not None else {}
    return jobs, parsed, results, C.analyse(jobs, parsed, results, ctx, bstate, bparsed)

# ------------------------------------------------------------------------------------------------------ git/hook mocks


DATA = {"ProvisionalEndTick": CC, "AIUpperRungProgressMultiplier": 1.0, "FleetDelayCountdownShare": 0.5, "EventChancePerSlot": 0.02, "EventSlotTicks": 10,
        "AIHostilePowerRatio": 0.79}


def fake_git_show(data):
    def show(root, rev, path):
        if path.endswith("DT_Rules.json"):
            return json.dumps([dict({"Name": "rules"}, **data)])
        if path.endswith("DT_Events.json"):
            return json.dumps([{"Name": "event_claim_eased", "BaseWeight": 1.2}])
        if path.endswith("DT_Hazards.json"):
            return json.dumps([{"Name": "crisis_species_extinction", "CountdownTicks": 250}])
        return None
    return show


def base_args(proj, **over):
    a = {"--c": str(CC), "--t-death": str(TD), "--build-sha": "abc1234", "--r": "1", "--chance": "0.02", "--w-eased": "1.2", "--ratio": "0.79",
         "--b9-ran": "no", "--br-world": "none", "--k-moved-r": "none", "--k-moved-w": "none", "--root": proj}
    a.update(over)
    out = []
    for k, v in a.items():
        if v is not None:
            out += [k, v]
    return out

# ----------------------------------------------------------------------------------------------------------- the test


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cpp", default=None)
    a = ap.parse_args()
    cpp_path = a.cpp or next((p for p in (os.path.join(HERE, "cmd_head.cpp"), os.path.join(SCRATCH, "b6b8", "ref", "cmdlet_head.cpp")) if os.path.exists(p)), None)
    if not cpp_path:
        print("no commandlet copy (scratchpad\\b6b11\\cmd_head.cpp) - pass --cpp")
        return 2
    Fm = formats(T.load_cpp(cpp_path))
    root = os.path.join(HERE, "selftest")
    if os.path.exists(root):
        shutil.rmtree(root)
    os.makedirs(root)
    proj = os.path.join(root, "proj")
    os.makedirs(os.path.join(proj, "Binaries", "Win64"))
    for p in (os.path.join(proj, "Cosmic_Ascension.uproject"), os.path.join(proj, "Binaries", "Win64", "UnrealEditor-Cosmic_Ascension.dll"),
              os.path.join(root, "UnrealEditor-Cmd.exe"), os.path.join(root, "frozen_4_3.md")):
        with open(p, "w", encoding="utf-8") as f:
            f.write("x\n")
    editor = os.path.join(root, "UnrealEditor-Cmd.exe")
    C.HOOKS.update(git_show=fake_git_show(DATA), git_head=lambda r: "abc1234 [B-6b10] mock", busy=lambda: [], memory=lambda: (16.0, 32.0),
                   run=lambda cmd: (_ for _ in ()).throw(RuntimeError("no commandlet in the self-test")))

    print("1. job list")
    full = C.build_jobs(r"C:\x\bundleC", CC, TD)
    by_cell = {}
    for j in full:
        by_cell[j["cell"]] = by_cell.get(j["cell"], 0) + 1
    check(len(full) == 620 and by_cell == {"R": 60, "slowR": 60, "death": 60, "wanderer": 120, "inputless": 120, "inputless075": 60, "test": 140}, "620 jobs %s" % by_cell)
    check(sum(1 for j in full if j["ticks"] == CC) == 300 and sum(1 for j in full if j["ticks"] == 20000) == 320, "300 at C, 320 at 20,000")
    check(all(j["ticks"] == (CC if j["cell"] in ("R", "slowR", "death", "wanderer") else 20000) for j in full), "ticks per cell")
    check(len({j["name"] for j in full}) == 620, "unique names")
    check(not any("ProvisionalEndTick" in x for j in full for x in j["args"]), "no ProvisionalEndTick in any argument")
    check(all(sum(1 for x in j["args"] if x.startswith("-Rules=")) <= 1 for j in full), "at most one -Rules= per job")
    idx = {j["name"]: i for i, j in enumerate(full)}
    check(all(idx[j["name"] + "_off"] == idx[j["name"]] + 1 for j in full if j["cell"] in ("wanderer", "inputless") and not j["off"]), "on/off adjacent")
    check(not any(j["off"] for j in full if j["cell"] not in ("wanderer", "inputless")), "off sides only wanderer and inputless")
    get = lambda n: next(j for j in full if j["name"] == n)  # noqa: E731
    R = ["-Answer=1", "-Build=1", "-Focus=1", "-Edict=1", "-Diplomacy=1", "-TellAll=0"]
    check(get("R_1")["args"] == R, "R args")
    check(get("slowR_9")["args"] == ["-Answer=1", "-Build=1", "-Focus=0", "-Edict=1", "-Diplomacy=1", "-TellAll=0"], "slowR = R with -Focus=0")
    check(get("death_3")["args"] == R + ["-DeathAt=4400"], "death = R + -DeathAt")
    check(get("wanderer_4")["args"] == R + ["-Abdicate=1200", "-Wander=1"] and get("wanderer_4_off")["args"] == R + ["-Abdicate=1200", "-Wander=1", "-WithoutB6bData=1"],
          "wanderer on/off args")
    check(get("inputless_5")["args"] == [] and get("inputless_5_off")["args"] == ["-WithoutB6bData=1"], "inputless: no arguments")
    check(get("inputless075_60")["args"] == ["-Rules=AIHostilePowerRatio:0.75"], "inputless075 args")
    check(get("test_140")["args"] == R + ["-Rules=FallenCoreMinLanesFromHome:1", "-LeaveNearSphere=1", "-PlayerPolicy=Faithful", "-Guard=1"], "test args in the b6a order")
    check(max(j["seed"] for j in full if j["cell"] == "test") == 140 and max(j["seed"] for j in full if j["cell"] == "R") == 60, "seeds 1-60, test 1-140")
    dropped = C.build_jobs("x", CC, 6000, drop_death=True)
    check(len(dropped) == 560 and not any(j["cell"] == "death" for j in dropped), "death dropped: 560")
    cmd = A.command_of(get("test_1"), "P.uproject", "E.exe")
    check(cmd[2:6] == ["-run=CosmicBalance", "-Seed=1", "-Ticks=20000", cmd[5]] and cmd[5].startswith("-Out=") and cmd[-1].startswith("-AbsLog=")
          and cmd.index("-Rules=FallenCoreMinLanesFromHome:1") > cmd.index("-TellAll=0"), "command: absolute -Out and -AbsLog")
    bad = [{"name": "x", "args": ["-Rules=ProvisionalEndTick:0"]}, {"name": "y", "args": ["-Rules=A:1", "-Rules=B:2"]}]
    v = C.validate_args(bad)
    check(len(v) == 2 and "ProvisionalEndTick" in v[0] and "2 -Rules=" in v[1], "F8 job part: %s" % v)
    saved = C.CELL_ARGS["R"]
    C.CELL_ARGS["R"] = saved + ["-Rules=ProvisionalEndTick:0"]
    try:
        C.build_jobs("x", CC, TD, cells={"R"}, max_seed=1)
        check(False, "build_jobs refuses a ProvisionalEndTick job")
    except ValueError:
        check(True, "")
    finally:
        C.CELL_ARGS["R"] = saved

    print("2. refusals without placeholders and against the build's data")
    for drop in ("--c", "--t-death", "--build-sha", "--r", "--chance", "--w-eased", "--ratio", "--b9-ran", "--br-world", "--k-moved-r", "--k-moved-w"):
        code, out = quiet(C.main, base_args(proj, **{drop: None}) + ["--dry-run"])
        check(code == 2 and "refusing" in out, "refuses without %s" % drop)
    code, out = quiet(C.main, base_args(proj, **{"--c": "7000"}) + ["--dry-run"])
    check(code == 2 and "ProvisionalEndTick at the build 6000" in out, "--c must equal the build's C")
    C.HOOKS["git_show"] = fake_git_show(dict(DATA, FleetDelayCountdownShare=0.0))
    code, out = quiet(C.main, base_args(proj, **{"--share": "0"}) + ["--dry-run"])
    check(code == 2 and "not 0.5" in out, "share 0 refused")
    C.HOOKS["git_show"] = fake_git_show(dict(DATA, ProvisionalEndTick=5000))
    code, out = quiet(C.main, base_args(proj, **{"--c": "5000"}) + ["--dry-run"])
    check(code == 2 and "outside" in out, "C outside [6,000, 18,000] refused")
    C.HOOKS["git_show"] = lambda r, rev, p: None
    code, out = quiet(C.main, base_args(proj) + ["--dry-run"])
    check(code == 2 and "git show" in out, "no git show: refused")
    C.HOOKS["git_show"] = fake_git_show(dict(DATA, EventSlotTicks=5))
    code, out = quiet(C.main, base_args(proj) + ["--dry-run"])
    check(code == 2 and "EventSlotTicks" in out, "slot ticks 10")
    C.HOOKS["git_show"] = fake_git_show(DATA)

    print("3. dry run")
    code, out = quiet(C.main, base_args(proj) + ["--dry-run", "--a-sec-per-20k", "16", "--editor", editor])
    lines = [l for l in out.splitlines() if l.startswith('"') or "-run=CosmicBalance" in l]
    check(code == 0 and len(lines) == 620 and "ProvisionalEndTick" not in out, "620 commands, no ProvisionalEndTick")
    est = C.wall_minutes(16, CC, 300, 320, 6)
    check(abs(est - 16 * (300 * 6000 + 320 * 20000) / 20000 / 6 / 60) < 1e-9 and ("%.1f min" % est) in out, "wall-time estimate %.1f" % est)
    code, out = quiet(C.main, base_args(proj, **{"--t-death": "6000"}) + ["--dry-run", "--editor", editor])
    check(code == 0 and "# 560 jobs" in out and C.DEATH_DROPPED % (6000, 6000) in out, "T_DEATH >= C: 560 and the dropped text")

    print("4. clean fixture: parser, run failures, controls, bridges")
    folder = os.path.join(root, "clean")
    jobs = make_c(Fm, folder)
    make_b6a(Fm, os.path.join(root, "b6a"))
    logs, csvs = os.path.join(root, "bB_logs"), os.path.join(root, "bB_csv")
    make_bundle_b(Fm, logs, csvs)
    p = C.parse_log(jobs["test_1"]["log"])
    check(p["pressure"] == {"policy": "Faithful", "guard": 1, "leave": 1, "guarded": 3, "guarded_by_tag": {"Hazard.AIAwakening": 3}, "delay": 220,
                            "delay_by_tag": {"Hazard.AIAwakening": 220}, "band": 2, "band_eased": 1}, "pressure parsed %s" % p["pressure"])
    check(p["g132"][1] == {"Hazard.AIAwakening": (1, 50.0), "Hazard.Plague": (1, 40.0)} and p["g132"][2] == {}, "#132 guarded phases")
    check(p["race132"]["at_cap"] == {"Hazard.AIAwakening": (1, 3)} and p["race132"]["guarded_ends"] == {"Hazard.AIAwakening": (2, 1, 0)}, "#132 by the record")
    check(p["rules_lines"] == [("FallenCoreMinLanesFromHome", "1")], "rules line %s" % p["rules_lines"])
    pd = C.parse_log(jobs["death_1"]["log"])
    check(pd["calling"]["by_end"] == 1 and pd["calling"]["first_death"] == 4401 and pd["calling"]["death_ticks"] == [4401] and pd["death"]["at"] == TD, "calling deaths")
    pw = C.parse_log(jobs["wanderer_1"]["log"])
    check(pw["crowned"]["tick"] == 2600 and pw["crowned"]["path"] == "return" and pw["left_at"] == 1201 and pw["race_b"]["side"]["wit_first"] == 1500, "wanderer line")
    pi = C.parse_log(jobs["inputless075_1"]["log"])
    check(pi["conflict_c"] == {"taken": 4, "bands": (1, 1, 2), "top": 3, "lost_max": 1, "delay": {}}, "conflict %s" % pi["conflict_c"])
    po = C.parse_log(jobs["wanderer_1_off"]["log"])
    check(po["off_knobs"] == {"r": "1", "share": "0.5"} and po["ending_x"]["kind"] == "Clock", "off knobs %s" % po["off_knobs"])
    check(all(C.run_failures(j, C.parse_log(j["log"]), 0) == [] for j in jobs.values()), "every clean run passes")
    ctx = ctx_of(root)
    _, parsed, results, res = analyse_dir(folder, ctx, logs, csvs)
    check(res["controls"]["ok"], "clean controls: %s" % res["controls"]["problems"][:4])
    check(res["f7"]["ok"] and [r["P"] for r in res["f7"]["runs"]] == [2, 2], "clean F7 %s" % res["f7"]["runs"])
    check(all(p["ok"] for p in res["pairs"]) and len(res["pairs"]) == 4, "clean B-a %s" % [(p["pair"], p["why"]) for p in res["pairs"]])
    check(all(b["ok"] for b in res["bridge_b"]) and len(res["bridge_b"]) == 4, "clean B-b %s" % [(b["pair"], b["why"]) for b in res["bridge_b"]])
    check(all(b["ok"] for b in res["bridge_c"]) and len(res["bridge_c"]) == 2, "clean B-c %s" % [(b["pair"], b["why"]) for b in res["bridge_c"]])
    check(res["bridge_e"]["ok"] is True and res["bridge_e"]["compared"] == 2 and "totals not compared" in res["bridge_e"]["notes"][0],
          "B-e per seed on a 2-seed subset, totals not compared: %s" % (res["bridge_e"],))
    check(res["bridge_d"]["ok"] is True and res["bridge_d"]["mode"] == "csv" and len(res["bridge_d"]["entries"]) == 14, "clean B-d (csv): %s" % ((
        res["bridge_d"].get("why"), [e for e in res["bridge_d"]["entries"] if e["why"]], len(res["bridge_d"]["entries"])),))
    check(C.verdict(res, {"end_check": 0, "race_check": 0}), "clean verdict PASS")
    wsplit = [j for j in C.build_jobs(folder, CC, TD, max_seed=2) if j["name"] == "wanderer_1"][0]
    w_extra = C.parse_log(wsplit["log"])
    write_log(wsplit["log"], Fm, specs()["wanderer_1"][0], CC, cell="wanderer")
    with open(wsplit["log"], "a", encoding="utf-8") as f:
        f.write(T.ue("CosmicBalance race check: x", "Error") + "\n")
    check(any("race check" in w for w in C.run_failures(wsplit, C.parse_log(wsplit["log"]), 0)), "a race check line fails the run")
    write_log(wsplit["log"], Fm, dict(specs()["wanderer_1"][0], drop_conflict=True), CC, cell="wanderer")
    check(any("unreadable: conflict_c" in w for w in C.run_failures(wsplit, C.parse_log(wsplit["log"]), 0)), "an unreadable conflict line fails the run")
    write_log(wsplit["log"], Fm, dict(specs()["wanderer_1"][0], break_race=True), CC, cell="wanderer")
    check(any("race line parts unreadable" in w for w in C.run_failures(wsplit, C.parse_log(wsplit["log"]), 0)), "an unreadable race part fails the run")
    check(any("exit code 1" in w for w in C.run_failures(wsplit, w_extra, 1)), "a non-zero exit fails the run")
    os.remove(wsplit["claims"])
    check(any("missing" in w for w in C.run_failures(wsplit, w_extra, 0)), "a missing file fails the run")
    make_c(Fm, folder)

    def variant(name, mutate, t_death=TD, **ckw):
        f = os.path.join(root, name)
        make_c(Fm, f, t_death, mutate)
        c = ctx_of(root, t_death=t_death, **ckw)
        return analyse_dir(f, c, logs, csvs)

    def setv(sp, name, **kw):
        v, ch, cl, ex = sp[name]
        sp[name] = (dict(v, **kw), ch, cl, ex)

    print("5. F1 breaks")
    for label, kw in (("Clock at C-1", dict(kind="Clock", rec=CC - 1)), ("record at C+1", dict(kind="RuledPeople", rec=CC + 1)),
                      ("no record", dict(kind="None", rec=-1))):
        _, _, _, r = variant("f1", lambda sp: setv(sp, "R_1", **kw))
        check(r["controls"]["by"]["F1"] and not r["controls"]["ok"] and not C.verdict(r, {}), "F1 breaks on %s: %s" % (label, r["controls"]["by"]["F1"]))
    print("6. F2, F3, F4, F5, F6, F8 breaks")
    f2 = os.path.join(root, "f2")
    make_c(Fm, f2)
    j2 = C.build_jobs(f2, CC, TD, {"R"}, 1)[0]
    write_csv(j2["csv"], Fm, CC - 100)
    _, _, _, r = analyse_dir(f2, ctx_of(root))
    check(any("ends at 5900" in x for x in r["controls"]["by"]["F2"]), "F2: a short CSV")
    _, _, _, r = variant("f3", lambda sp: setv(sp, "wanderer_1_off", was_share=0.0))
    check(any("FleetDelayCountdownShare was 0, not 0.5" in x for x in r["controls"]["by"]["F3"]), "F3 breaks on share 0: %s" % r["controls"]["by"]["F3"])
    _, _, _, r = variant("f3b", lambda sp: setv(sp, "inputless_1_off", on_ai=1))
    check(any("extinctions" in x for x in r["controls"]["by"]["F3"]), "F3: an off run with an extinction")
    _, _, _, r = variant("f4", lambda sp: setv(sp, "R_2", era=1, era_first=500))
    check(any("E >= 600" in x for x in r["controls"]["by"]["F4"]), "F4: era first before 600")
    _, _, _, r = variant("f4b", lambda sp: setv(sp, "wanderer_2", wit_first=1100))
    check(any("before the wanderer left" in x for x in r["controls"]["by"]["F4"]), "F4: witness before the wanderer left")
    _, _, _, r = variant("f5", lambda sp: setv(sp, "death_1", asked=4300))
    check(any("death asked at 4300, not 4400" in x for x in r["controls"]["by"]["F5"]), "F5 breaks on another asked tick")
    _, _, _, r = variant("f5n", lambda sp: setv(sp, "death_2", recorded=0))
    check(r["controls"]["ok"] and any("recorded 0" in n for n in r["controls"]["notes"]), "F5: recorded 0 is a note")
    _, _, _, r = variant("f6", lambda sp: setv(sp, "test_2", pressure=dict(specs()["test_2"][0]["pressure"], policy="Cautious")))
    check(any("Cautious" in x for x in r["controls"]["by"]["F6"]), "F6 breaks on another policy")
    _, _, _, r = variant("f8a", lambda sp: setv(sp, "test_1", rules=[]))
    check(any("test_1: rules lines []" in x for x in r["controls"]["by"]["F8"]), "F8: a missing rules line")
    _, _, _, r = variant("f8b", lambda sp: setv(sp, "R_1", rules=[("AIHostilePowerRatio", "0.75")]))
    check(any("R_1: rules lines" in x for x in r["controls"]["by"]["F8"]), "F8: a rules line in a cell without -Rules")
    _, _, _, r = variant("f8c", lambda sp: setv(sp, "inputless075_1", rules=[("AIHostilePowerRatio", "0.7")]))
    check(any("inputless075_1" in x for x in r["controls"]["by"]["F8"]), "F8: the wrong value")

    print("7. F7 bound")
    # X = 2 (eased, joined; the 300 row is out of the band), J = 1, O = 0: P in [1, 2]; eased X_e = 1: P_e in [1, 1].
    for P, Pe, ok in ((2, 1, True), (1, 1, True), (0, 0, False), (3, 1, False), (2, 0, False)):
        _, _, _, r = variant("f7", lambda sp: setv(sp, "test_1", pressure=dict(specs()["test_1"][0]["pressure"], band=P, eased=Pe)))
        check(r["f7"]["ok"] is ok and (r["readings"]["R2"]["f7_ok"] is ok), "F7 P %d P_e %d -> %s (%s)" % (P, Pe, ok, r["f7"]["broken"]))
        check(r["controls"]["ok"], "F7 does not touch the controls")
    oc = [claim(EndKind="eased"), claim(EndKind="joined", SystemID="sys_2"), claim(EndKind="eased", SystemID="sys_3", OwnerChanged=1, Ruled=0)]
    _, _, _, r = variant("f7o", lambda sp: (setv(sp, "test_1", pressure=dict(specs()["test_1"][0]["pressure"], band=3, eased=2)),
                                            sp.__setitem__("test_1", sp["test_1"][:2] + (oc,) + sp["test_1"][3:])))
    check(r["f7"]["ok"] and r["f7"]["owner_changed_rows"] == 1 and r["readings"]["R2"]["owner_changed"] == 1, "F7: O widens the bound; OwnerChanged reported")

    print("8. bridges that break")
    # B-c: one cell changed at 5,990 in the B6a file breaks; at 6,000 it does not.
    b6 = os.path.join(root, "b6a")
    write_csv(os.path.join(b6, "wanderer_1.csv"), Fm, 6000, [(5990, "x")], (5990,))
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs, csvs)
    check(any(not b["ok"] and "5990" in b["why"][0] for b in r["bridge_c"]) and not C.verdict(r, {}), "B-c breaks at 5,990")
    write_csv(os.path.join(b6, "wanderer_1.csv"), Fm, 6000, [(6000, "x")], (5990,))
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs, csvs)
    check(all(b["ok"] for b in r["bridge_c"]), "B-c: a change at 6,000 does not break")
    make_b6a(Fm, b6)
    # B-a: a wanderer pair differing at 1,400 < T_B 1,500 breaks unless the off log has Hazard.AIAwakening sum > 75.
    def wa(sp, delay):
        v, ch, cl, ex = sp["wanderer_1"]
        sp["wanderer_1"] = (v, [(1400, "W")], cl, ex)
        setv(sp, "wanderer_1_off", delay=delay)
    _, _, _, r = variant("ba", lambda sp: wa(sp, ""))
    pr = [p for p in r["pairs"] if p["pair"] == "wanderer_1"][0]
    check(pr["ok"] is False and "1400 < T_B 1500" in pr["why"][0], "B-a breaks below T_B: %s" % pr["why"])
    _, _, _, r = variant("ba2", lambda sp: wa(sp, "Hazard.AIAwakening=75(1) Hazard.Plague=100(1)"))
    pr = [p for p in r["pairs"] if p["pair"] == "wanderer_1"][0]
    check(pr["ok"] is False, "B-a: sum 75 is not > 75")
    _, _, _, r = variant("ba3", lambda sp: wa(sp, "Hazard.AIAwakening=80(1)"))
    pr = [p for p in r["pairs"] if p["pair"] == "wanderer_1"][0]
    check(pr["ok"] and pr["explained"] and pr["G"] == 1400 and pr["T_B_eff"] == 1400, "B-a: predicted - G recorded, T_B = G: %s" % pr)
    # B-b: wanderer equal to R through 1,200 and death through T_DEATH.
    _, _, _, r = variant("bb", lambda sp: sp.__setitem__("wanderer_2", (sp["wanderer_2"][0], [(1200, "W")], sp["wanderer_2"][2], ())))
    check(any(b["ok"] is False and "1200 <= 1200" in b["why"][-1] for b in r["bridge_b"]), "B-b breaks at the boundary tick 1,200")
    _, _, _, r = variant("bb2", lambda sp: sp.__setitem__("death_2", (sp["death_2"][0], [(4400, "D")], [], ())))
    check(any(b["ok"] is False and "4400 <= 4400" in b["why"][-1] for b in r["bridge_b"]), "B-b breaks at T_DEATH")
    # B-d (csv): a different first T4 below C (bundle B's CSV from 3,000) on an unpredicted seed breaks; predicted: recorded.
    def bd_mut(delay):
        def m(bs):
            v, ch, cl, ex, j = bs["R_1"]
            bs["R_1"] = (dict(v, t4_1=3000, delay=delay), list(ch) + [(3000, "T4")], cl, ex, j)
        return m
    logs2, csvs2 = os.path.join(root, "bB2_logs"), os.path.join(root, "bB2_csv")
    make_bundle_b(Fm, logs2, csvs2, bd_mut(""))
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs2, csvs2)
    bd = r["bridge_d"]
    check(bd["ok"] is False and any(e["name"] == "R_1" and "3000" in e["why"][0] for e in bd["entries"]), "B-d breaks on an unpredicted seed: %s" % bd.get("why"))
    shutil.rmtree(logs2)
    shutil.rmtree(csvs2)
    make_bundle_b(Fm, logs2, csvs2, bd_mut("Hazard.AIAwakening=100(2)"))
    _, _, _, r = analyse_dir(folder, ctx_of(root, k_moved_r=[1]), logs2, csvs2)
    bd = r["bridge_d"]
    check(bd["ok"] is True and bd["moved_count"] == 1 and bd["moved"][0]["G"] == 3000 and bd["may_move"] == {"R": [1]} and not bd["notes"],
          "B-d: a predicted seed is recorded (moved, G 3,000), not broken; K cross-check holds")
    _, _, _, r = analyse_dir(folder, ctx_of(root, k_moved_r=[2], k_moved_w=[1]), logs2, csvs2)
    check(len(r["bridge_d"]["notes"]) == 2, "B-d: K seeds not 'may move' are notes: %s" % r["bridge_d"]["notes"])
    _, _, _, r = analyse_dir(folder, ctx_of(root, b9_ran=True), logs2, csvs2)
    bd = r["bridge_d"]
    check(bd["ok"] is True and all(e["off"] for e in bd["entries"]) and len(bd["entries"]) == 4, "B-d: [B-6b9] ran - on sides skipped, off sides checked")
    _, _, _, r = analyse_dir(folder, ctx_of(root, br_world="[B-6br] moved"), logs2, csvs2)
    check(r["bridge_d"]["ok"] is True and not r["bridge_d"]["entries"] and len(r["bridge_d"]["skipped"]) == 14, "B-d: [B-6br] world - every side skipped")
    shutil.rmtree(logs2)
    shutil.rmtree(csvs2)
    # B-d at or after C: bundle B's change at 6,500 (and a fifth at 7,000 there) leaves B-d alone.
    make_bundle_b(Fm, logs2, csvs2, lambda bs: bs.__setitem__("slowR_1", (dict(bs["slowR_1"][0], kind="RuledPeople", rec=7000, t4_1=6500),) + bs["slowR_1"][1:]))
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs2, csvs2)
    check(r["bridge_d"]["ok"] is True, "B-d: differences at or after C do not break")
    # B-d E check: bundle B's record at 5,500 <= C must be C's record.
    _, _, _, r = analyse_dir(os.path.join(root, "f1"), ctx_of(root), logs, csvs)
    check(r["bridge_d"]["ok"] is False and any("bundle B gives" in w for e in r["bridge_d"]["entries"] for w in e["why"]), "B-d: E_i <= C must be C's record")
    # B-d fallback (no bundle B CSVs): first ticks as min(f, C).
    shutil.rmtree(csvs2)
    shutil.rmtree(logs2)
    make_bundle_b(Fm, logs2, csvs2, bd_mut(""))
    shutil.rmtree(csvs2)
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs2, csvs2)
    bd = r["bridge_d"]
    check(bd["mode"] == "first ticks" and bd["ok"] is False and any("AI T4 3000/6000" in w for e in bd["entries"] for w in e["why"]), "B-d fallback breaks below C: %s" % (
        [e["why"] for e in bd["entries"] if e["why"]]))
    shutil.rmtree(logs2)
    make_bundle_b(Fm, logs2, csvs2, lambda bs: bs.__setitem__("R_1", (dict(bs["R_1"][0], t4_1=7000),) + bs["R_1"][1:]))
    shutil.rmtree(csvs2)
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs2, csvs2)
    check(r["bridge_d"]["mode"] == "first ticks" and r["bridge_d"]["ok"] is True, "B-d fallback: a first T4 after C is min(f, C) = C on both sides")
    with open(os.path.join(logs2, "R_2.log"), "a", encoding="utf-8") as f:
        f.write("tampered\n")
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs2, csvs2)
    check(r["bridge_d"]["ok"] is False and "manifest" in r["bridge_d"]["why"][0], "B-d: logs that differ from the manifest break it")
    _, _, _, r = analyse_dir(folder, ctx_of(root), os.path.join(root, "nowhere"), csvs2)
    check(r["bridge_d"]["ok"] is None, "B-d: no bundle B logs - skipped")

    print("9. B-e and R4 (own vessel)")
    o = C.own_of({"crowned": {"tick": 3000, "path": "return"}, "calling": {"first_death": 2000}, "left_at": 1201})
    check(o["any"] and not o["own"], "a crowning after the first death is not own")
    o = C.own_of({"crowned": {"tick": 1900, "path": "return"}, "calling": {"first_death": 2000}, "left_at": 1201})
    check(o["own"], "a crowning before the first death is own")
    check(not C.own_of({"crowned": {"tick": 6001}, "calling": {"first_death": -1}})["any"], "after 6,000 not counted")
    saved_b6a = C.B6A_B1
    C.B6A_B1 = {"own": 2, "n": 2, "median": 1299, "paths": {"vacuum": 2}}
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs, csvs)
    check(r["bridge_e"]["ok"] is True and r["readings"]["R4"]["read"] and r["readings"]["R4"]["verdict"] == "잡음", "B-e pass (2/2, 1,299, vacuum 2): %s" % r["bridge_e"]["why"])
    C.B6A_B1 = {"own": 2, "n": 2, "median": 1350, "paths": {"vacuum": 2}}
    _, _, _, r = analyse_dir(folder, ctx_of(root), logs, csvs)
    check(r["bridge_e"]["ok"] is False and not r["readings"]["R4"]["read"] and not C.verdict(r, {}), "B-e breaks on another median; R4 not read")
    C.B6A_B1 = saved_b6a
    _, _, _, r = variant("be", lambda sp: setv(sp, "wanderer_1_off", fd=2000))
    check(r["readings"]["R4"]["off"]["own"] == 1 and r["readings"]["R4"]["off"]["any"] == 2, "off own counts drop a crowning after the first death")
    check(r["bridge_e"]["ok"] is False and any(w.startswith("seed 1: own False, B6a True") for w in r["bridge_e"]["why"]) and not r["readings"]["R4"]["read"]
          and r["readings"]["R4"]["text"].startswith("not read - bridge B-c or B-e broken"),
          "B-e per seed: one off seed that no longer matches B6a's own log breaks it (L1-2): %s" % r["bridge_e"]["why"])
    # F-3 (review): 60 off runs reproducing B6a (own 40, 22/15/3, median 1,350); one twice-failed off run -> that seed left out, not broken.
    jobs_e = C.build_jobs(os.path.join(root, "be60"), CC, TD, cells={"wanderer"})
    parsed_e, results_e = {}, {}
    for j in jobs_e:
        s_ = j["seed"]
        own = s_ <= 40
        path = "return" if s_ <= 22 else ("vacuum" if s_ <= 37 else "reverence")
        parsed_e[j["name"]] = {"crowned": {"tick": 1201 + 1350 if own else -1, "path": path if own else "none"}, "left_at": 1201, "calling": {"first_death": -1}}
        results_e[j["name"]] = {"ok": True}
    b6a_e = {j["seed"]: parsed_e[j["name"]] for j in jobs_e if j["off"]}
    be = C.bridge_e(jobs_e, parsed_e, results_e, b6a_parsed=b6a_e)
    check(be["ok"] is True and be["compared"] == 60 and not be["notes"], "B-e: 60 off runs 40/60, 22/15/3, median 1,350 pass: %s" % be["why"])
    b6a_e2 = dict(b6a_e)
    b6a_e2[7] = dict(b6a_e[7], crowned={"tick": 1201 + 1351, "path": "return"})
    be3 = C.bridge_e(jobs_e, parsed_e, results_e, b6a_parsed=b6a_e2)
    check(be3["ok"] is False and be3["why"] == ["seed 7: after leaving 1350, B6a 1351"], "B-e per seed: one seed's crowned - left differs: %s" % be3["why"])
    results_e["wanderer_5_off"] = {"ok": False}
    be2 = C.bridge_e(jobs_e, parsed_e, results_e, b6a_parsed=b6a_e)
    _, withheld_e = C.withheld_cells(jobs_e, results_e)
    check(withheld_e == [] and be2["ok"] is True and be2["skipped"] == [5] and be2["compared"] == 59 and "seeds not compared [5]" in be2["notes"][0],
          "B-e: one twice-failed off run (cell not withheld) -> that seed left out, the other 59 compared: %s %s" % (be2["ok"], be2.get("notes")))
    r4e = C.r4_b1(jobs_e, parsed_e, results_e, True, [5], None)
    check(r4e["read"] and r4e["excluded"] == [5] and "left out of the pairs" in r4e["text"] and "[5]" in r4e["text"], "R4 read without seed 5, listed: %s" % r4e["text"])
    check(C.r4_b1(jobs_e, parsed_e, results_e, False, [5], "skipped")["text"] == "not read - B-c/B-e skipped (seeds [5])", "R4 skipped text apart from broken")
    # L1-2 through analyse: wanderer_2_off fails (its claims file is missing) - one failure, cell not withheld; B-c seed 2 has no
    # result, B-e compares seed 1, R4 is read without seed 2 (before the fix: 'not read - bridge B-c or B-e broken').
    fl = os.path.join(root, "l12")
    make_c(Fm, fl)
    os.remove(os.path.join(fl, "wanderer_2_off_claims.csv"))
    _, _, rl, r = analyse_dir(fl, ctx_of(root), logs, csvs)
    r4l = r["readings"]["R4"]
    check(not rl["wanderer_2_off"]["ok"] and r["withheld"] == [] and r["bridge_e"]["ok"] is True and r["bridge_e"]["skipped"] == [2] and r4l["read"]
          and r4l["excluded"] == [2] and C.bridges_ok(r), "one failed off run: R4 read on seed 1, seed 2 listed: %s / %s" % (r4l.get("text"), r["bridge_e"]))

    print("10. frozen arithmetic")
    # #142 (decision 24): p_e = 1 - 0.976^47.5 = 0.6846; n = 30 region [0.5183, 0.8509]; n = 31 [0.5210, 0.8482].
    acc = C.accept_142(16, 30, 0.02, 1.2)
    check(abs(acc["p_e"] - 0.6846) < 5e-5 and abs(acc["lo"] - 0.5183) < 5e-4 and abs(acc["hi"] - 0.8509) < 5e-4, "p_e and n=30 region %s" % acc)
    check(abs((1 - acc["hi"]) - 0.15) < 0.002 and abs((1 - acc["lo"]) - 0.48) < 0.002, "joined side ~0.15-0.48 (N f)")
    for k, n, inside in ((16, 30, True), (15, 30, False), (25, 30, True), (26, 30, False), (16, 31, False), (17, 31, True), (26, 31, True), (27, 31, False)):
        check(C.accept_142(k, n, 0.02, 1.2)["inside"] is inside, "#142 %d/%d inside %s" % (k, n, inside))
    tj = {"name": "test_1", "cell": "test", "seed": 1, "off": False, "ticks": 20000, "claims": os.path.join(root, "r2_claims.csv")}
    for n, k_e, expect in ((30, 16, "모형 구간 안 — 16/30"), (30, 15, "[개발자]"), (31, 17, "모형 구간 안 — 17/31"), (31, 16, "[개발자]"), (29, 16, "재지 않음 — 잰 29")):
        rows = [claim(EndKind="eased", SystemID="s%d" % i) for i in range(k_e)] + [claim(EndKind="joined", SystemID="j%d" % i) for i in range(n - k_e)]
        rows += [claim(AfterBinding=1), claim(PlacedTick=CC), claim(Ruled=0), claim(OwnerChanged=1), claim(Relief=1), claim(EndKind="relief"), claim(RemainingAtCard=551)]
        write_claims(tj["claims"], Fm, rows)
        r2 = C.r2_142([tj], {"test_1": {}}, {"test_1": {"ok": True}}, ctx_of(root), {"ok": True})
        check(r2["n"] == n and r2["k_e"] == k_e and r2["text"].startswith(expect) and r2["after_binding"] == 1 and r2["placed_after_c"] == 1
              and r2["owner_changed"] == 1, "#142 n %d k_e %d -> %s (%s)" % (n, k_e, expect, r2["text"]))
    check(abs(r2["sum_p"] - 29 * (1 - 0.976 ** 45)) < 1e-9, "sum p_i: 29 claims x p(floor(450/10) = 45 slots)")
    check(abs(r2["model_range"][0] - (1 - 0.976 ** 40)) < 1e-12 and abs(r2["model_range"][1] - (1 - 0.976 ** 55)) < 1e-12, "B4 form p(401), p(550)")
    r2 = C.r2_142([tj], {"test_1": {}}, {"test_1": {"ok": True}}, ctx_of(root), {"ok": False, "broken": [1]})
    check(r2["judged"] is False and "F7 broken" in r2["text"], "F7 broken stops #142")
    # sign test (exact, two-sided): L 9 / G 2 -> 0.0654 noise; L 10 / G 1 -> 0.0117 'B6b가 줄였다'.
    check(abs(B.sign_test(9, 2) - 134 / 2048) < 1e-12 and round(B.sign_test(9, 2), 4) == 0.0654, "sign test 9/2 = 0.0654")
    check(round(B.sign_test(10, 1), 4) == 0.0117, "sign test 10/1 = 0.0117")

    def r4_case(L, G, paths=None):
        jobs4, parsed4, results4 = [], {}, {}
        for s in range(1, 12):
            on_own = s > L  # seeds 1..L: off own only; seeds L+1..L+G: on own only
            off_own = s <= L
            if L < s <= L + G:
                on_own, off_own = True, False
            for off, own in ((False, on_own), (True, off_own)):
                name = "wanderer_%d%s" % (s, "_off" if off else "")
                jobs4.append({"name": name, "cell": "wanderer", "seed": s, "off": off})
                pr = {"crowned": {"tick": 2500 if own else -1, "path": "return"}, "calling": {"first_death": -1}, "left_at": 1201,
                      "race": {"crown": {"refused": 0, "first": -1}, "extinction": {"erased_once_ruled": 0}},
                      "race_b": {"side": {"era": 0, "era_first": -1, "answered": 0, "wit_first": -1}}}
                if paths and not off and s in paths:
                    paths[s](pr)
                parsed4[name] = pr
                results4[name] = {"ok": True}
        return C.r4_b1(jobs4, parsed4, results4, True)
    r4 = r4_case(9, 2)
    check(r4["verdict"] == "잡음" and r4["text"].startswith("잡음 — L 9 / G 2 (p 0.0654)"), "R4: L 9 G 2 noise: %s" % r4["text"])
    paths = {1: lambda p: p["race"]["crown"].update(refused=1, first=3000), 2: lambda p: p["race"]["extinction"].update(erased_once_ruled=1),
             3: lambda p: p["race_b"]["side"].update(answered=1, wit_first=7000), 4: lambda p: p["race_b"]["side"].update(era=1, era_first=900),
             5: lambda p: p["race_b"]["side"].update(answered=2, wit_first=2000)}
    r4 = r4_case(10, 1, paths)
    check(r4["verdict"] == "B6b가 줄였다" and abs(r4["p"] - 0.0117) < 5e-5, "R4: L 10 G 1 -> B6b가 줄였다 (%s)" % r4["text"])
    check(r4["upper_bound_only"] == [2, 5] and r4["unexplained"] == [3, 6, 7, 8, 9, 10], "R4 paths: (b)/(c) apart %s, unexplained %s" % (r4["upper_bound_only"], r4["unexplained"]))
    check(r4["paths"][3]["c witness answered"] is False and r4["paths"][1]["a FarAbove"], "witness first at 7,000 > 6,000 is not path (c)")
    check(r4["gate"]["ok"] and r4["gate"]["text"].endswith("1299 <= 1,800"), "R4 gate: median after leaving 1,299")
    r4 = r4_case(2, 9)
    check(r4["verdict"] == "잡음" and r4["text"].startswith("잡음 — L 2 / G 9 (p 0.0654)"), "R4: L 2 G 9 (p >= 0.05) noise: %s" % r4["text"])
    # F-4 (review): L 0, G 12 -> p 0.000488 is outside the frozen rule, not '잡음'.
    jobs_g = C.build_jobs(os.path.join(root, "r4g"), CC, TD, cells={"wanderer"})
    parsed_g = {j["name"]: {"crowned": {"tick": 2000 if (not j["off"] or j["seed"] > 12) else -1, "path": "return"}, "left_at": 1201,
                            "calling": {"first_death": -1}} for j in jobs_g}
    r4 = C.r4_b1(jobs_g, parsed_g, {j["name"]: {"ok": True} for j in jobs_g}, True)
    check(r4["verdict"] is None and len(r4["L"]) == 0 and len(r4["G"]) == 12 and abs(r4["p"] - 2 * 0.5 ** 12) < 1e-12
          and r4["text"] == "규칙 밖 — L 0 < G 12, p 0.0005 (얼린 4.3은 G > L에 판정을 두지 않는다) [개발자]", "R4: L 0 G 12 -> 규칙 밖: %s" % r4["text"])
    # R1 (#136): k >= 20 of 60 closes.
    def r1_case(k):
        jobs1 = [{"name": "R_%d" % s, "cell": "R", "seed": s, "off": False} for s in range(1, 61)] + \
                [{"name": "slowR_%d" % s, "cell": "slowR", "seed": s, "off": False} for s in range(1, 61)]
        parsed1 = {j["name"]: {"first_person": 1 if j["seed"] <= k else 0} for j in jobs1}
        return C.r1_136(jobs1, parsed1, {j["name"]: {"ok": True} for j in jobs1})
    check(r1_case(20)["R"]["text"].startswith("닫는다 — 20/60") and r1_case(19)["R"]["text"].startswith("열어 둔다 — 19/60"), "R1 20 closes, 19 stays open")
    jobs1 = [{"name": "R_%d" % s, "cell": "R", "seed": s, "off": False} for s in range(1, 30)]
    check(C.r1_136(jobs1, {}, {})["R"]["text"] == "재지 않음 — 잰 29", "R1 below 30: 재지 않음 — 잰 N")
    # #10 per death: run 1 two deaths both succeeded by the record; run 2 the second succession after the record; run 3 none.
    mkd = lambda by_end, succ, rec: {"calling": {"by_end": by_end, "successions": [(t, 6) for t in succ]}, "ending_x": {"tick": rec, "kind": "Clock"}}  # noqa: E731
    check(C.deaths_of_run(mkd(2, (3000, 5000), CC)) == (2, 0, 2) and C.deaths_of_run(mkd(2, (3000, 6500), CC)) == (2, 1, 1)
          and C.deaths_of_run(mkd(1, (), CC)) == (1, 1, 0) and C.deaths_of_run(mkd(1, (6000,), CC)) == (1, 0, 1), "#10 per death k, succession at the record tick counts")
    jobs9 = [{"name": "%s_%d" % (c, s), "cell": c, "seed": s, "off": False} for c in ("wanderer", "inputless", "death") for s in (1, 2, 3)]
    parsed9 = {"wanderer_1": mkd(2, (3000, 5000), CC), "wanderer_2": mkd(2, (3000, 6500), CC), "wanderer_3": mkd(1, (), CC),
               "inputless_1": mkd(3, (100, 200, 300), CC), "inputless_2": mkd(0, (), CC), "inputless_3": mkd(1, (), 5000),
               "death_1": mkd(1, (), CC), "death_2": mkd(1, (4900,), CC), "death_3": mkd(0, (), CC)}
    r9 = C.r9_10(jobs9, parsed9, {n: {"ok": True} for n in parsed9}, ctx_of(root))
    check(r9["pool"]["n"] == 9 and r9["pool"]["k"] == 3 and r9["pool"]["text"].startswith("재지 않음 — 잰 9"), "#10 pool 3/9: %s" % r9["pool"])
    check(r9["wanderer"]["runs_in_interregnum"] == 2 and r9["wanderer"]["of_them_clock"] == 2 and r9["death"]["k"] == 1 and r9["death"]["n"] == 2, "#10 cells")
    parsed9b = {n: mkd(3, (100,), CC) for n in parsed9}
    r9 = C.r9_10(jobs9, parsed9b, {n: {"ok": True} for n in parsed9}, ctx_of(root))
    check(r9["pool"]["n"] == 18 and r9["pool"]["k"] == 12, "#10 pool 12/18")
    jobs9w = [{"name": "%s_%d" % (c, s), "cell": c, "seed": s, "off": False} for c in ("wanderer", "inputless") for s in range(1, 16)]
    r9 = C.r9_10(jobs9w, {j["name"]: mkd(1, (5000,), CC) if j["seed"] % 3 else mkd(1, (), CC) for j in jobs9w}, {j["name"]: {"ok": True} for j in jobs9w},
                 ctx_of(root))
    check(r9["pool"]["n"] == 30 and r9["pool"]["k"] == 10 and r9["pool"]["text"].startswith("no succession by the record 10/30"), "#10 pool 10/30 judged")
    r9 = C.r9_10(jobs9, parsed9, {n: {"ok": True} for n in parsed9}, ctx_of(root, t_death=6000, drop_death=True))
    check(r9["death"]["text"] == "재지 않음 — 강제 죽음(6000)이 시계(6000) 뒤", "#10 death dropped text")
    # #158 (3) dedup: R all keys; death only tick > T_DEATH; wanderer only tick > 1,200; slowR and test their own worlds; phase 2 dropped.
    keys = lambda *ks: {"keys": list(ks)}  # noqa: E731
    jobs10 = [{"name": n, "cell": n.split("_")[0], "seed": 1, "off": False} for n in ("R_1", "death_1", "wanderer_1", "slowR_1", "test_1")]
    parsed10 = {"R_1": keys((1200, "sys_1", 0, "k"), (4400, "sys_2", 1, "w"), (6100, "sys_9", 2, "k")),
                "death_1": keys((1200, "sys_1", 0, "k"), (4400, "sys_2", 1, "w"), (4401, "sys_3", 1, "s")),
                "wanderer_1": keys((1200, "sys_1", 0, "k"), (1201, "sys_4", 0, "k"), (4400, "sys_2", 1, "k")),
                "slowR_1": keys((1200, "sys_1", 0, "k")), "test_1": keys((1200, "sys_1", 0, "w"))}
    r10 = C.r10_158(jobs10, parsed10, {n: {"ok": True} for n in parsed10}, ctx_of(root))
    check(r10["n"] == 6 and r10["lost"] == 3 and r10["merged"] == 1 and r10["text"].startswith("재지 않음 — 잰 6"), "#158 dedup n 6 lost 3 merged 1: %s" % r10)
    jobs10b = [{"name": "R_%d" % s, "cell": "R", "seed": s, "off": False} for s in range(1, 31)]
    r10 = C.r10_158(jobs10b, {j["name"]: keys((100, "a", 0, "w" if j["seed"] <= 3 else "k")) for j in jobs10b}, {j["name"]: {"ok": True} for j in jobs10b}, ctx_of(root))
    check(r10["verdict"] == "developer" and r10["text"].startswith("[개발자] header first"), "#158 3/30 = 10% -> [개발자]")
    r10 = C.r10_158(jobs10b, {j["name"]: keys((100, "a", 0, "w" if j["seed"] <= 2 else "k")) for j in jobs10b}, {j["name"]: {"ok": True} for j in jobs10b}, ctx_of(root))
    check(r10["verdict"] == "closed", "#158 2/30 < 10% -> closed")
    # #154: late = 4,000 <= tick <= the record; D in 5..8.
    jobs6 = [{"name": "wanderer_1", "cell": "wanderer", "seed": 1, "off": False}, {"name": "inputless_1", "cell": "inputless", "seed": 1, "off": False}]
    parsed6 = {"wanderer_1": {"calling": {"successions": [(3999, 6), (4000, 5), (4500, 2), (6100, 8)]}, "ending_x": {"tick": 6000}},
               "inputless_1": {"calling": {"successions": [(4000, 8), (6000, 9)]}, "ending_x": {"tick": 6000}}}
    r6 = C.r6_154(jobs6, parsed6, {n: {"ok": True} for n in parsed6})
    check(r6["wanderer"]["late"]["k"] == 1 and r6["wanderer"]["late"]["n"] == 2 and r6["inputless"]["late"]["n"] == 2 and r6["pooled"]["late"]["k"] == 2
          and r6["pooled"]["early"]["n"] == 1, "#154 late/early split")
    # L2-3: an early succession after an early record (3,500) is not counted early (B5's split is over pre-clock successions).
    jobs6b = jobs6 + [{"name": "wanderer_2", "cell": "wanderer", "seed": 2, "off": False}]
    parsed6b = dict(parsed6, wanderer_2={"calling": {"successions": [(3000, 6), (3800, 6)]}, "ending_x": {"tick": 3500}})
    r6b = C.r6_154(jobs6b, parsed6b, {n: {"ok": True} for n in parsed6b})
    check(r6b["pooled"]["early"]["n"] == 2 and r6b["pooled"]["early"]["k"] == 2 and r6b["wanderer"]["early"]["n"] == 2,
          "#154 early only up to the record: %s" % r6b["pooled"]["early"])
    check("walking cell" in r6b["text"] and "7/29" in r6b["text"], "#154 text names B5's walking cell (L2-2)")
    # #132: whole run weighted by count, by the record phases 0-1.
    jobsC, parsedC, resultsC, resC = analyse_dir(folder, ctx_of(root))
    ph10 = C.placeholders(resC, {}, ctx_of(root))["<#10>"]
    check("28/37" in ph10 and "44/59" in ph10 and "walking cell" in ph10, "<#10> carries B5's like-for-like 28/37 and the 44/59 with the walking cell (L2-2): %s" % ph10)
    r3 = resC["readings"]["R3"]
    check(r3["n"] == 6 and abs(r3["share_pct"] - (2 * 48 + 50) / 3.0) < 1e-9 and r3["text"] == "재지 않음 — 잰 6" and r3["cap"]["k"] == 2 and r3["cap"]["n"] == 6
          and r3["ends_guarded"] == [4, 2, 0] and r3["pressure_delay"] == 440, "#132 weighted share 48.67%%, cap 2/6: %s" % r3)
    r5 = resC["readings"]["R5"]
    check(r5["0.75 on"]["pooled_top"] == 0.75 and r5["0.75 on"]["runs_top_above_half"] == 2 and r5["0.75 on"]["late_sum"] == 4 and r5["0.79 on"]["taken_sum"] == 6,
          "#129 top share and late band")
    r7 = resC["readings"]["R7"]
    check(r7["wanderer"]["n"] == 2 and r7["death"]["n"] == 2, "< 15% per cell n")
    r8 = resC["readings"]["R8"]
    check(r8["R"]["RuledPeople"].startswith("1/2") and r8["R"]["OtherPeople"].startswith("1/2") and r8["slowR"]["Clock"].startswith("2/2"), "record kinds %s" % r8)
    r11 = resC["readings"]["R11"]
    check(r11["R"]["min"] == 5000 and r11["R"]["median"] == 5250 and r11["R"]["p90"] == 5500 and r11["slowR"]["clock"]["k"] == 2, "run length %s" % r11["R"])
    _, _, _, rd = variant("lt", lambda sp: setv(sp, "inputless_1", kind="OtherPeople", rec=5000, novessel=1))
    check(rd["readings"]["R7"]["inputless"]["k"] == 1, "< 15%: OtherPeople with no vessel 1")
    _, _, _, rd = variant("dd", lambda sp: None, t_death=6000, drop_death=True)
    check(rd["readings"]["R7"]["death"]["text"] == "재지 않음 — 강제 죽음(6000)이 시계(6000) 뒤", "< 15% death dropped")

    fw = os.path.join(root, "withheld")
    make_c(Fm, fw)
    for n in ("test_1", "test_2"):
        os.remove(os.path.join(fw, n + "_claims.csv"))
    _, _, _, rw = analyse_dir(fw, ctx_of(root))
    ph = C.placeholders(rw, {}, ctx_of(root))
    check(rw["withheld"] == ["test"] and ph["<N142>"].startswith("WITHHELD") and ph["<A132>"].startswith("WITHHELD") and not ph["<J136>"].startswith("WITHHELD"),
          "two twice-failed runs withhold the cell's readings")

    print("11. hand-check selection")
    _, _, _, rh = variant("hc", lambda sp: setv(sp, "R_2", on_ruled=1, on_ruled_first=4000, ruled_expiries=1, died=1))
    hc = rh["handcheck"]
    check(hc["1"]["seed"] == 1 and hc["1"]["save_at"] == 2950 and hc["1"]["cell"] == "R", "(1) R seed 1 at T4 3,000 - 50")
    check(hc["2"]["seed"] == 2 and hc["2"]["save_at"] == 5470, "(2) OtherPeople seed 2 at 5,500 - 30")
    check(hc["3"]["seed"] == 2 and hc["3"]["save_at"] == 3990 and "4250 + guard delay" in hc["3"]["expected"], "(3) extinction at 4,000 - 10, expiry 4,250")
    check("record OtherPeople at 5500" in hc["3"]["expected"] and "before the countdown" not in hc["3"]["expected"], "(3) expected names the record: %s" % hc["3"]["expected"])
    check(hc["6i"]["seed"] == 1 and hc["6i"]["save_at"] == 870, "(6 i) era at 900 - 30")
    check("record RuledPeople at 5000" in hc["6i"]["expected"] and "record Clock at 6000" in hc["6ii"]["expected"], "(6) expected texts name the record (HC-2)")
    # HC-2: R_1's countdown (5,900 + 250) ends after its record (5,950); R_2's (4,000 + 250) ends before its Clock at 6,000 -> R_2.
    _, _, _, rh4 = variant("hc4", lambda sp: [setv(sp, "R_1", kind="OtherPeople", rec=5950, on_ruled=1, on_ruled_first=5900),
                                              setv(sp, "R_2", kind="Clock", rec=CC, on_ruled=1, on_ruled_first=4000)])
    h3 = rh4["handcheck"]["3"]
    check(h3["seed"] == 2 and "record Clock at 6000" in h3["expected"] and h3["save_at"] == 3990, "(3) prefers the seed whose countdown ends by its record: %s" % h3)
    _, _, _, rh5 = variant("hc5", lambda sp: [setv(sp, "R_1", kind="OtherPeople", rec=5950, on_ruled=1, on_ruled_first=5900),
                                              setv(sp, "R_2", kind="Clock", rec=CC, on_ruled=1, on_ruled_first=5800)])
    h3 = rh5["handcheck"]["3"]
    check(h3["seed"] == 1 and "the end comes before the countdown" in h3["expected"] and "record OtherPeople at 5950" in h3["expected"],
          "(3) no seed fits: the literal pick, said so: %s" % h3.get("expected"))
    check(hc["6ii"]["cell"] == "wanderer" and hc["6ii"]["save_at"] == 1470 and "-Abdicate=1200 -Wander=1" in hc["6ii"]["cmd"], "(6 ii) witness from a wanderer seed")
    for k in ("1", "2", "3", "6i", "6ii"):
        cmdk = hc[k]["cmd"]
        check("-SaveAt=" in cmdk and "-TellAll=0" in cmdk and "-Rules" not in cmdk and "-DeathAt" not in cmdk and "-WithoutB6bData" not in cmdk
              and "-Ticks=6000" in cmdk and "b6b_handcheck" in cmdk, "hand-check command %s in game shape" % k)
    _, _, _, rh = variant("hc2", lambda sp: [setv(sp, "R_1", kind="Clock", rec=CC), setv(sp, "R_2", kind="Clock", rec=CC, era=0),
                                             setv(sp, "wanderer_1", wit=0, wit_first=-1, answered=0), setv(sp, "wanderer_2", wit=0, wit_first=-1, answered=0)])
    check(rh["handcheck"]["1"]["text"].startswith("없음 - [개발자]") and rh["handcheck"]["2"]["text"] == "없음" and rh["handcheck"]["6ii"]["text"].startswith("없음"),
          "(1)/(2)/(6) 없음 when none qualifies")
    _, _, _, rh = variant("hc3", lambda sp: [setv(sp, "R_1", kind="Clock", rec=CC), setv(sp, "slowR_2", kind="RuledPeople", rec=5800, r4=4000, r5=5800)])
    check(rh["handcheck"]["1"]["cell"] == "slowR" and "-Focus=0" in rh["handcheck"]["1"]["cmd"], "(1) falls back to slowR")
    # HC-1: the -SaveAt lines. A failed save ('could not write', then the end-of-run 'wrote no slot' error) gives no path.
    hcl = os.path.join(root, "hc_logs")
    bad_log = os.path.join(hcl, "bad.log")
    write_log(bad_log, Fm, dict(specs()["R_1"][0], extra=[T.ue("CosmicBalance: -SaveAt 2797 could not write C:/a b/Saved/SaveGames/Galaxy_X.sav"),
                                                          T.ue("CosmicBalance: -SaveAt 2797 wrote no slot", "Error")]), CC)
    pb = C.parse_log(bad_log)
    check(pb["saveat"] == [] and pb["saveat_failed"], "a failed -SaveAt gives no save path: %s %s" % (pb["saveat"], pb["saveat_failed"]))
    proj2 = os.path.join(root, "proj_hc")
    games = os.path.join(proj2, "Saved", "SaveGames")
    os.makedirs(games)
    sav = os.path.join(games, "Galaxy_12B1.sav")
    with open(sav, "w", encoding="utf-8") as f:
        f.write("save\n")
    good_log = os.path.join(hcl, "good.log")
    write_log(good_log, Fm, dict(specs()["R_1"][0], extra=[T.ue("CosmicBalance: -SaveAt 2797 wrote " + A.fwd(sav))]), CC)
    pg = C.parse_log(good_log)
    check(pg["saveat"] == [(2797, A.fwd(sav))] and not pg["saveat_failed"], "a good -SaveAt line (path with spaces) parses: %s" % pg["saveat"])
    write_log(os.path.join(hcl, "spaces.log"), Fm, dict(specs()["R_1"][0], extra=[T.ue("CosmicBalance: -SaveAt 5 wrote C:/a b/Saved/SaveGames/Galaxy_X.sav")]), CC)
    check(C.parse_log(os.path.join(hcl, "spaces.log"))["saveat"] == [(5, "C:/a b/Saved/SaveGames/Galaxy_X.sav")], "'wrote C:/a b/.../Galaxy_X.sav' keeps the path")
    dest_dir = os.path.join(proj2, "Saved", "HandCheck", "B6b_3_extinction_seed1_2797")
    moved, why = C.take_save(bad_log, proj2, dest_dir)
    check(moved is None and "failed" in why and os.path.exists(sav), "take_save refuses a failed save: %s" % why)
    os.makedirs(dest_dir)
    with open(os.path.join(dest_dir, "other.sav"), "w", encoding="utf-8") as f:
        f.write("x\n")
    moved, why = C.take_save(good_log, proj2, dest_dir)
    check(moved is None and "not empty" in why and os.path.exists(sav), "take_save refuses a non-empty folder: %s" % why)
    moved, why = C.take_save(good_log, os.path.join(root, "proj"), dest_dir + "_2")
    check(moved is None and "not under" in why and os.path.exists(sav), "take_save refuses a path outside <root>/Saved/SaveGames: %s" % why)
    os.remove(os.path.join(dest_dir, "other.sav"))
    moved, why = C.take_save(good_log, proj2, dest_dir)
    check(moved == os.path.join(dest_dir, "Galaxy_12B1.sav") and os.path.exists(moved) and not os.path.exists(sav), "take_save moves only that file: %s" % why)

    print("12. driver: mocked commandlet, busy check, rerun, archive")
    tmpl = folder
    calls = {"n": 0, "fail_once": {"R_2"}, "busy_after": None, "check_once": set()}

    def fake_run(cmdl):
        calls["n"] += 1
        log = next(x for x in cmdl if x.startswith("-AbsLog="))[len("-AbsLog="):]
        stem = os.path.basename(log)[:-4]
        out_dir = os.path.dirname(log)
        if stem in calls["fail_once"]:
            calls["fail_once"].discard(stem)
            return 3
        for suffix in (".log", ".csv", "_rows.csv", "_claims.csv"):
            shutil.copy2(os.path.join(tmpl, stem + suffix), os.path.join(out_dir, stem + suffix))
        if stem in calls["check_once"]:
            # L1-1: a first attempt that prints an end check line and exits 0; the rerun is clean.
            calls["check_once"].discard(stem)
            with open(log, "a", encoding="utf-8") as f:
                f.write(T.ue("CosmicBalance end check: x", "Error") + "\n")
        return 0

    busy_calls = {"n": 0}

    def fake_busy():
        busy_calls["n"] += 1
        return ["UnrealEditor.exe (pid 1)"] if calls["busy_after"] is not None and busy_calls["n"] > calls["busy_after"] else []

    C.HOOKS.update(run=fake_run, busy=fake_busy)
    run_dir, arch = os.path.join(root, "run"), os.path.join(root, "archive")
    argv = base_args(proj) + ["--folder", run_dir, "--cells", "R,slowR", "--max-seed", "2", "--editor", editor, "--frozen", os.path.join(root, "frozen_4_3.md"),
                              "--archive-dir", arch, "--bundle-b-logs", logs, "--bundle-b-csv", csvs, "--b6a-dir", b6, "--workers", "2"]
    code, out = quiet(C.main, argv)
    runs = json.load(open(os.path.join(run_dir, "runs.json"), encoding="utf-8"))
    check(code == 0 and "bundle C: PASS" in out, "mocked run passes (%s)" % out[-400:])
    check(len(runs["R_2"]["attempts"]) == 2 and runs["R_2"]["ok"] and calls["n"] == 5, "one rerun of a failed process (%d launches)" % calls["n"])
    check(busy_calls["n"] == 1 + 5, "the busy check before every launch (and once at start): %d" % busy_calls["n"])
    check(os.path.exists(os.path.join(arch, "manifest.sha1")) and os.path.exists(os.path.join(arch, "R_1.log")) and not os.path.exists(os.path.join(arch, "R_1.csv")),
          "archive: logs and a sha1 manifest")
    check("freeze sha1" in out and C.freeze_hashes(os.path.join(root, "frozen_4_3.md"))["frozen_4_3.md"] in out, "freeze hashes printed before job 1")
    meta = json.load(open(os.path.join(run_dir, "bundle_meta.json"), encoding="utf-8"))
    check(meta["sha1_ok"] and meta["frozen"] and meta["aborted"] is None and meta["workers"] == 2, "bundle_meta: sha1 unchanged, freeze stored, workers 2")
    # L1-1: R_2's first attempt prints an end check line (exit 0), its rerun is clean -> the header still counts it, FAIL, and
    # the first attempt's log is kept and archived.
    calls.update(n=0, fail_once=set(), check_once={"R_2"})
    run_chk, arch_chk = os.path.join(root, "run_chk"), os.path.join(root, "archive_chk")
    i = argv.index("--folder")
    argv_chk = argv[:i + 1] + [run_chk] + argv[i + 2:]
    i = argv_chk.index("--archive-dir")
    argv_chk = argv_chk[:i + 1] + [arch_chk] + argv_chk[i + 2:]
    code, out = quiet(C.main, argv_chk)
    runs_chk = json.load(open(os.path.join(run_chk, "runs.json"), encoding="utf-8"))
    summ = json.load(open(os.path.join(run_chk, "summary.json"), encoding="utf-8"))
    check(code == 1 and "bundle C: FAIL" in out and "- end check 1, race check 0" in out and summ["header"]["end_check"] == 1
          and runs_chk["R_2"]["attempts"][0]["end_check"] == 1 and len(runs_chk["R_2"]["attempts"]) == 2 and runs_chk["R_2"]["ok"],
          "an end check line in a rerun job's first attempt still counts (L1-1): code %s, %s" % (code, [l for l in out.splitlines() if "end check" in l][:3]))
    check(os.path.exists(os.path.join(run_chk, "R_2.try1.log")) and os.path.exists(os.path.join(arch_chk, "R_2.try1.log")) and "rerun R_2: attempt 1" in out
          and "R_2.try1.log" in out, "the first attempt's log is kept, archived and its reason listed")
    code, out = quiet(C.main, base_args(proj) + ["--check-only", run_chk, "--cells", "R,slowR", "--max-seed", "2", "--bundle-b-logs", logs, "--bundle-b-csv", csvs,
                                                 "--b6a-dir", b6, "--no-archive"])
    check(code == 1 and "- end check 1, race check 0" in out, "check-only keeps the first attempt's end check from runs.json")
    # L1-3: 2.4 GB free of 31.1 GB (bundle B's start) at 6 workers -> 3 workers (0.8 GB each); 0.5 GB free -> refused.
    C.HOOKS["memory"] = lambda: (2.4, 31.1)
    calls.update(n=0, check_once=set())
    run_mem = os.path.join(root, "run_mem")
    i = argv.index("--folder")
    argv_mem = argv[:i + 1] + [run_mem] + argv[i + 2:]
    i = argv_mem.index("--workers")
    argv_mem = argv_mem[:i] + ["--workers", "6", "--no-archive"]
    code, out = quiet(C.main, argv_mem)
    meta_mem = json.load(open(os.path.join(run_mem, "bundle_meta.json"), encoding="utf-8"))
    check(code == 0 and "workers 6 -> 3" in out and "4 jobs, 3 workers" in out and meta_mem["workers"] == 3, "low free memory lowers the workers (L1-3): %s" % (
        [l for l in out.splitlines() if "memory" in l or "workers" in l][:3],))
    C.HOOKS["memory"] = lambda: (0.5, 31.1)
    code, out = quiet(C.main, argv_mem)
    check(code == 2 and "free memory 0.5 GB does not cover one commandlet" in out, "free memory below one commandlet: refused")
    C.HOOKS["memory"] = lambda: (16.0, 32.0)
    calls.update(n=0, fail_once=set(), busy_after=2)
    busy_calls["n"] = 0
    code, out = quiet(C.main, argv[:argv.index("--workers")] + ["--workers", "1", "--no-archive"])
    check(code == 1 and "ABORTED" in out and calls["n"] == 1, "a busy machine aborts the bundle early (%d launched)" % calls["n"])
    # HC-3: the same abort without --no-archive and with a fresh archive folder - nothing reaches the archive.
    calls.update(n=0, fail_once=set(), busy_after=2)
    busy_calls["n"] = 0
    arch_ab = os.path.join(root, "archive_abort")
    i = argv.index("--folder")
    argv_ab = argv[:i + 1] + [os.path.join(root, "run_abort")] + argv[i + 2:]
    i = argv_ab.index("--archive-dir")
    argv_ab = argv_ab[:i + 1] + [arch_ab] + argv_ab[i + 2:]
    argv_ab = argv_ab[:argv_ab.index("--workers")] + ["--workers", "1"]
    code, out = quiet(C.main, argv_ab)
    check(code == 1 and "ABORTED" in out and "archive skipped: aborted" in out and not os.path.exists(os.path.join(arch_ab, "manifest.sha1")),
          "an aborted bundle is not archived (HC-3): code %s, manifest %s" % (code, os.path.exists(os.path.join(arch_ab, "manifest.sha1"))))
    calls["busy_after"] = 0
    busy_calls["n"] = 0
    code, out = quiet(C.main, argv)
    check(code == 2 and "refusing to start" in out, "busy at start: refused")
    calls["busy_after"] = None
    code, out = quiet(C.main, base_args(proj) + ["--folder", run_dir, "--editor", editor, "--frozen", os.path.join(root, "absent.md")])
    check(code == 2 and "freeze step" in out, "no frozen block: refused")
    os.makedirs(os.path.join(proj, "Config"), exist_ok=True)
    with open(os.path.join(proj, "Config", "DefaultGame.ini"), "w", encoding="utf-8") as f:
        f.write("[x]\n")
    check(any(p.endswith("DefaultGame.ini") for p in C.guarded_files(proj)) and any(p.endswith(".dll") for p in C.guarded_files(proj)), "sha1 set includes the Config inis")
    code, out = quiet(C.main, base_args(proj) + ["--check-only", run_dir, "--cells", "R,slowR", "--max-seed", "2", "--bundle-b-logs", logs, "--bundle-b-csv", csvs,
                                                 "--b6a-dir", b6, "--no-archive"])
    check(code == 1 and "ABORTED" in out, "check-only keeps the aborted record")

    # F-1 (review): the default busy hook (bundleA.busy_processes over a mocked tasklist) sees this bundle's own
    # UnrealEditor-Cmd.exe children; they are not 'another job'. Another UnrealEditor.exe still aborts.
    C.OWN["pids"].clear()
    C.OWN["pids"].add(5001)
    C.HOOKS["busy_all"] = lambda: ["UnrealEditor-Cmd.exe (pid 5001)", "UnrealEditor-Cmd.exe (pid 500)", "UnrealEditor.exe (pid 50)"]
    check(C._busy_others() == ["UnrealEditor-Cmd.exe (pid 500)", "UnrealEditor.exe (pid 50)"], "busy: only the own pid 5001 is dropped")
    # The first full bundle C aborted at job 324 on the UBT SDK query every commandlet start runs. That query is not
    # another job; a UBT build still is.
    sdk = "dotnet: 9212 dotnet UnrealBuildTool.dll -Mode=ValidatePlatforms -OutputSDKs  -AllPlatforms -project=x.uproject"
    build = "dotnet: 9300 dotnet UnrealBuildTool.dll Cosmic_AscensionEditor Win64 Development -Project=x.uproject"
    C.HOOKS["busy_all"] = lambda: [sdk, build]
    check(C._busy_others() == [build], "busy: the SDK query (-Mode=ValidatePlatforms) is dropped, a UBT build is not")
    C.HOOKS["busy_all"] = lambda: [sdk]
    check(C._busy_others() == [], "busy: the SDK query alone is not a busy machine")
    C.OWN["pids"].clear()
    running, lock, pid_next, foreign = [], threading.Lock(), [5000], {"after_first": False, "on": False}

    class Done:
        def __init__(self, out):
            self.stdout, self.returncode = out, 0

    def fake_sub_run(cmd, **kw):
        if cmd and cmd[0] == "tasklist":
            with lock:
                rows = ['"UnrealEditor-Cmd.exe","%d","Console","1","100,000 K"' % pid for pid in running]
                if foreign["on"]:
                    rows.append('"UnrealEditor.exe","777","Console","1","900,000 K"')
            return Done("\n".join(['"explorer.exe","100","Console","1","1 K"'] + rows) + "\n")
        raise RuntimeError("unexpected subprocess %s" % cmd)

    class FakeProc:
        def __init__(self, cmdl):
            with lock:
                pid_next[0] += 1
                self.pid = pid_next[0]
                running.append(self.pid)
                if foreign["after_first"]:
                    foreign["on"] = True
            self.cmdl = cmdl

        def wait(self):
            try:
                time.sleep(0.2)
                log = next(x for x in self.cmdl if x.startswith("-AbsLog="))[len("-AbsLog="):]
                stem = os.path.basename(log)[:-4]
                for suffix in (".log", ".csv", "_rows.csv", "_claims.csv"):
                    shutil.copy2(os.path.join(tmpl, stem + suffix), os.path.join(os.path.dirname(log), stem + suffix))
                return 0
            finally:
                with lock:
                    running.remove(self.pid)

    real_sub = A.subprocess
    A.subprocess = types.SimpleNamespace(run=fake_sub_run, list2cmdline=real_sub.list2cmdline)
    try:
        C.HOOKS.update(run=C._run_process, popen=FakeProc, busy=C._busy_others, busy_all=A.busy_processes)
        argv_b = base_args(proj) + ["--folder", os.path.join(root, "run_busy"), "--cells", "R,slowR", "--max-seed", "2", "--editor", editor, "--frozen",
                                    os.path.join(root, "frozen_4_3.md"), "--no-archive", "--bundle-b-logs", logs, "--bundle-b-csv", csvs, "--b6a-dir", b6, "--workers", "2"]
        code, out = quiet(C.main, argv_b)
        check(code == 0 and "ABORTED" not in out and "bundle C: PASS" in out and pid_next[0] == 5004 and not C.OWN["pids"],
              "workers 2, tasklist shows only the bundle's own children: PASS (%d launched; %s)" % (pid_next[0] - 5000, [l for l in out.splitlines() if "busy" in l][:2]))
        foreign["after_first"] = True
        i = argv_b.index("--folder")
        code, out = quiet(C.main, argv_b[:i + 1] + [os.path.join(root, "run_busy2")] + argv_b[i + 2:])
        line = next((l for l in out.splitlines() if "ABORTED" in l), "")
        check(code == 1 and "UnrealEditor.exe (pid 777)" in line and "UnrealEditor-Cmd.exe" not in line,
              "another UnrealEditor.exe after the first launch aborts the bundle (own children not named): %s" % line)
    finally:
        A.subprocess = real_sub
        C.HOOKS.update(run=fake_run, popen=C._popen, busy=fake_busy, busy_all=A.busy_processes)

    print("13. --cell-set B over bundle B's logs")
    # F-2 (review): bundle B's death logs say 'death asked at 4400'; another --t-death is refused in every mode.
    for td in ("4300", "6000"):
        code, out = quiet(C.main, base_args(proj, **{"--t-death": td}) + ["--check-only", logs, "--cell-set", "B", "--max-seed", "2"])
        check(code == 2 and ("refusing: bundle B death logs say death asked at {4400 (2 logs)}, --t-death %s - stop and ask the lead" % td) in out,
              "--cell-set B with --t-death %s refused: %s" % (td, out.strip()[-200:]))
    code, out = quiet(C.main, base_args(proj, **{"--t-death": "4300"}) + ["--dry-run", "--editor", editor, "--bundle-b-logs", logs])
    check(code == 2 and "refusing: bundle B death logs" in out, "dry run with --t-death 4300 refused")
    code, out = quiet(C.main, base_args(proj, **{"--t-death": "4300"}) + ["--check-only", run_dir, "--cells", "R,slowR", "--max-seed", "2", "--bundle-b-logs", logs,
                                                                          "--bundle-b-csv", csvs, "--b6a-dir", b6, "--no-archive"])
    check(code == 2 and "refusing: bundle B death logs" in out, "check-only C with --t-death 4300 refused")
    code, out = quiet(C.main, base_args(proj, **{"--t-death": "6000"}) + ["--folder", os.path.join(root, "run_td"), "--editor", editor, "--frozen",
                                                                          os.path.join(root, "frozen_4_3.md"), "--bundle-b-logs", logs, "--no-archive", "--cells", "R",
                                                                          "--max-seed", "2"])
    check(code == 2 and "refusing: bundle B death logs" in out and not os.path.exists(os.path.join(root, "run_td")), "run with --t-death 6000 refused before job 1")
    code, out = quiet(C.main, base_args(proj) + ["--dry-run", "--editor", editor, "--bundle-b-logs", logs])
    check(code == 0 and "refusing" not in out, "dry run with --t-death 4400 matching bundle B goes on")
    logs_nl = os.path.join(root, "bB_logs_noline")
    shutil.copytree(logs, logs_nl)
    with open(os.path.join(logs_nl, "death_2.log"), encoding="utf-8") as f:
        kept = [l for l in f.read().splitlines() if "death asked at" not in l]
    with open(os.path.join(logs_nl, "death_2.log"), "w", encoding="utf-8") as f:
        f.write("\n".join(kept) + "\n")
    code, out = quiet(C.main, base_args(proj) + ["--check-only", logs_nl, "--cell-set", "B", "--max-seed", "2"])
    check(code == 2 and "{4400 (1 logs), no line (1 logs)}" in out, "a bundle B death log without the line is refused: %s" % out.strip()[-160:])
    st = C.bundle_b_state(logs, csvs, TD, 2)
    check(st["death_at"] == [4400], "bundle_b_state records the death ticks %s" % st.get("death_at"))
    code, out = quiet(C.main, base_args(proj) + ["--check-only", logs, "--cell-set", "B", "--max-seed", "2"])
    check(code == 0 and "check-only B: PASS" in out and "18 of 18 logs" in out, "bundle B logs parse: %s" % out[-300:])
    with open(os.path.join(logs, "R_1.log"), "w", encoding="utf-8") as f:
        f.write("broken\n")
    code, out = quiet(C.main, base_args(proj) + ["--check-only", logs, "--cell-set", "B", "--max-seed", "2"])
    check(code == 1 and "FAIL" in out, "a broken bundle B log fails the parser exercise")
    code, out = quiet(C.main, base_args(proj) + ["--cell-set", "B"])
    check(code == 2, "--cell-set B needs --check-only")

    print("")
    print("self-test: %d checks passed, %d failed" % (PASSES[0], len(FAILS)))
    return 0 if not FAILS else 1


if __name__ == "__main__":
    sys.exit(main())
