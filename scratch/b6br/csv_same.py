"""[B-6br] brief section 4 (c)/(d): the commandlet CSVs of a run are the ones before this commit, cell for cell.

For every run name and every file of it (<name>.csv, <name>_rows.csv, <name>_claims.csv) compares the --before folder's
file with the --after folder's file: every row, every cell. The only cells left out are the main CSV's TickMicros,
SaveBytes and SaveRawBytes columns (clock and save sizes); _rows and _claims are compared whole. Any other difference is
a FAIL (brief R1 - a stop, never a new exclusion). Based on <scratchpad>/b6b10/csv_cmp.py (its reader, DROPPED and
SUFFIXES), without the clock-cut logic.

One PASS/FAIL line per file pair; a last line 'csv_same: PASS|FAIL'.
Exit 0 when every pair passes, 1 on any FAIL, 2 when a file is missing (or the arguments are wrong).

Usage:
  python -I -X utf8 csv_same.py --before <dir> --after <dir> --runs checks,checks_ruler,...
Python stdlib only; writes nothing.
"""
import argparse
import csv
import io
import os
import sys

DROPPED = ("TickMicros", "SaveBytes", "SaveRawBytes")
SUFFIXES = ("", "_rows", "_claims")


class Missing(Exception):
    pass


class Bad(Exception):
    pass


def read_text(path):
    # Universal newlines (CRLF == LF), a BOM is dropped.
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        return f.read()


def read_csv(path):
    if not os.path.isfile(path):
        raise Missing("missing file %s" % path)
    rows = [r for r in csv.reader(io.StringIO(read_text(path), newline="")) if r != []]
    if not rows:
        raise Bad("empty file %s" % path)
    return rows[0], rows[1:]


def compare_pair(before_path, after_path, suffix):
    """(ok, detail). Raises Missing / Bad."""
    bh, brows = read_csv(before_path)
    ah, arows = read_csv(after_path)
    dropped = DROPPED if suffix == "" else ()
    bkeep = [c for c in bh if c not in dropped]
    akeep = [c for c in ah if c not in dropped]
    if bkeep != akeep:
        only_b = [c for c in bkeep if c not in akeep]
        only_a = [c for c in akeep if c not in bkeep]
        return False, "columns differ (before only %s, after only %s%s)" % (
            only_b, only_a, ", order" if not only_b and not only_a else "")
    bix = [bh.index(c) for c in bkeep]
    aix = [ah.index(c) for c in akeep]
    for header, rows, path in ((bh, brows, before_path), (ah, arows, after_path)):
        for n, row in enumerate(rows, start=2):
            if len(row) != len(header):
                raise Bad("%s line %d: %d fields, header has %d" % (path, n, len(row), len(header)))
    for n, (br, ar) in enumerate(zip(brows, arows), start=2):
        bv = [br[i] for i in bix]
        av = [ar[i] for i in aix]
        if bv != av:
            cols = [bkeep[i] for i in range(len(bv)) if bv[i] != av[i]]
            return False, "first difference at line %d in %s: before %s, after %s" % (
                n, cols[:6], [bv[bkeep.index(c)] for c in cols[:3]], [av[bkeep.index(c)] for c in cols[:3]])
    if len(brows) != len(arows):
        return False, "row count differs: before %d, after %d" % (len(brows), len(arows))
    return True, "%d rows, %d columns%s" % (len(brows), len(bkeep),
                                            (" (without %s)" % "/".join(c for c in DROPPED if c in bh)) if suffix == "" else "")


def run(args, out=print):
    runs = [r.strip() for r in args.runs.split(",") if r.strip()]
    if not runs:
        out("csv_same: no runs named")
        return 2
    any_fail = False
    any_missing = False
    for name in runs:
        for suffix in SUFFIXES:
            fname = name + suffix + ".csv"
            try:
                ok, detail = compare_pair(os.path.join(args.before, fname), os.path.join(args.after, fname), suffix)
            except Missing as e:
                any_missing = True
                out("FAIL %s: %s" % (fname, e))
                continue
            except Bad as e:
                any_fail = True
                out("FAIL %s: %s" % (fname, e))
                continue
            any_fail = any_fail or not ok
            out("%s %s: %s" % ("PASS" if ok else "FAIL", fname, detail))
    out("csv_same: %s" % ("PASS" if not (any_fail or any_missing) else "FAIL"))
    return 2 if any_missing else (1 if any_fail else 0)


def parse_args(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--before", required=True, help="folder of the CSVs before this commit")
    ap.add_argument("--after", required=True, help="folder of the CSVs of this commit's build")
    ap.add_argument("--runs", required=True, help="comma-separated run names (the file stems)")
    return ap.parse_args(argv)


if __name__ == "__main__":
    sys.exit(run(parse_args(sys.argv[1:])))
