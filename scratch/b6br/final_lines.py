"""[B-6br] section 4 (a)/(e) on the final runner's own logs (read only): the five balance logs print {BASE} letter for
letter (composed as the runner does), the two hash-run logs print the commandlet's seed line of checks / checks_ruler,
and every balance and hash-run log carries 0 'CosmicBalance end check:' / 'CosmicBalance race check:' /
'LogCosmic: Error'. The two test logs and GameSmoke.log get their error counts printed (record only).
usage: python -I final_lines.py <ROOT Saved/Logs dir> <seed42.json at HBR>"""
import json
import os
import re
import sys

log_dir, base_path = sys.argv[1], sys.argv[2]
base = json.load(open(base_path, encoding="utf-8"))
WROTE = re.compile(r", wrote .*?(?= \| |$)")
RUNS = [("checks", "summary_line", False, False), ("checks_ruler", "ruler_summary_line", True, False),
        ("checks_wanderer", "wanderer_summary_line", True, True), ("checks_20000", "summary_line_20000", False, False),
        ("checks_ruler_20000", "ruler_summary_line_20000", True, False)]
ERRS = ("CosmicBalance end check:", "CosmicBalance race check:", "LogCosmic: Error")


def last(text, pat):
    found = [m.group(1) for m in re.finditer(r"(" + re.escape(pat) + r"[^\r\n]*)", text)]
    return found[-1].rstrip() if found else None


def read(name):
    path = os.path.join(log_dir, name + ".log")
    if not os.path.exists(path):
        return None
    return open(path, encoding="utf-8", errors="replace").read()


bad = 0
seed = {}
for name, field, ruler, wanderer in RUNS:
    text = read(name)
    if text is None:
        print("MISSING", name)
        bad += 1
        continue
    line = last(text, "CosmicBalance: seed")
    m = re.search(r"CosmicBalance: (seed \d+, \d+ ticks, hash \d+, world \d+)", line or "")
    seed[name] = m.group(1) if m else None
    if line is not None:
        if ruler:
            line += " | " + (last(text, "CosmicBalance ruler:") or "<none>")
        if wanderer:
            line += " | " + (last(text, "CosmicBalance wanderer:") or "<none>")
        line = WROTE.sub("", line)
    errs = tuple(text.count(e) for e in ERRS)
    ok = line == WROTE.sub("", base[field]) and not any(errs)
    bad += 0 if ok else 1
    print("%s\t%s\terrors end/race/LogCosmic %s" % (name, "EQUAL" if line == WROTE.sub("", base[field]) else "DIFF", errs))
    print("   " + str(line))
    print("   " + (last(text, "CosmicBalance stages:") or "<no stages line>"))
for name, src in (("hashrun", "checks"), ("hashrun_policy", "checks_ruler")):
    text = read(name)
    if text is None:
        print("MISSING", name)
        bad += 1
        continue
    m = re.search(r"CosmicHash: (seed \d+, \d+ ticks, hash \d+, world \d+)", last(text, "CosmicHash: ") or "")
    got = m.group(1) if m else None
    errs = tuple(text.count(e) for e in ERRS)
    ok = got is not None and got == seed.get(src) and not any(errs)
    bad += 0 if ok else 1
    print("%s\t%s\terrors end/race/LogCosmic %s\n   %s" % (name, "EQUAL" if got == seed.get(src) else "DIFF", errs, got))
for name in ("Tests1", "Tests2", "GameSmoke"):
    text = read(name)
    if text is None:
        print("record", name, "missing")
        continue
    print("record %s\terrors end/race/LogCosmic %s" % (name, tuple(text.count(e) for e in ERRS)))
print("final_lines:", "PASS" if bad == 0 else "FAIL (%d)" % bad)
sys.exit(0 if bad == 0 else 1)
