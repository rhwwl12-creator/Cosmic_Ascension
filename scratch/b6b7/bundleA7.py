"""[B-6b7] the section 4 pool (brief scratchpad\\b6b7\\brief_final.md section 4, section 3.2 item 3): a filtered copy of
[B-6b6]'s scratchpad\\b6b6\\bundleA.py, which it imports for its guards, parsing and fail rules (that file is not changed).

The pool (37 processes, the 20,000-tick jobs first, 6 workers):
  A1   the 30 R on-runs of bundle A again (seeds 1-30, 20,000 ticks, -Rules=ProvisionalEndTick:0, the R policy), names
       R_<seed> in Saved\\Balance\\b6b7_A1 - bundleA's own command (so that J1 compares like with like);
  on   the two 20,000-tick runner runs (Tools/run_checks.ps1:131-132, its exact arguments, absolute -Out, own -AbsLog) in
       Saved\\Balance\\b6b7_on (the 3,000 and wanderer lines come from the build pass's runner, copied there by hand);
  off  the five runner runs with -WithoutB6bData=1 (section 3.2 item 3 (a)) in Saved\\Balance\\b6b7_off.

Fail rules (bundleA.run_failures): exit != 0, a missing line or file, any end/race check line, any 'LogCosmic: Error'; a
failed run is rerun once. The sha1 guard covers the module dll and every DT_*.uasset (bundleA.guarded_files) plus
ST_CA.uasset. It refuses to start while an UnrealEditor / UBT process runs.

After the runs (or with --check-only) it checks:
  (a)  every off log: exactly one 'rows taken out' line, equal to OFF_LINE (R_A as %g), and the whole key line (after
       Remove-WrotePath, with its ruler / wanderer parts) equal to [B-6b5]'s seed42.json (git show 10205b6:...);
  (b)  --base DIR (default Saved\\Balance\\b6b7_base) against --on DIR (default Saved\\Balance\\b6b7_on): per run, every
       CSV and _rows.csv row with Tick < T4* of the base run (race: parts 2-3; no T4* inside the horizon = the whole run,
       and then the key too) equal (TickMicros, SaveBytes, SaveRawBytes aside); runs whose files are missing are listed;
  (c)  0 end check and 0 race check lines in every log of the pool (part of the fail rules).
J1-J4 and #136 on A1 are m_awake.py --a1's.

usage: python -I -X utf8 bundleA7.py [--only A1,on,off] [--seeds 1-30] [--workers 6] [--dry-run] [--check-only]
                                     [--r-a 0.375] [--base DIR] [--on DIR] [--root ROOT]
"""
import argparse
import concurrent.futures
import importlib.util
import json
import os
import re
import subprocess
import sys
import threading
import time

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:  # noqa: BLE001
    pass

HERE = os.path.dirname(os.path.abspath(__file__))
BUNDLE_A_PY = os.path.join(os.path.dirname(HERE), "b6b6", "bundleA.py")


def load_bundle_a(path=BUNDLE_A_PY):
    spec = importlib.util.spec_from_file_location("bundleA", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


A = load_bundle_a()

POLICY = ["-Answer=1", "-Build=1", "-Focus=1", "-Edict=1", "-Diplomacy=1", "-TellAll=0"]
# (name, baseline field, extra, ruler part, wanderer part, ticks) - Tools/run_checks.ps1:115, :119, :126, :131, :132.
RUNNER_RUNS = [
    ("checks", "summary_line", [], False, False, 3000),
    ("checks_ruler", "ruler_summary_line", POLICY, True, False, 3000),
    ("checks_wanderer", "wanderer_summary_line", POLICY + ["-Abdicate=1200", "-Wander=1"], True, True, 6000),
    ("checks_20000", "summary_line_20000", [], False, False, 20000),
    ("checks_ruler_20000", "ruler_summary_line_20000", POLICY, True, False, 20000),
]
RUNNER_TAIL = ["-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput"]
# CosmicBalanceCommandlet.cpp:1596 with the Note order of Tests/CosmicTestHelpers.h:375-378 on [B-6b7]'s data.
EXPECTED_REMOVED = ("feat_passage_by_ark feat_laws_and_binding project_ark project_binding crisis_species_extinction "
                    "edict_wide_machine_accord edict_wide_quarantine edict_wide_warning")
OFF_LINE = ("CosmicBalance: tables without B6b's data - rows taken out [%s], AIUpperRungProgressMultiplier 1 (was %s), "
            "FleetDelayCountdownShare 0 (was 0)")
B6B5_REV = "10205b6"


def fmt_g(x):
    # C's %g (6 significant digits, no trailing zeros) - UE_LOG's %g.
    return "%g" % x


def parse_seeds(text):
    out = set()
    for part in text.split(","):
        if "-" in part:
            lo, hi = part.split("-")
            out.update(range(int(lo), int(hi) + 1))
        elif part.strip():
            out.add(int(part))
    return out


def build_jobs(root, only, seeds):
    bal = os.path.join(root, "Saved", "Balance")
    jobs = []
    if "A1" in only:
        folder = os.path.join(bal, "b6b7_A1")
        for j in A.build_jobs(folder, cells={"R"}):
            if not j["off"] and j["seed"] in seeds:
                j["group"] = "A1"
                jobs.append(j)
    for group, sub, extra_all in (("on", "b6b7_on", []), ("off", "b6b7_off", ["-WithoutB6bData=1"])):
        if group not in only:
            continue
        folder = os.path.join(bal, sub)
        for name, field, extra, ruler, wanderer, ticks in RUNNER_RUNS:
            if group == "on" and ticks != 20000:
                continue  # the 3,000 and wanderer on-lines come from the build pass's runner
            base = os.path.join(folder, name)
            jobs.append({"name": name, "group": group, "cell": group, "seed": 42, "ticks": ticks, "off": group == "off",
                         "field": field, "ruler": ruler, "wanderer": wanderer, "args": list(extra) + extra_all,
                         "csv": base + ".csv", "rows": base + "_rows.csv", "claims": base + "_claims.csv", "log": base + ".log"})
    # The 20,000-tick jobs first (stable within a length).
    return sorted(jobs, key=lambda j: -j["ticks"])


def command_of(job, project, editor):
    if job["group"] == "A1":
        return A.command_of(job, project, editor)
    return [editor, project, "-run=CosmicBalance", "-Seed=42", "-Ticks=%d" % job["ticks"], "-Out=" + A.fwd(job["csv"])] \
        + job["args"] + RUNNER_TAIL + ["-AbsLog=" + A.fwd(job["log"])]


def guarded(root):
    files = A.guarded_files(root)
    st = os.path.join(root, "Content", "CA", "Data", "ST_CA.uasset")
    if os.path.exists(st) and st not in files:
        files.append(st)
    return sorted(files)


def execute(job, project, editor, lock, results, results_path):
    attempts = []
    for attempt in (1, 2):
        A.remove_outputs(job)
        start = time.time()
        code = subprocess.run(command_of(job, project, editor), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
        why = A.run_failures(job, A.parse_log(job["log"]), code)
        attempts.append({"attempt": attempt, "exit": code, "seconds": round(time.time() - start, 1), "why": why})
        if not why:
            break
    with lock:
        results[job["group"] + "/" + job["name"]] = {"ok": not attempts[-1]["why"], "attempts": attempts}
        with open(results_path, "w", encoding="utf-8") as f:
            json.dump(results, f, ensure_ascii=False, indent=1)
    return job, attempts


# ------------------------------------------------------------------------------------------------------------- checks


def strip_wrote(line):
    return re.sub(r", wrote .*?(?= \| |$)", "", line.strip())


def key_line(log, ruler, wanderer):
    """run_checks.ps1:87-99: the last 'CosmicBalance: seed' line, then ' | ' the last ruler / wanderer line."""
    text = A.read_text(log).splitlines()

    def last(pat):
        hits = [l for l in text if pat in l]
        if not hits:
            return None
        return re.search("(" + re.escape(pat) + ".*)", hits[-1]).group(1).rstrip()
    s = last("CosmicBalance: seed")
    if s is None:
        return None
    if ruler:
        s += " | " + (last("CosmicBalance ruler:") or "<no ruler line>")
    if wanderer:
        s += " | " + (last("CosmicBalance wanderer:") or "<no wanderer line>")
    return s


def baseline_at(root, rev):
    out = subprocess.run(["git", "-C", root, "show", "%s:Tools/baselines/seed42.json" % rev], capture_output=True, check=True).stdout
    return json.loads(out.decode("utf-8"))


def check_off(jobs, root, r_a):
    """(a): the off line letter for letter (exactly one) and the whole key line = [B-6b5]'s."""
    b5 = baseline_at(root, B6B5_REV)
    want_line = OFF_LINE % (EXPECTED_REMOVED, fmt_g(r_a))
    out = []
    for j in jobs:
        if j["group"] != "off":
            continue
        row = {"run": j["name"], "ok": False, "why": []}
        if not os.path.exists(j["log"]):
            row["why"].append("no log")
            out.append(row)
            continue
        text = A.read_text(j["log"])
        lines = [m.group(1).rstrip() for m in re.finditer(r"(CosmicBalance: tables without B6b's data.*?)\r?$", text, re.M)]
        if len(lines) != 1:
            row["why"].append("%d off lines" % len(lines))
        elif lines[0] != want_line:
            row["why"].append("off line differs: %s" % lines[0])
        key = key_line(j["log"], j["ruler"], j["wanderer"])
        if key is None:
            row["why"].append("no key line")
        elif strip_wrote(key) != strip_wrote(b5[j["field"]]):
            row["why"].append("key differs from [B-6b5]: now %s / then %s" % (strip_wrote(key)[:300], strip_wrote(b5[j["field"]])[:300]))
        row["key"] = strip_wrote(key) if key else None
        row["ok"] = not row["why"]
        out.append(row)
    return out


def t4_star(log):
    """min(race part 2 first AI T4, part 3 ruled T4) of a log, -1 if neither; and who."""
    p = A.parse_log(log)
    race = p.get("race") or {}
    ai, ru = race.get("ai_t4"), race.get("ruled")
    if not ai or not ru:
        return None, "race parts unreadable"
    cands = []
    if ai["t4_1"] >= 0:
        cands.append((ai["t4_1"], "AI " + ai["t4_1_civ"]))
    if ru["r4"] >= 0:
        cands.append((ru["r4"], "ruled " + ru["r4_civ"]))
    if not cands:
        return -1, "-"
    return min(cands)


def check_b(base, on):
    """(b): rows with Tick < T4* of the base run equal; a run with no T4* inside its horizon equal on every row and key."""
    out = []
    for name, field, extra, ruler, wanderer, ticks in RUNNER_RUNS:
        row = {"run": name, "ok": False, "why": [], "T4*": None, "who": None, "first_diff_csv": None, "first_diff_rows": None}
        blog, olog = os.path.join(base, name + ".log"), os.path.join(on, name + ".log")
        missing = [p for p in (blog, olog, os.path.join(base, name + ".csv"), os.path.join(on, name + ".csv"),
                               os.path.join(base, name + "_rows.csv"), os.path.join(on, name + "_rows.csv")) if not os.path.exists(p)]
        if missing:
            row["why"].append("missing: " + ", ".join(os.path.basename(os.path.dirname(p)) + "/" + os.path.basename(p) for p in missing))
            out.append(row)
            continue
        t, who = t4_star(blog)
        row["T4*"], row["who"] = t, who
        if t is None:
            row["why"].append(who)
            out.append(row)
            continue
        whole = t < 0 or t >= ticks
        for suffix, label in ((".csv", "csv"), ("_rows.csv", "rows")):
            ha, a = A.read_csv_by_tick(os.path.join(base, name + suffix))
            hb, b = A.read_csv_by_tick(os.path.join(on, name + suffix))
            if ha != hb:
                row["why"].append("%s headers differ" % label)
            d = A.first_difference(a, b)
            row["first_diff_" + label] = d
            if d is not None and (whole or d < t):
                row["why"].append("%s differs at tick %d %s" % (label, d, "(no T4* inside the horizon)" if whole else "< T4* %d" % t))
        if whole:
            kb, ko = key_line(blog, ruler, wanderer), key_line(olog, ruler, wanderer)
            if kb is None or ko is None or strip_wrote(kb) != strip_wrote(ko):
                row["why"].append("key differs with no T4* inside the horizon")
        row["ok"] = not row["why"]
        out.append(row)
    return out


def check_logs(jobs):
    """(c) and the fail rules over every log of the pool that exists."""
    out = []
    for j in jobs:
        p = A.parse_log(j["log"])
        out.append({"run": j["group"] + "/" + j["name"], "why": A.run_failures(j, p) if p.get("log_exists") else ["no log"],
                    "end_check": p.get("end_check"), "race_check": p.get("race_check")})
    return out


def report(jobs, root, r_a, base, on):
    logs = check_logs(jobs)
    off = check_off(jobs, root, r_a) if any(j["group"] == "off" for j in jobs) else []
    b = check_b(base, on)
    lines = ["# [B-6b7] section 4 pool - checks", ""]
    bad = [l for l in logs if l["why"]]
    lines.append("- logs: %d, failing %d%s" % (len(logs), len(bad), "" if not bad else " - " + "; ".join("%s: %s" % (l["run"], ", ".join(l["why"])) for l in bad)))
    lines.append("- (c) end check lines %d, race check lines %d" % (sum(l["end_check"] or 0 for l in logs), sum(l["race_check"] or 0 for l in logs)))
    for r in off:
        lines.append("- (a) off %s: %s%s" % (r["run"], "ok" if r["ok"] else "FAIL", "" if r["ok"] else " - " + "; ".join(r["why"])))
    for r in b:
        lines.append("- (b) %s: T4* %s (%s), first differing sample csv %s rows %s: %s%s" % (
            r["run"], r["T4*"], r["who"], r["first_diff_csv"], r["first_diff_rows"], "ok" if r["ok"] else "FAIL",
            "" if r["ok"] else " - " + "; ".join(r["why"])))
    ok = not bad and all(r["ok"] for r in off) and all(r["ok"] for r in b)
    lines.append("")
    lines.append("RESULT: %s" % ("PASS" if ok else "FAIL"))
    return ok, "\n".join(lines), {"logs": logs, "off": off, "b": b}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=A.DEFAULT_ROOT)
    ap.add_argument("--only", default="A1,on,off", help="comma list of A1,on,off")
    ap.add_argument("--seeds", default="1-30", help="A1 seeds, e.g. 1-30 or 3,7")
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--check-only", action="store_true")
    ap.add_argument("--r-a", type=float, default=0.375, help="R_A, for the off line's '(was %%g)'")
    ap.add_argument("--base", default=None, help="(b)'s base folder (default <root>\\Saved\\Balance\\b6b7_base)")
    ap.add_argument("--on", default=None, help="(b)'s on folder (default <root>\\Saved\\Balance\\b6b7_on)")
    ap.add_argument("--editor", default=A.EDITOR)
    a = ap.parse_args(argv)
    only = set(x.strip() for x in a.only.split(",") if x.strip())
    if only - {"A1", "on", "off"}:
        print("unknown --only: %s" % ", ".join(sorted(only - {"A1", "on", "off"})))
        return 2
    seeds = parse_seeds(a.seeds)
    project = os.path.join(a.root, "Cosmic_Ascension.uproject")
    base = os.path.abspath(a.base or os.path.join(a.root, "Saved", "Balance", "b6b7_base"))
    on = os.path.abspath(a.on or os.path.join(a.root, "Saved", "Balance", "b6b7_on"))
    jobs = build_jobs(a.root, only, seeds)

    if a.dry_run:
        for j in jobs:
            print(A.quote_cmd(command_of(j, project, a.editor)))
        n20 = sum(1 for j in jobs if j["ticks"] == 20000)
        print("# %d jobs (%d at 20,000 ticks first), %d workers; off line expected: %s" % (
            len(jobs), n20, a.workers, OFF_LINE % (EXPECTED_REMOVED, fmt_g(a.r_a))))
        return 0

    if a.check_only:
        ok, md, _ = report(jobs, a.root, a.r_a, base, on)
        print(md)
        return 0 if ok else 1

    busy = A.busy_processes()
    if busy:
        print("refusing to start - running now:\n  " + "\n  ".join(busy))
        return 2
    files = guarded(a.root)
    if not any(f.lower().endswith(".dll") for f in files):
        print("no UnrealEditor-Cosmic_Ascension*.dll under Binaries/Win64 - build first")
        return 2
    before = A.sha1_files(files)
    for d in {os.path.dirname(j["log"]) for j in jobs}:
        os.makedirs(d, exist_ok=True)
    results_path = os.path.join(a.root, "Saved", "Balance", "b6b7_pool_runs.json")
    results, lock = {}, threading.Lock()
    t0 = time.time()
    print("pool: %d jobs, %d workers" % (len(jobs), a.workers), flush=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
        futures = [pool.submit(execute, j, project, a.editor, lock, results, results_path) for j in jobs]
        for done, f in enumerate(concurrent.futures.as_completed(futures), 1):
            job, attempts = f.result()
            last = attempts[-1]
            print("[%d/%d %.0fs] %s/%s %s%s" % (done, len(jobs), time.time() - t0, job["group"], job["name"],
                                               "ok" if not last["why"] else "FAILED: " + "; ".join(last["why"]),
                                               " (rerun)" if len(attempts) > 1 else ""), flush=True)
    changed = A.sha1_diff(before, A.sha1_files(guarded(a.root)))
    ok, md, _ = report(jobs, a.root, a.r_a, base, on)
    print(md)
    print("sha1 guard: %s" % ("unchanged (%d files)" % len(before) if not changed else "CHANGED - discard the pool: " + "; ".join(changed)))
    print("pool: %s in %.1f min" % ("PASS" if ok and not changed else "FAIL", (time.time() - t0) / 60.0))
    return 0 if ok and not changed else 1


if __name__ == "__main__":
    sys.exit(main())
