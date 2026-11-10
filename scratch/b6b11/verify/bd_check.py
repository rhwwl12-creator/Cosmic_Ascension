import os, csv, collections
SP = r"C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad"
FC = os.path.join(SP, 'b6b11', 'bundleC')
FB = os.path.join(SP, 'b6b8', 'bundleB')
IGN = {'TickMicros', 'SaveBytes', 'SaveRawBytes'}


def load(p, below):
    with open(p, encoding='utf-8') as f:
        r = csv.reader(f)
        h = next(r)
        keep = [i for i, c in enumerate(h) if c not in IGN]
        out = []
        for row in r:
            if below is not None and int(row[0]) >= below:
                continue
            out.append(tuple(row[i] for i in keep))
        return h, out


res = collections.Counter()
diffs = []
for cell, suf, below in [('R', '', 9000), ('slowR', '', 9000), ('death', '', 9000), ('wanderer', '', 9000), ('wanderer', '_off', 9000),
                         ('inputless', '', None), ('inputless', '_off', None)]:
    for s in range(1, 61):
        for kind in ('', '_rows'):
            n = '%s_%d%s%s.csv' % (cell, s, suf, kind)
            hc, a = load(os.path.join(FC, n), below)
            hb, b = load(os.path.join(FB, n), below)
            ok = hc == hb and a == b
            res[(cell + suf, kind, ok)] += 1
            if not ok:
                first = next((i for i in range(min(len(a), len(b))) if a[i] != b[i]), None)
                diffs.append((n, len(a), len(b), first, a[first][:2] if first is not None else None))
for k, v in sorted(res.items()):
    print(k, v)
print('diffs', diffs[:20])
