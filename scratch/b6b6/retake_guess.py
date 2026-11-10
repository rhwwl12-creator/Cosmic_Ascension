"""Which applying patches carry, as context or '-' line, a schema.json / data line that [B-6b6] edits (likely retakes)."""
import os, re, sys
root, before = sys.argv[1:3]
schema = open(os.path.join(root, 'Tools/data/schema.json'), encoding='utf-8').read().split('\n')
watch = {'schema.json': {45: 'holy_and_myth', 345: 'DT_LogTemplates.Templates', 472: 'DT_Events.LogTemplates', 724: 'DT_Hazards.DisplayName',
                         738: 'Outbreak', 739: 'Expire', 740: 'Resolve'}}
lines = {schema[n - 1]: lab for n, lab in watch['schema.json'].items()}
# the end of the DT_Megaprojects and DT_Feats rules (rules appended after) - context around them also moves
ok = {l.split(' ', 1)[1].strip() for l in open(before, encoding='utf-8') if l.startswith('OK ')}
hits = {}
for p in sorted(ok):
    txt = open(os.path.join(root, p), encoding='utf-8', errors='replace').read().split('\n')
    cur = None
    for l in txt:
        if l.startswith('+++ b/'):
            cur = l[6:]
        elif cur == 'Tools/data/schema.json' and l[:1] in (' ', '-') and l[1:] in lines:
            hits.setdefault(p, set()).add(lines[l[1:]])
        elif cur in ('Tools/data/DT_Hazards.json', 'Tools/data/DT_Megaprojects.json', 'Tools/data/DT_Feats.json') and l[:1] in (' ', '-') and l[1:].strip() == ']':
            hits.setdefault(p, set()).add('end of ' + os.path.basename(cur))
for p, s in hits.items():
    print(p.replace('Tools/mutations/', ''), sorted(s))
print(len(hits))
