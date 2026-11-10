"""[B-6b8] I1: append the eight data/schema mutation rows to Tools/mutations/README.md (LF, final newline kept).
usage: i1_readme_rows.py <project root>"""
import sys

path = sys.argv[1] + "/Tools/mutations/README.md"
data = open(path, "rb").read()
assert b"\r" not in data and data.endswith(b"\n")
text = data.decode("utf-8")
ROWS = [
    ("b6b8_schema_witness_rule_dropped", "the witness rule of `DT_Encounters` is gone ('증언' in a label, '목소리' in the body, '기록' outside a success line pass on a wanderer's card on the claim)",
     "`test_every_rule_of_the_schema_can_fail` (the count), `test_the_witness_words` (`Tools/test_data_schema.py`)"),
    ("b6b8_schema_witness_words_off_body", "the witness rule hangs `sign_voice_words` on the title instead of the body (a voice of the sign in a claim card's body passes; the count stays)",
     "`test_the_witness_words` (the body's voice case) (`Tools/test_data_schema.py`)"),
    ("b6b8_schema_witness_past_forms_dropped", "'기록했' and '떠났' leave `witness_words` (the contracted past forms: '기록했다' and '떠났다' pass, since '기록하' and '떠나' are not in them)",
     "`test_the_witness_words` (the '기록했다' and '떠났다' cases, then the exact list) (`Tools/test_data_schema.py`)"),
    ("b6b8_data_witness_says_jeungeon", "the first label of `enc_witness` says '증언' (a data patch, refused before an asset is written)",
     "`python -I Tools/check_data.py` (the witness rule: `witness_words` on `Choices.Label`)"),
    ("b6b8_schema_era_shape_dropped", "the era's shape rule of `DT_Events` is gone (a row on `CivPressureEnduredTicksReached` without `SystemOwnedBy Ruled`, or with the context people's reverence, passes)",
     "`test_every_rule_of_the_schema_can_fail` (the count), `test_the_witness_words` (the era's cases) (`Tools/test_data_schema.py`)"),
    ("b6b8_data_era_no_ruled", "`event_era_endured` drops its `SystemOwnedBy Ruled` (every AI people's endurance would lift its neighbours; a data patch, refused before an asset is written)",
     "`python -I Tools/check_data.py` (the era's shape rule: decision 22)"),
    ("b6b8_data_era_borders_0_3", "`event_era_endured` raises the neighbours' reverence by 0.3 (a data patch that the checker and its unit tests let through: rebuild the assets)",
     "`Cosmic_Ascension.Sim.EraEndured` (each bordering people gains 0.2)"),
    ("b6b8_data_witness_reverence_0_3", "the first choice of `enc_witness` raises the owner's reverence by 0.3 (a data patch that the checker and its unit tests let through: rebuild the assets)",
     "`Cosmic_Ascension.Sim.WitnessCard` (a passed check gives the owner 0.1)"),
]
for name, what, breaks in ROWS:
    assert ("`%s.patch`" % name) not in text, name
    text += "| `B6b/` | `%s.patch` — %s | %s |\n" % (name, what, breaks)
open(path, "wb").write(text.encode("utf-8"))
print("appended", len(ROWS))
