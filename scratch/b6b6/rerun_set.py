"""The mechanical rerun set of [B-6b6] on [H''] (brief section 2.5): old patches whose +++ b/ file this commit edits, plus
old patches whose README 'Breaks' test is defined in a test file this commit edits. usage: rerun_set.py <root> <apply_before>"""
import os, re, sys

root, before = sys.argv[1:3]
S = 'Source/Cosmic_Ascension/'
EDITED = ['Tools/data/schema.json', 'Tools/test_data_schema.py', 'Tools/data/DT_Hazards.json', 'Tools/data/DT_Megaprojects.json',
          'Tools/data/DT_Feats.json', 'Config/DefaultGameplayTags.ini',
          S + 'Tests/CosmicTestHelpers.h', S + 'Tests/CosmicDataTests.cpp', S + 'Tests/CosmicB6bRowNames.h', S + 'Tests/CosmicDeathTests.cpp',
          S + 'Tests/CosmicUpperRungTests.cpp', S + 'Tests/CosmicWarningLineTests.cpp',
          # fix stage (review T2, T1): the measure stage's Sim.Determinism edit and the raised Feats.Reachable
          S + 'Tests/CosmicSimTests.cpp', S + 'Tests/CosmicAscensionTests.cpp']
TEST_FILES = [p for p in EDITED if p.endswith('.cpp')]
# Feats.Reachable changed, so CosmicAscensionTests.cpp is no longer conditional; TopBarAndPanel did not change.
CONDITIONAL = [S + 'Tests/CosmicPanelTests.cpp']
# Comment-only edits (review T4): only the patches whose Breaks names the test that holds the comment.
COMMENT_ONLY = {S + 'Tests/CosmicHazardTests.cpp': ['Sim.NotablesCapped']}

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

tests = {p: tests_in(p) for p in TEST_FILES + CONDITIONAL}
readme = open(os.path.join(root, 'Tools/mutations/README.md'), encoding='utf-8').read()
rows = {}
for m in re.finditer(r'^\| `([^`]+)/` \| `([^`]+\.patch)`(.*)$', readme, re.M):
    folder, name, rest = m.groups()
    breaks = rest.rsplit('|', 2)[-2] if rest.count('|') >= 2 else rest
    rows['Tools/mutations/%s/%s' % (folder.strip('/'), name)] = breaks

def targets(p):
    return [l[6:].strip() for l in open(os.path.join(root, p), encoding='utf-8', errors='replace') if l.startswith('+++ b/')]

by_file, by_test, cond = {}, {}, {}
for p in sorted(state):
    for t in targets(p):
        if t in EDITED:
            by_file.setdefault(t, []).append(p)
    br = rows.get(p, '')
    for f, names in tests.items():
        hit = sorted(n for n in names if re.search(r'Cosmic_Ascension\.' + re.escape(n) + r'(?![\w.])', br))
        if hit:
            (cond if f in CONDITIONAL else by_test).setdefault(f, []).append((p, hit))

out = []
allset = set()
out.append('### Rerun set at [H\'\'] (mechanical - recomputed by the fix stage on the final edit list, review T2)')
for f in EDITED:
    ps = by_file.get(f, [])
    if ps:
        n_ok = sum(state[p] == 'OK' for p in ps)
        out.append('- by file `%s`: %d patches (%d apply at [H\'\'])' % (f, len(ps), n_ok))
        allset.update(ps)
for f in TEST_FILES:
    ps = by_test.get(f, [])
    extra = [p for p, _ in ps if p not in allset]
    if ps:
        out.append('- by Breaks test in `%s`: %d patches, %d not already counted: %s' % (f, len(ps), len(extra),
                   ', '.join('`%s` (%s)' % (os.path.basename(p), '/'.join(h)) for p, h in ps if p in extra)))
        allset.update(p for p, _ in ps)
out.append('- union: %d patches, of which %d apply at [H\'\'] (%d FAIL - already dead, not this commit\'s)' % (
    len(allset), sum(state[p] == 'OK' for p in allset), sum(state[p] != 'OK' for p in allset)))
for f, names in COMMENT_ONLY.items():
    ps = []
    for p in sorted(state):
        br = rows.get(p, '')
        hit = [n for n in names if re.search(r'Cosmic_Ascension\.' + re.escape(n) + r'(?![\w.])', br)]
        if hit:
            ps.append(p)
    extra = [p for p in ps if p not in allset]
    out.append('- comment-only `%s` (%s): %d patches, %d not already counted: %s' % (f, '/'.join(names), len(ps), len(extra), ', '.join('`%s`' % os.path.basename(p) for p in extra) or 'none'))
    allset.update(ps)
out.append('- union with the fix stage: %d patches, of which %d apply at [H\'\'] (%d FAIL)' % (
    len(allset), sum(state[p] == 'OK' for p in allset), sum(state[p] != 'OK' for p in allset)))
for f in CONDITIONAL:
    ps = cond.get(f, [])
    out.append('- conditional (only if Feats.Reachable / StartBlockReasons / TopBarAndPanel change) `%s`: %d patches by Breaks test - %s' % (
        f, len(ps), ', '.join('`%s` (%s)' % (os.path.basename(p), '/'.join(h)) for p, h in ps) or 'none'))
dead = sorted(p for p in allset if state[p] != 'OK')
out.append('- in the union but FAIL already at [H\'\'] (%d): %s' % (len(dead), ', '.join('`%s`' % p.replace('Tools/mutations/', '') for p in dead)))
print('\n'.join(out))
