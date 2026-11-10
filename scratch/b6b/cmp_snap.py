import os, sys, shutil
snap = sys.argv[1]
restore = len(sys.argv) > 2 and sys.argv[2] == "--restore"
for rel in ["Source/Cosmic_Ascension/Events/CosmicEventSubsystem.cpp", "Source/Cosmic_Ascension/Sim/CosmicDiplomacy.cpp",
            "Source/Cosmic_Ascension/Sim/CosmicEdicts.cpp", "Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp",
            "Tools/data/DT_Rules.json"]:
    a = open(rel, "rb").read(); b = open(os.path.join(snap, rel), "rb").read()
    print(rel, "crlf now", a.count(b"\r\n"), "snap", b.count(b"\r\n"), "same text", a.replace(b"\r\n", b"\n") == b.replace(b"\r\n", b"\n"))
    if restore and a.replace(b"\r\n", b"\n") == b.replace(b"\r\n", b"\n"):
        shutil.copyfile(os.path.join(snap, rel), rel)
        print("  restored")
