"""[B-6b8] fix stage: the measurement review's fixes M1-M7 on scratchpad/b6b8/bundleB.py (record and analysis only - no value,
no judgment's pass/fail arithmetic changes). Exact replacements, each found once; LF kept."""
import os
import sys

path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "bundleB.py")
with open(path, "rb") as f:
    src = f.read().decode("utf-8")
assert "\r\n" not in src

EDITS = []


def edit(old, new):
    EDITS.append((old, new))


# ---- docstring: the review fixes
edit('''slowR (-Focus=0) are existing switches.
"""''', '''slowR (-Focus=0) are existing switches.

Review fixes ([B-6b8] fix stage, record and analysis only): M1 (L4-2: (3)'s length split by the run's law breaks - the
breaks counted over the whole run; L4-3: the ruled third-rung workload under AI-held laws, R and slowR), M2 ((2)'s (나)
counts an extinction of the ruled people only up to the binding, or the record without one - read from the CSV), M3 ('vessel
left before T5' as left/ruled-bound, not a sum of -1 markers), M4 (every record-only median is the lower middle - dist()),
M5 (the #96 line reads the calling line's 'reign ended first' where a log has no wanderer line), M6 (T4 civs at the record),
M7 (the record-only heading says what the numbers are: whole runs, the shared front counted again in the wanderer and death
cells' on side - a departure from brief 4.3 'the same world counted once', recorded in N section 5).
"""''')

edit("import shutil\nimport subprocess\n", "import shutil\nimport statistics\nimport subprocess\n")

# ---- M5: the calling line's first reign end
edit('''RE_DEATH = re.compile(r"CosmicBalance calling deaths: death asked at (?P<at>-?\\d+) \\(recorded (?P<recorded>-?\\d+)\\)")
''', '''RE_DEATH = re.compile(r"CosmicBalance calling deaths: death asked at (?P<at>-?\\d+) \\(recorded (?P<recorded>-?\\d+)\\)")
RE_REIGN = re.compile(r"CosmicBalance calling: lives \\d+; reign ended first at (?P<first>-?\\d+)")
''')
edit('''    m = A.last_match(RE_WANDERER.pattern, text)
    p["left_at"] = int(m.group("left")) if m else None
''', '''    m = A.last_match(RE_WANDERER.pattern, text)
    p["left_at"] = int(m.group("left")) if m else None
    m = A.last_match(RE_REIGN.pattern, text)
    p["reign_ended"] = int(m.group("first")) if m else None
''')

# ---- M2: the ruled people's extinction erasures from the CSV, attached where the logs are parsed
edit('''def evaluate(jobs, results, ignore_errors):
    parsed = {j["name"]: parse_log(j["log"]) for j in jobs}
''', '''def ruled_erasures(csv_path):
    """Review M2: [(civ, last sample tick, erased tick)] of the civs erased from the CSV that were the ruled people (empty
    Expansion) at their last sample - the commandlet's bRuled = (Owner == RuledCivBeforeTick) at the expiry; a civ ruled only
    in an earlier life does not count. The erased tick is the first sample after the last row (100-tick samples)."""
    data = A.civ_track(csv_path)
    out = []
    for c in sorted(data["track"]):
        e = A.erased_tick(data, c)
        if e is None:
            continue
        last = data["track"][c][-1][0]
        if data["ruled"].get(last) == c:
            out.append((c, last, e))
    return out


def attach_csv_facts(jobs, parsed):
    """The CSV facts the judgments read beside the log (review M2): ruled_erased, only where the race line counts an erased
    ruled people (None when the CSV is missing - an archive holds the logs alone)."""
    for j in jobs:
        p = parsed.get(j["name"]) or {}
        ext = (p.get("race") or {}).get("extinction") or {}
        if ext.get("erased_ruled", 0) > 0:
            p["ruled_erased"] = ruled_erasures(j["csv"]) if j.get("csv") and os.path.exists(j["csv"]) else None


def evaluate(jobs, results, ignore_errors):
    parsed = {j["name"]: parse_log(j["log"]) for j in jobs}
    attach_csv_facts(jobs, parsed)
''')

# ---- M4: the lower median in every record-only distribution
edit('''def window_median(decisions, start, end):''', '''def dist(values):
    """Review M4: bundleA.dist's fields and text, the median the lower middle of an even count (the frozen arithmetic of brief
    4.3 - not statistics.median); the quartiles stay as bundle A's (inclusive)."""
    vals = sorted(v for v in values if v is not None and not (isinstance(v, float) and math.isinf(v)))
    if not vals:
        return {"n": 0, "median": None, "q1": None, "q3": None, "min": None, "max": None, "text": "n 0"}
    if len(vals) >= 2:
        q1, _, q3 = statistics.quantiles(vals, n=4, method="inclusive")
    else:
        q1 = q3 = vals[0]
    med = lower_median(vals)
    fmt = A.fmt
    return {"n": len(vals), "median": med, "q1": q1, "q3": q3, "min": vals[0], "max": vals[-1],
            "text": "n %d median %s [q1 %s, q3 %s] (min %s, max %s)" % (len(vals), fmt(med), fmt(q1), fmt(q3), fmt(vals[0]), fmt(vals[-1]))}


def window_median(decisions, start, end):''')

# ---- M2: split_of reads the erasure's time
edit('''def split_of(p):
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
        return "na", fields''', '''def split_of(p):
    """(2)'s split of one numerator run: (가) the binder is not the ruled people and ruled rung >= 4; (나) the ruled people
    erased by an extinction up to the binding (or up to the record when there is no binding - review M2: an erasure after it
    is a later life's, not the reason the race was lost), or its T4 -1 or after the binding; (다) every other (ruled rung 0 -
    interregnum or a dead vessel - lands here unless an extinction erased the ruled people before the cut). The erasure's
    time is the CSV's (p['ruled_erased'], 100-tick samples): a civ whose last sample is before the cut and whose erased
    sample is after it cannot be placed for sure and is printed ('erased ruled near the cut'), not folded into (나). Without
    the CSV (p['ruled_erased'] None or absent) the run-wide count decides, flagged."""
    b = (p.get("race_b") or {}).get("binding") or {}
    race = p.get("race") or {}
    ru, ext = race.get("ruled") or {}, race.get("extinction") or {}
    cut = b["tick"] if b and b.get("tick", -1) >= 0 else (p.get("ending_x") or {}).get("tick", -1)
    erased = p.get("ruled_erased")
    if ext.get("erased_ruled", 0) > 0 and erased is None:
        erased_before, unknown = True, True
    else:
        erased_before = ext.get("erased_ruled", 0) > 0 and cut >= 0 and any(e <= cut for _, _, e in (erased or []))
        unknown = False
    near = [c for c, last, e in (erased or []) if last < cut < e]
    fields = {"binding": b.get("tick"), "by": b.get("by"), "binder": b.get("civ"), "rung": b.get("rung"), "ruled T4": ru.get("r4"),
              "erased ruled": ext.get("erased_ruled"), "erased ruled before the cut": erased_before, "erased ruled near the cut": near,
              "record people": (p.get("ending_x") or {}).get("civ")}
    if unknown:
        fields["erased ruled timing unknown (no csv) - run-wide count"] = True
    if b and b.get("tick", -1) >= 0 and b.get("by") != "ruled" and b.get("rung", 0) >= 4:
        return "ga", fields
    if erased_before:
        return "na", fields''')

# ---- M1 (L4-2): (3)'s length by the run's law breaks
edit('''        decisions = ((p.get("race_b") or {}).get("decisions") or {}).get("pairs", [])
        med, windows = window_median(decisions, ru["r4"], ru["r5"])
        n3_runs.append({"seed": j["seed"], "length": ru["r5"] - ru["r4"], "median": med, "windows": windows})''',
     '''        decisions = ((p.get("race_b") or {}).get("decisions") or {}).get("pairs", [])
        med, windows = window_median(decisions, ru["r4"], ru["r5"])
        laws = (p.get("race_b") or {}).get("laws") or {}
        n3_runs.append({"seed": j["seed"], "length": ru["r5"] - ru["r4"], "median": med, "windows": windows,
                        "breaks": laws.get("resets_r", -1), "dropped": laws.get("dropped", -1)})''')
edit('''    out = {"n3": n3, "runs": n3_runs, "apart": apart, "length": length, "dense": dense, "zero_windows": [r["seed"] for r in n3_runs if r["windows"] == 0],
           "violations": [r for r in n3_runs if r["length"] < 1200], "density": wilson_share(dense, n3)}''',
     '''    out = {"n3": n3, "runs": n3_runs, "apart": apart, "length": length, "dense": dense, "zero_windows": [r["seed"] for r in n3_runs if r["windows"] == 0],
           "violations": [r for r in n3_runs if r["length"] < 1200], "density": wilson_share(dense, n3)}
    # [B-6b7]'s hand-off (L4-2): the season read as measured (length = r5 - r4 above) and split by the ruled people's law
    # breaks - the race line's 'resets ruled', counted over the whole run (Race.Resets[0]: after the fifth rung too), not
    # inside the fourth->fifth season. The ticks the laws stood empty are on no output (no line or CSV column holds the laws).
    by_breaks = {}
    for label, keep in (("0", lambda r: r["breaks"] == 0), (">=1", lambda r: r["breaks"] > 0), ("unread", lambda r: r["breaks"] < 0)):
        lens = [r["length"] for r in n3_runs if keep(r)]
        by_breaks[label] = {"n": len(lens), "median": lower_median(lens)}
    out["length by breaks (run-wide)"] = by_breaks
    out["ruled laws dropped at an input"] = sum(r["dropped"] for r in n3_runs if r["dropped"] > 0)
    out["empty law ticks"] = "재지 않음 — race 줄에도 CSV에도 법을 쥔 열이 없다"''')

# ---- M3, M4, M6: cell_extras
edit('''        "binding": {"bound": wilson_share(sum(1 for x in b if x["tick"] >= 0), n)["text"], "tick": A.dist(pos([x["tick"] for x in b]))["text"],''',
     '''        "binding": {"bound": wilson_share(sum(1 for x in b if x["tick"] >= 0), n)["text"], "tick": dist(pos([x["tick"] for x in b]))["text"],''')
edit('''                    "vessel left before T5": sum(x["vessel_left"] for x in b)},''',
     '''                    "vessel left before T5": "%d/%d" % (sum(1 for x in b if x["vessel_left"] > 0), sum(1 for x in b if x["vessel_left"] >= 0))},''')
edit('''                     "by the record": sum(s["era_rec"] for s in side), "first": A.dist(pos([s["era_first"] for s in side]))["text"]},''',
     '''                     "by the record": sum(s["era_rec"] for s in side), "first": dist(pos([s["era_first"] for s in side]))["text"]},''')
edit('''                         "by the record": sum(s["wit_rec"] for s in side), "first": A.dist(pos([s.get("wit_first") for s in side]))["text"],''',
     '''                         "by the record": sum(s["wit_rec"] for s in side), "first": dist(pos([s.get("wit_first") for s in side]))["text"],''')
edit('''        "laws": {"first held ruled": A.dist(pos([x["ruled"] for x in laws]))["text"], "first held AI": A.dist(pos([x["ai"] for x in laws]))["text"],''',
     '''        "laws": {"first held ruled": dist(pos([x["ruled"] for x in laws]))["text"], "first held AI": dist(pos([x["ai"] for x in laws]))["text"],''')
edit('''        "extinction": ARK_UNMEASURED,
    }
    return out''', '''        "extinction": ARK_UNMEASURED,
    }
    t4 = [x for x in ((p.get("race") or {}).get("t4civs") for p in ps) if x]
    out["T4 civs at the record"] = dist([x["at_record"] for x in t4 if x["at_record"] >= 0])["text"]
    out["T4 civs no record"] = sum(1 for x in t4 if x["at_record"] < 0)
    return out''')

# ---- M4, M6: per_pair
edit('''    keys = {"T4 civs most": lambda p: (p.get("race") or {}).get("t4civs", {}).get("most"),''',
     '''    keys = {"T4 civs most": lambda p: (p.get("race") or {}).get("t4civs", {}).get("most"),
            "T4 civs at the record": lambda p: (lambda x: x["at_record"] if x and x["at_record"] >= 0 else None)((p.get("race") or {}).get("t4civs")),''')
edit('''        out[label] = {"on": A.dist(on)["text"], "off": A.dist(off)["text"], "on-off": A.dist(diff)["text"]}''',
     '''        out[label] = {"on": dist(on)["text"], "off": dist(off)["text"], "on-off": dist(diff)["text"]}''')

# ---- M5: the #96 line's leaving tick
edit('''def vessel_left_tier(job, p):
    """The CSV Tier of the people the vessel ruled when it left (the ruled row - empty Expansion - of the last sample at or
    before 'left at'), at the first sample at or after it."""
    left = p.get("left_at")''', '''def left_tick(p):
    """Review M5: the tick the vessel left its people - the wanderer line's 'left at', or (a log without a wanderer line: R,
    slowR, death) the calling line's 'reign ended first at' (the same event in the wanderer runs: 1201 in both)."""
    return p["left_at"] if p.get("left_at") is not None else p.get("reign_ended")


def vessel_left_tier(job, p):
    """The CSV Tier of the people the vessel ruled when it left (the ruled row - empty Expansion - of the last sample at or
    before left_tick), at the first sample at or after it."""
    left = left_tick(p)''')

# ---- M1 (L4-3): the ruled third-rung workload under AI-held laws
edit('''def record_only(jobs, parsed, results, serial_ai_us=None):
    out = {"cells": {}, "pairs": {}}''', '''CARD_ROWS_NAMED = ("event_plague_repentance", "project_relief")   # A′ read them as cards before the player (N section 3)


def is_card_row(name):
    return name.startswith(("petition_", "enc_")) or name in CARD_ROWS_NAMED


def read_rows(path):
    """_rows.csv: (Tick, SourceRow, Lines) - lines told per 100-tick sample, labelled by the sample's last tick."""
    out = []
    with open(path, encoding="utf-8-sig") as f:
        head = f.readline().strip().split(",")
        ix = {h: i for i, h in enumerate(head)}
        for line in f:
            c = line.strip().split(",")
            if len(c) < len(head):
                continue
            out.append((int(c[ix["Tick"]]), c[ix["SourceRow"]], int(c[ix["Lines"]])))
    return out


def third_rung_workload(items):
    """[B-6b7]'s hand-off (L4-3), record only: the window from the first AI law hold (race 'laws ... AI at') to the earliest of
    the ruled T4, the record and 6,000, in runs whose ruled people stood at the third rung by then (ruled T3 <= the hold);
    otherwise '재지 않음' for the run. Decisions: whole 600-tick windows (window_median), per-run lower median. Cards: lines
    of the rows that wait before the player (petition_, enc_, and A′'s event_plague_repentance and project_relief) from
    _rows.csv samples labelled in (s, e] - 100-tick samples, so the window's ends are off by up to 100 ticks - pooled per 600
    ticks over the window ticks of the runs with a _rows.csv."""
    runs, unmeasured, no_rows = [], [], []
    lines, window_ticks = {}, 0
    for j, p, ok in items:
        if not ok:
            continue
        laws = (p.get("race_b") or {}).get("laws") or {}
        ru = (p.get("race") or {}).get("ruled") or {}
        s, r3 = laws.get("ai", -1), ru.get("r3", -1)
        if s is None or r3 is None or s < 0 or r3 < 0 or r3 > s:
            unmeasured.append(j["seed"])
            continue
        e = min(t for t in (ru.get("r4", -1), (p.get("ending_x") or {}).get("tick", -1), 6000) if t is not None and t >= 0)
        decisions = ((p.get("race_b") or {}).get("decisions") or {}).get("pairs", [])
        med, windows = window_median(decisions, s, e)
        runs.append({"seed": j["seed"], "s": s, "e": e, "median": med, "windows": windows})
        rows_path = j.get("rows")
        if e > s and rows_path and os.path.exists(rows_path):
            window_ticks += e - s
            for tick, row, n in read_rows(rows_path):
                if s < tick <= e and is_card_row(row):
                    lines[row] = lines.get(row, 0) + n
        elif e > s:
            no_rows.append(j["seed"])
    meds = [r["median"] for r in runs if r["windows"] > 0]
    return {"runs": len(runs), "decisions per 600 (per-run lower median, then the lower median)": lower_median(meds),
            "zero windows": [r["seed"] for r in runs if r["windows"] == 0], "재지 않음 (no AI law hold, or ruled T3 after it)": unmeasured,
            "cards per 600 (pooled, 100-tick samples)": {k: round(v * 600.0 / window_ticks, 3) for k, v in sorted(lines.items())} if window_ticks else "재지 않음(100틱 묶음 없음)",
            "card lines": sum(lines.values()), "window ticks": window_ticks, "runs without _rows.csv": no_rows}


def record_only(jobs, parsed, results, serial_ai_us=None):
    out = {"cells": {}, "pairs": {}}''')
edit('''            sides[side] = {"a": A.side_summary(items), "b": cell_extras(items)}''',
     '''            saved = A.dist
            A.dist = dist   # review M4: bundle A's side summary resolves dist from its module globals at call time
            try:
                summary = A.side_summary(items)
            finally:
                A.dist = saved
            sides[side] = {"a": summary, "b": cell_extras(items)}''')
edit('''                if v and v["tier"] is not None and v["tier"] >= 4:
                    left.append(dict(v, seed=j["seed"], cell=cell, left=p["left_at"]))
    out["vessels left at T4+"] = left''', '''                if v and v["tier"] is not None and v["tier"] >= 4:
                    left.append(dict(v, seed=j["seed"], cell=cell, left=left_tick(p)))
    out["vessels left at T4+"] = left
    # Review M5: an empty R side says why - the R runs whose vessel left its people (a reign that ended) at all.
    out["R reigns ended"] = sum(1 for j, p, ok in on_items(jobs, parsed, results, "R") if ok and p.get("reign_ended") is not None and p["reign_ended"] >= 0)
    # [B-6b7]'s hand-off (L4-3): the ruled third-rung workload under AI-held home-galaxy laws, R and slowR on.
    out["third-rung workload"] = {cell: third_rung_workload(on_items(jobs, parsed, results, cell)) for cell in ("R", "slowR")}''')
edit('''    out["AI us"] = {"bundle on (parallel)": A.dist(us)["text"],''', '''    out["AI us"] = {"bundle on (parallel)": dist(us)["text"],''')

# ---- markdown
edit('''    if j3["apart"]:
        L.append("  - 따로 적은 판: %s" % "; ".join("%d (%s)" % (a["seed"], a["why"]) for a in j3["apart"]))''',
     '''    if j3["apart"]:
        L.append("  - 따로 적은 판: %s" % "; ".join("%d (%s)" % (a["seed"], a["why"]) for a in j3["apart"]))
    if j3.get("length by breaks (run-wide)"):
        L.append("  - 잰 계절의 길이를 법의 끊김으로 나눔(끊김 수는 판 전체 — 다섯째 칸 뒤도 셈): %s; 입력에 떨군 다스리는 법 %s; 법이 빈 틱: %s" % (
            j3["length by breaks (run-wide)"], j3.get("ruled laws dropped at an input"), j3.get("empty law ticks")))''')
edit('''    L.append("## 적기만 (on / off / on − off; 같은 세계는 칸마다 따로 — 칸을 가로지르는 합은 만들지 않는다)")''',
     '''    L.append("## 적기만 (on / off / on − off; 칸의 수는 판 전체 — 방랑자 칸(1,200 아래)·강제 죽음 칸(T_DEATH 아래)의 켬 쪽은 R과 같은 세계의 "
             "앞부분을 다시 센다; 끔 쪽은 B6b 행이 빠져 절멸·방주·결박·법·시대·증인이 0이라 겹침이 없다 — 브리프 §4.3의 '같은 세계는 한 번만'과 다름, N §5)")''')
edit('''            L.append("- %s: 판 %d; T4 civs most %s; brake held %s; 기록 %s" % (side, a["runs"], a["T4 civs most"]["text"], a["brake held AI-ticks"]["text"], b["record kinds"]))''',
     '''            L.append("- %s: 판 %d; T4 civs at the record %s (no record %d); at end %s; most %s; brake held %s; 기록 %s" % (
                side, a["runs"], b["T4 civs at the record"], b["T4 civs no record"], a["T4 civs at end"]["text"], a["T4 civs most"]["text"],
                a["brake held AI-ticks"]["text"], b["record kinds"]))''')
edit('''    L.append("- 넷째 칸 이상에 백성을 두고 떠난 그릇(#96): %s" % rec["vessels left at T4+"])''',
     '''    L.append("- 넷째 칸 이상에 백성을 두고 떠난 그릇(#96): %s%s" % (rec["vessels left at T4+"],
                                                     "; R: 재지 않음(폐위·죽음 0)" if not rec.get("R reigns ended") else "; R의 끝난 재위 %d" % rec["R reigns ended"]))
    for cell, w in (rec.get("third-rung workload") or {}).items():
        L.append("- 다스리는 셋째 칸의 일감(AI가 고향 은하 법을 쥔 틱부터 다스리는 T4·기록·6,000 가운데 이른 틱까지; [B-6b7]의 넘김, 적기만) %s: %s" % (cell, w))''')

for old, new in EDITS:
    n = src.count(old)
    assert n == 1, (n, old[:120])
    src = src.replace(old, new)
with open(path, "wb") as f:
    f.write(src.encode("utf-8"))
print("applied", len(EDITS), "edits to", path)
