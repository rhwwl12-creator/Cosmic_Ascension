import os, re, json, csv, collections
F = r"C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b11\bundleC"
C = 9000
cells = [('R', 60, C, ''), ('slowR', 60, C, ''), ('death', 60, C, ''), ('wanderer', 60, C, ''), ('wanderer', 60, C, '_off'),
         ('inputless', 60, 20000, ''), ('inputless', 60, 20000, '_off'), ('inputless075', 60, 20000, ''), ('test', 140, 20000, '')]
bad = collections.Counter()
for cell, n, ticks, suf in cells:
    kinds = collections.Counter()
    tmax = -1
    for s in range(1, n + 1):
        name = '%s_%d%s' % (cell, s, suf)
        txt = open(os.path.join(F, name + '.log'), encoding='utf-8', errors='replace').read()
        bad['end check'] += len(re.findall(r'end check', txt))
        bad['race check'] += len(re.findall(r'race check', txt))
        bad['LogCosmic: Error'] += txt.count('LogCosmic: Error')
        bad['rules cannot set'] += txt.count('-Rules cannot set')
        m = re.search(r'ending: record (\w+) at (-?\d+)', txt)
        kd, t = m.group(1), int(m.group(2))
        if kd == 'None' or t < 0 or t > C or (kd == 'Clock' and t != C) or (kd != 'Clock' and t == C and False):
            bad['F1 ' + name] += 1
        kinds[kd] += 1
        tmax = max(tmax, t)
        rl = re.findall(r'CosmicBalance: rules (\S+) = (\S+)', txt)
        exp = {'test': [('FallenCoreMinLanesFromHome', '1')], 'inputless075': [('AIHostilePowerRatio', '0.75')]}.get(cell, [])
        if [(a, b) for a, b in rl] != exp:
            bad['F8 ' + name + ' ' + str(rl)] += 1
        # F2 csv last tick
        with open(os.path.join(F, name + '.csv'), encoding='utf-8') as f:
            last = None
            for row in csv.reader(f):
                last = row
        if int(last[0]) != ticks:
            bad['F2 ' + name] += 1
    print('%-15s %d x %d: %s (latest record %d)' % (cell + suf, n, ticks, dict(sorted(kinds.items())), tmax))
print('problems', dict(bad))
d = json.load(open(os.path.join(F, 'summary.json'), encoding='utf-8'))
pairs = d['pairs']
print('pairs', len(pairs), 'ok', sum(p['ok'] for p in pairs), 'predicted', sum(p['predicted'] for p in pairs), 'G<T_B', [(p['pair'], p['G'], p['T_B']) for p in pairs if p['G'] is not None and p['G'] < p['T_B']])
for cell in ('wanderer', 'inputless'):
    ps = [p for p in pairs if p['cell'] == cell]
    gaps = [(p['G'] - p['T_B']) if p['G'] is not None else None for p in ps]
    print(cell, 'pairs', len(ps), 'G none', sum(g is None for g in gaps), 'min G-T_B', min(g for g in gaps if g is not None), 'T_B_eff==T_B', all(p['T_B_eff'] == p['T_B'] for p in ps))
    print('  G list', [(p['seed'], p['T_B'], p['G']) for p in ps])
bd = d['bridge_d']
ents = bd['entries']
print('bridge_d keys', list(bd.keys()))
print('B-d entries', len(ents), 'on', sum(not e['off'] for e in ents), 'off', sum(e['off'] for e in ents), 'may_move', [e['name'] for e in ents if e['may_move']], 'moved', [e['name'] for e in ents if e['moved']], 'not ok', [e for e in ents if e['why']])
print('B-d cells', collections.Counter((e['cell'], e['off']) for e in ents))
print({k: v for k, v in bd.items() if k != 'entries'})
print('B-b', len(d['bridge_b']), sum(b['ok'] for b in d['bridge_b']), collections.Counter(b['bridge'] for b in d['bridge_b']))
print('B-c', len(d['bridge_c']), sum(b['ok'] for b in d['bridge_c']))
be = d['bridge_e']
print('B-e keys', list(be.keys()), {k: v for k, v in be.items() if k not in ('off', 'on')})
print('B-e off', {k: v for k, v in be['off'].items() if k != 'seeds'})
print('f7', d['f7']['ok'], sum(r['P'] for r in d['f7']['runs']), sum(r['X'] for r in d['f7']['runs']), {k: v for k, v in d['f7'].items() if k != 'runs'})
print('R10', d['readings'].get('R10'))
print('R6 keys', list(d['readings'].keys()))
