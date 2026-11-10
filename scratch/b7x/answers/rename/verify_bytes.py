"""Checks that rename_leaves.py changed only token lines: same line count, same CRLF count, same BOM, per changed file.
Usage: python -I -X utf8 verify_bytes.py <run log> <pristine> <renamed>"""
import os
import sys

LONG = "\\\\?\\"
log, pristine, renamed = sys.argv[1:4]
files = [l.split()[0] for l in open(log, encoding="utf-8") if l.startswith(("  Source/", "  Config/", "  Tools/"))]
total = 0
bad = 0
for f in files:
    a = open(LONG + os.path.abspath(os.path.join(pristine, *f.split("/"))), "rb").read()
    b = open(LONG + os.path.abspath(os.path.join(renamed, *f.split("/"))), "rb").read()
    la, lb = a.split(b"\n"), b.split(b"\n")
    ok = len(la) == len(lb) and a.count(b"\r\n") == b.count(b"\r\n") and a.count(b"\r") == b.count(b"\r") \
        and (a[:3] == b"\xef\xbb\xbf") == (b[:3] == b"\xef\xbb\xbf")
    changed = sum(1 for x, y in zip(la, lb) if x != y)
    total += changed
    bad += not ok
    print("%-62s lines changed %3d  CRLF %5d  BOM %-5s  ascii-only %-5s %s" % (f, changed, a.count(b"\r\n"),
          a[:3] == b"\xef\xbb\xbf", all(c < 128 for c in b), "ok" if ok else "MISMATCH"))
print("files %d, changed lines %d, mismatches %d" % (len(files), total, bad))
sys.exit(1 if bad else 0)
