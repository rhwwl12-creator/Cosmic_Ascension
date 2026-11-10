"""I1: the spec file for i1_pymut.py - the new and retaken patches (by hand) and the rerun set (every applying patch on a file
I1 edits, its tool read from the README's Breaks cell; one line per named test so each must fail on its own).
usage: i1_specs.py <project root> <out spec> <out skipped>"""
import os, re, subprocess, sys
root, out, skipped_out = sys.argv[1:4]
here = os.path.dirname(os.path.abspath(__file__))
readme = open(os.path.join(root, 'Tools', 'mutations', 'README.md'), encoding='utf-8').read()
U, E, A, R, F, P = ('test_upper_rung_text_words', 'test_every_rule_of_the_schema_can_fail', 'test_the_ark_and_its_word',
                    'test_first_person_and_card_words', None, None)
hand = {
    'B6b/b6b6_schema_ark_rule_dropped.patch': [E, A],
    'B6b/b6b6_schema_ark_guard_feats_dropped.patch': [E, A],
    'B6b/b6b6_schema_ark_guard_projects_dropped.patch': [E, A],
    'B6b/b6b6_schema_ark_words_off_hazards.patch': [A],
    'B6b/b6b6_schema_ark_words_off_lines.patch': [A, U],
    'B6b/b6b6_schema_myth_without_noah.patch': [A],
    'B6b/b6b6_data_extinction_says_ark.patch': ['check_data'],
    'B6b/b6b6_data_ark_rescues.patch': ['check_data'],
    'B6b/b6b6_data_ark_word_in_card.patch': [A],
    'B6b/b6b6_schema_myth_takes_baro.patch': [R],
    'B6b/b6b6_schema_verdict_without_soul_has_soul.patch': [R],
}

def tools_of(name):
    m = re.search(r'^\| `B\w+/` \| `%s` .*\| ([^|]*)\|\s*$' % re.escape(os.path.basename(name)), readme, re.M)
    if not m:
        return None
    cell = m.group(1)
    found = [t for t in re.findall(r'\b(test_[a-z0-9_]+)\b', cell) if t != 'test_data_schema']
    if not found and 'test_data_schema.py' in cell:
        found.append('schema')
    if 'check_data.py' in cell:
        found.append('check_data')
    if 'Cosmic_Ascension.' in cell:
        found.append('CPP')
    return found

mine = {'Tools/data/schema.json', 'Tools/data/DT_Hazards.json', 'Tools/data/DT_Megaprojects.json', 'Tools/data/DT_Feats.json',
        'Tools/test_data_schema.py', 'Config/DefaultGameplayTags.ini'}
specs, skipped = [], []
for name, whats in hand.items():
    specs += ['Tools/mutations/%s=%s' % (name, w) for w in whats]
for d, _, fs in os.walk(os.path.join(root, 'Tools', 'mutations')):
    for f in sorted(fs):
        if not f.endswith('.patch'):
            continue
        rel = os.path.relpath(os.path.join(d, f), root).replace('\\', '/')
        short = rel[len('Tools/mutations/'):]
        if short in hand or os.path.basename(f).startswith('b6b6_'):
            continue
        text = open(os.path.join(root, rel), encoding='utf-8', errors='replace').read()
        if not set(re.findall(r'^\+\+\+ b/(\S+)', text, re.M)) & mine:
            continue
        if subprocess.run(['git', 'apply', '--check', rel], cwd=root, capture_output=True).returncode != 0:
            continue
        whats = tools_of(f)
        if not whats:
            skipped.append('%s (no README row read)' % rel)
            continue
        if 'CPP' in whats:
            skipped.append('%s (C++ test: %s)' % (rel, ', '.join(w for w in whats if w != 'CPP') or 'Cosmic_Ascension.*'))
            whats = [w for w in whats if w != 'CPP']
            if not whats:
                continue
        specs += ['%s=%s' % (rel, w) for w in whats]
open(out, 'w', encoding='utf-8', newline='\n').write('\n'.join(specs) + '\n')
open(skipped_out, 'w', encoding='utf-8', newline='\n').write('\n'.join(skipped) + '\n')
print(len(specs), 'spec lines;', len(skipped), 'skipped')
