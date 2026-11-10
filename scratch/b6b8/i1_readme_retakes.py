"""[B-6b8] I1: README rows of the three retaken patches - the old rows marked, the new rows appended, the asset list of
line 5 extended. usage: i1_readme_retakes.py <project root>"""
import sys

path = sys.argv[1] + "/Tools/mutations/README.md"
data = open(path, "rb").read()
assert b"\r" not in data and data.endswith(b"\n")
text = data.decode("utf-8")


def once(old, new):
    global text
    assert text.count(old) == 1, (text.count(old), old[:80])
    text = text.replace(old, new)


once("`b6b7_data_ark_margin_half`, `b6b8_data_era_borders_0_3` and `b6b8_data_witness_reverence_0_3`) needs the assets rebuilt",
     "`b6b7_data_ark_margin_half`, `b6b8_data_era_borders_0_3`, `b6b8_data_witness_reverence_0_3` and `b6b8_data_extinction_expiry_effect`) needs the assets rebuilt")
once("— '등불' leaves `desolation_words` (regenerated at [B-6b6] - `ark_words` follows the list) |",
     "— '등불' leaves `desolation_words` (regenerated at [B-6b6] - `ark_words` follows the list; no longer applies since [B-6b8] - taken again as `b6b8_schema_desolation_without_lamp`) |")
once("— the era rule is gone ('새벽의 시대' passes) |",
     "— the era rule is gone ('새벽의 시대' passes) (no longer applies since [B-6b8] - taken again as `b6b8_schema_era_rule_dropped`) |")
once("and `crisis_species_extinction` carries a `ChangeStability` on its expiry (a data patch: rebuild the assets - `check_data.py` and `test_data_schema.py`, which `make_data_tables.py` runs first, let it through) |",
     "and `crisis_species_extinction` carries a `ChangeStability` on its expiry (a data patch: rebuild the assets - `check_data.py` and `test_data_schema.py`, which `make_data_tables.py` runs first, let it through; no longer applies since [B-6b8] - taken again as `b6b8_data_extinction_expiry_effect`) |")
ROWS = [
    ("b6b8_schema_desolation_without_lamp", "'등불' leaves `desolation_words` (`b6b5_schema_desolation_without_lamp` taken again at [B-6b8] - the witness lists follow `ark_words`)",
     "`test_upper_rung_text_words` (`Tools/test_data_schema.py`)"),
    ("b6b8_schema_era_rule_dropped", "the era words rule is gone ('새벽의 시대' passes; the era's shape rule stays) (`b6b5_schema_era_rule_dropped` taken again at [B-6b8] - the shape rule follows it)",
     "`test_every_rule_of_the_schema_can_fail`, `test_upper_rung_text_words` (`Tools/test_data_schema.py`)"),
    ("b6b8_data_extinction_expiry_effect", "the schema's extinction rule (rule 5) keeps `rows_of` alone, the checker's unit tests lose rule 5's three breaking cases and count 326 then-tests, and `crisis_species_extinction` carries a `ChangeStability` on its expiry (`b6b6_data_extinction_expiry_effect` taken again at [B-6b8] - the count; a data patch: rebuild the assets - `check_data.py` and `test_data_schema.py`, which `make_data_tables.py` runs first, let it through)",
     "`Cosmic_Ascension.Data.Integrity` (the extinction's expiry carries no effect of the data)"),
]
for name, what, breaks in ROWS:
    assert ("`%s.patch`" % name) not in text, name
    text += "| `B6b/` | `%s.patch` — %s | %s |\n" % (name, what, breaks)
open(path, "wb").write(text.encode("utf-8"))
print("done")
