"""[B-6b8] fix stage: self-test cases for the measurement review's fixes (M1-M6) in scratchpad/b6b8/bundleB_selftest.py."""
import os

path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "bundleB_selftest.py")
with open(path, "rb") as f:
    src = f.read().decode("utf-8")
assert "\r\n" not in src
EDITS = []


def edit(old, new):
    EDITS.append((old, new))


# mk(): the CSV's ruled erasures (review M2)
edit('''def mk(cell, seed, kind="None", rec=-1, civ="-", some=0, r3=-1, r4=-1, r4civ="-", r5=-1, r5civ="-", binding=(-1, "-", "-", 0), erased_ruled=0,
       decisions=(), locked=0, ok=True):''', '''def mk(cell, seed, kind="None", rec=-1, civ="-", some=0, r3=-1, r4=-1, r4civ="-", r5=-1, r5civ="-", binding=(-1, "-", "-", 0), erased_ruled=0,
       decisions=(), locked=0, ok=True, ruled_erased=()):''')
edit('''                    "decisions": {"pairs": list(decisions)}}}
    return job, p, ok''', '''                    "decisions": {"pairs": list(decisions)}}}
    p["ruled_erased"] = list(ruled_erased) if ruled_erased is not None else None
    return job, p, ok''')

# 6: the record-only dist (review M4)
edit('''    check(abs(B.sign_test(5, 0) - 0.0625) < 1e-12 and B.sign_test(0, 0) == 1.0 and abs(B.sign_test(3, 3) - 1.0) < 1e-12, "sign test")''',
     '''    check(abs(B.sign_test(5, 0) - 0.0625) < 1e-12 and B.sign_test(0, 0) == 1.0 and abs(B.sign_test(3, 3) - 1.0) < 1e-12, "sign test")
    check(B.dist([1, 2, 3, 4])["median"] == 2 and B.dist([5])["median"] == 5 and B.dist([None, float("inf")])["n"] == 0
          and B.dist([1, 2, 3, 4])["text"].startswith("n 4 median 2 "), "review M4: the record-only median is the lower middle")''')

# 8: (나) up to the cut (review M2)
edit('''    na = dict(binding=(8000, "AI", "civ_3", 3), erased_ruled=1)''',
     '''    na = dict(binding=(8000, "AI", "civ_3", 3), erased_ruled=1, ruled_erased=[("civ_7", 7900, 8000)])''')
edit('''             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1),                 # (나): erased by an extinction''',
     '''             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=[("civ_7", 7900, 8000)]),  # (나): erased by an extinction''')
edit('''             dict(binding=(-1, "-", "-", 0))]                                        # (다): no binding printed
    items = []
    for i, kw in enumerate(cases, 1):
        kw = dict(kw)
        r4 = kw.pop("r4", 7000)
        items.append(mk("R", i, "OtherPeople", 9000, "civ_3", 0, r4=r4, r4civ="civ_7", **kw))
    items += [mk("R", s, "RuledPeople", 9000, "civ_7") for s in range(10, 41)]
    j = B.judge_2(items)
    got = {k: [f["seed"] for f in v] for k, v in j["split"].items()}
    check(got == {"ga": [1, 2], "na": [3, 4, 5], "da": [6, 7, 8, 9]} and j["ga_binder_is_record_people"] == 1, "(가)/(나)/(다): %s" % got)''',
     '''             dict(binding=(-1, "-", "-", 0)),                                        # (다): no binding printed
             # review M2: a later life's ruled people erased after the binding - rung 0 stays (다), never folded into (나)
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=[("civ_7", 12000, 12100)]),
             # the erasure's sample straddles the cut (last 7,950 < 8,000 < 8,050): not placed for sure - (다), printed
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=[("civ_7", 7950, 8050)]),
             # no CSV (an archive's logs alone): the run-wide count decides, flagged
             dict(binding=(8000, "AI", "civ_3", 0), erased_ruled=1, ruled_erased=None),
             # no binding: the cut is the record (9,000) - erased at 8,500 is (나)
             dict(binding=(-1, "-", "-", 0), erased_ruled=1, ruled_erased=[("civ_7", 8400, 8500)])]
    items = []
    for i, kw in enumerate(cases, 1):
        kw = dict(kw)
        r4 = kw.pop("r4", 7000)
        items.append(mk("R", i, "OtherPeople", 9000, "civ_3", 0, r4=r4, r4civ="civ_7", **kw))
    items += [mk("R", s, "RuledPeople", 9000, "civ_7") for s in range(len(cases) + 1, 41)]
    j = B.judge_2(items)
    got = {k: [f["seed"] for f in v] for k, v in j["split"].items()}
    check(got == {"ga": [1, 2], "na": [3, 4, 5, 12, 13], "da": [6, 7, 8, 9, 10, 11]} and j["ga_binder_is_record_people"] == 1, "(가)/(나)/(다): %s" % got)
    fields = {f["seed"]: f for v in j["split"].values() for f in v}
    check(fields[10]["erased ruled before the cut"] is False and fields[11]["erased ruled near the cut"] == ["civ_7"]
          and fields[12].get("erased ruled timing unknown (no csv) - run-wide count") is True and fields[3]["erased ruled before the cut"] is True,
          "review M2: the erasure's time against the cut, the straddling sample printed, the missing CSV flagged")''')

# 9: (3)'s length by the law breaks (review M1, L4-2)
edit('''    j = B.judge_3(r3([1800] * 29))
    check(not j["judged"] and j["text"] == "재지 않음 — 잰 29", "n3 < 30 -> 재지 않음")''',
     '''    j = B.judge_3(r3([1800] * 29))
    check(not j["judged"] and j["text"] == "재지 않음 — 잰 29", "n3 < 30 -> 재지 않음")
    items = r3([1800] * 15 + [3000] * 15)
    plain = B.judge_3(r3([1800] * 15 + [3000] * 15))
    for i, (_, p, _) in enumerate(items):
        p["race_b"]["laws"] = {"resets_r": 0 if i < 15 else 2, "dropped": 1 if i == 0 else 0}
    j = B.judge_3(items)
    lb = j["length by breaks (run-wide)"]
    check(lb["0"] == {"n": 15, "median": 1800} and lb[">=1"] == {"n": 15, "median": 3000} and lb["unread"] == {"n": 0, "median": None}
          and j["ruled laws dropped at an input"] == 1 and j["empty law ticks"].startswith("재지 않음"), "review M1 (L4-2): the length by the run's law breaks %s" % lb)
    check(j["passed"] == plain["passed"] and j["length"] == plain["length"] and plain["length by breaks (run-wide)"]["unread"]["n"] == 30,
          "the split leaves (3)'s verdict as it was; a run without the laws part is 'unread'")''')

# 11: L4-3 workload, M3, M5, M6
edit('''    print("12. --cell-set A against bundleA.py's own verdict")''', '''    print("11b. record-only fixes of the review (M1 L4-3, M3, M5, M6)")

    def tw(seed, ai, r3, r4=-1, rec=-1, decisions=(), rows=None):
        job, p, ok = mk("R", seed, "RuledPeople" if rec >= 0 else "None", rec, r3=r3, r4=r4, decisions=decisions)
        p["race_b"]["laws"] = {"ai": ai}
        if rows is not None:
            job["rows"] = rows
        return job, p, ok
    rows_path = os.path.join(root, "tw_rows.csv")
    write_rows(rows_path, F, ((3000, "petition_plague", 9), (3100, "petition_plague", 2), (5400, "enc_witness", 1), (5500, "petition_plague", 5),
                              (4000, "event_era_endured", 7), (4100, "project_relief", 3)))
    w = B.third_rung_workload([tw(1, 3000, 2000, r4=5400, decisions=every(3000, 5400), rows=rows_path),   # 4 whole windows of 2
                               tw(2, -1, 2000),                                                          # no AI hold: 재지 않음
                               tw(3, 3000, 3500),                                                        # ruled T3 after the hold: 재지 않음
                               tw(4, 3000, 2000, r4=2900),                                               # e <= s: 0 windows
                               tw(5, 3000, 2000, rec=7000, decisions=every(3000, 7000))])               # e = 6,000 (before the record)
    key = lambda prefix: next(k for k in w if k.startswith(prefix))  # noqa: E731
    check(w["runs"] == 3 and w[key("재지 않음")] == [2, 3] and w["zero windows"] == [4] and w[key("decisions per 600")] == 2,
          "review M1 (L4-3): the window from the first AI law hold, unmeasured and 0-window runs: %s" % w)
    check(w[key("cards per 600")] == {"enc_witness": 0.25, "petition_plague": 0.5, "project_relief": 0.75} and w["window ticks"] == 2400
          and w["runs without _rows.csv"] == [5], "cards: petition_/enc_ and A′'s two named rows in (s, e] per 600 (not the era's line): %s" % w)
    xs = [mk("R", s, binding=(8000, "ruled", "civ_7", 4))[0:2] for s in (1, 2, 3)]
    for (_, p), v in zip(xs, (-1, 0, 1)):
        p["race_b"]["binding"]["vessel_left"] = v
    xs[0][1]["race"]["t4civs"] = {"at_record": 2, "at_end": 3, "most": 3}
    xs[1][1]["race"]["t4civs"] = {"at_record": -1, "at_end": 1, "most": 1}
    ce = B.cell_extras(xs)
    check(ce["binding"]["vessel left before T5"] == "1/2", "review M3: left before T5 over the runs the ruled people bound (-1 is a marker): %s" % ce["binding"])
    check(ce["T4 civs at the record"].startswith("n 1 median 2 ") and ce["T4 civs no record"] == 1, "review M6: T4 civs at the record, the runs without one counted")
    mf = os.path.join(root, "m5")
    jr = next(j for j in B.build_jobs(mf, 4400, cells={"R"}, max_seed=1) if not j["off"])
    write_job(F, jr, dict(seed=1), extra=[T.ue("CosmicBalance calling: lives 2; reign ended first at 1500, last at 1500; first throne after the first end at -1")],
              tier7=TIER_1)
    pr = {jr["name"]: B.parse_log(jr["log"])}
    rr = {jr["name"]: {"ok": True}}
    rec = B.record_only([jr], pr, rr)
    left = rec["vessels left at T4+"]
    check(pr[jr["name"]]["reign_ended"] == 1500 and pr[jr["name"]]["left_at"] is None and len(left) == 1 and left[0]["cell"] == "R"
          and left[0]["left"] == 1500 and left[0]["tier"] == 4 and rec["R reigns ended"] == 1, "review M5: an R log's reign end reads on the #96 line: %s" % left)
    saved = B.left_tick
    B.left_tick = lambda p: p.get("left_at")
    try:
        check(B.record_only([jr], pr, rr)["vessels left at T4+"] == [], "(the fallback broken once: the R entry is gone - the case above would fail)")
    finally:
        B.left_tick = saved

    print("12. --cell-set A against bundleA.py's own verdict")''')

for old, new in EDITS:
    n = src.count(old)
    assert n == 1, (n, old[:120])
    src = src.replace(old, new)
with open(path, "wb") as f:
    f.write(src.encode("utf-8"))
print("applied", len(EDITS), "edits")
