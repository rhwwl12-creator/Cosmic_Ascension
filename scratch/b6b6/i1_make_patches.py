"""I1: write the [B-6b6] Python-side mutation patches (new ones and retakes) from the current project files, without
changing the project's sources. usage: i1_make_patches.py <project root>"""
import difflib, os, re, sys
root = sys.argv[1]

def read(rel):
    return open(os.path.join(root, rel), 'rb').read().decode('utf-8')

def patch_text(rel, old, new):
    assert old != new, rel
    diff = difflib.unified_diff(old.splitlines(keepends=True), new.splitlines(keepends=True), 'a/' + rel, 'b/' + rel, n=3)
    lines = list(diff)
    # difflib writes '--- a/x\n'/'+++ b/x\n' with no date when none is given; keep git's header line first.
    return 'diff --git a/%s b/%s\n' % (rel, rel) + ''.join(lines)

def write_patch(name, rel, new_text, old_text=None):
    old_text = read(rel) if old_text is None else old_text
    p = os.path.join(root, 'Tools', 'mutations', name)
    text = patch_text(rel, old_text, new_text)
    open(p, 'wb').write(text.encode('utf-8'))
    print('wrote', name, text.count('\n@@'), 'hunk(s)')

def once(s, old, new):
    assert s.count(old) == 1, (s.count(old), old[:100])
    return s.replace(old, new)

def retake(old_patch, new_name):
    """The change of an old patch put on the current file: paired -/+ lines by their differing middle, lone - blocks deleted."""
    text = read('Tools/mutations/' + old_patch)
    rel = re.search(r'^\+\+\+ b/(\S+)', text, re.M).group(1)
    cur = read(rel)
    lines = cur.split('\n')
    for hunk in re.split(r'^@@[^\n]*\n', text, flags=re.M)[1:]:
        body = [l for l in hunk.split('\n') if l[:1] in ('-', '+', ' ')]
        minus = [l[1:] for l in body if l.startswith('-')]
        plus = [l[1:] for l in body if l.startswith('+')]
        if plus and len(minus) == len(plus):
            for old, new in zip(minus, plus):
                p = 0
                while p < min(len(old), len(new)) and old[p] == new[p]:
                    p += 1
                s = 0
                while s < min(len(old), len(new)) - p and old[-1 - s] == new[-1 - s]:
                    s += 1
                old_mid, new_mid = old[p:len(old) - s], new[p:len(new) - s]
                head = old[:p] + old_mid
                hits = [i for i, l in enumerate(lines) if l.startswith(head)]
                assert len(hits) == 1, (old_patch, len(hits), head[:80])
                i = hits[0]
                lines[i] = old[:p] + new_mid + lines[i][len(head):]
        elif minus and not plus:
            n = len(minus)
            hits = [i for i in range(len(lines) - n + 1) if lines[i:i + n] == minus]
            assert len(hits) == 1, (old_patch, len(hits))
            del lines[hits[0]:hits[0] + n]
        else:
            raise AssertionError((old_patch, 'unhandled hunk'))
    write_patch(new_name, rel, '\n'.join(lines), cur)

# ---- retakes (apply_before OK -> now FAIL) ----
retake('B4/b4d1_schema_myth_takes_baro.patch', 'B6b/b6b6_schema_myth_takes_baro.patch')
retake('B4/b4d1_schema_verdict_without_soul_has_soul.patch', 'B6b/b6b6_schema_verdict_without_soul_has_soul.patch')
for name in ('b6b5_data_feat_says_bound', 'b6b5_schema_binding_name_dropped', 'b6b5_schema_desolation_without_lamp',
             'b6b5_schema_event_column_without_titles', 'b6b5_schema_event_lines_holy_off', 'b6b5_schema_event_lines_soul_off',
             'b6b5_schema_fifth_feat_rule_dropped', 'b6b5_schema_hazard_columns_without_grasp', 'b6b5_schema_hazard_expiry_rescue_off',
             'b6b5_schema_log_column_without_grasp'):
    retake('B6b/%s.patch' % name, 'B6b/%s.patch' % name)

# ---- new ----
S = 'Tools/data/schema.json'
s = read(S)

def without_block(s, first_line_start):
    """Take out the rule object whose "why" line starts with first_line_start: from its '{' to its closing '}' plus the
    comma before it (it is the last rule of its table) - the rules stay valid JSON."""
    lines = s.split('\n')
    w = [i for i, l in enumerate(lines) if l.startswith(first_line_start)]
    assert len(w) == 1, first_line_start
    start = w[0] - 1
    assert lines[start].strip() == '{'
    end = w[0]
    while lines[end].strip() not in ('}', '},'):
        end += 1
    if lines[end].strip() == '}':          # last rule: the comma of the rule before goes
        assert lines[start - 1].strip() == '},'
        lines[start - 1] = lines[start - 1].replace('},', '}')
    del lines[start:end + 1]
    return '\n'.join(lines)

ark_rule = '\t\t\t\t\t"why": "the people crosses the extinction in the ark it built'
proj_guard = '\t\t\t\t\t"why": "\'방주\' is the ark\'s word: no other project'
feat_guard = '\t\t\t\t\t"why": "\'방주\' is said among the feats by the fourth-rung feat alone'
write_patch('B6b/b6b6_schema_ark_rule_dropped.patch', S, without_block(s, ark_rule))
write_patch('B6b/b6b6_schema_ark_guard_projects_dropped.patch', S, without_block(s, proj_guard))
write_patch('B6b/b6b6_schema_ark_guard_feats_dropped.patch', S, without_block(s, feat_guard))

t = s
t = once(t, '"DisplayName": {"kind": "text", "required": true, "slots": [], "forbidden": ["verdict", "ark_words"]},',
         '"DisplayName": {"kind": "text", "required": true, "slots": [], "forbidden": "verdict"},')
for col in ('OutbreakTemplates', 'ExpireTemplates', 'ResolveTemplates'):
    t = once(t, '"%s": {"kind": "texts", "slots": ["Civ", "System", "Hazard"], "forbidden": ["verdict", "rescue_words", "rung_grasp_words", "title_words", "ark_words"]},' % col,
             '"%s": {"kind": "texts", "slots": ["Civ", "System", "Hazard"], "forbidden": ["verdict", "rescue_words", "rung_grasp_words", "title_words"]},' % col)
write_patch('B6b/b6b6_schema_ark_words_off_hazards.patch', S, t)

t = s
t = once(t, '"forbidden": ["verdict", "soul_verbs", "holy_words", "rescue_words", "rung_grasp_words", "title_words", "ark_words"]\n',
         '"forbidden": ["verdict", "soul_verbs", "holy_words", "rescue_words", "rung_grasp_words", "title_words"]\n')
t = once(t, '"forbidden": ["verdict", "holy_words", "soul_verbs", "rescue_words", "rung_grasp_words", "title_words", "ark_words"]},',
         '"forbidden": ["verdict", "holy_words", "soul_verbs", "rescue_words", "rung_grasp_words", "title_words"]},')
write_patch('B6b/b6b6_schema_ark_words_off_lines.patch', S, t)

write_patch('B6b/b6b6_schema_myth_without_noah.patch', S, once(s, '"마라나타", "노아"]', '"마라나타"]'))

H = 'Tools/data/DT_Hazards.json'
write_patch('B6b/b6b6_data_extinction_says_ark.patch', H,
            once(read(H), '"{System}의 위기가 그쳤다. 장이 서는 날이면 {Civ}의 장터가 붐볐다."',
                 '"{System}의 위기가 그쳤다. 장이 서는 날이면 {Civ}의 장터가 붐볐다. 방주는 그 곁에 서 있었다."'))
M = 'Tools/data/DT_Megaprojects.json'
write_patch('B6b/b6b6_data_ark_rescues.patch', M,
            once(read(M), '"{Civ}|hpp(이,가) {System}에서 {Project}|hpp(을,를) 완성했다."',
                 '"{Civ}|hpp(이,가) {System}에서 {Project}|hpp(을,를) 완성했다. 그들은 그것으로 백성을 구했다."'))
E = 'Tools/data/DT_Encounters.json'
write_patch('B6b/b6b6_data_ark_word_in_card.patch', E,
            once(read(E), '"Body": "의회가 첫 청을 올렸다. 이 백성이 무엇을 먼저 구할지 당신이 정해 달라고 했다."',
                 '"Body": "의회가 첫 청을 올렸다. 이 백성이 무엇을 먼저 구할지 당신이 정해 달라고 했다. 방주의 일은 아직 멀다."'))
