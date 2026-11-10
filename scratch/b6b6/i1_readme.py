"""I1: README rows for the [B-6b6] Python-side mutations (new, retaken under b6b6_ names, regenerated b6b5 ones).
usage: i1_readme.py <project root>"""
import os, re, sys
root = sys.argv[1]
p = os.path.join(root, 'Tools', 'mutations', 'README.md')
s = open(p, 'rb').read().decode('utf-8')
assert '\r' not in s

def once(old, new):
    global s
    assert s.count(old) == 1, (s.count(old), old[:100])
    s = s.replace(old, new)

# Head: the Python-only list.
once("`b6b3a_schema_far_rule_takes_other` and the `b6b5_schema_` and `b6b5_data_` ones are run by the Python tools alone",
     "`b6b3a_schema_far_rule_takes_other`, the `b6b5_schema_` and `b6b5_data_` ones and the `b6b6_schema_` ones, `b6b6_data_extinction_says_ark`, "
     "`b6b6_data_ark_rescues` and `b6b6_data_ark_word_in_card` are run by the Python tools alone")
once("`python -I Tools/check_data.py`): the checker refuses them before an asset is written, so no asset is rebuilt.",
     "`python -I Tools/check_data.py`): the checker refuses them before an asset is written, so no asset is rebuilt "
     "(`b6b6_data_ark_word_in_card` passes the checker and fails `Tools/test_data_schema.py` alone - the runner then writes the assets, "
     "records the failure and exits 1; rebuild them after the revert).")

# The two B4 patches that no longer apply.
for old, new in (('b4d1_schema_myth_takes_baro', 'b6b6_schema_myth_takes_baro'),
                 ('b4d1_schema_verdict_without_soul_has_soul', 'b6b6_schema_verdict_without_soul_has_soul')):
    m = re.search(r'^(\| `B4/` \| `%s\.patch` — [^|]*?) \|' % old, s, re.M)
    assert m, old
    once(m.group(0), m.group(1) + ' (no longer applies since [B-6b6] - taken again as `%s`) |' % new)

# The b6b5 patches regenerated on the [B-6b6] schema and data (same block: same name, docs/40).
why = {
    'b6b5_schema_log_column_without_grasp': "the column's list ends in `ark_words`",
    'b6b5_schema_event_column_without_titles': "the column's list ends in `ark_words`",
    'b6b5_schema_event_lines_holy_off': "the column's list ends in `ark_words`",
    'b6b5_schema_event_lines_soul_off': "the column's list ends in `ark_words`",
    'b6b5_schema_hazard_columns_without_grasp': "the columns' lists end in `ark_words`",
    'b6b5_schema_hazard_expiry_rescue_off': "the columns' lists end in `ark_words`",
    'b6b5_schema_desolation_without_lamp': "`ark_words` follows the list",
    'b6b5_schema_fifth_feat_rule_dropped': "the feats' `row_not` rule of '방주' follows the rule",
    'b6b5_schema_binding_name_dropped': "the ark's two rules follow the rule",
    'b6b5_data_feat_says_bound': "`feat_passage_by_ark` follows the row",
}
for name, reason in why.items():
    m = re.search(r'^(\| `B6b/` \| `%s\.patch` — [^|]*?) \|' % name, s, re.M)
    assert m, name
    cell = m.group(1)
    if cell.endswith(')'):
        once(m.group(0), cell[:-1] + '; regenerated at [B-6b6] - %s) |' % reason)
    else:
        once(m.group(0), cell + ' (regenerated at [B-6b6] - %s) |' % reason)

rows = [
    ('b6b6_schema_ark_rule_dropped', "the ark's row rule is gone ('…방주로 백성을 건져 냈다' on `project_ark`'s completion lines and '노아의 배' as its name pass)",
     '`test_every_rule_of_the_schema_can_fail` (the count), `test_the_ark_and_its_word` (`Tools/test_data_schema.py`)'),
    ('b6b6_schema_ark_guard_projects_dropped', "the projects' `row_not` rule of '방주' is gone ('방주' on `project_test_build`'s completion lines passes)",
     '`test_every_rule_of_the_schema_can_fail` (the count), `test_the_ark_and_its_word` (`Tools/test_data_schema.py`)'),
    ('b6b6_schema_ark_guard_feats_dropped', "the feats' `row_not` rule of '방주' is gone ('방주' on `feat_test_climb`'s lines passes)",
     '`test_every_rule_of_the_schema_can_fail` (the count), `test_the_ark_and_its_word` (`Tools/test_data_schema.py`)'),
    ('b6b6_schema_ark_words_off_hazards', "`DT_Hazards`' name and three template columns no longer refuse `ark_words`",
     '`test_the_ark_and_its_word` (`Tools/test_data_schema.py`)'),
    ('b6b6_schema_ark_words_off_lines', "`DT_Events.LogTemplates` and `DT_LogTemplates.Templates` no longer refuse `ark_words`",
     '`test_the_ark_and_its_word`, `test_upper_rung_text_words` (the exact lists) (`Tools/test_data_schema.py`)'),
    ('b6b6_schema_myth_without_noah', "'노아' leaves `holy_and_myth`", '`test_the_ark_and_its_word` (`Tools/test_data_schema.py`)'),
    ('b6b6_schema_myth_takes_baro', "holy_and_myth takes a name of the hand list ('바로') (`b4d1_schema_myth_takes_baro` taken again at [B-6b6])",
     '`test_first_person_and_card_words` (`Tools/test_data_schema.py`)'),
    ('b6b6_schema_verdict_without_soul_has_soul', "verdict_without_soul keeps '영혼' (`b4d1_schema_verdict_without_soul_has_soul` taken again at [B-6b6])",
     '`test_first_person_and_card_words` (`Tools/test_data_schema.py`)'),
    ('b6b6_data_extinction_says_ark', "the first resolution line of `crisis_species_extinction` says '방주' (a data patch, refused before an asset is written)",
     "`python -I Tools/check_data.py` (`DT_Hazards.ResolveTemplates`: `ark_words`)"),
    ('b6b6_data_ark_rescues', "the first completion line of `project_ark` says '…구했다' (a data patch, refused before an asset is written)",
     "`python -I Tools/check_data.py` (the ark's row rule: `self_rescue_words`)"),
    ('b6b6_data_ark_word_in_card', "the body of `petition_wise_heart` says '방주' (a data patch that `check_data.py` lets through - `DT_Encounters.Body` refuses the verdict alone)",
     "`test_the_ark_and_its_word` (`Tools/test_data_schema.py` - the whole project outside the two rows)"),
]
add = ''.join('| `B6b/` | `%s.patch` — %s | %s |\n' % row for row in rows)
assert s.endswith('\n')
s = s + add
open(p, 'wb').write(s.encode('utf-8'))
print('README: +%d rows' % len(rows))
