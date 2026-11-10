"""git apply --check every Tools/mutations patch against HEAD's Source/Tools/Config overlaid with a snapshot of the
working tree's changed files (the [B-6b6] tree), in a scratch dir; then the diff against apply_before.
usage: apply_after.py <project root> <snapshot dir> <scratch dir> <apply_before> <out file>"""
import io, os, shutil, subprocess, sys, tarfile

root, snap, scratch, before, out = sys.argv[1:6]
if os.path.exists(scratch):
    shutil.rmtree(scratch)
os.makedirs(scratch)
data = subprocess.run(['git', '-c', 'core.eol=lf', '-c', 'core.autocrlf=false', '-C', root, 'archive', '--format=tar', 'HEAD',
                       'Source', 'Tools', 'Config'], capture_output=True, check=True).stdout
with tarfile.open(fileobj=io.BytesIO(data)) as t:
    t.extractall(scratch, filter='data')
n = 0
for d, _, fs in os.walk(snap):
    for f in fs:
        rel = os.path.relpath(os.path.join(d, f), snap)
        if rel.split(os.sep)[0] in ('Source', 'Tools', 'Config'):
            dst = os.path.join(scratch, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copy2(os.path.join(d, f), dst)
            n += 1
patches = []
for d, _, fs in os.walk(os.path.join(scratch, 'Tools', 'mutations')):
    for f in fs:
        if f.endswith('.patch'):
            patches.append(os.path.relpath(os.path.join(d, f), scratch).replace('\\', '/'))
patches.sort()
lines = []
for p in patches:
    r = subprocess.run(['git', 'apply', '--check', p], cwd=scratch, capture_output=True, text=True)
    lines.append(('OK ' if r.returncode == 0 else 'FAIL ') + p)
with open(out, 'w', encoding='utf-8', newline='\n') as f:
    f.write('\n'.join(lines) + '\n')
b = dict(reversed(l.strip().split(' ', 1)) for l in open(before, encoding='utf-8') if l.strip())
a = dict(reversed(l.split(' ', 1)) for l in lines)
print('overlaid', n, '| patches', len(lines), 'OK', sum(v == 'OK' for v in a.values()), 'FAIL', sum(v == 'FAIL' for v in a.values()))
print('OK -> FAIL:', sorted(p for p in a if b.get(p) == 'OK' and a[p] == 'FAIL'))
print('FAIL -> OK:', sorted(p for p in a if b.get(p) == 'FAIL' and a[p] == 'OK'))
print('new:', sorted(p for p in a if p not in b))
print('gone:', sorted(p for p in b if p not in a))
print('new that FAIL:', sorted(p for p in a if p not in b and a[p] == 'FAIL'))
