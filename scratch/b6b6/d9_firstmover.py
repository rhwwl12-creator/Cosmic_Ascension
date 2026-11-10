import csv, sys, os
root = sys.argv[1]
SKIP = {'TickMicros','SaveBytes','SaveRawBytes','LogLen','ChronicleLen','ChronicleLetGo','ToldLines'}
def load(p):
    with open(p, encoding='utf-8') as f:
        return list(csv.DictReader(f))
for name in ['checks','checks_ruler','checks_wanderer','checks_20000','checks_ruler_20000']:
    on = load(os.path.join(root,'b6b6_on',name+'.csv'))
    off = load(os.path.join(root,'b6b6_offA',name+'.csv'))
    first = None
    for a,b in zip(on,off):
        ka = {k:v for k,v in a.items() if k not in SKIP}
        kb = {k:v for k,v in b.items() if k not in SKIP}
        if ka != kb:
            diff = [k for k in ka if ka[k]!=kb.get(k)]
            first = (a['Tick'], a['CivID'], diff, [(k,ka[k],kb.get(k)) for k in diff][:4])
            break
    # first T3 tick and treaty growth in on vs off
    t3 = min((int(r['Tick']) for r in on if r['Tier'] and int(r['Tier'])>=3), default=None)
    # rows file: extinction first placement
    rows = load(os.path.join(root,'b6b6_on',name+'_rows.csv'))
    keys = rows[0].keys() if rows else []
    ext = [r for r in rows if 'xtinction' in ' '.join(r.values())]
    ext_first = min((int(r.get('Tick',0) or 0) for r in ext), default=None)
    print(name, 'firstT3', t3, 'firstdiff', first, 'first extinction row', ext_first)
