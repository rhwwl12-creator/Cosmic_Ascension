"""Apply each mutation patch, build the editor target (or, for a patch of Tools/data, rebuild the assets), run its test,
revert. Prints one line per patch: KILLED when the test fails under the mutation (what we want), SURVIVED when it passes,
BUILD FAILED when the mutant does not compile; under KILLED, the first failing assertion lines of the log.
Usage (project root, editor closed): python -I run_mutations2.py <patch>=<test filter> ..."""
import os
import re
import subprocess
import sys

sys.stdout.reconfigure(encoding="utf-8")
ROOT = os.getcwd()
UE = r"C:\Program Files\Epic Games\UE_5.8"
BUILD = os.path.join(UE, r"Engine\Build\BatchFiles\Build.bat")
EDITOR = os.path.join(UE, r"Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
PROJECT = os.path.join(ROOT, "Cosmic_Ascension.uproject")
LOGS = os.path.join(ROOT, "Saved", "Logs")
ROOTF = ROOT.replace("\\", "/")


def build():
    r = subprocess.run([BUILD, "Cosmic_AscensionEditor", "Win64", "Development", "-Project=" + PROJECT, "-WaitMutex"],
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    return "Result: Succeeded" in r.stdout


def make_data(tag):
    log = os.path.join(LOGS, "MutationData_%s.log" % tag)
    subprocess.run([EDITOR, PROJECT, "-run=pythonscript", "-Script=%s/Tools/make_data_tables.py" % ROOTF, "-unattended",
                    "-nosplash", "-stdout", "-FullStdOutLogOutput", "-AbsLog=" + log], capture_output=True)
    text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
    return "written and verified" in text


def run_test(test, tag):
    log = os.path.join(LOGS, "Mutation_%s.log" % tag)
    subprocess.run([EDITOR, PROJECT, "-game", "-nullrhi", "-nosound", "-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput",
                    "-ExecCmds=Automation RunTests %s; Quit" % test, "-TestExit=Automation Test Queue Empty", "-AbsLog=" + log],
                   capture_output=True)
    text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
    results = re.findall(r"Test Completed\. Result=\{(\w+)\}", text)
    errors = [l.strip()[:300] for l in text.splitlines() if "Error:" in l and ("Expected" in l or "LogAutomation" in l)][:4]
    return results, errors


# A reverted C++ patch leaves its mutant DLL behind until the next build; a data patch does not build, so it would run its
# test on that mutant (found at [B-6b8b]). Track it and build clean before a data patch runs.
dll_dirty = False
for arg in sys.argv[1:]:
    patch, test = arg.split("=", 1)
    tag = os.path.splitext(os.path.basename(patch))[0]
    data = "_data_" in tag
    if subprocess.run(["git", "-c", "core.autocrlf=false", "-c", "core.eol=lf", "apply", patch]).returncode != 0:
        print(tag, "PATCH DID NOT APPLY", flush=True)
        continue
    try:
        if data:
            if dll_dirty:
                print(tag, "clean build before the data patch:", "ok" if build() else "FAILED", flush=True)
                dll_dirty = False
            print(tag, "assets:", "written" if make_data(tag) else "NOT written (checker stopped)", flush=True)
        else:
            dll_dirty = True
            if not build():
                print(tag, "BUILD FAILED", flush=True)
                continue
        results, errors = run_test(test, tag)
        verdict = "KILLED" if any(r != "Success" for r in results) else ("SURVIVED" if results else "NO RESULT")
        print(tag, verdict, results, flush=True)
        for e in errors:
            print("    ", e, flush=True)
    finally:
        subprocess.run(["git", "-c", "core.autocrlf=false", "-c", "core.eol=lf", "apply", "-R", patch])
        if data:
            print(tag, "assets after revert:", "written" if make_data(tag + "_revert") else "NOT written", flush=True)
print("clean build:", "ok" if build() else "FAILED", flush=True)
