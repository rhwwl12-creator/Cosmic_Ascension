"""Self-test of timing_summary.py ([B-6br] brief section 9 step 3): fake TSVs built from a dry-run TSV of section 0.5's
logs (run_timing_ab.ps1 -Sides after -DryLogs ...). Usage: python -I -X utf8 timing_summary_selftest.py <dry timing.tsv>"""
import csv, io, os, shutil, subprocess, sys, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
SRC = sys.argv[1]
meta = [l for l in open(SRC, encoding='utf-8') if l.startswith('#')]
body = [l for l in open(SRC, encoding='utf-8') if not l.startswith('#') and l.strip()]
rows = list(csv.DictReader(body, delimiter='\t'))
fields = list(rows[0].keys())
base = {r['name']: r for r in rows if r['round'] == '1'}
fails = 0
def expect(cond, what):
    global fails
    print(('ok   ' if cond else 'FAIL ') + what)
    fails += 0 if cond else 1
def make(rounds, ai_after=0.45, eco_after=None, before_ai=1.0, tweak=None):
    out = []
    for rd in rounds:
        for name, r in base.items():
            for side in ('before', 'after'):
                x = dict(r); x['side'] = side; x['round'] = str(rd)
                if side == 'after':
                    x['ai_us'] = '%.2f' % (float(r['ai_us']) * ai_after)
                    if eco_after and rd in eco_after:
                        x['economy_us'] = '%.2f' % (float(r['economy_us']) * eco_after[rd])
                else:
                    x['ai_us'] = '%.2f' % (float(r['ai_us']) * before_ai)
                if tweak:
                    tweak(x)
                out.append(x)
    return out
def run(tmp, rows_, *extra):
    p = os.path.join(tmp, 'timing.tsv')
    with open(p, 'w', encoding='utf-8', newline='') as f:
        f.writelines(meta)
        w = csv.DictWriter(f, fieldnames=fields, delimiter='\t', lineterminator='\n')
        w.writeheader(); w.writerows(rows_)
    r = subprocess.run([sys.executable, '-I', '-X', 'utf8', os.path.join(HERE, 'timing_summary.py'), '--tsv', p] + list(extra), capture_output=True, text=True, encoding='utf-8')
    md = open(os.path.join(tmp, 'summary.md'), encoding='utf-8').read() if os.path.exists(os.path.join(tmp, 'summary.md')) else ''
    return r.returncode, r.stdout + r.stderr, md
tmp = tempfile.mkdtemp(prefix='timing_summary_selftest_')
try:
    code, out, md = run(tmp, make([1, 2, 3]))
    expect(code == 0 and 'timing_summary: PASS' in out, 'three valid rounds, after AI x0.45: exit 0 (got %d)' % code)
    code, out, _ = run(tmp, make([1, 2, 3]), '--next-round')
    expect(code == 0 and out.strip() == 'none', '--next-round with enough rounds: none (got %r)' % out.strip())
    void2 = make([1, 2, 3], eco_after={2: 1.40})
    code, out, md = run(tmp, void2)
    expect(code == 1 and 'next: round 4' in out and '2 valid' in out, 'round 2 void (economy x1.40): exit 1, asks for round 4, two valid rounds (got %d)' % code)
    expect('| 2 |' in md and 'yes - economy after/before 1.39' in md and 'outside 0.85-1.18' in md, 'the void round is recorded with its economy ratio')
    code, out, _ = run(tmp, void2, '--next-round')
    expect(out.strip() == '4', '--next-round after one void round: 4 (got %r)' % out.strip())
    low = make([1, 2, 3], eco_after={3: 0.80})
    code, out, _ = run(tmp, low, '--next-round')
    expect(out.strip() == '4', 'a round below the band (x0.80) is void too: next 4 (got %r)' % out.strip())
    code, out, _ = run(tmp, void2 + make([4]))
    expect(code == 0 and '4 run, 1 void, 3 valid' in out, 'round 4 replaces the void round: exit 0 (got %d)' % code)
    bad = make([1, 2, 3, 4, 5], eco_after={1: 1.3, 3: 1.3, 5: 1.3})
    code, out, _ = run(tmp, bad, '--next-round')
    expect(out.strip() == 'invalid', 'three void rounds of five: invalid (got %r)' % out.strip())
    code, out, _ = run(tmp, bad)
    expect(code == 1 and 'next: invalid' in out, 'three void rounds of five: exit 1, session invalid (got %d)' % code)
    code, out, _ = run(tmp, make([1, 2, 3], ai_after=1.0))
    expect(code == 1 and 'FAIL gate' in out, 'after AI as before (19.34 ...): gate FAIL, exit 1 (got %d)' % code)
    def moved(x):
        if x['side'] == 'after' and x['name'] == 'checks_ruler' and x['round'] == '2':
            x['summary'] = x['summary'].replace('hash 1189870756', 'hash 1189870757')
    code, out, md = run(tmp, make([1, 2, 3], tweak=moved))
    expect(code == 1 and 'FAIL lines' in out and 'after checks_ruler r2: line differs' in md, 'one moved hash: lines FAIL, exit 1 (got %d)' % code)
    def err(x):
        if x['side'] == 'before' and x['name'] == 'off_checks' and x['round'] == '1':
            x['errors'] = '0/0/1'
    code, out, _ = run(tmp, make([1, 2, 3], tweak=err))
    expect(code == 1 and 'FAIL lines' in out, 'a LogCosmic error line on the before side: exit 1 (got %d)' % code)
    code, out, md = run(tmp, make([1, 2, 3], before_ai=0.8))
    expect(code == 0 and 'before_below_gate:' in out and 'checks_ruler' in out, 'before medians under 16.05: before_below_gate flagged (got %d)' % code)
    def spike(x):
        if x['side'] == 'after' and x['name'] == 'checks_20000' and x['round'] == '3':
            x['ai_us'] = '16.50'
    code, out, md = run(tmp, make([1, 2, 3], tweak=spike))
    expect(code == 0 and 'checks_20000 r3 16.50' in md, 'one after-run >= 16.05 with a passing median: recorded, exit 0 (got %d)' % code)
    rp = os.path.join(tmp, 'runner.txt')
    open(rp, 'w', encoding='utf-8').write(''.join('   stages: AI %.2f us, economy 1.60 us, crisis 9.00 us (free memory 10.5 GB)\n' % v for v in (7.1, 6.6, 8.6, 12.9, 10.8)))
    code, out, md = run(tmp, make([1, 2, 3]), '--runner-out', rp)
    expect(code == 0 and 'ruler 20,000: AI 10.80' in md, "the final runner's five stages lines are recorded (got %d)" % code)
finally:
    shutil.rmtree(tmp, ignore_errors=True)
print('timing_summary_selftest: %s' % ('PASS' if fails == 0 else 'FAIL (%d)' % fails))
sys.exit(1 if fails else 0)
