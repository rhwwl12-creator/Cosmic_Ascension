"""I1: git apply --check (in the project, no change) of every patch that touches a file I1 edits; compare with apply_before.txt.
usage: i1_apply_mine.py <project root>"""
import os, re, subprocess, sys
root = sys.argv[1]
here = os.path.dirname(os.path.abspath(__file__))
mine = {'Tools/data/schema.json', 'Tools/data/DT_Hazards.json', 'Tools/data/DT_Megaprojects.json', 'Tools/data/DT_Feats.json',
        'Tools/test_data_schema.py', 'Config/DefaultGameplayTags.ini'}
before = {}
for line in open(os.path.join(here, 'apply_before.txt'), encoding='utf-8'):
    state, path = line.split()
    before[path] = state
rows = []
for d, _, fs in os.walk(os.path.join(root, 'Tools', 'mutations')):
    for f in sorted(fs):
        if not f.endswith('.patch'):
            continue
        rel = os.path.relpath(os.path.join(d, f), root).replace('\\', '/')
        text = open(os.path.join(root, rel), encoding='utf-8', errors='replace').read()
        files = set(re.findall(r'^\+\+\+ b/(\S+)', text, re.M))
        if not files & mine:
            continue
        r = subprocess.run(['git', 'apply', '--check', rel], cwd=root, capture_output=True, text=True)
        now = 'OK' if r.returncode == 0 else 'FAIL'
        rows.append((rel, before.get(rel, 'NEW'), now, sorted(files & mine), sorted(files - mine)))
for rel, b, n, m, other in sorted(rows):
    flag = 'RETAKE' if (b == 'OK' and n == 'FAIL') else ''
    print(f'{b:4} -> {n:4} {flag:6} {rel}  mine={m} other={other}')
print('total', len(rows), 'retake', sum(1 for r in rows if r[1] == 'OK' and r[2] == 'FAIL'))
