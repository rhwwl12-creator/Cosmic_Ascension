"""[B-6b7] item 8: do the five b6b6_on logs give seed42.json's whole key lines (after the 'wrote' path, as run_checks.ps1's
Remove-WrotePath)? Read-only. usage: check_base.py <project root> <folder>"""
import json, os, re, sys

root, folder = sys.argv[1], sys.argv[2]
base = json.load(open(os.path.join(root, 'Tools', 'baselines', 'seed42.json'), encoding='utf-8'))
pairs = [('checks', 'summary_line'), ('checks_ruler', 'ruler_summary_line'), ('checks_wanderer', 'wanderer_summary_line'),
         ('checks_20000', 'summary_line_20000'), ('checks_ruler_20000', 'ruler_summary_line_20000')]


def strip(line):
    # Remove-WrotePath: drop ', wrote <path>' up to ' | ' or the end.
    return re.sub(r', wrote [^|]*?(?= \| |$)', '', line.strip())


def key_line(log):
    text = open(log, encoding='utf-8', errors='replace').read().splitlines()
    head = None
    parts = []
    for l in text:
        m = re.search(r'(CosmicBalance: seed .*)$', l)
        if m:
            head = m.group(1)
        m2 = re.search(r'(CosmicBalance (ruler|wanderer): .*)$', l)
        if m2 and head is not None:
            parts.append(m2.group(1))
    return head, parts


ok = True
for stem, key in pairs:
    want = strip(base[key])
    head, parts = key_line(os.path.join(folder, stem + '.log'))
    got = strip(' | '.join([head] + [p for p in parts if p.split(':')[0] in want]))
    same = got == want
    ok &= same
    print(('SAME ' if same else 'DIFF ') + stem)
    if not same:
        print('  want', want)
        print('  got ', got)
print('ALL SAME' if ok else 'NOT ALL SAME')
sys.exit(0 if ok else 1)  # fix stage (review L2-1): a script may gate on it
