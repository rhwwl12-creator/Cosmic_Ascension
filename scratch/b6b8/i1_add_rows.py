"""[B-6b8] I1: append the two side rows (brief_working.md 1.1 and 1.2 JSON blocks, tab-indented) to DT_Events.json and
DT_Encounters.json, keeping LF, tabs and the closing bracket. Re-checks the texts against N 1.5 C6/C7 before writing.
usage: i1_add_rows.py <project root> <brief_working.md>"""
import json
import re
import sys

root, brief_path = sys.argv[1], sys.argv[2]
brief = open(brief_path, encoding='utf-8').read()
blocks = re.findall(r'```json\n(.*?)\n```', brief, re.S)
rows = {json.loads(b)['Name']: b for b in blocks}
notes = open(root + '/docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md', encoding='utf-8').read()
start = notes.index('**`event_era_endured`**(`DT_Events`, `[B-6b8]`)')
block = notes[start:notes.index('## 1.6 ', start)]
c6, c7 = block[:block.index('**`enc_witness`**')], block[block.index('**`enc_witness`**'):]
era, wit = json.loads(rows['event_era_endured']), json.loads(rows['enc_witness'])
texts6 = era['LogTemplates']
texts7 = [wit['Title'], wit['Body']] + [c[k] for c in wit['Choices'] for k in ('Label', 'SuccessText', 'FailureText') if c[k]]
for t in texts6:
    assert ("'" + t + "'") in c6, t
for t in texts7:
    assert ("'" + t + "'") in c7, t

for name, fname in (('event_era_endured', 'DT_Events.json'), ('enc_witness', 'DT_Encounters.json')):
    path = root + '/Tools/data/' + fname
    data = open(path, 'rb').read()
    assert b'\r' not in data
    text = data.decode('utf-8')
    assert ('"' + name + '"') not in text, 'already there: ' + name
    tail = '\t}\n]\n'
    assert text.endswith(tail)
    text = text[:-len(tail)] + '\t},\n' + rows[name] + '\n]\n'
    json.loads(text)
    open(path, 'wb').write(text.encode('utf-8'))
    print('appended', name, 'to', fname, 'rows now', len(json.loads(text)))
