# I2 of [B-6b8]: write the two C++ mutation patches against the working tree (unified diff, 3 lines of context, LF).
import difflib
import os
import sys

ROOT = sys.argv[1]
OUT = os.path.join(ROOT, "Tools", "mutations", "B6b")

PATCHES = [
    ("b6b8_wanderer_nocheck_not_by_row.patch", "Source/Cosmic_Ascension/World/CosmicWandererPolicy.cpp",
     "\t\tif (bNoCheck && Taken.Outcome != ECosmicPetitionOutcome::Withdrawn)\n",
     "\t\tif (Taken.Outcome != ECosmicPetitionOutcome::Withdrawn)\n"),
    ("b6b8_cmdlet_witness_first_any_card.patch", "Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp",
     "\t\t\t\tRace.WitnessFirst = (Race.WitnessFirst < 0 && RaceWitnessCardsCame > 0) ? State.Tick : Race.WitnessFirst;\n",
     "\t\t\t\tRace.WitnessFirst = (Race.WitnessFirst < 0 && WandererCounts.Cards > 0) ? State.Tick : Race.WitnessFirst;\n"),
]

for name, rel, old, new in PATCHES:
    with open(os.path.join(ROOT, rel), "rb") as f:
        raw = f.read()
    assert b"\r\n" not in raw
    text = raw.decode("ascii")
    assert text.count(old) == 1, (rel, text.count(old))
    before = text.splitlines(keepends=True)
    after = text.replace(old, new).splitlines(keepends=True)
    diff = list(difflib.unified_diff(before, after, "a/" + rel, "b/" + rel, n=3))
    # difflib writes '--- a/x\n' with no date when none is given; git apply takes it. Hunk header lines as git writes them.
    body = "diff --git a/%s b/%s\n" % (rel, rel) + "".join(diff)
    with open(os.path.join(OUT, name), "wb") as f:
        f.write(body.encode("ascii"))
    print(name)
    print(body)
