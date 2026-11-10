"""I1: the [B-6b6] schema edits (brief 1.6 1-4a) as exact text replacements on Tools/data/schema.json.
usage: i1_schema.py <project root>"""
import json, os, sys
root = sys.argv[1]
p = os.path.join(root, 'Tools', 'data', 'schema.json')
s = open(p, 'rb').read().decode('utf-8')

def once(old, new):
    global s
    assert s.count(old) == 1, (s.count(old), old[:120])
    s = s.replace(old, new)

# 1. '노아' at the end of holy_and_myth
once('"호산나", "마라나타"]', '"호산나", "마라나타", "노아"]')

# 2. ark_words after the last list (binding_name_words)
once('"binding_name_words": ["탑", "둘레", "울타리", "산울", "고리", "관측소", "공명", "감응", "교감"]\n',
     '"binding_name_words": ["탑", "둘레", "울타리", "산울", "고리", "관측소", "공명", "감응", "교감"],\n'
     '\t\t"ark_words": ["방주"]\n')

# 3. the about line, after the upper rungs' words line
once('\t\t"A table whose columns are null has no rows yet: its columns are declared when its first row is written."\n',
     '\t\t"The ark\'s word (B6b [B-6b6], decision 33): ark_words (\'방주\') stands only in the ark and the fourth-rung feat (project_ark, feat_passage_by_ark) - '
     'it hangs on the name and the three template columns of DT_Hazards, on DT_Events.LogTemplates and DT_LogTemplates.Templates, and by two row_not rules on '
     'the name and completion lines of every other project and the title, narrative and lines of every other feat; Tools/test_data_schema.py reads every '
     'other source for it, ST_CA included. The ark\'s own row rule keeps rescue, title and holy names out of its name and self-rescue and holy names out of '
     'its completion lines, and \'노아\' is a name of holy_and_myth.",\n'
     '\t\t"A table whose columns are null has no rows yet: its columns are declared when its first row is written."\n')

# 4a. forbidden places
once('\t\t\t\t\t"forbidden": ["verdict", "soul_verbs", "holy_words", "rescue_words", "rung_grasp_words", "title_words"]\n',
     '\t\t\t\t\t"forbidden": ["verdict", "soul_verbs", "holy_words", "rescue_words", "rung_grasp_words", "title_words", "ark_words"]\n')
once('"LogTemplates": {"kind": "texts", "slots": ["Civ", "System", "Hazard"], "forbidden": ["verdict", "holy_words", "soul_verbs", "rescue_words", "rung_grasp_words", "title_words"]},',
     '"LogTemplates": {"kind": "texts", "slots": ["Civ", "System", "Hazard"], "forbidden": ["verdict", "holy_words", "soul_verbs", "rescue_words", "rung_grasp_words", "title_words", "ark_words"]},')
hz = s.index('"DT_Hazards": {')
hz_end = s.index('"DT_Megaprojects": {')
part = s[hz:hz_end]
old = '"DisplayName": {"kind": "text", "required": true, "slots": [], "forbidden": "verdict"},'
assert part.count(old) == 1
part = part.replace(old, '"DisplayName": {"kind": "text", "required": true, "slots": [], "forbidden": ["verdict", "ark_words"]},')
for col in ('OutbreakTemplates', 'ExpireTemplates', 'ResolveTemplates'):
    old = f'"{col}": {{"kind": "texts", "slots": ["Civ", "System", "Hazard"], "forbidden": ["verdict", "rescue_words", "rung_grasp_words", "title_words"]}},'
    assert part.count(old) == 1, col
    part = part.replace(old, old[:-len(']},')] + ', "ark_words"]},')
s = s[:hz] + part + s[hz_end:]

# 4a. the ark's rule and the projects' row_not rule, after project_binding's rule
once('\t\t\t\t\t\t{"col": "CompleteTemplates", "lacks_words": "binding_words"}, {"col": "CompleteTemplates", "lacks_words": "triumph_words"}, {"col": "CompleteTemplates", "lacks_words": "dawn_words"}]\n'
     '\t\t\t\t}\n'
     '\t\t\t]\n',
     '\t\t\t\t\t\t{"col": "CompleteTemplates", "lacks_words": "binding_words"}, {"col": "CompleteTemplates", "lacks_words": "triumph_words"}, {"col": "CompleteTemplates", "lacks_words": "dawn_words"}]\n'
     '\t\t\t\t},\n'
     '\t\t\t\t{\n'
     '\t\t\t\t\t"why": "the people crosses the extinction in the ark it built; the ark is no one\'s rescuer - no rescue, title or holy name in its name, no self-rescue, new birth, washing, new land or Flood picture in its completion lines (B6b header 3.3 rule 13, 3.5 rule 2, decision 33)",\n'
     '\t\t\t\t\t"when": [{"row": "project_ark"}],\n'
     '\t\t\t\t\t"then": [{"col": "DisplayName", "lacks_words": "rescue_words"}, {"col": "DisplayName", "lacks_words": "title_words"}, {"col": "DisplayName", "lacks_words": "holy_and_myth"},\n'
     '\t\t\t\t\t\t{"col": "DisplayName", "lacks_words": "self_rescue_words"}, {"col": "CompleteTemplates", "lacks_words": "self_rescue_words"}, {"col": "CompleteTemplates", "lacks_words": "holy_and_myth"}]\n'
     '\t\t\t\t},\n'
     '\t\t\t\t{\n'
     '\t\t\t\t\t"why": "\'방주\' is the ark\'s word: no other project is named or told with it - it stands only in the ark and the fourth-rung feat (B6b decision 33)",\n'
     '\t\t\t\t\t"when": [{"row_not": "project_ark"}],\n'
     '\t\t\t\t\t"then": [{"col": "DisplayName", "lacks_words": "ark_words"}, {"col": "CompleteTemplates", "lacks_words": "ark_words"}]\n'
     '\t\t\t\t}\n'
     '\t\t\t]\n')

# 4a. the feats' row_not rule, after the fifth-rung rule (last of DT_Feats)
once('\t\t\t\t\t\t{"col": "MythicNarrative", "lacks_words": "dawn_words"}, {"col": "LogTemplates", "lacks_words": "holy_passive"}, {"col": "LogTemplates", "lacks_words": "dawn_words"}]\n'
     '\t\t\t\t}\n'
     '\t\t\t]\n',
     '\t\t\t\t\t\t{"col": "MythicNarrative", "lacks_words": "dawn_words"}, {"col": "LogTemplates", "lacks_words": "holy_passive"}, {"col": "LogTemplates", "lacks_words": "dawn_words"}]\n'
     '\t\t\t\t},\n'
     '\t\t\t\t{\n'
     '\t\t\t\t\t"why": "\'방주\' is said among the feats by the fourth-rung feat alone, whose deed is the ark: no other feat\'s title, narrative or lines carry it (B6b decision 33)",\n'
     '\t\t\t\t\t"when": [{"row_not": "feat_passage_by_ark"}],\n'
     '\t\t\t\t\t"then": [{"col": "Title", "lacks_words": "ark_words"}, {"col": "MythicNarrative", "lacks_words": "ark_words"}, {"col": "LogTemplates", "lacks_words": "ark_words"}]\n'
     '\t\t\t\t}\n'
     '\t\t\t]\n')

json.loads(s)
open(p, 'wb').write(s.encode('utf-8'))
print('schema edited')
