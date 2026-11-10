"""[B-6b8b] bundle K (brief section 4): 300 commandlet runs of CosmicBalance - 150 on the new build's game data
(FleetDelayCountdownShare 0.5) and 150 share-0 twins - the pair check of the two-sided rule (section 3 (d) (i)-(iii)),
the switch controls, the sha1 guard on the module DLL and DT_Rules.uasset, and K1-K3.

A copy of scratchpad\\b6b6\\bundleA.py (its runner, guards, log readers and CSV reader kept as they are); the analysis of
bundle A (section 4.5 of [B-6b6]) is replaced by bundle K's. Stdlib only.

Run with `python -I -X utf8 bundleK.py [options]`, the editor closed and nothing else building.
  --root <project>        project root (default: this PC's)
  --folder <dir>          output folder (default <root>\\Saved\\Balance\\b6b8b_bundleK)
  --workers N             ThreadPoolExecutor workers (default 6; 4 when free memory is under 8 GB)
  --dry-run               print the commands, run nothing, write nothing
  --check-only <dir>      re-parse the outputs already in <dir> (no commandlet) and write the summary there
  --cells R,inputless,wanderer and --max-seed N   a subset (for a smoke run)

The twin's rules are ONE argument -Rules=ProvisionalEndTick:0;FleetDelayCountdownShare:0: FParse::Value(..."Rules="...)
(CosmicBalanceCommandlet.cpp:1485 at 84f0d36) reads only the first -Rules=.
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

COMMON_ARGS = ["-Rules=ProvisionalEndTick:0"]
TWIN_RULES = ["-Rules=ProvisionalEndTick:0;FleetDelayCountdownShare:0"]
R_ARGS = ["-Answer=1", "-Build=1", "-Focus=1", "-Edict=1", "-Diplomacy=1", "-TellAll=0"]
WANDERER_ARGS = R_ARGS + ["-Abdicate=1200", "-Wander=1"]
# (cell, ticks, args, seeds) - brief section 4: R 60 x 20,000, input-less 30 x 20,000, wanderer 60 x 6,000.
CELLS = [
    ("R", 20000, R_ARGS, range(1, 61)),
    ("inputless", 20000, [], range(1, 31)),
    ("wanderer", 6000, WANDERER_ARGS, range(1, 61)),
]
IGNORED_COLUMNS = ("TickMicros", "SaveBytes", "SaveRawBytes")
AWAKE = "Hazard.AIAwakening"
NEW_CAP = 75          # 0.5 x 150 (the awakening's cap at the game's share)
AWAKE_COUNTDOWN = 150
RULE_END = "CosmicBalance: rules ProvisionalEndTick = 0"
RULE_SHARE = "CosmicBalance: rules FleetDelayCountdownShare = 0"
RULE_REFUSED = "-Rules cannot set"
NOT_MEASURED = "재지 않음 — 잰 %d"

# ----------------------------------------------------------------------------------------------------------------- jobs


def build_jobs(folder, cells=None, max_seed=None):
    """The bundle's jobs, 20,000-tick jobs first; each seed's on run and its share-0 twin next to each other."""
    jobs = []
    for cell, ticks, args, seeds in sorted(CELLS, key=lambda c: -c[1]):
        if cells and cell not in cells:
            continue
        for seed in seeds:
            if max_seed is not None and seed > max_seed:
                continue
            for twin in (False, True):
                name = "%s_%d%s" % (cell, seed, "_twin" if twin else "")
                base = os.path.join(folder, name)
                jobs.append({
                    "name": name, "cell": cell, "seed": seed, "ticks": ticks, "off": twin,
                    "args": (TWIN_RULES if twin else COMMON_ARGS) + list(args),
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


def guarded_files_k(root):
    """Brief section 4: the module DLL(s) and DT_Rules.uasset."""
    files = glob.glob(os.path.join(root, "Binaries", "Win64", "UnrealEditor-Cosmic_Ascension*.dll"))
    files += [os.path.join(root, "Content", "CA", "Data", "DT_Rules.uasset")]
    return sorted(f for f in files if os.path.exists(f))

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


def tag_sum_count(text):
    """'Tag=sum(count) Tag=sum(count)' into {tag: (sum, count)}."""
    out = {}
    for m in re.finditer(r"(\S+?)=(-?\d+)\((\d+)\)", text or ""):
        out[m.group(1)] = (int(m.group(2)), int(m.group(3)))
    return out


def tag_eq(text):
    """'Tag=n Tag=n' into {tag: n}."""
    return {m.group(1): int(m.group(2)) for m in re.finditer(r"(\S+?)=(-?\d+)", text or "")}


def tag_slash(text):
    """'Tag:a/b[/c] ...' into {tag: [a, b, (c)]}."""
    return {m.group(1): [int(x) for x in m.group(2).split("/")] for m in re.finditer(r"(\S+?):(\d+(?:/\d+)+)", text or "")}


def counts_text(text):
    toks = text.split()
    if toks == ["none"]:
        return {}
    return {toks[i]: int(toks[i + 1]) for i in range(0, len(toks) - 1, 2)}


def parse_log(path):
    """Everything bundle K reads from one commandlet log."""
    p = {"log_exists": os.path.exists(path)}
    if not p["log_exists"]:
        return p
    text = read_text(path)
    m = last_match(r"CosmicBalance: seed (\d+), (\d+) ticks, hash (\d+), world (\d+)", text)
    p["seed_line"] = {"seed": int(m.group(1)), "ticks": int(m.group(2)), "hash": int(m.group(3)), "world": int(m.group(4))} if m else None
    m = last_match(r"CosmicBalance race: (.*?)\r?$", text)
    p["race_text"] = m.group(1) if m else None
    m = last_match(r"CosmicBalance ending: (.*?)\r?$", text)
    p["ending_text"] = m.group(1) if m else None
    p["end_check"] = len(re.findall(r"CosmicBalance end check", text))
    p["race_check"] = len(re.findall(r"CosmicBalance race check", text))
    p["errors"] = [l.strip() for l in text.splitlines() if "LogCosmic: Error" in l]
    p["rule_end"] = text.count(RULE_END)
    p["rule_share"] = text.count(RULE_SHARE)
    p["rule_refused"] = text.count(RULE_REFUSED)
    m = last_match(r"CosmicBalance conflict: .*?delay bought per tag \[([^\]]*)\]", text)
    p["delay_bought"] = tag_sum_count(m.group(1)) if m else None
    m = last_match(r"CosmicBalance pressure: .*?guarded crises (\d+) by tag \[([^\]]*)\] \(by end (\d+)\), delay bought (\d+) by tag \[([^\]]*)\], "
                   r"ends of guarded crises: answered (\d+), expired (\d+), resolved by an edict (\d+)", text)
    p["pressure"] = {"guarded": int(m.group(1)), "guarded_by_tag": tag_eq(m.group(2)), "delay": int(m.group(4)), "delay_by_tag": tag_eq(m.group(5)),
                     "answered": int(m.group(6)), "expired": int(m.group(7)), "edict": int(m.group(8))} if m else None
    m = last_match(r"#132 by the record: guarded at the row's cap by tag \[([^\]]*)\], guarded ends by tag \[([^\]]*)\], ruled unguarded ends by tag \[([^\]]*)\]", text)
    p["at_cap"] = tag_slash(m.group(1)) if m else None
    p["guarded_ends"] = tag_slash(m.group(2)) if m else None
    m = last_match(r"CosmicBalance hazards: placed(.*?) \| answered", text)
    p["placed"] = counts_text(m.group(1)) if m else None
    return p


def run_failures(job, parsed, exit_code=None, ignore_errors=()):
    """bundle A's fail rules (exit code, log, seed/race/ending lines, check lines, errors, output files, ticks) plus the lines
    bundle K reads."""
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
    if not parsed.get("ending_text"):
        why.append("no 'ending:' line")
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
    if parsed.get("delay_bought") is None:
        why.append("no 'delay bought per tag' list")
    if parsed.get("pressure") is None:
        why.append("no 'pressure:' guarded crises cells")
    if parsed.get("at_cap") is None:
        why.append("no '#132 by the record' part")
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


def pair_check(on_job, twin_job, on_p, twin_p):
    """Section 3 (d) / K1 / K2: the whole CSV and _rows (three timing columns aside) and both hashes, against the twin's
    conflict entry Hazard.AIAwakening=<sum>(<count>)."""
    res = {"pair": on_job["name"], "cell": on_job["cell"], "seed": on_job["seed"], "ok": True, "why": [], "differs": False,
           "first_diff_csv": None, "first_diff_rows": None, "hash_differs": None, "twin_awake": None, "class": None}
    for key in ("csv", "rows"):
        if not (os.path.exists(on_job[key]) and os.path.exists(twin_job[key])):
            res["why"].append("missing %s file" % key)
            continue
        h_on, a = read_csv_by_tick(on_job[key])
        h_tw, b = read_csv_by_tick(twin_job[key])
        if h_on != h_tw:
            res["why"].append("%s headers differ" % key)
            res["differs"] = True
        d = first_difference(a, b)
        res["first_diff_" + key] = d
        if d is not None:
            res["differs"] = True
    s_on, s_tw = on_p.get("seed_line"), twin_p.get("seed_line")
    if s_on and s_tw:
        res["hash_differs"] = (s_on["hash"], s_on["world"]) != (s_tw["hash"], s_tw["world"])
        res["differs"] = res["differs"] or res["hash_differs"]
    aw = (twin_p.get("delay_bought") or {}).get(AWAKE)
    res["twin_awake"] = aw
    if aw is None or aw[0] <= NEW_CAP:
        res["class"] = "iii"   # must be identical
        if res["differs"]:
            res["why"].append("moved without the twin's %s above %d (twin %s) - (i)/(iii) broken" % (AWAKE, NEW_CAP, aw))
    elif aw[0] > NEW_CAP * aw[1]:
        res["class"] = "ii"    # must differ
        if not res["differs"]:
            res["why"].append("twin %s=%d(%d) > 75 x count, but the pair is identical - (ii) broken" % (AWAKE, aw[0], aw[1]))
    else:
        res["class"] = "may"   # sum > 75, sum <= 75 x count: may move
    firsts = [t for t in (res["first_diff_csv"], res["first_diff_rows"]) if t is not None]
    res["first_moved_sample"] = min(firsts) if firsts else None
    res["ok"] = not res["why"]
    return res


def switch_controls(jobs, parsed):
    """Section 4: every twin log has both rules lines, every on log the first and not the second, none '-Rules cannot set'."""
    problems = []
    for j in jobs:
        p = parsed[j["name"]]
        if not p.get("log_exists"):
            continue
        if p["rule_refused"]:
            problems.append("%s: '-Rules cannot set'" % j["name"])
        if p["rule_end"] < 1:
            problems.append("%s: no '%s'" % (j["name"], RULE_END))
        if j["off"] and p["rule_share"] < 1:
            problems.append("%s: no '%s'" % (j["name"], RULE_SHARE))
        if not j["off"] and p["rule_share"]:
            problems.append("%s: an on log has '%s'" % (j["name"], RULE_SHARE))
    return {"ok": not problems, "problems": problems}

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

def fmt(v):
    if v is None:
        return "-"
    if isinstance(v, float) and math.isinf(v):
        return "inf"
    if isinstance(v, float):
        return ("%.1f" % v) if v != int(v) else "%d" % v
    return str(v)


def sum_awake(items, key):
    tot = [0, 0, 0]
    for p in items:
        v = (p.get(key) or {}).get(AWAKE)
        if v:
            for i, x in enumerate(v):
                tot[i] += x
    return tot


def k_summary(jobs, parsed, results, pairs, controls, header):
    by_name = {j["name"]: j for j in jobs}
    out = {"header": header, "controls": controls, "pairs": pairs, "runs": results, "cells": {}}
    for cell in ("R", "inputless", "wanderer"):
        cp = [p for p in pairs if p["cell"] == cell]
        if not cp:
            continue
        on = [parsed[j["name"]] for j in jobs if j["cell"] == cell and not j["off"]]
        tw = [parsed[j["name"]] for j in jobs if j["cell"] == cell and j["off"]]
        c = {"pairs": len(cp), "moved": sum(1 for p in cp if p["differs"]), "failed": [p["pair"] for p in cp if not p["ok"]],
             "class": {k: sum(1 for p in cp if p["class"] == k) for k in ("ii", "may", "iii")},
             "moved_by_class": {k: sum(1 for p in cp if p["class"] == k and p["differs"]) for k in ("ii", "may", "iii")},
             "moved_pairs": [{"seed": p["seed"], "first_moved_sample": p["first_moved_sample"], "twin_awake": p["twin_awake"],
                              "hash_differs": p["hash_differs"]} for p in cp if p["differs"]],
             "twins_listing_awake": sum(1 for p in tw if (p.get("delay_bought") or {}).get(AWAKE)),
             "twin_awake_delay_sum_count": [sum(((p.get("delay_bought") or {}).get(AWAKE) or (0, 0))[i] for p in tw) for i in (0, 1)],
             "on_awake_delay_sum_count": [sum(((p.get("delay_bought") or {}).get(AWAKE) or (0, 0))[i] for p in on) for i in (0, 1)],
             "on_at_cap": sum_awake(on, "at_cap")[:2], "twin_at_cap": sum_awake(tw, "at_cap")[:2],
             "on_guarded_ends": sum_awake(on, "guarded_ends"), "twin_guarded_ends": sum_awake(tw, "guarded_ends"),
             "on_pressure_guarded_awake": sum((p.get("pressure") or {}).get("guarded_by_tag", {}).get(AWAKE, 0) for p in on),
             "twin_pressure_guarded_awake": sum((p.get("pressure") or {}).get("guarded_by_tag", {}).get(AWAKE, 0) for p in tw),
             "on_pressure_delay_awake": sum((p.get("pressure") or {}).get("delay_by_tag", {}).get(AWAKE, 0) for p in on),
             "twin_pressure_delay_awake": sum((p.get("pressure") or {}).get("delay_by_tag", {}).get(AWAKE, 0) for p in tw),
             "on_pressure_ends_all_tags": [sum((p.get("pressure") or {}).get(k, 0) for p in on) for k in ("answered", "expired", "edict")],
             "twin_pressure_ends_all_tags": [sum((p.get("pressure") or {}).get(k, 0) for p in tw) for k in ("answered", "expired", "edict")],
             "on_placed_awake": sum((p.get("placed") or {}).get(AWAKE, 0) for p in on),
             "twin_placed_awake": sum((p.get("placed") or {}).get(AWAKE, 0) for p in tw)}
        g = c["on_pressure_guarded_awake"]
        c["on_guarded_awake_delay_share"] = (c["on_pressure_delay_awake"] / float(g * AWAKE_COUNTDOWN)) if g else None
        out["cells"][cell] = c
    tot_twin_list = sum(c["twins_listing_awake"] for c in out["cells"].values())
    out["K1"] = None
    if "inputless" in out["cells"]:
        nl = out["cells"]["inputless"]
        out["K1"] = {"ok": not nl["failed"], "moved": nl["moved"], "pairs": nl["pairs"],
                     "risk2": [m for m in nl["moved_pairs"] if m["twin_awake"] and m["twin_awake"][0] > NEW_CAP]}
    k2 = [c for k, c in out["cells"].items() if k in ("R", "wanderer")]
    out["K2"] = {"ok": all(not c["failed"] for c in k2), "failed": sum((c["failed"] for c in k2), [])}
    out["K3"] = {"K_MOVED_R": "%d/%d" % (out["cells"]["R"]["moved"], out["cells"]["R"]["pairs"]) if "R" in out["cells"] else None,
                 "K_MOVED_W": "%d/%d" % (out["cells"]["wanderer"]["moved"], out["cells"]["wanderer"]["pairs"]) if "wanderer" in out["cells"] else None,
                 "no_guarded_awakening": tot_twin_list == 0}
    shares = [c["on_guarded_awake_delay_share"] for c in out["cells"].values() if c["on_guarded_awake_delay_share"] is not None]
    out["K3"]["guard_lost_its_fleet"] = any(s < 0.10 for s in shares) if shares else None
    return out


def markdown(s):
    h = s["header"]
    L = ["# [B-6b8b] 묶음 K — %s" % h["date"], ""]
    L.append("- 빌드: %s; DLL sha1 %s" % (h.get("git"), h.get("dll_sha1")))
    L.append("- 프로세스: %d회(작업 %d, 다시 돌림 %d), %s분" % (h["processes"], h["jobs"], h["reruns"], h.get("minutes", "-")))
    L.append("- 실패한 판: %d (%s)" % (len(h["failed"]), ", ".join(h["failed"]) or "없음"))
    L.append("- 폴더: `%s`" % h["folder"])
    L.append("- end check %d, race check %d" % (h["end_check"], h["race_check"]))
    L.append("- 스위치 대조: %s%s" % ("통과" if s["controls"]["ok"] else "실패", "" if s["controls"]["ok"] else " — " + "; ".join(s["controls"]["problems"][:10])))
    L.append("- sha1 지킴: %s" % h["sha1_guard"])
    L.append("")
    for cell, c in s["cells"].items():
        L.append("## %s (쌍 %d)" % (cell, c["pairs"]))
        for k, v in c.items():
            if k in ("pairs",):
                continue
            L.append("- %s: %s" % (k, v))
        L.append("")
    L.append("## 판단")
    L.append("- K1: %s" % s["K1"])
    L.append("- K2: %s" % s["K2"])
    L.append("- K3: %s" % s["K3"])
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


def analyse(jobs, folder, results, header, ignore_errors):
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
    by_name = {j["name"]: j for j in jobs}
    pairs = []
    for j in jobs:
        if j["off"]:
            continue
        tw = by_name.get(j["name"] + "_twin")
        if not tw:
            continue
        pr = pair_check(j, tw, parsed[j["name"]], parsed[tw["name"]])
        if not (results[j["name"]]["ok"] and results[tw["name"]]["ok"]):
            pr["ok"] = False
            pr["why"].append("a run of the pair failed")
        pairs.append(pr)
    controls = switch_controls(jobs, parsed)
    header["end_check"] = sum(p.get("end_check", 0) for p in parsed.values())
    header["race_check"] = sum(p.get("race_check", 0) for p in parsed.values())
    header["failed"] = sorted(n for n, r in results.items() if n in by_name and not r["ok"])
    header["processes"] = sum(len(r["attempts"]) for n, r in results.items() if n in by_name)
    header["reruns"] = sum(max(0, len(r["attempts"]) - 1) for n, r in results.items() if n in by_name)
    header["jobs"] = len(jobs)
    summary = k_summary(jobs, parsed, results, pairs, controls, header)
    with open(os.path.join(folder, "summary.json"), "w", encoding="utf-8") as f:
        json.dump(summary, f, ensure_ascii=False, indent=1)
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
    ap.add_argument("--workers", type=int, default=None)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--check-only", metavar="FOLDER", default=None)
    ap.add_argument("--cells", default=None)
    ap.add_argument("--max-seed", type=int, default=None)
    ap.add_argument("--ignore-error", action="append", default=[])
    ap.add_argument("--editor", default=EDITOR)
    a = ap.parse_args(argv)
    cells = set(a.cells.split(",")) if a.cells else None
    project = os.path.join(a.root, "Cosmic_Ascension.uproject")
    folder = os.path.abspath(a.check_only or a.folder or os.path.join(a.root, "Saved", "Balance", "b6b8b_bundleK"))
    jobs = build_jobs(folder, cells, a.max_seed)
    avail, total = free_memory_gb()
    workers = a.workers or (6 if (avail is not None and avail >= 8.0) else 4)

    if a.dry_run:
        for j in jobs:
            print(quote_cmd(command_of(j, project, a.editor)))
        n20 = sum(1 for j in jobs if j["ticks"] == 20000)
        print("# %d jobs (%d at 20,000 ticks first, %d at 6,000), %d workers (free %.1f GB), folder %s" % (len(jobs), n20, len(jobs) - n20, workers, avail or -1, folder))
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
        header.update({k: meta[k] for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes", "workers") if k in meta})
        header.setdefault("sha1_guard", "not recorded")
        ok, _, md = analyse(jobs, folder, results, header, a.ignore_error)
        print(md)
        print("bundle K check-only: %s" % ("PASS" if ok else "FAIL"))
        return 0 if ok else 1

    busy = busy_processes()
    if busy:
        print("refusing to start - running now:\n  " + "\n  ".join(busy))
        return 2
    print("free memory %s GB of %s GB -> %d workers" % (fmt(round(avail, 1)) if avail else "?", fmt(round(total, 1)) if total else "?", workers))
    if not os.path.exists(a.editor) or not os.path.exists(project):
        print("no editor or project: %s / %s" % (a.editor, project))
        return 2
    files = guarded_files_k(a.root)
    if not any(f.lower().endswith(".dll") for f in files) or not any(f.endswith("DT_Rules.uasset") for f in files):
        print("no module DLL or DT_Rules.uasset - build first")
        return 2
    before = sha1_files(files)
    os.makedirs(folder, exist_ok=True)
    header["dll_sha1"] = " ".join(sorted(h[:12] for p, h in before.items() if p.lower().endswith(".dll")))
    header["rules_sha1"] = " ".join(h[:12] for p, h in before.items() if p.endswith("DT_Rules.uasset"))
    header["workers"] = workers
    results, lock = {}, threading.Lock()
    t0 = time.time()
    print("bundle K: %d jobs, %d workers, folder %s" % (len(jobs), workers, folder), flush=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futures = [pool.submit(execute, j, project, a.editor, a.ignore_error, lock, results, results_path) for j in jobs]
        for done, f in enumerate(concurrent.futures.as_completed(futures), 1):
            name, attempts = f.result()
            last = attempts[-1]
            print("[%d/%d %.0fs] %s %s%s" % (done, len(jobs), time.time() - t0, name, "ok" if not last["why"] else "FAILED: " + "; ".join(last["why"]),
                                            " (rerun)" if len(attempts) > 1 else ""), flush=True)
    after = sha1_files(guarded_files_k(a.root))
    changed = sha1_diff(before, after)
    header["sha1_ok"] = not changed
    header["sha1_guard"] = ("unchanged (%d files: %s)" % (len(before), ", ".join(os.path.basename(p) for p in sorted(before)))) if not changed \
        else "CHANGED - discard the bundle: " + "; ".join(changed)
    header["minutes"] = round((time.time() - t0) / 60.0, 1)
    with open(os.path.join(folder, "bundle_meta.json"), "w", encoding="utf-8") as f:
        json.dump({k: header[k] for k in ("dll_sha1", "rules_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes", "workers")}, f, ensure_ascii=False, indent=1)
    ok, _, md = analyse(jobs, folder, results, header, a.ignore_error)
    print(md)
    print("bundle K: %s in %.1f min" % ("PASS" if ok else "FAIL", header["minutes"]))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
