import os, re, csv, math, statistics, sys
from math import comb
F = r"C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b11\bundleC"
C = 9000


def line(name, tag):
    with open(os.path.join(F, name), encoding='utf-8', errors='replace') as f:
        for l in f:
            i = l.find('CosmicBalance ' + tag + ':')
            if i >= 0:
                return l[i:]
    return None


def wilson(k, n, z=1.959963984540054):
    if n == 0:
        return (0, 0)
    p = k / n
    d = 1 + z * z / n
    c = p + z * z / (2 * n)
    h = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n))
    return ((c - h) / d, (c + h) / d)


def wt(k, n):
    lo, hi = wilson(k, n)
    return "%d/%d = %.3f [%.3f, %.3f]" % (k, n, k / n, lo, hi)


S = range(1, 61)


def ending(name):
    e = line(name, 'ending')
    m = re.search(r'ending: record (\w+) at (-?\d+) \(people ([^,]*), a people some life ruled (\d+), no vessel (\d+)\)', e)
    return m.group(1), int(m.group(2)), int(m.group(5)), e


# R1
for cell in ('R', 'slowR'):
    k = 0
    for s in S:
        e = ending('%s_%d.log' % (cell, s))[3]
        m = re.search(r'by the record told: first-person claim lines (\d+)', e)
        k += int(m.group(1)) >= 1
    print('R1', cell, wt(k, 60))
# R7
for cell in ('inputless', 'wanderer', 'death'):
    k = 0
    for s in S:
        r = ending('%s_%d.log' % (cell, s))
        k += (r[0] == 'OtherPeople' and r[2] == 1)
    print('R7', cell, wt(k, 60))
# R8 + R11
for cell in ('R', 'slowR'):
    kinds = {}
    ticks = []
    for s in S:
        kd, t, nv, _ = ending('%s_%d.log' % (cell, s))
        kinds[kd] = kinds.get(kd, 0) + 1
        ticks.append(t)
        assert 0 <= t <= C and (kd != 'Clock' or t == C)
    print('R8', cell, {k: wt(v, 60) for k, v in sorted(kinds.items())})
    ts = sorted(ticks)
    n = len(ts)
    nearest = ts[math.ceil(0.9 * n) - 1]
    pos = (n - 1) * 0.9
    lin = ts[int(pos)] + (pos - int(pos)) * (ts[int(pos) + 1] - ts[int(pos)])
    excl = statistics.quantiles(ts, n=10)[8]
    print('R11', cell, 'min', ts[0], 'median', statistics.median(ts), 'p90 nearest', nearest, 'linear', lin, 'exclusive', excl, 'clock', wt(kinds.get('Clock', 0), 60))


# R4
def wand(name):
    w = line(name, 'wanderer')
    d = line(name, 'calling deaths')
    left = int(re.search(r'left at (-?\d+)', w).group(1))
    cr = int(re.search(r'crowned at (-?\d+)', w).group(1))
    path = re.search(r'path (\w+)', w)
    path = path.group(1) if path else None
    fd = int(re.search(r'first death at (-?\d+)', d).group(1))
    own = 0 <= cr <= 6000 and (fd == -1 or cr < fd)
    anyv = 0 <= cr <= 6000
    return own, anyv, left, cr, fd, path


on = {s: wand('wanderer_%d.log' % s) for s in S}
off = {s: wand('wanderer_%d_off.log' % s) for s in S}
kon = sum(v[0] for v in on.values())
koff = sum(v[0] for v in off.values())
L = [s for s in S if not on[s][0] and off[s][0]]
G = [s for s in S if on[s][0] and not off[s][0]]
n = len(L) + len(G)
m = min(len(L), len(G))
p = min(1.0, 2 * sum(comb(n, i) for i in range(m + 1)) / 2 ** n)
print('R4 own on', wt(kon, 60), 'off', wt(koff, 60), 'any on', wt(sum(v[1] for v in on.values()), 60), 'any off', wt(sum(v[1] for v in off.values()), 60))
print('R4 L', len(L), L, 'G', len(G), G, 'p', round(p, 4), repr(p))
for side, dd in (('on', on), ('off', off)):
    md = statistics.median([v[3] - v[2] for v in dd.values() if v[0]])
    paths = {}
    for v in dd.values():
        if v[0]:
            paths[v[5]] = paths.get(v[5], 0) + 1
    print('R4', side, 'median after leaving', md, 'paths', paths)


# R5
def taken(name):
    c = line(name, 'conflict')
    m = re.search(r'taken (\d+) \((\d+)/(\d+)/(\d+) by tick bands', c)
    top = int(re.search(r'taken by top conqueror (\d+)', c).group(1))
    return int(m.group(1)), int(m.group(4)), top


def q(xs):
    xs = sorted(xs)
    qs = statistics.quantiles(xs, n=4, method='inclusive')
    return "median %s q1 %.3g q3 %.3g min %d max %d" % (statistics.median(xs), qs[0], qs[2], xs[0], xs[-1])


on5 = {s: taken('inputless_%d.log' % s) for s in S}
off5 = {s: taken('inputless_%d_off.log' % s) for s in S}
r75 = {s: taken('inputless075_%d.log' % s) for s in S}
print('R5 taken 0.79 on', q([v[0] for v in on5.values()]))
print('R5 taken 0.79 off', q([v[0] for v in off5.values()]))
print('R5 on-off', q([on5[s][0] - off5[s][0] for s in S]))
print('R5 taken 0.75 on', q([v[0] for v in r75.values()]))
print('R5 late on', q([v[1] for v in on5.values()]), '| off', q([v[1] for v in off5.values()]), '| on-off', q([on5[s][1] - off5[s][1] for s in S]), '| 0.75', q([v[1] for v in r75.values()]))
for nm, dd in (('0.79on', on5), ('0.75', r75)):
    sh = [v[2] / v[0] for v in dd.values() if v[0] > 0]
    print('R5 top share', nm, 'n', len(sh), 'median', round(statistics.median(sh), 3), 'runs>50%', sum(x > 0.5 for x in sh))
print('R5 totals on', sum(v[0] for v in on5.values()), 'off', sum(v[0] for v in off5.values()), '0.75', sum(v[0] for v in r75.values()), 'late on', sum(v[1] for v in on5.values()), 'late off', sum(v[1] for v in off5.values()), 'late 0.75', sum(v[1] for v in r75.values()))

# R2
keep = []
eased = joined = ab = oc = 0
sump = 0.0
for s in range(1, 141):
    with open(os.path.join(F, 'test_%d_claims.csv' % s), encoding='utf-8') as f:
        for r in csv.DictReader(f):
            base = (int(r['CardTick']) >= 0 and 401 <= int(r['RemainingAtCard']) <= 550 and r['VigilKept'] == '1' and r['Relief'] == '0'
                    and r['EndKind'] in ('joined', 'eased', 'other', 'expired') and int(r['PlacedTick']) < C)
            if not base:
                continue
            if r['Ruled'] == '1' and r['OwnerChanged'] == '1':
                oc += 1
            if r['Ruled'] != '1' or r['OwnerChanged'] != '0':
                continue
            if r['AfterBinding'] == '1':
                ab += 1
                continue
            keep.append((s, r))
            eased += r['EndKind'] == 'eased'
            joined += r['EndKind'] == 'joined'
            sump += 1 - (1 - 0.02 * 1.2) ** (int(r['RemainingAtCard']) // 10)
print('R2 n', len(keep), 'k_e', eased, 'joined', joined, 'after binding', ab, 'owner changed', oc, 'sum p', round(sump, 4), 'seeds', [s for s, _ in keep], wt(eased, len(keep)) if keep else '-')
print('R2 rows', [(s, r['RemainingAtCard'], r['EndKind'], r['PlacedTick'], r['CardTick']) for s, r in keep])
pe = 1 - (1 - 0.024) ** 47.5
print('R2 p_e', round(pe, 4), 'model range', 1 - (1 - 0.024) ** 40, 1 - (1 - 0.024) ** 55)

# R9
res = {}
for cell in ('wanderer', 'inputless', 'death'):
    n = k = byend = runs_int = runs_d = 0
    for s in S:
        kd, rt, _, _ = ending('%s_%d.log' % (cell, s))
        d = line('%s_%d.log' % (cell, s), 'calling deaths')
        m = re.search(r'deaths (\d+) \(by end (\d+)\)', d)
        byend += int(m.group(2))
        pd = re.search(r'per death \[([^\]]*)\]', d).group(1)
        dts = [int(x.split(':')[0]) for x in pd.split(',') if x]
        sc = re.search(r'successions \[([^\]]*)\]', d).group(1)
        sts = [int(x.split(':')[0]) for x in sc.split(',') if x]
        pre = [t for t in dts if t <= rt]
        if len(pre) != int(m.group(2)):
            print('  by-end mismatch', cell, s, len(pre), m.group(2), rt, dts)
        n += len(pre)
        runs_d += bool(pre)
        for i, t in enumerate(dts):
            if t > rt:
                continue
            ok = i < len(sts) and sts[i] <= rt
            k += not ok
        if pre and not (len(pre) - 1 < len(sts) and sts[len(pre) - 1] <= rt):
            runs_int += 1
    res[cell] = (n, k, byend)
    print('R9', cell, 'n', n, '(by end sum', byend, ') no succession', wt(k, n) if n else '-', 'runs with pre-record deaths', runs_d, 'runs ending in interregnum', runs_int)
pn = res['wanderer'][0] + res['inputless'][0]
pk = res['wanderer'][1] + res['inputless'][1]
print('R9 pool', wt(pk, pn), 'succeeded', wt(pn - pk, pn))
