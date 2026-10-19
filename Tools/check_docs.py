"""Documentation lint (standard library only): broken relative links, references to section numbers that the section map
does not know, and the root files every session depends on.

Usage: python -I Tools/check_docs.py        exit 1 on broken links / missing root files; other findings are notes.
"""
import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = os.path.join(ROOT, 'docs')
errors, notes = [], []

for fn in ('CLAUDE.md', 'STATUS.md', 'WORKPLAN.md', 'docs/SECTION_MAP.md', 'docs/50_ROADMAP.md', 'docs/CHANGELOG.md'):
    if not os.path.exists(os.path.join(ROOT, fn)): errors.append('missing: ' + fn)

md_files = [os.path.join(ROOT, f) for f in ('CLAUDE.md', 'STATUS.md', 'WORKPLAN.md')]
for dirpath, _, files in os.walk(DOCS):
    if 'archive' in dirpath: continue
    md_files += [os.path.join(dirpath, f) for f in files if f.endswith('.md')]

link_re = re.compile(r'\]\(([^)\s]+)\)')
known_sections = set()
smap = os.path.join(DOCS, 'SECTION_MAP.md')
if os.path.exists(smap):
    known_sections = set(re.findall(r'^\| §(\d+(?:\.\d+)?) ', open(smap, encoding='utf-8').read(), re.M))

long_lines = 0
for path in md_files:
    rel = os.path.relpath(path, ROOT).replace('\\', '/')
    text = open(path, encoding='utf-8').read()
    for m in link_re.finditer(text):
        t = m.group(1)
        if t.startswith(('http://', 'https://', '#', 'mailto:')): continue
        target = os.path.normpath(os.path.join(os.path.dirname(path), t.split('#')[0]))
        if not os.path.exists(target): errors.append(f'{rel}: broken link {t}')
    if known_sections and not rel.startswith('docs/archive'):
        for s in set(re.findall(r'§(\d+\.\d+)', text)):
            if s not in known_sections and s.split('.')[0] in {x for x in known_sections if '.' not in x}:
                notes.append(f'{rel}: §{s} is not in SECTION_MAP (sub-section added after the freeze?)')
    long_lines += sum(1 for ln in text.split('\n') if len(ln) > 2000)

cl = os.path.join(ROOT, 'CLAUDE.md')
if os.path.exists(cl) and os.path.getsize(cl) > 16 * 1024:
    notes.append(f'CLAUDE.md is {os.path.getsize(cl) // 1024}KB (target <= 16KB — move detail to docs/)')
notes.append(f'lines over 2000 bytes across living docs: {long_lines} (verbatim-moved text; shrink per system as it is rewritten)')
for n in notes[:15]: print('NOTE  ' + n)
if len(notes) > 15: print(f'NOTE  ... {len(notes) - 15} more')
for e in errors: print('ERROR ' + e)
print(f'check_docs: {len(md_files)} files, {len(errors)} error(s)')
sys.exit(1 if errors else 0)
