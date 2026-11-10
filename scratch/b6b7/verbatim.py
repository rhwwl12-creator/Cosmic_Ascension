import json
N = open('docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md', encoding='utf-8').read()
s = N[N.index("**`feat_laws_and_binding`**"):N.index("**`event_era_endured`**")].replace('\|', '|')
E = {r['Name']: r for r in json.load(open('Tools/data/DT_Edicts.json', encoding='utf-8'))}
F = {r['Name']: r for r in json.load(open('Tools/data/DT_Feats.json', encoding='utf-8'))}
f = F['feat_laws_and_binding']
texts = [f['Title'], f['MythicNarrative']] + f['LogTemplates']
for n in ['edict_wide_machine_accord', 'edict_wide_quarantine', 'edict_wide_warning']:
    texts += [E[n]['DisplayName'], E[n]['Summary']]
    assert f"`{n}`: 이름 '{E[n]['DisplayName']}', 요약 '{E[n]['Summary']}'." in s, n
for t in texts:
    assert f"'{t}'" in s, t
print('verbatim ok', len(texts), 'texts;', 'edicts', len(E), 'feats', len(F))
