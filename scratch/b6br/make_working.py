"""[B-6br] brief_working.md = working_head.md (with the build result) + brief_final.md with the scalar placeholders filled.
usage: python -I make_working.py <out dir> <build result sentence>"""
import os
import sys

out, build = sys.argv[1], sys.argv[2]
head = open(os.path.join(out, "working_head.md"), encoding="utf-8").read().replace("BUILD_RESULT_PLACEHOLDER", build)
body = open(os.path.join(out, "brief_final.md"), encoding="utf-8").read()
for k, v in (("{HBR}", "8b6c5a7"), ("{APPROVAL_DATE}", "2026-11-10"), ("{H_BYTES0}", "148,811"), ("{TESTS0}", "225")):
    body = body.replace(k, v)
fixes = [
    ("If the runner does not echo UBT's diagnostics, run §0.5's `Build.bat` on ROOT once right before §6 (nothing builds) and save its output.",
     "[WC drift 2: an up-to-date build prints no `[Adaptive Build]` line - ROOT's start build had none.] If the runner "
     "does not echo UBT's diagnostics, take the line from the mutation stage's rebuild output or copy UBT's "
     "`C:\\Users\\rhwwl\\AppData\\Local\\UnrealBuildTool\\Log.txt` right after the final runner's editor build."),
    ("`GalaxyLawRowsOf` mutates its copy with `RemoveAll` (`Sim/CosmicEdicts.cpp:217`)",
     "`GalaxyLawRowsOf` mutates its copy with `RemoveAll` (`Sim/CosmicEdicts.cpp:218` [WC])"),
    ("Insert just before `$baselinePath = …` (`:101` at HEAD).", "Insert just before `$baselinePath = …` (`:100` at 8b6c5a7 [WC])."),
]
for a, b in fixes:
    assert a in body, a
    body = body.replace(a, b)
with open(os.path.join(out, "brief_working.md"), "w", encoding="utf-8", newline="\n") as f:
    f.write(head + body)
print("brief_working.md", len(head + body), "chars")
