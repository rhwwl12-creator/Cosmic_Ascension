"""[B-6b7] the read-only computations the brief (scratchpad\\b6b7\\brief_final.md) asks of a scratch script before the
implementers run, over the [B-6b6] bundle A outputs that scratchpad\\b6b6\\bundleA.py wrote. Nothing is written unless
--out names a file outside the project; no commandlet, no git write (DT_Civs.json is read with `git show HEAD:`).

  section 0.3  P_BIND_A guard: the bundle A resample (bundleA.recompute_45) at P_bind and r = R_A - inside 15-35%?
  section 0.4  M_AWAKE: per R on-run with a ruled T4, k (ruled Systems at the first sample >= T4), a (archetype factor on
               Hazard.AIAwakening, DT_Civs), R_L (ruled Society 1.0 - T4, from the ending line), S_m for m in
               {0.5, 0.375, 0.25}; the largest m whose median S_m <= 3,600 (n < 5: the k {4, 7}, a 1.2 model);
               and #136 - R on-runs with a first-person line heard before the record or before 6,000 (Wilson 95%).
  section 0.5  LINES_MOVED_B6B7: T4* of each of the five baseline runs, whether it lies inside the horizon, the hash runs,
               and the commit-title phrase.
  section 4    (after the implementers - optional) --a1 <A' folder>: J1-J4, #136 on A' against A, and the probe seeds of
               section 5 (PROBE_SEED_1 / PROBE_SEED_2) from A' and the baseline logs.

  python -I -X utf8 m_awake.py [--root R] [--folder <bundle A>] [--pbind 1800] [--n 1200] [--r 0.375]
                              [--baseline-logs DIR ...] [--a1 DIR] [--civs-json FILE] [--out FILE]

The line formats are those of Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp at 529cab9 (= cef8cc7 for that
file): the race line :3955-3993, the ending line :3890, the pressure line :3818 ('first claim line at'), the CSV :1716.
"""
import argparse
import importlib.util
import json
import math
import os
import re
import subprocess
import sys

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:  # noqa: BLE001
    pass

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
DEFAULT_BUNDLE_A_PY = os.path.join(os.path.dirname(HERE), "b6b6", "bundleA.py")

M_LADDER = (0.5, 0.375, 0.25)          # brief 0.4: largest admissible first
S_TARGET = 3600                        # H 5.3 (3): the season's upper end
SPAWN_PER_SYSTEM = 0.0002              # N (c)/(d): lambda = 0.0002 * a * k * m
MODEL_K = (4, 7)                       # brief 0.4: n < 5
MODEL_A = 1.2
MIN_RUNS = 5
VOICE_HORIZON = 6000                   # #136: before the record or before 6,000
BAND = (0.15, 0.35)
AWAKENING = "Hazard.AIAwakening"
# The runner's five lines (Tools/run_checks.ps1:116-132): log name, horizon, Korean name for the title; and the hash runs.
BASELINE_RUNS = (("checks", 3000, "입력 없는 3,000"), ("checks_ruler", 3000, "통치자 3,000"), ("checks_wanderer", 6000, "방랑자 6,000"),
                 ("checks_20000", 20000, "입력 없는 20,000"), ("checks_ruler_20000", 20000, "통치자 20,000"))
HASH_RUNS = (("hashrun", "checks", "해시 판"), ("hashrun_policy", "checks_ruler", "정책 해시 판"))


def load_bundle_a(path):
    spec = importlib.util.spec_from_file_location("bundleA", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


A = load_bundle_a(os.environ.get("BUNDLE_A_PY", DEFAULT_BUNDLE_A_PY))

# ------------------------------------------------------------------------------------------------------------ parsing

RE_ENDING_HEAD = re.compile(r"record (?P<kind>\S+) at (?P<tick>-?\d+) \(people (?P<civ>[^,]*), a people some life ruled (?P<some>-?\d+), "
                            r"no vessel (?P<novessel>-?\d+)\)")
RE_ENDING_RESEARCH = re.compile(r"research 1\.0 ruled (?P<civ>\S+) \[(?P<fields>[^\]]*)\], first AI (?P<aciv>\S+) \[(?P<afields>[^\]]*)\]")
RE_ENDING_VOICE = re.compile(r"by the record told: first-person claim lines (?P<voice>-?\d+)")
RE_PRESSURE_VOICE = re.compile(r"CosmicBalance pressure: .*?first sign at (?P<sign>-?\d+), first claim line at (?P<first>-?\d+)")
RE_LAWS = re.compile(r"laws first held ruled at (?P<ruled>-?\d+), AI at (?P<ai>-?\d+) \((?P<ai_civ>[^)]*)\), longest held ruled (?P<long_r>-?\d+) "
                     r"AI (?P<long_a>-?\d+), resets ruled (?P<resets_r>-?\d+) \(by upkeep (?P<upkeep_r>-?\d+)\) AI (?P<resets_a>-?\d+) "
                     r"\(by upkeep (?P<upkeep_a>-?\d+), held again in the tick (?P<held_again>-?\d+)\), ruled laws dropped at an input (?P<dropped>-?\d+)")


def fields_of(text):
    """'Engineering=12 Physics=-1' -> {'Engineering': 12, 'Physics': -1}."""
    out = {}
    for tok in text.split():
        if "=" in tok:
            k, v = tok.split("=", 1)
            try:
                out[k] = int(v)
            except ValueError:
                pass
    return out


def parse_extra(path):
    """The readings bundleA.parse_log does not take: the ending line's head, research and voice; the pressure line's
    first claim line; race part 10 (laws)."""
    out = {"ending_x": None, "research": None, "voice_by_record": None, "first_voice": None, "laws": None}
    if not os.path.exists(path):
        return out
    text = A.read_text(path)
    m = A.last_match(r"CosmicBalance ending: (.*?)\r?$", text)
    if m:
        e = m.group(1)
        h = RE_ENDING_HEAD.search(e)
        if h:
            out["ending_x"] = {"kind": h.group("kind"), "tick": int(h.group("tick")), "civ": h.group("civ"), "some_life_ruled": int(h.group("some")),
                               "no_vessel": int(h.group("novessel"))}
        r = RE_ENDING_RESEARCH.search(e)
        if r:
            out["research"] = {"civ": r.group("civ"), "fields": fields_of(r.group("fields")), "ai_civ": r.group("aciv"), "ai_fields": fields_of(r.group("afields"))}
        v = RE_ENDING_VOICE.search(e)
        if v:
            out["voice_by_record"] = int(v.group("voice"))
    m = A.last_match(RE_PRESSURE_VOICE.pattern, text)
    if m:
        out["first_voice"] = int(m.group("first"))
    m = A.last_match(r"CosmicBalance race: (.*?)\r?$", text)
    if m:
        l = RE_LAWS.search(m.group(1))
        if l:
            out["laws"] = {k: (v if k == "ai_civ" else int(v)) for k, v in l.groupdict().items()}
    return out


def parse_run(log):
    p = A.parse_log(log)
    p.update(parse_extra(log))
    return p


def archetype_of(csv_path, civ):
    """The Archetype column of the civ's first CSV row (None if the civ has no row)."""
    text = A.read_text(csv_path)
    lines = [l for l in text.splitlines() if l.strip()]
    ix = {h: i for i, h in enumerate(lines[0].split(","))}
    for l in lines[1:]:
        c = l.split(",")
        if c[ix["CivID"]] == civ:
            return c[ix["Archetype"]]
    return None


def awakening_factors(civ_rows):
    """{archetype row: factor on Hazard.AIAwakening} from DT_Civs rows (absent key -> 1 at the reader)."""
    out = {}
    for row in civ_rows:
        mult = row.get("HazardSpawnMultipliers") or {}
        if AWAKENING in mult:
            out[row["Name"]] = float(mult[AWAKENING])
    return out


def load_civs(root, path=None):
    if path:
        with open(path, encoding="utf-8-sig") as f:
            return json.load(f), path
    out = subprocess.run(["git", "--no-optional-locks", "-C", root, "show", "HEAD:Tools/data/DT_Civs.json"], capture_output=True, text=True,
                         encoding="utf-8", errors="replace", timeout=60)
    if out.returncode != 0:
        raise RuntimeError("git show HEAD:Tools/data/DT_Civs.json failed: %s" % out.stderr.strip())
    return json.loads(out.stdout.lstrip("\ufeff")), "git show HEAD:Tools/data/DT_Civs.json"

# --------------------------------------------------------------------------------------------------------- arithmetic


def lower_median(values):
    """The commandlet's MedianOf (CosmicBalanceCommandlet.cpp:765-774): the lower middle of an even count; inf sorts last."""
    vals = sorted(values)
    if not vals:
        return None
    return vals[(len(vals) - 1) // 2]


def upper_median(values):
    vals = sorted(values)
    if not vals:
        return None
    return vals[len(vals) // 2]


def season_of(P, N, R_L, a, k, m):
    """N (d): S = max(P, R_L + (e^(lambda N) - 1) / lambda), lambda = 0.0002 a k m (U, the unpaid binding ticks, not measured)."""
    if R_L is None or math.isinf(R_L):
        return math.inf
    lam = SPAWN_PER_SYSTEM * a * k * m
    tail = N if lam == 0 else (math.exp(lam * N) - 1.0) / lam
    return max(float(P), R_L + tail)


def wilson_text(k, n):
    lo, hi = A.wilson(k, n)
    if not n:
        return "0/0"
    return "%d/%d = %.3f [%.3f, %.3f]" % (k, n, k / n, lo, hi)


def fmt(v):
    return A.fmt(v)

# ----------------------------------------------------------------------------------------------------- section 0.4 M


def r_jobs(folder, seeds):
    """Bundle A's R on-runs (names R_<seed>) in folder."""
    return [j for j in A.build_jobs(folder, cells={"R"}) if not j["off"] and j["seed"] in seeds]


def m_rows(folder, seeds, factors, P, N):
    """One row per R on-run with a ruled T4 (race part 3); a run without the T4 or unreadable is listed in 'skipped'."""
    rows, skipped = [], []
    for j in r_jobs(folder, seeds):
        p = parse_run(j["log"])
        race = p.get("race") or {}
        ru = race.get("ruled")
        if not p.get("log_exists") or not ru:
            skipped.append((j["seed"], "no log or no readable race part 3"))
            continue
        if ru["r4"] < 0:
            skipped.append((j["seed"], "no ruled T4"))
            continue
        if not os.path.exists(j["csv"]):
            skipped.append((j["seed"], "no CSV"))
            continue
        civ, t4 = ru["r4_civ"], ru["r4"]
        track = A.civ_track(j["csv"])["track"].get(civ, [])
        k = next((x[2] for x in track if x[0] >= t4), None)
        if k is None:
            skipped.append((j["seed"], "%s has no CSV sample at or after T4 %d" % (civ, t4)))
            continue
        arch = archetype_of(j["csv"], civ)
        a = factors.get(arch, 1.0)
        res = p.get("research")
        soc = res["fields"].get("Society") if res else None
        note = []
        if not res:
            note.append("no research part in the ending line")
        elif res["civ"] != civ:
            note.append("the ending line's research people %s is not the T4 people %s" % (res["civ"], civ))
        if soc is None or soc < 0:
            R_L = math.inf
            note.append("Society 1.0 never reached: S = inf")
        else:
            R_L = max(0, soc - t4)
        S = {m: season_of(P, N, R_L, a, k, m) for m in M_LADDER}
        rows.append({"seed": j["seed"], "civ": civ, "t4": t4, "k": k, "arch": arch, "a": a, "society": soc, "R_L": R_L, "S": S, "note": "; ".join(note)})
    return rows, skipped


def decide_m(rows, P, N):
    """Brief 0.4: M_AWAKE = the largest of {0.5, 0.375, 0.25} whose median S_m <= 3,600; none -> 0.5 and [개발자];
    n < 5 -> the k {4, 7}, a 1.2 model (R_L 0). Medians: the lower middle (MedianOf); the upper middle is printed beside it
    and a disagreement on the choice is flagged."""
    n = len(rows)
    out = {"n": n, "medians": {}, "medians_upper": {}, "model": None}
    if n < MIN_RUNS:
        model = {m: [season_of(P, N, 0, MODEL_A, k, m) for k in MODEL_K] for m in M_LADDER}
        out["model"] = model
        fits = [m for m in M_LADDER if max(model[m]) <= S_TARGET]
        out["m"], out["why"] = (fits[0], "n %d < 5: model k %s, a %.1f, every model S <= 3,600 at the largest such m" % (n, MODEL_K, MODEL_A)) if fits \
            else (0.5, "[개발자] - n < 5 and no m keeps the model S <= 3,600 (N cannot go below 1,200, H:344)")
        out["developer"] = not fits
        out["upper_disagrees"] = False
        return out
    for m in M_LADDER:
        out["medians"][m] = lower_median([r["S"][m] for r in rows])
        out["medians_upper"][m] = upper_median([r["S"][m] for r in rows])
    fits = [m for m in M_LADDER if out["medians"][m] <= S_TARGET]
    fits_upper = [m for m in M_LADDER if out["medians_upper"][m] <= S_TARGET]
    if fits:
        out["m"], out["why"], out["developer"] = fits[0], "largest m with median S <= 3,600", False
    else:
        out["m"], out["why"], out["developer"] = 0.5, "[개발자] - no m keeps the median S <= 3,600; keep 0.5 (N floor 1,200, H:344)", True
    out["upper_disagrees"] = (fits_upper[0] if fits_upper else None) != (fits[0] if fits else None)
    return out

# ---------------------------------------------------------------------------------------------------------- #136


def voice_reading(folder, seeds):
    """#136 per R on-run: by the record (the ending line's first-person claim lines > 0 - N:162's method), by 6,000 (the
    pressure line's first claim line at <= 6,000), by min(record, 6,000) (the B6a 6,000-clock reading) and the brief's
    'before the record or before 6,000' (either)."""
    runs = []
    for j in r_jobs(folder, seeds):
        p = parse_run(j["log"])
        if not p.get("log_exists") or p.get("voice_by_record") is None or p.get("first_voice") is None or not p.get("ending_x"):
            runs.append({"seed": j["seed"], "missing": True})
            continue
        rec = p["ending_x"]["tick"]
        first = p["first_voice"]
        by_record = p["voice_by_record"] > 0
        by_6000 = 0 <= first <= VOICE_HORIZON
        by_min = by_record if (0 <= rec <= VOICE_HORIZON) else by_6000
        runs.append({"seed": j["seed"], "missing": False, "record": rec, "kind": p["ending_x"]["kind"], "first": first, "by_record": by_record,
                     "by_6000": by_6000, "by_min": by_min, "either": by_record or by_6000})
    good = [r for r in runs if not r["missing"]]
    n = len(good)
    out = {"runs": runs, "n": n, "missing": [r["seed"] for r in runs if r["missing"]]}
    for key in ("either", "by_record", "by_6000", "by_min"):
        k = sum(1 for r in good if r[key])
        out[key] = {"k": k, "n": n, "text": wilson_text(k, n)}
    return out

# ------------------------------------------------------------------------------------------------------- section 0.5


def find_log(dirs, name):
    for d in dirs:
        p = os.path.join(d, name + ".log")
        if os.path.exists(p):
            return p
    return None


def t4_star(race):
    """T4* = min(part 2 first AI T4, part 3 ruled T4) and who climbed; (-1, '-') when neither."""
    a, r = race.get("ai_t4"), race.get("ruled")
    cands = []
    if a and a["t4_1"] >= 0:
        cands.append((a["t4_1"], "AI %s" % a["t4_1_civ"]))
    if r and r["r4"] >= 0:
        cands.append((r["r4"], "ruled %s" % r["r4_civ"]))
    if not cands:
        return -1, "-"
    cands.sort(key=lambda c: (c[0], 0 if c[1].startswith("AI") else 1))
    t = cands[0][0]
    who = " and ".join(c[1] for c in cands if c[0] == t)
    return t, who


def lines_moved(dirs):
    """Brief 0.5: a line moves only if T4* < its horizon; a hash run moves with its paired 3,000 line."""
    lines = []
    for name, horizon, label in BASELINE_RUNS:
        log = find_log(dirs, name)
        if not log:
            lines.append({"name": name, "label": label, "log": None, "moved": None, "why": "no %s.log in %s" % (name, dirs)})
            continue
        p = A.parse_log(log)
        race = p.get("race") or {}
        if not race.get("ai_t4") or not race.get("ruled"):
            lines.append({"name": name, "label": label, "log": log, "moved": None, "why": "race parts 2-3 unreadable"})
            continue
        h = p["seed_line"]["ticks"] if p.get("seed_line") else horizon
        t, who = t4_star(race)
        lines.append({"name": name, "label": label, "log": log, "horizon": h, "t4": t, "who": who, "moved": t >= 0 and t < h,
                      "why": "" if h == horizon else "horizon %d read from the seed line (runner's is %d)" % (h, horizon)})
    by = {l["name"]: l for l in lines}
    hashes = [{"name": hn, "paired": paired, "label": label, "moved": by[paired]["moved"]} for hn, paired, label in HASH_RUNS]
    unknown = [l["name"] for l in lines if l["moved"] is None]
    moved = [l for l in lines if l["moved"]]
    if unknown:
        phrase = None
    elif not moved:
        phrase = "비트 그대로"
    elif len(moved) == len(lines):
        phrase = "기준값 다섯과 해시 판 둘이 옮았다"
    else:
        hm = [h["label"] for h in hashes if h["moved"]]
        phrase = "기준값 %s%s이 옮았다" % ("·".join(l["label"] for l in moved), (" 줄과 " + "·".join(hm)) if hm else " 줄")
    return {"lines": lines, "hashes": hashes, "phrase": phrase, "unknown": unknown}

# ------------------------------------------------------------------------------------------------------- section 0.3


def resample_check(folder, P, R_A, N):
    """Brief 0.3: bundleA's 4.5 resample at P (R and input-less on-runs, read only) and its other-order share at R_A."""
    jobs = [j for j in A.build_jobs(folder, cells={"R", "inputless"}) if not j["off"]]
    by_name = {j["name"]: j for j in jobs}
    parsed = {j["name"]: A.parse_log(j["log"]) for j in jobs}
    rc = A.recompute_45(by_name, parsed, pbinds=(P,), N=N)
    d = rc["direct"][P].get(R_A)
    m = rc["mc"].get(P, {}).get(R_A)
    inside = lambda res: res is not None and res["other_share"]["p"] is not None and BAND[0] <= res["other_share"]["p"] <= BAND[1]  # noqa: E731
    return {"P": P, "R_A": R_A, "direct": d["other_share"]["text"] if d else "-", "direct_inside": inside(d),
            "mc": m["other_share"]["text"] if m else "-", "mc_inside": inside(m), "decision": rc["decision"].get(P), "t": rc["t"]}

# ------------------------------------------------------------------------------------------------- section 4 (A')


def j_checks(a_folder, a1_folder, seeds):
    """Brief 4: J1 (rows with Tick < T4* of the [B-6b6] run equal on A'), J2 (0 check lines and errors over A' R logs),
    J3 (laws wiring), J4 (binding wiring)."""
    out = {"J1": [], "J2": [], "J3": None, "J4": None}
    base = {j["seed"]: j for j in r_jobs(a_folder, seeds)}
    new = {j["seed"]: j for j in r_jobs(a1_folder, seeds)}
    parsed_new = {}
    for seed in sorted(seeds):
        b, n = base.get(seed), new.get(seed)
        pb = A.parse_log(b["log"]) if b else {}
        pn = parse_run(n["log"]) if n else {}
        parsed_new[seed] = pn
        if not pb.get("race") or not pn.get("log_exists"):
            out["J1"].append({"seed": seed, "ok": False, "why": "a log or its race line is missing"})
            continue
        t, who = t4_star(pb["race"])
        limit = t if t >= 0 else math.inf
        why = []
        for key in ("csv", "rows"):
            if not (os.path.exists(b[key]) and os.path.exists(n[key])):
                why.append("missing %s" % key)
                continue
            hb, rb = A.read_csv_by_tick(b[key])
            hn, rn = A.read_csv_by_tick(n[key])
            if hb != hn:
                why.append("%s headers differ" % key)
            d = A.first_difference(rb, rn)
            if d is not None and d < limit:
                why.append("%s differs at %d < T4* %s" % (key, d, fmt(limit)))
        out["J1"].append({"seed": seed, "ok": not why, "why": "; ".join(why), "T4*": t, "who": who})
        f = A.run_failures(n, pn, None)
        bad = [w for w in f if "check line" in w or "LogCosmic: Error" in w or w == "no log"]
        if bad:
            out["J2"].append({"seed": seed, "why": bad})
    # J3: a ruled T4 with Society 1.0 before the run's end -> some such run holds the laws ruled.
    eligible, held = [], []
    any_t4, wired = [], []
    for seed, p in parsed_new.items():
        race = p.get("race") or {}
        ru, a4, up = race.get("ruled"), race.get("ai_t4"), race.get("upper")
        soc = (p.get("research") or {}).get("fields", {}).get("Society", -1)
        if ru and ru["r4"] >= 0 and soc is not None and soc >= 0:
            eligible.append(seed)
            if p.get("laws") and p["laws"]["ruled"] >= 0:
                held.append(seed)
        if (ru and ru["r4"] >= 0) or (a4 and a4["t4_1"] >= 0):
            any_t4.append(seed)
            if up and (up["ruled_binding"] >= 0 or up["ai_start"] >= 0):
                wired.append(seed)
    out["J3"] = {"eligible": eligible, "held": held, "ok": (not eligible) or bool(held)}
    out["J4"] = {"any_t4": any_t4, "wired": wired, "ok": (not any_t4) or bool(wired)}
    out["J1_ok"] = all(r["ok"] for r in out["J1"])
    out["J2_ok"] = not out["J2"]
    return out


def probe_seeds(dirs_or_files):
    """Brief 5: PROBE_SEED_1 - the earliest AI climb to T4 while another people already stands there (part 2 second AI T4,
    or the first AI T4 after a ruled T4); PROBE_SEED_2 - a ruled climb while exactly one AI stands there (first AI T4 <
    ruled T4 < second AI T4 or no second). Read from race parts 2-3 only (an AI erased in between is not seen)."""
    logs = []
    for d in dirs_or_files:
        if os.path.isdir(d):
            logs += [os.path.join(d, f) for f in sorted(os.listdir(d)) if f.endswith(".log") and not f.endswith("_off.log") and not f.endswith("_2.log")]
        elif os.path.exists(d):
            logs.append(d)
    c1, c2 = [], []
    for log in logs:
        p = A.parse_log(log)
        race = p.get("race") or {}
        a, r = race.get("ai_t4"), race.get("ruled")
        if not a or not r:
            continue
        seed = p["seed_line"]["seed"] if p.get("seed_line") else None
        ticks = []
        if a["t4_2"] >= 0:
            ticks.append((a["t4_2"], "second AI T4 (%s) after %s at %d" % (a["t4_2_civ"], a["t4_1_civ"], a["t4_1"])))
        if r["r4"] >= 0 and a["t4_1"] >= 0 and r["r4"] < a["t4_1"]:
            ticks.append((a["t4_1"], "first AI T4 (%s) after the ruled T4 %d" % (a["t4_1_civ"], r["r4"])))
        if ticks:
            t, why = min(ticks)
            c1.append({"log": log, "seed": seed, "tick": t, "ticks_arg": t + 100, "why": why})
        if r["r4"] >= 0 and 0 <= a["t4_1"] < r["r4"] and (a["t4_2"] < 0 or a["t4_2"] > r["r4"]):
            c2.append({"log": log, "seed": seed, "tick": r["r4"], "ticks_arg": r["r4"] + 100, "ruled": r["r4_civ"],
                       "why": "ruled %s T4 at %d with only %s (T4 %d) before it" % (r["r4_civ"], r["r4"], a["t4_1_civ"], a["t4_1"])})
    c1.sort(key=lambda c: c["tick"])
    c2.sort(key=lambda c: c["tick"])
    return {"logs": len(logs), "probe1": c1, "probe2": c2}

# --------------------------------------------------------------------------------------------------------- report


def report(res):
    L = []
    h = res["header"]
    L.append("# [B-6b7] m_awake — 묶음 A에서 읽은 것")
    L.append("")
    L.append("- 묶음 A 폴더: `%s`; R 시드 %s; P_BIND_A %s, N_A %s, R_A %s; DT_Civs: %s" % (h["folder"], h["seeds"], h["P"], h["N"], h["R_A"], h["civs_source"]))
    L.append("- 아키타입의 각성 몫: %s (없는 아키타입 1)" % h["factors"])
    rs = res.get("resample")
    if rs:
        L.append("")
        L.append("## §0.3 P_BIND_A 지킴 — 묶음 A의 다시 셈을 P %s, r %s에서" % (rs["P"], rs["R_A"]))
        L.append("- 직접 다시 뽑기: 다른 질서 %s — 띠 안 %s" % (rs["direct"], "예" if rs["direct_inside"] else "아니오"))
        L.append("- 몬테카를로: %s — 띠 안 %s" % (rs["mc"], "예" if rs["mc_inside"] else "아니오"))
        L.append("- 4.5의 판단(이 P): %s; t = %s (n %s)" % (rs["decision"], rs["t"]["t"], rs["t"]["n"]))
    m = res["m"]
    L.append("")
    L.append("## §0.4 M_AWAKE")
    L.append("")
    L.append("| seed | ruled T4 (civ) | k | archetype | a | Society 1.0 | R_L | S(0.5) | S(0.375) | S(0.25) | 메모 |")
    L.append("|---|---|---|---|---|---|---|---|---|---|---|")
    for r in res["m_rows"]:
        L.append("| %d | %d (%s) | %d | %s | %s | %s | %s | %s | %s | %s | %s |" % (r["seed"], r["t4"], r["civ"], r["k"], r["arch"], fmt(r["a"]), fmt(r["society"]),
                                                                   fmt(r["R_L"]), fmt(round(r["S"][0.5], 1)) if not math.isinf(r["S"][0.5]) else "inf",
                                                                   fmt(round(r["S"][0.375], 1)) if not math.isinf(r["S"][0.375]) else "inf",
                                                                   fmt(round(r["S"][0.25], 1)) if not math.isinf(r["S"][0.25]) else "inf", r["note"] or ""))
    if res["m_skipped"]:
        L.append("- 빠진 판: %s" % ", ".join("%d (%s)" % s for s in res["m_skipped"]))
    ks = [r["k"] for r in res["m_rows"]]
    L.append("- n %d; k 중앙값(아래) %s; a %s" % (m["n"], fmt(lower_median(ks)), sorted({r["a"] for r in res["m_rows"]})))
    if m["model"]:
        L.append("- 모형(n < 5): %s" % {mm: [round(x) for x in v] for mm, v in m["model"].items()})
    else:
        L.append("- S의 중앙값(아래 가운데 — MedianOf): %s; (위 가운데: %s)" % (
            ", ".join("m %s → %s" % (mm, fmt(round(v, 1)) if not math.isinf(v) else "inf") for mm, v in m["medians"].items()),
            ", ".join("m %s → %s" % (mm, fmt(round(v, 1)) if not math.isinf(v) else "inf") for mm, v in m["medians_upper"].items())))
    L.append("- **M_AWAKE = %s** — %s%s" % (m["m"], m["why"], " (위 가운데로는 다른 답 — 표시)" if m.get("upper_disagrees") else ""))
    v = res["voice"]
    L.append("")
    L.append("## §0.4 #136 — 묶음 A R 켬 (N:162의 방법)")
    L.append("- 기록 전 또는 6,000 전(둘 중 하나): **%s**" % v["either"]["text"])
    L.append("- 기록 전(ending 줄의 first-person claim lines > 0): %s; 6,000 이하(pressure 줄의 first claim line at): %s; min(기록, 6,000) 전: %s" % (
        v["by_record"]["text"], v["by_6000"]["text"], v["by_min"]["text"]))
    if v["missing"]:
        L.append("- 읽지 못한 판: %s" % v["missing"])
    L.append("- 판마다(seed:기록 종류@틱:첫 줄): %s" % " ".join("%d:%s@%d:%d" % (r["seed"], r["kind"], r["record"], r["first"]) for r in v["runs"] if not r["missing"]))
    lm = res.get("lines")
    if lm:
        L.append("")
        L.append("## §0.5 LINES_MOVED_B6B7")
        L.append("| 줄 | 로그 | 지평 | T4* (누구) | 옮음 |")
        L.append("|---|---|---|---|---|")
        for l in lm["lines"]:
            if l["moved"] is None:
                L.append("| %s | %s | - | - | ? (%s) |" % (l["name"], l.get("log") or "-", l["why"]))
            else:
                L.append("| %s | `%s` | %d | %d (%s) | %s |" % (l["name"], l["log"], l["horizon"], l["t4"], l["who"], "예" if l["moved"] else "아니오"))
        for hh in lm["hashes"]:
            L.append("- %s (%s와 짝): %s" % (hh["name"], hh["paired"], {True: "옮음", False: "그대로", None: "?"}[hh["moved"]]))
        L.append("- 제목의 구: %s" % (lm["phrase"] or "(읽지 못한 줄이 있어 정하지 않음: %s)" % lm["unknown"]))
    a1 = res.get("a1")
    if a1:
        L.append("")
        L.append("## §4 A′ (`%s`)" % a1["folder"])
        j = a1["J"]
        L.append("- J1 (Tick < T4* 같음): %s%s" % ("통과" if j["J1_ok"] else "실패", "" if j["J1_ok"] else " — " + "; ".join("%d: %s" % (r["seed"], r["why"]) for r in j["J1"] if not r["ok"])))
        L.append("- J2 (R 로그의 check 줄·에러 0 — 나머지 일곱 로그는 리드가 본다): %s" % ("통과" if j["J2_ok"] else "실패 — %s" % j["J2"]))
        L.append("- J3 (법의 배선): %s — 사회 1.0과 다스리는 T4가 선 판 %s, 법을 쥔 판 %s" % ("통과" if j["J3"]["ok"] else "실패", j["J3"]["eligible"], j["J3"]["held"]))
        L.append("- J4 (결박의 배선): %s — T4가 선 판 %s, 결박/위 건설이 시작된 판 %s" % ("통과" if j["J4"]["ok"] else "실패", j["J4"]["any_t4"], j["J4"]["wired"]))
        v1 = a1["voice"]
        L.append("- #136 A′: 둘 중 하나 %s (A %s); 기록 전 %s (A %s); 6,000 이하 %s (A %s)" % (
            v1["either"]["text"], v["either"]["text"], v1["by_record"]["text"], v["by_record"]["text"], v1["by_6000"]["text"], v["by_6000"]["text"]))
    ps = res.get("probes")
    if ps:
        L.append("")
        L.append("## §5 탐침의 시드 후보 (로그 %d개)" % ps["logs"])
        for key, label in (("probe1", "PROBE_SEED_1"), ("probe2", "PROBE_SEED_2")):
            c = ps[key]
            if c:
                L.append("- %s: seed %s, `%s`, 틱 %d → -Ticks=%d (%s)%s" % (label, c[0]["seed"], c[0]["log"], c[0]["tick"], c[0]["ticks_arg"], c[0]["why"],
                                                                         "; 다음 후보 %s" % ", ".join("%s@%d" % (x["seed"], x["tick"]) for x in c[1:4]) if len(c) > 1 else ""))
            else:
                L.append("- %s: 없음 — 'not probed — no seed'를 [B-6b8]의 묶음 B로" % label)
    return "\n".join(L) + "\n"


def parse_seeds(text):
    out = set()
    for part in text.split(","):
        if "-" in part:
            a, b = part.split("-")
            out.update(range(int(a), int(b) + 1))
        elif part:
            out.add(int(part))
    return out


def inside(path, root):
    try:
        return os.path.commonpath([os.path.abspath(path).lower(), os.path.abspath(root).lower()]) == os.path.abspath(root).lower()
    except ValueError:
        return False


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=os.environ.get("PROJECT_ROOT", DEFAULT_ROOT))
    ap.add_argument("--folder", default=None, help="bundle A folder (default <root>\\Saved\\Balance\\b6b_bundleA)")
    ap.add_argument("--seeds", default="1-30")
    ap.add_argument("--pbind", type=int, default=1800, help="P_BIND_A (start 1,800 - H:429, N:552)")
    ap.add_argument("--n", type=int, default=1200, help="N_A (start 1,200 - H:432)")
    ap.add_argument("--r", type=float, default=0.375, help="R_A (start 0.375 - H:43)")
    ap.add_argument("--civs-json", default=None, help="a DT_Civs.json copy (default: git show HEAD:Tools/data/DT_Civs.json)")
    ap.add_argument("--baseline-logs", action="append", default=None,
                    help="folders searched in order for checks*.log (default <root>\\Saved\\Balance\\b6b6_on then <root>\\Saved\\Logs)")
    ap.add_argument("--no-resample", action="store_true", help="skip the section 0.3 resample (needs the input-less on-runs too)")
    ap.add_argument("--a1", default=None, help="A' folder (section 4): J1-J4, #136 on A', probe seeds")
    ap.add_argument("--out", default=None, help="also write the report (and a .json beside it) to this file - never inside the project")
    a = ap.parse_args(argv)
    folder = os.path.abspath(a.folder or os.path.join(a.root, "Saved", "Balance", "b6b_bundleA"))
    seeds = parse_seeds(a.seeds)
    civs, source = load_civs(a.root, a.civs_json)
    factors = awakening_factors(civs)
    res = {"header": {"folder": folder, "seeds": a.seeds, "P": a.pbind, "N": a.n, "R_A": a.r, "civs_source": source, "factors": factors}}
    if not a.no_resample:
        res["resample"] = resample_check(folder, a.pbind, a.r, a.n)
    res["m_rows"], res["m_skipped"] = m_rows(folder, seeds, factors, a.pbind, a.n)
    res["m"] = decide_m(res["m_rows"], a.pbind, a.n)
    res["voice"] = voice_reading(folder, seeds)
    dirs = a.baseline_logs or [os.path.join(a.root, "Saved", "Balance", "b6b6_on"), os.path.join(a.root, "Saved", "Logs")]
    res["lines"] = lines_moved(dirs)
    if a.a1:
        a1 = os.path.abspath(a.a1)
        res["a1"] = {"folder": a1, "J": j_checks(folder, a1, seeds), "voice": voice_reading(a1, seeds)}
    res["probes"] = probe_seeds([find_log(dirs, n) or "" for n, _, _ in BASELINE_RUNS] + ([os.path.abspath(a.a1)] if a.a1 else []))
    md = report(res)
    print(md)
    if a.out:
        if inside(a.out, a.root):
            print("refusing --out inside the project: %s" % a.out)
            return 2
        with open(a.out, "w", encoding="utf-8") as f:
            f.write(md)
        with open(os.path.splitext(a.out)[0] + ".json", "w", encoding="utf-8") as f:
            json.dump(A.jsonable(res), f, ensure_ascii=False, indent=1)
    return 0


if __name__ == "__main__":
    sys.exit(main())
