"""Write the three [B-6b10] mutation patches from clean `git show HEAD:` copies ({P} = 5ed07e1).
Copies and edited copies land in scratchpad/b6b10/src/; patches in Tools/mutations/B6b/."""
import difflib, os, subprocess, sys
ROOT = r"C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension"
SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "src")
OUT = os.path.join(ROOT, "Tools", "mutations", "B6b")

def show(path):
    b = subprocess.run(["git", "-C", ROOT, "show", "HEAD:" + path], capture_output=True, check=True).stdout
    assert b"\r" not in b, path
    return b.decode("utf-8")

def sub1(text, a, b):
    assert text.count(a) == 1, (a, text.count(a))
    return text.replace(a, b)

def write_patch(name, path, before, after):
    base = os.path.join(SRC, name)
    open(base + ".orig", "w", encoding="utf-8", newline="").write(before)
    open(base + ".new", "w", encoding="utf-8", newline="").write(after)
    lines = list(difflib.unified_diff(before.splitlines(True), after.splitlines(True), "a/" + path, "b/" + path, n=3))
    assert lines
    # difflib writes "--- a/x\n" with no date when none is given; keep git's form.
    body = "diff --git a/%s b/%s\n" % (path, path) + "".join(l if l.endswith("\n") else l + "\n" for l in lines)
    open(os.path.join(OUT, name + ".patch"), "w", encoding="utf-8", newline="").write(body)
    print("wrote", name)

# 1. The latch compares the record with the knob.
p = "Source/Cosmic_Ascension/World/CosmicGameMode.cpp"
t = show(p)
write_patch("b6b10_latch_reads_the_knob", p, t, sub1(t,
    "\t\tif (!Ending.Kind.IsValid())\n\t\t{\n\t\t\treturn;\n\t\t}\n\t\tRunEnd = Ending.Kind;",
    "\t\tif (!Ending.Kind.IsValid() || (Ending.Kind == CosmicTags::EndingClock() && Ending.Tick < Sim->GetRules().ProvisionalEndTick))\n\t\t{\n\t\t\treturn;\n\t\t}\n\t\tRunEnd = Ending.Kind;"))

# 2. MarkOf compares the record with the knob (and includes the tags it then names).
p = "Source/Cosmic_Ascension/Save/CosmicSaveSubsystem.cpp"
t = show(p)
u = sub1(t, '#include "Core/CosmicIDs.h"\n#include "Core/GalaxyState.h"\n', '#include "Core/CosmicIDs.h"\n#include "Core/CosmicTags.h"\n#include "Core/GalaxyState.h"\n')
u = sub1(u, "MarkOf(const FGalaxyState& State, const FCosmicRulesDef& /*Rules*/)", "MarkOf(const FGalaxyState& State, const FCosmicRulesDef& Rules)")
u = sub1(u, "\tMark.bEnded = State.Ending.Kind.IsValid();",
    "\tMark.bEnded = State.Ending.Kind.IsValid() && !(State.Ending.Kind == CosmicTags::EndingClock() && State.Ending.Tick < Rules.ProvisionalEndTick);")
write_patch("b6b10_markof_reads_the_knob", p, t, u)

# 3. The data back at the B6a start value (on the tree with this commit's C = 9000).
p = "Tools/data/DT_Rules.json"
t = sub1(show(p), '"ProvisionalEndTick": 6000,', '"ProvisionalEndTick": 9000,')
cur = open(os.path.join(ROOT, p), encoding="utf-8", newline="").read()
assert cur == t, "working DT_Rules.json differs from HEAD + the C edit"
write_patch("b6b10_data_clock_six_thousand", p, t, sub1(t, '"ProvisionalEndTick": 9000,', '"ProvisionalEndTick": 6000,'))
