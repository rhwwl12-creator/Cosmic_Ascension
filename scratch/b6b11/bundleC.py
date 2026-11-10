"""[B-6b11] bundle C (brief scratchpad\\b6b11\\brief_final.md sections 1.2, 3, 4.1-4.4): 620 commandlet runs of CosmicBalance
in game form on the build of the last code commit (560 when the forced death comes after the clock) - R, slowR, death and
wanderer at the clock C; input-less (with its -WithoutB6bData=1 pair), the 0.75 cell and the test cell at 20,000 - the run
failures, the field controls F1-F8, the bridges B-a..B-e, the frozen arithmetic R1-R11 of brief 4.3, and the hand-check seed
selection of 4.4 (selection only - [B-6b11] makes the saves itself, one at a time).

Loads scratchpad\\b6b8\\bundleB.py through BUNDLE_B_PY (default ..\\b6b8\\bundleB.py) with importlib; bundleB loads
scratchpad\\b6b6\\bundleA.py through BUNDLE_A_PY. Neither is edited. bundleB.build_jobs is NOT used for C (its COMMON_ARGS
carry -Rules=ProvisionalEndTick:0): bundle C has its own build_jobs, no common arguments, at most one -Rules= per job.

Run with `python -I -X utf8 bundleC.py [options]`, the editor closed and nothing else building. Stdlib only.

Placeholders (no default - the script refuses to run without them, in every mode):
  --c N                 <C>, ProvisionalEndTick; must equal git show <BUILD_SHA>:Tools/data/DT_Rules.json
  --t-death N           <T_DEATH> (N section 3 '묶음 B'); T_DEATH >= C drops the death cell (560 jobs)
  --build-sha SHA       <BUILD_SHA>, the last commit that touched Source/, Tools/data or Content/CA/Data
  --r X                 <R_COMMITTED> AIUpperRungProgressMultiplier      --share X  <SHARE_COMMITTED> (default 0.5, must be 0.5)
  --chance X            <CHANCE> EventChancePerSlot                      --w-eased X  <W_EASED> event_claim_eased BaseWeight
  --ratio X             <RATIO> AIHostilePowerRatio
  --b9-ran yes|no       <B9_RAN>        --br-world none|<text>  <BR_WORLD>
  --k-moved-r none|1,5  --k-moved-w none|3  <K_MOVED> (bundle K's moved R and wanderer seeds)
Every value is checked against the files at <BUILD_SHA> (git show, read only) before anything runs.

Options:
  --folder <dir>        outputs (default scratchpad\\b6b11\\bundleC - outside OneDrive)
  --archive-dir <dir>   default <root>\\Saved\\Balance\\b6b_bundleC; --archive / --no-archive (default on after a run)
  --workers N           default 6           --dry-run    print the commands, the count and the wall-time estimate
  --check-only <dir>    re-read the outputs of <dir> and judge (no commandlet)
  --cell-set B|C        with --check-only: B = bundle B's logs (parser exercise on real text, brief 2), C = bundle C
  --bundle-b-logs <dir> default <root>\\Saved\\Balance\\b6b_bundleB (logs + manifest.sha1)
  --bundle-b-csv <dir>  default scratchpad\\b6b8\\bundleB (<BUNDLE_B_CSV> is true only if every CSV matches the manifest)
  --bundle-k-dir <dir>  recorded in the summary (the K seeds come from --k-moved-r/--k-moved-w)
  --b6a-dir <dir>       default <root>\\Saved\\Balance\\b6a_bundle1 (decision 36 bridge B-c)
  --frozen <file>       default scratchpad\\b6b11\\frozen_4_3.md (hashed with the harness files before job 1)
  --a-sec-per-20k S     wall seconds per 20,000-tick run at 6 workers, for <C_MINUTES> in the dry run
  --cells, --max-seed   a subset (smoke run, self-test)

Formats: Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp (git show) - the ending line (first-person claim lines,
#132 guarded, #135 keys), the conflict line (taken, bands, top conqueror, lost max, delay bought per tag), the wanderer line,
the calling deaths line (deaths by end, first death, successions :D<n>), the pressure line (policy head, guarded/delay by tag,
the 401-550 band), the race line's #132 by-the-record part, the rules line, the claims file.
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
import statistics
import subprocess
import sys
import threading
import time

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:  # noqa: BLE001
    pass

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_BUNDLE_B_PY = os.path.join(os.path.dirname(HERE), "b6b8", "bundleB.py")


def _load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


B = _load("bundleB", os.environ.get("BUNDLE_B_PY", DEFAULT_BUNDLE_B_PY))
A = B.A

DEFAULT_ROOT = A.DEFAULT_ROOT
EDITOR = A.EDITOR
HORIZON = 20000
ABDICATE_AT = 1200
B1_CUT = 6000
B6A_CUT = 6000
R_ARGS = list(A.R_ARGS)
assert R_ARGS == ["-Answer=1", "-Build=1", "-Focus=1", "-Edict=1", "-Diplomacy=1", "-TellAll=0"], R_ARGS
SLOW_ARGS = ["-Focus=0" if a == "-Focus=1" else a for a in R_ARGS]
WANDERER_ARGS = R_ARGS + ["-Abdicate=1200", "-Wander=1"]
TEST_ARGS = R_ARGS + ["-Rules=FallenCoreMinLanesFromHome:1", "-LeaveNearSphere=1", "-PlayerPolicy=Faithful", "-Guard=1"]
OFF_ARG = A.OFF_ARG
# 20,000-tick cells first (the pool packs better with the long runs first); each seed's on and off run next to each other.
CELL_ORDER = ("inputless", "inputless075", "test", "R", "slowR", "death", "wanderer")
CELL_ARGS = {"R": R_ARGS, "slowR": SLOW_ARGS, "death": R_ARGS, "wanderer": WANDERER_ARGS, "inputless": [],
             "inputless075": ["-Rules=AIHostilePowerRatio:0.75"], "test": TEST_ARGS}
AT_CLOCK = ("R", "slowR", "death", "wanderer")
HAS_OFF = {"wanderer": True, "inputless": True}
SEEDS = {"test": range(1, 141)}
DEFAULT_SEEDS = range(1, 61)
EXPECTED_RULES = {"test": {"FallenCoreMinLanesFromHome": "1"}, "inputless075": {"AIHostilePowerRatio": "0.75"}}
EXPECTED_REMOVED = B.EXPECTED_REMOVED
ENDING_KINDS = B.ENDING_KINDS
KINDS_R8 = ("RuledPeople", "OtherPeople", "Clock", "NoCandidate", "Vessel")
NOT_MEASURED = B.NOT_MEASURED                                     # "재지 않음 — 잰 %d"
DEATH_DROPPED = "재지 않음 — 강제 죽음(%d)이 시계(%d) 뒤"          # brief 0.2, verbatim
NONE_KO = "없음"
DEV = "[개발자]"
AWAKENING = "Hazard.AIAwakening"
PREDICTOR_SUM = 75                                                # bundle K's rule (b6b8b brief 3 (d)(i))
CLAIM_ENDS = ("joined", "eased", "other", "expired")
BAND_401_550 = (401, 550)
SLOTS_MID = 47.5                                                  # N (f): the band midpoint in slots
SLOT_TICKS = 10
C_RANGE = (6000, 18000)                                           # rule (d), H:349
# B6a bundle 1's on side, the own-vessel reading (B6a N:298) - bridge B-e (the off side must reproduce it).
B6A_B1 = {"own": 40, "n": 60, "median": 1350, "paths": {"return": 22, "vacuum": 15, "reverence": 3}}
PATH_KO = {"return": "복귀", "vacuum": "빈자리", "reverence": "경외"}
B3_129 = "B3: 143 -> 1, 50 after 6,000 (OQ:98)"
LATE_154 = 4000

# ----------------------------------------------------------------------------------------- swappable externals (self-test)


# The commandlet children this bundle has running (their UnrealEditor-Cmd.exe is not 'another job' for the busy check).
# Launch-and-record and the busy check take the same lock, so no child is listed by tasklist before its pid is recorded.
OWN = {"pids": set(), "lock": threading.Lock()}


def _popen(cmd):
    return subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def _run_process(cmd):
    with OWN["lock"]:
        proc = HOOKS["popen"](cmd)
        OWN["pids"].add(proc.pid)
    try:
        return proc.wait()
    finally:
        with OWN["lock"]:
            OWN["pids"].discard(proc.pid)


def _busy_others():
    """bundleA.busy_processes without this bundle's own commandlet children (matched by '(pid N)')."""
    with OWN["lock"]:
        own = set(OWN["pids"])
        found = HOOKS["busy_all"]()
    return [b for b in found if not any("(pid %d)" % p in b for p in own)]


def _git_show(root, rev, path):
    """git show <rev>:<path> (read only, no optional locks); None when it fails."""
    try:
        r = subprocess.run(["git", "--no-optional-locks", "-C", root, "show", "%s:%s" % (rev, path)], capture_output=True, text=True,
                           encoding="utf-8", errors="replace", timeout=60)
        return r.stdout if r.returncode == 0 else None
    except Exception:  # noqa: BLE001
        return None


def _git_head(root):
    try:
        return subprocess.run(["git", "--no-optional-locks", "-C", root, "log", "--oneline", "-1"], capture_output=True, text=True,
                              errors="replace", timeout=30).stdout.strip()
    except Exception as e:  # noqa: BLE001
        return "git unreadable: %s" % e


HOOKS = {"run": _run_process, "popen": _popen, "busy": _busy_others, "busy_all": A.busy_processes, "memory": A.free_memory_gb, "git_show": _git_show, "git_head": _git_head}

# ----------------------------------------------------------------------------------------------------------------- jobs


def parse_seeds(text):
    return B.parse_seeds(text)


def validate_args(jobs):
    """F8's job part: no argument contains ProvisionalEndTick (C runs in game form); at most one -Rules= per job (FParse::Value
    reads only the first, cmd:1482)."""
    bad = []
    for j in jobs:
        if any("ProvisionalEndTick" in a for a in j["args"]):
            bad.append("%s: an argument contains ProvisionalEndTick" % j["name"])
        n = sum(1 for a in j["args"] if a.startswith("-Rules="))
        if n > 1:
            bad.append("%s: %d -Rules= arguments" % (j["name"], n))
    return bad


def build_jobs(folder, c, t_death, cells=None, max_seed=None, drop_death=False):
    """Bundle C's jobs: no common arguments; ticks C for R/slowR/death/wanderer and 20,000 for inputless/inputless075/test;
    seeds 1-60 (test 1-140); on and off of a seed next to each other (wanderer, inputless)."""
    jobs = []
    for cell in CELL_ORDER:
        if cells and cell not in cells:
            continue
        if cell == "death" and drop_death:
            continue
        for seed in SEEDS.get(cell, DEFAULT_SEEDS):
            if max_seed is not None and seed > max_seed:
                continue
            for off in ((False, True) if HAS_OFF.get(cell) else (False,)):
                name = "%s_%d%s" % (cell, seed, "_off" if off else "")
                base = os.path.join(folder, name)
                args = list(CELL_ARGS[cell])
                if cell == "death":
                    args.append("-DeathAt=%d" % t_death)
                if off:
                    args.append(OFF_ARG)
                jobs.append({"name": name, "cell": cell, "seed": seed, "ticks": c if cell in AT_CLOCK else HORIZON, "off": off, "args": args,
                             "csv": base + ".csv", "rows": base + "_rows.csv", "claims": base + "_claims.csv", "log": base + ".log"})
    bad = validate_args(jobs)
    if bad:
        raise ValueError("; ".join(bad))
    return jobs


def guarded_files(root):
    """bundleB's set (module DLLs, Content/CA/Data/*.uasset incl. ST_CA, Tools/data/generated.sha1) plus the three Config inis."""
    files = list(B.guarded_files(root))
    for name in ("DefaultGameplayTags.ini", "DefaultGame.ini", "DefaultEngine.ini"):
        p = os.path.join(root, "Config", name)
        if os.path.exists(p):
            files.append(p)
    return sorted(set(files))


def wall_minutes(a_sec_per_20k, c, n_clock, n_20k, workers):
    """Brief 0.4: <A_SEC_PER_20K> x (n_clock * C + n_20k * 20,000) / 20,000 / workers / 60."""
    return a_sec_per_20k * (n_clock * c + n_20k * HORIZON) / float(HORIZON) / workers / 60.0

# -------------------------------------------------------------------------------------------------------------- parsing


RE_FIRST_PERSON = re.compile(r"by the record told: first-person claim lines (-?\d+)")
RE_G132 = re.compile(r"#132 guarded \[([^\]]*)\] / \[([^\]]*)\] / \[([^\]]*)\]")
RE_KEYS = re.compile(r"#135 claim trigger .*? keys \[([^\]]*)\]")
RE_CONFLICT = re.compile(r"taken (\d+) \((\d+)/(\d+)/(\d+) by tick bands 0-3000/3000-6000/later; .*?taken by top conqueror (\d+), lost by one people max (\d+); "
                         r".*?delay bought per tag \[([^\]]*)\]")
RE_CALLING = re.compile(r"death asked at (-?\d+) \(recorded (-?\d+)\); deaths (\d+) \(by end (\d+)\)")
RE_FIRST_DEATH = re.compile(r"first death at (-?\d+)")
RE_SUCC = re.compile(r"successions \[([^\]]*)\] \(tick:")
RE_PER_DEATH = re.compile(r"per death \[([^\]]*)\] \(tick:")
RE_PRESSURE_HEAD = re.compile(r"policy pressure (\S+) guard (-?\d+) leave near sphere (-?\d+)")
RE_GUARDED_TAGS = re.compile(r"guarded crises (\d+) by tag \[([^\]]*)\]")
RE_DELAY_TAGS = re.compile(r"delay bought (-?\d+) by tag \[([^\]]*)\]")
RE_BAND = re.compile(r"401-550 (\d+) eased (\d+)")
RE_RACE132 = re.compile(r"#132 by the record: guarded at the row's cap by tag \[([^\]]*)\], guarded ends by tag \[([^\]]*)\], ruled unguarded ends by tag \[([^\]]*)\]")
RE_RULES = re.compile(r"CosmicBalance: rules (\S+) = (\S+?)\r?$", re.M)
RE_SAVEAT = re.compile(r"CosmicBalance: -SaveAt (\d+) wrote (.*?)\r?$", re.M)


def phase_text(text):
    """GuardedPhaseText: 'tag=count:share%' tokens or '-' -> {tag: (count, share)}."""
    out = {}
    for tok in text.split():
        if tok == "-":
            continue
        m = re.match(r"([^=]+)=(\d+):(-?[0-9.]+)%$", tok)
        if not m:
            raise ValueError(tok)
        out[m.group(1)] = (int(m.group(2)), float(m.group(3)))
    return out


def eq_tokens(text, rx=r"([^=]+)=(-?\d+)$"):
    out = {}
    for tok in text.split():
        m = re.match(rx, tok)
        if not m:
            raise ValueError(tok)
        out[m.group(1)] = int(m.group(2))
    return out


def colon_tokens(text, n):
    """'tag:a/b[/c]' tokens -> {tag: (a, b[, c])}."""
    out = {}
    for tok in text.split():
        tag, _, rest = tok.rpartition(":")
        vals = tuple(int(v) for v in rest.split("/"))
        if not tag or len(vals) != n:
            raise ValueError(tok)
        out[tag] = vals
    return out


def parse_log(path):
    """bundleB.parse_log plus what bundle C reads. A part that does not parse stays None (a run failure: run_failures)."""
    p = B.parse_log(path)
    if not p.get("log_exists"):
        return p
    text = A.read_text(path)
    for k in ("first_person", "g132", "keys", "conflict_c", "calling", "pressure", "race132"):
        p[k] = None
    m = A.last_match(r"CosmicBalance ending: (.*?)\r?$", text)
    ending = m.group(1) if m else ""
    m = RE_FIRST_PERSON.search(ending)
    p["first_person"] = int(m.group(1)) if m else None
    m = RE_G132.search(ending)
    if m:
        try:
            p["g132"] = [phase_text(m.group(i)) for i in (1, 2, 3)]
        except ValueError:
            p["g132"] = None
    m = RE_KEYS.search(ending)
    if m:
        keys = []
        try:
            for tok in m.group(1).split():
                parts = tok.split(":")
                t, phase, flag = int(parts[0]), int(parts[-2]), parts[-1]
                system = ":".join(parts[1:-2])
                if flag not in ("k", "w", "s"):
                    raise ValueError(tok)
                keys.append((t, system, phase, flag))
            p["keys"] = keys
        except (ValueError, IndexError):
            p["keys"] = None
    m = A.last_match(r"CosmicBalance conflict: (.*?)\r?$", text)
    c = RE_CONFLICT.search(m.group(1)) if m else None
    if c:
        try:
            delay = {}
            for tok in c.group(7).split():
                d = re.match(r"([^=]+)=(-?\d+)\((\d+)\)$", tok)
                if not d:
                    raise ValueError(tok)
                delay[d.group(1)] = (int(d.group(2)), int(d.group(3)))
            p["conflict_c"] = {"taken": int(c.group(1)), "bands": (int(c.group(2)), int(c.group(3)), int(c.group(4))), "top": int(c.group(5)),
                               "lost_max": int(c.group(6)), "delay": delay}
        except ValueError:
            p["conflict_c"] = None
    m = A.last_match(r"CosmicBalance calling deaths: (.*?)\r?$", text)
    if m:
        line = m.group(1)
        h, fd, sc, pd = RE_CALLING.search(line), RE_FIRST_DEATH.search(line), RE_SUCC.search(line), RE_PER_DEATH.search(line)
        if h and fd and sc and pd:
            try:
                succ = []
                for tok in sc.group(1).split(",") if sc.group(1) else []:
                    parts = tok.split(":")
                    dm = re.match(r"D(-?\d+)$", parts[-1])
                    if not dm:
                        raise ValueError(tok)
                    succ.append((int(parts[0]), int(dm.group(1))))
                per = [int(tok.split(":")[0]) for tok in (pd.group(1).split(",") if pd.group(1) else [])]
                p["calling"] = {"asked": int(h.group(1)), "recorded": int(h.group(2)), "deaths": int(h.group(3)), "by_end": int(h.group(4)),
                                "first_death": int(fd.group(1)), "successions": succ, "death_ticks": per}
            except (ValueError, IndexError):
                p["calling"] = None
    m = A.last_match(r"CosmicBalance pressure: (.*?)\r?$", text)
    if m:
        line = m.group(1)
        h, g, d, bd = RE_PRESSURE_HEAD.search(line), RE_GUARDED_TAGS.search(line), RE_DELAY_TAGS.search(line), RE_BAND.search(line)
        if h and g and d and bd:
            try:
                p["pressure"] = {"policy": h.group(1), "guard": int(h.group(2)), "leave": int(h.group(3)), "guarded": int(g.group(1)),
                                 "guarded_by_tag": eq_tokens(g.group(2)), "delay": int(d.group(1)), "delay_by_tag": eq_tokens(d.group(2)),
                                 "band": int(bd.group(1)), "band_eased": int(bd.group(2))}
            except ValueError:
                p["pressure"] = None
    m = RE_RACE132.search(p.get("race_text") or "")
    if m:
        try:
            p["race132"] = {"at_cap": colon_tokens(m.group(1), 2), "guarded_ends": colon_tokens(m.group(2), 3), "unguarded_ends": colon_tokens(m.group(3), 3)}
        except ValueError:
            p["race132"] = None
    p["rules_lines"] = [(k, v) for k, v in RE_RULES.findall(text)]
    p["rules_cannot"] = len(re.findall(r"-Rules cannot set", text))
    p["saveat"] = [(int(t), w.strip()) for t, w in RE_SAVEAT.findall(text)]
    return p


NEEDED = ("first_person", "g132", "keys", "conflict_c", "calling", "race132")


def run_failures(job, parsed, exit_code=None, ignore_errors=(), need_files=True):
    """Brief 1.2: bundleB.run_failures (non-zero exit, missing file, any end check / race check line, LogCosmic: Error, an
    unreadable race part or ending head) plus the parts bundle C's readings read (the test cell's pressure line too)."""
    why = B.run_failures(job, parsed, exit_code, ignore_errors)
    if not need_files:
        why = [w for w in why if not w.startswith("missing ")]
    if not parsed.get("log_exists"):
        return why
    bad = [k for k in NEEDED if parsed.get(k) is None]
    if job.get("cell") == "test" and parsed.get("pressure") is None:
        bad.append("pressure")
    if bad:
        why.append("unreadable: %s" % ", ".join(bad))
    return why

# -------------------------------------------------------------------------------------------------------------- helpers


def g_text(v):
    return "%g" % v


def wilson_share(k, n):
    return B.wilson_share(k, n)


def predicted(p):
    """The moved-awakening predictor (brief 1.2): the log's conflict 'delay bought per tag' lists Hazard.AIAwakening with sum > 75."""
    d = ((p or {}).get("conflict_c") or {}).get("delay") or {}
    return d.get(AWAKENING, (0, 0))[0] > PREDICTOR_SUM


def ok_of(results, name):
    return bool(results.get(name, {}).get("ok"))


def items_of(jobs, parsed, results, cell, off=False):
    out = []
    for j in sorted((j for j in jobs if j["cell"] == cell and j["off"] == off), key=lambda j: j["seed"]):
        out.append((j, parsed.get(j["name"]) or {}, ok_of(results, j["name"])))
    return out


def read_claims(path):
    if not os.path.exists(path):
        return None
    lines = [l for l in A.read_text(path).splitlines() if l.strip()]
    if not lines:
        return []
    head = lines[0].split(",")
    out = []
    for l in lines[1:]:
        c = l.split(",")
        row = dict(zip(head, c))
        for k in ("Ruled", "PlacedTick", "CardTick", "RemainingAtCard", "VigilKept", "Relief", "OwnerChanged", "EndTick", "AfterBinding", "SealedTick"):
            row[k] = int(row[k])
        out.append(row)
    return out


def band_claim(r, ruled=True, owner_same=True):
    """The CSV form of the pressure line's 401-550 band (cmd:3303-3321): Ruled=1, CardTick>=0, 401<=RemainingAtCard<=550,
    VigilKept=1, Relief=0, OwnerChanged=0, EndKind in {joined, eased, other, expired}."""
    return (r["CardTick"] >= 0 and BAND_401_550[0] <= r["RemainingAtCard"] <= BAND_401_550[1] and r["VigilKept"] == 1 and r["Relief"] == 0
            and r["EndKind"] in CLAIM_ENDS and (not ruled or r["Ruled"] == 1) and (not owner_same or r["OwnerChanged"] == 0))


def p90(values):
    vals = sorted(values)
    return vals[int(math.ceil(0.9 * len(vals))) - 1] if vals else None


def median(values):
    vals = list(values)
    return statistics.median(vals) if vals else None


def fmtv(v):
    return A.fmt(v)

# ------------------------------------------------------------------------------------------------------------- controls


def controls(jobs, parsed, results, ctx):
    """F1-F6 and F8 (problems discard the bundle; notes do not). F7 is separate (it stops the #142 reading only)."""
    C, t_death = ctx["c"], ctx["t_death"]
    problems, notes = [], []
    by = {k: [] for k in ("F1", "F2", "F3", "F4", "F5", "F6", "F8")}

    def bad(f, text):
        by[f].append(text)
        problems.append("%s %s" % (f, text))

    for text in validate_args(jobs):
        bad("F8", text)
    signal = False
    deaths = []
    for j in jobs:
        p = parsed.get(j["name"]) or {}
        if not p.get("log_exists") or not ok_of(results, j["name"]):
            continue
        name = j["name"]
        ex = p.get("ending_x")
        if not ex or ex["kind"] == "None" or ex["tick"] < 0:
            bad("F1", "%s: no record (kind %s at %s) - with C > 0 every run records by C" % (name, ex and ex["kind"], ex and ex["tick"]))
        elif ex["tick"] > C:
            bad("F1", "%s: record %s at %d after C %d" % (name, ex["kind"], ex["tick"], C))
        elif ex["kind"] == "Clock" and ex["tick"] != C:
            bad("F1", "%s: record Clock at %d, not at C %d" % (name, ex["tick"], C))
        last = B.csv_last_tick(j["csv"])
        if last != j["ticks"]:
            bad("F2", "%s: the CSV ends at %s, not %d" % (name, last, j["ticks"]))
        want = EXPECTED_RULES.get(j["cell"], {})
        got = p.get("rules_lines") or []
        if sorted(got) != sorted(want.items()) or len(got) != len(want):
            bad("F8", "%s: rules lines %s, expected %s" % (name, got, sorted(want.items())))
        if p.get("rules_cannot"):
            bad("F8", "%s: '-Rules cannot set' %d" % (name, p["rules_cannot"]))
        side = (p.get("race_b") or {}).get("side")
        race = p.get("race") or {}
        if j["off"]:
            if p.get("rows_taken_out") != EXPECTED_REMOVED or p.get("rows_taken_out_lines") != 1:
                bad("F3", "%s: rows taken out [%s] (%s lines), expected the ten" % (name, p.get("rows_taken_out"), p.get("rows_taken_out_lines")))
            k = p.get("off_knobs")
            if not k:
                bad("F3", "%s: no 'AIUpperRungProgressMultiplier 1 (was r), FleetDelayCountdownShare 0 (was s)' tail" % name)
            else:
                if k["share"] != g_text(ctx["share"]):
                    bad("F3", "%s: FleetDelayCountdownShare was %s, not %s" % (name, k["share"], g_text(ctx["share"])))
                if k["r"] != g_text(ctx["r"]):
                    bad("F3", "%s: AIUpperRungProgressMultiplier was %s, not %s" % (name, k["r"], g_text(ctx["r"])))
            ext, tr, t4 = race.get("extinction"), race.get("treaties"), race.get("ai_t4")
            if side and (side["era"] or side["wit"]):
                bad("F3", "%s: off run with era %d / witness %d" % (name, side["era"], side["wit"]))
            if ext and (ext["on_ruled"] or ext["on_ai"]):
                bad("F3", "%s: off run with extinctions %d/%d" % (name, ext["on_ruled"], ext["on_ai"]))
            if tr and tr["ai_treaties"]:
                bad("F3", "%s: off run with AI-step treaties %d" % (name, tr["ai_treaties"]))
            if t4 and t4["t4_1"] >= 0:
                bad("F3", "%s: off run with an AI T4 at %d" % (name, t4["t4_1"]))
            continue
        if p.get("rows_taken_out_lines"):
            bad("F4", "%s: an on log has a 'rows taken out' line" % name)
        if side:
            if side["layout"] != "new":
                bad("F4", "%s: side part in the layout before [B-6b8] (no witness 'first at')" % name)
            if (side["era"] > 0) != (side["era_first"] >= 0) or (side["era"] > 0 and side["era_first"] < 600):
                bad("F4", "%s: era lines %d with first at %d (E >= 600 iff lines > 0)" % (name, side["era"], side["era_first"]))
            if side["era"] > 0 or side["wit"] > 0:
                signal = signal or j["cell"] != "death"
            if side["layout"] == "new":
                W = side["wit_first"]
                if (side["wit"] > 0) != (W >= 0):
                    bad("F4", "%s: witness cards %d with first at %d (W >= 0 iff cards > 0)" % (name, side["wit"], W))
                if W >= 0 and j["cell"] == "wanderer" and (p.get("left_at") is None or p["left_at"] < 0 or W < p["left_at"]):
                    bad("F4", "%s: witness first at %d before the wanderer left at %s" % (name, W, p.get("left_at")))
                if W >= 0:
                    first_claim = B.claims_first_placed(j["claims"])
                    if first_claim is None or W < first_claim:
                        bad("F4", "%s: witness first at %d before the first claim placed (%s)" % (name, W, first_claim))
        if j["cell"] == "death":
            d = p.get("death")
            if not d:
                bad("F5", "%s: no 'calling deaths: death asked at' line" % name)
            else:
                deaths.append((j["seed"], d["at"], d["recorded"]))
                if d["at"] != t_death:
                    bad("F5", "%s: death asked at %d, not %d" % (name, d["at"], t_death))
                if not d["at"] < C:
                    bad("F5", "%s: death asked at %d is not before C %d" % (name, d["at"], C))
        if j["cell"] == "test":
            pr = p.get("pressure") or {}
            if (pr.get("policy"), pr.get("guard"), pr.get("leave")) != ("Faithful", 1, 1):
                bad("F6", "%s: pressure head %s guard %s leave near sphere %s, expected Faithful 1 1" % (name, pr.get("policy"), pr.get("guard"), pr.get("leave")))
    if not signal:
        notes.append("era 0 and witness 0 over every on-run: #156 and #141 '%s'" % (NOT_MEASURED % 0))
    unrecorded = [s for s, _, r in deaths if r == 0]
    if unrecorded:
        notes.append("F5 note: forced death recorded 0 in seeds %s" % unrecorded)
    return {"ok": not problems, "problems": problems, "notes": notes, "by": by, "deaths": deaths}


def f7_check(jobs, parsed, results):
    """F7 per test run: X (CSV band rows, no tick filter), J (X's joined), O (same filter but Ruled and OwnerChanged, with
    OwnerChanged=1); P = the pressure line's 401-550 count. X - J <= P <= X + O; eased: X_e <= P_e <= X_e + O_e."""
    out = {"ok": True, "runs": [], "broken": [], "owner_changed_rows": 0, "owner_changed_seeds": []}
    for j, p, ok in items_of(jobs, parsed, results, "test"):
        if not ok:
            continue
        rows = read_claims(j["claims"]) or []
        X = [r for r in rows if band_claim(r)]
        J = sum(1 for r in X if r["EndKind"] == "joined")
        O = [r for r in rows if band_claim(r, ruled=False, owner_same=False) and r["OwnerChanged"] == 1]
        Xe = sum(1 for r in X if r["EndKind"] == "eased")
        Oe = sum(1 for r in O if r["EndKind"] == "eased")
        pr = p.get("pressure") or {}
        P, Pe = pr.get("band"), pr.get("band_eased")
        good =P is not None and len(X) - J <= P <= len(X) + len(O) and Xe <= Pe <= Xe + Oe
        rec = {"seed": j["seed"], "X": len(X), "J": J, "O": len(O), "P": P, "Xe": Xe, "Oe": Oe, "Pe": Pe, "ok": good}
        out["runs"].append(rec)
        if not good:
            out["ok"] = False
            out["broken"].append(rec)
        oc = sum(1 for r in rows if r["OwnerChanged"] == 1)
        if oc:
            out["owner_changed_rows"] += oc
            out["owner_changed_seeds"].append(j["seed"])
    return out

# -------------------------------------------------------------------------------------------------------------- bridges


def first_diffs(a_job, b_job):
    """(structural problems, {csv/rows: first differing tick or None})."""
    why, first = [], {}
    for key in ("csv", "rows"):
        if not (os.path.exists(a_job[key]) and os.path.exists(b_job[key])):
            why.append("missing %s file" % key)
            continue
        ha, ra = A.read_csv_by_tick(a_job[key])
        hb, rb = A.read_csv_by_tick(b_job[key])
        if ha != hb:
            why.append("%s headers differ" % key)
        first[key] = A.first_difference(ra, rb)
    return why, first


def min_first(first):
    vals = [v for v in first.values() if v is not None]
    return min(vals) if vals else None


def pair_check(on_job, off_job, on_p, off_p, use_w):
    """B-a: T_B = min(T + 1, E[, W]) (bundleB.bridge_of). A difference below T_B is explained only when the off log satisfies
    the predictor; then G = the first differing sample and T_B = min(T_B, G)."""
    res = {"pair": on_job["name"], "cell": on_job["cell"], "seed": on_job["seed"], "ok": False, "why": [], "T_B": None, "T_B_eff": None,
           "G": None, "predicted": predicted(off_p), "explained": False}
    tb, T, E, W = B.bridge_of(on_job, on_p, use_w)
    res.update(T_B=tb, T=T, E=E, W=W)
    if tb is None:
        res["why"].append("no bridge: %s" % ("on ending/side unreadable" if T is None or E is None else "W unknown"))
        return res
    why, first = first_diffs(on_job, off_job)
    res["why"] += why
    G = min_first(first)
    res["G"], res["first_diff"] = G, first
    res["T_B_eff"] = tb
    if G is not None and G < tb:
        if res["predicted"]:
            res["explained"] = True
            res["T_B_eff"] = G
        else:
            res["why"].append("differs at tick %d < T_B %s and the off log has no %s sum > %d" % (G, A.fmt(tb), AWAKENING, PREDICTOR_SUM))
    T_off = A.threshold_of(off_p.get("ending"), off_job["ticks"])
    if tb == T + 1 and T_off != T and not (res["explained"] and G <= T):
        res["why"].append("T differs: on %s, off %s" % (T, T_off))
    res["ok"] = not res["why"]
    return res


def bridge_b(ref_job, job, boundary, label):
    """B-b: rows with Tick <= boundary equal (the action is recorded at the boundary and acts in the next tick)."""
    why, first = first_diffs(ref_job, job)
    G = min_first(first)
    if G is not None and G <= boundary:
        why.append("differs at tick %d <= %d" % (G, boundary))
    return {"pair": "%s ~ %s" % (job["name"], ref_job["name"]), "bridge": label, "ok": not why, "why": why, "first_diff": first}


def bridge_c(job, b6a_dir):
    """B-c (decision 36): wanderer_s_off CSV and _rows.csv against B6a bundle 1's wanderer_s, every row with Tick < 6,000."""
    ref = {"csv": os.path.join(b6a_dir, "wanderer_%d.csv" % job["seed"]), "rows": os.path.join(b6a_dir, "wanderer_%d_rows.csv" % job["seed"])}
    why, first = first_diffs(ref, job)
    G = min_first(first)
    if G is not None and G < B6A_CUT:
        why.append("differs at tick %d < 6,000" % G)
    return {"pair": "%s ~ b6a %s" % (job["name"], os.path.basename(ref["csv"])), "ok": not why, "why": why, "first_diff": first}


def own_of(p, cut=B1_CUT):
    """own(s): the wanderer line's crowned at t with 0 <= t <= cut and (first death -1 or t < first death). Also the any-vessel
    reading (0 <= t <= cut) - 'crowned at' crosses lives."""
    cr = p.get("crowned") or {}
    t = cr.get("tick", -1)
    fd = (p.get("calling") or {}).get("first_death", -1)
    anyv = 0 <= t <= cut
    own = anyv and (fd < 0 or t < fd)
    return {"own": own, "any": anyv, "tick": t, "path": cr.get("path"), "left": p.get("left_at"), "first_death": fd}


def b1_side(items):
    own = [(j, own_of(p)) for j, p, ok in items if ok]
    n = len(items)
    owned = [(j, o) for j, o in own if o["own"]]
    after = [o["tick"] - o["left"] for _, o in owned if o["left"] is not None and o["left"] >= 0]
    paths = {}
    for _, o in owned:
        paths[o["path"]] = paths.get(o["path"], 0) + 1
    return {"n": n, "own": len(owned), "any": sum(1 for _, o in own if o["any"]), "own_share": wilson_share(len(owned), n),
            "any_share": wilson_share(sum(1 for _, o in own if o["any"]), n), "median_after": median(after), "paths": paths,
            "seeds": {j["seed"]: o for j, o in own}}


def bridge_e(jobs, parsed, results):
    """B-e: the off side's own-vessel reading reproduces B6a bundle 1's on side (40/60, median after leaving 1,350, paths
    return 22 / vacuum 15 / reverence 3). A mismatch with B-c intact is a parser error."""
    items = items_of(jobs, parsed, results, "wanderer", off=True)
    off = b1_side(items)
    failed = [j["seed"] for j, p, ok in items if not ok]
    if failed:
        # One twice-failed run does not withhold the cell, but B-e compares counts out of all 60: not judged, not broken.
        return {"ok": None, "why": ["skipped: off runs failed twice %s - B-e needs all 60" % failed], "off": off}
    want = B6A_B1
    if off["n"] != want["n"]:
        return {"ok": None, "why": ["skipped: %d off runs, B6a has %d" % (off["n"], want["n"])], "off": off}
    why = []
    if off["own"] != want["own"]:
        why.append("own %d/%d, B6a %d/%d" % (off["own"], off["n"], want["own"], want["n"]))
    if off["median_after"] != want["median"]:
        why.append("median after leaving %s, B6a %d" % (fmtv(off["median_after"]), want["median"]))
    got = {k: off["paths"].get(k, 0) for k in want["paths"]}
    if got != want["paths"] or sum(off["paths"].values()) != sum(want["paths"].values()):
        why.append("paths %s, B6a %s" % (off["paths"], want["paths"]))
    return {"ok": not why, "why": why, "off": off}


def first_ticks(p, c, whole):
    race = p.get("race") or {}
    vals = {"AI T4": (race.get("ai_t4") or {}).get("t4_1"), "AI T5": (race.get("ai_t4") or {}).get("t5"), "ruled T3": (race.get("ruled") or {}).get("r3"),
            "ruled T4": (race.get("ruled") or {}).get("r4"), "ruled T5": (race.get("ruled") or {}).get("r5"),
            "fifth": (race.get("fifth") or {}).get("fifth"), "silence": (race.get("fifth") or {}).get("silence")}
    out = {}
    for k, v in vals.items():
        f = math.inf if v is None or v < 0 else v
        out[k] = f if whole else min(f, c)
    return out


def manifest_of(folder):
    path = os.path.join(folder, "manifest.sha1")
    if not os.path.exists(path):
        return None
    out = {}
    for l in A.read_text(path).splitlines():
        if l.strip():
            h, name = l.split(None, 1)
            out[name.strip()] = h
    return out


def bundle_b_death_ats(logs_dir, max_seed=None):
    """Brief 0.2: what bundle B's death_*.log files say ('death asked at <T>'), as {value: [log names]}; a log without the
    line counts under None. Only logs that exist are read; an absent folder gives {}."""
    out = {}
    if not logs_dir or not os.path.isdir(logs_dir):
        return out
    for j in B.build_jobs(logs_dir, None, cells={"death"}, max_seed=max_seed):
        path = os.path.join(logs_dir, j["name"] + ".log")
        if not os.path.exists(path):
            continue
        d = B.parse_log(path).get("death")
        out.setdefault(d["at"] if d else None, []).append(j["name"])
    return out


def death_refusal(ats, t_death):
    """None when every bundle B death log says 'death asked at <t_death>' (or there is none); else the refusal line."""
    if not ats or set(ats) == {t_death}:
        return None
    return "refusing: bundle B death logs say death asked at %s, --t-death %d - stop and ask the lead" % (
        "{%s}" % ", ".join("%s (%d logs)" % ("no line" if k is None else k, len(v)) for k, v in sorted(ats.items(), key=lambda kv: (kv[0] is None, kv[0] or 0))),
        t_death)


def bundle_b_state(logs_dir, csv_dir, t_death, max_seed=None):
    """Bundle B's jobs (logs from logs_dir, CSVs from csv_dir), whether the logs match the manifest, and <BUNDLE_B_CSV>."""
    st = {"jobs": {}, "logs_ok": None, "csv_ok": False, "why": []}
    if not logs_dir or not os.path.isdir(logs_dir):
        st["why"].append("no bundle B log folder %s" % logs_dir)
        return st
    st["death_at"] = sorted(k for k in bundle_b_death_ats(logs_dir, max_seed) if k is not None)
    man = manifest_of(logs_dir)
    for j in B.build_jobs(csv_dir or logs_dir, t_death, max_seed=max_seed):
        j = dict(j, log=os.path.join(logs_dir, j["name"] + ".log"))
        st["jobs"][j["name"]] = j
    if man is None:
        st["why"].append("no manifest.sha1 in %s" % logs_dir)
        return st
    bad_logs = []
    for j in st["jobs"].values():
        name = os.path.basename(j["log"])
        if os.path.exists(j["log"]) and A.sha1_files([j["log"]])[j["log"]] != man.get(name):
            bad_logs.append(name)
    st["logs_ok"] = not bad_logs
    if bad_logs:
        st["why"].append("bundle B logs differ from the manifest: %s" % ", ".join(bad_logs[:10]))
    csv_ok = bool(csv_dir) and os.path.isdir(csv_dir)
    if csv_ok:
        for j in st["jobs"].values():
            for key in ("csv", "rows", "claims"):
                path = j[key]
                if not os.path.exists(path) or A.sha1_files([path])[path] != man.get(os.path.basename(path)):
                    csv_ok = False
                    break
            if not csv_ok:
                break
    st["csv_ok"] = csv_ok
    return st


def bridge_d(jobs, parsed, results, ctx, bstate, bparsed):
    """B-d: C against bundle B per seed and cell. CSV rows below C equal (input-less: the whole run); without bundle B's CSVs,
    the race line's first ticks as min(f, C). A matching seed's bundle B record E_i <= C must be C's record (kind and tick),
    otherwise C records Clock at C. On sides: per seed, valid only when B9 did not run and no world-moving [B-6br]; a seed
    whose bundle B on log satisfies the predictor 'may move' (recorded, not broken). Off sides: valid when no [B-6br]."""
    C = ctx["c"]
    out = {"ok": True, "entries": [], "skipped": [], "notes": [], "may_move": {}, "moved": [], "mode": "csv" if bstate["csv_ok"] else "first ticks"}
    if bstate["logs_ok"] is False:
        out["ok"] = False
        out["why"] = bstate["why"]
        return out
    if bstate["logs_ok"] is None:
        out["ok"] = None
        out["why"] = ["skipped: %s" % "; ".join(bstate["why"])]
        return out
    on_valid = not ctx["b9_ran"] and not ctx["br_world"]
    off_valid = not ctx["br_world"]
    for j in jobs:
        if j["cell"] not in ("R", "slowR", "death", "wanderer", "inputless"):
            continue
        if not ok_of(results, j["name"]):
            out["skipped"].append("%s: C run failed" % j["name"])
            continue
        if j["off"] and not off_valid:
            out["skipped"].append("%s: off side skipped - [B-6br] moved the world (%s)" % (j["name"], ctx["br_world"]))
            continue
        if not j["off"] and not on_valid:
            out["skipped"].append("%s: on side skipped - %s" % (j["name"], "[B-6b9] ran" if ctx["b9_ran"] else "[B-6br] moved the world"))
            continue
        bj = bstate["jobs"].get(j["name"])
        bp = bparsed.get(j["name"]) if bj else None
        if not bp or not bp.get("log_exists"):
            out["skipped"].append("%s: no bundle B log" % j["name"])
            continue
        may = (not j["off"]) and predicted(bp)
        if may:
            out["may_move"].setdefault(j["cell"], []).append(j["seed"])
        whole = j["cell"] == "inputless"
        why, G = [], None
        if bstate["csv_ok"]:
            w, first = first_diffs(bj, j)
            why += w
            G = min_first(first)
            if G is not None and (whole or G < C):
                why.append("differs at tick %d%s" % (G, "" if whole else " < C"))
        else:
            fb, fc = first_ticks(bp, C, whole), first_ticks(parsed[j["name"]], C, whole)
            diff = sorted(k for k in fb if fb[k] != fc[k])
            if diff:
                why.append("first ticks differ: %s" % ", ".join("%s %s/%s" % (k, A.fmt(fb[k]), A.fmt(fc[k])) for k in diff))
                G = min(min(fb[k], fc[k]) for k in diff)
        if not why:
            bex, cex = bp.get("ending_x") or {}, parsed[j["name"]].get("ending_x") or {}
            if bex.get("kind") not in (None, "None") and 0 <= bex.get("tick", -1) <= C:
                want = (bex["kind"], bex["tick"])
            else:
                want = ("Clock", C)
            if (cex.get("kind"), cex.get("tick")) != want:
                why.append("record %s@%s, bundle B gives %s@%s" % (cex.get("kind"), cex.get("tick"), want[0], want[1]))
        entry = {"name": j["name"], "cell": j["cell"], "seed": j["seed"], "off": j["off"], "may_move": may, "moved": bool(why), "G": G, "why": why}
        if why and may:
            out["moved"].append(entry)
        elif why:
            out["ok"] = False
        out["entries"].append(entry)
    for cell, seeds, label in (("R", ctx["k_moved_r"], "R"), ("wanderer", ctx["k_moved_w"], "wanderer")):
        if not on_valid:
            break
        stray = [s for s in seeds if s not in out["may_move"].get(cell, [])]
        if stray:
            out["notes"].append("bundle K moved %s seeds %s are not 'may move' by the bundle B logs - explain" % (label, stray))
    out["moved_count"] = len(out["moved"])
    return out

# ------------------------------------------------------------------------------------------------------ frozen arithmetic


def withheld_cells(jobs, results):
    failed = {}
    for j in jobs:
        if not ok_of(results, j["name"]):
            failed.setdefault(j["cell"], []).append(j["name"])
    return failed, sorted(c for c, v in failed.items() if len(v) >= 2)


def r1_136(jobs, parsed, results):
    out = {}
    for cell in ("R", "slowR"):
        items = items_of(jobs, parsed, results, cell)
        n = len(items)
        k = sum(1 for j, p, ok in items if ok and (p.get("first_person") or 0) >= 1)
        sh = wilson_share(k, n)
        if n < 30:
            text = NOT_MEASURED % n
        else:
            text = ("닫는다 — %d/%d [%.3f, %.3f]" if k * 3 >= n else "열어 둔다 — %d/%d [%.3f, %.3f]") % (k, n, sh["lo"], sh["hi"])
        out[cell] = {"k": k, "n": n, "share": sh, "closed": (k * 3 >= n) if n >= 30 else None, "text": text,
                     "seeds": [j["seed"] for j, p, ok in items if ok and (p.get("first_person") or 0) >= 1]}
    out["text"] = out["R"]["text"] + "; slowR (reference) %d/%d" % (out["slowR"]["k"], out["slowR"]["n"])
    return out


def p_eased(chance, w, slots):
    return 1.0 - (1.0 - chance * w) ** slots


def accept_142(k_e, n, chance, w):
    """N (f) / decision 24: p_e = 1 - (1 - CHANCE*W)^47.5; region p_e +- 1.96 sqrt(p_e(1 - p_e)/n)."""
    pe = p_eased(chance, w, SLOTS_MID)
    half = 1.96 * math.sqrt(pe * (1 - pe) / n)
    lo, hi = pe - half, pe + half
    share = k_e / n
    return {"p_e": pe, "lo": lo, "hi": hi, "share": share, "inside": lo <= share <= hi}


def r2_142(jobs, parsed, results, ctx, f7):
    C, chance, w = ctx["c"], ctx["chance"], ctx["w_eased"]
    counted, after_binding, owner_changed, card_after_c, placed_after_c = [], [], 0, 0, 0
    for j, p, ok in items_of(jobs, parsed, results, "test"):
        if not ok:
            continue
        for r in read_claims(j["claims"]) or []:
            if band_claim(r, ruled=False, owner_same=False) and r["OwnerChanged"] == 1 and r["PlacedTick"] < C:
                owner_changed += 1
            if not band_claim(r):
                continue
            if r["PlacedTick"] >= C:
                placed_after_c += 1
                continue
            if r["AfterBinding"] == 1:
                after_binding.append(j["seed"])
                continue
            counted.append(dict(r, seed=j["seed"]))
            card_after_c += 1 if r["CardTick"] >= C else 0
    n = len(counted)
    k_e = sum(1 for r in counted if r["EndKind"] == "eased")
    k_j = sum(1 for r in counted if r["EndKind"] == "joined")
    sum_p = sum(p_eased(chance, w, r["RemainingAtCard"] // SLOT_TICKS) for r in counted)
    sh_e, sh_j = wilson_share(k_e, n), wilson_share(k_j, n)
    p401, p550 = p_eased(chance, w, BAND_401_550[0] // SLOT_TICKS), p_eased(chance, w, BAND_401_550[1] // SLOT_TICKS)
    out = {"n": n, "k_e": k_e, "k_j": k_j, "eased": sh_e, "joined": sh_j, "sum_p": sum_p, "after_binding": len(after_binding),
           "after_binding_seeds": sorted(set(after_binding)), "owner_changed": owner_changed, "card_after_c": card_after_c,
           "placed_after_c": placed_after_c, "model_range": (p401, p550),
           "b4_form": (sh_e["lo"] is not None and sh_e["lo"] <= p401 and p550 <= sh_e["hi"]), "f7_ok": f7["ok"]}
    if not f7["ok"]:
        out.update(judged=False, verdict=None, text="F7 broken - #142 not read (%d runs outside X - J <= P <= X + O)" % len(f7["broken"]))
    elif n < 30:
        out.update(judged=False, verdict=None, text="%s (k_e/n %s)" % (NOT_MEASURED % n, sh_e["text"]))
    else:
        acc = accept_142(k_e, n, chance, w)
        out.update(judged=True, verdict=acc["inside"], acceptance=acc,
                   text=("모형 구간 안 — %d/%d" % (k_e, n)) if acc["inside"] else
                   ("%s #142 outside the model interval: %d/%d = %.3f, region [%.3f, %.3f] (p_e %.4f)" % (DEV, k_e, n, k_e / n, acc["lo"], acc["hi"], acc["p_e"])))
    return out


def r3_132(jobs, parsed, results):
    whole_c = whole_s = rec_c = rec_s = 0.0
    tags = {}
    delay = guards = cap_a = cap_b = 0
    ends_g, ends_u = [0, 0, 0], [0, 0, 0]
    for j, p, ok in items_of(jobs, parsed, results, "test"):
        if not ok:
            continue
        for ph, d in enumerate(p.get("g132") or []):
            for tag, (cnt, share) in d.items():
                t = tags.setdefault(tag, [0, 0.0])
                t[0] += cnt
                t[1] += cnt * share
            cnt, share = d.get(AWAKENING, (0, 0.0))
            whole_c += cnt
            whole_s += cnt * share
            if ph < 2:
                rec_c += cnt
                rec_s += cnt * share
        pr = p.get("pressure") or {}
        delay += (pr.get("delay_by_tag") or {}).get(AWAKENING, 0)
        guards += (pr.get("guarded_by_tag") or {}).get(AWAKENING, 0)
        r132 = p.get("race132") or {}
        a, b = (r132.get("at_cap") or {}).get(AWAKENING, (0, 0))
        cap_a += a
        cap_b += b
        for acc, src in ((ends_g, "guarded_ends"), (ends_u, "unguarded_ends")):
            for i, v in enumerate((r132.get(src) or {}).get(AWAKENING, (0, 0, 0))):
                acc[i] += v
    n = int(whole_c)
    share = whole_s / whole_c if whole_c else None
    out = {"n": n, "share_pct": share, "by_record_n": int(rec_c), "by_record_pct": (rec_s / rec_c) if rec_c else None,
           "pressure_delay": delay, "pressure_guards": guards, "cap": wilson_share(cap_a, cap_b), "ends_guarded": ends_g, "ends_unguarded": ends_u,
           "all_tags": {t: {"count": v[0], "share_pct": (v[1] / v[0]) if v[0] else None} for t, v in sorted(tags.items())}}
    if n < 30:
        out.update(text=NOT_MEASURED % n, below10=None)
    else:
        out.update(text="awakening delay share %.1f%% over %d guarded (by the record %s%% over %d); cap %s" % (
            share, n, fmtv(round(out["by_record_pct"], 1)) if out["by_record_pct"] is not None else "-", int(rec_c), out["cap"]["text"]),
            below10=share < 10.0)
        if out["below10"]:
            out["text"] += " - below 10%: find the cause (the guard lost its fleet?)"
    return out


def r4_b1(jobs, parsed, results, bridges_ok):
    on_items = items_of(jobs, parsed, results, "wanderer")
    off_items = items_of(jobs, parsed, results, "wanderer", off=True)
    on, off = b1_side(on_items), b1_side(off_items)
    out = {"on": on, "off": off, "read": bridges_ok}
    if not bridges_ok:
        out["text"] = "not read - bridge B-c or B-e broken (R4 needs both)"
        return out
    L, G = [], []
    for s, o in on["seeds"].items():
        f = off["seeds"].get(s)
        if f is None:
            continue
        if not o["own"] and f["own"]:
            L.append(s)
        if o["own"] and not f["own"]:
            G.append(s)
    pv = B.sign_test(len(L), len(G))
    out.update(L=L, G=G, p=pv)
    by = {j["seed"]: (j, p) for j, p, ok in on_items}
    offp = {j["seed"]: p for j, p, ok in off_items}
    if len(L) > len(G) and pv < 0.05:
        paths = {}
        apart, unexplained = [], []
        for s in L:
            p = by[s][1]
            race, side = p.get("race") or {}, (p.get("race_b") or {}).get("side") or {}
            cr, ext = race.get("crown") or {}, race.get("extinction") or {}
            a = cr.get("refused", 0) >= 1 and 0 <= cr.get("first", -1) <= B1_CUT
            b = ext.get("erased_once_ruled", 0) >= 1
            c = (side.get("answered") or 0) >= 1 and 0 <= (side.get("wit_first") if side.get("wit_first") is not None else -1) <= B1_CUT
            d = side.get("era", 0) >= 1 and 0 <= side.get("era_first", -1) <= B1_CUT
            paths[s] = {"a FarAbove": a, "b erased once ruled": b, "c witness answered": c, "d era": d, "off predictor": predicted(offp.get(s))}
            if not (a or b or c or d):
                unexplained.append(s)
            elif not (a or d):
                apart.append(s)
        out.update(verdict="B6b가 줄였다", paths=paths, upper_bound_only=apart, unexplained=unexplained,
                   text="B6b가 줄였다 — L %d / G %d (p %.4f); explained only by (b)/(c) %s%s" % (
                       len(L), len(G), pv, apart, ("; %s unexplained %d %s" % (DEV, len(unexplained), unexplained)) if unexplained else ""))
    elif pv >= 0.05:
        out.update(verdict="잡음", text="잡음 — L %d / G %d (p %.4f)" % (len(L), len(G), pv))
    else:
        out.update(verdict=None, developer=True, text="규칙 밖 — L %d < G %d, p %.4f (얼린 4.3은 G > L에 판정을 두지 않는다) %s" % (
            len(L), len(G), pv, DEV))
    m = on["median_after"]
    if m is not None and m <= 1800:
        out["gate"] = {"ok": True, "text": "on-side median after leaving %s <= 1,800" % fmtv(m)}
    else:
        out["gate"] = {"ok": False, "text": "깨짐 — 중앙값 %s; own paths %s against B6a %s (%s)" % (
            fmtv(m), {PATH_KO.get(k, k): v for k, v in on["paths"].items()}, {PATH_KO[k]: v for k, v in B6A_B1["paths"].items()}, DEV)}
    out["paths_on"] = {PATH_KO.get(k, k): v for k, v in on["paths"].items()}
    return out


def conflict_items(jobs, parsed, results, cell, off=False):
    out = {}
    for j, p, ok in items_of(jobs, parsed, results, cell, off):
        c = p.get("conflict_c")
        if ok and c:
            out[j["seed"]] = c
    return out


def r5_129(jobs, parsed, results):
    def summary(d):
        taken = [c["taken"] for c in d.values()]
        late = [c["bands"][2] for c in d.values()]
        tops = [c["top"] / c["taken"] for c in d.values() if c["taken"] > 0]
        return {"runs": len(d), "taken": A.dist(taken), "late": A.dist(late), "top_share": A.dist(tops),
                "pooled_top": (sum(c["top"] for c in d.values()) / sum(taken)) if sum(taken) else None,
                "runs_top_above_half": sum(1 for t in tops if t > 0.5), "lost_max": A.dist([c["lost_max"] for c in d.values()]),
                "taken_sum": sum(taken), "late_sum": sum(late)}
    on, off, s075 = (conflict_items(jobs, parsed, results, "inputless"), conflict_items(jobs, parsed, results, "inputless", True),
                     conflict_items(jobs, parsed, results, "inputless075"))
    diff = [on[s]["taken"] - off[s]["taken"] for s in on if s in off]
    diff_late = [on[s]["bands"][2] - off[s]["bands"][2] for s in on if s in off]
    out = {"0.79 on": summary(on), "0.79 off": summary(off), "on-off taken": A.dist(diff), "on-off late": A.dist(diff_late), "0.75 on": summary(s075),
           "B3": B3_129}
    out["text"] = "taken per run 0.79 on %s / off %s / on-off %s; 0.75 on %s; after 6,000: %s / %s / %s; 0.75 %s; top share 0.79 on %s (runs > 50%%: %d), 0.75 %s; %s" % (
        out["0.79 on"]["taken"]["text"], out["0.79 off"]["taken"]["text"], out["on-off taken"]["text"], out["0.75 on"]["taken"]["text"],
        out["0.79 on"]["late"]["text"], out["0.79 off"]["late"]["text"], out["on-off late"]["text"], out["0.75 on"]["late"]["text"],
        out["0.79 on"]["top_share"]["text"], out["0.79 on"]["runs_top_above_half"], out["0.75 on"]["top_share"]["text"], B3_129)
    return out


def r6_154(jobs, parsed, results):
    out = {}
    pool = {"late": [0, 0], "early": [0, 0]}
    for cell in ("wanderer", "inputless"):
        late, early = [0, 0], [0, 0]
        for j, p, ok in items_of(jobs, parsed, results, cell):
            if not ok:
                continue
            rec = (p.get("ending_x") or {}).get("tick", -1)
            for t, d in (p.get("calling") or {}).get("successions", []):
                full = 5 <= d <= 8
                if LATE_154 <= t <= rec:
                    late[0] += full
                    late[1] += 1
                elif t < LATE_154:
                    early[0] += full
                    early[1] += 1
        out[cell] = {"late": wilson_share(*late), "early": wilson_share(*early)}
        for k, v in (("late", late), ("early", early)):
            pool[k][0] += v[0]
            pool[k][1] += v[1]
    out["pooled"] = {"late": wilson_share(*pool["late"]), "early": wilson_share(*pool["early"])}
    out["text"] = "late D 5-8: wanderer %s, inputless %s, pooled %s; early pooled %s" % (
        out["wanderer"]["late"]["text"], out["inputless"]["late"]["text"], out["pooled"]["late"]["text"], out["pooled"]["early"]["text"])
    return out


def r7_lt15(jobs, parsed, results, ctx):
    out = {}
    for cell in ("inputless", "wanderer", "death"):
        if cell == "death" and ctx["drop_death"]:
            out[cell] = {"text": DEATH_DROPPED % (ctx["t_death"], ctx["c"])}
            continue
        items = items_of(jobs, parsed, results, cell)
        k = sum(1 for j, p, ok in items if ok and (p.get("ending_x") or {}).get("kind") == "OtherPeople" and (p.get("ending_x") or {}).get("no_vessel") == 1)
        sh = wilson_share(k, len(items))
        out[cell] = {"k": k, "n": len(items), "share": sh, "text": sh["text"]}
    return out


def r8_kinds(jobs, parsed, results):
    out = {}
    for cell in ("R", "slowR"):
        items = items_of(jobs, parsed, results, cell)
        n = len(items)
        kinds = {}
        for j, p, ok in items:
            if ok:
                k = (p.get("ending_x") or {}).get("kind", "?")
                kinds[k] = kinds.get(k, 0) + 1
        out[cell] = {k: wilson_share(kinds.get(k, 0), n)["text"] for k in KINDS_R8}
        extra = {k: v for k, v in kinds.items() if k not in KINDS_R8}
        if extra:
            out[cell]["other"] = extra
    return out


def deaths_of_run(p):
    """(n, k, succeeded) of one run: n = deaths before the record (the 'by end' count), succeeded = successions at or before
    the record tick (each succession follows its death, in order), k = n - succeeded."""
    cl = p.get("calling") or {}
    rec = (p.get("ending_x") or {}).get("tick", -1)
    n = cl.get("by_end", 0)
    succ = sum(1 for t, _ in cl.get("successions", []) if 0 <= t <= rec)
    return n, n - succ, succ


def r9_10(jobs, parsed, results, ctx):
    def cell_sum(cell):
        n = k = runs_in = runs_with = 0
        clock_in = 0
        odd = []
        for j, p, ok in items_of(jobs, parsed, results, cell):
            if not ok:
                continue
            dn, dk, _ = deaths_of_run(p)
            if dk < 0:
                odd.append(j["seed"])
                dk = 0
            n += dn
            k += dk
            runs_with += dn > 0
            if dk > 0:
                runs_in += 1
                clock_in += (p.get("ending_x") or {}).get("kind") == "Clock"
        return {"n": n, "k": k, "no_succession": wilson_share(k, n), "succeeded": wilson_share(n - k, n), "runs_with_deaths": runs_with,
                "runs_in_interregnum": runs_in, "of_them_clock": clock_in, "odd": odd}
    out = {"wanderer": cell_sum("wanderer"), "inputless": cell_sum("inputless")}
    n = out["wanderer"]["n"] + out["inputless"]["n"]
    k = out["wanderer"]["k"] + out["inputless"]["k"]
    out["pool"] = {"n": n, "k": k, "no_succession": wilson_share(k, n), "succeeded": wilson_share(n - k, n)}
    if n < 30:
        out["pool"]["text"] = "%s (k/n %s)" % (NOT_MEASURED % n, out["pool"]["no_succession"]["text"])
    else:
        out["pool"]["text"] = "no succession by the record %s; succeeded (B5 (3)) %s" % (out["pool"]["no_succession"]["text"], out["pool"]["succeeded"]["text"])
    out["death"] = {"text": DEATH_DROPPED % (ctx["t_death"], ctx["c"])} if ctx["drop_death"] else cell_sum("death")
    return out


def r10_158(jobs, parsed, results, ctx):
    family = {"R": "R", "death": "R", "wanderer": "R", "slowR": "slowR", "test": "test"}
    floor = {"death": ctx["t_death"], "wanderer": ABDICATE_AT}
    seen = {}
    merged = 0
    for cell in ("R", "death", "wanderer", "slowR", "test"):
        if cell == "death" and ctx["drop_death"]:
            continue
        for j, p, ok in items_of(jobs, parsed, results, cell):
            if not ok:
                continue
            for t, system, phase, flag in p.get("keys") or []:
                if phase not in (0, 1):
                    continue
                if cell in floor and not t > floor[cell]:
                    continue
                key = (family[cell], j["seed"], t, system)
                if key in seen:
                    merged += 1
                    continue
                seen[key] = flag
    n = len(seen)
    lost = sum(1 for f in seen.values() if f in ("w", "s"))
    sh = wilson_share(lost, n)
    out = {"n": n, "lost": lost, "share": sh, "merged": merged}
    if n < 30:
        out.update(verdict=None, text="%s (lost %s)" % (NOT_MEASURED % n, sh["text"]))
    elif lost * 10 >= n:
        out.update(verdict="developer", text="%s header first - lost %s >= 10%%" % (DEV, sh["text"]))
    else:
        out.update(verdict="closed", text="closed - lost %s < 10%%" % sh["text"])
    return out


def r11_runlen(jobs, parsed, results):
    out = {}
    for cell in ("R", "slowR"):
        ticks, clock, n = [], 0, 0
        for j, p, ok in items_of(jobs, parsed, results, cell):
            n += 1
            if ok:
                ex = p.get("ending_x") or {}
                if ex.get("tick", -1) >= 0:
                    ticks.append(ex["tick"])
                clock += ex.get("kind") == "Clock"
        out[cell] = {"min": min(ticks) if ticks else None, "median": median(ticks), "p90": p90(ticks), "clock": wilson_share(clock, n),
                     "text": "min %s, median %s, p90 %s; at the clock %s" % (fmtv(min(ticks) if ticks else None), fmtv(median(ticks)), fmtv(p90(ticks)),
                                                                           wilson_share(clock, n)["text"])}
    return out

# --------------------------------------------------------------------------------------------------------- hand checks


def handcheck_command(root, kind, c, seed, save_at, n, what, editor=EDITOR):
    """Brief 4.4 (B6a N:413 form, game shape - bSaveShape refuses -Rules, -DeathAt, -TellAll != 0 and -WithoutB6bData)."""
    stem = "B6b_%s_%s_seed%d_%d" % (n, what, seed, save_at)
    out_dir = os.path.join(root, "Saved", "Balance", "b6b_handcheck")
    args = ["-run=CosmicBalance", "-Ticks=%d" % c, "-Seed=%d" % seed, "-SaveAt=%d" % save_at, "-TellAll=0", "-Answer=1", "-Build=1",
            "-Focus=0" if kind == "slowR" else "-Focus=1", "-Edict=1", "-Diplomacy=1"]
    if kind == "wanderer":
        args += ["-Abdicate=1200", "-Wander=1"]
    args += ["-Out=" + A.fwd(os.path.join(out_dir, stem + ".csv")), "-unattended", "-nosplash", "-AbsLog=" + A.fwd(os.path.join(out_dir, stem + ".log"))]
    assert not any(a.startswith(("-Rules", "-DeathAt", "-WithoutB6bData", "-FourthRungCap")) for a in args)
    assert "-TellAll=0" in args and 1 <= save_at <= c
    cmd = [editor, os.path.join(root, "Cosmic_Ascension.uproject")] + args
    return {"cmd": A.quote_cmd(cmd), "save_dir": os.path.join(root, "Saved", "HandCheck", stem), "stem": stem}


def handcheck_seeds(jobs, parsed, results, ctx, root):
    C = ctx["c"]
    out = {}
    R = [(j, p) for j, p, ok in items_of(jobs, parsed, results, "R") if ok]
    S = [(j, p) for j, p, ok in items_of(jobs, parsed, results, "slowR") if ok]
    W = [(j, p) for j, p, ok in items_of(jobs, parsed, results, "wanderer") if ok]

    def pick(items, f):
        return next(((j, p) for j, p in items if f(p)), None)

    def ruled_pick(p):
        ex = p.get("ending_x") or {}
        return ex.get("kind") == "RuledPeople" and 0 <= ex.get("tick", -1) <= C and ((p.get("race") or {}).get("ruled") or {}).get("r4", -1) >= 1

    for kind, items in (("R", R), ("slowR", S)):
        hit = pick(items, ruled_pick)
        if hit:
            j, p = hit
            r4 = p["race"]["ruled"]["r4"]
            t = max(1, r4 - 50)
            out["1"] = dict(handcheck_command(root, kind, C, j["seed"], t, "1", "ruledT4", ctx.get("editor", EDITOR)), cell=kind, seed=j["seed"], save_at=t,
                            expected="ruled T4 at %d; record RuledPeople at %d" % (r4, p["ending_x"]["tick"]))
            break
    if "1" not in out:
        out["1"] = {"text": "%s - %s: milestone B's console-free ending rests on the clock end and hand check (5)" % (NONE_KO, DEV)}
    hit = pick(R, lambda p: (p.get("ending_x") or {}).get("kind") == "OtherPeople")
    if hit:
        j, p = hit
        t = max(1, p["ending_x"]["tick"] - 30)
        out["2"] = dict(handcheck_command(root, "R", C, j["seed"], t, "2", "otherOrder", ctx.get("editor", EDITOR)), cell="R", seed=j["seed"], save_at=t,
                        expected="record OtherPeople at %d" % p["ending_x"]["tick"])
    else:
        out["2"] = {"text": NONE_KO}
    ext = lambda p: (p.get("race") or {}).get("extinction") or {}  # noqa: E731
    with_ext = [(j, p) for j, p in R if ext(p).get("on_ruled", 0) >= 1 and ext(p).get("on_ruled_first", -1) >= 1]
    hit = (pick(with_ext, lambda p: ext(p)["ruled_expiries"] >= 1 and ext(p)["died"] >= 1) or pick(with_ext, lambda p: ext(p)["ruled_expiries"] >= 1)
           or (with_ext[0] if with_ext else None))
    if hit:
        j, p = hit
        f = ext(p)["on_ruled_first"]
        t = max(1, f - 10)
        cd = ctx.get("extinction_countdown")
        out["3"] = dict(handcheck_command(root, "R", C, j["seed"], t, "3", "extinction", ctx.get("editor", EDITOR)), cell="R", seed=j["seed"], save_at=t,
                        expected="extinction placed on ruled first at %d; ruled expiries %d (died %d); expected expiry about %s" % (
                            f, ext(p)["ruled_expiries"], ext(p)["died"], ("%d + guard delay" % (f + cd)) if cd else "placement + the row's countdown"))
    else:
        out["3"] = {"text": NONE_KO}
    side = lambda p: (p.get("race_b") or {}).get("side") or {}  # noqa: E731
    hit = pick(R, lambda p: 0 <= side(p).get("era_first", -1) < C and side(p).get("era", 0) >= 1)
    if hit:
        j, p = hit
        t = max(1, side(p)["era_first"] - 30)
        out["6i"] = dict(handcheck_command(root, "R", C, j["seed"], t, "6", "era", ctx.get("editor", EDITOR)), cell="R", seed=j["seed"], save_at=t,
                         expected="era line first at %d" % side(p)["era_first"])
    else:
        out["6i"] = {"text": "%s (windowless screenshot from after [B-6b8])" % NONE_KO}
    wit = lambda p: 0 <= (side(p).get("wit_first") if side(p).get("wit_first") is not None else -1) < C  # noqa: E731
    hit, kind = pick(R, wit), "R"
    if not hit:
        hit, kind = pick(W, wit), "wanderer"
    if hit:
        j, p = hit
        t = max(1, side(p)["wit_first"] - 30)
        out["6ii"] = dict(handcheck_command(root, kind, C, j["seed"], t, "6", "witness", ctx.get("editor", EDITOR)), cell=kind, seed=j["seed"], save_at=t,
                          expected="enc_witness first at %d" % side(p)["wit_first"])
    else:
        out["6ii"] = {"text": "%s (windowless screenshot from after [B-6b8])" % NONE_KO}
    out["4"] = {"text": "uses (1)'s save"}
    out["5"] = {"text": "no save"}
    return out

# ------------------------------------------------------------------------------------------------------------ analysis


def analyse(jobs, parsed, results, ctx, bstate=None, bparsed=None):
    by = {j["name"]: j for j in jobs}
    pairs, bb, bc = [], [], []
    for j in jobs:
        if j["off"] or not HAS_OFF.get(j["cell"]):
            continue
        o = by.get(j["name"] + "_off")
        if not o:
            continue
        if not (ok_of(results, j["name"]) and ok_of(results, o["name"])):
            pairs.append({"pair": j["name"], "cell": j["cell"], "ok": None, "why": ["skipped: a run of the pair failed twice"]})
            continue
        pairs.append(pair_check(j, o, parsed[j["name"]], parsed[o["name"]], j["cell"] == "wanderer"))
    for j in jobs:
        if j["off"] or j["cell"] not in ("death", "wanderer"):
            continue
        r = by.get("R_%d" % j["seed"])
        if not r:
            continue
        if not (ok_of(results, j["name"]) and ok_of(results, r["name"])):
            bb.append({"pair": "%s ~ %s" % (j["name"], r["name"]), "ok": None, "why": ["skipped: a run failed"]})
            continue
        bb.append(bridge_b(r, j, ctx["t_death"] if j["cell"] == "death" else ABDICATE_AT, "death <= T_DEATH" if j["cell"] == "death" else "wanderer <= 1,200"))
    for j in jobs:
        if j["cell"] == "wanderer" and j["off"]:
            bc.append(bridge_c(j, ctx["b6a_dir"]) if ok_of(results, j["name"]) else {"pair": j["name"], "ok": None, "why": ["skipped: run failed"]})
    be = bridge_e(jobs, parsed, results)
    bd = bridge_d(jobs, parsed, results, ctx, bstate, bparsed) if bstate is not None else {"ok": None, "why": ["not asked"], "entries": [], "moved": [], "moved_count": 0,
                                                                                          "may_move": {}, "skipped": [], "notes": []}
    ctl = controls(jobs, parsed, results, ctx)
    f7 = f7_check(jobs, parsed, results)
    failed, withheld = withheld_cells(jobs, results)
    bc_ok = bool(bc) and all(x["ok"] for x in bc)
    readings = {
        "R1": r1_136(jobs, parsed, results), "R2": r2_142(jobs, parsed, results, ctx, f7), "R3": r3_132(jobs, parsed, results),
        "R4": r4_b1(jobs, parsed, results, bc_ok and be["ok"] is True), "R5": r5_129(jobs, parsed, results), "R6": r6_154(jobs, parsed, results),
        "R7": r7_lt15(jobs, parsed, results, ctx), "R8": r8_kinds(jobs, parsed, results), "R9": r9_10(jobs, parsed, results, ctx),
        "R10": r10_158(jobs, parsed, results, ctx), "R11": r11_runlen(jobs, parsed, results)}
    hc = handcheck_seeds(jobs, parsed, results, ctx, ctx["root"])
    return {"pairs": pairs, "bridge_b": bb, "bridge_c": bc, "bridge_e": be, "bridge_d": bd, "controls": ctl, "f7": f7, "failed": failed,
            "withheld": withheld, "readings": readings, "handcheck": hc}


def bridges_ok(res):
    return (all(p["ok"] is not False for p in res["pairs"]) and all(b["ok"] is not False for b in res["bridge_b"])
            and all(b["ok"] is not False for b in res["bridge_c"]) and res["bridge_e"]["ok"] is not False and res["bridge_d"]["ok"] is not False)


def verdict(res, header):
    return (bool(header.get("sha1_ok", True)) and not header.get("aborted") and res["controls"]["ok"] and bridges_ok(res)
            and header.get("end_check", 0) == 0 and header.get("race_check", 0) == 0)

# ------------------------------------------------------------------------------------------------------------ markdown


READING_CELLS = {"<J136>": ("R",), "<N142>": ("test",), "<A132>": ("test",), "<B1>": ("wanderer",), "<129>": ("inputless", "inputless075"),
                 "<154>": ("wanderer", "inputless"), "<LT15 x3>": ("inputless", "wanderer", "death"), "<KINDS R/slowR>": ("R", "slowR"),
                 "<#10>": ("wanderer", "inputless", "death"), "<#158(3)>": ("R", "death", "wanderer", "slowR", "test"), "<RUNLEN>": ("R", "slowR")}


def placeholders(res, header, ctx):
    out = placeholders_raw(res, header, ctx)
    for key, cells in READING_CELLS.items():
        held = [c for c in cells if c in res["withheld"]]
        if held:
            out[key] = "WITHHELD (two or more twice-failed runs in %s: %s) - %s" % (", ".join(held), {c: res["failed"][c] for c in held}, out[key])
    return out


def placeholders_raw(res, header, ctx):
    rd = res["readings"]
    r4 = rd["R4"]
    b1 = r4.get("text", "-")
    b1 += "; on own %s, off own %s; any-vessel on %s, off %s; median after leaving on %s, off %s; on paths %s; gate: %s" % (
        r4["on"]["own_share"]["text"], r4["off"]["own_share"]["text"], r4["on"]["any_share"]["text"], r4["off"]["any_share"]["text"],
        fmtv(r4["on"]["median_after"]), fmtv(r4["off"]["median_after"]), r4.get("paths_on"), (r4.get("gate") or {}).get("text", "-"))
    r2 = rd["R2"]
    hc = res["handcheck"]
    return {
        "<C_MINUTES>": header.get("minutes", "-"), "<FROZEN_SHA1>": header.get("frozen", "-"),
        "<J136>": rd["R1"]["text"],
        "<N142>": "%s; n %d, excluded after binding %d (seeds %s), owner changed %d, k_e %d, joined %s, sum p %.2f, B4 form %s, cards after C %d" % (
            r2["text"], r2["n"], r2["after_binding"], r2["after_binding_seeds"], r2["owner_changed"], r2["k_e"], r2["joined"]["text"], r2["sum_p"],
            r2["b4_form"], r2["card_after_c"]),
        "<A132>": rd["R3"]["text"], "<B1>": b1, "<129>": rd["R5"]["text"], "<154>": rd["R6"]["text"],
        "<LT15 x3>": "; ".join("%s %s" % (c, rd["R7"][c]["text"]) for c in ("inputless", "wanderer", "death")),
        "<KINDS R/slowR>": "R %s; slowR %s" % (rd["R8"]["R"], rd["R8"]["slowR"]),
        "<#10>": "pool %s; wanderer %s; inputless %s; death %s" % (rd["R9"]["pool"]["text"], rd["R9"]["wanderer"].get("no_succession", {}).get("text"),
                                                                   rd["R9"]["inputless"].get("no_succession", {}).get("text"),
                                                                   rd["R9"]["death"].get("text") or rd["R9"]["death"]["no_succession"]["text"]),
        "<#158(3)>": rd["R10"]["text"],
        "<RUNLEN>": "R %s; slowR %s" % (rd["R11"]["R"]["text"], rd["R11"]["slowR"]["text"]),
        "<HC>": "; ".join("%s: %s" % (k, v.get("cmd") and "seed %d at %d (%s) - %s" % (v["seed"], v["save_at"], v["cell"], v["expected"]) or v["text"])
                          for k, v in hc.items()),
    }


def markdown(res, header, ctx):
    L = []
    L.append("# [B-6b11] bundle C - %s" % header["date"])
    L.append("")
    L.append("- build %s (git %s); C %d; T_DEATH %d%s; jobs %d; processes %s (reruns %s)" % (
        ctx["build_sha"], header.get("git"), ctx["c"], ctx["t_death"], " - death cell dropped" if ctx["drop_death"] else "", header["jobs"],
        header.get("processes"), header.get("reruns")))
    L.append("- folder `%s`%s" % (header["folder"], "; archive `%s`" % header["archive"] if header.get("archive") else ""))
    L.append("- end check %s, race check %s; sha1 guard: %s%s" % (header.get("end_check"), header.get("race_check"), header.get("sha1_guard"),
                                                                 "; ABORTED: %s" % header.get("aborted") if header.get("aborted") else ""))
    L.append("- freeze sha1: %s" % header.get("freeze"))
    L.append("- failed twice: %s; withheld cells: %s" % (res["failed"] or "none", res["withheld"] or "none"))
    c = res["controls"]
    L.append("- controls F1-F6, F8: %s" % ("pass" if c["ok"] else "FAIL"))
    for f, v in c["by"].items():
        L.append("  - %s: %s" % (f, "pass" if not v else "%d - %s" % (len(v), "; ".join(v[:8]))))
    for n in c["notes"]:
        L.append("  - note: %s" % n)
    f7 = res["f7"]
    L.append("- F7 (stops #142 only): %s%s; OwnerChanged=1 rows %d (seeds %s)" % ("pass" if f7["ok"] else "FAIL", "" if f7["ok"] else " - %s" % f7["broken"][:5],
                                                                                    f7["owner_changed_rows"], f7["owner_changed_seeds"]))
    bad = [p for p in res["pairs"] if p["ok"] is False]
    L.append("- B-a pairs: %d/%d pass%s; explained by the predictor: %s" % (
        sum(1 for p in res["pairs"] if p["ok"]), len(res["pairs"]), "" if not bad else " - " + "; ".join("%s (%s)" % (p["pair"], ", ".join(p["why"])) for p in bad[:8]),
        ["%s G %s" % (p["pair"], p.get("G")) for p in res["pairs"] if p.get("explained")]))
    for key, label in (("bridge_b", "B-b"), ("bridge_c", "B-c")):
        bad = [b for b in res[key] if b["ok"] is False]
        L.append("- %s: %d/%d pass%s" % (label, sum(1 for b in res[key] if b["ok"]), len(res[key]),
                                        "" if not bad else " - " + "; ".join("%s (%s)" % (b["pair"], ", ".join(b["why"])) for b in bad[:8])))
    be = res["bridge_e"]
    L.append("- B-e: %s" % ({True: "pass", False: "FAIL - ", None: ""}[be["ok"]] + "; ".join(be["why"])))
    bd = res["bridge_d"]
    L.append("- B-d (%s): %s; may move %s; moved %d %s; skipped %d%s" % (
        bd.get("mode"), {True: "pass", False: "FAIL", None: "skipped"}[bd["ok"]], bd.get("may_move"), bd.get("moved_count", 0),
        ["%s G %s" % (e["name"], e["G"]) for e in bd.get("moved", [])], len(bd.get("skipped", [])), ("; " + "; ".join(bd["why"])) if bd.get("why") else ""))
    for e in [e for e in bd.get("entries", []) if e["why"] and not e["may_move"]][:8]:
        L.append("  - broken %s: %s" % (e["name"], "; ".join(e["why"])))
    for n in bd.get("notes", []):
        L.append("  - note: %s" % n)
    L.append("- verdict: **%s**%s" % ("PASS" if header.get("verdict") else "FAIL", "" if bridges_ok(res) else " - a bridge broke: find the cause before reading anything"))
    L.append("")
    L.append("## Readings (brief 4.3, frozen)")
    for k, v in placeholders(res, header, ctx).items():
        L.append("- %s %s" % (k, v))
    L.append("")
    L.append("## Details")
    rd = res["readings"]
    L.append("- R2 #142: %s" % {k: v for k, v in rd["R2"].items() if k not in ("eased", "joined")})
    L.append("- R3 #132: %s" % rd["R3"])
    L.append("- R4 B1: L %s, G %s, paths %s, unexplained %s" % (rd["R4"].get("L"), rd["R4"].get("G"), rd["R4"].get("paths"), rd["R4"].get("unexplained")))
    L.append("- R6 #154: %s" % {k: v for k, v in rd["R6"].items() if k != "text"})
    L.append("- R9 #10: %s" % rd["R9"])
    L.append("- R10 #158(3): n %d lost %d merged after the boundary %d" % (rd["R10"]["n"], rd["R10"]["lost"], rd["R10"]["merged"]))
    L.append("")
    L.append("## Hand checks (brief 4.4 - selection only; run one at a time, move only the file on the log's 'wrote' line)")
    for k, v in res["handcheck"].items():
        if v.get("cmd"):
            L.append("- %s seed %d (%s) save at %d, expected %s; save to `%s`" % (k, v["seed"], v["cell"], v["save_at"], v["expected"], v["save_dir"]))
            L.append("  - `%s`" % v["cmd"])
        else:
            L.append("- %s %s" % (k, v["text"]))
    return "\n".join(L) + "\n"

# -------------------------------------------------------------------------------------------------------------- driver


def execute(job, project, editor, ignore_errors, state):
    """One job: the busy-process guard before every launch (a busy machine aborts the bundle), one rerun of a failed process."""
    attempts = []
    for attempt in (1, 2):
        if state["abort"].is_set():
            break
        busy = HOOKS["busy"]()
        if busy:
            with state["lock"]:
                state["aborted"] = state.get("aborted") or "busy before %s: %s" % (job["name"], "; ".join(busy)[:300])
            state["abort"].set()
            break
        A.remove_outputs(job)
        start = time.time()
        code = HOOKS["run"](A.command_of(job, project, editor))
        why = run_failures(job, parse_log(job["log"]), code, ignore_errors)
        attempts.append({"attempt": attempt, "exit": code, "seconds": round(time.time() - start, 1), "why": why})
        if not why:
            break
    with state["lock"]:
        ok = bool(attempts) and not attempts[-1]["why"]
        state["results"][job["name"]] = {"ok": ok, "attempts": attempts, "aborted": not ok and not (attempts and attempts[-1]["why"])}
        with open(state["results_path"], "w", encoding="utf-8") as f:
            json.dump(state["results"], f, ensure_ascii=False, indent=1)
    return job["name"], attempts


def evaluate(jobs, results, ignore_errors, need_files=True):
    parsed = {j["name"]: parse_log(j["log"]) for j in jobs}
    for j in jobs:
        known = results.get(j["name"])
        code = known["attempts"][-1]["exit"] if known and known.get("attempts") else None
        why = run_failures(j, parsed[j["name"]], code, ignore_errors, need_files)
        if known and known.get("aborted"):
            known["why_now"] = ["aborted"]
            continue
        if known:
            known["ok"] = not why
            known["why_now"] = why
        else:
            results[j["name"]] = {"ok": not why, "attempts": [], "why_now": why}
    return parsed


def sha1_of(path):
    return A.sha1_files([path])[path] if path and os.path.exists(path) else None


def freeze_hashes(frozen):
    files = [("bundleC.py", os.path.abspath(__file__)), ("bundleC_selftest.py", os.path.join(HERE, "bundleC_selftest.py")),
             ("bundleB.py", os.path.abspath(B.__file__)), ("bundleA.py", os.path.abspath(A.__file__)), ("frozen_4_3.md", frozen)]
    return {name: sha1_of(path) for name, path in files}


def num(v):
    try:
        return float(v)
    except (TypeError, ValueError):
        return None


def read_build_values(root, build_sha):
    """The values of brief 0.2 at <BUILD_SHA> (git show, read only)."""
    rules_text = HOOKS["git_show"](root, build_sha, "Tools/data/DT_Rules.json")
    events_text = HOOKS["git_show"](root, build_sha, "Tools/data/DT_Events.json")
    hazards_text = HOOKS["git_show"](root, build_sha, "Tools/data/DT_Hazards.json")
    if rules_text is None or events_text is None:
        return None, "git show %s:Tools/data/DT_Rules.json or DT_Events.json failed" % build_sha
    try:
        rules = next(r for r in json.loads(rules_text) if r.get("Name") == "rules")
        eased = next(r for r in json.loads(events_text) if r.get("Name") == "event_claim_eased")
        hz = next((r for r in json.loads(hazards_text or "[]") if r.get("Name") == "crisis_species_extinction"), {})
    except (ValueError, StopIteration) as e:
        return None, "unreadable data at %s: %s" % (build_sha, e)
    return {"c": rules.get("ProvisionalEndTick"), "r": rules.get("AIUpperRungProgressMultiplier"), "share": rules.get("FleetDelayCountdownShare"),
            "chance": rules.get("EventChancePerSlot"), "slot_ticks": rules.get("EventSlotTicks"), "ratio": rules.get("AIHostilePowerRatio"),
            "w_eased": eased.get("BaseWeight"), "extinction_countdown": hz.get("CountdownTicks")}, None


def check_values(a, vals):
    """Refusals of brief 0.2: every placeholder equal to the data at <BUILD_SHA>; share 0.5; C in [6,000, 18,000]; slots 10."""
    why = []
    close = lambda x, y: x is not None and y is not None and abs(float(x) - float(y)) < 1e-9  # noqa: E731
    if not close(a.c, vals["c"]):
        why.append("--c %s, ProvisionalEndTick at the build %s" % (a.c, vals["c"]))
    for opt, key in (("r", "r"), ("share", "share"), ("chance", "chance"), ("w_eased", "w_eased"), ("ratio", "ratio")):
        if not close(getattr(a, opt), vals[key]):
            why.append("--%s %s, the build has %s" % (opt.replace("_", "-"), getattr(a, opt), vals[key]))
    if not close(a.share, 0.5):
        why.append("<SHARE_COMMITTED> %s is not 0.5 - stop and ask the lead" % a.share)
    if not (C_RANGE[0] <= a.c <= C_RANGE[1]):
        why.append("C %d outside [6,000, 18,000] (rule (d), H:349) - stop and ask the lead" % a.c)
    if vals["slot_ticks"] != 10:
        why.append("EventSlotTicks %s, not 10" % vals["slot_ticks"])
    return why


def seeds_arg(text):
    if text is None:
        return None
    if text.strip().lower() in ("none", "-", ""):
        return []
    return parse_seeds(text)


PLACEHOLDERS = (("c", "--c (<C>)"), ("t_death", "--t-death (<T_DEATH>)"), ("build_sha", "--build-sha (<BUILD_SHA>)"), ("r", "--r (<R_COMMITTED>)"),
                ("chance", "--chance (<CHANCE>)"), ("w_eased", "--w-eased (<W_EASED>)"), ("ratio", "--ratio (<RATIO>)"), ("b9_ran", "--b9-ran (<B9_RAN>)"),
                ("br_world", "--br-world (<BR_WORLD>)"), ("k_moved_r", "--k-moved-r (<K_MOVED_R>)"), ("k_moved_w", "--k-moved-w (<K_MOVED_W>)"))


def check_only_b(folder, ctx, ignore_errors, max_seed):
    """Brief 2 / 7.1: bundle C's parsers over bundle B's logs (real text). Every part bundle C reads must parse in every log;
    the 'may move' seeds by the predictor are listed (B-d uses them)."""
    jobs = [dict(j, log=os.path.join(folder, j["name"] + ".log")) for j in B.build_jobs(folder, ctx["t_death"], max_seed=max_seed)]
    unread, failures, may = [], [], {}
    seen = 0
    for j in jobs:
        if not os.path.exists(j["log"]):
            unread.append("%s: no log" % j["name"])
            continue
        seen += 1
        p = parse_log(j["log"])
        why = run_failures(j, p, None, ignore_errors, need_files=False)
        if why:
            failures.append("%s: %s" % (j["name"], "; ".join(why)))
        if not (p.get("race_b") or {}).get("side") or p["race_b"]["side"]["layout"] != "new":
            unread.append("%s: side part not in the [B-6b8] layout" % j["name"])
        if not j["off"] and predicted(p):
            may.setdefault(j["cell"], []).append(j["seed"])
    ats = bundle_b_death_ats(folder, max_seed)
    refusal = death_refusal(ats, ctx["t_death"])
    return {"logs": seen, "jobs": len(jobs), "unread": unread, "failures": failures, "may_move": may, "death_at": sorted(k for k in ats if k is not None),
            "refusal": refusal, "ok": seen > 0 and not unread and not failures and not refusal}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=os.environ.get("PROJECT_ROOT", DEFAULT_ROOT))
    ap.add_argument("--folder", default=None)
    ap.add_argument("--c", type=int, default=None)
    ap.add_argument("--t-death", type=int, default=None)
    ap.add_argument("--build-sha", default=None)
    ap.add_argument("--r", type=float, default=None)
    ap.add_argument("--share", type=float, default=0.5)
    ap.add_argument("--chance", type=float, default=None)
    ap.add_argument("--w-eased", type=float, default=None)
    ap.add_argument("--ratio", type=float, default=None)
    ap.add_argument("--b9-ran", choices=("yes", "no"), default=None)
    ap.add_argument("--br-world", default=None)
    ap.add_argument("--k-moved-r", default=None)
    ap.add_argument("--k-moved-w", default=None)
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--mem-per-worker-gb", type=float, default=2.5)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--check-only", metavar="FOLDER", default=None)
    ap.add_argument("--cell-set", choices=("B", "C"), default="C")
    ap.add_argument("--cells", default=None)
    ap.add_argument("--max-seed", type=int, default=None)
    ap.add_argument("--bundle-b-logs", default=None)
    ap.add_argument("--bundle-b-csv", default=None)
    ap.add_argument("--bundle-k-dir", default=None)
    ap.add_argument("--b6a-dir", default=None)
    ap.add_argument("--frozen", default=os.path.join(HERE, "frozen_4_3.md"))
    ap.add_argument("--archive", dest="archive", action="store_true", default=None)
    ap.add_argument("--no-archive", dest="archive", action="store_false")
    ap.add_argument("--archive-dir", default=None)
    ap.add_argument("--a-sec-per-20k", type=float, default=None)
    ap.add_argument("--ignore-error", action="append", default=[])
    ap.add_argument("--editor", default=EDITOR)
    a = ap.parse_args(argv)

    missing = [label for key, label in PLACEHOLDERS if getattr(a, key) is None]
    if missing:
        print("refusing: no default for the placeholders %s" % ", ".join(missing))
        return 2
    vals, err = read_build_values(a.root, a.build_sha)
    if err:
        print("refusing: %s" % err)
        return 2
    why = check_values(a, vals)
    if why:
        print("refusing: " + "; ".join(why))
        return 2
    br_world = "" if a.br_world.strip().lower() in ("none", "-", "") else a.br_world
    ctx = {"c": a.c, "t_death": a.t_death, "r": a.r, "share": a.share, "chance": a.chance, "w_eased": a.w_eased, "ratio": a.ratio,
           "build_sha": a.build_sha, "b9_ran": a.b9_ran == "yes", "br_world": br_world, "k_moved_r": seeds_arg(a.k_moved_r),
           "k_moved_w": seeds_arg(a.k_moved_w), "drop_death": a.t_death >= a.c, "root": a.root, "editor": a.editor,
           "b6a_dir": os.path.abspath(a.b6a_dir or os.path.join(a.root, "Saved", "Balance", "b6a_bundle1")),
           "extinction_countdown": vals.get("extinction_countdown")}
    cells = set(a.cells.split(",")) if a.cells else None
    project = os.path.join(a.root, "Cosmic_Ascension.uproject")

    if a.cell_set == "B":
        if not a.check_only:
            print("--cell-set B is for --check-only on bundle B's log folder")
            return 2
    # Brief 0.2: every bundle B death_*.log must say 'death asked at <T_DEATH>' (run, dry run and check-only alike).
    b_logs_dir = os.path.abspath(a.check_only if a.cell_set == "B" else (a.bundle_b_logs or os.path.join(a.root, "Saved", "Balance", "b6b_bundleB")))
    refusal = death_refusal(bundle_b_death_ats(b_logs_dir), a.t_death)
    if refusal:
        print(refusal)
        return 2
    if a.cell_set == "B":
        r = check_only_b(os.path.abspath(a.check_only), ctx, a.ignore_error, a.max_seed)
        print("bundle C parsers over bundle B's logs: %d of %d logs; unreadable %d, run failures %d" % (r["logs"], r["jobs"], len(r["unread"]), len(r["failures"])))
        for line in (r["unread"] + r["failures"])[:20]:
            print("  " + line)
        print("may move by the predictor (%s sum > %d): %s" % (AWAKENING, PREDICTOR_SUM, r["may_move"]))
        print("check-only B: %s" % ("PASS" if r["ok"] else "FAIL"))
        return 0 if r["ok"] else 1

    folder = os.path.abspath(a.check_only or a.folder or os.path.join(HERE, "bundleC"))
    jobs = build_jobs(folder, a.c, a.t_death, cells, a.max_seed, ctx["drop_death"])
    by_cell = {}
    for j in jobs:
        by_cell[j["cell"]] = by_cell.get(j["cell"], 0) + 1
    n_clock = sum(1 for j in jobs if j["cell"] in AT_CLOCK)
    n_20k = len(jobs) - n_clock
    freeze = freeze_hashes(os.path.abspath(a.frozen))

    if a.dry_run:
        for j in jobs:
            print(A.quote_cmd(A.command_of(j, project, a.editor)))
        est = ("; <C_MINUTES> ~ %.1f min at %d workers (%.1f s per 20,000-tick run)" % (wall_minutes(a.a_sec_per_20k, a.c, n_clock, n_20k, a.workers), a.workers,
                                                                                         a.a_sec_per_20k)) if a.a_sec_per_20k else \
            "; <C_MINUTES> = <A_SEC_PER_20K> x (%d x %d + %d x 20,000) / 20,000 / %d / 60 (pass --a-sec-per-20k)" % (n_clock, a.c, n_20k, a.workers)
        print("# %d jobs (%s); %d at C=%d, %d at 20,000; %d workers; folder %s%s%s" % (
            len(jobs), ", ".join("%s %d" % (c, by_cell[c]) for c in CELL_ORDER if c in by_cell), n_clock, a.c, n_20k, a.workers, folder, est,
            "; death cell dropped (%s)" % (DEATH_DROPPED % (a.t_death, a.c)) if ctx["drop_death"] else ""))
        print("# freeze sha1: %s" % freeze)
        return 0

    header = {"date": datetime.date.today().isoformat(), "folder": folder, "git": HOOKS["git_head"](a.root), "jobs": len(jobs), "freeze": freeze,
              "frozen": freeze.get("frozen_4_3.md") or "-", "bundle_k_dir": a.bundle_k_dir}
    results_path = os.path.join(folder, "runs.json")
    if a.check_only:
        results = {}
        if os.path.exists(results_path):
            with open(results_path, encoding="utf-8") as f:
                results = json.load(f)
        meta_path = os.path.join(folder, "bundle_meta.json")
        if os.path.exists(meta_path):
            with open(meta_path, encoding="utf-8") as f:
                meta = json.load(f)
            header.update({k: meta[k] for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes", "aborted", "freeze", "frozen") if k in meta})
        header.setdefault("sha1_guard", "not recorded (check-only without bundle_meta.json)")
    else:
        if not freeze.get("frozen_4_3.md"):
            print("refusing: no %s - the freeze step (brief 7.1 step 6) comes before job 1" % a.frozen)
            return 2
        busy = HOOKS["busy"]()
        if busy:
            print("refusing to start - running now:\n  " + "\n  ".join(busy))
            return 2
        avail, total = HOOKS["memory"]()
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
        for name, h in freeze.items():
            print("freeze sha1 %s %s" % (h, name), flush=True)
        before = A.sha1_files(files)
        os.makedirs(folder, exist_ok=True)
        header["dll_sha1"] = " ".join(sorted(h[:12] for p, h in before.items() if p.lower().endswith(".dll")))
        state = {"results": {}, "lock": threading.Lock(), "abort": threading.Event(), "aborted": None, "results_path": results_path}
        t0 = time.time()
        print("bundle C: %d jobs, %d workers, folder %s" % (len(jobs), a.workers, folder), flush=True)
        with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
            futures = [pool.submit(execute, j, project, a.editor, a.ignore_error, state) for j in jobs]
            for done, f in enumerate(concurrent.futures.as_completed(futures), 1):
                name, attempts = f.result()
                last = attempts[-1] if attempts else {"why": ["aborted"]}
                print("[%d/%d %.0fs] %s %s%s" % (done, len(jobs), time.time() - t0, name, "ok" if not last["why"] else "FAILED: " + "; ".join(last["why"]),
                                                " (rerun)" if len(attempts) > 1 else ""), flush=True)
        results = state["results"]
        header["aborted"] = state["aborted"]
        changed = A.sha1_diff(before, A.sha1_files(guarded_files(a.root)))
        header["sha1_ok"] = not changed
        header["sha1_guard"] = ("unchanged (%d files)" % len(before)) if not changed else "CHANGED - discard the bundle: " + "; ".join(changed)
        header["minutes"] = round((time.time() - t0) / 60.0, 1)
        with open(os.path.join(folder, "bundle_meta.json"), "w", encoding="utf-8") as f:
            json.dump({k: header.get(k) for k in ("dll_sha1", "sha1_guard", "sha1_ok", "date", "git", "minutes", "aborted", "freeze", "frozen")}, f,
                      ensure_ascii=False, indent=1)

    parsed = evaluate(jobs, results, a.ignore_error)
    header["end_check"] = sum(p.get("end_check", 0) for p in parsed.values())
    header["race_check"] = sum(p.get("race_check", 0) for p in parsed.values())
    header["processes"] = sum(len(results.get(j["name"], {}).get("attempts", [])) for j in jobs)
    header["reruns"] = sum(max(0, len(results.get(j["name"], {}).get("attempts", [])) - 1) for j in jobs)
    logs_dir = os.path.abspath(a.bundle_b_logs or os.path.join(a.root, "Saved", "Balance", "b6b_bundleB"))
    csv_dir = os.path.abspath(a.bundle_b_csv or os.path.join(os.path.dirname(HERE), "b6b8", "bundleB"))
    bstate = bundle_b_state(logs_dir, csv_dir, a.t_death, a.max_seed)
    bparsed = {name: parse_log(j["log"]) for name, j in bstate["jobs"].items()} if bstate["logs_ok"] is not None else {}
    header["bundle_b_csv"] = bstate["csv_ok"]
    res = analyse(jobs, parsed, results, ctx, bstate, bparsed)
    header["verdict"] = verdict(res, header)
    do_archive = a.archive if a.archive is not None else not a.check_only
    if do_archive:
        header["archive"] = os.path.abspath(a.archive_dir or os.path.join(a.root, "Saved", "Balance", "b6b_bundleC"))
    md = markdown(res, header, ctx)
    with open(os.path.join(folder, "summary.json"), "w", encoding="utf-8") as f:
        json.dump(A.jsonable({"header": header, "ctx": ctx, **res}), f, ensure_ascii=False, indent=1)
    with open(os.path.join(folder, "summary.md"), "w", encoding="utf-8") as f:
        f.write(md)
    if do_archive:
        header["archived_files"] = B.archive(folder, header["archive"])
    print(md)
    print("bundle C: %s" % ("PASS" if header["verdict"] else "FAIL"))
    return 0 if header["verdict"] else 1


if __name__ == "__main__":
    sys.exit(main())
