"""[B-6b6] bundle A (brief sections 4.1-4.5, 0.8): 240 commandlet runs of CosmicBalance, on and with -WithoutB6bData=1,
the per-pair breakable check, the switch controls, the sha1 guard, and the summary of section 4.4 with the inputs and the
recomputation of section 4.5.

Run with `python -I -X utf8 bundleA.py [options]`, the editor closed and nothing else building. Stdlib only.

  --root <project>        project root (default: this PC's)
  --folder <dir>          output folder (default <root>\\Saved\\Balance\\b6b_bundleA)
  --workers N             ThreadPoolExecutor workers (default 6; above 6 only if free memory covers them)
  --dry-run               print the commands, run nothing, write nothing
  --check-only <dir>      re-parse the outputs already in <dir> (no commandlet) and write the summary there
  --cells R,inputless,wanderer and --max-seed N   a subset (for a smoke run or the self-test)

Modelled on scratchpad\\b6b3b_b\\runs.py. The commandlet's switches and lines are those of
Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp at cef8cc7 (read with git show), the editor path that of
Tools/run_checks.ps1 at the same commit.
"""
import argparse
import concurrent.futures
import csv
import ctypes
import datetime
import glob
import hashlib
import io
import json
import math
import os
import random
import re
import statistics
import subprocess
import sys
import threading
import time

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

DEFAULT_ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
UE_ROOT = os.environ.get("UE_ROOT", r"C:\Program Files\Epic Games\UE_5.8")
EDITOR = os.path.join(UE_ROOT, r"Engine\Binaries\Win64\UnrealEditor-Cmd.exe")

# Brief 4.1: every run on the home row (no -Systems/-Civs), the provisional end off.
COMMON_ARGS = ["-Rules=ProvisionalEndTick:0"]
R_ARGS = ["-Answer=1", "-Build=1", "-Focus=1", "-Edict=1", "-Diplomacy=1", "-TellAll=0"]
WANDERER_ARGS = R_ARGS + ["-Abdicate=1200", "-Wander=1"]
OFF_ARG = "-WithoutB6bData=1"
# (cell, ticks, args, seeds) - the 20,000-tick cells first (brief 4.2).
CELLS = [
    ("R", 20000, R_ARGS, range(1, 31)),
    ("inputless", 20000, [], range(1, 31)),
    ("wanderer", 6000, WANDERER_ARGS, range(1, 61)),
]
# CosmicBalanceCommandlet.cpp:1596-1597 - the Note order of Tests/CosmicTestHelpers.h:375-387 on [B-6b6]'s data.
EXPECTED_REMOVED = "feat_passage_by_ark project_ark project_binding crisis_species_extinction"
# Brief 3.2.3 (b) / 4.3: the columns the pair check does not compare.
IGNORED_COLUMNS = ("TickMicros", "SaveBytes", "SaveRawBytes")
LADDER_R = (0.25, 0.3125, 0.375, 0.4375, 0.5)
MODEL_RULED_T4 = (2913, 4310)
BAND = (0.15, 0.35)
BAND_TARGET = 0.26
COUNTDOWN = 250
NOT_MEASURED = "재지 않음 — 잰 %d"

# ----------------------------------------------------------------------------------------------------------------- jobs


def build_jobs(folder, cells=None, max_seed=None):
    """The bundle's jobs, 20,000-tick jobs first; each seed's on and off run next to each other."""
    jobs = []
    for cell, ticks, args, seeds in sorted(CELLS, key=lambda c: -c[1]):
        if cells and cell not in cells:
            continue
        for seed in seeds:
            if max_seed is not None and seed > max_seed:
                continue
            for off in (False, True):
                name = "%s_%d%s" % (cell, seed, "_off" if off else "")
                base = os.path.join(folder, name)
                jobs.append({
                    "name": name, "cell": cell, "seed": seed, "ticks": ticks, "off": off,
                    "args": COMMON_ARGS + list(args) + ([OFF_ARG] if off else []),
                    "csv": base + ".csv", "rows": base + "_rows.csv", "claims": base + "_claims.csv", "log": base + ".log",
                })
    return jobs


def fwd(path):
    return path.replace("\\", "/")


def command_of(job, project, editor=EDITOR):
    # Brief 4.2's command; UE's ProcessCommandLine (Launch/Private/Windows/LaunchWindows.cpp) re-quotes an argument with a
    # space after its '=', so a list argument with the OneDrive path's space reaches FParse::Value whole.
    return [editor, project, "-run=CosmicBalance", "-Seed=%d" % job["seed"], "-Ticks=%d" % job["ticks"], "-Out=" + fwd(job["csv"])] \
        + job["args"] + ["-unattended", "-nosplash", "-AbsLog=" + fwd(job["log"])]


def quote_cmd(cmd):
    return subprocess.list2cmdline(cmd)

# --------------------------------------------------------------------------------------------------------------- guards


def busy_processes():
    """UnrealEditor / UBT processes (and dotnet running UnrealBuildTool), or a reason the check could not be made."""
    try:
        out = subprocess.run(["tasklist", "/FO", "CSV", "/NH"], capture_output=True, text=True, errors="replace", timeout=60).stdout
    except Exception as e:  # noqa: BLE001
        return ["tasklist failed: %s" % e]
    found = []
    dotnet = False
    for row in csv.reader(io.StringIO(out)):
        if not row:
            continue
        image = row[0].strip().lower()
        if image.startswith("unrealeditor") or image.startswith("unrealbuildtool") or image in ("ubt.exe",):
            found.append("%s (pid %s)" % (row[0], row[1] if len(row) > 1 else "?"))
        elif image == "dotnet.exe":
            dotnet = True
    if dotnet:
        ps = ["powershell", "-NoProfile", "-NonInteractive", "-Command",
              "Get-CimInstance Win32_Process -Filter \"Name='dotnet.exe'\" | ForEach-Object { [string]$_.ProcessId + ' ' + $_.CommandLine }"]
        try:
            lines = subprocess.run(ps, capture_output=True, text=True, errors="replace", timeout=60).stdout.splitlines()
            found += ["dotnet: " + l.strip() for l in lines if "unrealbuildtool" in l.lower()]
        except Exception as e:  # noqa: BLE001
            found.append("dotnet.exe running and its command line unreadable (%s) - refusing" % e)
    return found


def free_memory_gb():
    class MEMORYSTATUSEX(ctypes.Structure):
        _fields_ = [("dwLength", ctypes.c_ulong), ("dwMemoryLoad", ctypes.c_ulong), ("ullTotalPhys", ctypes.c_ulonglong),
                    ("ullAvailPhys", ctypes.c_ulonglong), ("ullTotalPageFile", ctypes.c_ulonglong), ("ullAvailPageFile", ctypes.c_ulonglong),
                    ("ullTotalVirtual", ctypes.c_ulonglong), ("ullAvailVirtual", ctypes.c_ulonglong), ("ullAvailExtendedVirtual", ctypes.c_ulonglong)]
    try:
        stat = MEMORYSTATUSEX()
        stat.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
        ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(stat))
        return stat.ullAvailPhys / 1024.0 ** 3, stat.ullTotalPhys / 1024.0 ** 3
    except Exception:  # noqa: BLE001
        return None, None


def guarded_files(root):
    """Brief 4.2: the module DLLs and the DT_*.uasset files (all of them - a superset of the three the commit touches)."""
    files = glob.glob(os.path.join(root, "Binaries", "Win64", "UnrealEditor-Cosmic_Ascension*.dll"))
    files += glob.glob(os.path.join(root, "Content", "CA", "Data", "DT_*.uasset"))
    return sorted(files)


def sha1_files(paths):
    out = {}
    for p in paths:
        h = hashlib.sha1()
        with open(p, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
        out[p] = h.hexdigest()
    return out


def sha1_diff(before, after):
    changed = []
    for p in sorted(set(before) | set(after)):
        if before.get(p) != after.get(p):
            changed.append("%s: %s -> %s" % (os.path.basename(p), before.get(p, "missing"), after.get(p, "missing")))
    return changed

# -------------------------------------------------------------------------------------------------------------- parsing


def read_text(path):
    with open(path, "rb") as f:
        data = f.read()
    return data.decode("utf-8-sig", errors="replace")


def last_match(pattern, text):
    found = None
    for m in re.finditer(pattern, text, re.M):
        found = m
    return found


def ints(m, *names):
    return {n: int(m.group(n)) for n in names}


RACE_PATTERNS = {
    "home": r"home (?P<home>\S+) cap (?P<cap>-?\d+)",
    "ai_t4": r"first AI T4 at (?P<t4_1>-?\d+) \((?P<t4_1_civ>[^,]*), feat (?P<t4_feat>[^)]*)\), second AI T4 at (?P<t4_2>-?\d+) \((?P<t4_2_civ>[^)]*)\), "
             r"first AI T5 at (?P<t5>-?\d+) \((?P<t5_civ>[^,]*), feat (?P<t5_feat>[^)]*)\)",
    "ruled": r"ruled T3 at (?P<r3>-?\d+) \((?P<r3_civ>[^)]*)\), T4 at (?P<r4>-?\d+) \((?P<r4_civ>[^)]*)\), T5 at (?P<r5>-?\d+) \((?P<r5_civ>[^)]*)\)",
    "fifth": r"fifth rung first at (?P<fifth>-?\d+) \((?P<fifth_kind>[^,]*), people (?P<fifth_civ>[^)]*)\), ruled people's end first at (?P<ruled_end>-?\d+), "
             r"silence first at (?P<silence>-?\d+)",
    "t4civs": r"T4 civs at the record (?P<at_record>-?\d+), at the end (?P<at_end>-?\d+), most (?P<most>-?\d+)",
    "brake": r"brake held AI-ticks (?P<held>-?\d+) \(first at (?P<first>-?\d+), peoples (?P<peoples>-?\d+)\)",
    "upper": r"upper construction first started by an AI at (?P<ai_start>-?\d+) \((?P<ai_civ>[^,]*), (?P<ai_row>[^)]*)\), ruled binding started at (?P<ruled_binding>-?\d+)",
    "extinction": r"extinction placed on ruled (?P<on_ruled>\d+) \(first at (?P<on_ruled_first>-?\d+)\) on AI (?P<on_ai>\d+), arks completed (?P<arks>\d+) "
                  r"lost before T4 (?P<arks_lost>\d+), expired (?P<expired>\d+), kept by grace (?P<grace>\d+) \[(?P<grace_keys>[^\]]*)\], "
                  r"ruled expiries (?P<ruled_expiries>\d+) \(died (?P<died>\d+), deposed (?P<deposed>\d+), neither (?P<neither>\d+)\), "
                  r"ruler deaths (?P<ruler_deaths>\d+), peoples erased (?P<erased>\d+) \(by an extinction (?P<erased_ext>\d+): ruled (?P<erased_ruled>\d+), "
                  r"AI (?P<erased_ai>\d+); once ruled (?P<erased_once_ruled>\d+)\)",
    "treaties": r"treaties by the AI step (?P<ai_treaties>\d+) \(first at (?P<ai_treaty_first>-?\d+)\), AI at T3\+ holding one at the end (?P<holders>\d+), "
                r"ruled treaties that met an AI's ask (?P<met_ask>\d+)",
    "crown": r"crown refused far above (?P<refused>\d+) \(first at (?P<first>-?\d+), returns (?P<returns>\d+), peoples (?P<peoples>\d+)\)",
    "claims": r"claims (?P<claims>\d+) \(standing at the end (?P<standing>\d+)\)",
}
STRING_GROUPS = {"home", "t4_1_civ", "t4_feat", "t4_2_civ", "t5_civ", "t5_feat", "r3_civ", "r4_civ", "r5_civ", "fifth_kind", "fifth_civ",
                 "ai_civ", "ai_row", "grace_keys"}


def parse_race(text):
    """The race line (CosmicBalanceCommandlet.cpp:3958-3993) by part; a part that does not match is None."""
    out = {}
    for part, pat in RACE_PATTERNS.items():
        m = re.search(pat, text)
        if not m:
            out[part] = None
            continue
        out[part] = {k: (v if k in STRING_GROUPS else int(v)) for k, v in m.groupdict().items()}
    return out


def counts_text(text):
    """CountsText (' key n key n' or ' none') into a dict."""
    toks = text.split()
    if toks == ["none"]:
        return {}
    return {toks[i]: int(toks[i + 1]) for i in range(0, len(toks) - 1, 2)}


def parse_log(path):
    """Everything bundle A reads from one commandlet log."""
    p = {"log_exists": os.path.exists(path)}
    if not p["log_exists"]:
        return p
    text = read_text(path)
    m = last_match(r"CosmicBalance: seed (\d+), (\d+) ticks, hash (\d+), world (\d+)", text)
    p["seed_line"] = {"seed": int(m.group(1)), "ticks": int(m.group(2)), "hash": int(m.group(3)), "world": int(m.group(4))} if m else None
    m = last_match(r"CosmicBalance race: (.*?)\r?$", text)
    p["race_text"] = m.group(1) if m else None
    p["race"] = parse_race(p["race_text"]) if m else None
    m = last_match(r"CosmicBalance ending: (.*?)\r?$", text)
    p["ending"] = None
    if m:
        e = re.search(r"record (?P<kind>\S+) at (?P<tick>-?\d+) \(people (?P<civ>[^,]*), .*?ruled T3 at (?P<r3>-?\d+) \((?P<r3_civ>[^)]*)\), "
                      r"season (?P<season>-?\d+);.*?first AI T3 at (?P<ai3>-?\d+) \((?P<ai3_civ>[^,]*), feat (?P<ai3_feat>[^)]*)\)", m.group(1))
        if e:
            p["ending"] = {k: (int(v) if k in ("tick", "r3", "season", "ai3") else v) for k, v in e.groupdict().items()}
    p["end_check"] = len(re.findall(r"CosmicBalance end check", text))
    p["race_check"] = len(re.findall(r"CosmicBalance race check", text))
    p["errors"] = [l.strip() for l in text.splitlines() if "LogCosmic: Error" in l]
    m = last_match(r"rows taken out \[([^\]]*)\]", text)
    p["rows_taken_out"] = m.group(1) if m else None
    p["rows_taken_out_lines"] = len(re.findall(r"rows taken out \[", text))
    m = last_match(r"CosmicBalance stages: .*?, AI (-?[0-9.]+) us", text)
    p["ai_us"] = float(m.group(1)) if m else None
    m = last_match(r"CosmicBalance conflict: sieges begun (\d+) \(on ruled (\d+) by end (\d+)\), taken (\d+) \((\d+)/(\d+)/(\d+) by tick bands "
                   r"0-3000/3000-6000/later; from ruled (\d+) by end (\d+)\)", text)
    p["conflict"] = {"begun": int(m.group(1)), "on_ruled": int(m.group(2)), "taken": int(m.group(4)), "from_ruled": int(m.group(8))} if m else None
    if p["conflict"]:
        p["conflict"]["ai_sieges"] = p["conflict"]["begun"] - p["conflict"]["on_ruled"]
        m2 = last_match(r"AI sieges into a higher sphere (\d+)", text)
        p["conflict"]["ai_into_sphere"] = int(m2.group(1)) if m2 else None
    m = last_match(r"CosmicBalance diplomacy: .*?treaties trade (\d+) nonaggression (\d+) \(signed (\d+) broken (\d+)\)", text)
    p["diplomacy"] = {"trade": int(m.group(1)), "nonaggression": int(m.group(2)), "signed": int(m.group(3)), "broken": int(m.group(4))} if m else None
    m = last_match(r"CosmicBalance wanderer: .*?crowned at (-?\d+) \(([^,]*), tier (\d+), path ([^)]*)\)", text)
    p["crowned"] = {"tick": int(m.group(1)), "civ": m.group(2), "tier": int(m.group(3)), "path": m.group(4)} if m else None
    m = last_match(r"CosmicBalance hazards: placed(.*?) \| answered(.*?) \| expired(.*?) \| ended(.*?) \| resolved by edict (\d+)", text)
    p["hazards"] = {"placed": counts_text(m.group(1)), "answered": counts_text(m.group(2)), "expired": counts_text(m.group(3)),
                    "ended": counts_text(m.group(4))} if m else None
    m = last_match(r"death chances \d+ \(by end \d+, grace \d+, branch side \d+\) \[([^\]]*)\]", text)
    p["chances"] = []
    if m and m.group(1):
        for tok in m.group(1).split(","):
            c = re.match(r"([^@]+)@(-?\d+):([^:]*):(-?\d+)", tok)
            if c:
                p["chances"].append({"tag": c.group(1), "tick": int(c.group(2)), "flags": c.group(3)})
    return p


def threshold_of(ending, ticks):
    """Brief 3.1/4.3: T = min(first AI T3, ruled T3) from the ending line; neither reached: the whole run (T = ticks)."""
    if not ending:
        return None
    reached = [t for t in (ending["r3"], ending["ai3"]) if t >= 0]
    return min(reached) if reached else ticks


def run_failures(job, parsed, exit_code=None, ignore_errors=()):
    """Brief 4.2: why a run fails (empty: it passes). exit_code None = unknown (check-only without runs.json)."""
    why = []
    if exit_code is not None and exit_code != 0:
        why.append("exit code %s" % exit_code)
    if not parsed.get("log_exists"):
        why.append("no log")
        return why
    if not parsed.get("seed_line"):
        why.append("no 'CosmicBalance: seed' line")
    if not parsed.get("race_text"):
        why.append("no 'race:' line")
    elif any(v is None for v in parsed["race"].values()):
        why.append("race line parts unreadable: %s" % ", ".join(k for k, v in parsed["race"].items() if v is None))
    if not parsed.get("ending"):
        why.append("no readable 'ending:' line")
    if parsed.get("end_check"):
        why.append("%d end check line(s)" % parsed["end_check"])
    if parsed.get("race_check"):
        why.append("%d race check line(s)" % parsed["race_check"])
    errors = [e for e in parsed.get("errors", []) if not any(re.search(rx, e) for rx in ignore_errors)]
    if errors:
        why.append("%d LogCosmic: Error line(s), first: %s" % (len(errors), errors[0][-300:]))
    for key in ("csv", "rows", "claims"):
        if not os.path.exists(job[key]):
            why.append("missing %s" % os.path.basename(job[key]))
    if parsed.get("seed_line") and parsed["seed_line"]["ticks"] != job["ticks"]:
        why.append("ticks %d, asked %d" % (parsed["seed_line"]["ticks"], job["ticks"]))
    return why

# ------------------------------------------------------------------------------------------------------------ pair check


def read_csv_by_tick(path, ignored=IGNORED_COLUMNS):
    """{tick: [row tuple without the ignored columns]} and the kept header. Split on ',' as the commandlet writes (no quoting)."""
    text = read_text(path)
    lines = [l for l in text.splitlines() if l.strip()]
    if not lines:
        return [], {}
    header = lines[0].split(",")
    drop = {i for i, h in enumerate(header) if h in ignored}
    kept_header = [h for i, h in enumerate(header) if i not in drop]
    by_tick = {}
    for l in lines[1:]:
        cells = l.split(",")
        tick = int(cells[0])
        by_tick.setdefault(tick, []).append(tuple(c for i, c in enumerate(cells) if i not in drop))
    return kept_header, by_tick


def first_difference(a, b):
    """The first tick at which two {tick: rows} differ (None: equal)."""
    for tick in sorted(set(a) | set(b)):
        if a.get(tick) != b.get(tick):
            return tick
    return None


def pair_check(on_job, off_job, on_parsed, off_parsed):
    """Brief 4.3: T from the on-run's ending line, both sides the same T, and every CSV and _rows.csv row with Tick <= T equal
    (TickMicros, SaveBytes, SaveRawBytes aside). Also returns the first differing tick (the pair's first mover sample)."""
    res = {"pair": on_job["name"], "ok": False, "why": [], "T": None, "T_off": None, "first_diff_csv": None, "first_diff_rows": None,
           "differs_after_T": False, "world_differs": None}
    t_on = threshold_of(on_parsed.get("ending"), on_job["ticks"])
    t_off = threshold_of(off_parsed.get("ending"), off_job["ticks"])
    res["T"], res["T_off"] = t_on, t_off
    if t_on is None:
        res["why"].append("no T (on-run ending line unreadable)")
        return res
    if t_on != t_off:
        res["why"].append("T differs: on %s, off %s" % (t_on, t_off))
    for key, label in (("csv", "csv"), ("rows", "rows")):
        if not (os.path.exists(on_job[key]) and os.path.exists(off_job[key])):
            res["why"].append("missing %s file" % label)
            continue
        h_on, a = read_csv_by_tick(on_job[key])
        h_off, b = read_csv_by_tick(off_job[key])
        if h_on != h_off:
            res["why"].append("%s headers differ" % label)
        d = first_difference(a, b)
        res["first_diff_" + label] = d
        if d is not None and d <= t_on:
            res["why"].append("%s differs at tick %d <= T %d" % (label, d, t_on))
        if d is not None and d > t_on:
            res["differs_after_T"] = True
    if on_parsed.get("seed_line") and off_parsed.get("seed_line"):
        res["world_differs"] = on_parsed["seed_line"]["world"] != off_parsed["seed_line"]["world"]
    res["ok"] = not res["why"]
    return res


def switch_controls(pairs, expected_removed=EXPECTED_REMOVED):
    """Brief 4.2's switch controls over (on_job, off_job, on_parsed, off_parsed, pair_result) tuples."""
    problems = []
    placements = 0
    ai_treaties = 0
    differs_after = 0
    for on_job, off_job, on_p, off_p, pr in pairs:
        if off_p.get("log_exists"):
            if off_p.get("rows_taken_out") != expected_removed or off_p.get("rows_taken_out_lines") != 1:
                problems.append("%s: rows taken out [%s] (%s lines), expected [%s]" % (off_job["name"], off_p.get("rows_taken_out"),
                                                                                    off_p.get("rows_taken_out_lines"), expected_removed))
            race = off_p.get("race") or {}
            ext, tr, t4 = race.get("extinction"), race.get("treaties"), race.get("ai_t4")
            if not (ext and tr and t4):
                problems.append("%s: race parts unreadable" % off_job["name"])
            else:
                if ext["on_ruled"] or ext["on_ai"]:
                    problems.append("%s: extinctions placed %d/%d" % (off_job["name"], ext["on_ruled"], ext["on_ai"]))
                if ext["arks"]:
                    problems.append("%s: arks completed %d" % (off_job["name"], ext["arks"]))
                if tr["ai_treaties"]:
                    problems.append("%s: AI-step treaties %d" % (off_job["name"], tr["ai_treaties"]))
                if t4["t4_1"] >= 0:
                    problems.append("%s: AI T4 at %d" % (off_job["name"], t4["t4_1"]))
        if on_p.get("log_exists") and on_p.get("rows_taken_out_lines"):
            problems.append("%s: an on log has a 'rows taken out' line" % on_job["name"])
        race = on_p.get("race") or {}
        if race.get("extinction"):
            placements += race["extinction"]["on_ruled"] + race["extinction"]["on_ai"]
        if race.get("treaties"):
            ai_treaties += race["treaties"]["ai_treaties"]
        differs_after += 1 if (pr and pr["differs_after_T"]) else 0
    if placements <= 0:
        problems.append("on side: no extinction placed in any run")
    if ai_treaties <= 0:
        problems.append("on side: no AI-step treaty in any run")
    if differs_after <= 0:
        problems.append("on side: no pair differs after T")
    return {"ok": not problems, "problems": problems, "on_placements": placements, "on_ai_treaties": ai_treaties, "pairs_differing_after_T": differs_after}

# ---------------------------------------------------------------------------------------------------------- statistics


def wilson(k, n, z=1.96):
    if n <= 0:
        return None, None
    p = k / n
    d = 1 + z * z / n
    c = (p + z * z / (2 * n)) / d
    h = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d
    return max(0.0, c - h), min(1.0, c + h)


def share(k, n):
    lo, hi = wilson(k, n)
    out = {"k": k, "n": n, "p": (k / n) if n else None, "lo": lo, "hi": hi, "judged": n >= 30}
    out["text"] = ("%d/%d = %.3f [%.3f, %.3f]" % (k, n, k / n, lo, hi)) if n else "0/0"
    if n < 30:
        out["text"] += " (" + NOT_MEASURED % n + ")"
    return out


def dist(values):
    vals = sorted(v for v in values if v is not None and not (isinstance(v, float) and math.isinf(v)))
    if not vals:
        return {"n": 0, "median": None, "q1": None, "q3": None, "min": None, "max": None, "text": "n 0"}
    if len(vals) >= 2:
        q1, _, q3 = statistics.quantiles(vals, n=4, method="inclusive")
    else:
        q1 = q3 = vals[0]
    med = statistics.median(vals)
    return {"n": len(vals), "median": med, "q1": q1, "q3": q3, "min": vals[0], "max": vals[-1],
            "text": "n %d median %s [q1 %s, q3 %s] (min %s, max %s)" % (len(vals), fmt(med), fmt(q1), fmt(q3), fmt(vals[0]), fmt(vals[-1]))}


def fmt(v):
    if v is None:
        return "-"
    if isinstance(v, float) and math.isinf(v):
        return "inf"
    if isinstance(v, float):
        return ("%.1f" % v) if v != int(v) else "%d" % v
    return str(v)


def censored_median(values):
    """Median with None/never as +inf (brief 4.5 t)."""
    vals = sorted(math.inf if v is None else v for v in values)
    if not vals:
        return None
    n = len(vals)
    if n % 2:
        return vals[n // 2]
    a, b = vals[n // 2 - 1], vals[n // 2]
    return math.inf if math.isinf(a) or math.isinf(b) else (a + b) / 2.0

# ---------------------------------------------------------------------------------------------------- CSV-derived data


def civ_track(csv_path):
    """Per civ: [(tick, tier, systems, energy, matter)] and the ruled civ (empty Expansion, CosmicBalanceCommandlet.cpp:1321)."""
    text = read_text(csv_path)
    lines = [l for l in text.splitlines() if l.strip()]
    header = lines[0].split(",")
    ix = {h: i for i, h in enumerate(header)}
    track = {}
    ruled = {}
    ticks = []
    for l in lines[1:]:
        c = l.split(",")
        tick = int(c[ix["Tick"]])
        if not ticks or ticks[-1] != tick:
            ticks.append(tick)
        civ = c[ix["CivID"]]
        track.setdefault(civ, []).append((tick, int(c[ix["Tier"]]), int(c[ix["Systems"]]), float(c[ix["Energy"]]), float(c[ix["Matter"]])))
        if c[ix["Expansion"]] == "":
            ruled[tick] = civ
    return {"track": track, "ruled": ruled, "ticks": ticks}


def erased_tick(data, civ):
    """The first sample after a civ's last row (it left Civs in between); None while it stands at the last sample."""
    rows = data["track"].get(civ)
    if not rows:
        return None
    last = rows[-1][0]
    later = [t for t in data["ticks"] if t > last]
    return later[0] if later else None


def first_sample_at_tier(rows, tier):
    for r in rows:
        if r[1] >= tier:
            return r
    return None


def rows_first(rows_path, source_row, from_tick=-1, to_tick=None):
    """The sample ticks of _rows.csv lines with SourceRow source_row (a line's tick is in (sample-100, sample])."""
    out = []
    text = read_text(rows_path)
    for l in text.splitlines()[1:]:
        c = l.split(",")
        if len(c) >= 3 and c[1] == source_row:
            t = int(c[0])
            if t >= from_tick and (to_tick is None or t <= to_tick):
                out.append((t, int(c[2])))
    return out

# ------------------------------------------------------------------------------------------------------------- 4.5


def direct_resample(runs, P, r, N, horizon):
    """Brief 4.5: per R run, who binds first from the run's measured ruled T4 and AI T4 ticks; a binder the run shows erased
    before T4 + P (ruled) or T4 + P/r (AI) does not bind. Ties go to the AI (remodel.py's 'rbind < abind')."""
    ruled_first = other = none = beyond = 0
    ends, seasons_all, seasons_ruled_first = [], [], []
    for run in runs:
        rb = math.inf
        if run["r4"] is not None:
            bind = run["r4"] + P
            er = run["erased"].get(run["r4_civ"])
            beyond += 1 if bind > horizon else 0
            if er is None or er > bind:
                rb = bind
        best = (math.inf, None)
        for t, civ in run["ai_t4"]:
            bind = t + P / r
            er = run["erased"].get(civ)
            beyond += 1 if bind > horizon else 0
            if (er is None or er > bind) and bind < best[0]:
                best = (bind, t)
        ab, at = best
        if math.isinf(rb) and math.isinf(ab):
            none += 1
            continue
        if rb < ab:
            ruled_first += 1
            e = max(rb, run["r4"] + N)
        else:
            other += 1
            e = max(ab, at + N)
        ends.append(e)
        if run["r4"] is not None:
            seasons_all.append(e - run["r4"])
            if rb < ab:
                seasons_ruled_first.append(e - run["r4"])
    return {"P": P, "r": r, "N": N, "ruled_first": ruled_first, "other": other, "none": none, "binds_past_horizon": beyond,
            "other_share": share(other, ruled_first + other), "end": dist(ends), "season_all": dist(seasons_all),
            "season_ruled_first": dist(seasons_ruled_first)}


def monte_carlo(L, AIOFF, FIRST, ruled_durs, ai_durs, P, r, N, cap=2, n=6000, seed=7):
    """remodel.py's run() with the measured T3->T4 durations (bootstrap, +inf for a people that never reached T4 - erased or
    past the horizon) in place of wait()+150. A proposal: the brief names the inputs, not the draw."""
    rnd = random.Random(seed)
    rf = other = none = 0
    ends, seasons = [], []
    for i in range(n):
        l = L[i % len(L)]
        rt4 = FIRST + l + rnd.choice(ruled_durs)
        ai = sorted(FIRST + o + rnd.choice(ai_durs) for o in AIOFF)
        passed = []
        for t in ai:
            if math.isinf(t):
                continue
            occ = len(passed) + (1 if rt4 <= t else 0)
            if occ < cap:
                passed.append(t)
        rbind = rt4 + P
        abind = min((t + P / r for t in passed), default=math.inf)
        if math.isinf(rbind) and math.isinf(abind):
            none += 1
            continue
        if rbind < abind:
            rf += 1
            e = max(rbind, rt4 + N)
        else:
            other += 1
            t = min(passed, key=lambda t: t + P / r)
            e = max(t + P / r, t + N)
        ends.append(e)
        if not math.isinf(rt4):
            seasons.append(e - rt4)
    return {"P": P, "r": r, "N": N, "ruled_first": rf, "other": other, "none": none, "other_share": share(other, rf + other),
            "end": dist(ends), "season": dist(seasons)}


def pick_r(results_by_r, base_r=0.375):
    """Brief 4.5: keep r if its other-order share is in 15-35%; else the fewest ladder steps into the band, nearest 26%."""
    i0 = LADDER_R.index(base_r)
    inside = [(abs(LADDER_R.index(r) - i0), abs(res["other_share"]["p"] - BAND_TARGET), r)
              for r, res in results_by_r.items() if res["other_share"]["p"] is not None and BAND[0] <= res["other_share"]["p"] <= BAND[1]]
    if any(x[2] == base_r for x in inside):
        return base_r, "kept (inside 15-35%)"
    if not inside:
        return None, "[개발자] - no ladder value inside 15-35%"
    inside.sort()
    return inside[0][2], "moved %d ladder step(s) into the band" % inside[0][0]


def recompute_45(jobs_by_name, parsed, pbinds=(1800,), N=1200, mc_runs=6000, mc_seed=7):
    """Brief 4.5's inputs from the R and input-less on runs, the direct resample and the Monte Carlo."""
    R = [j for j in jobs_by_name.values() if j["cell"] == "R" and not j["off"]]
    NL = [j for j in jobs_by_name.values() if j["cell"] == "inputless" and not j["off"]]
    table = []
    for j in sorted(R, key=lambda j: j["seed"]):
        p = parsed.get(j["name"]) or {}
        race, ending = p.get("race") or {}, p.get("ending") or {}
        if not race.get("ruled") or not race.get("ai_t4") or not ending or not os.path.exists(j["csv"]):
            table.append({"seed": j["seed"], "missing": True})
            continue
        data = civ_track(j["csv"])
        ru, a4 = race["ruled"], race["ai_t4"]
        r3 = ru["r3"] if ru["r3"] >= 0 else None
        r4 = ru["r4"] if ru["r4"] >= 0 else None
        ai3 = ending["ai3"] if ending["ai3"] >= 0 else None
        ruled_civ = ru["r4_civ"] if r4 is not None else ru["r3_civ"]
        ai_t4 = [(a4["t4_1"], a4["t4_1_civ"])] if a4["t4_1"] >= 0 else []
        ai_t4 += [(a4["t4_2"], a4["t4_2_civ"])] if a4["t4_2"] >= 0 else []
        erased = {civ: erased_tick(data, civ) for civ in data["track"]}
        k3 = k4 = None
        rows = data["track"].get(ruled_civ, [])
        if r3 is not None:
            k3 = next((x[2] for x in rows if x[0] >= r3), None)
        if r4 is not None:
            k4 = next((x[2] for x in rows if x[0] >= r4), None)
        table.append({"seed": j["seed"], "missing": False, "r3": r3, "r4": r4, "r4_civ": ruled_civ, "ai3": ai3,
                      "L": (r3 - ai3) if (r3 is not None and ai3 is not None) else None, "ai_t4": ai_t4,
                      "erased": {c: t for c, t in erased.items() if t is not None}, "k3": k3, "k4": k4,
                      "ruled_erased": erased.get(ruled_civ)})
    # Input-less: FIRST (exact, the ending line) and AIOFF (rank-wise medians of the other AIs' T3 sample offsets).
    firsts, offsets_per_run, ai_durs_nl = [], [], []
    for j in sorted(NL, key=lambda j: j["seed"]):
        p = parsed.get(j["name"]) or {}
        ending = p.get("ending")
        if not ending or ending["ai3"] < 0 or not os.path.exists(j["csv"]):
            continue
        firsts.append(ending["ai3"])
        data = civ_track(j["csv"])
        ruled_civs = set(data["ruled"].values())
        t3s = []
        for civ, rows in data["track"].items():
            if civ in ruled_civs:
                continue
            at3 = first_sample_at_tier(rows, 3)
            if at3:
                t3s.append(at3[0])
                at4 = first_sample_at_tier(rows, 4)
                ai_durs_nl.append((at4[0] - at3[0]) if at4 else math.inf)
        t3s.sort()
        if t3s:
            offsets_per_run.append([t - t3s[0] for t in t3s])
    FIRST = statistics.median(firsts) if firsts else None
    AIOFF, aioff_n = [], []
    if offsets_per_run:
        for rank in range(max(len(o) for o in offsets_per_run)):
            vals = [o[rank] for o in offsets_per_run if len(o) > rank]
            if len(vals) * 2 >= len(offsets_per_run):
                AIOFF.append(statistics.median(vals))
                aioff_n.append(len(vals))
    good = [t for t in table if not t.get("missing")]
    L = [t["L"] for t in good if t["L"] is not None]
    ruled_durs = [(t["r4"] - t["r3"]) if t["r4"] is not None else math.inf for t in good if t["r3"] is not None]
    horizon = max((j["ticks"] for j in R), default=20000)
    out = {"inputs": {"L": L, "FIRST": FIRST, "FIRST_n": len(firsts), "AIOFF": AIOFF, "AIOFF_n": aioff_n,
                      "k3": dist([t["k3"] for t in good]), "k4": dist([t["k4"] for t in good]),
                      "ruled_T3_T4": dist([d for d in ruled_durs if not math.isinf(d)]), "ruled_T3_T4_never": sum(1 for d in ruled_durs if math.isinf(d)),
                      "ai_T3_T4_inputless": dist([d for d in ai_durs_nl if not math.isinf(d)]),
                      "ai_T3_T4_inputless_never": sum(1 for d in ai_durs_nl if math.isinf(d))},
           "table": table, "direct": {}, "mc": {}, "decision": {}}
    for P in pbinds:
        out["direct"][P] = {r: direct_resample(good, P, r, N, horizon) for r in LADDER_R}
        if L and FIRST is not None and AIOFF and ruled_durs and ai_durs_nl:
            out["mc"][P] = {r: monte_carlo(L, AIOFF, FIRST, ruled_durs, ai_durs_nl, P, r, N, n=mc_runs, seed=mc_seed) for r in LADDER_R}
        dr, dwhy = pick_r(out["direct"][P])
        out["decision"][P] = {"direct": {"r": dr, "why": dwhy}}
        if P in out["mc"]:
            mr, mwhy = pick_r(out["mc"][P])
            out["decision"][P]["mc"] = {"r": mr, "why": mwhy}
    # t: the median ruled T4 over the R runs that reach T4, rounded up to the next 100 (H:330).
    r4s = [t["r4"] for t in good if t["r4"] is not None]
    t_med = statistics.median(r4s) if r4s else None
    out["t"] = {"n": len(r4s), "median": t_med, "t": (int(math.ceil(t_med / 100.0)) * 100) if t_med is not None else 4400,
                "censored_median_all": censored_median([t["r4"] for t in good]), "flag_n_below_15": len(r4s) < 15,
                "n0_fallback": not r4s}
    return out

# --------------------------------------------------------------------------------------------------------- summary (4.4)


def three_part_split(job, p):
    """Ruled T3 -> first placement on the ruled people -> the ark's completion line (rows file, to 100 ticks) -> ruled T4."""
    race = p.get("race") or {}
    ru, ext = race.get("ruled"), race.get("extinction")
    if not ru or not ext or ru["r3"] < 0 or ru["r4"] < 0 or ext["on_ruled_first"] < 0 or not os.path.exists(job["rows"]):
        return None
    placed = ext["on_ruled_first"]
    done = rows_first(job["rows"], "project_ark", from_tick=placed, to_tick=int(math.ceil(ru["r4"] / 100.0)) * 100)
    if not done:
        return {"strike": placed - ru["r3"], "build": None, "treaty": None, "placements_on_ruled": ext["on_ruled"]}
    completed = min(done[-1][0], ru["r4"])
    return {"strike": placed - ru["r3"], "build": completed - placed, "treaty": ru["r4"] - completed, "placements_on_ruled": ext["on_ruled"],
            "completion_sample": done[-1][0]}


def countdown_stock(job, p):
    """The ruled people's least Energy and Matter in the samples of its first extinction's countdown (CSV, every 100 ticks)."""
    race = p.get("race") or {}
    ext, ru = race.get("extinction"), race.get("ruled")
    if not ext or not ru or ext["on_ruled_first"] < 0 or not os.path.exists(job["csv"]):
        return None
    data = civ_track(job["csv"])
    start = ext["on_ruled_first"]
    # The people the vessel ruled at the placement: the CSV's ruled row (empty Expansion) of the last sample at or before it.
    civ = data["ruled"].get(max((t for t in data["ruled"] if t <= start), default=-1)) or ru["r3_civ"]
    rows = [r for r in data["track"].get(civ, []) if start <= r[0] <= start + COUNTDOWN]
    if not rows:
        return None
    return {"energy": min(r[3] for r in rows), "matter": min(r[4] for r in rows)}


def side_summary(items):
    """Section 4.4's fields over one (cell, side): items = [(job, parsed)] of passing runs."""
    s = {"runs": len(items)}
    races = [(j, p, p["race"]) for j, p in items if p.get("race")]
    get = lambda part, key: [r[part][key] for _, _, r in races if r.get(part)]  # noqa: E731
    pos = lambda vals: [v for v in vals if v >= 0]  # noqa: E731
    n = len(races)
    for label, part, key in (("first AI T4", "ai_t4", "t4_1"), ("second AI T4", "ai_t4", "t4_2"), ("first AI T5", "ai_t4", "t5"),
                             ("ruled T3", "ruled", "r3"), ("ruled T4", "ruled", "r4"), ("ruled T5", "ruled", "r5")):
        vals = get(part, key)
        s[label] = {"reached": share(len(pos(vals)), n), "ticks": dist(pos(vals))}
    s["brake held AI-ticks"] = dist(get("brake", "held"))
    s["brake held peoples"] = dist(get("brake", "peoples"))
    s["brake held runs"] = share(sum(1 for v in get("brake", "held") if v > 0), n)
    s["T4 civs at end"] = dist(get("t4civs", "at_end"))
    s["T4 civs most"] = dist(get("t4civs", "most"))
    s["T4 civs most above cap 2"] = sum(1 for v in get("t4civs", "most") if v > 2)
    ext = [r["extinction"] for _, _, r in races if r.get("extinction")]
    tot = lambda key: sum(e[key] for e in ext)  # noqa: E731
    s["extinction"] = {k: tot(k) for k in ("on_ruled", "on_ai", "arks", "arks_lost", "expired", "grace", "ruled_expiries", "died", "deposed",
                                              "neither", "ruler_deaths", "erased", "erased_ext", "erased_ruled", "erased_ai", "erased_once_ruled")}
    s["extinction"]["completed within the countdown"] = share(tot("arks"), tot("arks") + tot("expired"))
    keys = " ".join(e["grace_keys"] for e in ext if e["grace_keys"]).split()
    s["extinction"]["grace keys"] = keys
    s["extinction"]["grace on ruled / AI"] = (sum(1 for k in keys if k.endswith(":r")), sum(1 for k in keys if k.endswith(":a")))
    s["extinction"]["runs with a placement"] = share(sum(1 for e in ext if e["on_ruled"] + e["on_ai"] > 0), n)
    s["extinction"]["deleted AIs per run"] = dist([e["erased_ai"] for e in ext])
    s["extinction"]["unpaid ticks"] = "재지 않음 (no output carries it - :2364-2367)"
    s["extinction"]["expired with an ark standing"] = "재지 않음 (no output carries it - hazards: :3610)"
    tr = [r["treaties"] for _, _, r in races if r.get("treaties")]
    s["AI treaties"] = {"by the step": dist([t["ai_treaties"] for t in tr]), "by the step total": sum(t["ai_treaties"] for t in tr),
                        "first": dist(pos([t["ai_treaty_first"] for t in tr])), "AI T3+ holding one": dist([t["holders"] for t in tr]),
                        "ruled met an AI's ask": dist([t["met_ask"] for t in tr])}
    conf = [p["conflict"] for _, p in items if p.get("conflict")]
    s["conflict"] = {"AI sieges begun": dist([c["ai_sieges"] for c in conf]), "taken": dist([c["taken"] for c in conf]),
                     "taken not from ruled": dist([c["taken"] - c["from_ruled"] for c in conf])}
    s["crown refused far above"] = dist(get("crown", "refused"))
    s["AI us"] = dist([p["ai_us"] for _, p in items if p.get("ai_us") is not None])
    crowned = [p["crowned"] for _, p in items if p.get("crowned")]
    if crowned:
        s["crowned"] = {"crowned": share(sum(1 for c in crowned if c["tick"] >= 0), len(crowned)), "tick": dist(pos([c["tick"] for c in crowned]))}
    graces = {}
    for _, p in items:
        for c in p.get("chances", []):
            if "g" in c["flags"]:
                graces[c["tag"]] = graces.get(c["tag"], 0) + 1
    s["grace by tag (calling line, vessel at risk only)"] = graces
    return s


def summarize(jobs, parsed, results, pairs, controls, header, recompute=None):
    by_name = {j["name"]: j for j in jobs}
    out = {"header": header, "cells": {}, "pairs": pairs, "controls": controls, "runs": results}
    for cell in sorted({j["cell"] for j in jobs}, key=lambda c: ["R", "inputless", "wanderer"].index(c)):
        out["cells"][cell] = {}
        for side, off in (("on", False), ("off", True)):
            items = [(j, parsed[j["name"]]) for j in jobs if j["cell"] == cell and j["off"] == off and results.get(j["name"], {}).get("ok")]
            out["cells"][cell][side] = side_summary(items)
    # Per-pair AI us difference (on - off), the first mover sample of each pair.
    diffs = {}
    for pr in pairs:
        on, off = by_name.get(pr["pair"]), by_name.get(pr["pair"] + "_off")
        a, b = parsed.get(pr["pair"], {}).get("ai_us"), parsed.get(pr["pair"] + "_off", {}).get("ai_us")
        if on and a is not None and b is not None:
            diffs.setdefault(on["cell"], []).append(a - b)
    out["AI us on-off per pair"] = {c: dist(v) for c, v in diffs.items()}
    # R on: the three-part T3 -> T4 split and the stocks during the first ruled countdown.
    split, stock = [], []
    for j in jobs:
        if j["cell"] == "R" and not j["off"] and results.get(j["name"], {}).get("ok"):
            sp = three_part_split(j, parsed[j["name"]])
            if sp:
                split.append(sp)
            st = countdown_stock(j, parsed[j["name"]])
            if st:
                stock.append(st)
    out["R three-part split"] = {"runs": len(split), "strike wait": dist([x["strike"] for x in split]),
                                 "ark build (to 100 ticks)": dist([x["build"] for x in split if x["build"] is not None]),
                                 "treaty wait (to 100 ticks)": dist([x["treaty"] for x in split if x["treaty"] is not None]),
                                 "runs with more than one ruled placement": sum(1 for x in split if x["placements_on_ruled"] > 1)}
    out["R first countdown stocks"] = {"energy least": dist([x["energy"] for x in stock]), "matter least": dist([x["matter"] for x in stock])}
    # Extinction outbreak lines per 600-tick window (rows file; told lines only under -TellAll=0).
    windows = {}
    for j in jobs:
        if not j["off"] and j["cell"] in ("R", "inputless") and results.get(j["name"], {}).get("ok") and os.path.exists(j["rows"]):
            for t, c in rows_first(j["rows"], "crisis_species_extinction"):
                w = windows.setdefault(j["cell"], {})
                k = max(0, t - 1) // 600  # a sample's lines lie in (t-100, t]
                w[k] = w.get(k, 0) + c
    out["extinction lines per 600-tick window"] = {c: {"%d-%d" % (k * 600, k * 600 + 600): v for k, v in sorted(w.items())} for c, w in windows.items()}
    r_on = out["cells"].get("R", {}).get("on")
    if r_on:
        med = r_on["ruled T4"]["ticks"]["median"]
        out["c review"] = {"deleted AIs (R on)": r_on["extinction"]["erased_ai"], "ruled T4 median": med,
                           "model medians": MODEL_RULED_T4,
                           "first AI T4 before ruled T4": first_before_ruled(jobs, parsed, results)}
    out["grace two paths"] = {"grace events whose people held more than one extinction slot":
                              NOT_MEASURED % sum(len(out["cells"][c]["on"]["extinction"]["grace keys"]) for c in out["cells"]),
                              "model P(second extinction on the same people within 250) at c 1e-4, k 4": 1 - (1 - 1e-4) ** (3 * 250)}
    if recompute:
        out["recompute_45"] = recompute
    return out


def first_before_ruled(jobs, parsed, results):
    """Brief 4.6 Q4: the share of R on runs whose first AI T4 comes before the ruled T4 (a run without the ruled T4 counts when an AI reached it)."""
    k = n = 0
    for j in jobs:
        if j["cell"] == "R" and not j["off"] and results.get(j["name"], {}).get("ok"):
            race = parsed[j["name"]].get("race") or {}
            a, r = race.get("ai_t4"), race.get("ruled")
            if not a or not r:
                continue
            if a["t4_1"] < 0 and r["r4"] < 0:
                continue
            n += 1
            k += 1 if (a["t4_1"] >= 0 and (r["r4"] < 0 or a["t4_1"] < r["r4"])) else 0
    return share(k, n)


def jsonable(o):
    if isinstance(o, float) and math.isinf(o):
        return "inf"
    if isinstance(o, dict):
        return {str(k): jsonable(v) for k, v in o.items()}
    if isinstance(o, (list, tuple)):
        return [jsonable(v) for v in o]
    return o


def markdown(summary):
    h = summary["header"]
    L = []
    L.append("# [B-6b6] 묶음 A — %s" % h["date"])
    L.append("")
    L.append("- 빌드: %s; DLL sha1 %s" % (h.get("git"), h.get("dll_sha1")))
    L.append("- 프로세스: 묶음 %d회 실행(작업 %d, 재실행 %d) — 묶음 밖의 ⓐ·base·on 15회는 리드가 더한다(총 255)" % (h["processes"], h["jobs"], h["reruns"]))
    L.append("- 실패: %d (%s)" % (len(h["failed"]), ", ".join(h["failed"]) or "없음"))
    L.append("- 폴더: `%s`" % h["folder"])
    L.append("- check 줄: end check %d, race check %d" % (h["end_check"], h["race_check"]))
    L.append("- 스위치 대조: %s%s" % ("통과" if summary["controls"]["ok"] else "실패", "" if summary["controls"]["ok"] else " — " + "; ".join(summary["controls"]["problems"][:10])))
    L.append("  - on: 놓인 절멸 %d, AI 걸음 조약 %d, T 뒤에 갈린 쌍 %d" % (summary["controls"]["on_placements"], summary["controls"]["on_ai_treaties"],
                                                                 summary["controls"]["pairs_differing_after_T"]))
    L.append("- 쌍 검사(Tick ≤ T 같음): %d/%d 통과%s" % (sum(1 for p in summary["pairs"] if p["ok"]), len(summary["pairs"]),
                                                  "" if all(p["ok"] for p in summary["pairs"]) else " — 실패: " + ", ".join(p["pair"] for p in summary["pairs"] if not p["ok"])))
    L.append("- sha1 지킴: %s" % h["sha1_guard"])
    L.append("")
    for cell, sides in summary["cells"].items():
        L.append("## %s" % cell)
        L.append("")
        L.append("| 값 | on | off |")
        L.append("|---|---|---|")
        on, off = sides["on"], sides["off"]
        for label in ("first AI T4", "second AI T4", "first AI T5", "ruled T3", "ruled T4", "ruled T5"):
            L.append("| %s 닿음 | %s | %s |" % (label, on[label]["reached"]["text"], off[label]["reached"]["text"]))
            L.append("| %s 틱 | %s | %s |" % (label, on[label]["ticks"]["text"], off[label]["ticks"]["text"]))
        for label in ("brake held AI-ticks", "brake held peoples", "T4 civs at end", "T4 civs most", "crown refused far above", "AI us"):
            L.append("| %s | %s | %s |" % (label, on[label]["text"], off[label]["text"]))
        L.append("| brake held runs | %s | %s |" % (on["brake held runs"]["text"], off["brake held runs"]["text"]))
        L.append("| T4 civs most > cap 2 (runs) | %d | %d |" % (on["T4 civs most above cap 2"], off["T4 civs most above cap 2"]))
        for k in ("on_ruled", "on_ai", "arks", "arks_lost", "expired", "grace", "ruled_expiries", "died", "deposed", "neither", "ruler_deaths",
                  "erased", "erased_ext", "erased_ruled", "erased_ai", "erased_once_ruled"):
            L.append("| extinction %s (합) | %d | %d |" % (k, on["extinction"][k], off["extinction"][k]))
        L.append("| 카운트다운 안 완성 = arks/(arks+expired) | %s | %s |" % (on["extinction"]["completed within the countdown"]["text"],
                                                                    off["extinction"]["completed within the countdown"]["text"]))
        L.append("| grace ruled/AI (keys) | %s | %s |" % (on["extinction"]["grace on ruled / AI"], off["extinction"]["grace on ruled / AI"]))
        L.append("| grace by tag (calling) | %s | %s |" % (on["grace by tag (calling line, vessel at risk only)"], off["grace by tag (calling line, vessel at risk only)"]))
        L.append("| 지워진 AI/판 | %s | %s |" % (on["extinction"]["deleted AIs per run"]["text"], off["extinction"]["deleted AIs per run"]["text"]))
        for k in ("by the step", "first", "AI T3+ holding one", "ruled met an AI's ask"):
            L.append("| AI treaties %s | %s | %s |" % (k, on["AI treaties"][k]["text"], off["AI treaties"][k]["text"]))
        for k in ("AI sieges begun", "taken", "taken not from ruled"):
            L.append("| conflict %s | %s | %s |" % (k, on["conflict"][k]["text"], off["conflict"][k]["text"]))
        if "crowned" in on:
            L.append("| B1 지킴: crowned | %s | %s |" % (on["crowned"]["crowned"]["text"], off.get("crowned", {}).get("crowned", {}).get("text")))
            L.append("| B1 지킴: crowned at | %s | %s |" % (on["crowned"]["tick"]["text"], off.get("crowned", {}).get("tick", {}).get("text")))
        L.append("| 치르지 못한 틱 / 방주가 선 채 만료 | 재지 않음 | 재지 않음 |")
        L.append("")
    L.append("## R on: 셋째 → 넷째 칸의 세 토막")
    sp = summary["R three-part split"]
    L.append("- 판 %d; 타격 기다림 %s; 방주 짓기 %s; 조약 기다림 %s; 다스리는 백성에 둘 이상 놓인 판 %d" % (
        sp["runs"], sp["strike wait"]["text"], sp["ark build (to 100 ticks)"]["text"], sp["treaty wait (to 100 ticks)"]["text"],
        sp["runs with more than one ruled placement"]))
    st = summary["R first countdown stocks"]
    L.append("- 첫 카운트다운의 최소 에너지 %s, 물질 %s" % (st["energy least"]["text"], st["matter least"]["text"]))
    L.append("- 절멸 줄 600틱 창: %s" % summary["extinction lines per 600-tick window"])
    L.append("- AI µs on−off (쌍): %s" % {c: d["text"] for c, d in summary["AI us on-off per pair"].items()})
    if "c review" in summary:
        c = summary["c review"]
        L.append("- c 검토: 지워진 AI(R on 합) %d; 다스리는 T4 중앙값 %s (모형 2,913 / 4,310); 첫 AI T4가 다스리는 T4보다 먼저 %s" % (
            c["deleted AIs (R on)"], fmt(c["ruled T4 median"]), c["first AI T4 before ruled T4"]["text"]))
    g = summary["grace two paths"]
    L.append("- 은혜의 두 길: 둘 이상 절멸 칸을 가진 백성의 은혜 — %s; 모형 P = %.3f" % (
        g["grace events whose people held more than one extinction slot"], g["model P(second extinction on the same people within 250) at c 1e-4, k 4"]))
    L.append("")
    rc = summary.get("recompute_45")
    if rc:
        L.append("## 4.5 다시 셈의 입력")
        i = rc["inputs"]
        L.append("- L (R 30, ruled T3 − first AI T3): %s" % i["L"])
        L.append("- FIRST %s (n %d); AIOFF %s (n per rank %s)" % (fmt(i["FIRST"]), i["FIRST_n"], i["AIOFF"], i["AIOFF_n"]))
        L.append("- k at T3 %s; k at T4 %s" % (i["k3"]["text"], i["k4"]["text"]))
        L.append("- ruled T3→T4 %s (never %d); AI T3→T4 input-less %s (never %d)" % (i["ruled_T3_T4"]["text"], i["ruled_T3_T4_never"],
                                                                                 i["ai_T3_T4_inputless"]["text"], i["ai_T3_T4_inputless_never"]))
        L.append("")
        L.append("| seed | ruled T3 | ruled T4 (civ) | first AI T3 | L | AI T4 | ruled erased | k3 | k4 |")
        L.append("|---|---|---|---|---|---|---|---|---|")
        for t in rc["table"]:
            if t.get("missing"):
                L.append("| %d | (없음) |||||||" % t["seed"])
                continue
            L.append("| %d | %s | %s (%s) | %s | %s | %s | %s | %s | %s |" % (t["seed"], fmt(t["r3"]), fmt(t["r4"]), t["r4_civ"], fmt(t["ai3"]), fmt(t["L"]),
                                                                    " ".join("%d:%s" % x for x in t["ai_t4"]) or "-", fmt(t["ruled_erased"]), fmt(t["k3"]), fmt(t["k4"])))
        L.append("")
        for P in rc["direct"]:
            L.append("### P_bind %s, N 1,200" % P)
            L.append("| r | 직접 다시 뽑기: 다른 질서 | 끝 중앙값 | 계절(전부) | 계절(다스리는 쪽 먼저) | 몬테카를로: 다른 질서 | 끝 | 계절 |")
            L.append("|---|---|---|---|---|---|---|---|")
            for r in LADDER_R:
                d = rc["direct"][P][r]
                m = rc["mc"].get(P, {}).get(r)
                L.append("| %s | %s | %s | %s | %s | %s | %s | %s |" % (
                    r, d["other_share"]["text"], fmt(d["end"]["median"]), fmt(d["season_all"]["median"]), fmt(d["season_ruled_first"]["median"]),
                    m["other_share"]["text"] if m else "-", fmt(m["end"]["median"]) if m else "-", fmt(m["season"]["median"]) if m else "-"))
            L.append("- 판단(직접): %s; 판단(몬테카를로): %s" % (rc["decision"][P]["direct"], rc["decision"][P].get("mc", "-")))
            L.append("")
        t = rc["t"]
        L.append("- t = %s (R에서 T4에 닿은 판 n %d, 중앙값 %s; 30판 검열 중앙값 %s)%s%s" % (
            t["t"], t["n"], fmt(t["median"]), fmt(t["censored_median_all"]), " — n < 15: N §5에 표시" if t["flag_n_below_15"] else "",
            " — n 0: 4,400(H §11에 표시)" if t["n0_fallback"] else ""))
    return "\n".join(L) + "\n"

# ------------------------------------------------------------------------------------------------------------- driver


def remove_outputs(job):
    stem = job["log"][:-4]
    for path in (job["log"], stem + "_2.log", job["csv"], job["rows"], job["claims"]):
        if os.path.exists(path):
            os.remove(path)


def execute(job, project, editor, ignore_errors, lock, results, results_path):
    attempts = []
    for attempt in (1, 2):
        remove_outputs(job)
        start = time.time()
        code = subprocess.run(command_of(job, project, editor), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode
        parsed = parse_log(job["log"])
        why = run_failures(job, parsed, code, ignore_errors)
        attempts.append({"attempt": attempt, "exit": code, "seconds": round(time.time() - start, 1), "why": why})
        if not why:
            break
    with lock:
        results[job["name"]] = {"ok": not attempts[-1]["why"], "attempts": attempts}
        with open(results_path, "w", encoding="utf-8") as f:
            json.dump(results, f, ensure_ascii=False, indent=1)
    return job["name"], attempts


def git_head(root):
    try:
        head = subprocess.run(["git", "-C", root, "log", "--oneline", "-1"], capture_output=True, text=True, errors="replace", timeout=30).stdout.strip()
        # --no-optional-locks: a status that never refreshes the index (another workflow may be using the tree).
        dirty = subprocess.run(["git", "--no-optional-locks", "-C", root, "status", "--porcelain"], capture_output=True, text=True, errors="replace", timeout=60).stdout
        return "%s (%d changed paths)" % (head, len([l for l in dirty.splitlines() if l.strip()]))
    except Exception as e:  # noqa: BLE001
        return "git unreadable: %s" % e


def analyse(jobs, folder, results, header, ignore_errors, expected_removed, pbinds):
    """Parse every job, evaluate runs (with the exit codes in results when known), check pairs and controls, write the summary."""
    parsed = {j["name"]: parse_log(j["log"]) for j in jobs}
    for j in jobs:
        known = results.get(j["name"])
        code = known["attempts"][-1]["exit"] if known else None
        why = run_failures(j, parsed[j["name"]], code, ignore_errors)
        if known:
            known["ok"] = not why
            known["why_now"] = why
        else:
            results[j["name"]] = {"ok": not why, "attempts": [], "why_now": why}
    by_name = {j["name"]: j for j in jobs}
    pairs, quads = [], []
    for j in jobs:
        if j["off"]:
            continue
        off = by_name.get(j["name"] + "_off")
        if not off:
            continue
        pr = pair_check(j, off, parsed[j["name"]], parsed[off["name"]])
        if not (results[j["name"]]["ok"] and results[off["name"]]["ok"]):
            pr["ok"] = False
            pr["why"].append("a run of the pair failed")
        pairs.append(pr)
        quads.append((j, off, parsed[j["name"]], parsed[off["name"]], pr))
    controls = switch_controls(quads, expected_removed)
    header["end_check"] = sum(p.get("end_check", 0) for p in parsed.values())
    header["race_check"] = sum(p.get("race_check", 0) for p in parsed.values())
    header["failed"] = sorted(n for n, r in results.items() if n in by_name and not r["ok"])
    header["processes"] = sum(len(r["attempts"]) for n, r in results.items() if n in by_name)
    header["reruns"] = sum(max(0, len(r["attempts"]) - 1) for n, r in results.items() if n in by_name)
    header["jobs"] = len(jobs)
    recompute = recompute_45(by_name, parsed, pbinds=pbinds) if any(j["cell"] == "R" for j in jobs) else None
    summary = summarize(jobs, parsed, results, pairs, controls, header, recompute)
    with open(os.path.join(folder, "summary.json"), "w", encoding="utf-8") as f:
        json.dump(jsonable(summary), f, ensure_ascii=False, indent=1)
    md = markdown(summary)
    with open(os.path.join(folder, "summary.md"), "w", encoding="utf-8") as f:
        f.write(md)
    ok = (not header["failed"]) and controls["ok"] and all(p["ok"] for p in pairs) and header["end_check"] == 0 and header["race_check"] == 0 \
        and header.get("sha1_ok", True)
    return ok, summary, md


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=os.environ.get("PROJECT_ROOT", DEFAULT_ROOT))
    ap.add_argument("--folder", default=None)
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--mem-per-worker-gb", type=float, default=2.5, help="free memory asked per worker above 6 workers")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--check-only", metavar="FOLDER", default=None)
    ap.add_argument("--cells", default=None, help="comma list of R,inputless,wanderer (default all)")
    ap.add_argument("--max-seed", type=int, default=None)
    ap.add_argument("--ignore-error", action="append", default=[], help="regex of a LogCosmic: Error line that does not fail a run")
    ap.add_argument("--expect-removed", default=EXPECTED_REMOVED)
    ap.add_argument("--pbind", default="1800", help="comma list of P_bind values for 4.5 (default 1800 = the start value, H:429)")
    ap.add_argument("--editor", default=EDITOR)
    a = ap.parse_args(argv)
    cells = set(a.cells.split(",")) if a.cells else None
    pbinds = tuple(int(x) for x in a.pbind.split(","))
    project = os.path.join(a.root, "Cosmic_Ascension.uproject")
    folder = os.path.abspath(a.check_only or a.folder or os.path.join(a.root, "Saved", "Balance", "b6b_bundleA"))
    jobs = build_jobs(folder, cells, a.max_seed)

    if a.dry_run:
        for j in jobs:
            print(quote_cmd(command_of(j, project, a.editor)))
        n20 = sum(1 for j in jobs if j["ticks"] == 20000)
        print("# %d jobs (%d at 20,000 ticks first, %d at 6,000), %d workers, folder %s" % (len(jobs), n20, len(jobs) - n20, a.workers, folder))
        return 0

    header = {"date": datetime.date.today().isoformat(), "folder": folder, "git": git_head(a.root)}
    results_path = os.path.join(folder, "runs.json")

    if a.check_only:
        results = {}
        if os.path.exists(results_path):
            with open(results_path, encoding="utf-8") as f:
                results = json.load(f)
        meta_path = os.path.join(folder, "bundle_meta.json")
        meta = {}
        if os.path.exists(meta_path):
            with open(meta_path, encoding="utf-8") as f:
                meta = json.load(f)
        header.update({k: meta[k] for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git") if k in meta})
        header.setdefault("sha1_guard", "not recorded (check-only without bundle_meta.json)")
        ok, _, md = analyse(jobs, folder, results, header, a.ignore_error, a.expect_removed, pbinds)
        print(md)
        print("bundle A check-only: %s" % ("PASS" if ok else "FAIL"))
        return 0 if ok else 1

    busy = busy_processes()
    if busy:
        print("refusing to start - running now:\n  " + "\n  ".join(busy))
        return 2
    avail, total = free_memory_gb()
    print("free memory %s GB of %s GB" % (fmt(round(avail, 1)) if avail else "?", fmt(round(total, 1)) if total else "?"))
    if a.workers > 6 and (avail is None or avail < a.workers * a.mem_per_worker_gb):
        print("refusing %d workers: free memory does not cover %d x %.1f GB (brief 4.2: 8 only when memory covers them)" % (a.workers, a.workers, a.mem_per_worker_gb))
        return 2
    if not os.path.exists(a.editor) or not os.path.exists(project):
        print("no editor or project: %s / %s" % (a.editor, project))
        return 2
    files = guarded_files(a.root)
    if not any(f.lower().endswith(".dll") for f in files):
        print("no UnrealEditor-Cosmic_Ascension*.dll under Binaries/Win64 - build first")
        return 2
    before = sha1_files(files)
    os.makedirs(folder, exist_ok=True)
    dll = [h for p, h in before.items() if p.lower().endswith(".dll")]
    header["dll_sha1"] = " ".join(sorted(h[:12] for h in dll))
    results, lock = {}, threading.Lock()
    t0 = time.time()
    print("bundle A: %d jobs, %d workers, folder %s" % (len(jobs), a.workers, folder), flush=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
        futures = [pool.submit(execute, j, project, a.editor, a.ignore_error, lock, results, results_path) for j in jobs]
        for done, f in enumerate(concurrent.futures.as_completed(futures), 1):
            name, attempts = f.result()
            last = attempts[-1]
            print("[%d/%d %.0fs] %s %s%s" % (done, len(jobs), time.time() - t0, name, "ok" if not last["why"] else "FAILED: " + "; ".join(last["why"]),
                                            " (rerun)" if len(attempts) > 1 else ""), flush=True)
    after = sha1_files(guarded_files(a.root))
    changed = sha1_diff(before, after)
    header["sha1_ok"] = not changed
    header["sha1_guard"] = ("unchanged (%d files)" % len(before)) if not changed else "CHANGED - discard the bundle: " + "; ".join(changed)
    header["minutes"] = round((time.time() - t0) / 60.0, 1)
    with open(os.path.join(folder, "bundle_meta.json"), "w", encoding="utf-8") as f:
        json.dump({k: header[k] for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes")}, f, ensure_ascii=False, indent=1)
    ok, _, md = analyse(jobs, folder, results, header, a.ignore_error, a.expect_removed, pbinds)
    print(md)
    print("bundle A: %s in %.1f min" % ("PASS" if ok else "FAIL", header["minutes"]))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
