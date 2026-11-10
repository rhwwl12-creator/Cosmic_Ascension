import os, re, collections
SP = r"C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad"
FC = os.path.join(SP, 'b6b11', 'bundleC')
FB = os.path.join(SP, 'b6b8', 'bundleB')
C = 9000
res = collections.Counter()
bad = []
for cell, suf in [('R', ''), ('slowR', ''), ('death', ''), ('wanderer', ''), ('wanderer', '_off'), ('inputless', ''), ('inputless', '_off')]:
    for s in range(1, 61):
        n = '%s_%d%s.log' % (cell, s, suf)
        tb = open(os.path.join(FB, n), encoding='utf-8', errors='replace').read()
        tc = open(os.path.join(FC, n), encoding='utf-8', errors='replace').read()
        f5 = re.search(r'fifth rung first at (-?\d+) \((\w+)', tb)
        si = int(re.search(r'silence first at (-?\d+)', tb).group(1))
        cands = []
        if f5 and int(f5.group(1)) >= 0:
            cands.append((int(f5.group(1)), f5.group(2)))
        if si >= 0:
            cands.append((si, 'NoCandidate'))
        cands.sort()
        m = re.search(r'ending: record (\w+) at (-?\d+)', tc)
        kd, t = m.group(1), int(m.group(2))
        if cands and cands[0][0] <= C:
            exp = cands[0]
        else:
            exp = (C, 'Clock')
        ok = (t, kd) == exp
        res[(cell + suf, ok)] += 1
        if not ok:
            bad.append((n, exp, (t, kd), cands))
for k, v in sorted(res.items()):
    print(k, v)
print(bad[:20])
