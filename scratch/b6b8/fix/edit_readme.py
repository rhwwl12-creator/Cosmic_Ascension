import sys
p = sys.argv[1]
b = open(p, 'rb').read().decode('utf-8')
assert '\r\n' not in b
lines = b.split('\n')
def after(prefix, row):
    idx = [i for i, l in enumerate(lines) if l.startswith(prefix)]
    assert len(idx) == 1, (prefix, idx)
    lines.insert(idx[0] + 1, row)
after("| `B6b/` | `b6b8_schema_witness_past_forms_dropped.patch`",
      "| `B6b/` | `b6b8_schema_witness_adnominal_forms_dropped.patch` — the forms ending in -ㄴ/-ㄹ and -ㅁ ('말한', '기록한', '떠난', '떠남' and the rest, the fix stage's review L1-2) leave `witness_words` ('떠난 뒤', '기록한 것', '말한다', '떠남' pass, since '떠나', '기록하' and '말하' are not in them) | `test_the_witness_words` (the -ㄴ/-ㄹ/-ㅁ cases, then the exact list) (`Tools/test_data_schema.py`) |")
after("| `B6b/` | `b6b8_data_era_no_ruled.patch`",
      "| `B6b/` | `b6b8_data_era_no_ruled_past_checker.patch` — the era's shape rule keeps its `Borders` test alone, the checker's unit tests lose the two `Ruled` expects and count 329 then-tests, and `event_era_endured` drops its `SystemOwnedBy Ruled` (review L3-2: `b6b8_data_era_no_ruled` never gets past `check_data.py`; a data patch: rebuild the assets - `check_data.py` and `test_data_schema.py`, which `make_data_tables.py` runs first, let it through) | `Cosmic_Ascension.Sim.EraEndured` (an AI people under the same drain gets no era line; its neighbours gain nothing) |")
after("| `B6b/` | `b6b8_wanderer_nocheck_not_by_row.patch`",
      "| `B6b/` | `b6b8_wanderer_nocheck_reads_next_choice.patch` — the wanderer policy reads the `CheckStat` of the choice after the count of answers moved on (the check counted as a step back and the step back as a check: the total stays one; review L3-1) | `Cosmic_Ascension.Sim.WitnessCard` (per boundary: no answer without a check after the first, one after the second) |")
after("| `B6b/` | `b6b8_wanderer_nocheck_reads_next_choice.patch`",
      "| `B6b/` | `b6b8_wanderer_nocheck_inverted.patch` — the wanderer policy counts the answers with a check in `NoCheckByRow` (`!=` for `==`: the total stays one; review L3-1) | `Cosmic_Ascension.Sim.WitnessCard` (per boundary: no answer without a check after the first) |")
b = '\n'.join(lines)
old = "`b6b8_data_era_borders_0_3`, `b6b8_data_witness_reverence_0_3` and `b6b8_data_extinction_expiry_effect`) needs the assets rebuilt"
assert b.count(old) == 1
b = b.replace(old, "`b6b8_data_era_borders_0_3`, `b6b8_data_witness_reverence_0_3`, `b6b8_data_extinction_expiry_effect` and `b6b8_data_era_no_ruled_past_checker`) needs the assets rebuilt")
open(p, 'wb').write(b.encode('utf-8'))
print("ok")
