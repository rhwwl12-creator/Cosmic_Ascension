"""[B-6b8] fix stage: write the new mutation patches (L3-1, L3-2) and regenerate the two whose context holds the witness_words
line (L1-2) against the working tree, with LF endings. usage: python -I -X utf8 make_patches.py <project root>"""
import difflib
import os
import sys

root = sys.argv[1]
OUT = os.path.join(root, "Tools", "mutations", "B6b")
S = "Source/Cosmic_Ascension/"


def read(rel):
    with open(os.path.join(root, rel), "rb") as f:
        b = f.read()
    assert b"\r\n" not in b, rel
    return b.decode("utf-8")


def hunk(rel, edits):
    """edits: [(old, new)] each found exactly once."""
    a = read(rel)
    b = a
    for old, new in edits:
        assert b.count(old) == 1, (rel, old[:80], b.count(old))
        b = b.replace(old, new)
    lines = list(difflib.unified_diff(a.splitlines(True), b.splitlines(True), "a/" + rel, "b/" + rel, n=3))
    assert lines, rel
    return "diff --git a/%s b/%s\n" % (rel, rel) + "".join(lines)


def write(name, parts):
    path = os.path.join(OUT, name)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("".join(parts))
    print("wrote", path)


POLICY = S + "World/CosmicWandererPolicy.cpp"
NOCHECK = "\t\tconst bool bNoCheck = Row->Choices[Counts.Answers % Row->Choices.Num()].CheckStat == ECosmicCheckStat::NoCheck;\n"
ANSWERS = "\t\tCounts.Answers += (Taken.Outcome == ECosmicPetitionOutcome::Withdrawn) ? 0 : 1;\n"

# L3-1 (a): the choice is read after the count of answers moved on.
write("b6b8_wanderer_nocheck_reads_next_choice.patch", [hunk(POLICY, [(NOCHECK + ANSWERS, ANSWERS + NOCHECK)])])
# L3-1 (b): the predicate turned over - the check counted, not the step back.
write("b6b8_wanderer_nocheck_inverted.patch", [hunk(POLICY, [(NOCHECK, NOCHECK.replace("== ECosmicCheckStat::NoCheck", "!= ECosmicCheckStat::NoCheck"))])])

# L3-2: past the checker - the shape rule keeps Borders alone, its unit tests lose the two Ruled expects (walker 330 -> 329),
# and event_era_endured drops its SystemOwnedBy Ruled.
SHAPE_OLD = '"then": [{"col": "Conditions", "has_cond": {"type": "SystemOwnedBy", "key": "Ruled"}}, {"col": "Effects", "has_cond": {"type": "ChangeReverence", "key": "Borders"}}]'
SHAPE_NEW = '"then": [{"col": "Effects", "has_cond": {"type": "ChangeReverence", "key": "Borders"}}]'
T_OLD1 = ('        self.expect(self.after(era([endured], borders)), "DT_Events.event_test_harvest", shape_why, json.dumps(shape_then[0]))\n')
T_OLD2 = ('        # The game\'s era row is in the rule\'s scope: its Ruled taken out is refused.\n'
          '        self.expect(game("DT_Events", "event_era_endured", lambda row: row.update(Conditions=[endured])),\n'
          '                    "DT_Events.event_era_endured", shape_why, json.dumps(shape_then[0]))\n')
WALK_OLD = "        self.assertEqual(walked, 330, "
WALK_NEW = "        self.assertEqual(walked, 329, "
ERA_OLD = ('\t\t"Name": "event_era_endured",\n\t\t"Category": "Event.Civ.Society",\n\t\t"LogCategory": "Log.Civ",\n\t\t"Trigger": "Standing",\n'
           '\t\t"Subject": "Civilization",\n\t\t"Conditions": [\n\t\t\t{\n\t\t\t\t"Type": "SystemOwnedBy",\n\t\t\t\t"Tag": "",\n\t\t\t\t"Key": "Ruled",\n'
           '\t\t\t\t"Value": 0.0\n\t\t\t},\n\t\t\t{\n\t\t\t\t"Type": "CivPressureEnduredTicksReached",')
ERA_NEW = ('\t\t"Name": "event_era_endured",\n\t\t"Category": "Event.Civ.Society",\n\t\t"LogCategory": "Log.Civ",\n\t\t"Trigger": "Standing",\n'
           '\t\t"Subject": "Civilization",\n\t\t"Conditions": [\n\t\t\t{\n\t\t\t\t"Type": "CivPressureEnduredTicksReached",')
write("b6b8_data_era_no_ruled_past_checker.patch", [
    hunk("Tools/data/DT_Events.json", [(ERA_OLD, ERA_NEW)]),
    hunk("Tools/data/schema.json", [(SHAPE_OLD, SHAPE_NEW)]),
    hunk("Tools/test_data_schema.py", [(T_OLD1, ""), (T_OLD2, ""), (WALK_OLD, WALK_NEW)]),
])

# L1-2: the two patches whose context holds the witness_words line, regenerated on the longer list.
LAMP_LIST_KEY = '"desolation_words": ['
schema = read("Tools/data/schema.json")
line = next(l for l in schema.split("\n") if LAMP_LIST_KEY in l)
assert line.count('"등불", ') == 1 or line.count(', "등불"') == 1, line
lamp_new = line.replace('"등불", ', "", 1) if '"등불", ' in line else line.replace(', "등불"', "", 1)
write("b6b8_schema_desolation_without_lamp.patch", [hunk("Tools/data/schema.json", [(line + "\n", lamp_new + "\n")])])
wline = next(l for l in schema.split("\n") if '"witness_words": [' in l)
w_new = wline.replace('"기록했", ', "", 1).replace('"떠났", ', "", 1)
assert w_new != wline and w_new.count('"기록했"') == 0 and w_new.count('"떠났"') == 0
write("b6b8_schema_witness_past_forms_dropped.patch", [hunk("Tools/data/schema.json", [(wline + "\n", w_new + "\n")])])
# L1-2: the -ㄴ/-ㄹ and -ㅁ forms leave the list (the new breaking cases fail first, then the exact list).
NEW_FORMS = ', "말한", "말할", "전한", "전할", "기록한", "기록할", "떠난", "떠날", "지나간", "지나갈", "말함", "전함", "기록함", "떠남", "지나감"]'
assert wline.count(NEW_FORMS) == 1
write("b6b8_schema_witness_adnominal_forms_dropped.patch", [hunk("Tools/data/schema.json", [(wline + "\n", wline.replace(NEW_FORMS, "]") + "\n")])])
