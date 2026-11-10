"""[B-6br] brief section 6: the summary of the time gate re-measure (G1) from run_timing_ab.ps1's TSV.

Writes <out> (default: summary.md beside the TSV) and exits 0 only when every check holds:
  1. Lines: every run (warm-ups included) prints its expected line on both sides - {BASE} (Tools/baselines/seed42.json at
     8b6c5a7, ', wrote ...' removed) for the five gate runs, {OFF} for off_checks / off_checks_20000 - with exit 0 and
     no 'CosmicBalance end check:' / 'race check:' / 'LogCosmic: Error' line.
  2. Load control, per round: the after-sum / before-sum of the economy us/tick of the five gate runs lies within
     0.85-1.18, or the round is void (both sides, all seven runs). The per-pair economy ratio is recorded only.
  3. Enough rounds: each gate line has at least three valid after-runs. Otherwise it asks for the next round (4, then 5);
     after round 5 the session is invalid (R4).
  4. The gate: for each of the five gate runs the median of its valid after-runs < 16.05 us/tick.
Everything else is record only (brief section 6 'Record only').

--next-round prints only what the harness should do next: a round number, 'none' (enough valid rounds), or 'invalid'
(fewer than three valid rounds after round 5); exit 0. --one-sided (dry tests only) judges a TSV without a before side:
no round is voided and the before columns are empty.

Usage: python -I -X utf8 timing_summary.py --tsv <timing.tsv> [--out summary.md] [--base seed42_hbr.json]
                                           [--runner-out <final runner output>] [--next-round] [--one-sided]
Stdlib only.
"""
import argparse
import csv
import json
import os
import re
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
GATE = 16.05
BAND = (0.85, 1.18)
MAX_ROUND = 5
GATE_RUNS = ["checks", "checks_ruler", "checks_wanderer", "checks_20000", "checks_ruler_20000"]
OFF_RUNS = ["off_checks", "off_checks_20000"]
BASE_FIELDS = {"checks": "summary_line", "checks_ruler": "ruler_summary_line", "checks_wanderer": "wanderer_summary_line",
               "checks_20000": "summary_line_20000", "checks_ruler_20000": "ruler_summary_line_20000"}
OFF_LINES = {  # {OFF} of the brief (the [B-6b5] lines; section 0.5 found them unchanged at 8b6c5a7)
    "off_checks": "CosmicBalance: seed 42, 3000 ticks, hash 2816687688, world 3300228750",
    "off_checks_20000": "CosmicBalance: seed 42, 20000 ticks, hash 2359424635, world 726895141",
}
LEAD_QUIET = {"checks": 18.00, "checks_ruler": 17.27, "checks_wanderer": 20.06, "checks_20000": 22.96,
              "checks_ruler_20000": 19.65}
LABEL = {"checks": "input-less 3,000", "checks_ruler": "ruler 3,000", "checks_wanderer": "wanderer 6,000",
         "checks_20000": "input-less 20,000", "checks_ruler_20000": "ruler 20,000",
         "off_checks": "off 3,000", "off_checks_20000": "off 20,000"}
WROTE = re.compile(r", wrote .*?(?= \| |$)")
STAGES_OUT = re.compile(r"stages: AI ([0-9.]+) us, economy ([0-9.]+) us, crisis ([0-9.]+) us \(free memory ([0-9.]+) GB\)")


def num(v):
    try:
        return float(v)
    except (TypeError, ValueError):
        return None


def read_tsv(path):
    meta, body = [], []
    with open(path, encoding="utf-8-sig", newline="") as f:
        for line in f:
            if line.startswith("#"):
                meta.append(line[1:].strip())
            elif line.strip():
                body.append(line)
    rows = list(csv.DictReader(body, delimiter="\t"))
    for r in rows:
        r["round"] = int(r["round"])
        for k in ("ai_us", "economy_us", "crisis_us", "tick_mean_csv", "tick_mean_log", "free_gb", "cpu_pct", "wall_s"):
            r[k] = num(r.get(k))
    return meta, rows


def expected_lines(base_path):
    base = json.load(open(base_path, encoding="utf-8"))
    out = {name: WROTE.sub("", base[field]) for name, field in BASE_FIELDS.items()}
    out.update(OFF_LINES)
    return out


def fmt(v, nd=2):
    return "-" if v is None else ("%.*f" % (nd, v))


def med(vals):
    vals = [v for v in vals if v is not None]
    return statistics.median(vals) if vals else None


def analyse(rows, one_sided):
    rounds = sorted({r["round"] for r in rows if r["round"] >= 1})
    by = {}
    for r in rows:
        by[(r["side"], r["name"], r["round"])] = r
    round_info = {}
    for rd in rounds:
        sums = {}
        complete = True
        for side in ("before", "after"):
            vals = [by.get((side, n, rd), {}).get("economy_us") for n in GATE_RUNS]
            if any(v is None for v in vals):
                complete = complete and (one_sided and side == "before")
                sums[side] = None
            else:
                sums[side] = sum(vals)
        if one_sided:
            ratio = None
            void = sums["after"] is None
            why = "one-sided (no load control)" if not void else "after side incomplete"
        elif sums["before"] is None or sums["after"] is None:
            ratio, void, why = None, True, "a side is incomplete (a run is missing or has no stage line)"
        else:
            ratio = sums["after"] / sums["before"] if sums["before"] else None
            void = ratio is None or not (BAND[0] <= ratio <= BAND[1])
            why = "economy after/before %s outside %.2f-%.2f" % (fmt(ratio, 3), BAND[0], BAND[1]) if void else ""
        round_info[rd] = {"sums": sums, "ratio": ratio, "void": void, "why": why}
    valid_rounds = [rd for rd in rounds if not round_info[rd]["void"]]

    def valid_runs(side, name):
        out = []
        for rd in valid_rounds:
            r = by.get((side, name, rd))
            if r and r["ai_us"] is not None and r.get("exit") == "0":
                out.append(r)
        return out

    return rounds, by, round_info, valid_rounds, valid_runs


def next_round(rounds, valid_runs):
    short = [n for n in GATE_RUNS if len(valid_runs("after", n)) < 3]
    if not short:
        return "none", short
    last = max(rounds) if rounds else 0
    if last >= MAX_ROUND:
        return "invalid", short
    return str(last + 1), short


def runner_stage_lines(path):
    if not path:
        return None
    try:
        text = open(path, encoding="utf-8-sig", errors="replace").read()
    except OSError:
        return None
    return [tuple(float(x) for x in m.groups()) for m in STAGES_OUT.finditer(text)]


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--tsv", required=True)
    ap.add_argument("--out")
    ap.add_argument("--base", default=os.path.join(HERE, "seed42_hbr.json"))
    ap.add_argument("--runner-out")
    ap.add_argument("--next-round", action="store_true")
    ap.add_argument("--one-sided", action="store_true")
    args = ap.parse_args(argv)
    meta, rows = read_tsv(args.tsv)
    rounds, by, info, valid_rounds, valid_runs = analyse(rows, args.one_sided)
    nxt, short = next_round(rounds, valid_runs)
    if args.next_round:
        print(nxt)
        return 0

    expect = expected_lines(args.base)
    checks = []  # (ok, text)
    md = []
    w = md.append

    # 1. Lines
    bad_lines = []
    for r in rows:
        want = expect.get(r["name"])
        problems = []
        if r.get("exit") != "0":
            problems.append("exit %s" % r.get("exit"))
        if want is None:
            problems.append("unknown run name")
        elif r.get("summary") != want:
            problems.append("line differs: %s" % r.get("summary"))
        if r.get("errors") not in ("0/0/0", None, ""):
            problems.append("error lines end/race/LogCosmic %s" % r.get("errors"))
        if problems:
            bad_lines.append("%s %s r%d: %s" % (r["side"], r["name"], r["round"], "; ".join(problems)))
    checks.append((not bad_lines and bool(rows), "lines: %d runs print their expected line ({BASE}/{OFF}), exit 0, no error line%s" % (
        len(rows), "" if not bad_lines else " - %d bad" % len(bad_lines))))

    # 2./3. rounds
    voids = [rd for rd in rounds if info[rd]["void"]]
    enough = not short
    checks.append((enough, "rounds: %d run, %d void, %d valid; every gate line has >= 3 valid after-runs%s" % (
        len(rounds), len(voids), len(valid_rounds),
        "" if enough else " - short: %s; next: %s" % (", ".join(short), "round " + nxt if nxt.isdigit() else nxt))))

    # 4. gate
    after_med, before_med, after_max = {}, {}, {}
    gate_ok = True
    for n in GATE_RUNS + OFF_RUNS:
        a = [r["ai_us"] for r in valid_runs("after", n)]
        b = [r["ai_us"] for r in valid_runs("before", n)]
        after_med[n], before_med[n] = med(a), med(b)
        after_max[n] = max(a) if a else None
    gate_fail = [n for n in GATE_RUNS if after_med[n] is None or not (after_med[n] < GATE)]
    gate_ok = enough and not gate_fail
    checks.append((gate_ok, "gate: the median of the valid after-runs < %.2f us/tick on all five lines%s" % (
        GATE, "" if not gate_fail else " - not under: %s" % ", ".join(
            "%s %s" % (n, fmt(after_med[n])) for n in gate_fail))))
    below = [n for n in GATE_RUNS if before_med[n] is not None and before_med[n] < GATE]

    ok_all = all(c[0] for c in checks)
    w("# [B-6br] G1 re-measure - summary")
    w("")
    w("TSV: `%s`%s" % (args.tsv, " (one-sided: dry test)" if args.one_sided else ""))
    for m in meta:
        w("- %s" % m)
    w("")
    w("## Checks")
    w("")
    for ok, text in checks:
        w("- **%s** %s" % ("PASS" if ok else "FAIL", text))
    for b in bad_lines[:40]:
        w("  - %s" % b)
    if not enough:
        w("")
        w("**next: %s**" % ("round %s" % nxt if nxt.isdigit() else ("session invalid (R4): fewer than three valid rounds after round %d" % MAX_ROUND if nxt == "invalid" else nxt)))
    w("")
    w("verdict: **%s**" % ("PASS - G1 closes" if ok_all else "FAIL"))
    if below:
        w("")
        w("before_below_gate: %s" % ", ".join("%s %s" % (n, fmt(before_med[n])) for n in below))

    w("")
    w("## AI stage us/tick (median of valid runs; gate %.2f judged on the after side)" % GATE)
    w("")
    w("| run | before | after | after/before | after max | valid before/after | lead's quiet before | before vs lead |")
    w("|---|---|---|---|---|---|---|---|")
    for n in GATE_RUNS + OFF_RUNS:
        ratio = after_med[n] / before_med[n] if after_med[n] and before_med[n] else None
        lead = LEAD_QUIET.get(n)
        dev = (before_med[n] / lead - 1) if (lead and before_med[n]) else None
        note = "" if dev is None else ("%+.0f%%%s" % (dev * 100, " (more than 20% off)" if abs(dev) > 0.20 else ""))
        w("| %s | %s | %s | %s | %s | %d/%d | %s | %s |" % (
            LABEL[n], fmt(before_med[n]), fmt(after_med[n]), fmt(ratio, 3), fmt(after_max[n]),
            len(valid_runs("before", n)), len(valid_runs("after", n)), fmt(lead), note))
    singles = []
    for n in GATE_RUNS:
        if after_med[n] is None or not (after_med[n] < GATE):
            continue
        for r in valid_runs("after", n):
            if r["ai_us"] >= GATE:
                singles.append("%s r%d %s (round economy ratio %s)" % (n, r["round"], fmt(r["ai_us"]), fmt(info[r["round"]]["ratio"], 3)))
    if singles:
        w("")
        w("After-runs at or above %.2f with a passing median (recorded, not a fail): %s" % (GATE, "; ".join(singles)))

    w("")
    w("## AI / economy (median per run of the per-run ratio) and the other stages")
    w("")
    w("| run | AI/eco before | AI/eco after | ratio of ratios | economy before | economy after | crisis before | crisis after | tick mean CSV before | after | tick mean log before | after |")
    w("|---|---|---|---|---|---|---|---|---|---|---|---|")
    for n in GATE_RUNS + OFF_RUNS:
        rb = med([r["ai_us"] / r["economy_us"] for r in valid_runs("before", n) if r["economy_us"]])
        ra = med([r["ai_us"] / r["economy_us"] for r in valid_runs("after", n) if r["economy_us"]])
        rr = ra / rb if ra and rb else None
        cols = []
        for k in ("economy_us", "crisis_us", "tick_mean_csv", "tick_mean_log"):
            cols.append(fmt(med([r[k] for r in valid_runs("before", n)])))
            cols.append(fmt(med([r[k] for r in valid_runs("after", n)])))
        w("| %s | %s | %s | %s | %s |" % (LABEL[n], fmt(rb), fmt(ra), fmt(rr, 3), " | ".join(cols)))

    w("")
    w("## Rounds (load control: after-sum / before-sum of the five gate runs' economy us/tick within %.2f-%.2f)" % BAND)
    w("")
    w("| round | economy sum before | after | after/before | void | per-pair economy ratios (record only) |")
    w("|---|---|---|---|---|---|")
    for rd in rounds:
        i = info[rd]
        pairs = []
        for n in GATE_RUNS + OFF_RUNS:
            a, b = by.get(("after", n, rd)), by.get(("before", n, rd))
            if a and b and a["economy_us"] and b["economy_us"]:
                pairs.append("%s %.2f" % (n, a["economy_us"] / b["economy_us"]))
        w("| %d | %s | %s | %s | %s | %s |" % (rd, fmt(i["sums"].get("before")), fmt(i["sums"].get("after")), fmt(i["ratio"], 3),
                                          ("yes - " + i["why"]) if i["void"] else "no", ", ".join(pairs)))
    frees = [r["free_gb"] for r in rows if r["free_gb"] is not None]
    cpus = [r["cpu_pct"] for r in rows if r["cpu_pct"] is not None]
    w("")
    w("Free memory at the start of a run: %s-%s GB; CPU at the start: %s-%s %%; void rounds: %s." % (
        fmt(min(frees) if frees else None, 1), fmt(max(frees) if frees else None, 1), fmt(min(cpus) if cpus else None, 1),
        fmt(max(cpus) if cpus else None, 1),
        ", ".join("%d (%s)" % (rd, info[rd]["why"]) for rd in voids) or "none"))

    stages = runner_stage_lines(args.runner_out)
    w("")
    w("## The final runner's own serial lines (record only)")
    w("")
    if stages is None:
        w("not given (--runner-out)")
    elif not stages:
        w("no `stages:` line in %s" % args.runner_out)
    else:
        for n, s in zip(GATE_RUNS + ["(extra)"] * 10, stages):
            w("- %s: AI %s, economy %s, crisis %s us/tick, free memory %s GB" % (LABEL.get(n, n), fmt(s[0]), fmt(s[1]), fmt(s[2]), fmt(s[3], 1)))

    out = args.out or os.path.join(os.path.dirname(os.path.abspath(args.tsv)), "summary.md")
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(md) + "\n")
    for ok, text in checks:
        print(("PASS " if ok else "FAIL ") + text)
    if below:
        print("before_below_gate: %s" % ", ".join(below))
    if not enough:
        print("next: %s" % (("round " + nxt) if nxt.isdigit() else nxt))
    print("timing_summary: %s (%s)" % ("PASS" if ok_all else "FAIL", out))
    return 0 if ok_all else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
