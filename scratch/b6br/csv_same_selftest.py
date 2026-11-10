"""Self-test of csv_same.py ([B-6br] brief section 4), on copies of section 0.5's before files in a temporary folder.
Cases: identical files -> PASS (0); only TickMicros changed -> PASS (0); one Systems cell changed -> FAIL (1); one
_claims row removed -> FAIL (1); one _rows file missing -> exit 2. Also: SaveBytes/SaveRawBytes changed -> PASS; a
_rows cell changed -> FAIL; the off_ names pair the same way.
Usage: python -I -X utf8 csv_same_selftest.py [<before dir>]   (stdlib; writes only to a temporary folder)"""
import csv
import io
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
BEFORE = sys.argv[1] if len(sys.argv) > 1 else \
    r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension\Saved\Balance\b6br_before"
RUNS = "checks,checks_ruler,checks_wanderer,checks_20000,checks_ruler_20000"
OFF_RUNS = "off_checks,off_checks_ruler,off_checks_wanderer,off_checks_20000,off_checks_ruler_20000"
fails = 0


def expect(cond, what):
    global fails
    print(("ok   " if cond else "FAIL ") + what)
    fails += 0 if cond else 1


def call(before, after, runs):
    r = subprocess.run([sys.executable, "-I", "-X", "utf8", os.path.join(HERE, "csv_same.py"), "--before", before,
                        "--after", after, "--runs", runs], capture_output=True, text=True, encoding="utf-8")
    return r.returncode, r.stdout + r.stderr


def edit(path, fn):
    with open(path, encoding="utf-8-sig", newline="") as f:
        rows = list(csv.reader(io.StringIO(f.read(), newline="")))
    rows = fn(rows)
    with open(path, "w", encoding="utf-8", newline="") as f:
        csv.writer(f, lineterminator="\n").writerows(rows)


def set_cell(col, row_at, value):
    def fn(rows):
        i = rows[0].index(col)
        rows[row_at][i] = value(rows[row_at][i])
        return rows
    return fn


def fresh(tmp):
    after = os.path.join(tmp, "after")
    if os.path.isdir(after):
        shutil.rmtree(after)
    shutil.copytree(BEFORE, after)
    return after


tmp = tempfile.mkdtemp(prefix="csv_same_selftest_")
try:
    after = fresh(tmp)
    code, out = call(BEFORE, after, RUNS)
    expect(code == 0 and out.count("PASS ") == 15 and "csv_same: PASS" in out, "identical files: PASS, 15 pairs (exit %d)" % code)
    code, out = call(BEFORE, after, OFF_RUNS)
    expect(code == 0 and out.count("PASS ") == 15, "identical off_ files: PASS, 15 pairs (exit %d)" % code)

    # Rewritten with LF line ends and no BOM: the reader does not see the difference.
    edit(os.path.join(after, "checks.csv"), lambda rows: rows)
    code, out = call(BEFORE, after, "checks")
    expect(code == 0, "the same cells rewritten (other line ends): PASS (exit %d)" % code)

    after = fresh(tmp)
    edit(os.path.join(after, "checks_20000.csv"), set_cell("TickMicros", 5, lambda v: str(float(v or 0) + 17.25)))
    edit(os.path.join(after, "checks_20000.csv"), set_cell("TickMicros", -1, lambda v: "999999"))
    code, out = call(BEFORE, after, RUNS)
    expect(code == 0, "only TickMicros changed: PASS (exit %d)" % code)
    edit(os.path.join(after, "checks.csv"), set_cell("SaveBytes", 3, lambda v: str(int(v or 0) + 1)))
    edit(os.path.join(after, "checks.csv"), set_cell("SaveRawBytes", 3, lambda v: str(int(v or 0) + 1)))
    code, out = call(BEFORE, after, RUNS)
    expect(code == 0, "TickMicros, SaveBytes and SaveRawBytes changed: PASS (exit %d)" % code)

    after = fresh(tmp)
    edit(os.path.join(after, "checks_ruler.csv"), set_cell("Systems", 7, lambda v: str(int(v) + 1)))
    code, out = call(BEFORE, after, RUNS)
    expect(code == 1 and "FAIL checks_ruler.csv" in out and "Systems" in out and "csv_same: FAIL" in out,
           "one Systems cell changed: FAIL naming the column (exit %d)" % code)

    after = fresh(tmp)
    edit(os.path.join(after, "checks_20000_claims.csv"), lambda rows: rows[:2] + rows[3:])
    code, out = call(BEFORE, after, RUNS)
    expect(code == 1 and "FAIL checks_20000_claims.csv" in out, "one _claims row removed: FAIL (exit %d)" % code)

    after = fresh(tmp)
    edit(os.path.join(after, "checks_wanderer_rows.csv"), set_cell("Lines", 4, lambda v: str(int(v) + 1)))
    code, out = call(BEFORE, after, RUNS)
    expect(code == 1 and "FAIL checks_wanderer_rows.csv" in out, "one _rows cell changed: FAIL (exit %d)" % code)

    after = fresh(tmp)
    edit(os.path.join(after, "off_checks_ruler_20000.csv"), set_cell("Stability", 9, lambda v: v + "1"))
    code, out = call(BEFORE, after, OFF_RUNS)
    expect(code == 1 and "FAIL off_checks_ruler_20000.csv" in out, "an off_ cell changed: FAIL (exit %d)" % code)

    after = fresh(tmp)
    os.remove(os.path.join(after, "checks_ruler_rows.csv"))
    code, out = call(BEFORE, after, RUNS)
    expect(code == 2 and "missing file" in out and "csv_same: FAIL" in out, "one _rows file missing: exit 2 (exit %d)" % code)
finally:
    shutil.rmtree(tmp, ignore_errors=True)
print("csv_same_selftest: %s" % ("PASS" if fails == 0 else "FAIL (%d)" % fails))
sys.exit(1 if fails else 0)
