import re, sys, os
sys.stdout.reconfigure(encoding='utf-8')
readme = open('Tools/mutations/README.md', encoding='utf-8').read().splitlines()
head = '\n'.join(readme[:8])
py_only = set(re.findall(r'`(b6b\w+?)`', head.split('are run by the Python tools alone')[0].split('needs the assets rebuilt')[1] if False else ''))
# Python-only: names in the sentence "The ... are run by the Python tools alone"
m = re.search(r'after the apply and again after the revert\. (.*?) are run by the Python tools alone', head, re.S)
pysent = m.group(1)
cpp, py, missing = [], [], []
patches = sorted(f for f in os.listdir('Tools/mutations/B6b') if f.endswith('.patch'))
rows = {}
for l in readme:
    mm = re.match(r'\| `B6b/` \| `(b6b\w+\.patch)`(.*)', l)
    if mm:
        rows.setdefault(mm.group(1), mm.group(2))
for p in patches:
    stem = p[:-6]
    is_py = stem in pysent or any(stem.startswith(pref) for pref in ('b6b2b_schema_', 'b6b2b_data_', 'b6b2c_schema_'))
    if is_py:
        py.append(p); continue
    r = rows.get(p)
    if r is None:
        missing.append(p); continue
    cols = r.split(' | ')
    tests = re.findall(r'`(Cosmic_Ascension\.[\w.]+)`', cols[-1] if len(cols) > 1 else r)
    if not tests:
        missing.append(p + '  (no test) ' + r[-200:]); continue
    cpp.append(f'Tools/mutations/B6b/{p}=' + '+'.join(dict.fromkeys(tests)))
open(sys.argv[1], 'w', encoding='utf-8', newline='\n').write('\n'.join(cpp) + '\n')
open(sys.argv[2], 'w', encoding='utf-8', newline='\n').write('\n'.join(py) + '\n')
print('cpp', len(cpp), 'py', len(py), 'missing', len(missing))
for x in missing: print(' MISSING', x)
