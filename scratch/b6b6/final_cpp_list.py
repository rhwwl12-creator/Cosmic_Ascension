"""[B-6b6] final stage: the serial C++ mutation list (patch=filter) from the mechanical rerun set (rerun_set.py's rule)
plus the fix stage additions and the new b6b6_ C++/asset patches. Python-only patches (no Cosmic_Ascension.* test in
their README Breaks cell) are left out - the fix stage ran them on these bytes.
usage (project root): python -I -X utf8 final_cpp_list.py <apply_before.txt> <out list>"""
import os, re, subprocess, sys

before, out_path = sys.argv[1:3]
S = 'Source/Cosmic_Ascension/'
EDITED = ['Tools/data/schema.json', 'Tools/test_data_schema.py', 'Tools/data/DT_Hazards.json', 'Tools/data/DT_Megaprojects.json',
          'Tools/data/DT_Feats.json', 'Config/DefaultGameplayTags.ini',
          S + 'Tests/CosmicTestHelpers.h', S + 'Tests/CosmicDataTests.cpp', S + 'Tests/CosmicB6bRowNames.h', S + 'Tests/CosmicDeathTests.cpp',
          S + 'Tests/CosmicUpperRungTests.cpp', S + 'Tests/CosmicWarningLineTests.cpp',
          S + 'Tests/CosmicSimTests.cpp', S + 'Tests/CosmicAscensionTests.cpp']
TEST_FILES = [p for p in EDITED if p.endswith('.cpp')]
COMMENT_ONLY = ['Sim.NotablesCapped']

state = {}
for line in open(before, encoding='utf-8'):
    ok, p = line.strip().split(' ', 1)
    state[p] = ok

def tests_in(path):
    txt = open(path, encoding='utf-8', errors='replace').read()
    return set(re.findall(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\(\s*\w+\s*,\s*"Cosmic_Ascension\.([\w.]+)"', txt))

tests = {p: tests_in(p) for p in TEST_FILES}
readme = open('Tools/mutations/README.md', encoding='utf-8').read()
rows = {}
for m in re.finditer(r'^\| `([^`]+)/` \| `([^`]+\.patch)`(.*)$', readme, re.M):
    folder, name, rest = m.groups()
    breaks = rest.rsplit('|', 2)[-2] if rest.count('|') >= 2 else rest
    rows['Tools/mutations/%s/%s' % (folder.strip('/'), name)] = breaks

def targets(p):
    return [l[6:].strip() for l in open(p, encoding='utf-8', errors='replace') if l.startswith('+++ b/')]

allset = set()
for p in state:
    if any(t in EDITED for t in targets(p)):
        allset.add(p)
    br = rows.get(p, '')
    for f, names in tests.items():
        if any(re.search(r'Cosmic_Ascension\.' + re.escape(n) + r'(?![\w.])', br) for n in names):
            allset.add(p)
    if any(re.search(r'Cosmic_Ascension\.' + re.escape(n) + r'(?![\w.])', br) for n in COMMENT_ONLY):
        allset.add(p)
print('union', len(allset), 'apply at H\'\'', sum(state[p] == 'OK' for p in allset))

NEW = ['Tools/mutations/B6b/b6b6_kills_ruler_not_extinction.patch', 'Tools/mutations/B6b/b6b6_data_extinction_expiry_effect.patch',
       'Tools/mutations/B6b/b6b6_brake_always_shut.patch']
lines, py_only, wild, notapply = [], [], [], []
for p in NEW + sorted(allset):
    if p not in NEW and state.get(p) != 'OK':
        continue
    br = rows.get(p, '')
    names = re.findall(r'Cosmic_Ascension\.([A-Za-z][\w.]*[A-Za-z0-9])', br)
    names = [n for n in dict.fromkeys(names)]
    if not names:
        if 'Cosmic_Ascension.*' in br or '`Cosmic_Ascension`' in br:
            wild.append(p)
        else:
            py_only.append(p)
        continue
    r = subprocess.run(['git', 'apply', '--check', p], capture_output=True)
    if r.returncode != 0:
        notapply.append(p)
        continue
    lines.append('%s=%s' % (p, '+'.join('Cosmic_Ascension.' + n for n in names)))
open(out_path, 'w', encoding='utf-8', newline='\n').write('\n'.join(lines) + '\n')
print('C++ runs', len(lines), '| python only', len(py_only), '| wildcard', wild, '| do not apply now', notapply)
