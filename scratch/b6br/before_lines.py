"""[B-6br] section 0.5: the summary lines of the ten before runs, as the runner composes them (hash line, + ruler,
+ wanderer, joined by ' | ', without ', wrote ...'), compared with {BASE} (on) and the [B-6b5] lines (off).
usage: python -I before_lines.py <log dir> <seed42.json at HBR> [<out tsv>]"""
import json
import os
import re
import sys

log_dir, base_path = sys.argv[1], sys.argv[2]
out_path = sys.argv[3] if len(sys.argv) > 3 else None
base = json.load(open(base_path, encoding="utf-8"))
WROTE = re.compile(r", wrote .*?(?= \| |$)")
RUNS = [("checks", "summary_line", False, False), ("checks_ruler", "ruler_summary_line", True, False),
        ("checks_wanderer", "wanderer_summary_line", True, True), ("checks_20000", "summary_line_20000", False, False),
        ("checks_ruler_20000", "ruler_summary_line_20000", True, False)]
OFF_EXPECT = {"checks": (2816687688, 3300228750), "checks_ruler": (3561771757, 2462331303),
              "checks_wanderer": (2665013209, 2229239127), "checks_20000": (2359424635, 726895141),
              "checks_ruler_20000": (1611658162, 29194274)}


def last(text, pat):
    found = [m.group(1) for m in re.finditer(r"(" + re.escape(pat) + r"[^\r\n]*)", text)]
    return found[-1].rstrip() if found else None


def summary(path, ruler, wanderer):
    text = open(path, encoding="utf-8", errors="replace").read()
    line = last(text, "CosmicBalance: seed")
    if line is None:
        return None, text
    if ruler:
        line += " | " + (last(text, "CosmicBalance ruler:") or "<none>")
    if wanderer:
        line += " | " + (last(text, "CosmicBalance wanderer:") or "<none>")
    return line, text


bad = 0
rows = []
for off in (False, True):
    for name, field, ruler, wanderer in RUNS:
        tag = ("off_" if off else "") + name
        path = os.path.join(log_dir, tag + ".log")
        if not os.path.exists(path):
            print("MISSING", tag)
            bad += 1
            continue
        line, text = summary(path, ruler, wanderer)
        if line is None:
            print("NOLINE", tag)
            bad += 1
            continue
        line = WROTE.sub("", line)
        errs = (text.count("CosmicBalance end check:"), text.count("CosmicBalance race check:"), text.count("LogCosmic: Error"))
        stages = last(text, "CosmicBalance stages:") or ""
        if not off:
            ok = line == WROTE.sub("", base[field])
            verdict = "EQUAL" if ok else "DIFF"
            if not ok:
                bad += 1
        else:
            m = re.search(r"hash (\d+), world (\d+)", line)
            got = (int(m.group(1)), int(m.group(2))) if m else None
            verdict = "AS_B6B5" if got == OFF_EXPECT[name] else "DIFFERS_FROM_B6B5 (recorded, not a stop)"
        print(f"{tag}\t{verdict}\terrors end/race/LogCosmic {errs}")
        print(f"   {line}")
        print(f"   {stages}")
        if any(errs):
            bad += 1
        rows.append((tag, verdict, line, stages, errs))
if out_path:
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        for r in rows:
            f.write("\t".join([r[0], r[1], r[2], r[3], "%d/%d/%d" % r[4]]) + "\n")
print("before_lines:", "PASS" if bad == 0 else "FAIL (%d)" % bad)
sys.exit(0 if bad == 0 else 1)
