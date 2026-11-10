"""[B-6b8] put Tools/baselines/seed42.json back into the committed shape (json.dumps indent 4, LF, final newline, no BOM,
no \\u0027 - the shape of HEAD). Adapted from scratchpad\\b6b7\\fix_seed42.py (lesson 3).

usage (from the project root): python -I -X utf8 <this file> [--from-on] [--final]
  (no flag)   reshape only (after any runner with -UpdateBaseline - PowerShell 5.1's ConvertTo-Json rewrites the file with
              CRLF, aligned keys and \\u0027)
  --from-on   also write the five key lines from the probe's Saved/Balance/b6b8_on logs (the smoke runner rewrites only the
              three short lines; the two 20,000-tick lines come from the probe's own runs with the runner's arguments) and
              set verified_at to 2026-11-10
  --final     require that the five lines equal b6b8_on's letter for letter (wrote path aside) - after the final full runner
last_recorded_in_changelog is filled by the records stage (brief 3.3 item 4), not here."""
import json
import os
import re
import subprocess
import sys

P = 'Tools/baselines/seed42.json'
HERE = os.path.dirname(os.path.abspath(__file__))
ON = os.path.join('Saved', 'Balance', 'b6b8_on')
PAIRS = [('checks', 'summary_line', ()), ('checks_ruler', 'ruler_summary_line', ('ruler',)),
         ('checks_wanderer', 'wanderer_summary_line', ('ruler', 'wanderer')), ('checks_20000', 'summary_line_20000', ()),
         ('checks_ruler_20000', 'ruler_summary_line_20000', ('ruler',))]


def key_line(log, heads):
    head, parts = None, []
    for l in open(log, encoding='utf-8', errors='replace').read().splitlines():
        m = re.search(r'(CosmicBalance: seed .*)$', l)
        if m:
            head = m.group(1).rstrip()
        m2 = re.search(r'(CosmicBalance (ruler|wanderer): .*)$', l)
        if m2 and head is not None and m2.group(2) in heads:
            parts.append(m2.group(1).rstrip())
    return ' | '.join([head] + parts)


d = json.load(open(P, encoding='utf-8-sig'))
if '--from-on' in sys.argv[1:]:
    for stem, key, heads in PAIRS:
        d[key] = key_line(os.path.join(ON, stem + '.log'), heads)
    d['verified_at'] = '2026-11-10'
    print('five lines written from', ON)
text = json.dumps(d, indent=4) + '\n'
with open(P, 'w', encoding='utf-8', newline='\n') as f:
    f.write(text)
b = open(P, 'rb').read()
assert b.count(b'\r') == 0 and not b.startswith(b'\xef\xbb\xbf') and b.endswith(b'}\n')
assert b'\\u0027' not in b and b'\\u003e' not in b
print('written: %d bytes, LF, no BOM' % len(b))
if '--final' in sys.argv[1:]:
    rc = subprocess.call([sys.executable, '-I', '-X', 'utf8', os.path.join(HERE, 'check_base.py'), '.', ON])
    if rc != 0:
        print('FINAL: the five lines are NOT those of b6b8_on - stop')
        sys.exit(1)
    print('FINAL: the five lines equal b6b8_on letter for letter (wrote path aside)')
