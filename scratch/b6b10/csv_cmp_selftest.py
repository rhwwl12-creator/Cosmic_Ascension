"""Self-test of csv_cmp.py with synthetic files under this folder (selftest_tmp/). Single process, stdlib.

Run: python -I -X utf8 <scratchpad>/b6b10/csv_cmp_selftest.py
"""
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import csv_cmp  # noqa: E402

TMP = os.path.join(HERE, "selftest_tmp")

MAIN_H = "Tick,CivID,Tier,Stability,TickMicros,SaveBytes,LogLen,SaveRawBytes,Expansion"
ROWS_H = "Tick,SourceRow,Lines"
CLAIMS_H = "Claim,SystemID,Owner,PlacedTick,EndTick,EndKind"


def main_rows(last_tick, micros="1.0"):
    out = []
    for t in range(0, last_tick + 1, 1000):
        out.append("%d,civ_0,1,0.7500,%s,8437,%d,123473,RowTier" % (t, micros, t // 100))
        out.append("%d,civ_1,1,0.7500,%s,8437,%d,123473," % (t, micros, t // 100))
    return out


ROWS = ["100,event_civ_festival,4", "5900,project_reform,2", "6000,event_rebellion_rises,1"]
CLAIMS = ["1,sys_32,civ_1,3098,3398,joined", "2,sys_29,civ_1,5800,-1,", "3,sys_4,civ_5,5900,6100,joined",
          "4,sys_5,civ_5,1000,2000,relief"]


def end_line(kind, tick, clock, extra=""):
    return ("[2026.11.10-01.00.00:000][  0]LogCosmic: Display: CosmicBalance end: first %s at %d; clock at %d, "
            "deposed at -1; chronicle life 1 shown 1 of 1, more 0, ranks 0/0/0/0\n%s" % (kind, tick, clock, extra))


def write(path, lines, eol="\n"):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.write(eol.join(lines) + eol)


def build(case_dir, parent_kinds, run_mut=None, clock=9000, eol_run="\n", log_extra=None):
    """Parent and run folders with identical files; run_mut(name, suffix, lines) -> lines edits the run side."""
    parent = os.path.join(case_dir, "parent")
    bal = os.path.join(case_dir, "Balance")
    logs = os.path.join(case_dir, "Logs")
    for name in csv_cmp.PAIR_RUNS:
        last = 6000 if name == "checks_wanderer" else 20000
        files = {"": [MAIN_H] + main_rows(last), "_rows": [ROWS_H] + ROWS, "_claims": [CLAIMS_H] + CLAIMS}
        for suffix, lines in files.items():
            write(os.path.join(parent, name + suffix + ".csv"), lines)
            rl = list(lines)
            if run_mut:
                rl = run_mut(name, suffix, rl)
            if rl is not None:
                write(os.path.join(bal, name + suffix + ".csv"), rl, eol_run)
        kind, tick = parent_kinds.get(name, ("Clock", 6000))
        write(os.path.join(parent, name + ".log"), [end_line(kind, tick, 6000)])
    for name in csv_cmp.ERROR_LOGS:
        c = -1
        if name in ("checks_20000", "checks_ruler_20000"):
            c = clock
        extra = (log_extra or {}).get(name, "")
        write(os.path.join(logs, name + ".log"), [end_line("Clock" if c > 0 else "None", c, c, extra)])
    return parent, bal, logs


def go(case, parent_kinds=None, run_mut=None, clock=9000, eol_run="\n", log_extra=None, check_clock=9000,
       extra_argv=None):
    d = os.path.join(TMP, case)
    parent, bal, logs = build(d, parent_kinds or {}, run_mut, clock, eol_run, log_extra)
    argv = ["--parent", parent, "--balance", bal, "--logs", logs]
    if check_clock is not None:
        argv += ["--clock", str(check_clock)]
    argv += extra_argv or []
    lines = []
    rc = csv_cmp.run(csv_cmp.parse_args(argv), out=lines.append)
    return rc, lines


def edit(target_name, target_suffix, fn):
    def mut(name, suffix, lines):
        if name == target_name and suffix == target_suffix:
            return fn(lines)
        return lines
    return mut


def set_field(lines, line_no, col, value):
    header = lines[0].split(",")
    row = lines[line_no].split(",")
    row[header.index(col)] = value
    out = list(lines)
    out[line_no] = ",".join(row)
    return out


FAILS = []


def expect(label, cond, lines):
    print(("ok   " if cond else "FAIL ") + label)
    if not cond:
        FAILS.append(label)
        for ln in lines:
            print("     | " + ln)


def fails_of(lines):
    return [ln for ln in lines if ln.startswith("FAIL ")]


def main():
    shutil.rmtree(TMP, ignore_errors=True)

    rc, out = go("equal")
    expect("equal: rc 0, 17 PASS lines (9 pairs + 5 logs + 3 clock) + final", rc == 0 and not fails_of(out)
           and out[-1].startswith("csv_cmp: PASS (17 lines)"), out)
    expect("equal: Clock@6000 ruler main cut at < 6000 (12 rows), input-less every row (42)",
           any("checks_ruler_20000.csv: rows 12/12 (rows < 6000" in ln for ln in out)
           and any("checks_20000.csv: rows 42/42 (every row" in ln for ln in out), out)
    expect("equal: claims filter keeps 2 (end -1 and end 6100 dropped)",
           any("checks_wanderer_claims.csv: rows 2/2 (PlacedTick < 6000 and 0 <= EndTick < 6000" in ln for ln in out), out)

    # A kept column differs before 6000 in a filtered file.
    rc, out = go("kept_diff", run_mut=edit("checks_ruler_20000", "", lambda l: set_field(l, 3, "Stability", "0.7400")))
    f = fails_of(out)
    expect("kept column differs: rc 1, one FAIL naming Stability", rc == 1 and len(f) == 1
           and "checks_ruler_20000.csv" in f[0] and "Stability" in f[0], out)

    # A kept column differs only at/after 6000 in a filtered file: passes; in the input-less run: fails.
    late = 2 * 7 + 1  # line index of tick 7000 first row
    rc, out = go("late_diff", run_mut=edit("checks_ruler_20000", "", lambda l: set_field(l, late, "LogLen", "999")))
    expect("difference at tick 7000 in ruler 20000 (Clock@6000 parent): PASS", rc == 0, out)
    rc, out = go("late_diff_all", run_mut=edit("checks_20000", "", lambda l: set_field(l, late, "LogLen", "999")))
    expect("difference at tick 7000 in input-less 20000: FAIL", rc == 1 and len(fails_of(out)) == 1, out)
    rc, out = go("late_diff_noclock", parent_kinds={"checks_ruler_20000": ("OtherPeople", 4321)},
                 run_mut=edit("checks_ruler_20000", "", lambda l: set_field(l, late, "LogLen", "999")))
    expect("difference at 7000 when parent record is not Clock@6000: FAIL (every row)", rc == 1
           and any("every row" in ln and "OtherPeople@4321" in ln for ln in fails_of(out)), out)

    # Only dropped columns differ.
    def dropped(lines):
        for i in (1, 5, 9):
            lines = set_field(lines, i, "TickMicros", "77.7")
            lines = set_field(lines, i, "SaveBytes", "1")
            lines = set_field(lines, i, "SaveRawBytes", "2")
        return lines
    rc, out = go("dropped_only", run_mut=edit("checks_20000", "", dropped))
    expect("only TickMicros/SaveBytes/SaveRawBytes differ: PASS", rc == 0, out)

    # A dropped column missing on one side is fine; a kept column missing fails.
    def drop_col(col):
        def fn(lines):
            h = lines[0].split(",")
            i = h.index(col)
            return [",".join(c for j, c in enumerate(ln.split(",")) if j != i) for ln in lines]
        return fn
    rc, out = go("missing_dropped_col", run_mut=edit("checks_20000", "", drop_col("SaveRawBytes")))
    expect("dropped column missing in run: PASS", rc == 0, out)
    rc, out = go("missing_col", run_mut=edit("checks_wanderer", "_rows", drop_col("Lines")))
    f = fails_of(out)
    expect("kept column missing: FAIL naming Lines", rc == 1 and len(f) == 1 and "Lines" in f[0], out)

    # A missing file.
    rc, out = go("missing_file", run_mut=edit("checks_ruler_20000", "_claims", lambda l: None))
    f = fails_of(out)
    expect("missing run file: FAIL 'missing file'", rc == 1 and len(f) == 1 and "missing file" in f[0], out)

    # Row count mismatch (a row added before 6000; and one dropped at the end of an every-row file).
    rc, out = go("rowcount", run_mut=edit("checks_wanderer", "_rows", lambda l: l[:2] + ["200,event_x,1"] + l[2:]))
    expect("extra row before 6000: FAIL", rc == 1 and len(fails_of(out)) == 1, out)
    rc, out = go("rowcount_tail", run_mut=edit("checks_20000", "", lambda l: l[:-1]))
    f = fails_of(out)
    expect("missing last row of an every-row file: FAIL 'row count differs'", rc == 1 and len(f) == 1
           and "row count differs" in f[0], out)
    rc, out = go("rowcount_after_cut", run_mut=edit("checks_ruler_20000", "", lambda l: l[:-4]))
    expect("rows after 6000 removed in a Clock@6000 file: PASS", rc == 0, out)

    # CRLF on the run side, LF on the parent side.
    rc, out = go("crlf", eol_run="\r\n")
    expect("CRLF vs LF: PASS", rc == 0, out)

    # Error lines in the logs.
    rc, out = go("err_end", log_extra={"checks_ruler": "[x]LogCosmic: Error: CosmicBalance end check: at 600 (a) the record is A, the predicate's is B\n"})
    f = fails_of(out)
    expect("end-check line: FAIL in checks_ruler.log", rc == 1 and len(f) == 1 and f[0].startswith("FAIL checks_ruler.log"), out)
    rc, out = go("err_race", log_extra={"checks": "CosmicBalance race check: the home cap was passed at 2 boundaries\n"})
    expect("race-check line: FAIL", rc == 1 and len(fails_of(out)) == 1, out)
    rc, out = go("err_plain", log_extra={"checks_20000": "[x]LogCosmic: Error: something\n"})
    expect("LogCosmic: Error line: FAIL", rc == 1 and len(fails_of(out)) == 1, out)
    rc, out = go("warn_ok", log_extra={"checks_20000": "[x]LogCosmic: Warning: fine\n"})
    expect("warning line: PASS", rc == 0, out)

    # Missing parent log -> the three pairs of that run fail (no override exists, brief 0.1 step 5).
    d = os.path.join(TMP, "no_parent_log")
    parent, bal, logs = build(d, {})
    os.remove(os.path.join(parent, "checks_wanderer.log"))
    base = ["--parent", parent, "--balance", bal, "--logs", logs, "--clock", "9000"]
    lines = []
    rc = csv_cmp.run(csv_cmp.parse_args(base), out=lines.append)
    expect("parent log missing: 3 FAIL", rc == 1 and len(fails_of(lines)) == 3, lines)
    # Missing run log (parent log restored).
    write(os.path.join(parent, "checks_wanderer.log"), [end_line("Clock", 6000, 6000)])
    os.remove(os.path.join(logs, "checks.log"))
    lines = []
    rc = csv_cmp.run(csv_cmp.parse_args(base), out=lines.append)
    expect("run log missing: FAIL", rc == 1 and len(fails_of(lines)) == 1 and "checks.log" in fails_of(lines)[0], lines)

    # R1 (reviewer's override_vs_log / override_typo / override_noeq): --parent-kind no longer exists,
    # argparse refuses it (exit 2) instead of letting it overwrite the parent log's record.
    for case, extra in (("override_vs_log", ["--parent-kind", "checks_ruler_20000=Clock@6000"]),
                        ("override_typo", ["--parent-kind", "checks_ruler20000=Clock@6000"]),
                        ("override_noeq", ["--parent-kind", "Clock@6000"])):
        try:
            go(case, parent_kinds={"checks_ruler_20000": ("OtherPeople", 5000)}, extra_argv=extra)
            code = None
        except SystemExit as e:
            code = e.code
        expect("R1 %s: --parent-kind refused (SystemExit 2)" % case, code == 2, [repr(code)])

    # R1: parent log 'first OtherPeople at 5000'; the run's ruler 20,000 main differs only at a tick-6001
    # row (Stability 0.75 vs 0.99) -> every row is compared, FAIL.
    def add6001(stability):
        def fn(lines):
            out = list(lines)
            i = next(k for k, ln in enumerate(out) if ln.startswith("7000,civ_0,"))
            out.insert(i, "6001,civ_0,1,%s,1.0,8437,60,123473,RowTier" % stability)
            return out
        return fn
    d = os.path.join(TMP, "parent_none_diff6001")
    parent, bal, logs = build(d, {"checks_ruler_20000": ("OtherPeople", 5000)},
                              run_mut=edit("checks_ruler_20000", "", add6001("0.9900")))
    pm = os.path.join(parent, "checks_ruler_20000.csv")
    with open(pm, encoding="utf-8") as fh:
        plines = fh.read().splitlines()
    write(pm, add6001("0.7500")(plines))
    lines = []
    rc = csv_cmp.run(csv_cmp.parse_args(["--parent", parent, "--balance", bal, "--logs", logs, "--clock", "9000"]),
                     out=lines.append)
    f = fails_of(lines)
    expect("R1 parent log OtherPeople@5000, Stability differs at 6001: FAIL (every row)", rc == 1 and len(f) == 1
           and "checks_ruler_20000.csv" in f[0] and "OtherPeople@5000" in f[0] and "Stability" in f[0], lines)
    late7 = 2 * 7 + 1
    rc, out = go("parent_clock7000", parent_kinds={"checks_ruler_20000": ("Clock", 7000)},
                 run_mut=edit("checks_ruler_20000", "", lambda l: set_field(l, late7, "LogLen", "999")))
    expect("R1 parent record Clock@7000 (not the cut): every row, FAIL", rc == 1 and len(fails_of(out)) == 1
           and "every row" in fails_of(out)[0], out)

    # --clock: 20,000 logs at C, wanderer at -1.
    rc, out = go("clock_ok", clock=9000, check_clock=9000)
    expect("--clock 9000 with logs at 9000/-1: PASS, 3 clock lines", rc == 0
           and sum("clock at" in ln and "(want" in ln for ln in out) == 3, out)
    rc, out = go("clock_stale", clock=6000, check_clock=9000)
    expect("--clock 9000 with a stale 6000: FAIL x2", rc == 1 and len(fails_of(out)) == 2, out)
    # R2: wanderer log 'clock at 6000' (stale DT_Rules.uasset) fails under branch M.
    d = os.path.join(TMP, "clock_stale_wanderer")
    parent, bal, logs = build(d, {})
    write(os.path.join(logs, "checks_wanderer.log"), [end_line("Clock", 6000, 6000)])
    lines = []
    rc = csv_cmp.run(csv_cmp.parse_args(["--parent", parent, "--balance", bal, "--logs", logs, "--clock", "9000"]),
                     out=lines.append)
    f = fails_of(lines)
    expect("R2 wanderer log 'clock at 6000' with --clock 9000: FAIL", rc == 1 and len(f) == 1
           and "checks_wanderer.log: clock at 6000 (want -1)" in f[0], lines)
    rc, out = go("clock_18000", clock=18000, check_clock=18000)
    expect("R2 --clock 18000 (upper bound) accepted: PASS", rc == 0, out)
    # R2 (reviewer's stale-build case): --clock 6000, no --clock, above 18000, or C == cut -> SystemExit 2.
    for case, argv in (("clock_6000", ["--clock", "6000"]), ("clock_missing", []),
                       ("clock_18001", ["--clock", "18001"]), ("clock_eq_cut", ["--cut", "7000", "--clock", "7000"])):
        try:
            csv_cmp.parse_args(["--parent", "p"] + argv)
            code = None
        except SystemExit as e:
            code = e.code
        expect("R2 %s: refused (SystemExit 2)" % case, code == 2, [repr(code)])

    # A Tick that is not an integer in a filtered file.
    rc, out = go("bad_tick", run_mut=edit("checks_ruler_20000", "_rows", lambda l: set_field(l, 1, "Tick", "x")))
    expect("non-integer Tick: FAIL", rc == 1 and len(fails_of(out)) == 1 and "not an integer" in fails_of(out)[0], out)

    shutil.rmtree(TMP, ignore_errors=True)
    print("selftest: %s (%d failed)" % ("PASS" if not FAILS else "FAIL", len(FAILS)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
