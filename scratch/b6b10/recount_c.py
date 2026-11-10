"""[B-6b10] L4 independent recount of the clock C from the bundle B slow-R logs (written fresh, not copied from bundleB.py).

Rules (brief_final.md section 4, L4 1-6):
 1. E = record tick if record kind in {RuledPeople, OtherPeople} and 0 <= tick <= 20000, else inf.
 2. 48th value ascending; C0 = ceil(E48/1000)*1000 (inf if inf); C = 18000 if C0 inf else min(18000, max(6000, C0)); (i) = #{E <= C}.
 3. end = min(record tick of any kind, C), or C when no record.
 4. excluded from (ii) when r3 < 0 or r3 >= end.
 5. whole half-open 600 windows [r3+600k, r3+600(k+1)) with r3+600(k+1) <= end; window sum of decision counts.
 6. per-run value = lower median of window sums; r3 < end with 0 whole windows fails; (ii) passes when dense*10 >= 9*judged; judged < 30 = not measured.
Usage: python -I recount_c.py <folder with slowR_<seed>.log>
"""
import math
import os
import re
import sys

FOLDER = sys.argv[1]
INF = math.inf
RE_RECORD = re.compile(r"CosmicBalance ending: record (\S+) at (-?\d+)")
RE_RACE = re.compile(r"CosmicBalance race: .*")
RE_R3 = re.compile(r"ruled T3 at (-?\d+)")
RE_DEC = re.compile(r"decisions \[([^\]]*)\]")


def read(seed):
    path = os.path.join(FOLDER, "slowR_%d.log" % seed)
    text = open(path, encoding="utf-8", errors="replace").read()
    recs = RE_RECORD.findall(text)
    races = RE_RACE.findall(text)
    if len(recs) != 1 or len(races) != 1:
        raise SystemExit("seed %d: %d ending lines, %d race lines" % (seed, len(recs), len(races)))
    kind, tick = recs[0][0], int(recs[0][1])
    race = races[0]
    r3 = int(RE_R3.search(race).group(1))
    decs = []
    for tok in RE_DEC.search(race).group(1).split():
        t, n = tok.split(":")
        decs.append((int(t), int(n)))
    return kind, tick, r3, decs


def lower_median(v):
    v = sorted(v)
    return v[(len(v) - 1) // 2]


runs = {s: read(s) for s in range(1, 61)}
E = {}
for s, (kind, tick, r3, decs) in runs.items():
    E[s] = tick if kind in ("RuledPeople", "OtherPeople") and 0 <= tick <= 20000 else INF
asc = sorted(E.values())
E48 = asc[47]
C0 = INF if E48 == INF else int(math.ceil(E48 / 1000.0)) * 1000
C = 18000 if C0 == INF else min(18000, max(6000, C0))
I = sum(1 for e in E.values() if e <= C)

judged, dense, excluded, zero = 0, 0, [], []
for s, (kind, tick, r3, decs) in runs.items():
    has_record = kind not in ("None", "none", "-") and tick >= 0
    end = min(tick, C) if has_record else C
    if r3 < 0 or r3 >= end:
        excluded.append(s)
        continue
    judged += 1
    k = (end - r3) // 600
    if k <= 0:
        zero.append(s)
        continue
    sums = [0] * k
    for t, n in decs:
        if r3 <= t < r3 + 600 * k:
            sums[(t - r3) // 600] += n
    if lower_median(sums) >= 1:
        dense += 1
ii_pass = judged >= 30 and dense * 10 >= 9 * judged
kinds = {}
for s, (kind, tick, r3, decs) in runs.items():
    kinds[kind] = kinds.get(kind, 0) + 1
print("kinds", kinds)
print("E", " ".join("%d:%s" % (s, E[s]) for s in range(1, 61)))
print("E ascending", " ".join(str(e) for e in asc))
print("E48 %s C0 %s C %s" % (E48, C0, C))
print("(i) %d/60 %s" % (I, "pass" if I >= 30 else "FAIL"))
print("(ii) %d/%d excluded %d %s zero-window %s -> %s" % (dense, judged, len(excluded), excluded, zero, "pass" if ii_pass else ("NOT MEASURED" if judged < 30 else "FAIL")))
lo_med = lower_median([e for e in E.values() if e != INF])
print("E lower median %s min %s max %s finite %d" % (lo_med, asc[0], max(e for e in E.values() if e != INF), sum(1 for e in E.values() if e != INF)))
