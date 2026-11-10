"""The mechanical rerun set of [B-6b8] (brief section 5; the form of scratchpad/b6b6/rerun_set.py): the old patches whose
+++ b/ file this commit edits, plus the old patches whose README 'Breaks' names an automation test defined in a C++ test file
this commit edits (the [B-6b6] precedent: Python test files are covered by their by-file patches), plus this commit's new
b6b8_ patches. An old patch that does not apply at HEAD (apply_before.txt FAIL) is dead and not this commit's: listed apart.

Edited files = the brief's list (section 5) united with every file the working tree changes with a non-zero numstat (OneDrive
stat noise has none), docs/, the README, generated.sha1 and Content/ left out (no patch targets them).

usage: python -I rerun_set.py <root> <apply_before.txt> <out.txt>
out: one line per patch, '<patch>=<filter>' where filter is
  Cosmic_Ascension.<A>[+Cosmic_Ascension.<B>...]   an automation test run (scratchpad/b6b/run_mutations2.py; a Tools/data patch
                                                  that reaches the assets is rebuilt by it)
  python:<tool>                                   a Tools-only patch run by scratchpad/b6b/run_py_mutations.py (<tool> the one it
                                                  should fail: Tools/test_data_schema.py or Tools/check_data.py)
  probe                                           a commandlet probe, not a mutation (README) - apply check only
  harness:bundleB-smoke                           checked by the bundleB.py --max-seed 2 wanderer smoke
Lines starting with '#' are comments (sections, dead patches)."""
import os
import re
import subprocess
import sys

root, before, out_path = sys.argv[1:4]
S = 'Source/Cosmic_Ascension/'
BRIEF = ['Tools/data/DT_Events.json', 'Tools/data/DT_Encounters.json', 'Tools/data/schema.json', 'Tools/test_data_schema.py',
         S + 'Tests/CosmicDataTests.cpp', S + 'Tests/CosmicB6bRowNames.h', S + 'World/CosmicWandererPolicy.h',
         S + 'World/CosmicWandererPolicy.cpp', S + 'Tests/CosmicBalanceCommandlet.cpp', S + 'Tests/CosmicSideRowTests.cpp']

numstat = subprocess.run(['git', '-C', root, 'diff', '--numstat', 'HEAD'], capture_output=True, text=True, encoding='utf-8').stdout
changed = []
for line in numstat.splitlines():
    a, d, path = line.split('\t', 2)
    if (a, d) != ('0', '0') and not path.startswith(('docs/', 'Content/')) and path not in ('Tools/mutations/README.md', 'Tools/data/generated.sha1'):
        changed.append(path)
EDITED = list(BRIEF) + [p for p in changed if p not in BRIEF]
TEST_FILES = [p for p in EDITED if p.startswith(S + 'Tests/') and p.endswith('.cpp')]

state = {}
for line in open(before, encoding='utf-8'):
    ok, p = line.strip().split(' ', 1)
    state[p] = ok


def tests_in(path):
    full = os.path.join(root, path)
    if not os.path.exists(full):
        return set()
    txt = open(full, encoding='utf-8', errors='replace').read()
    return set(re.findall(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\(\s*\w+\s*,\s*"Cosmic_Ascension\.([\w.]+)"', txt))


tests = {p: tests_in(p) for p in TEST_FILES}
readme = open(os.path.join(root, 'Tools/mutations/README.md'), encoding='utf-8').read()
rows = {}
for m in re.finditer(r'^\| `([^`]+)/` \| `([^`]+\.patch)`(.*)$', readme, re.M):
    folder, name, rest = m.groups()
    breaks = rest.rsplit('|', 2)[-2] if rest.count('|') >= 2 else rest
    rows['Tools/mutations/%s/%s' % (folder.strip('/'), name)] = breaks


def targets(p):
    full = os.path.join(root, p)
    if not os.path.exists(full):
        return []
    return [l[6:].strip() for l in open(full, encoding='utf-8', errors='replace') if l.startswith('+++ b/')]


def filter_of(p):
    br = rows.get(p, '')
    name = os.path.basename(p)
    # The B1 b1d_ patches have no README row (README keeps rows from b1a on only for B1/): the [B-1d] commit message (36c19f2)
    # says its nine mutations broke Sim.WandererLoop and Sim.WandererPolicy - both are named, so the patch dies if either fails.
    if not br and name.startswith('b1d_'):
        return 'Cosmic_Ascension.Sim.WandererPolicy+Cosmic_Ascension.Sim.WandererLoop'
    if '_probe_' in name or 'a probe, not a mutation' in br:
        return 'probe'
    if 'bundleB.py' in br:
        return 'harness:bundleB-smoke'
    named = re.findall(r'`?Cosmic_Ascension\.([\w.]+?)`?(?=[\s`(),;]|$)', br)
    named = list(dict.fromkeys(n.rstrip('.') for n in named))
    if named:
        return '+'.join('Cosmic_Ascension.' + n for n in named)
    if 'check_data.py' in br:
        return 'python:Tools/check_data.py'
    if 'test_data_schema' in br or 'test_' in br:
        return 'python:Tools/test_data_schema.py'
    return 'UNKNOWN'


by_file, by_test = {}, {}
for p in sorted(state):
    for t in targets(p):
        if t in EDITED:
            by_file.setdefault(t, []).append(p)
    br = rows.get(p, '')
    for f, names in tests.items():
        hit = sorted(n for n in names if re.search(r'Cosmic_Ascension\.' + re.escape(n) + r'(?![\w.])', br))
        if hit:
            by_test.setdefault(f, []).append((p, hit))

old = set()
for ps in by_file.values():
    old.update(ps)
for pairs in by_test.values():
    old.update(p for p, _ in pairs)
live = sorted(p for p in old if state.get(p) == 'OK')
dead = sorted(p for p in old if state.get(p) != 'OK')

new = sorted(p for p in rows if os.path.basename(p).startswith('b6b8_'))
new_files = sorted('Tools/mutations/B6b/' + f for f in os.listdir(os.path.join(root, 'Tools/mutations/B6b')) if f.startswith('b6b8_'))
new_all = sorted(set(new) | set(new_files))

lines = ['# [B-6b8] rerun set (mechanical - scratchpad/b6b8/rerun_set.py; apply state at HEAD 220129d from apply_before.txt)']
lines.append('# edited files: ' + ', '.join(EDITED))
for f in EDITED:
    ps = by_file.get(f, [])
    lines.append('# by file %s: %d patches (%d apply at HEAD)' % (f, len(ps), sum(state[p] == 'OK' for p in ps)))
for f in TEST_FILES:
    pairs = by_test.get(f, [])
    extra = [p for p, _ in pairs if not any(p in v for v in by_file.values())]
    lines.append('# by Breaks test in %s: %d patches, %d not already counted by file' % (f, len(pairs), len(extra)))
lines.append('# old union: %d patches, %d apply at HEAD (rerun), %d FAIL at HEAD (dead, not this commit\'s)' % (len(old), len(live), len(dead)))
counts = {}
lines.append('# --- old patches to rerun (%d)' % len(live))
for p in live:
    flt = filter_of(p)
    kind = flt.split(':')[0] if ':' in flt or flt in ('probe', 'UNKNOWN') else 'cpp'
    counts[kind] = counts.get(kind, 0) + 1
    lines.append('%s=%s' % (p, flt))
lines.append('# --- new b6b8_ patches (%d; a README row without its file yet, or a file without its row, is marked)' % len(new_all))
for p in new_all:
    mark = '' if (p in rows and os.path.exists(os.path.join(root, p))) else ('  # NO FILE YET' if p in rows else '  # NO README ROW')
    flt = filter_of(p)
    lines.append('%s=%s%s' % (p, flt, mark))
lines.append('# --- dead (FAIL at HEAD, not rerun) (%d)' % len(dead))
lines += ['# ' + p for p in dead]
lines.append('# old by kind: ' + ', '.join('%s %d' % kv for kv in sorted(counts.items())))
with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
    f.write('\n'.join(lines) + '\n')
print('\n'.join(l for l in lines if l.startswith('#')))
print('new:', len(new_all))
