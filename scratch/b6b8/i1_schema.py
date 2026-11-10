"""[B-6b8] I1: Rules W and E and the three witness lists into Tools/data/schema.json (text edits, LF and tabs kept).
usage: i1_schema.py <project root>"""
import json
import sys

path = sys.argv[1] + '/Tools/data/schema.json'
data = open(path, 'rb').read()
assert b'\r' not in data
text = data.decode('utf-8')


def once(old, new):
    global text
    assert text.count(old) == 1, (text.count(old), old[:80])
    text = text.replace(old, new)


witness = ["본 것", "보고 들은", "말하", "말했", "말해", "전하", "전했", "전해", "증거", "증언", "증인", "목격", "기록하", "기록했", "기록해",
           "지나간다", "지나가", "지나갔", "떠난다", "떠나", "떠났", "외면", "눈을 돌", "등을 돌", "모른 척", "피했", "도망", "부끄"]
assert len(witness) == 28
sign_voice = ["목소리", "속삭", "이르기를", "말씀"]
dump = lambda words: json.dumps(words, ensure_ascii=False)  # noqa: E731

# The lists, after the last list ([B-6b6]'s ark_words).
once('\t\t"ark_words": ["방주"]\n\t},',
     '\t\t"ark_words": ["방주"],\n'
     '\t\t"witness_words": %s,\n'
     '\t\t"sign_voice_words": %s,\n'
     '\t\t"witness_record_words": ["기록"]\n\t},' % (dump(witness), dump(sign_voice)))

# The about-text, after the ark's paragraph.
ark_about = ("'노아' is a name of holy_and_myth.\",\n")
once(ark_about, ark_about +
     "\t\t\"The witness's words (B6b [B-6b8], header 2.8, notes 1.6 rule 7; decision 20): witness_words (a testimony, a record made, "
     "a telling or a speaking, and a step back that leaves, looks away or is ashamed - the contracted past forms are listed apart), "
     "sign_voice_words (a voice of the sign) and witness_record_words ('기록') hang by one DT_Encounters row rule on the wanderer's "
     "cards whose conditions name the claim: witness_words on every text column, sign_voice_words on the body, witness_record_words "
     "on every text column but the choices' success lines (a column test cannot tell one choice from another, so the step back's "
     "success line is held by the text audit alone). A substring test: a false hit inside a longer word is accepted in this narrow "
     "scope. The era of an endured pressure (decision 22) keeps its shape by a DT_Events row rule: the ruled people's condition "
     "and the Borders reverence.\",\n")

# Rule E, after the era words rule of DT_Events.
era_rule_end = ('{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "desolation_words"}]\n'
                '\t\t\t\t}\n')
once(era_rule_end, era_rule_end[:-1] + ',\n'
     '\t\t\t\t{\n'
     '\t\t\t\t\t"why": "the era of an endured pressure is told for the ruled people alone and raises its neighbours\' reverence for the vessel '
     '(B6b decision 22, the answer A; header 2.7) - without Ruled every AI people\'s endurance would lift its neighbours",\n'
     '\t\t\t\t\t"when": [{"col": "Conditions", "has_type": "CivPressureEnduredTicksReached"}],\n'
     '\t\t\t\t\t"then": [{"col": "Conditions", "has_cond": {"type": "SystemOwnedBy", "key": "Ruled"}}, '
     '{"col": "Effects", "has_cond": {"type": "ChangeReverence", "key": "Borders"}}]\n'
     '\t\t\t\t}\n')

# Rule W, after the petition_pressure_measured rule (the last DT_Encounters rule).
measured_end = ('\t\t\t\t\t"when": [{"row_not": "petition_pressure_measured"}],\n'
                '\t\t\t\t\t"then": [{"col": "Choices.OnSuccess", "lacks_cond": {"type": "DelayHazard", "tag": "Hazard.Claim"}}]\n'
                '\t\t\t\t}\n')
cols = ("Title", "Body", "VoiceBodies", "Choices.Label", "Choices.SuccessText", "Choices.FailureText")
w_tests = ', '.join('{"col": "%s", "lacks_words": "witness_words"}' % c for c in cols[:3])
w_tests2 = ', '.join('{"col": "%s", "lacks_words": "witness_words"}' % c for c in cols[3:])
r_tests = ', '.join('{"col": "%s", "lacks_words": "witness_record_words"}' % c for c in cols[:3])
r_tests2 = ', '.join('{"col": "%s", "lacks_words": "witness_record_words"}' % c for c in ("Choices.Label", "Choices.FailureText"))
once(measured_end, measured_end[:-1] + ',\n'
     '\t\t\t\t{\n'
     '\t\t\t\t\t"why": "a wanderer\'s card on the claim is a sign seen or measured: it does not testify, record, tell or speak, the sign has no '
     'voice, and the step back neither leaves nor shames (B6b header 2.8, notes 1.6 rule 7; decision 20). A substring test: the contracted '
     'past forms are listed apart, and a false hit inside a longer word is accepted within this narrow scope",\n'
     '\t\t\t\t\t"when": [{"row_prefix": "enc_"}, {"col": "Conditions", "has_cond": {"tag": "Hazard.Claim"}}],\n'
     '\t\t\t\t\t"then": [\n'
     '\t\t\t\t\t\t' + w_tests + ',\n'
     '\t\t\t\t\t\t' + w_tests2 + ',\n'
     '\t\t\t\t\t\t{"col": "Body", "lacks_words": "sign_voice_words"},\n'
     '\t\t\t\t\t\t' + r_tests + ',\n'
     '\t\t\t\t\t\t' + r_tests2 + '\n'
     '\t\t\t\t\t]\n'
     '\t\t\t\t}\n')

json.loads(text)
open(path, 'wb').write(text.encode('utf-8'))
print('schema written')
