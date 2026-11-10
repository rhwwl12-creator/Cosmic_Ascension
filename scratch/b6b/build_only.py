import os, subprocess, sys
ROOT = os.getcwd()
UE = r"C:\Program Files\Epic Games\UE_5.8"
BUILD = os.path.join(UE, r"Engine\Build\BatchFiles\Build.bat")
PROJECT = os.path.join(ROOT, "Cosmic_Ascension.uproject")
r = subprocess.run([BUILD, "Cosmic_AscensionEditor", "Win64", "Development", "-Project=" + PROJECT, "-WaitMutex"],
                   capture_output=True, text=True, encoding="utf-8", errors="replace")
out = r.stdout + r.stderr
open(sys.argv[1], "w", encoding="utf-8").write(out)
for line in out.splitlines():
    if "error" in line.lower() or "warning C" in line or "Result:" in line:
        print(line[:400])
