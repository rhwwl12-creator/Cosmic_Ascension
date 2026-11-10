"""[B-6b10] brief section 3.4: the CSV and log check of branch M (read-only).

Compares the parent's copies (brief section 0.1 step 4, <scratchpad>/b6b10/parent_csv/) with this
run's files (Saved/Balance/<name>.csv, _rows.csv, _claims.csv), dropping the columns TickMicros,
SaveBytes and SaveRawBytes, and checks the five logs of this run (Saved/Logs/<name>.log) for error
lines. One PASS/FAIL line per file pair and per log; a last line 'csv_cmp: PASS|FAIL ...'.
Exit 0 only when every line passes.

Which rows (brief section 3.4 table):
  - checks_20000 (input-less 20,000): every row of all three files, whatever the parent record.
  - checks_ruler_20000, checks_wanderer: every row when the parent record (the 'first <kind> at
    <tick>' part of the parent log's 'CosmicBalance end:' line) is not Clock@<cut>; when it is,
    main and _rows keep rows with Tick < cut, _claims keeps rows with PlacedTick < cut and
    0 <= EndTick < cut.
Error lines (all five logs): 'CosmicBalance end check:', 'CosmicBalance race check:',
'LogCosmic: Error'. The knob arrival check of brief section 3.3 (branch M, --clock C is required,
cut < C <= 18000): the two 20,000 logs show 'clock at C', the wanderer log 'clock at -1'.
The record kind comes only from the parent log (brief section 0.1 step 5); there is no override.

Usage (from the project root):
  python -I -X utf8 <scratchpad>/b6b10/csv_cmp.py --parent <scratchpad>/b6b10/parent_csv
      --clock C [--balance Saved/Balance] [--logs Saved/Logs] [--cut 6000]
Python stdlib only; writes nothing.
"""
import argparse
import csv
import io
import os
import re
import sys

DROPPED = ("TickMicros", "SaveBytes", "SaveRawBytes")
PAIR_RUNS = ("checks_20000", "checks_ruler_20000", "checks_wanderer")
ALWAYS_ALL = ("checks_20000",)
ERROR_LOGS = ("checks", "checks_ruler", "checks_wanderer", "checks_20000", "checks_ruler_20000")
SUFFIXES = ("", "_rows", "_claims")
ERROR_PATTERNS = ("CosmicBalance end check:", "CosmicBalance race check:", "LogCosmic: Error")
END_RE = re.compile(r"CosmicBalance end: first (\S+) at (-?\d+); clock at (-?\d+)")


class CmpError(Exception):
    pass


def read_text(path):
    # Swappable reader for the self-test; universal newlines (CRLF == LF), a BOM is dropped.
    with open(path, "r", encoding="utf-8-sig", newline="") as f:
        return f.read()


def read_csv(path, reader=read_text):
    if not os.path.isfile(path):
        raise CmpError("missing file %s" % path)
    rows = list(csv.reader(io.StringIO(reader(path), newline="")))
    rows = [r for r in rows if r != []]
    if not rows:
        raise CmpError("empty file %s" % path)
    return rows[0], rows[1:]


def end_line_of(path, reader=read_text):
    """(kind, tick, clock) of the last 'CosmicBalance end:' line, or None."""
    if not os.path.isfile(path):
        return None
    found = None
    for line in reader(path).splitlines():
        m = END_RE.search(line)
        if m:
            found = (m.group(1), int(m.group(2)), int(m.group(3)))
    return found


def int_of(value, col, path, lineno):
    try:
        return int(value)
    except ValueError:
        raise CmpError("%s line %d: column %s is not an integer: %r" % (path, lineno, col, value))


def row_filter(suffix, header, path, cut):
    """A predicate over (row, lineno) for a Clock@cut parent, by the file kind."""
    def idx(col):
        if col not in header:
            raise CmpError("%s has no column %s" % (path, col))
        return header.index(col)
    if suffix == "_claims":
        p, e = idx("PlacedTick"), idx("EndTick")
        def keep(row, n):
            placed = int_of(row[p], "PlacedTick", path, n)
            end = int_of(row[e], "EndTick", path, n)
            return placed < cut and 0 <= end < cut
        return keep
    t = idx("Tick")
    return lambda row, n: int_of(row[t], "Tick", path, n) < cut


def compare_pair(parent_path, run_path, suffix, filtered, cut, reader=read_text):
    """Returns (ok, message, parent_kept, run_kept)."""
    ph, prows = read_csv(parent_path, reader)
    rh, rrows = read_csv(run_path, reader)
    pkeep = [c for c in ph if c not in DROPPED]
    rkeep = [c for c in rh if c not in DROPPED]
    if pkeep != rkeep:
        only_p = [c for c in pkeep if c not in rkeep]
        only_r = [c for c in rkeep if c not in pkeep]
        return False, "kept columns differ (parent only %s, run only %s%s)" % (
            only_p, only_r, ", order" if not only_p and not only_r else ""), len(prows), len(rrows)
    pidx = [ph.index(c) for c in pkeep]
    ridx = [rh.index(c) for c in rkeep]

    def kept(header, rows, ix, path):
        keep = row_filter(suffix, header, path, cut) if filtered else None
        out = []
        for n, row in enumerate(rows, start=2):
            if len(row) != len(header):
                raise CmpError("%s line %d: %d fields, header has %d" % (path, n, len(row), len(header)))
            if keep is None or keep(row, n):
                out.append((n, [row[i] for i in ix]))
        return out

    pk = kept(ph, prows, pidx, parent_path)
    rk = kept(rh, rrows, ridx, run_path)
    for (pn, pv), (rn, rv) in zip(pk, rk):
        if pv != rv:
            cols = [pkeep[i] for i in range(len(pv)) if pv[i] != rv[i]]
            return False, "first difference parent line %d / run line %d in %s: %s vs %s" % (
                pn, rn, cols[:6], [pv[pkeep.index(c)] for c in cols[:3]],
                [rv[pkeep.index(c)] for c in cols[:3]]), len(pk), len(rk)
    if len(pk) != len(rk):
        return False, "row count differs", len(pk), len(rk)
    return True, "", len(pk), len(rk)


def run(args, out=print, reader=read_text):
    ok_all = True
    lines = 0

    def emit(ok, text):
        nonlocal ok_all, lines
        ok_all = ok_all and ok
        lines += 1
        out(("PASS " if ok else "FAIL ") + text)

    for name in PAIR_RUNS:
        rec = end_line_of(os.path.join(args.parent, name + ".log"), reader)
        if rec is None:
            for suffix in SUFFIXES:
                emit(False, "%s%s.csv: no 'CosmicBalance end:' line in parent log %s" % (
                    name, suffix, os.path.join(args.parent, name + ".log")))
            continue
        kind, tick = rec[0], rec[1]
        source = "parent log"
        clock_parent = kind == "Clock" and tick == args.cut
        filtered = clock_parent and name not in ALWAYS_ALL
        scope = ("rows < %d" % args.cut) if filtered else "every row"
        for suffix in SUFFIXES:
            fname = name + suffix + ".csv"
            if filtered and suffix == "_claims":
                scope_here = "PlacedTick < %d and 0 <= EndTick < %d" % (args.cut, args.cut)
            else:
                scope_here = scope
            try:
                ok, msg, pn, rn = compare_pair(os.path.join(args.parent, fname),
                                               os.path.join(args.balance, fname),
                                               suffix, filtered, args.cut, reader)
            except CmpError as e:
                emit(False, "%s: %s" % (fname, e))
                continue
            emit(ok, "%s: rows %d/%d (%s; parent record %s@%d from %s; without %s)%s" % (
                fname, pn, rn, scope_here, kind, tick, source, "/".join(DROPPED),
                "" if ok else " - " + msg))

    for name in ERROR_LOGS:
        path = os.path.join(args.logs, name + ".log")
        if not os.path.isfile(path):
            emit(False, "%s.log: missing (%s)" % (name, path))
            continue
        bad = [ln.strip() for ln in reader(path).splitlines() if any(p in ln for p in ERROR_PATTERNS)]
        rec = end_line_of(path, reader)
        tail = "" if rec is None else "; first %s at %d; clock at %d" % rec
        if rec is None:
            emit(False, "%s.log: no 'CosmicBalance end:' line" % name)
        elif bad:
            emit(False, "%s.log: %d error line(s), first: %s" % (name, len(bad), bad[0][:300]))
        else:
            emit(True, "%s.log: no end-check, race-check or LogCosmic error line%s" % (name, tail))
        if rec is not None and name in ("checks_20000", "checks_ruler_20000", "checks_wanderer"):
            # Branch M only (cut < C, checked in parse_args): the wanderer run ends at cut, before C.
            want = -1 if name == "checks_wanderer" else args.clock
            emit(rec[2] == want, "%s.log: clock at %d (want %d)" % (name, rec[2], want))

    out("csv_cmp: %s (%d lines)" % ("PASS" if ok_all else "FAIL", lines))
    return 0 if ok_all else 1


def parse_args(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--parent", required=True, help="folder with the parent's CSVs and logs (parent_csv)")
    ap.add_argument("--balance", default=os.path.join("Saved", "Balance"), help="this run's CSV folder")
    ap.add_argument("--logs", default=os.path.join("Saved", "Logs"), help="this run's log folder")
    ap.add_argument("--cut", type=int, default=6000, help="the old clock tick (6000)")
    ap.add_argument("--clock", type=int, required=True,
                    help="branch-M value C (cut < C <= 18000): checks 'clock at' of the run logs")
    args = ap.parse_args(argv)
    if not (args.cut < args.clock <= 18000):
        ap.error("--clock must be the branch-M value C with %d < C <= 18000" % args.cut)
    return args


if __name__ == "__main__":
    sys.exit(run(parse_args(sys.argv[1:])))
