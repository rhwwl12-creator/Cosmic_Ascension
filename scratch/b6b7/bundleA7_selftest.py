"""Self-test of bundleA7.py's checks on outputs that exist (read-only): [B-6b6]'s off runs (b6b6_offA, four rows, r 1)
must pass (a) with [B-6b6]'s expectations; b6b6_on against b6b7_base must pass (b); b6b6_offA as the 'on' side must fail
(b) on every run; the 37-job pool and the filters."""
import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("bundleA7", os.path.join(HERE, "bundleA7.py"))
M = importlib.util.module_from_spec(spec)
spec.loader.exec_module(M)

ROOT = M.A.DEFAULT_ROOT
BAL = os.path.join(ROOT, "Saved", "Balance")
fails = []


def check(cond, what):
    print(("ok   " if cond else "FAIL ") + what)
    if not cond:
        fails.append(what)


jobs = M.build_jobs(ROOT, {"A1", "on", "off"}, set(range(1, 31)))
check(len(jobs) == 37, "37 jobs (%d)" % len(jobs))
check([j["ticks"] for j in jobs[:34]] == [20000] * 34, "the 34 jobs at 20,000 ticks first")
check(sum(j["group"] == "A1" for j in jobs) == 30 and sum(j["group"] == "on" for j in jobs) == 2 and sum(j["group"] == "off" for j in jobs) == 5,
      "30 A1, 2 on, 5 off")
check(len(M.build_jobs(ROOT, {"A1"}, {3, 7})) == 2, "seed filter 3,7 gives two A1 jobs")
check(len(M.build_jobs(ROOT, {"off"}, set())) == 5, "only off gives five")
cmd = M.command_of([j for j in jobs if j["group"] == "off" and j["name"] == "checks_wanderer"][0], "P", "E")
check(cmd[cmd.index("-Wander=1") + 1] == "-WithoutB6bData=1" and "-stdout" in cmd and cmd[-1].startswith("-AbsLog="), "off wanderer: runner args then the switch, the runner tail")
check(M.fmt_g(0.375) == "0.375" and M.fmt_g(1.0) == "1" and M.fmt_g(0.5) == "0.5", "%g as UE_LOG prints it")

# (a) on [B-6b6]'s off runs with [B-6b6]'s expectations.
off_jobs = []
for j in M.build_jobs(ROOT, {"off"}, set()):
    stem = os.path.join(BAL, "b6b6_offA", j["name"])
    j.update({"log": stem + ".log", "csv": stem + ".csv", "rows": stem + "_rows.csv", "claims": stem + "_claims.csv"})
    off_jobs.append(j)
saved = M.EXPECTED_REMOVED
M.EXPECTED_REMOVED = "feat_passage_by_ark project_ark project_binding crisis_species_extinction"
res = M.check_off(off_jobs, ROOT, 1.0)
check(len(res) == 5 and all(r["ok"] for r in res), "(a) passes on b6b6_offA with the four rows and r 1: %s" % [r["why"] for r in res if not r["ok"]])
res = M.check_off(off_jobs, ROOT, 0.375)
check(all(not r["ok"] and any("off line differs" in w for w in r["why"]) for r in res), "(a) fails on the wrong '(was %g)'")
M.EXPECTED_REMOVED = saved
res = M.check_off(off_jobs, ROOT, 1.0)
check(all(not r["ok"] for r in res), "(a) fails on [B-6b7]'s eight-row list against [B-6b6]'s logs")

# (b)
b = M.check_b(os.path.join(BAL, "b6b7_base"), os.path.join(BAL, "b6b6_on"))
check(all(r["ok"] for r in b), "(b) b6b6_on = b6b7_base: %s" % [(r["run"], r["T4*"], r["why"]) for r in b])
check([r["T4*"] for r in b] == [-1, -1, 3197, 3149, 3651], "T4* per run -1 -1 3197 3149 3651: %s" % [r["T4*"] for r in b])
b = M.check_b(os.path.join(BAL, "b6b7_base"), os.path.join(BAL, "b6b6_offA"))
check(all(not r["ok"] for r in b), "(b) fails with the off runs as the on side: %s" % [(r["run"], r["why"][:1]) for r in b])
b = M.check_b(os.path.join(BAL, "b6b7_base"), os.path.join(BAL, "no_such_folder"))
check(all(not r["ok"] and r["why"][0].startswith("missing") for r in b), "(b) lists missing files")

# Fail rules on the logs.
logs = M.check_logs(off_jobs)
check(all(not l["why"] for l in logs), "fail rules pass on b6b6_offA: %s" % [l for l in logs if l["why"]])

print("self-test: %d failed" % len(fails))
sys.exit(1 if fails else 0)
