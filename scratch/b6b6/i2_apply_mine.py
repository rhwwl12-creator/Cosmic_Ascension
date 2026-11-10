"""git apply --check, on HEAD plus I2's C++ files from the working tree, every patch that touches one of those files;
print the ones whose state differs from apply_before.txt."""
import io, os, shutil, subprocess, sys, tarfile

root = r"c:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension"
here = os.path.dirname(os.path.abspath(__file__))
scratch = os.path.join(here, 'i2_apply_copy')
mine = [
    'Source/Cosmic_Ascension/Tests/CosmicB6bRowNames.h',
    'Source/Cosmic_Ascension/Tests/CosmicDataTests.cpp',
    'Source/Cosmic_Ascension/Tests/CosmicDeathTests.cpp',
    'Source/Cosmic_Ascension/Tests/CosmicTestHelpers.h',
    'Source/Cosmic_Ascension/Tests/CosmicUpperRungTests.cpp',
    'Source/Cosmic_Ascension/Tests/CosmicWarningLineTests.cpp',
] + sys.argv[1:]
if os.path.exists(scratch):
    shutil.rmtree(scratch)
os.makedirs(scratch)
data = subprocess.run(['git', '-c', 'core.eol=lf', '-c', 'core.autocrlf=false', '-C', root, 'archive', '--format=tar', 'HEAD', 'Source', 'Tools', 'Config'], capture_output=True, check=True).stdout
with tarfile.open(fileobj=io.BytesIO(data)) as t:
    t.extractall(scratch, filter='data')
for p in mine:
    shutil.copyfile(os.path.join(root, p), os.path.join(scratch, p))
before = {}
for line in open(os.path.join(here, 'apply_before.txt'), encoding='utf-8'):
    state, path = line.split()
    before[path] = state
changed = []
total = 0
for d, _, fs in os.walk(os.path.join(root, 'Tools', 'mutations')):
    for f in fs:
        if not f.endswith('.patch'):
            continue
        full = os.path.join(d, f)
        rel = os.path.relpath(full, root).replace('\\', '/')
        text = open(full, encoding='utf-8', errors='replace').read()
        if not any(('+++ b/' + m) in text for m in mine):
            continue
        total += 1
        r = subprocess.run(['git', 'apply', '--check', full], cwd=scratch, capture_output=True, text=True)
        now = 'OK' if r.returncode == 0 else 'FAIL'
        if before.get(rel) != now:
            changed.append(f'{before.get(rel)} -> {now} {rel}')
print('patches touching my files:', total)
print('\n'.join(changed) if changed else 'no change of state')
