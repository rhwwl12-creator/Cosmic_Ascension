"""git apply --check every Tools/mutations patch against a scratch copy of a revision (never in the project).
usage: apply_state.py <project root> <rev> <scratch dir> <out file>"""
import io, os, shutil, subprocess, sys, tarfile

root, rev, scratch, out = sys.argv[1:5]
if os.path.exists(scratch):
    shutil.rmtree(scratch)
os.makedirs(scratch)
data = subprocess.run(['git', '-c', 'core.eol=lf', '-c', 'core.autocrlf=false', '-C', root, 'archive', '--format=tar', rev, 'Source', 'Tools', 'Config'], capture_output=True, check=True).stdout
with tarfile.open(fileobj=io.BytesIO(data)) as t:
    t.extractall(scratch, filter='data')
# The archive must be the revision's bytes (LF): spot-check a few files against the blobs.
for p in ['Tools/data/schema.json', 'Source/Cosmic_Ascension/Tests/CosmicTestHelpers.h', 'Tools/data/DT_Hazards.json']:
    blob = subprocess.run(['git', '-C', root, 'cat-file', 'blob', f'{rev}:{p}'], capture_output=True, check=True).stdout
    assert open(os.path.join(scratch, p), 'rb').read() == blob, p
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
print('patches', len(lines), 'OK', sum(l.startswith('OK') for l in lines), 'FAIL', sum(l.startswith('FAIL') for l in lines))
