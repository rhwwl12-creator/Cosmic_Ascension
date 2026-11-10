# I2 of [B-6b8]: the README rows of the two C++ patches, appended at the end of the table (after I1's rows).
import sys

PATH = sys.argv[1]
ROWS = [
    "| `B6b/` | `b6b8_wanderer_nocheck_not_by_row.patch` — the wanderer policy counts every accepted answer in `NoCheckByRow` "
    "(the `CheckStat` of the choice taken not read: a passed or failed check counts as a step back) | `Cosmic_Ascension.Sim.WitnessCard` "
    "(the policy answering two witness cards in turn - the check, then the step back - counts one answer without a check) |",
    "| `B6b/` | `b6b8_cmdlet_witness_first_any_card.patch` — the race line's witness `first at` is set at the first boundary any card "
    "of the wanderer policy came (not a rise of `CardsByRow[enc_witness]`; measurement only - no test reads the line) | "
    "the `bundleB.py --max-seed 2` smoke on the wanderer cell (its W controls: `first at` >= 0 if and only if witness cards > 0, "
    "W not before the wanderer left nor before the first claim placed) - a harness check, not an automation test |",
]
with open(PATH, "rb") as f:
    raw = f.read()
assert b"\r\n" not in raw and raw.endswith(b"\n")
text = raw.decode("utf-8")
for row in ROWS:
    name = row.split("`")[3]
    assert name not in text, name
lines = text.split("\n")
assert lines[-1] == "" and lines[-2].startswith("| `B6b/` |"), lines[-2][:40]
text = text + "\n".join(ROWS) + "\n"
with open(PATH, "wb") as f:
    f.write(text.encode("utf-8"))
print("ok")
