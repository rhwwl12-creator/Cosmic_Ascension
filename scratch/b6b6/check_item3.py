import json, subprocess, sys
root = sys.argv[1]
s = json.load(open(root + '/Tools/data/schema.json', encoding='utf-8'))
w = s['words']
new9 = ['rung_grasp_words', 'title_words', 'binding_words', 'triumph_words', 'stay_words',
        'self_rescue_words', 'dawn_words', 'desolation_words', 'binding_name_words']
print('nine lists present:', all(k in w for k in new9), [k for k in new9 if k not in w])
for k in new9:
    v = w[k]
    words = v['words'] if isinstance(v, dict) else v
    print('  ', k, len(words))
hp = w['holy_passive']; hpw = hp['words'] if isinstance(hp, dict) else hp
print('holy_passive tail:', hpw[-5:])
ham = w['holy_and_myth']; hamw = ham['words'] if isinstance(ham, dict) else ham
print('holy_and_myth count', len(hamw), 'tail', hamw[-3:], 'noah in?', '노아' in hamw)
print('ark_words present?', 'ark_words' in w)
T = s['tables']
print('--- string forbidden values:')
for tn, t in T.items():
    for cn, c in t['columns'].items():
        f = c.get('forbidden')
        if isinstance(f, str):
            print('  STRING', tn, cn, repr(f))
print('--- forbidden of brief columns:')
for tn, cn in [('DT_Hazards', 'DisplayName'), ('DT_Hazards', 'OutbreakTemplates'), ('DT_Hazards', 'ExpireTemplates'),
               ('DT_Hazards', 'ResolveTemplates'), ('DT_Events', 'LogTemplates'), ('DT_LogTemplates', 'Templates'),
               ('DT_Megaprojects', 'DisplayName'), ('DT_Megaprojects', 'CompleteTemplates'),
               ('DT_Feats', 'Title'), ('DT_Feats', 'MythicNarrative'), ('DT_Feats', 'LogTemplates')]:
    print('  ', tn, cn, json.dumps(T[tn]['columns'][cn].get('forbidden'), ensure_ascii=False))
print('--- rules:')
for tn in ['DT_Hazards', 'DT_Megaprojects', 'DT_Feats']:
    for i, r in enumerate(T[tn].get('rules', [])):
        print('  ', tn, i, json.dumps(r.get('when'), ensure_ascii=False), '| then n=', len(r.get('then', [])))
        if tn == 'DT_Hazards' and any(x.get('is') == 'Hazard.Extinction' for x in r.get('when', [])):
            for th in r['then']:
                print('       ', json.dumps(th, ensure_ascii=False))
        if tn == 'DT_Megaprojects' and any(x.get('row') == 'project_binding' for x in r.get('when', [])):
            for th in r['then']:
                print('       ', json.dumps(th, ensure_ascii=False))
