"""Self-test of mut_labels.py (B6b [B-6br] brief section 5 / section 9 step 3).
1. Label truth: every label of mut_labels.json is a substring of a message the test file can print (its TEXT literals
   with an id, expanded over the accessors, the two sets and the game's megaproject and edict rows).
2. On copies of a real mutation log (its own Error lines removed): a full fake log per patch -> exit 0; one label removed
   -> exit 1 naming it; a passing test -> exit 1; a missing log -> exit 1; both log names present -> the newer is read.
Usage: python -I mut_labels_selftest.py [ROOT]   (stdlib; writes only to a temporary folder)"""
import glob, itertools, json, os, re, shutil, subprocess, sys, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = sys.argv[1] if len(sys.argv) > 1 else r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
CPP = os.path.join(ROOT, "Source", "Cosmic_Ascension", "Tests", "CosmicTableNamesTests.cpp")
table = json.load(open(os.path.join(HERE, "mut_labels.json"), encoding="utf-8"))
ACC = ['GetCivRowNames', 'GetEventRowNames', 'GetTranscendentRowNames', 'GetHazardRowNames', 'GetFeatRowNames',
       'GetEncounterRowNames', 'GetEdictRowNames', 'GetGalaxyRowNames']
SETS = ['fresh', 'live']
rows = [r['Name'] for t in ('DT_Megaprojects', 'DT_Edicts') for r in json.load(open(os.path.join(ROOT, 'Tools', 'data', t + '.json'), encoding='utf-8'))]
src = open(CPP, encoding='utf-8').read()
lits = re.findall(r'TEXT\("((?:ii\'|ii|iii|i|b)-?\d*:[^"]*)"\)', src)
msgs = []
for lit in lits:
    n = lit.count('%s')
    if n == 0:
        msgs.append(lit)
    elif lit.startswith('b-3'):
        msgs += [lit.replace('%s', r) for r in rows]
    elif n == 1:
        msgs += [lit.replace('%s', a) for a in ACC]
    else:
        msgs += [lit % (a, s) for a, s in itertools.product(ACC, SETS)]
fails = 0
def expect(cond, what):
    global fails
    print(("ok   " if cond else "FAIL ") + what)
    fails += 0 if cond else 1
ids = sorted({m.split(':')[0] for m in msgs})
expect(len(lits) == 16 and len(ids) == 16, "%d id literals in the test file (ids %s)" % (len(lits), ", ".join(ids)))
for patch, labels in table.items():
    bad = [l for l in labels if not any(l in m for m in msgs)]
    expect(not bad, "labels of %s are messages of the test file%s" % (patch, (": not found " + " | ".join(bad)) if bad else ""))
# a real log as the template
logs = sorted(glob.glob(os.path.join(ROOT, 'Saved', 'Logs', 'Mut_*.log')), key=os.path.getmtime)
assert logs, 'no Mut_*.log to copy'
base = [l for l in open(logs[-1], encoding='utf-8-sig', errors='replace').read().splitlines() if 'Error:' not in l and 'Result={' not in l]
stamp = '[2026.11.10-12.00.00:000][  1]LogAutomationController: Error: '
def write_log(d, patch, labels, result='Fail', drop=None):
    lines = list(base)
    for lab in labels:
        if lab == drop:
            continue
        m = next(m for m in msgs if lab in m)
        lines.append(stamp + "Expected '%s' to be true. [%s(1)]" % (m, CPP))
    lines.append(stamp.replace('Error: ', 'Error: ' if result != 'Success' else 'Display: ') + 'Test Completed. Result={%s} Name={X} Path={Cosmic_Ascension.Data.X}' % result)
    with open(os.path.join(d, 'Mut_%s.log' % patch), 'w', encoding='utf-8-sig', newline='\r\n') as f:
        f.write('\n'.join(lines) + '\n')
def run(d, *patches):
    r = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'mut_labels.py'), '--logs', d] + list(patches), capture_output=True, text=True)
    return r.returncode, r.stdout + r.stderr
tmp = tempfile.mkdtemp(prefix='mut_labels_selftest_')
try:
    for p, labs in table.items():
        write_log(tmp, p, labs)
    code, out = run(tmp, 'all')
    expect(code == 0, 'full logs for all ten patches: exit 0 (got %d)' % code)
    p = 'b6br_memo_unsorted'; drop = "ii'-3:"
    write_log(tmp, p, table[p], drop=drop)
    code, out = run(tmp, p)
    expect(code == 1 and ('missing: ' + drop) in out, 'one label removed (%s): exit 1 naming it (got %d)' % (drop, code))
    # another id failing that the table does not name is info only
    write_log(tmp, p, table[p] + [])
    with open(os.path.join(tmp, 'Mut_%s.log' % p), 'a', encoding='utf-8') as f:
        f.write(stamp + "Expected 'ii-5: No table: no names, not the memo's' to be true.\n")
    code, out = run(tmp, p)
    expect(code == 0 and 'INFO' in out and 'ii-5:' in out, 'an extra failing id is INFO only: exit 0 (got %d)' % code)
    p = 'b6br_memo_case_blind'
    write_log(tmp, p, table[p], result='Success')
    code, out = run(tmp, p)
    expect(code == 1 and 'the test passed' in out, 'labels present but Result={Success}: exit 1 (got %d)' % code)
    os.remove(os.path.join(tmp, 'Mut_b6br_answer_miss_true.log'))
    code, out = run(tmp, 'b6br_answer_miss_true')
    expect(code == 1 and 'no log' in out, 'missing log: exit 1 (got %d)' % code)
    write_log(tmp, 'x', [])
    os.replace(os.path.join(tmp, 'Mut_x.log'), os.path.join(tmp, 'Mutation_b6br_answer_miss_true.log'))
    code, out = run(tmp, 'b6br_answer_miss_true')
    expect(code == 1 and 'Mutation_b6br_answer_miss_true.log' in out, "run_mutations2.py's log name is read too (no labels in it: exit 1, got %d)" % code)
    # both names present: the newer one is read (a stale Mut_ log with a label dropped, a full Mutation_ log)
    p = 'b6br_memo_unsorted'; drop = "ii'-3:"
    write_log(tmp, p, table[p], drop=drop)
    write_log(tmp, 'y', table[p])
    os.replace(os.path.join(tmp, 'Mut_y.log'), os.path.join(tmp, 'Mutation_%s.log' % p))
    old_log, new_log = os.path.join(tmp, 'Mut_%s.log' % p), os.path.join(tmp, 'Mutation_%s.log' % p)
    os.utime(old_log, (1700000000, 1700000000)); os.utime(new_log, (1700000100, 1700000100))
    code, out = run(tmp, p)
    expect(code == 0 and ('Mutation_%s.log' % p) in out, 'both logs, Mutation_ newer and full: exit 0 reading it (got %d)' % code)
    os.utime(old_log, (1700000100, 1700000100)); os.utime(new_log, (1700000000, 1700000000))
    code, out = run(tmp, p)
    expect(code == 1 and ('Mut_%s.log' % p) in out and ('missing: ' + drop) in out, 'both logs, stale Mut_ newer: exit 1 reading it (got %d)' % code)
    code, out = run(tmp, 'b6br_no_such_patch')
    expect(code == 2, 'unknown patch: exit 2 (got %d)' % code)
finally:
    shutil.rmtree(tmp, ignore_errors=True)
print('mut_labels_selftest: %s' % ('PASS' if fails == 0 else 'FAIL (%d)' % fails))
sys.exit(1 if fails else 0)
