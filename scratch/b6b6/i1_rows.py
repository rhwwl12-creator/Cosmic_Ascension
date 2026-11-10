"""I1: append the three [B-6b6] rows (texts verbatim from N 1.5, via rows_dryrun's C1-C3 cross-checked) to the project's JSON.
usage: i1_rows.py <project root>"""
import json, os, sys, re
root = sys.argv[1]
D = os.path.join(root, 'Tools', 'data')
N = open(os.path.join(root, 'docs', '60_DECISIONS', 'headers', 'B6b_upper_rungs_and_the_race_notes.md'), encoding='utf-8').read()

def q(s):
    return json.dumps(s, ensure_ascii=False)

def texts(lst):
    return '[\n' + ',\n'.join('\t\t\t' + q(s) for s in lst) + '\n\t\t]'

def from_n(label_regex, block):
    """Pull ' · '-separated [i] '...' items of one bullet from N's block."""
    m = re.search(label_regex + r'[^\n]*?: (\[0\] .*?)\.?\n', block)
    assert m, label_regex
    items = re.findall(r"\[(\d)\] '(.*?)'(?= · \[|\.?$)", m.group(1))
    assert [i for i, _ in items] == ['0', '1', '2'], (label_regex, items)
    return [t.replace('\\|', '|') for _, t in items]

sec = N[N.index('## 1.5 '):N.index('## 1.6 ')]
b1 = sec[sec.index('**`crisis_species_extinction`**'):sec.index('**`project_ark`**')]
b2 = sec[sec.index('**`project_ark`**'):sec.index('**`feat_passage_by_ark`**')]
b3 = sec[sec.index('**`feat_passage_by_ark`**'):sec.index('**`feat_laws_and_binding`**')]
out = from_n(r'- 발생 줄', b1); exp = from_n(r'- 만료 줄', b1); res = from_n(r'- 해결 줄', b1)
c1name = re.search(r"- 화면 이름: '(.*?)'", b1).group(1)
c2name = re.search(r"- 화면 이름: '(.*?)'", b2).group(1)
comp = from_n(r'- 완성 줄', b2)
title = re.search(r"- 제목: '(.*?)'\. 서사: '(.*?)'\.\n", b3)
lines = from_n(r'- 줄\(', b3)
assert c1name == '절멸 위기' and c2name == '방주' and title.group(1) == '방주와 조약'
for s in out + exp + res:
    assert '방주' not in s

hazard = (
    '\t{\n'
    '\t\t"Name": "crisis_species_extinction",\n'
    '\t\t"HazardTag": "Hazard.Extinction",\n'
    '\t\t"SourceDoctrine": "",\n'
    f'\t\t"DisplayName": {q(c1name)},\n'
    '\t\t"CountdownTicks": 250,\n'
    '\t\t"SpawnRule": "Draw",\n'
    '\t\t"SpawnChancePerTick": 0.0001,\n'
    '\t\t"SpawnConditions": [\n'
    '\t\t\t{\n'
    '\t\t\t\t"Type": "CivTierAtLeast",\n'
    '\t\t\t\t"Tag": "",\n'
    '\t\t\t\t"Key": "None",\n'
    '\t\t\t\t"Value": 3.0\n'
    '\t\t\t}\n'
    '\t\t],\n'
    '\t\t"OnExpireEffects": [],\n'
    '\t\t"BranchChance": 0.0,\n'
    '\t\t"OnExpireBranchEffects": [],\n'
    '\t\t"OnExpireNeighbourEffects": [],\n'
    '\t\t"OnResolveEffects": [],\n'
    '\t\t"RulerResolutions": ["project_ark"],\n'
    '\t\t"WandererResolutions": [],\n'
    '\t\t"LogCategory": "Log.Crisis",\n'
    '\t\t"Severity": 1,\n'
    f'\t\t"OutbreakTemplates": {texts(out)},\n'
    f'\t\t"ExpireTemplates": {texts(exp)},\n'
    f'\t\t"ResolveTemplates": {texts(res)},\n'
    '\t\t"OutbreakRow": "None"\n'
    '\t}\n')
project = (
    '\t{\n'
    '\t\t"Name": "project_ark",\n'
    f'\t\t"DisplayName": {q(c2name)},\n'
    '\t\t"RequiredTier": "Tier3_Causal",\n'
    '\t\t"CostPerTick": {"Energy": 3.0, "Matter": 3.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0},\n'
    '\t\t"RequiredProgress": 150.0,\n'
    f'\t\t"CompleteTemplates": {texts(comp)}\n'
    '\t}\n')
feat = (
    '\t{\n'
    '\t\t"Name": "feat_passage_by_ark",\n'
    f'\t\t"Title": {q(title.group(1))},\n'
    f'\t\t"MythicNarrative": {q(title.group(2))},\n'
    '\t\t"RequiredTier": "Tier3_Causal",\n'
    '\t\t"TargetTier": "Tier4_Transcendent",\n'
    '\t\t"Path": "Ruler",\n'
    '\t\t"CompletionConditions": [\n'
    '\t\t\t{"Type": "MegaprojectComplete", "Tag": "", "Key": "project_ark", "Value": 0.0},\n'
    '\t\t\t{"Type": "CivHasTreaty", "Tag": "", "Key": "None", "Value": 1.0}\n'
    '\t\t],\n'
    '\t\t"AscensionCost": [\n'
    '\t\t\t{"Type": "ShiftEthics", "Tag": "", "Key": "Openness", "Value": -0.1, "DurationTicks": 0}\n'
    '\t\t],\n'
    '\t\t"GrantedPowers": "(GameplayTags=)",\n'
    '\t\t"DedicationBank": "",\n'
    '\t\t"FX": "",\n'
    '\t\t"Sound": "",\n'
    f'\t\t"LogTemplates": {texts(lines)}\n'
    '\t}\n')

for name, row in (('DT_Hazards.json', hazard), ('DT_Megaprojects.json', project), ('DT_Feats.json', feat)):
    p = os.path.join(D, name)
    raw = open(p, 'rb').read().decode('utf-8')
    assert raw.endswith('\t}\n]\n') and '\r' not in raw, name
    new = raw[:-len('\t}\n]\n')] + '\t},\n' + row + ']\n'
    json.loads(new)
    open(p, 'wb').write(new.encode('utf-8'))
    print('appended to', name)

# cross-check against the start-state dry-run copy (rows_dryrun.py's C1-C3)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
src = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'rows_dryrun.py'), encoding='utf-8').read()
ns = {}
exec(src[src.index('C1 = {'):src.index('for f, row in')], ns)
for name, key in (('DT_Hazards.json', 'C1'), ('DT_Megaprojects.json', 'C2'), ('DT_Feats.json', 'C3')):
    data = json.load(open(os.path.join(D, name), encoding='utf-8'))
    assert data[-1] == ns[key], (name, 'differs from rows_dryrun')
print('rows equal rows_dryrun C1-C3')
