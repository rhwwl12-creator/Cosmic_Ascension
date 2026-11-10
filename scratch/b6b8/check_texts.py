"""[B-6b8] start stage, item 2: the brief's two JSON rows (sections 1.1, 1.2) carry N section 1.5's C6/C7 texts letter for
letter, and the Rule W words give 0 hits on the witness texts. Read-only.
usage: check_texts.py <brief_final.md> <notes .md>"""
import json
import re
import sys

brief = open(sys.argv[1], encoding='utf-8').read()
notes = open(sys.argv[2], encoding='utf-8').read()
start = notes.index('**`event_era_endured`**(`DT_Events`, `[B-6b8]`)')
end = notes.index('## 1.6 ', start)
block = notes[start:end]
c6 = block[:block.index('**`enc_witness`**')]
c7 = block[block.index('**`enc_witness`**'):]

rows = [json.loads(m.group(1)) for m in re.finditer(r'```json\n(.*?)\n```', brief, re.S)]
era = next(r for r in rows if r['Name'] == 'event_era_endured')
wit = next(r for r in rows if r['Name'] == 'enc_witness')

ok = True
def need(text, where, label):
    global ok
    hit = ("'" + text + "'") in where
    ok &= hit
    print(('OK   ' if hit else 'MISS ') + label + ': ' + text)

for i, t in enumerate(era['LogTemplates']):
    need(t, c6, 'C6 LogTemplates[%d]' % i)
need(wit['Title'], c7, 'C7 Title')
need(wit['Body'], c7, 'C7 Body')
for i, ch in enumerate(wit['Choices']):
    need(ch['Label'], c7, 'C7 Choices[%d].Label' % i)
    need(ch['SuccessText'], c7, 'C7 Choices[%d].SuccessText' % i)
    if ch['FailureText']:
        need(ch['FailureText'], c7, 'C7 Choices[%d].FailureText' % i)
    else:
        hit = '실패 ""(빈 글)' in c7
        ok &= hit
        print(('OK   ' if hit else 'MISS ') + 'C7 Choices[%d].FailureText empty' % i)
print('C7 check stats:', [c['CheckStat'] for c in wit['Choices']], [c['Difficulty'] for c in wit['Choices']],
      '(N: first Mind 3, second NoCheck)', 'Mind 3' in c7 and 'NoCheck' in c7)

# Rule W of brief 1.4, as a plain substring test.
m = re.search(r'`witness_words` = \[(.*?)\]', brief)
witness = json.loads('[' + m.group(1) + ']')
sign_voice = json.loads('[' + re.search(r'`sign_voice_words` = \[(.*?)\]', brief).group(1) + ']')
cols = {'Title': [wit['Title']], 'Body': [wit['Body']], 'VoiceBodies': wit['VoiceBodies'],
        'Choices.Label': [c['Label'] for c in wit['Choices']], 'Choices.SuccessText': [c['SuccessText'] for c in wit['Choices']],
        'Choices.FailureText': [c['FailureText'] for c in wit['Choices']]}
hits = [(col, w) for col, texts in cols.items() for t in texts for w in witness if w in t]
hits += [('Body', w) for w in sign_voice if w in wit['Body']]
hits += [(col, '기록') for col in ('Title', 'Body', 'VoiceBodies', 'Choices.Label', 'Choices.FailureText') for t in cols[col] if '기록' in t]
print('witness_words', len(witness), 'sign_voice_words', len(sign_voice), 'Rule W hits on C7:', hits or 0)
ok &= not hits
print('ALL OK' if ok else 'NOT OK')
sys.exit(0 if ok else 1)
