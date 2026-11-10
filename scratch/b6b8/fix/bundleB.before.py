"""[B-6b8] bundle B (brief scratchpad\\b6b8\\brief_final.md section 4; header H 5.2-5.3, notes 2-2.3): 540 commandlet runs of
CosmicBalance at 20,000 ticks without the clock - R, slowR, wanderer and input-less, each with its -WithoutB6bData=1 pair,
and the forced death cell (R + -DeathAt=<T_DEATH>, paired with R on) - the per-cell bridges, the switch and field
controls, the sha1 guard, the judgments of H 5.3 (1)-(4) with the frozen arithmetic of the brief's 4.3, and the
record-only readings.

A copy/extension of scratchpad\\b6b6\\bundleA.py: it imports that file (BUNDLE_A_PY, default ..\\b6b6\\bundleA.py) for the
command line, the process guards, the bundle A parser, the CSV pair reader, Wilson and the bundle A side summary.

Run with `python -I -X utf8 bundleB.py [options]`, the editor closed and nothing else building. Stdlib only.

  --t-death N             the forced-death tick (N section 2 "[B-6b6]의 다시 셈"); required for a run with the death cell
  --r X                   the committed AIUpperRungProgressMultiplier; the off logs' "(was %g)" must equal it (default: one
                          value across the off logs)
  --folder <dir>          output folder (default scratchpad\\b6b8\\bundleB - outside OneDrive, brief 4.2)
  --workers N             default 6; above 6 only if free memory covers them
  --dry-run               print the commands and the job count, run nothing, write nothing
  --check-only <dir>      re-read the outputs in <dir> and judge (no commandlet)
  --summary-dir <dir>     with --check-only: write summary.json/.md there instead of into the checked folder; required when
                          the checked folder is under --root or is an archived copy (it holds manifest.sha1); a --summary-dir
                          under --root or holding manifest.sha1 is refused ([B-6b9] review F4, back-ported). With --cell-set A:
                          where bundleA.analyse writes its summary (default scratchpad\\b6b8\\bundleA_check), same refusals
  --cell-set A            with --check-only: bundle A's cells and verdict (bundleA.analyse) plus this parser over its logs
  --cells, --max-seed, --seeds   a subset (a smoke run, the self-test, or [B-6b9]'s B' on seeds 61-120)
  --w-all-cells           the fall-back bridge min(T + 1, E, W) in every cell (brief 3.2, if L2 does not confirm)
  --archive / --no-archive    copy every .log and a sha1 manifest to <root>\\Saved\\Balance\\b6b_bundleB after the
                          judgments (default: on after a full run, off with --check-only; with --check-only an --archive
                          into a folder under --root or holding manifest.sha1 is refused - it would rewrite the archive)
  --a-sec-per-20k S       bundle A's wall seconds per 20,000-tick run, for <B_MINUTES> in the dry run
  --serial-logs <dir>     logs of the serial runner whose 'stages:' AI us is read for the +50% record

Formats: Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp at 529cab9 (git show) - the race line :3955-3993, the
ending line :3890, the off line :1596, the wanderer line :3738, the calling deaths line :3774, the CSV :1716, the claims
file :698. Arriving with [B-6b8] (brief 2.3) and read by name here: the side part's new layout "witness cards %d (first at
%lld, by the record %d; answered %d: by the check %d, failed %d, without a check %d; gone before the answer %d)"; the old
layout "(by the record %d; succeeded %d, failed %d, withdrawn %d)" is still read (bundle A). -DeathAt (B5 [B-5b]) and
slowR (-Focus=0) are existing switches.
"""
import argparse
import concurrent.futures
import datetime
import glob
import hashlib
import importlib.util
import json
import math
import os
import re
import shutil
import subprocess
import sys
import threading
import time

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:  # noqa: BLE001
    pass

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_BUNDLE_A_PY = os.path.join(os.path.dirname(HERE), "b6b6", "bundleA.py")


def _load_bundle_a(path):
    spec = importlib.util.spec_from_file_location("bundleA", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


A = _load_bundle_a(os.environ.get("BUNDLE_A_PY", DEFAULT_BUNDLE_A_PY))

DEFAULT_ROOT = A.DEFAULT_ROOT
EDITOR = A.EDITOR
HORIZON = 20000
COMMON_ARGS = list(A.COMMON_ARGS)                        # -Rules=ProvisionalEndTick:0 (H:325 "A·B는 시계 없이")
R_ARGS = list(A.R_ARGS)
SLOW_ARGS = ["-Focus=0" if a == "-Focus=1" else a for a in R_ARGS]
WANDERER_ARGS = R_ARGS + ["-Abdicate=1200", "-Wander=1"]
OFF_ARG = A.OFF_ARG
# Brief 4.1: the death cell takes R's arguments literally (no -Wander), and has no off side - it pairs with R on.
CELL_ORDER = ("R", "slowR", "wanderer", "inputless", "death")
CELL_ARGS = {"R": R_ARGS, "slowR": SLOW_ARGS, "wanderer": WANDERER_ARGS, "inputless": [], "death": R_ARGS}
HAS_OFF = {"R": True, "slowR": True, "wanderer": True, "inputless": True, "death": False}
EXPECTED_REMOVED = ("feat_passage_by_ark feat_laws_and_binding project_ark project_binding crisis_species_extinction edict_wide_machine_accord "
                    "edict_wide_quarantine edict_wide_warning event_era_endured enc_witness")
ENDING_KINDS = ("RuledPeople", "Vessel", "OtherPeople", "NoCandidate", "Clock", "None")   # Sim/CosmicEndings.cpp:85-111
FIFTH = ("RuledPeople", "OtherPeople")                   # Vessel is not a fifth-rung end (brief 4.3)
WINDOW = 600
ABDICATE_AT = 1200
BAND = (0.15, 0.35)
LADDER_R = (0.25, 0.3125, 0.375, 0.4375, 0.5)
AI_US_B6A = (10.4, 10.7)
NOT_MEASURED = "재지 않음 — 잰 %d"
ARK_UNMEASURED = "방주를 둔 채 만료 — 재지 않음 (카운터 없음)"

# ----------------------------------------------------------------------------------------------------------------- jobs


def parse_seeds(text):
    out = []
    for part in text.split(","):
        if "-" in part:
            a, b = part.split("-")
            out += list(range(int(a), int(b) + 1))
        elif part:
            out.append(int(part))
    return out


def build_jobs(folder, t_death=None, cells=None, max_seed=None, seeds=range(1, 61)):
    """Bundle B's jobs in cell order; each seed's on and off run next to each other; all at 20,000 ticks."""
    jobs = []
    for cell in CELL_ORDER:
        if cells and cell not in cells:
            continue
        for seed in seeds:
            if max_seed is not None and seed > max_seed:
                continue
            for off in ((False, True) if HAS_OFF[cell] else (False,)):
                name = "%s_%d%s" % (cell, seed, "_off" if off else "")
                base = os.path.join(folder, name)
                args = COMMON_ARGS + list(CELL_ARGS[cell])
                if cell == "death":
                    args.append("-DeathAt=%s" % (t_death if t_death is not None else "<T_DEATH>"))
                if off:
                    args.append(OFF_ARG)
                jobs.append({"name": name, "cell": cell, "seed": seed, "ticks": HORIZON, "off": off, "args": args,
                             "csv": base + ".csv", "rows": base + "_rows.csv", "claims": base + "_claims.csv", "log": base + ".log"})
    return jobs


def guarded_files(root):
    """Brief 4.2: every UnrealEditor-Cosmic_Ascension*.dll (Live Coding patch DLLs included), every Content/CA/Data/*.uasset,
    and Tools/data/generated.sha1."""
    files = glob.glob(os.path.join(root, "Binaries", "Win64", "UnrealEditor-Cosmic_Ascension*.dll"))
    files += glob.glob(os.path.join(root, "Content", "CA", "Data", "*.uasset"))
    stamp = os.path.join(root, "Tools", "data", "generated.sha1")
    files += [stamp] if os.path.exists(stamp) else []
    return sorted(files)

# -------------------------------------------------------------------------------------------------------------- parsing


RE_SIDE_NEW = re.compile(r"era lines (?P<era>-?\d+) \(by the record (?P<era_rec>-?\d+), first at (?P<era_first>-?\d+)\), witness cards (?P<wit>-?\d+) "
                         r"\(first at (?P<wit_first>-?\d+), by the record (?P<wit_rec>-?\d+); answered (?P<answered>-?\d+): by the check (?P<passed>-?\d+), "
                         r"failed (?P<failed>-?\d+), without a check (?P<nocheck>-?\d+); gone before the answer (?P<gone>-?\d+)\)")
RE_SIDE_OLD = re.compile(r"era lines (?P<era>-?\d+) \(by the record (?P<era_rec>-?\d+), first at (?P<era_first>-?\d+)\), witness cards (?P<wit>-?\d+) "
                         r"\(by the record (?P<wit_rec>-?\d+); succeeded (?P<succeeded>-?\d+), failed (?P<failed>-?\d+), withdrawn (?P<gone>-?\d+)\)")
RE_B_PARTS = {
    "binding": re.compile(r"binding at (?P<tick>-?\d+) by (?P<by>\S+) \((?P<civ>\S+) on (?P<system>[^,]+), ruled rung (?P<rung>-?\d+)\), completions (?P<completions>-?\d+) "
                          r"\(ticks with more than one (?P<more>-?\d+)\), ended without result (?P<no_result>-?\d+) \(first at (?P<no_result_first>-?\d+)\), "
                          r"locked (?P<locked>-?\d+) \(first at (?P<locked_first>-?\d+)\), retaken (?P<retaken>-?\d+) \(first at (?P<retaken_first>-?\d+)\), "
                          r"vessel left before T5 (?P<vessel_left>-?\d+)"),
    "p162": re.compile(r"#162 other order (?P<civ>\S+) shown (?P<shown>-?\d+) once ruled (?P<once>-?\d+)"),
    "laws": re.compile(r"laws first held ruled at (?P<ruled>-?\d+), AI at (?P<ai>-?\d+) \((?P<ai_civ>[^)]*)\), longest held ruled (?P<long_r>-?\d+) "
                       r"AI (?P<long_a>-?\d+), resets ruled (?P<resets_r>-?\d+) \(by upkeep (?P<upkeep_r>-?\d+)\) AI (?P<resets_a>-?\d+) "
                       r"\(by upkeep (?P<upkeep_a>-?\d+), held again in the tick (?P<held_again>-?\d+)\), ruled laws dropped at an input (?P<dropped>-?\d+)"),
    "decisions": re.compile(r"decisions \[(?P<list>[^\]]*)\], longest gap from ruled T3 to the record (?P<gap>-?\d+)"),
}
B_STRINGS = {"by", "civ", "system", "ai_civ", "list"}
RE_ENDING_HEAD = re.compile(r"record (?P<kind>\S+) at (?P<tick>-?\d+) \(people (?P<civ>[^,]*), a people some life ruled (?P<some>-?\d+), no vessel (?P<novessel>-?\d+)\)")
RE_OFF_KNOBS = re.compile(r"rows taken out \[[^\]]*\], AIUpperRungProgressMultiplier 1 \(was (?P<r>[^)]*)\), FleetDelayCountdownShare 0 \(was (?P<share>[^)]*)\)")
RE_WANDERER = re.compile(r"CosmicBalance wanderer: abdicate (?P<abdicate>-?\d+) wander (?P<wander>-?\d+); left at (?P<left>-?\d+)")
RE_DEATH = re.compile(r"CosmicBalance calling deaths: death asked at (?P<at>-?\d+) \(recorded (?P<recorded>-?\d+)\)")


def parse_side(text):
    m = RE_SIDE_NEW.search(text)
    if m:
        d = {k: int(v) for k, v in m.groupdict().items()}
        d["layout"] = "new"
        return d
    m = RE_SIDE_OLD.search(text)
    if m:
        d = {k: int(v) for k, v in m.groupdict().items()}
        d.update(layout="old", wit_first=None, answered=None, passed=None, nocheck=None)
        return d
    return None


def decisions_of(text):
    out = []
    for tok in text.split():
        t, n = tok.split(":")
        out.append((int(t), int(n)))
    return out


def parse_log(path):
    """bundleA.parse_log plus the parts bundle B reads: the side part (both layouts), binding, #162, laws, decisions; the
    ending's head (people, once ruled, no vessel); the off line's knobs; the wanderer's left at; the death asked."""
    p = A.parse_log(path)
    if not p.get("log_exists"):
        return p
    text = A.read_text(path)
    race_text = p.get("race_text") or ""
    p["race_b"] = {"side": parse_side(race_text)}
    for part, rx in RE_B_PARTS.items():
        m = rx.search(race_text)
        p["race_b"][part] = {k: (v if k in B_STRINGS else int(v)) for k, v in m.groupdict().items()} if m else None
    if p["race_b"]["decisions"]:
        p["race_b"]["decisions"]["pairs"] = decisions_of(p["race_b"]["decisions"]["list"])
    m = A.last_match(r"CosmicBalance ending: (.*?)\r?$", text)
    h = RE_ENDING_HEAD.search(m.group(1)) if m else None
    p["ending_x"] = {"kind": h.group("kind"), "tick": int(h.group("tick")), "civ": h.group("civ"), "some_life_ruled": int(h.group("some")),
                     "no_vessel": int(h.group("novessel"))} if h else None
    m = A.last_match(RE_OFF_KNOBS.pattern, text)
    p["off_knobs"] = {"r": m.group("r"), "share": m.group("share")} if m else None
    m = A.last_match(RE_WANDERER.pattern, text)
    p["left_at"] = int(m.group("left")) if m else None
    m = A.last_match(RE_DEATH.pattern, text)
    p["death"] = {"at": int(m.group("at")), "recorded": int(m.group("recorded"))} if m else None
    return p


def run_failures(job, parsed, exit_code=None, ignore_errors=()):
    """Brief 4.2: bundle A's rules (exit code, no seed/race line, any check line, any LogCosmic: Error, a missing file) plus
    the bundle B parts unreadable and an ending kind outside Sim/CosmicEndings.cpp's names."""
    why = A.run_failures(job, parsed, exit_code, ignore_errors)
    if not parsed.get("log_exists"):
        return why
    rb = parsed.get("race_b")
    if parsed.get("race_text") and rb:
        bad = [k for k, v in rb.items() if v is None]
        if bad:
            why.append("race line B parts unreadable: %s" % ", ".join(bad))
    if not parsed.get("ending_x"):
        why.append("no readable ending head (people, once ruled, no vessel)")
    elif parsed["ending_x"]["kind"] not in ENDING_KINDS:
        why.append("ending record kind %s outside %s" % (parsed["ending_x"]["kind"], "/".join(ENDING_KINDS)))
    return why

# ----------------------------------------------------------------------------------------------------------- pair check


def inf_if_neg(v):
    return math.inf if v is None or v < 0 else v


def bridge_of(job, parsed, use_w):
    """Brief 3.2: T_B = min(T + 1, E[, W]); T from the on run's ending line (bundleA.threshold_of), E the side part's era
    'first at', W its witness 'first at' (-1 = inf). W is None (unknown) in the old layout with witness cards > 0."""
    T = A.threshold_of(parsed.get("ending"), job["ticks"])
    side = (parsed.get("race_b") or {}).get("side")
    if T is None or side is None:
        return None, T, None, None
    E = inf_if_neg(side["era_first"])
    if side["layout"] == "old":
        W = math.inf if side["wit"] == 0 else None
    else:
        W = inf_if_neg(side["wit_first"])
    if use_w and W is None:
        return None, T, E, W
    tb = min(T + 1, E, W) if use_w else min(T + 1, E)
    return tb, T, E, W


def rows_equal_below(a_job, b_job, limit):
    """CSV and _rows.csv rows with Tick < limit equal (TickMicros, SaveBytes, SaveRawBytes aside); first differing ticks."""
    why, first = [], {}
    for key in ("csv", "rows"):
        if not (os.path.exists(a_job[key]) and os.path.exists(b_job[key])):
            why.append("missing %s file" % key)
            continue
        ha, ra = A.read_csv_by_tick(a_job[key])
        hb, rb = A.read_csv_by_tick(b_job[key])
        if ha != hb:
            why.append("%s headers differ" % key)
        d = A.first_difference(ra, rb)
        first[key] = d
        if d is not None and d < limit:
            why.append("%s differs at tick %d < %s" % (key, d, A.fmt(limit)))
    return why, first


def pair_check(on_job, off_job, on_p, off_p, use_w):
    res = {"pair": on_job["name"], "cell": on_job["cell"], "ok": False, "why": [], "T_B": None, "T": None, "E": None, "W": None, "first_diff": {}}
    tb, T, E, W = bridge_of(on_job, on_p, use_w)
    res.update(T_B=tb, T=T, E=E, W=W)
    if tb is None:
        res["why"].append("no bridge: %s" % ("on ending/side unreadable" if T is None or E is None else "W unknown (side part in the layout before [B-6b8])"))
        return res
    T_off = A.threshold_of(off_p.get("ending"), off_job["ticks"])
    if tb == T + 1 and T_off != T:
        # Below T + 1 the worlds are the same, so the off run reaches the same first third rung (bundle A's 'T differs').
        res["why"].append("T differs: on %s, off %s" % (T, T_off))
    why, first = rows_equal_below(on_job, off_job, tb)
    res["why"] += why
    res["first_diff"] = first
    res["ok"] = not res["why"]
    return res


def bridge_check(ref_job, job, limit, label):
    why, first = rows_equal_below(ref_job, job, limit)
    return {"pair": "%s ~ %s" % (job["name"], ref_job["name"]), "cell": job["cell"], "bridge": label, "limit": limit, "ok": not why, "why": why, "first_diff": first}

# ------------------------------------------------------------------------------------------------------------- controls


def csv_last_tick(path):
    if not os.path.exists(path):
        return None
    with open(path, "rb") as f:
        f.seek(0, 2)
        size = f.tell()
        f.seek(max(0, size - 65536))
        tail = f.read().decode("utf-8", errors="replace")
    lines = [l for l in tail.splitlines() if l.strip()]
    try:
        return int(lines[-1].split(",")[0]) if lines else None
    except ValueError:
        return None


def claims_first_placed(path):
    """The least PlacedTick of the claims file (None: no claim rows) - it lists every claim placed on any system
    (CosmicBalanceCommandlet.cpp:2719-2757, one row per placement)."""
    if not os.path.exists(path):
        return None
    lines = [l for l in A.read_text(path).splitlines() if l.strip()]
    if len(lines) < 2:
        return None
    ix = lines[0].split(",").index("PlacedTick")
    return min(int(l.split(",")[ix]) for l in lines[1:])


def g_text(v):
    return "%g" % v


def controls(jobs, parsed, results, t_death=None, r_expected=None):
    """Brief 4.2's switch and field controls. problems discard the bundle; notes do not."""
    problems, notes = [], []
    r_values = set()
    signal = False
    deaths = []
    for j in jobs:
        p = parsed.get(j["name"]) or {}
        if not p.get("log_exists") or not results.get(j["name"], {}).get("ok"):
            continue  # a twice-failed run is reported with the judgments (brief 4.2), not here
        name = j["name"]
        ex = p.get("ending_x")
        if ex and ex["kind"] == "Clock":
            problems.append("%s: record Clock - -Rules=ProvisionalEndTick:0 did not take" % name)
        last = csv_last_tick(j["csv"])
        if last != j["ticks"]:
            problems.append("%s: the CSV ends at %s, not %d" % (name, last, j["ticks"]))
        side = (p.get("race_b") or {}).get("side")
        race = p.get("race") or {}
        if j["off"]:
            if p.get("rows_taken_out") != EXPECTED_REMOVED or p.get("rows_taken_out_lines") != 1:
                problems.append("%s: rows taken out [%s] (%s lines), expected the ten" % (name, p.get("rows_taken_out"), p.get("rows_taken_out_lines")))
            k = p.get("off_knobs")
            if not k:
                problems.append("%s: no 'AIUpperRungProgressMultiplier 1 (was r), FleetDelayCountdownShare 0 (was 0)' tail" % name)
            else:
                r_values.add(k["r"])
                if k["share"] != "0":
                    problems.append("%s: FleetDelayCountdownShare was %s, not 0 (bundle B runs before [B-6b8b])" % (name, k["share"]))
                if r_expected is not None and k["r"] != g_text(r_expected):
                    problems.append("%s: AIUpperRungProgressMultiplier was %s, not %s" % (name, k["r"], g_text(r_expected)))
            ext, tr, t4 = race.get("extinction"), race.get("treaties"), race.get("ai_t4")
            if side and (side["era"] or side["wit"]):
                problems.append("%s: off run with era %d / witness %d" % (name, side["era"], side["wit"]))
            if ext and (ext["on_ruled"] or ext["on_ai"]):
                problems.append("%s: off run with extinctions %d/%d" % (name, ext["on_ruled"], ext["on_ai"]))
            if tr and tr["ai_treaties"]:
                problems.append("%s: off run with AI-step treaties %d" % (name, tr["ai_treaties"]))
            if t4 and t4["t4_1"] >= 0:
                problems.append("%s: off run with an AI T4 at %d" % (name, t4["t4_1"]))
            continue
        if p.get("rows_taken_out_lines"):
            problems.append("%s: an on log has a 'rows taken out' line" % name)
        if not side:
            continue
        if side["layout"] == "old":
            problems.append("%s: side part in the layout before [B-6b8] (no witness 'first at')" % name)
        if (side["era"] > 0) != (side["era_first"] >= 0) or (side["era"] > 0 and side["era_first"] < 600):
            problems.append("%s: era lines %d with first at %d (E >= 600 iff lines > 0)" % (name, side["era"], side["era_first"]))
        if side["era"] > 0 or side["wit"] > 0:
            signal = signal or j["cell"] != "death"
        if side["layout"] == "new":
            W = side["wit_first"]
            if (side["wit"] > 0) != (W >= 0):
                problems.append("%s: witness cards %d with first at %d (W >= 0 iff cards > 0)" % (name, side["wit"], W))
            if W >= 0 and j["cell"] == "wanderer" and (p.get("left_at") is None or p["left_at"] < 0 or W < p["left_at"]):
                problems.append("%s: witness first at %d before the wanderer left at %s" % (name, W, p.get("left_at")))
            if W >= 0:
                first_claim = claims_first_placed(j["claims"])
                if first_claim is None or W < first_claim:
                    problems.append("%s: witness first at %d before the first claim placed (%s)" % (name, W, first_claim))
        if j["cell"] == "death":
            d = p.get("death")
            if not d:
                problems.append("%s: no 'calling deaths: death asked at' line" % name)
            else:
                deaths.append((j["seed"], d["at"], d["recorded"]))
                if t_death is not None and d["at"] != t_death:
                    problems.append("%s: death asked at %d, not %d" % (name, d["at"], t_death))
    if r_expected is None and len(r_values) > 1:
        problems.append("off logs disagree on the committed r: %s" % sorted(r_values))
    if t_death is None and len({d[1] for d in deaths}) > 1:
        problems.append("death runs disagree on the asked tick: %s" % sorted({d[1] for d in deaths}))
    if not signal:
        notes.append("era 0 and witness 0 over every on-run: #156 and #141 '%s' (brief 7 risk 4 - not a stop)" % (NOT_MEASURED % 0))
    unrecorded = [s for s, _, r in deaths if r == 0]
    if unrecorded:
        notes.append("forced death recorded 0 (not a failure) in seeds %s" % unrecorded)
    return {"ok": not problems, "problems": problems, "notes": notes, "r_values": sorted(r_values), "deaths": deaths}

# ------------------------------------------------------------------------------------------------- frozen arithmetic


def lower_median(values):
    """MedianOf (CosmicBalanceCommandlet.cpp:765-774): the lower middle of an even count; None for none."""
    vals = sorted(values)
    return vals[(len(vals) - 1) // 2] if vals else None


def window_median(decisions, start, end):
    """Whole half-open windows [start + 600k, start + 600(k + 1)) whose end is <= end; per window the sum of n over the
    decision ticks in it; returns (lower median, window count) - (None, 0) for no whole window (counts as failing)."""
    k = (end - start) // WINDOW if end > start else 0
    if k <= 0:
        return None, 0
    counts = [0] * int(k)
    for t, n in decisions:
        if start <= t < start + WINDOW * k:
            counts[int((t - start) // WINDOW)] += n
    return lower_median(counts), int(k)


def wilson_share(k, n):
    lo, hi = A.wilson(k, n)
    return {"k": k, "n": n, "p": (k / n) if n else None, "lo": lo, "hi": hi,
            "text": ("%d/%d = %.3f [%.3f, %.3f]" % (k, n, k / n, lo, hi)) if n else "0/0"}

# ------------------------------------------------------------------------------------------------------------ judgments


def on_items(jobs, parsed, results, cell):
    """(job, parsed, ok) for every on job of a cell, in seed order."""
    out = []
    for j in sorted((j for j in jobs if j["cell"] == cell and not j["off"]), key=lambda j: j["seed"]):
        out.append((j, parsed.get(j["name"]) or {}, bool(results.get(j["name"], {}).get("ok"))))
    return out


def is_fifth(p):
    ex = p.get("ending_x")
    return bool(ex) and ex["kind"] in FIFTH and 0 <= ex["tick"] <= HORIZON


def judge_1(items):
    """(1) J1 = R runs whose record is RuledPeople or OtherPeople at 0 <= tick <= 20,000 (a twice-failed run: not reached);
    pass J1 >= 40/60 (scaled as 2/3 of n for a subset), judged at n >= 30."""
    n = len(items)
    reached = [j["seed"] for j, p, ok in items if ok and is_fifth(p)]
    k = len(reached)
    out = {"n": n, "J1": k, "share": wilson_share(k, n), "reached": reached, "failed": [j["seed"] for j, _, ok in items if not ok]}
    if n < 30:
        out.update(judged=False, passed=None, text=NOT_MEASURED % n)
    else:
        out.update(judged=True, passed=k * 60 >= 40 * n, text="%d/%d %s 40/60" % (k, n, ">=" if k * 60 >= 40 * n else "<"))
    return out


def split_of(p):
    """(2)'s split of one numerator run: (가) the binder is not the ruled people and ruled rung >= 4; (나) the ruled people
    erased by an extinction, or its T4 -1 or after the binding; (다) every other (ruled rung 0 - interregnum or a dead
    vessel - lands here unless an extinction erased the ruled people)."""
    b = (p.get("race_b") or {}).get("binding") or {}
    race = p.get("race") or {}
    ru, ext = race.get("ruled") or {}, race.get("extinction") or {}
    fields = {"binding": b.get("tick"), "by": b.get("by"), "binder": b.get("civ"), "rung": b.get("rung"), "ruled T4": ru.get("r4"),
              "erased ruled": ext.get("erased_ruled"), "record people": (p.get("ending_x") or {}).get("civ")}
    if b and b.get("tick", -1) >= 0 and b.get("by") != "ruled" and b.get("rung", 0) >= 4:
        return "ga", fields
    if ext.get("erased_ruled", 0) > 0:
        return "na", fields
    if not b or b.get("rung", 0) == 0 or b.get("tick", -1) < 0:
        return "da", fields
    if ru.get("r4", -1) < 0 or ru.get("r4", -1) > b["tick"]:
        return "na", fields
    return "da", fields


def judge_2(items):
    """(2) n2 = (1)'s runs; J2 = OtherPeople with 'a people some life ruled 0'; pass: the point in 15-35%. Once-ruled others
    stay in the denominator and are reported apart. The (가)/(나)/(다) split of J2's numerator, and [B-6b9]'s reading of
    H:343 (r moves one step only if (가)'s share alone lies outside the band on the same side)."""
    base = [(j, p) for j, p, ok in items if ok and is_fifth(p)]
    n2 = len(base)
    num = [(j, p) for j, p in base if p["ending_x"]["kind"] == "OtherPeople" and p["ending_x"]["some_life_ruled"] == 0]
    once = [j["seed"] for j, p in base if p["ending_x"]["kind"] == "OtherPeople" and p["ending_x"]["some_life_ruled"] != 0]
    sh = wilson_share(len(num), n2)
    split = {"ga": [], "na": [], "da": []}
    ga_record = 0
    for j, p in num:
        key, fields = split_of(p)
        split[key].append(dict(fields, seed=j["seed"]))
        if key == "ga" and fields["binder"] == fields["record people"]:
            ga_record += 1
    out = {"n2": n2, "J2": len(num), "share": sh, "once_ruled": once, "split": split, "ga_binder_is_record_people": ga_record,
           "wholly_outside": sh["n"] > 0 and (sh["lo"] > BAND[1] or sh["hi"] < BAND[0])}
    if n2 < 30:
        out.update(judged=False, passed=None, side=None, text=NOT_MEASURED % n2)
        return out
    p = sh["p"]
    side = "above" if p > BAND[1] else ("below" if p < BAND[0] else None)
    out.update(judged=True, passed=side is None, side=side, text="%s %s 15-35%%" % (sh["text"], "inside" if side is None else side))
    ga_share = len(split["ga"]) / n2
    ga_side = "above" if ga_share > BAND[1] else ("below" if ga_share < BAND[0] else None)
    if side is None:
        reading = "-"
    elif ga_side == side:
        reading = "r one ladder step %s, then B' on seeds 61-120" % ("down" if side == "above" else "up")
    else:
        reading = "[개발자] with the split ((가) alone %.3f is not outside on the same side)" % ga_share
    out["b6b9_reading"] = {"ga_share": ga_share, "ga_side": ga_side, "reading": reading}
    return out


def judge_2_slow(items):
    out = judge_2(items)
    out["show_developer"] = out["share"]["n"] > 0 and out["share"]["lo"] > BAND[1]
    out["passed"] = None   # report only
    return out


def judge_3(items):
    """(3) n3 = (1)'s runs with record RuledPeople, record people == RuledCivID[1] == RuledCivID[2], r5 <= record tick <=
    20,000. Length: lower median of r5 - r4 <= 3,600. Density: whole 600 windows from r4 ending <= r5, decisions per window,
    per-run lower median >= 1 in >= 90% of n3 (0 windows fails). r5 - r4 >= 1,200 asserted in every run (reported)."""
    n3_runs, apart = [], []
    for j, p, ok in items:
        race = p.get("race") or {}
        ru = race.get("ruled")
        if not ok or not ru or ru["r5"] < 0:
            continue
        ex = p["ending_x"]
        reasons = []
        if not is_fifth(p):
            reasons.append("no fifth-rung record")
        elif ex["kind"] != "RuledPeople":
            reasons.append("record %s" % ex["kind"])
        if not (ex["civ"] == ru["r4_civ"] == ru["r5_civ"]):
            reasons.append("다른 재위 (record %s, T4 %s, T5 %s)" % (ex["civ"], ru["r4_civ"], ru["r5_civ"]))
        if ex["tick"] < ru["r5"]:
            reasons.append("record before r5 (%d < %d)" % (ex["tick"], ru["r5"]))
        if ex["tick"] > HORIZON:
            reasons.append("record after 20,000")
        if ru["r4"] < 0:
            reasons.append("no ruled T4")
        if reasons:
            apart.append({"seed": j["seed"], "why": "; ".join(reasons)})
            continue
        decisions = ((p.get("race_b") or {}).get("decisions") or {}).get("pairs", [])
        med, windows = window_median(decisions, ru["r4"], ru["r5"])
        n3_runs.append({"seed": j["seed"], "length": ru["r5"] - ru["r4"], "median": med, "windows": windows})
    n3 = len(n3_runs)
    length = lower_median([r["length"] for r in n3_runs])
    dense = sum(1 for r in n3_runs if r["windows"] > 0 and r["median"] >= 1)
    out = {"n3": n3, "runs": n3_runs, "apart": apart, "length": length, "dense": dense, "zero_windows": [r["seed"] for r in n3_runs if r["windows"] == 0],
           "violations": [r for r in n3_runs if r["length"] < 1200], "density": wilson_share(dense, n3)}
    if n3 < 30:
        out.update(judged=False, passed=None, length_ok=None, density_ok=None, text=NOT_MEASURED % n3)
        return out
    out["length_ok"] = length <= 3600
    out["density_ok"] = dense * 10 >= 9 * n3
    out.update(judged=True, passed=out["length_ok"] and out["density_ok"],
               text="length %s %s 3,600; density %d/%d %s 90%%" % (length, "<=" if out["length_ok"] else ">", dense, n3, ">=" if out["density_ok"] else "<"))
    return out


def judge_4(items):
    """(4) slowR: E_i = the record tick if RuledPeople/OtherPeople (<= 20,000), else inf (twice-failed runs excluded). C0 =
    the 48th of 60 in ascending order (nearest rank ceil(0.8 n)) rounded up to 1,000 (inf stays); C = min(18,000,
    max(6,000, C0)). (i) #{E <= C} >= 30. (ii) from ruled T3 to end = min(record tick of any kind, C): whole 600 windows,
    per-run lower median >= 1 in >= 90% of the runs whose ruled T3 < end (0 windows fails). Fewer than 30 such runs ->
    '재지 않음' and no C."""
    good = [(j, p) for j, p, ok in items if ok]
    n = len(good)
    E = []
    for j, p in good:
        E.append((j["seed"], p["ending_x"]["tick"] if is_fifth(p) else math.inf))
    Es = sorted(e for _, e in E)
    out = {"n": n, "E": E, "excluded": [j["seed"] for j, p, ok in items if not ok]}
    if n < 30:
        out.update(judged=False, passed=None, C0=None, C=None, text=NOT_MEASURED % n)
        return out
    rank = int(math.ceil(0.8 * n))
    e48 = Es[rank - 1]
    C0 = math.inf if math.isinf(e48) else int(math.ceil(e48 / 1000.0)) * 1000
    C = 18000 if math.isinf(C0) else min(18000, max(6000, C0))
    within = sum(1 for e in Es if e <= C)
    runs, not_reached = [], []
    for j, p in good:
        ru = (p.get("race") or {}).get("ruled") or {}
        rec = p["ending_x"]["tick"]
        end = min(rec, C) if rec >= 0 else C
        r3 = ru.get("r3", -1)
        if r3 < 0 or r3 >= end:
            not_reached.append(j["seed"])
            continue
        decisions = ((p.get("race_b") or {}).get("decisions") or {}).get("pairs", [])
        med, windows = window_median(decisions, r3, end)
        runs.append({"seed": j["seed"], "r3": r3, "end": end, "median": med, "windows": windows})
    dense = sum(1 for r in runs if r["windows"] > 0 and r["median"] >= 1)
    out.update(rank=rank, E48=e48, C0=C0, C=C, i_count=within, i_ok=within >= 30, ii_runs=runs, ii_not_reached=not_reached,
               ii_dense=dense, ii_zero_windows=[r["seed"] for r in runs if r["windows"] == 0])
    if len(runs) < 30:
        out.update(judged=False, passed=None, ii_ok=None, C=None, text="(ii) %s - no C" % (NOT_MEASURED % len(runs)))
        return out
    out["ii_ok"] = dense * 10 >= 9 * len(runs)
    out.update(judged=True, passed=out["i_ok"] and out["ii_ok"],
               text="C0 %s, C %s; (i) %d %s 30; (ii) %d/%d %s 90%%" % (A.fmt(C0), C, within, ">=" if out["i_ok"] else "<", dense, len(runs),
                                                                    ">=" if out["ii_ok"] else "<"))
    if not out["passed"]:
        out["text"] += " - no C, [개발자] with the numbers"
    return out


def judgments(jobs, parsed, results):
    """H 5.3 (1)-(4) and slowR (2); withheld (computed, flagged) when a cell has two or more twice-failed runs."""
    failed = {}
    for j in jobs:
        if not results.get(j["name"], {}).get("ok"):
            failed.setdefault(j["cell"], []).append(j["name"])
    R = on_items(jobs, parsed, results, "R")
    S = on_items(jobs, parsed, results, "slowR")
    return {"1": judge_1(R), "2": judge_2(R), "2slow": judge_2_slow(S), "3": judge_3(R), "4": judge_4(S),
            "failed": failed, "withheld": sorted(c for c, v in failed.items() if len(v) >= 2)}

# --------------------------------------------------------------------------------------------------------- record only


def sign_test(L, G):
    n = L + G
    if n == 0:
        return 1.0
    k = min(L, G)
    tail = sum(math.comb(n, i) for i in range(k + 1)) / 2.0 ** n
    return min(1.0, 2 * tail)


def vessel_left_tier(job, p):
    """The CSV Tier of the people the vessel ruled when it left (the ruled row - empty Expansion - of the last sample at or
    before 'left at'), at the first sample at or after it."""
    left = p.get("left_at")
    if left is None or left < 0 or not os.path.exists(job["csv"]):
        return None
    data = A.civ_track(job["csv"])
    before = [t for t in data["ruled"] if t <= left]
    if not before:
        return None
    civ = data["ruled"][max(before)]
    row = next((r for r in data["track"].get(civ, []) if r[0] >= left), None)
    return {"civ": civ, "tier": row[1] if row else None, "returns": ((p.get("race") or {}).get("crown") or {}).get("returns")}


def cell_extras(items):
    """The bundle B parts over one cell side: binding, #162, #156 era, #141 witness, record kinds, laws."""
    ps = [p for _, p in items]
    rb = [p.get("race_b") or {} for p in ps]
    b = [x["binding"] for x in rb if x.get("binding")]
    side = [x["side"] for x in rb if x.get("side")]
    laws = [x["laws"] for x in rb if x.get("laws")]
    kinds = {}
    for p in ps:
        k = (p.get("ending_x") or {}).get("kind", "?")
        kinds[k] = kinds.get(k, 0) + 1
    n = len(ps)
    other = [x["p162"] for x in rb if x.get("p162") and x["p162"]["civ"] != "-"]
    pos = lambda vals: [v for v in vals if v is not None and v >= 0]  # noqa: E731
    out = {
        "record kinds": {k: wilson_share(v, n)["text"] for k, v in sorted(kinds.items())},
        "binding": {"bound": wilson_share(sum(1 for x in b if x["tick"] >= 0), n)["text"], "tick": A.dist(pos([x["tick"] for x in b]))["text"],
                    "by": {k: sum(1 for x in b if x["tick"] >= 0 and x["by"] == k) for k in sorted({x["by"] for x in b if x["tick"] >= 0})},
                    "ended without result": sum(x["no_result"] for x in b), "locked": sum(x["locked"] for x in b), "retaken": sum(x["retaken"] for x in b),
                    "vessel left before T5": sum(x["vessel_left"] for x in b)},
        "#162": {"other-order ends": len(other), "shown": sum(1 for x in other if x["shown"] == 1), "once ruled": sum(1 for x in other if x["once"] == 1)},
        "#156 era": {"runs": wilson_share(sum(1 for s in side if s["era"] > 0), n)["text"], "lines": sum(s["era"] for s in side),
                     "by the record": sum(s["era_rec"] for s in side), "first": A.dist(pos([s["era_first"] for s in side]))["text"]},
        "#141 witness": {"runs": wilson_share(sum(1 for s in side if s["wit"] > 0), n)["text"], "came": sum(s["wit"] for s in side),
                         "by the record": sum(s["wit_rec"] for s in side), "first": A.dist(pos([s.get("wit_first") for s in side]))["text"],
                         "passed the check": sum(s.get("passed") or 0 for s in side), "failed": sum(s["failed"] for s in side),
                         "without a check": sum(s.get("nocheck") or 0 for s in side), "gone before the answer": sum(s["gone"] for s in side)},
        "laws": {"first held ruled": A.dist(pos([x["ruled"] for x in laws]))["text"], "first held AI": A.dist(pos([x["ai"] for x in laws]))["text"],
                 "resets ruled by upkeep": sum(x["upkeep_r"] for x in laws), "resets AI by upkeep": sum(x["upkeep_a"] for x in laws),
                 "held again in the tick": sum(x["held_again"] for x in laws), "ruled laws dropped at an input": sum(x["dropped"] for x in laws)},
        "extinction": ARK_UNMEASURED,
    }
    return out


def per_pair(jobs, parsed, results, cell):
    """(on, off, on - off) of per-run quantities over the cell's passing pairs."""
    by = {j["name"]: j for j in jobs}
    keys = {"T4 civs most": lambda p: (p.get("race") or {}).get("t4civs", {}).get("most"),
            "extinctions placed": lambda p: (lambda e: e["on_ruled"] + e["on_ai"] if e else None)((p.get("race") or {}).get("extinction")),
            "era lines": lambda p: ((p.get("race_b") or {}).get("side") or {}).get("era"),
            "witness cards": lambda p: ((p.get("race_b") or {}).get("side") or {}).get("wit"),
            "claims": lambda p: (p.get("race") or {}).get("claims", {}).get("claims"),
            "AI us": lambda p: p.get("ai_us")}
    out = {}
    for label, f in keys.items():
        on, off, diff = [], [], []
        for j in jobs:
            if j["cell"] != cell or j["off"]:
                continue
            o = by.get(j["name"] + "_off")
            if not o or not (results.get(j["name"], {}).get("ok") and results.get(o["name"], {}).get("ok")):
                continue
            a, b = f(parsed[j["name"]]), f(parsed[o["name"]])
            if a is None or b is None:
                continue
            on.append(a)
            off.append(b)
            diff.append(a - b)
        out[label] = {"on": A.dist(on)["text"], "off": A.dist(off)["text"], "on-off": A.dist(diff)["text"]}
    return out


def record_only(jobs, parsed, results, serial_ai_us=None):
    out = {"cells": {}, "pairs": {}}
    for cell in CELL_ORDER:
        sides = {}
        for side, off in (("on", False), ("off", True)):
            items = [(j, parsed[j["name"]]) for j in jobs if j["cell"] == cell and j["off"] == off and results.get(j["name"], {}).get("ok")]
            if not items:
                continue
            sides[side] = {"a": A.side_summary(items), "b": cell_extras(items)}
        if sides:
            out["cells"][cell] = sides
        if HAS_OFF[cell]:
            out["pairs"][cell] = per_pair(jobs, parsed, results, cell)
    # Locks: R runs with locked >= 1 and no fifth-rung record; >= 3 -> #165 to [개발자].
    locks = []
    for j, p, ok in on_items(jobs, parsed, results, "R"):
        b = (p.get("race_b") or {}).get("binding")
        if ok and b and b["locked"] >= 1 and not is_fifth(p):
            locks.append(j["seed"])
    out["locks"] = {"runs": locks, "count": len(locks), "flag_165": len(locks) >= 3}
    # Vessels that left a people standing at T4+ (wanderer and R cells; the #96 line).
    left = []
    for cell in ("wanderer", "R"):
        for j, p, ok in on_items(jobs, parsed, results, cell):
            if ok:
                v = vessel_left_tier(j, p)
                if v and v["tier"] is not None and v["tier"] >= 4:
                    left.append(dict(v, seed=j["seed"], cell=cell, left=p["left_at"]))
    out["vessels left at T4+"] = left
    # Preview only: the B1 guard on wanderer pairs cut at 6,000 (bundle C's reading), on against off.
    by = {j["name"]: j for j in jobs}
    on_k = off_k = L = G = n = 0
    ticks_on, ticks_off = [], []
    for j in jobs:
        if j["cell"] != "wanderer" or j["off"]:
            continue
        o = by.get(j["name"] + "_off")
        if not o or not (results.get(j["name"], {}).get("ok") and results.get(o["name"], {}).get("ok")):
            continue
        c_on, c_off = parsed[j["name"]].get("crowned") or {}, parsed[o["name"]].get("crowned") or {}
        a = 0 <= c_on.get("tick", -1) <= 6000
        b = 0 <= c_off.get("tick", -1) <= 6000
        n += 1
        on_k += a
        off_k += b
        L += (not a) and b
        G += a and not b
        if a:
            ticks_on.append(c_on["tick"])
        if b:
            ticks_off.append(c_off["tick"])
    out["B1 preview"] = {"pairs": n, "on": wilson_share(on_k, n)["text"], "off": wilson_share(off_k, n)["text"], "L (only off)": L, "G (only on)": G,
                         "sign test p": sign_test(L, G), "median on": lower_median(ticks_on), "median off": lower_median(ticks_off)}
    # AI-stage us: the bundle's (parallel) and the serial runner's, against +50% of B6a's 10.4-10.7.
    us = [p["ai_us"] for j in jobs for p in [parsed.get(j["name"]) or {}] if p.get("ai_us") is not None and not j["off"]]
    out["AI us"] = {"bundle on (parallel)": A.dist(us)["text"], "threshold": 1.5 * AI_US_B6A[1], "threshold low": 1.5 * AI_US_B6A[0],
                    "serial": serial_ai_us, "serial above": (max(serial_ai_us) > 1.5 * AI_US_B6A[1]) if serial_ai_us else None}
    # Death cell against R on, per seed: the record kind moved by the death.
    moved = []
    for j, p, ok in on_items(jobs, parsed, results, "death"):
        r = by.get("R_%d" % j["seed"])
        if ok and r and results.get(r["name"], {}).get("ok"):
            a, b = (p.get("ending_x") or {}), (parsed[r["name"]].get("ending_x") or {})
            if (a.get("kind"), a.get("tick")) != (b.get("kind"), b.get("tick")):
                moved.append("%d: %s@%s -> %s@%s" % (j["seed"], b.get("kind"), b.get("tick"), a.get("kind"), a.get("tick")))
    out["death vs R on"] = moved
    return out


def serial_ai_us(folder):
    if not folder or not os.path.isdir(folder):
        return None
    vals = []
    for f in sorted(glob.glob(os.path.join(folder, "*.log"))):
        p = A.parse_log(f)
        if p.get("ai_us") is not None:
            vals.append(p["ai_us"])
    return vals or None

# ------------------------------------------------------------------------------------------------------------ analysis


def analyse(jobs, parsed, results, args_t_death, r_expected, use_w_all, serial_us=None):
    by = {j["name"]: j for j in jobs}
    pairs, bridges = [], []
    for j in jobs:
        if j["off"] or not HAS_OFF[j["cell"]]:
            continue
        o = by.get(j["name"] + "_off")
        if not o:
            continue
        if not (results.get(j["name"], {}).get("ok") and results.get(o["name"], {}).get("ok")):
            pairs.append({"pair": j["name"], "cell": j["cell"], "ok": None, "why": ["skipped: a run of the pair failed twice"]})
            continue
        pairs.append(pair_check(j, o, parsed[j["name"]], parsed[o["name"]], use_w_all or j["cell"] == "wanderer"))
    for j in jobs:
        if j["off"] or j["cell"] not in ("death", "wanderer"):
            continue
        r = by.get("R_%d" % j["seed"])
        if not r:
            continue
        limit = args_t_death if j["cell"] == "death" else ABDICATE_AT
        if j["cell"] == "death" and limit is None:
            d = (parsed[j["name"]].get("death") or {}).get("at")
            limit = d if d is not None and d >= 0 else None
        if limit is None or not (results.get(j["name"], {}).get("ok") and results.get(r["name"], {}).get("ok")):
            bridges.append({"pair": "%s ~ %s" % (j["name"], r["name"]), "cell": j["cell"], "ok": None, "why": ["skipped: a run failed or no T_DEATH"]})
            continue
        bridges.append(bridge_check(r, j, limit, "death < T_DEATH" if j["cell"] == "death" else "wanderer < 1,200"))
    ctl = controls(jobs, parsed, results, args_t_death, r_expected)
    jd = judgments(jobs, parsed, results)
    rec = record_only(jobs, parsed, results, serial_us)
    return {"pairs": pairs, "bridges": bridges, "controls": ctl, "judgments": jd, "record": rec}


def placeholders(res, header):
    jd = res["judgments"]
    j1, j2, j3, j4 = jd["1"], jd["2"], jd["3"], jd["4"]
    return {
        "<J1>": "%d/%d (%s)" % (j1["J1"], j1["n"], j1["text"]),
        "<J2>": "%s; (가) %d (binder = record people %d), (나) %d, (다) %d; once-ruled %d" % (
            j2["share"]["text"], len(j2["split"]["ga"]), j2["ga_binder_is_record_people"], len(j2["split"]["na"]), len(j2["split"]["da"]), len(j2["once_ruled"])),
        "<J3>": "%s; violations %d; zero windows %d; apart %d" % (j3["text"], len(j3["violations"]), len(j3["zero_windows"]), len(j3["apart"])),
        "<E_1..E_60>": " ".join("%d:%s" % (s, A.fmt(e)) for s, e in j4["E"]),
        "<C0>": A.fmt(j4.get("C0")), "<C>": A.fmt(j4.get("C")) if j4.get("passed") else "없음 (%s)" % j4["text"],
        "<(i)/(ii)>": "(i) %s (ii) %s" % (j4.get("i_ok"), j4.get("ii_ok")),
        "<B_LOCKED>": "%d (%s)" % (res["record"]["locks"]["count"], res["record"]["locks"]["runs"]),
        "<B_AI_US>": res["record"]["AI us"]["bundle on (parallel)"] + ("; serial %s" % res["record"]["AI us"]["serial"] if res["record"]["AI us"]["serial"] else ""),
        "<B_MINUTES>": header.get("minutes", "-"),
    }


def verdict(res, header):
    pairs_ok = all(p["ok"] is not False for p in res["pairs"]) and all(b["ok"] is not False for b in res["bridges"])
    return bool(header.get("sha1_ok", True)) and res["controls"]["ok"] and pairs_ok and header.get("end_check", 0) == 0 and header.get("race_check", 0) == 0

# ------------------------------------------------------------------------------------------------------------ markdown


def yn(v):
    return {True: "통과", False: "어긋남", None: "재지 않음"}[v]


def markdown(res, header):
    L = []
    jd = res["judgments"]
    L.append("# [B-6b8] 묶음 B — %s" % header["date"])
    L.append("")
    L.append("- 빌드: %s; DLL sha1 %s" % (header.get("git"), header.get("dll_sha1")))
    L.append("- 프로세스: 묶음 %s회(작업 %d, 재실행 %s) — ⓐ·base·on 15회는 리드가 더한다(총 555)" % (header.get("processes"), header["jobs"], header.get("reruns")))
    L.append("- 두 번 실패: %s%s" % (jd["failed"] or "없음", " — 칸마다 둘 이상: %s → 판단 전에 [개발자]" % jd["withheld"] if jd["withheld"] else ""))
    L.append("- 폴더: `%s`%s" % (header["folder"], "; 사본 `%s`" % header["archive"] if header.get("archive") else ""))
    L.append("- check 줄: end check %s, race check %s" % (header.get("end_check"), header.get("race_check")))
    c = res["controls"]
    L.append("- 스위치·필드 대조: %s%s" % ("통과" if c["ok"] else "어긋남", "" if c["ok"] else " — " + "; ".join(c["problems"][:12])))
    for n in c["notes"]:
        L.append("  - %s" % n)
    L.append("  - 끈 쪽의 r: %s" % c["r_values"])
    bad_pairs = [p for p in res["pairs"] if p["ok"] is False]
    skipped = [p for p in res["pairs"] + res["bridges"] if p["ok"] is None]
    L.append("- 짝(Tick < T_B): %d/%d 통과%s" % (sum(1 for p in res["pairs"] if p["ok"]), len(res["pairs"]),
                                          "" if not bad_pairs else " — 어긋남: " + "; ".join("%s (%s)" % (p["pair"], ", ".join(p["why"])) for p in bad_pairs[:10])))
    bad_b = [b for b in res["bridges"] if b["ok"] is False]
    L.append("- 다리(강제 죽음 < T_DEATH, 방랑자 < 1,200): %d/%d 통과%s" % (sum(1 for b in res["bridges"] if b["ok"]), len(res["bridges"]),
                                                               "" if not bad_b else " — 어긋남: " + "; ".join("%s (%s)" % (b["pair"], ", ".join(b["why"])) for b in bad_b[:10])))
    if skipped:
        L.append("- 건너뛴 짝·다리: %s" % ", ".join(p["pair"] for p in skipped))
    L.append("- sha1 지킴: %s" % header.get("sha1_guard"))
    L.append("- 판정: **%s**" % ("PASS" if header.get("verdict") else "FAIL"))
    L.append("")
    L.append("## 판단 (H §5.3 — 브리프 §4.3의 얼린 셈)")
    j1, j2, js, j3, j4 = jd["1"], jd["2"], jd["2slow"], jd["3"], jd["4"]
    L.append("- ① R: J1 = %s — %s (두 번 실패 %s)" % (j1["share"]["text"], yn(j1["passed"]), j1["failed"]))
    L.append("- ② R: n₂ %d, J2 = %s — %s; 통째로 띠 밖 %s" % (j2["n2"], j2["share"]["text"], yn(j2["passed"]) + ("" if j2["passed"] is not False else " (%s)" % j2["side"]),
                                                    j2["wholly_outside"]))
    L.append("  - (가) %d (결박한 백성 = 기록의 백성 %d), (나) %d, (다) %d; 다스린 적 있는 백성의 다른 질서 %s" % (
        len(j2["split"]["ga"]), j2["ga_binder_is_record_people"], len(j2["split"]["na"]), len(j2["split"]["da"]), j2["once_ruled"]))
    for key, label in (("na", "(나)"), ("da", "(다)")):
        for f in j2["split"][key]:
            L.append("    - %s %s" % (label, f))
    if j2.get("b6b9_reading"):
        L.append("  - [B-6b9]의 읽기(이 커밋의 일이 아니다): (가)만의 몫 %.3f → %s" % (j2["b6b9_reading"]["ga_share"], j2["b6b9_reading"]["reading"]))
    L.append("- ② 느린 R(적기만): n₂ %d, %s%s" % (js["n2"], js["share"]["text"], " — 구간이 통째로 35%% 위: ④의 받아들임과 함께 [개발자]에게" if js["show_developer"] else ""))
    L.append("- ③ R: n₃ %d — %s — %s; r5 − r4 < 1,200: %s; 창 0: %s" % (j3["n3"], j3["text"], yn(j3["passed"]), [(v["seed"], v["length"]) for v in j3["violations"]],
                                                           j3["zero_windows"]))
    if j3["apart"]:
        L.append("  - 따로 적은 판: %s" % "; ".join("%d (%s)" % (a["seed"], a["why"]) for a in j3["apart"]))
    L.append("- ④ 느린 R: %s — %s" % (j4["text"], yn(j4["passed"])))
    if j4.get("E"):
        L.append("  - E: %s" % " ".join("%d:%s" % (s, A.fmt(e)) for s, e in j4["E"]))
    if j4.get("ii_runs") is not None and j4.get("C0") is not None:
        L.append("  - 48번째(순위 %s) E %s; (ii) 셋째 칸에 닿지 않은 판 %s; 창 0 %s" % (j4.get("rank"), A.fmt(j4.get("E48")), j4.get("ii_not_reached"), j4.get("ii_zero_windows")))
    L.append("")
    L.append("## N §3 자리")
    for k, v in placeholders(res, header).items():
        L.append("- %s %s" % (k, v))
    L.append("")
    L.append("## 적기만 (on / off / on − off; 같은 세계는 칸마다 따로 — 칸을 가로지르는 합은 만들지 않는다)")
    rec = res["record"]
    for cell, sides in rec["cells"].items():
        L.append("### %s" % cell)
        for side, s in sides.items():
            a, b = s["a"], s["b"]
            L.append("- %s: 판 %d; T4 civs most %s; brake held %s; 기록 %s" % (side, a["runs"], a["T4 civs most"]["text"], a["brake held AI-ticks"]["text"], b["record kinds"]))
            L.append("  - 절멸 %s; %s" % ({k: a["extinction"][k] for k in ("on_ruled", "on_ai", "arks", "arks_lost", "expired", "grace", "erased_ext", "ruler_deaths")},
                                         b["extinction"]))
            L.append("  - 결박 %s; #162 %s" % (b["binding"], b["#162"]))
            L.append("  - #156 %s; #141 %s" % (b["#156 era"], b["#141 witness"]))
            L.append("  - 법 %s; AI µs %s" % (b["laws"], a["AI us"]["text"]))
        if cell in rec["pairs"]:
            L.append("- 짝마다: %s" % {k: v["on-off"] for k, v in rec["pairs"][cell].items()})
    L.append("- 잠김(R, locked ≥ 1이고 다섯째 칸 기록 없음): %d %s%s" % (rec["locks"]["count"], rec["locks"]["runs"], " — ≥ 3: #165를 [개발자]에게(결정 13·14 다시)" if rec["locks"]["flag_165"] else ""))
    L.append("- 넷째 칸 이상에 백성을 두고 떠난 그릇(#96): %s" % rec["vessels left at T4+"])
    L.append("- B1 미리 봄(판단 아님, 6,000에서 자름): %s" % rec["B1 preview"])
    L.append("- AI µs: %s" % rec["AI us"])
    L.append("- 강제 죽음이 바꾼 기록(R 켬과 견줌): %s" % (rec["death vs R on"] or "없음"))
    return "\n".join(L) + "\n"

# -------------------------------------------------------------------------------------------------------------- driver


def execute(job, project, editor, ignore_errors, lock, results, results_path):
    attempts = []
    for attempt in (1, 2):
        A.remove_outputs(job)
        start = time.time()
        code = subprocess.run(A.command_of(job, project, editor), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
        why = run_failures(job, parse_log(job["log"]), code, ignore_errors)
        attempts.append({"attempt": attempt, "exit": code, "seconds": round(time.time() - start, 1), "why": why})
        if not why:
            break
    with lock:
        results[job["name"]] = {"ok": not attempts[-1]["why"], "attempts": attempts}
        with open(results_path, "w", encoding="utf-8") as f:
            json.dump(results, f, ensure_ascii=False, indent=1)
    return job["name"], attempts


def evaluate(jobs, results, ignore_errors):
    parsed = {j["name"]: parse_log(j["log"]) for j in jobs}
    for j in jobs:
        known = results.get(j["name"])
        code = known["attempts"][-1]["exit"] if known and known.get("attempts") else None
        why = run_failures(j, parsed[j["name"]], code, ignore_errors)
        if known:
            known["ok"] = not why
            known["why_now"] = why
        else:
            results[j["name"]] = {"ok": not why, "attempts": [], "why_now": why}
    return parsed


def write_summary(folder, res, header, md):
    with open(os.path.join(folder, "summary.json"), "w", encoding="utf-8") as f:
        json.dump(A.jsonable({"header": header, **res}), f, ensure_ascii=False, indent=1)
    with open(os.path.join(folder, "summary.md"), "w", encoding="utf-8") as f:
        f.write(md)


def archive(folder, dest):
    """Brief 4.2: every .log and a sha1 manifest of all output files to dest (the B6a b6a_bundle1 precedent)."""
    os.makedirs(dest, exist_ok=True)
    manifest = []
    for name in sorted(os.listdir(folder)):
        path = os.path.join(folder, name)
        if not os.path.isfile(path):
            continue
        h = hashlib.sha1()
        with open(path, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
        manifest.append("%s  %s" % (h.hexdigest(), name))
        if name.endswith(".log") or name in ("summary.md", "summary.json", "runs.json", "bundle_meta.json"):
            shutil.copy2(path, os.path.join(dest, name))
    with open(os.path.join(dest, "manifest.sha1"), "w", encoding="utf-8") as f:
        f.write("\n".join(manifest) + "\n")
    return len(manifest)


def check_only_a(folder, summary_dir, ignore_errors, max_seed, cells):
    """Brief 4.2 self-test: bundle A's cells and verdict through bundleA.analyse (summary written to summary_dir), and this
    parser over the same logs (the old side layout must read)."""
    jobs = A.build_jobs(folder, cells, max_seed)
    results = {}
    rp = os.path.join(folder, "runs.json")
    if os.path.exists(rp):
        with open(rp, encoding="utf-8") as f:
            results = json.load(f)
    header = {"date": datetime.date.today().isoformat(), "folder": folder, "git": A.git_head(DEFAULT_ROOT) if os.path.isdir(DEFAULT_ROOT) else "-"}
    meta_path = os.path.join(folder, "bundle_meta.json")
    if os.path.exists(meta_path):
        with open(meta_path, encoding="utf-8") as f:
            meta = json.load(f)
        header.update({k: meta[k] for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes") if k in meta})
    header.setdefault("sha1_guard", "not recorded (check-only without bundle_meta.json)")
    os.makedirs(summary_dir, exist_ok=True)
    ok, _, md = A.analyse(jobs, summary_dir, results, header, ignore_errors, A.EXPECTED_REMOVED, (1800,))
    unread = []
    for j in jobs:
        if not os.path.exists(j["log"]):
            continue
        p = parse_log(j["log"])
        bad = [k for k, v in (p.get("race_b") or {}).items() if v is None] + ([] if p.get("ending_x") else ["ending head"])
        if bad:
            unread.append("%s: %s" % (j["name"], ", ".join(bad)))
    return ok, md, unread


def inside(path, root):
    """True when path lies in root (a --check-only never writes under the project tree)."""
    p, r = os.path.normcase(os.path.abspath(path)), os.path.normcase(os.path.abspath(root))
    try:
        return os.path.commonpath([p, r]) == r
    except ValueError:
        return False


def same_dir(x, y):
    return os.path.normcase(os.path.abspath(x)) == os.path.normcase(os.path.abspath(y))


def check_only_write_problem(folder, out_dir, root, what="the summary", allow_folder=True):
    """[B-6b9] review F4, back-ported: why a --check-only must not write `what` (summary.json/.md, or an archive) into
    out_dir (None: it may). Never under --root; never into a folder holding manifest.sha1 (an archived copy); and, with
    allow_folder False (--cell-set A: bundle A's own summary.json is the reference), never into the checked folder."""
    flag = "--archive-dir" if what == "an archive" else "--summary-dir"
    if inside(out_dir, root):
        return "refusing to write %s under the project tree (%s): pass %s outside %s" % (what, out_dir, flag, root)
    if os.path.exists(os.path.join(out_dir, "manifest.sha1")):
        return "refusing to write %s into an archived copy (%s holds manifest.sha1): pass %s" % (what, out_dir, flag)
    if not allow_folder and same_dir(out_dir, folder):
        return "refusing to write %s into the checked folder %s (its own summary.json is the reference)" % (what, folder)
    return None


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=os.environ.get("PROJECT_ROOT", DEFAULT_ROOT))
    ap.add_argument("--folder", default=None)
    ap.add_argument("--t-death", type=int, default=None)
    ap.add_argument("--r", type=float, default=None)
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--mem-per-worker-gb", type=float, default=2.5)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--check-only", metavar="FOLDER", default=None)
    ap.add_argument("--cell-set", choices=("B", "A"), default="B")
    ap.add_argument("--summary-dir", default=None, help="--check-only: where summary.json/.md go (default the checked folder); --cell-set A: "
                                                        "where bundleA.analyse writes its summary (default scratchpad\\b6b8\\bundleA_check)")
    ap.add_argument("--cells", default=None, help="comma list of R,slowR,wanderer,inputless,death (A: R,inputless,wanderer)")
    ap.add_argument("--max-seed", type=int, default=None)
    ap.add_argument("--seeds", default="1-60")
    ap.add_argument("--w-all-cells", action="store_true")
    ap.add_argument("--archive", dest="archive", action="store_true", default=None)
    ap.add_argument("--no-archive", dest="archive", action="store_false")
    ap.add_argument("--archive-dir", default=None, help="default <root>\\Saved\\Balance\\b6b_bundleB")
    ap.add_argument("--a-sec-per-20k", type=float, default=None)
    ap.add_argument("--serial-logs", default=None)
    ap.add_argument("--ignore-error", action="append", default=[])
    ap.add_argument("--editor", default=EDITOR)
    a = ap.parse_args(argv)
    cells = set(a.cells.split(",")) if a.cells else None

    if a.cell_set == "A":
        if not a.check_only:
            print("--cell-set A is for --check-only on bundle A's folder")
            return 2
        folder = os.path.abspath(a.check_only)
        summary_dir = os.path.abspath(a.summary_dir or os.path.join(HERE, "bundleA_check"))
        problem = check_only_write_problem(folder, summary_dir, a.root, "bundle A's summary", allow_folder=False)
        if problem:
            print(problem)
            return 2
        ok, md, unread = check_only_a(folder, summary_dir, a.ignore_error, a.max_seed, cells)
        print(md)
        print("bundle B parser over bundle A's logs: %s" % ("every part read" if not unread else "unreadable: " + "; ".join(unread[:10])))
        print("bundle A check-only through bundle B: %s" % ("PASS" if ok and not unread else "FAIL"))
        return 0 if ok and not unread else 1

    project = os.path.join(a.root, "Cosmic_Ascension.uproject")
    folder = os.path.abspath(a.check_only or a.folder or os.path.join(HERE, "bundleB"))
    do_archive = a.archive if a.archive is not None else not a.check_only
    archive_dest = os.path.abspath(a.archive_dir or os.path.join(a.root, "Saved", "Balance", "b6b_bundleB"))
    out_dir = os.path.abspath(a.summary_dir) if (a.check_only and a.summary_dir) else folder
    if a.check_only and not a.dry_run:
        # [B-6b9] review F4, back-ported: a --check-only never writes summary.json/.md (nor an archive) under --root or
        # into an archived copy; the checked folder is written only when it is neither.
        if do_archive and a.summary_dir:
            print("refusing --archive with --summary-dir: the archive copies the checked folder's summary, not the new one")
            return 2
        for target, what, allow in ((out_dir, "the summary", True),) + (((archive_dest, "an archive", False),) if do_archive else ()):
            problem = check_only_write_problem(folder, target, a.root, what, allow)
            if problem:
                print(problem)
                return 2
    elif a.summary_dir and not a.check_only:
        print("--summary-dir is for --check-only (a run writes its summary into --folder)")
        return 2
    jobs = build_jobs(folder, a.t_death, cells, a.max_seed, parse_seeds(a.seeds))

    if a.dry_run:
        for j in jobs:
            print(A.quote_cmd(A.command_of(j, project, a.editor)))
        by_cell = {}
        for j in jobs:
            by_cell[j["cell"]] = by_cell.get(j["cell"], 0) + 1
        est = ("; <B_MINUTES> ~ %.1f min at %d workers (%.1f s per 20,000-tick run x %d / %d / 60)" % (
            a.a_sec_per_20k * len(jobs) / a.workers / 60.0, a.workers, a.a_sec_per_20k, len(jobs), a.workers)) if a.a_sec_per_20k else \
            "; <B_MINUTES> = <A_SEC_PER_20K> x %d / %d / 60 (pass --a-sec-per-20k)" % (len(jobs), a.workers)
        print("# %d jobs (%s), all at 20,000 ticks, %d workers, folder %s%s%s" % (
            len(jobs), ", ".join("%s %d" % (c, by_cell[c]) for c in CELL_ORDER if c in by_cell), a.workers, folder, est,
            "" if a.t_death is not None or (cells and "death" not in cells) else "; -DeathAt=<T_DEATH> unset (pass --t-death)"))
        print("# then ⓐ 5 + base 5 + on 5 by the lead = %d processes" % (len(jobs) + 15))
        return 0

    header = {"date": datetime.date.today().isoformat(), "folder": folder, "git": A.git_head(a.root), "jobs": len(jobs)}
    results_path = os.path.join(folder, "runs.json")
    serial = serial_ai_us(a.serial_logs)

    if a.check_only:
        results = {}
        if os.path.exists(results_path):
            with open(results_path, encoding="utf-8") as f:
                results = json.load(f)
        meta_path = os.path.join(folder, "bundle_meta.json")
        if os.path.exists(meta_path):
            with open(meta_path, encoding="utf-8") as f:
                meta = json.load(f)
            header.update({k: meta[k] for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes") if k in meta})
        header.setdefault("sha1_guard", "not recorded (check-only without bundle_meta.json)")
    else:
        if cells is None or "death" in cells:
            if a.t_death is None:
                print("refusing: the death cell needs --t-death (N section 2 '[B-6b6]의 다시 셈')")
                return 2
        busy = A.busy_processes()
        if busy:
            print("refusing to start - running now:\n  " + "\n  ".join(busy))
            return 2
        avail, total = A.free_memory_gb()
        print("free memory %s GB of %s GB" % (A.fmt(round(avail, 1)) if avail else "?", A.fmt(round(total, 1)) if total else "?"))
        if a.workers > 6 and (avail is None or avail < a.workers * a.mem_per_worker_gb):
            print("refusing %d workers: free memory does not cover %d x %.1f GB" % (a.workers, a.workers, a.mem_per_worker_gb))
            return 2
        if not os.path.exists(a.editor) or not os.path.exists(project):
            print("no editor or project: %s / %s" % (a.editor, project))
            return 2
        files = guarded_files(a.root)
        if not any(f.lower().endswith(".dll") for f in files):
            print("no UnrealEditor-Cosmic_Ascension*.dll under Binaries/Win64 - build first")
            return 2
        before = A.sha1_files(files)
        os.makedirs(folder, exist_ok=True)
        header["dll_sha1"] = " ".join(sorted(h[:12] for p, h in before.items() if p.lower().endswith(".dll")))
        results, lock = {}, threading.Lock()
        t0 = time.time()
        print("bundle B: %d jobs, %d workers, folder %s" % (len(jobs), a.workers, folder), flush=True)
        with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
            futures = [pool.submit(execute, j, project, a.editor, a.ignore_error, lock, results, results_path) for j in jobs]
            for done, f in enumerate(concurrent.futures.as_completed(futures), 1):
                name, attempts = f.result()
                last = attempts[-1]
                print("[%d/%d %.0fs] %s %s%s" % (done, len(jobs), time.time() - t0, name, "ok" if not last["why"] else "FAILED: " + "; ".join(last["why"]),
                                                " (rerun)" if len(attempts) > 1 else ""), flush=True)
        changed = A.sha1_diff(before, A.sha1_files(guarded_files(a.root)))
        header["sha1_ok"] = not changed
        header["sha1_guard"] = ("unchanged (%d files)" % len(before)) if not changed else "CHANGED - discard the bundle: " + "; ".join(changed)
        header["minutes"] = round((time.time() - t0) / 60.0, 1)
        with open(os.path.join(folder, "bundle_meta.json"), "w", encoding="utf-8") as f:
            json.dump({k: header[k] for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes")}, f, ensure_ascii=False, indent=1)

    parsed = evaluate(jobs, results, a.ignore_error)
    header["end_check"] = sum(p.get("end_check", 0) for p in parsed.values())
    header["race_check"] = sum(p.get("race_check", 0) for p in parsed.values())
    header["processes"] = sum(len(results[j["name"]].get("attempts", [])) for j in jobs)
    header["reruns"] = sum(max(0, len(results[j["name"]].get("attempts", [])) - 1) for j in jobs)
    res = analyse(jobs, parsed, results, a.t_death, a.r, a.w_all_cells, serial)
    header["verdict"] = verdict(res, header)
    if do_archive:
        header["archive"] = archive_dest
    if out_dir != folder:
        header["summary_dir"] = out_dir
    md = markdown(res, header)
    os.makedirs(out_dir, exist_ok=True)
    write_summary(out_dir, res, header, md)
    if do_archive:
        header["archived_files"] = archive(folder, header["archive"])
    print(md)
    print("bundle B: %s" % ("PASS" if header["verdict"] else "FAIL"))
    return 0 if header["verdict"] else 1


if __name__ == "__main__":
    sys.exit(main())
