"""[B-6b7] put Tools/baselines/seed42.json back into the committed shape (json.dumps indent 4, LF, final newline, no BOM,
no \\u0027 - the shape of HEAD) and fill last_recorded_in_changelog (brief 3.2 item 2). Adapted from
scratchpad\\b6b6\\fix\\fix_seed42.py.

usage (from the project root): python -I -X utf8 <this file> [--final]
Run it after every -UpdateBaseline (PowerShell 5.1's ConvertTo-Json rewrites the file with CRLF, aligned keys and \\u0027).
--final: also require that the five lines carry the measured hashes (true only after the final full runner - the smoke
runner does not rewrite the two 20,000-tick lines)."""
import json
import re
import sys

P = 'Tools/baselines/seed42.json'
FINAL = '--final' in sys.argv[1:]

NEW = {  # measured by the [B-6b7] measure stage (Saved/Balance/b6b7_on); the final runner must give the same
    'summary_line': (2557850423, 2906170550),
    'ruler_summary_line': (1189870756, 3359656751),
    'wanderer_summary_line': (1631315934, 489794244),
    'summary_line_20000': (1957158585, 3764704052),
    'ruler_summary_line_20000': (1375149537, 510604211),
}
OLD = {  # [B-6b6]
    'summary_line': (2557850423, 2906170550),
    'ruler_summary_line': (1189870756, 3359656751),
    'wanderer_summary_line': (271068805, 309834531),
    'summary_line_20000': (3168555397, 1640870674),
    'ruler_summary_line_20000': (4053448003, 129517356),
}
TASK = ('B6b B-6b7 (data 2 fourth to fifth: edict_wide_machine_accord/quarantine/warning, feat_laws_and_binding, '
        'project_binding P 1800, r 0.375)')

d = json.load(open(P, encoding='utf-8-sig'))
bad = []
for key, (h, w) in NEW.items():
    m = re.search(r'hash (\d+), world (\d+)', d[key])
    got = (int(m.group(1)), int(m.group(2)))
    if got != (h, w):
        bad.append('%s: file %d / %d, measured %d / %d' % (key, got[0], got[1], h, w))
if bad:
    print('NOT the measured hashes:\n  ' + '\n  '.join(bad))
    if FINAL:
        sys.exit(1)

rec = d['last_recorded_in_changelog']
if not rec.get('task', '').startswith('B6b B-6b7'):
    old_task, old_note = rec['task'], rec['note']
    moved = ', '.join('%s %d / %d -> %d / %d' % (label, OLD[k][0], OLD[k][1], NEW[k][0], NEW[k][1]) for label, k in (
        ('wanderer 6,000', 'wanderer_summary_line'), ('input-less 20,000', 'summary_line_20000'),
        ('ruler 20,000', 'ruler_summary_line_20000')))
    note = (
        'Measured 2026-11-10 on Windows (editor binaries). The galaxy laws, feat_laws_and_binding and r 0.375 are read only '
        'from the first fourth rung T4* of any people, so a line may move only when T4* lies inside its horizon. The 3,000-tick '
        'lines (and so both hash runs) have no T4* and stay bit-identical: input-less 2557850423 / 2906170550, ruler '
        '1189870756 / 3359656751. First movers: wanderer 6,000 - civ_2 (AI) on T4 at 3,197, its binding started at 3,198 and '
        'its three laws held from 3,199, first differing CSV sample 3,200; the record stays the clock at 6,000. Input-less '
        '20,000 - civ_8 (AI) on T4 at 3,149, binding from 3,150, laws from 3,151, first differing sample 3,200; civ_8 binds '
        'and stands on the fifth rung at 7,950 (another order, after the clock record at 6,000, which stays). Ruler 20,000 - '
        'civ_6 (AI) on T4 at 3,651, binding from 3,652, laws from 3,653, first differing sample 3,700; the ruled civ_0 now '
        'reaches T4 at 3,844 (was 5,168), starts the binding at 3,844, holds the laws from 3,887, binds at 5,644 and stands '
        'on the fifth rung at 5,899 - the record is now the ruled people\'s end at 5,899 (was the clock at 6,000). Every CSV '
        'and _rows row with Tick < T4* equals the B-6b6 run\'s (TickMicros, SaveBytes and SaveRawBytes aside). The off side '
        '(-WithoutB6bData=1, eight rows taken out and r back to 1) gives the B-6b5 lines letter for letter; every log has '
        '0 end check and 0 race check lines. Moved: ' + moved + '. The two hash runs of the game process equal the '
        'commandlet\'s 3,000-tick lines. Older values: the previous note - ' + old_task + ': ' + old_note)
    d['last_recorded_in_changelog'] = {
        'task': TASK,
        'state_hash_3000': NEW['summary_line'][0],
        'world_hash_3000': NEW['summary_line'][1],
        'state_hash_20000': NEW['summary_line_20000'][0],
        'world_hash_20000': NEW['summary_line_20000'][1],
        'note': note,
    }
    print('last_recorded_in_changelog filled')
else:
    print('last_recorded_in_changelog already B-6b7 - kept')

text = json.dumps(d, indent=4) + '\n'
with open(P, 'w', encoding='utf-8', newline='\n') as f:
    f.write(text)

if FINAL:
    # Review L2-1: -UpdateBaseline overwrites without comparing, and the hash check above reads two numbers per line. Compare
    # the whole five lines (ruler and wanderer parts included, the 'wrote' path dropped as Remove-WrotePath does) with the
    # key lines of the measure stage's Saved/Balance/b6b7_on logs (the same comparison as check_base.py).
    import subprocess, os
    here = os.path.dirname(os.path.abspath(__file__))
    rc = subprocess.call([sys.executable, '-I', '-X', 'utf8', os.path.join(here, 'check_base.py'), '.',
                          os.path.join('Saved', 'Balance', 'b6b7_on')])
    if rc != 0:
        print('FINAL: the five lines are NOT those of b6b7_on - stop')
        sys.exit(1)
    print('FINAL: the five lines equal b6b7_on letter for letter (wrote path aside)')
b = open(P, 'rb').read()
assert b.count(b'\r') == 0 and not b.startswith(b'\xef\xbb\xbf') and b.endswith(b'}\n')
assert b'\\u0027' not in b and b'\\u003e' not in b
print('written: %d bytes, LF, no BOM' % len(b))
