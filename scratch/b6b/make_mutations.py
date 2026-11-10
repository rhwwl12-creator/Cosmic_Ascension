"""Write the B-6b1 mutation patches (Tools/mutations/B6b). Run from the project root: python -I make_mutations.py <scratch dir>"""
import os
import subprocess
import sys

T = sys.argv[1]
S = "Source/Cosmic_Ascension"


def mk(name, rel, old, new):
    src = open(rel, encoding="utf-8").read()
    if src.count(old) != 1:
        raise SystemExit("%s: found %d" % (name, src.count(old)))
    for side, text in (("a", src), ("b", src.replace(old, new))):
        path = os.path.join(T, side, rel)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        open(path, "w", encoding="utf-8", newline="\n").write(text)
    out = subprocess.run(["git", "diff", "--no-index", os.path.join(T, "a", rel), os.path.join(T, "b", rel)],
                         capture_output=True, text=True, encoding="utf-8").stdout
    body = "".join(l for l in out.splitlines(True) if not l.startswith(("diff ", "index ", "--- ", "+++ ")))
    patch = "diff --git a/%s b/%s\n--- a/%s\n+++ b/%s\n" % (rel, rel, rel, rel) + body
    open("Tools/mutations/B6b/%s.patch" % name, "w", encoding="utf-8", newline="\n").write(patch)


mk("b6b1_laws_held_initializer_one", S + "/Core/GalaxyState.h", "\tint32 LawsHeldTicks = 0;", "\tint32 LawsHeldTicks = 1;")
mk("b6b1_hasher_skips_counters", S + "/Sim/GalaxySimulationSubsystem.cpp",
   "\t\t\tfor (TFieldIterator<FProperty> It(Struct); It; ++It)\n\t\t\t{\n",
   "\t\t\tfor (TFieldIterator<FProperty> It(Struct); It; ++It)\n\t\t\t{\n"
   "\t\t\t\tif (It->GetFName() == TEXT(\"LawsHeldTicks\") || It->GetFName() == TEXT(\"PressureEnduredTicks\"))\n\t\t\t\t{\n\t\t\t\t\tcontinue;\n\t\t\t\t}\n")
mk("b6b1_move_capital_keeps_endured", S + "/Sim/CosmicHazards.cpp", "\tCiv.PressureEnduredTicks = 0;\n", "")
mk("b6b1_endured_capital_inside", S + "/Sim/GalaxySimulationSubsystem.cpp", "\t\t\tEnduredPressure[CivIndex] = !bHomeInside;", "\t\t\tEnduredPressure[CivIndex] = true;")
mk("b6b1_endured_any_sphere", S + "/Sim/GalaxySimulationSubsystem.cpp",
   "bHomeInside = FallenPresser[TranscendentIndex] && InSphere[TranscendentIndex].IsValidIndex(HomeIndex)",
   "bHomeInside = InSphere[TranscendentIndex].IsValidIndex(HomeIndex)")
mk("b6b1_endured_ruled_only", S + "/Sim/GalaxySimulationSubsystem.cpp", "			EnduredPressure[CivIndex] = !bHomeInside;", "			EnduredPressure[CivIndex] = !bHomeInside && Civ.bIsPlayerRuled;")
mk("b6b1_endured_fallen_drain_only", S + "/Sim/GalaxySimulationSubsystem.cpp", "			EnduredPressure[CivIndex] = !bHomeInside;", "			EnduredPressure[CivIndex] = !bHomeInside && InFallenSphere[CivIndex];")
print("written")
