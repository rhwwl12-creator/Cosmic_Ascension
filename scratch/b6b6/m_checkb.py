"""[B-6b6] check b (brief 3.2.3): each of the five on-runs in Saved/Balance/b6b6_on against its b6b6_base copy; rows with
Tick <= T equal (TickMicros, SaveBytes, SaveRawBytes aside), T equal; plus check c (end check / race check lines) and each
run's first movers (first differing sample, AI treaty first, first extinction placement)."""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bundleA as B

ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
BAL = os.path.join(ROOT, "Saved", "Balance")
NAMES = [("checks", 3000), ("checks_ruler", 3000), ("checks_wanderer", 6000), ("checks_20000", 20000), ("checks_ruler_20000", 20000)]


def job(folder, name, ticks):
    b = os.path.join(BAL, folder, name)
    return {"name": name, "ticks": ticks, "csv": b + ".csv", "rows": b + "_rows.csv", "claims": b + "_claims.csv", "log": b + ".log"}


allok = True
for name, ticks in NAMES:
    on, base = job("b6b6_on", name, ticks), job("b6b6_base", name, ticks)
    pon, pbase = B.parse_log(on["log"]), B.parse_log(base["log"])
    pr = B.pair_check(on, base, pon, pbase)
    allok &= pr["ok"]
    race = pon.get("race") or {}
    ext = race.get("extinction") or {}
    tr = race.get("treaties") or {}
    hits = B.rows_first(on["rows"], "crisis_species_extinction") if os.path.exists(on["rows"]) else []
    first_ext_row = hits[0] if hits else None
    print("%-20s ok %s T %s (base %s) first diff csv %s rows %s | end check %s race check %s errors %d | AITreatyFirst %s, "
          "extinction on ruled first %s, first rows extinction %s | why %s" % (
              name, pr["ok"], pr["T"], pr["T_off"], pr["first_diff_csv"], pr["first_diff_rows"], pon.get("end_check"),
              pon.get("race_check"), len(pon.get("errors", [])), tr.get("ai_treaty_first"), ext.get("on_ruled_first"), first_ext_row,
              pr["why"]))
    if pon.get("race_text"):
        print("    race: " + pon["race_text"][:1200])
print("CHECK B PASSES" if allok else "CHECK B FAILS")
