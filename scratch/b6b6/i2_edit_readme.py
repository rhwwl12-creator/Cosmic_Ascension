import os
p = os.path.join(r"c:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension", "Tools", "mutations", "README.md")
s = open(p, 'rb').read().decode('utf-8')
assert '\r' not in s


def rep(old, new):
    global s
    assert s.count(old) == 1, (old[:90], s.count(old))
    s = s.replace(old, new)


rep("`b6b3a_data_far_line_shares_text` and `b6b4_data_bound_words_seal`) needs the assets rebuilt",
    "`b6b3a_data_far_line_shares_text`, `b6b4_data_bound_words_seal` and `b6b6_data_extinction_expiry_effect`) needs the assets rebuilt")
rep("`CosmicHazards::KillsTheRulerOf` is true for `Hazard.Plague` as well | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the one answer) |",
    "`CosmicHazards::KillsTheRulerOf` is true for `Hazard.Plague` as well | `Cosmic_Ascension.Sim.ExtinctionExpiry` (the one answer); from [B-6b6] `Cosmic_Ascension.Data.Integrity` too (the code's one death: two rows kill the ruler) |")
rep("| `Cosmic_Ascension.Data.Integrity` (the ark's margin under the game's rules) |",
    "| `Cosmic_Ascension.Data.Integrity` (the ark's margin under the game's rules; from [B-6b6] on the game's own extinction and ark as well) |")
rep("| `Cosmic_Ascension.Data.Integrity` (today's data: project_binding alone; the ten on the full copy) |",
    "| `Cosmic_Ascension.Data.Integrity` (today's data - from [B-6b6] its four rows; the ten on the full copy) |")
rep("`WithoutB6bData` keeps the extinction row | `Cosmic_Ascension.Data.Integrity` (the ten taken out; no hazard kills the ruler, either order) |",
    "`WithoutB6bData` keeps the extinction row | `Cosmic_Ascension.Data.Integrity` (the ten taken out; no hazard kills the ruler, either order; from [B-6b6] today's four rows too) |")
if not s.endswith('\n'):
    s += '\n'
s += ("| `B6b/` | `b6b6_kills_ruler_not_extinction.patch` — `CosmicHazards::KillsTheRulerOf` is never true (no row's expiry kills the ruler) | `Cosmic_Ascension.Data.Integrity` (the code's one death: exactly `crisis_species_extinction` kills the ruler); `Cosmic_Ascension.Sim.ExtinctionExpiry` |\n"
      "| `B6b/` | `b6b6_data_extinction_expiry_effect.patch` — the schema's extinction rule (rule 5) keeps `rows_of` alone, and `crisis_species_extinction` carries a `ChangeStability` on its expiry (a data patch: rebuild the assets - `check_data.py` lets it through) | `Cosmic_Ascension.Data.Integrity` (the extinction's expiry carries no effect of the data) |\n")
open(p, 'wb').write(s.encode('utf-8'))
print("ok")
